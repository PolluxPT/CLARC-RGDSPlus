#!/usr/bin/env python3
"""Build a source-dependent copy/add delta; no third-party Python modules."""
import base64,bisect,hashlib,json,pathlib,sys,zlib

def generate(old,new):
 index={}
 for i in range(len(old)-15):index.setdefault(old[i:i+16],[]).append(i)
 operations=[];literal=bytearray();pos=0
 def flush():
  if not literal:return
  offset=min(pos-len(literal),len(old)-len(literal))
  if offset<0:raise ValueError('Literal span exceeds original assembly')
  diff=bytes((b-old[offset+i])&255 for i,b in enumerate(literal))
  operations.append(['add',offset,base64.b64encode(diff).decode()]);literal.clear()
 while pos<len(new):
  best=0;offset=0;positions=index.get(new[pos:pos+16],[]) if pos+16<=len(new) else []
  if positions:
   k=bisect.bisect_left(positions,pos)
   for p in set(positions[max(0,k-2):k+3]+positions[:2]+positions[-2:]):
    n=16;limit=min(len(old)-p,len(new)-pos)
    while n+1024<=limit and old[p+n:p+n+1024]==new[pos+n:pos+n+1024]:n+=1024
    while n<limit and old[p+n]==new[pos+n]:n+=1
    if n>best:best=n;offset=p
  if best>=16:flush();operations.append(['copy',offset,best]);pos+=best
  else:literal.append(new[pos]);pos+=1
 flush()
 meta={'format':'CLARC-copy-add-delta-1','source_sha256':hashlib.sha256(old).hexdigest(),'target_sha256':hashlib.sha256(new).hexdigest(),'target_size':len(new),'operations':operations}
 return b'CLARCP1\n'+zlib.compress(json.dumps(meta,separators=(',',':')).encode(),9)
if __name__=='__main__':
 pathlib.Path(sys.argv[3]).write_bytes(generate(pathlib.Path(sys.argv[1]).read_bytes(),pathlib.Path(sys.argv[2]).read_bytes()))
