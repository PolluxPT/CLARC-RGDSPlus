#!/usr/bin/env python3
"""Local-only, bounded-memory import of an owner's CLARC Linux archive."""
import base64,hashlib,json,os,pathlib,shutil,sys,tarfile,tempfile,zlib
ROOT=pathlib.Path(__file__).resolve().parents[1]
def digest(p):
 h=hashlib.sha256()
 with p.open('rb') as f:
  for block in iter(lambda:f.read(1024*1024),b''):h.update(block)
 return h.hexdigest()
def apply_delta(original,patch):
 if not patch.startswith(b'CLARCP1\n'):raise ValueError('Invalid CLARC patch')
 meta=json.loads(zlib.decompress(patch[8:]))
 if meta['format']!='CLARC-copy-add-delta-1' or hashlib.sha256(original).hexdigest()!=meta['source_sha256']:raise ValueError('This is not the supported original CLARC 1.0.0.3 Linux assembly')
 out=bytearray()
 for op in meta['operations']:
  if op[0]=='copy':
   _,offset,size=op
   if offset<0 or size<0 or offset+size>len(original):raise ValueError('Invalid copy span')
   out.extend(original[offset:offset+size])
  elif op[0]=='add':
   _,offset,encoded=op;diff=base64.b64decode(encoded,validate=True)
   if offset<0 or offset+len(diff)>len(original):raise ValueError('Invalid add span')
   out.extend((original[offset+i]+v)&255 for i,v in enumerate(diff))
  else:raise ValueError('Unknown patch operation')
  if len(out)>meta['target_size']:raise ValueError('Oversized patch output')
 if len(out)!=meta['target_size'] or hashlib.sha256(out).hexdigest()!=meta['target_sha256']:raise ValueError('Patched assembly verification failed')
 return bytes(out)
def valid_file(p,expected):return p.is_file() and digest(p)==expected

def prepare(root):
 root=pathlib.Path(root);rt=root/'runtime';spec=json.loads((rt/'game-import.json').read_text());managed='CLARC_LINUX_Data/Managed/'
 required={
  'game/CLARC_LINUX.x86':spec['executable_sha256'],
  'game/'+managed+'Assembly-CSharp.dll':spec['assembly_patched_sha256'],
  'game/'+managed+'ClarcRGDS.dll':spec['helper_sha256'],
  'game/CLARC_LINUX_Data/Plugins/x86/ScreenSelector.so':spec['selector_sha256'],
  'runtime/libs.x86/libGLU.so.1':spec['glu_sha256'],
  'runtime/libs.x86/libstdc++.so.6':spec['stdcpp_sha256']}
 # A complete existing installation, including the confirmed Alpha1.4, is accepted.
 if all(valid_file(root/name,h) for name,h in required.items()) and all((root/name).is_file() and (root/name).stat().st_size==size for name,size in spec['game_sizes'].items() if name not in required):
  print('Owned game files already prepared.',flush=True);return
 archive=root/'clarc_1.0.0.3.tar.gz'
 if not archive.is_file():raise ValueError('Copy your own clarc_1.0.0.3.tar.gz into APPS/CLARC_RGDSPlus, then launch CLARC again. Your existing game and saves were kept.')
 if shutil.disk_usage(root).free<spec['extracted_bytes']+64*1024*1024:raise ValueError('Not enough free SD-card space. Allow at least 1 GB free for the first installation.')
 staging=pathlib.Path(tempfile.mkdtemp(prefix='installation-',dir=str(root)))
 try:
  print('Preparing owned Linux game files. First launch can take a few minutes...',flush=True)
  needed={'CLARC/lib/libGLU.so.1.3.1':'libs/libGLU.so.1','CLARC/lib/libstdc++.so.6.0.19':'libs/libstdc++.so.6'}
  # Sequential gzip extraction; game assets never accumulate in RAM.
  with tarfile.open(archive,'r|gz') as tar:
   for member in tar:
    path=pathlib.PurePosixPath(member.name)
    if path.is_absolute() or '..' in path.parts:raise ValueError('Unsafe archive path')
    if not member.isfile():continue
    name=path.as_posix()
    if name.startswith('CLARC/game/'):dest=staging/'game'/name[len('CLARC/game/'):]
    elif name in needed:dest=staging/needed[name]
    else:continue
    dest.parent.mkdir(parents=True,exist_ok=True)
    with tar.extractfile(member) as src,dest.open('wb') as out:shutil.copyfileobj(src,out,1024*1024)
  for name,size in spec['original_game_sizes'].items():
   p=staging/name
   if not p.is_file() or p.stat().st_size!=size:raise ValueError('The archive is incomplete or is not the supported Linux release')
  asm=staging/'game'/managed/'Assembly-CSharp.dll'
  asm.write_bytes(apply_delta(asm.read_bytes(),(rt/'patches/assembly.delta').read_bytes()))
  shutil.copyfile(rt/'patches/ClarcRGDS.dll',staging/'game'/managed/'ClarcRGDS.dll')
  shutil.copyfile(rt/'patches/ScreenSelector.so',staging/'game/CLARC_LINUX_Data/Plugins/x86/ScreenSelector.so')
  for dest,h in required.items():
   candidate=staging/('libs/'+pathlib.Path(dest).name) if dest.startswith('runtime/') else staging/dest
   if not valid_file(candidate,h):raise ValueError('Game import verification failed: '+dest)
  # Validate everything before committing; saves and existing user preferences stay in place.
  for p in (staging/'game').rglob('*'):
   if p.is_file():
    destination=root/p.relative_to(staging);destination.parent.mkdir(parents=True,exist_ok=True);os.replace(p,destination)
  for p in (staging/'libs').iterdir():os.replace(p,rt/'libs.x86'/p.name)
  os.chmod(root/'game/CLARC_LINUX.x86',0o755)
  print('Game preparation completed. Starting CLARC.',flush=True)
 finally:shutil.rmtree(staging)

if __name__=='__main__':
 try:prepare(sys.argv[1] if len(sys.argv)>1 else ROOT)
 except (OSError,ValueError,tarfile.TarError,KeyError) as e:print('CLARC setup: '+str(e),flush=True);sys.exit(1)
