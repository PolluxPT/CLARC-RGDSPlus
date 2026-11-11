using Mono.Cecil;using Mono.Cecil.Cil;
var resolver=new DefaultAssemblyResolver();resolver.AddSearchDirectory(args[0]);using var a=AssemblyDefinition.ReadAssembly(Path.Combine(args[0],"Assembly-CSharp.dll"),new ReaderParameters{AssemblyResolver=resolver});using var hook=AssemblyDefinition.ReadAssembly(args[1]);var helper=hook.MainModule.GetType("ClarcRGDS");int awake=0,audio=0;
foreach(var t in a.MainModule.Types)foreach(var m in t.Methods.Where(m=>m.HasBody)){
 if(m.Name=="Awake"||m.Name=="Start"||m.Name=="OnEnable"){var il=m.Body.GetILProcessor();il.InsertBefore(m.Body.Instructions[0],il.Create(OpCodes.Call,a.MainModule.ImportReference(helper.Methods.Single(x=>x.Name=="Init"))));awake++;}
 foreach(var i in m.Body.Instructions)if(i.Operand is MethodReference mr&&mr.DeclaringType.FullName=="UnityEngine.AudioListener"&&mr.Name=="set_volume"){i.Operand=a.MainModule.ImportReference(helper.Methods.Single(x=>x.Name=="SetVolume"));audio++;}
}
a.Write(args[2]);Console.WriteLine($"CLARC hooks: {awake} initialization points, {audio} volume setters.");if(awake==0)throw new Exception("No startup callbacks found");
