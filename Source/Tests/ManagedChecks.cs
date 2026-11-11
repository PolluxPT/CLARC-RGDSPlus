using System.Reflection;using Mono.Cecil;using Mono.Cecil.Cil;
var h=Assembly.LoadFrom(Path.GetFullPath(args[0]));var t=h.GetType("ClarcRGDS")!;var axis=t.GetMethod("AxisValue")!;int tests=0;
void Check(float original,int a,int p,uint b,float expected){float actual=(float)axis.Invoke(null,new object[]{original,a,p,b})!;if(actual!=expected)throw new Exception($"axis {a}: {actual} != {expected}");tests++;}
for(int mask=0;mask<64;mask++){uint b=(uint)mask<<20;Check(.37f,0,1,b,(mask&3)==0?.37f:((mask&2)!=0?1:0)-((mask&1)!=0?1:0));Check(-.43f,1,1,b,(mask&12)==0?-.43f:((mask&4)!=0?1:0)-((mask&8)!=0?1:0));Check(.2f,2,1,b,(mask&48)==0?.2f:((mask&32)!=0?1:0)-((mask&16)!=0?1:0));Check(.7f,0,2,b,.7f);Check(.8f,6,1,b,0);Check(.8f,7,1,b,0);}
var button=t.GetMethod("ButtonValue")!;
void ButtonCheck(bool original,int code,int player,int action,uint value,uint prior,bool expected){bool actual=(bool)button.Invoke(null,new object[]{original,code,player,action,value,prior})!;if(actual!=expected)throw new Exception($"Button {code} action {action}: {actual} != {expected}");tests++;}
foreach(var (logical,bit) in new[]{(0,16),(1,17),(2,18),(3,19),(10,22),(11,23),(12,20),(13,21),(14,27),(15,26)}){
 uint b=1u<<bit;
 ButtonCheck(false,logical,1,0,b,0,true);ButtonCheck(false,logical,1,1,b,0,true);ButtonCheck(true,logical,1,2,b,0,false);
 ButtonCheck(false,logical,1,0,b,b,true);ButtonCheck(true,logical,1,1,b,b,false);
 ButtonCheck(true,logical,1,0,0,b,false);ButtonCheck(false,logical,1,2,0,b,true);ButtonCheck(true,logical,1,2,0,0,false);
 ButtonCheck(true,logical,2,0,0,0,true);
}
foreach(int logical in new[]{4,5,8,9})foreach(int action in new[]{0,1,2})ButtonCheck(true,logical,1,action,uint.MaxValue,0,false);
for(int mask=0;mask<16;mask++){
 uint b=0;for(int bit=0;bit<4;bit++)if((mask&(1<<bit))!=0)b|=1u<<(24+bit);
 // End-to-end packets produced by the actual Python mapper.
 using var packets=System.Text.Json.JsonDocument.Parse(File.ReadAllText(args[2]));uint packet=packets.RootElement[mask].GetProperty("bits").GetUInt32();
 Check(0,2,1,packet,((mask&2)!=0?1:0)-((mask&1)!=0?1:0));
 ButtonCheck(true,14,1,0,packet,0,(mask&8)!=0);ButtonCheck(true,15,1,0,packet,0,(mask&4)!=0);
 Check(1,4,1,packet,0);Check(1,5,1,packet,0);
 ButtonCheck(true,0,1,1,packet,0,false);ButtonCheck(true,2,1,1,packet,0,false);
}
// Target Unity 4/.NET 2 references only; no accidental .NET 8 game dependency.
using var helper=AssemblyDefinition.ReadAssembly(args[0]);if(helper.MainModule.AssemblyReferences.Any(r=>r.Name=="System.Runtime"))throw new Exception("Wrong game framework");
using var game=AssemblyDefinition.ReadAssembly(args[1]);var cf=game.MainModule.GetType("ControlFreak");foreach(var name in new[]{"GetAxis","GetButton","GetButtonDown","GetButtonUp"}){var m=cf.Methods.Single(m=>m.Name==name);foreach(var i in m.Body.Instructions.Where(i=>i.OpCode==OpCodes.Ret)){if(i.Previous.Operand is not MethodReference hook||hook.DeclaringType.Name!="ClarcRGDS")throw new Exception("Unhooked return "+name);tests++;}}
Console.WriteLine($"PASS {tests} managed mapping/IL checks");
