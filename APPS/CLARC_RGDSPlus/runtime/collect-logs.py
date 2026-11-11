import pathlib,sys,zipfile
root=pathlib.Path(sys.argv[1]);logs=root/'logs'
with zipfile.ZipFile(root/'CLARC_TestLogs.zip','w',zipfile.ZIP_DEFLATED) as z:
 for p in logs.glob('*.txt'):
  if p.is_file():
   if p.stat().st_size<=2*1024*1024:z.write(p,'logs/'+p.name)
   else:
    with p.open('rb') as f:f.seek(-2*1024*1024,2);z.writestr('logs/'+p.name,f.read())
