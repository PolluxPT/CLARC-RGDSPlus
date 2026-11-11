using Mono.Cecil;using Mono.Cecil.Cil;
var resolver=new DefaultAssemblyResolver();resolver.AddSearchDirectory(args[0]);using var a=AssemblyDefinition.ReadAssembly(Path.Combine(args[0],"Assembly-CSharp.dll"),new ReaderParameters{AssemblyResolver=resolver});using var h=AssemblyDefinition.ReadAssembly(args[1]);var helper=h.MainModule.GetType("ClarcRGDS");MethodReference Hook(string name)=>a.MainModule.ImportReference(helper.Methods.Single(m=>m.Name==name));
int axes=0,buttons=0,keys=0;
foreach(var t in a.MainModule.Types)foreach(var m in t.Methods.Where(m=>m.HasBody)){
 foreach(var i in m.Body.Instructions)if(i.Operand is MethodReference mr&&mr.DeclaringType.FullName=="UnityEngine.Input"&&new[]{"GetKey","GetKeyDown","GetKeyUp"}.Contains(mr.Name)&&mr.Parameters[0].ParameterType.FullName=="UnityEngine.KeyCode"){i.Operand=Hook(mr.Name);keys++;}
 if(t.Name!="ControlFreak")continue;
 bool axis=m.Name=="GetAxis";int action=m.Name=="GetButton"?0:m.Name=="GetButtonDown"?1:m.Name=="GetButtonUp"?2:-1;
 if(!axis&&action<0)continue;
 foreach(var ret in m.Body.Instructions.Where(i=>i.OpCode==OpCodes.Ret).ToArray()){
  var il=m.Body.GetILProcessor();ret.OpCode=OpCodes.Ldarg_0;var cursor=ret;
  void Add(Instruction i){il.InsertAfter(cursor,i);cursor=i;}
  Add(il.Create(OpCodes.Ldarg_1));if(!axis)Add(il.Create(OpCodes.Ldc_I4,action));Add(il.Create(OpCodes.Call,Hook(axis?"FilterAxis":"FilterButton")));Add(il.Create(OpCodes.Ret));if(axis)axes++;else buttons++;
 }
 m.Body.MaxStackSize+=4;
}
if(axes!=2||buttons!=6||keys==0)throw new Exception($"Unexpected hook counts {axes}/{buttons}/{keys}");a.Write(args[2]);Console.WriteLine($"Axis returns {axes}; button returns {buttons}; raw key calls {keys}");
