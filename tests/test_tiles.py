"""Check global ray coordinates, edge padding, and 2D bilinear tile seams."""
import sys
from pathlib import Path
import unittest
from dataclasses import replace
from unittest.mock import patch
import torch

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'model'))
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from model import rays,render,Upscale,normal,RenderConfig,get_methods
from primitives import c,scene,normalize
from scripts.update_display_config import update_orientation


class Tiles(unittest.TestCase):
    def test_orientation_metadata_preserves_model_contract(self):
        header=(Path(__file__).resolve().parents[1]/'ai_layer/model_io.h').read_text()
        config=RenderConfig.load()
        updated=update_orientation(replace(config,rotation_degrees=0),header)
        self.assertIn('#define SDF_ROTATION_DEGREES 0',updated)
        self.assertEqual(update_orientation(config,updated),header)
        for changed in (replace(config,tile_width=40),replace(config,normal_epsilon=.001)):
            with self.assertRaisesRegex(ValueError,'re-export'):
                update_orientation(changed,header)

    def test_landscape_camera_and_calibration(self):
        config=RenderConfig.load()
        self.assertEqual((config.image_width,config.image_height),(800,480))
        portrait=replace(config,rotation_degrees=0)
        self.assertEqual((portrait.image_width,portrait.image_height),(480,800))
        self.assertEqual(config.shape(False),portrait.shape(False))
        self.assertEqual(config.shape(True),portrait.shape(True))
        # The exporter must calibrate the landscape camera, with unchanged
        # full/half tensor shapes, rather than the physical panel's aspect.
        with patch('model.render') as preview:
            get_methods(config=config)
        self.assertEqual([call.args[:2] for call in preview.call_args_list],
                         [(800,480)]*5+[(400,240)]*5)
        ro,rd,_,_=rays(80,48,time=7)
        tro,trd,_,_=rays(80,48,time=7,row=16,rows=8,col=24,cols=16)
        torch.testing.assert_close(tro,ro)
        torch.testing.assert_close(trd,rd[:,16:24,24:40],atol=0,rtol=0)

    def test_clockwise_display_corners(self):
        config=RenderConfig(display_width=4,display_height=6,tile_width=2,tile_height=2)
        image=torch.arange(24).reshape(1,4,6,1)
        panel=config.to_display(image)
        self.assertEqual(tuple(panel.shape),(1,6,4,1))
        self.assertEqual(panel[0,:,:,0].tolist(),
                         [[18,12,6,0],[19,13,7,1],[20,14,8,2],
                          [21,15,9,3],[22,16,10,4],[23,17,11,5]])
        torch.testing.assert_close(replace(config,rotation_degrees=0).to_display(image),image)
        with self.assertRaisesRegex(ValueError,'Rotation'):
            replace(config,rotation_degrees=45)

    def test_rotated_half_tile_coverage(self):
        # Assemble actual bilinear tile outputs on the rotated panel. Uneven
        # tile counts exercise clipped right/bottom tiles and halo joins.
        config=RenderConfig(display_width=26,display_height=38,tile_width=12,tile_height=8)
        low=torch.rand(1,13,19,3,generator=torch.Generator().manual_seed(9))
        expected=config.to_display(Upscale()(low))
        panel=torch.full_like(expected,float('nan'))
        written=torch.zeros(26*38,dtype=torch.int)
        for top in range(0,26,8):
            for left in range(0,38,12):
                yy=torch.arange(top//2-1,top//2+5).clamp(0,12)
                xx=torch.arange(left//2-1,left//2+7).clamp(0,18)
                tile=Upscale()(low[:,yy][:,:,xx])[:,2:10,2:14]
                h,w=min(8,26-top),min(12,38-left)
                # Logical rectangle maps to swapped physical row/column axes.
                panel[:,left:left+w,26-top-h:26-top]=config.to_display(tile[:,:h,:w])
                for y in range(top,top+h):
                    indices=torch.arange(left,left+w)*26+25-y
                    written[indices]+=1
        self.assertTrue(torch.all(written==1))
        torch.testing.assert_close(panel,expected,atol=2e-7,rtol=2e-7)

    def test_packed_normal_samples(self):
        # Compare the packed image evaluation with four separate shader
        # samples. Include multiple rows and batches to catch layout errors.
        g=torch.Generator().manual_seed(21)
        p=torch.rand(2,3,5,3,generator=g)*c(torch.zeros(1),5,.8,5)+c(torch.zeros(1),-2.5,0,-3)
        expected=torch.zeros_like(p)
        for direction in [(1,-1,-1),(-1,-1,1),(-1,1,-1),(1,1,1)]:
            e=c(p,*direction)*.5773
            expected+=e*scene(p+.0005*e)[0]
        torch.testing.assert_close(normal(p),normalize(expected),atol=2e-6,rtol=2e-6)

    def test_global_rays(self):
        ro,rd,dx,dy=rays(48,80,time=7)
        tro,trd,tdx,tdy=rays(48,80,time=7,row=23,rows=8,col=17,cols=16)
        torch.testing.assert_close(tro,ro)
        for a,b in [(trd,rd),(tdx,dx),(tdy,dy)]:
            torch.testing.assert_close(a,b[:,23:31,17:33],atol=0,rtol=0)

    def test_full_tile_equivalence(self):
        with torch.no_grad():
            full=render(24,40,time=3)
            tile=render(24,40,time=3,row=15,rows=8,col=5,cols=12)
        torch.testing.assert_close(tile,full[:,15:23,5:17],atol=2e-5,rtol=2e-5)

    def test_bilinear_halos(self):
        # Include partial edge tiles; both horizontal and vertical joins must
        # match a single complete-image resize exactly, modulo float rounding.
        g=torch.Generator().manual_seed(9)
        low=torch.rand(1,13,19,3,generator=g)
        high=Upscale()(low)
        assembled=torch.empty_like(high)
        for top in range(0,13,4):
            for left in range(0,19,6):
                yy=torch.arange(top-1,top+5).clamp(0,12)
                xx=torch.arange(left-1,left+7).clamp(0,18)
                tile=low[:,yy][:,:,xx]
                enlarged=Upscale()(tile)[:,2:10,2:14]
                h,w=min(4,13-top)*2,min(6,19-left)*2
                assembled[:,top*2:top*2+h,left*2:left*2+w]=enlarged[:,:h,:w]
        torch.testing.assert_close(assembled,high,atol=2e-7,rtol=2e-7)


if __name__=='__main__':
    torch.set_num_threads(4)
    unittest.main()
