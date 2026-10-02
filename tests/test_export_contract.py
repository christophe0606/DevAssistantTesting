"""Regressions for shader normalization and the firmware's delegate IO contract."""
import struct
import sys
from pathlib import Path
from types import SimpleNamespace as NS
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
import torch
from executorch.exir.schema import DelegateCall, Tensor, TensorShapeDynamism
from executorch.exir.scalar_type import ScalarType
from scripts.inspect_program import validate_delegate_io,vela_blocks
from scripts.prepare_shader_graph import prepare_shader_graph


def block(name,data):
    return name.ljust(16,b'\0')+struct.pack('<I',len(data))+bytes(12)+data+bytes((-len(data))%16)


def method(dtype=ScalarType.SHORT,size=252):
    tensor=Tensor(dtype,0,[1,6,42,1],[0,1,2,3],False,0,0,None,TensorShapeDynamism.STATIC)
    output=Tensor(dtype,0,[size],[0],False,0,0,None,TensorShapeDynamism.STATIC)
    return NS(name='ao_half',values=[NS(val=tensor),NS(val=output)],chains=[NS(instructions=[NS(instr_args=DelegateCall(0,[0,1]))])])


class ExportContract(unittest.TestCase):
    def setUp(self):
        row=struct.pack('<10i',1,1,1,1,6,42,1,2,0,1)
        self.payload=block(b'inputs',row)+block(b'outputs',row)

    def test_integer_io(self):
        self.assertEqual(validate_delegate_io(method(),[self.payload]),2)

    def test_float_destination_is_rejected(self):
        plan=method()
        plan.values[1].val.scalar_type=ScalarType.FLOAT
        with self.assertRaisesRegex(ValueError,r'outputs\[0\].*FLOAT'):
            validate_delegate_io(plan,[self.payload])

    def test_wrong_shape_is_rejected(self):
        with self.assertRaisesRegex(ValueError,'PTE shape'):
            validate_delegate_io(method(size=253),[self.payload])

    def test_truncated_block_is_rejected(self):
        with self.assertRaisesRegex(ValueError,'Truncated'):
            vela_blocks(self.payload[:-1])

    def test_normalized_masks_clamps_and_split_preserve_values(self):
        class Shader(torch.nn.Module):
            def forward(self,x):
                a,b,c=torch.split(x,[2,1,1],dim=-1)
                a=a.clamp_min(-.2).clamp_max(.8)
                b=torch.where(b>0,b,0.)
                c=torch.where(c>0,1.,-.5)
                return torch.cat((a,b,c),dim=-1)
        model=Shader()
        x=torch.tensor([[-1.,2.,-.1,.1],[.3,.4,.6,-.2]])
        graph=prepare_shader_graph(torch.export.export(model,(x,)).module())
        torch.testing.assert_close(graph(x),model(x),rtol=0,atol=0)
        for n in graph.graph.nodes:
            self.assertNotIn(n.target,(torch.ops.aten.where.ScalarOther,torch.ops.aten.where.Scalar,torch.ops.aten.split_with_sizes.default,torch.ops.aten.clamp_min.default,torch.ops.aten.clamp_max.default))

    def test_recurrent_slice_comparison_quantization(self):
        from create_ai_layer import quantize_method
        from executorch.backends.arm.ethosu import EthosUCompileSpec
        class State(torch.nn.Module):
            def forward(self,state):
                p,t,tmax,m=state.split(1,dim=-1)
                step=torch.where(t<tmax,t,torch.zeros_like(t))
                return torch.cat((p+step,t,tmax,m),dim=-1)
        module=State()
        x=torch.tensor([[[[.2,.3,.8,1.],[.4,.9,.6,2.]]]])
        spec=EthosUCompileSpec(target='ethos-u85-256')
        method=NS(module=module,example=(x,),samples=[(x,)],activation_bits=16)
        quantized=quantize_method(spec,method)
        torch.testing.assert_close(quantized(x),module(x),rtol=0,atol=.001)


if __name__=='__main__':
    unittest.main()
