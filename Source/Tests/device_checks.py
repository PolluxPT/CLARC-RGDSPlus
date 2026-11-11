"""Run against Source/device.py; no handheld required."""
import ast,itertools,json,pathlib,sys
source=pathlib.Path(__file__).resolve().parents[1]/'device.py'
tree=ast.parse(source.read_text());names={'CALIBRATION','CAPTURED_KEY_WORDS','physical','mapping'}
selected=[n for n in tree.body if isinstance(n,ast.FunctionDef) or isinstance(n,ast.Assign) and any(isinstance(t,ast.Name) and t.id in names for t in n.targets)]
ns={};exec(compile(ast.Module(body=selected,type_ignores=[]),str(source),'exec'),ns)
f=ns['controls_bits'];physical=ns['physical'];assert physical=={'a':304,'b':305,'x':307,'y':306,'l1':308,'r1':309,'select':310,'start':311,'l2':314,'r2':315}
assert len(ns['sdl_button_codes'](ns['bitmap_words'](ns['CAPTURED_KEY_WORDS'])))==17
# Live EVIOCGBIT byte layout must resolve to the same Chasm button ordering.
b=ns['bitmap_words'](ns['CAPTURED_KEY_WORDS']);assert ns['calibrated_keys'](ns['sdl_button_codes'](int.from_bytes(b.to_bytes(96,'little'),'little')))==physical
# Exhaust physical L1/R1/SELECT/START combinations: no exchanged functions.
for mask in range(16):
 keys={physical[name] for i,name in enumerate(['l1','r1','select','start']) if mask>>i&1}
 value=f(keys,{})
 expected=sum(1<<(24+i) for i in range(4) if mask>>i&1)|((1<<9) if mask&8 else 0)
 assert value==expected,(mask,value,expected)
for x,y in itertools.product([-1,0,1],repeat=2):
 value=f(set(),{5:{16:x,17:y,0:32767,1:-32768}})
 expected=((1<<20) if x<0 else (1<<21) if x>0 else 0)|((1<<22) if y<0 else (1<<23) if y>0 else 0)
 assert value==expected
assert f(set(),{})==0
assert f({308},{})==1<<24 and f({309},{})==1<<25
assert not f({310,311},{})&((1<<24)|(1<<25))
assert not f({314,315},{})&((1<<24)|(1<<25)|(1<<9))
assert f({307},{})==1<<18 and f({306},{})==1<<19
assert f({114,115},{})==0
# Execute the actual exit-chord branch with a fake exit file and controlled times.
branch=next(n for n in ast.walk(tree) if isinstance(n,ast.If) and 'physical[\'select\']' in ast.unparse(n.test))
code=compile(ast.Module(body=[branch],type_ignores=[]),str(source),'exec')
class Exit:
 def __init__(self):self.calls=[]
 def write_text(self,text):self.calls.append(text)
ns.update(start=None,exitfile=Exit())
def step(keys,now):ns.update(keys=keys,now=now);exec(code,ns)
step({308,309},0);step({308,309},2);assert not ns['exitfile'].calls
step({310},3);step({311},5);assert not ns['exitfile'].calls
step({310,311},6);step({310,311},7.49);assert not ns['exitfile'].calls
step({310,311},7.5);assert ns['exitfile'].calls==['START+SELECT\n']
step(set(),8);assert ns['start'] is None
step({310,311},10);step({310},11);step({310,311},11.1);step({310,311},12);assert len(ns['exitfile'].calls)==1
print('PASS: saved Chasm calibration, SDL order, all shoulder/start/select combinations, D-pad, releases, exit hold and volume isolation')
# Packets are also consumed by the real compiled managed helper checks.
packets=[]
for mask in range(16):
 keys={physical[name] for i,name in enumerate(['l1','r1','select','start']) if mask>>i&1};packets.append({'mask':mask,'bits':f(keys,{})})
output=pathlib.Path(sys.argv[1]) if len(sys.argv)>1 else pathlib.Path(__file__).with_name('packets.json')
output.parent.mkdir(parents=True,exist_ok=True);output.write_text(json.dumps(packets))
