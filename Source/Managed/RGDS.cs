using System;
using System.IO;
using UnityEngine;
public static class ClarcRGDS {
 static bool initialized;static float gameVolume=1f;static float gain=.014f;static string node,exit;
 // One unbuffered four-byte mailbox read per Unity frame. No per-frame allocation.
 static FileStream controls;static byte[] packet=new byte[4];static int frame=-1;static uint bits,previous;
 static uint ReadControls(){
  int current=Time.frameCount;if(frame==current)return bits;frame=current;previous=bits;bits=0;
  try{if(controls==null){string path=Environment.GetEnvironmentVariable("CLARC_INPUT_STATE");if(path==null)return 0;controls=new FileStream(path,FileMode.Open,FileAccess.Read,FileShare.ReadWrite,1);}
   controls.Position=0;if(controls.Read(packet,0,4)==4)bits=(uint)(packet[0]|packet[1]<<8|packet[2]<<16|packet[3]<<24);
  }catch(IOException){if(controls!=null)controls.Close();controls=null;}
  return bits;
 }
 public static float AxisValue(float original,int axis,int player,uint value){
  if(player!=1)return original;
  if(axis==0||axis==1){uint mask=axis==0?((1u<<20)|(1u<<21)):((1u<<22)|(1u<<23));
   if((value&mask)!=0){if(axis==0)return ((value&(1u<<21))!=0?1f:0f)-((value&(1u<<20))!=0?1f:0f);
    return ((value&(1u<<22))!=0?1f:0f)-((value&(1u<<23))!=0?1f:0f);}
  }
  if(axis==2&&(value&((1u<<24)|(1u<<25)))!=0)return ((value&(1u<<25))!=0?1f:0f)-((value&(1u<<24))!=0?1f:0f);
  // Movement must take the same left-stick path (including its angle correction).
  if(axis==4||axis==5||axis==6||axis==7)return 0f;
  return original;
 }
 public static float FilterAxis(float original,int axis,int player){return AxisValue(original,axis,player,ReadControls());}
 public static bool ButtonValue(bool original,int button,int player,int action,uint value,uint prior){
  if(player!=1)return original;
  // Shoulders are reserved for right-stick rotation; triggers have no assigned action.
  if(button==4||button==5||button==8||button==9)return false;
  int bit=-1;
  if(button>=0&&button<=3)bit=16+button;
  else if(button>=10&&button<=13)bit=button==10?22:button==11?23:button==12?20:21;
  else if(button==14)bit=27;
  else if(button==15)bit=26;
  if(bit<0)return original;
  uint mask=1u<<bit;
  return action==1?(value&mask)!=0&&(prior&mask)==0:action==2?(value&mask)==0&&(prior&mask)!=0:(value&mask)!=0;
 }
 public static bool FilterButton(bool original,int button,int player,int action){uint value=ReadControls();return ButtonValue(original,button,player,action,value,previous);}
 // Unity's Linux joystick numbers: generic buttons start at 330; numbered devices at 350.
 public static bool IsShoulder(KeyCode key){int k=(int)key;return k==334||k==335||(k>=350&&k<550&&((k-350)%20==4||(k-350)%20==5));}
 public static bool GetKey(KeyCode key){return !IsShoulder(key)&&Input.GetKey(key);}
 public static bool GetKeyDown(KeyCode key){return !IsShoulder(key)&&Input.GetKeyDown(key);}
 public static bool GetKeyUp(KeyCode key){return !IsShoulder(key)&&Input.GetKeyUp(key);}
 public static void Init(){if(initialized)return;initialized=true;node=Environment.GetEnvironmentVariable("CLARC_VOLUME_NODE");exit=Environment.GetEnvironmentVariable("CLARC_EXIT_REQUEST");Refresh();QualitySettings.SetQualityLevel(0);Application.targetFrameRate=30;var o=new GameObject("CLARC RGDS controls");UnityEngine.Object.DontDestroyOnLoad(o);o.AddComponent<ClarcRGDSUpdate>();}
 public static void SetVolume(float value){gameVolume=Mathf.Clamp01(value);AudioListener.volume=gameVolume*gain;}
 public static void Refresh(){if(node!=null)try{int level=int.Parse(File.ReadAllText(node).Trim());level=Math.Max(0,Math.Min(10,level));float f=level/10f;gain=.35f*f*f;}catch(Exception){}AudioListener.volume=gameVolume*gain;if(exit!=null&&File.Exists(exit))Application.Quit();}
}
public class ClarcRGDSUpdate:MonoBehaviour {float next;void Update(){if(Time.realtimeSinceStartup>=next){next=Time.realtimeSinceStartup+.2f;ClarcRGDS.Refresh();}}}
