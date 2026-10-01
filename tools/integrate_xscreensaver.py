"""Generate the CMSIS source group from the pinned CPU inventory."""
import json,re
from pathlib import Path
root=Path(__file__).resolve().parents[1]
manifest=json.loads((root/'third_party/xscreensaver/inventory.json').read_text())
glnames={p.stem for p in (root/'third_party/xscreensaver/hacks/config').glob('*.xml') if 'gl="yes"' in p.read_text(encoding='latin-1')}
unknown=[n for n in manifest['excluded'] if n not in glnames]
print('Non-CPU catalog names without matching GL source:',unknown)
sources=['../third_party/xscreensaver/'+e['source'] for e in manifest['cpu'] if e['status']!='excluded-memory']
sources=list(dict.fromkeys(sources))
sources+=['../third_party/xscreensaver/hacks/'+s+'.c' for s in ['xlockmore','analogtv','apple2','pacman_ai','pacman_level','asm6502','delaunay','ansi-tty','bubbles-default']]
sources+=['../third_party/xscreensaver/utils/'+s+'.c' for s in ['hsv','colors','spline','erase','xshm','xdbe','aligned_malloc','thread_util','pow2','xft','xftwrap','utf8wc','font-retry','easing','doubletime']]
sources+=['./xs_port/'+p.name for p in sorted((root/'M55_HP/xs_port').glob('*.c'))]
p=root/'M55_HP/M55_HP.cproject.yml'
text=p.read_text()
groups=['  define:', '    - HAVE_CONFIG_H', '    - STANDALONE', '  add-path:', '    - ./xs_port', '    - ../third_party/xscreensaver/hacks', '    - ../third_party/xscreensaver/utils', '    - ../third_party/xscreensaver/jwxyz',
        '  groups:', '    - group: Board application','      files:', '        - file: ./main.c', '    - group: CPU XScreenSaver ports', '      misc:', '        - C:', '            - -Wno-everything', '            - -fno-color-diagnostics', '      files:']
groups+=['        - file: '+s for s in sources]
text=re.sub(r'  (?:define:|groups:).*?(?=  # List components)', '\n'.join(groups)+'\n\n',text,flags=re.S)
p.write_text(text)
p=root/'M55_HP/main.c'
s=p.read_text()
s=s.replace(' * Joystick Pac-Man for the DevKit-E8 standard MIPI LCD.',' * CPU XScreenSaver gallery for the DevKit-E8 standard MIPI LCD.')
s=s.replace('#include "pacman.h"','#include "xs_port/xs_gallery.h"')
s=s.replace('LCD_WIDTH == GAME_LCD_WIDTH && LCD_HEIGHT == GAME_LCD_HEIGHT','LCD_WIDTH == XS_HEIGHT * 2 && LCD_HEIGHT == XS_WIDTH * 2').replace('"Pac-Man UI requires the standard 480 x 800 LCD"','"The gallery requires the standard 480 x 800 LCD"')
s=re.sub(r'/\* E8 mapping.*?(?=static void present)', '',s,flags=re.S)
s=s.replace('        service_input();\n','')
s=s.replace('    joystick_init();\n    pacman_init(&game, ms_ticks);','    xs_gallery_init(ms_ticks);')
s=s.replace('pacman_render(&framebuffers[i][0][0], &game);','xs_gallery_render(&framebuffers[i][0][0]);')
s=s.replace('pacman_render(&framebuffers[back][0][0], &game);','xs_gallery_update(ms_ticks);\n        xs_gallery_render(&framebuffers[back][0][0]);')
p.write_text(s)
print(len(sources),'gallery source files wired into CMSIS')
