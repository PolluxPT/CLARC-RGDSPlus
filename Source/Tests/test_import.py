import importlib.util,json,pathlib,shutil,sys,tarfile,tempfile,unittest,io
repo=pathlib.Path(__file__).resolve().parents[2]
runtime=repo/'APPS/CLARC_RGDSPlus/runtime'
spec=importlib.util.spec_from_file_location('clarc_installer',runtime/'install-game.py');installer=importlib.util.module_from_spec(spec);spec.loader.exec_module(installer)
class ImportChecks(unittest.TestCase):
 def test_delta_requires_supported_original(self):
  with self.assertRaises(ValueError):installer.apply_delta(b'wrong game', (runtime/'patches/assembly.delta').read_bytes())
 def test_bad_delta_header(self):
  with self.assertRaises(ValueError):installer.apply_delta(b'abc',b'not a patch')
 def root(self,tmp):
  root=pathlib.Path(tmp);shutil.copytree(runtime,root/'runtime');(root/'savedata').mkdir();(root/'savedata/save-marker').write_text('keep');return root
 def test_missing_archive_retains_saves(self):
  with tempfile.TemporaryDirectory() as tmp:
   root=self.root(tmp)
   with self.assertRaisesRegex(ValueError,'Copy your own'):installer.prepare(root)
   self.assertEqual((root/'savedata/save-marker').read_text(),'keep')
 def test_unsafe_archive_retains_existing_files(self):
  with tempfile.TemporaryDirectory() as tmp:
   root=self.root(tmp);(root/'game').mkdir();(root/'game/existing').write_text('keep')
   with tarfile.open(root/'clarc_1.0.0.3.tar.gz','w:gz') as t:
    m=tarfile.TarInfo('CLARC/game/../../outside');m.size=1;t.addfile(m,io.BytesIO(b'x'))
   with self.assertRaisesRegex(ValueError,'Unsafe archive'):installer.prepare(root)
   self.assertEqual((root/'game/existing').read_text(),'keep');self.assertFalse((root/'outside').exists());self.assertFalse(list(root.glob('installation-*')))
 def test_incomplete_archive_retains_existing_files(self):
  with tempfile.TemporaryDirectory() as tmp:
   root=self.root(tmp)
   with tarfile.open(root/'clarc_1.0.0.3.tar.gz','w:gz'):pass
   with self.assertRaisesRegex(ValueError,'incomplete'):installer.prepare(root)
   self.assertEqual((root/'savedata/save-marker').read_text(),'keep');self.assertFalse(list(root.glob('installation-*')))
if __name__=='__main__':unittest.main()
