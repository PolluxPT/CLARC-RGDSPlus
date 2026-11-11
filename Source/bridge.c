/* CLARC RGDS+ private X11/GLX-to-SDL Wayland bridge. MIT integration. */
#define XLIB_ILLEGAL_ACCESS
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
extern int SDL_Init(unsigned);extern void SDL_Quit(void);extern void *SDL_CreateWindow(const char*,int,int,int,int,unsigned);extern void SDL_DestroyWindow(void*);extern void SDL_ShowWindow(void*);extern int SDL_GL_SetAttribute(int,int);extern void *SDL_GL_CreateContext(void*);extern int SDL_GL_MakeCurrent(void*,void*);extern void SDL_GL_DeleteContext(void*);extern void SDL_GL_SwapWindow(void*);extern int SDL_GL_SetSwapInterval(int);extern int SDL_PollEvent(void*);extern void SDL_Delay(unsigned);extern void SDL_GL_GetDrawableSize(void*,int*,int*);extern unsigned SDL_GetTicks(void);extern const char*SDL_GetError(void);
static unsigned context_refs;static Display display;static Screen screen;static Visual visual;static Depth depth;static void*window,*context;static int width=640,height=360,mousex=320,mousey=180;static unsigned buttons;static XEvent events[256];static unsigned rd,wr;static Window current=2;static unsigned long nextid=3;static char *atoms[256];static int natoms=1;static volatile uint32_t *input;static uint32_t prior;static unsigned long selected;
static void queue(XEvent*e){if(wr-rd<256){e->xany.display=&display;e->xany.window=current;e->xany.serial=wr+1;e->xany.send_event=False;events[wr++%256]=*e;}}
static KeySym keysyms[]={0,0xff51,0xff53,0xff52,0xff54,0xff0d,0x20,0x65,0x71,0xff1b,0x72,0x77,0x61,0x73,0x64};
static void key(int n,int down){if(n<=0||n>=15)return;XEvent e={0};e.xkey.type=down?KeyPress:KeyRelease;e.xkey.root=1;e.xkey.same_screen=True;e.xkey.keycode=n+8;e.xkey.time=SDL_GetTicks();queue(&e);}
static int fromsym(int s){if(s==13)return 5;if(s==27)return 9;for(int i=1;i<15;i++)if(keysyms[i]==(KeySym)s)return i;if(s==1073741904)return 1;if(s==1073741903)return 2;if(s==1073741906)return 3;if(s==1073741905)return 4;return 0;}
static void pump(void){unsigned char e[56];while(SDL_PollEvent(e)){uint32_t type;memcpy(&type,e,4);if(type==0x100){XEvent x={0};x.xclient.type=ClientMessage;x.xclient.format=32;x.xclient.message_type=0;for(int i=1;i<natoms;i++)if(!strcmp(atoms[i],"WM_DELETE_WINDOW"))x.xclient.data.l[0]=i;queue(&x);}else if(type==0x300||type==0x301){int s;memcpy(&s,e+20,4);key(fromsym(s),type==0x300);}else if(type==0x400){memcpy(&mousex,e+20,4);memcpy(&mousey,e+24,4);XEvent x={0};x.xmotion.type=MotionNotify;x.xmotion.x=mousex;x.xmotion.y=mousey;x.xmotion.same_screen=True;queue(&x);}else if(type==0x401||type==0x402){int b=e[16];if(b<=3){if(type==0x401)buttons|=1u<<b;else buttons&=~(1u<<b);XEvent x={0};x.xbutton.type=type==0x401?ButtonPress:ButtonRelease;x.xbutton.button=b;x.xbutton.x=mousex;x.xbutton.y=mousey;x.xbutton.same_screen=True;queue(&x);}}}
 if(input){uint32_t bits=*input,change=bits^prior;for(int i=1;i<15;i++)if(change&(1u<<i))key(i,(bits>>i)&1);prior=bits;}}
Display*XOpenDisplay(const char*n){(void)n;if(display.screens)return &display;if(SDL_Init(0x4020)!=0){fprintf(stderr,"CLARC SDL init: %s\n",SDL_GetError());return NULL;}visual.visualid=1;visual.class=TrueColor;visual.red_mask=0xff0000;visual.green_mask=0xff00;visual.blue_mask=0xff;visual.bits_per_rgb=8;visual.map_entries=256;depth.depth=24;depth.nvisuals=1;depth.visuals=&visual;screen.display=&display;screen.root=1;screen.width=width;screen.height=height;screen.mwidth=160;screen.mheight=90;screen.ndepths=1;screen.depths=&depth;screen.root_depth=24;screen.root_visual=&visual;screen.default_gc=(GC)1;screen.cmap=1;screen.white_pixel=0xffffff;screen.black_pixel=0;display.fd=-1;display.proto_major_version=11;display.vendor="CLARC RGDS Wayland";display.default_screen=0;display.nscreens=1;display.screens=&screen;display.display_name="CLARC-Wayland";display.bitmap_unit=32;display.bitmap_pad=32;display.byte_order=LSBFirst;display.bitmap_bit_order=LSBFirst;display.max_request_size=65535;
 const char*p=getenv("CLARC_INPUT_STATE");if(p){int fd=open(p,O_RDONLY);if(fd>=0){void*m=mmap(NULL,4096,PROT_READ,MAP_SHARED,fd,0);if(m!=MAP_FAILED)input=m;close(fd);}}fprintf(stderr,"CLARC bridge: X11 metadata on native SDL/Wayland\n");return &display;}
int XCloseDisplay(Display*d){(void)d;if(context)SDL_GL_DeleteContext(context);if(window)SDL_DestroyWindow(window);context=window=NULL;context_refs=0;SDL_Quit();return 0;}
Window XCreateWindow(Display*d,Window p,int x,int y,unsigned w,unsigned h,unsigned b,int dep,unsigned c,Visual*v,unsigned long m,XSetWindowAttributes*a){(void)d;(void)p;(void)x;(void)y;(void)w;(void)h;(void)b;(void)dep;(void)c;(void)v;(void)m;(void)a;return current;}
Window XCreateSimpleWindow(Display*d,Window p,int x,int y,unsigned w,unsigned h,unsigned b,unsigned long border,unsigned long bg){return XCreateWindow(d,p,x,y,w,h,b,24,InputOutput,&visual,0,NULL);}
int XMapWindow(Display*d,Window w){(void)d;current=w;XEvent e={0};e.xconfigure.type=ConfigureNotify;e.xconfigure.width=width;e.xconfigure.height=height;queue(&e);e.type=Expose;queue(&e);e.type=FocusIn;queue(&e);return 1;}
int XDestroyWindow(Display*d,Window w){(void)d;(void)w;return 1;}
int XGetWindowAttributes(Display*d,Window w,XWindowAttributes*a){(void)d;memset(a,0,sizeof(*a));a->width=width;a->height=height;a->depth=24;a->visual=&visual;a->root=1;a->class=InputOutput;a->map_state=IsViewable;a->screen=&screen;a->all_event_masks=a->your_event_mask=selected;a->colormap=1;return 1;}
Status XGetGeometry(Display*d,Drawable w,Window*r,int*x,int*y,unsigned*ww,unsigned*hh,unsigned*b,unsigned*dep){(void)d;(void)w;if(r)*r=1;if(x)*x=0;if(y)*y=0;if(ww)*ww=width;if(hh)*hh=height;if(b)*b=0;if(dep)*dep=24;return 1;}
int XPending(Display*d){(void)d;pump();return wr-rd;}int XEventsQueued(Display*d,int mode){(void)mode;return XPending(d);}
int XNextEvent(Display*d,XEvent*e){(void)d;while(wr==rd){pump();if(wr==rd)SDL_Delay(1);}*e=events[rd++%256];return 0;}
static long eventmask(int t){switch(t){case KeyPress:return KeyPressMask;case KeyRelease:return KeyReleaseMask;case ButtonPress:return ButtonPressMask;case ButtonRelease:return ButtonReleaseMask;case MotionNotify:return PointerMotionMask;case Expose:return ExposureMask;case ConfigureNotify:return StructureNotifyMask;case FocusIn:case FocusOut:return FocusChangeMask;default:return 0;}}
Bool XCheckWindowEvent(Display*d,Window w,long mask,XEvent*e){XPending(d);for(unsigned i=rd;i<wr;i++)if(events[i%256].xany.window==w&&(eventmask(events[i%256].type)&mask)){*e=events[i%256];for(unsigned j=i;j+1<wr;j++)events[j%256]=events[(j+1)%256];wr--;return True;}return False;}
Bool XCheckTypedWindowEvent(Display*d,Window w,int type,XEvent*e){(void)w;XPending(d);for(unsigned i=rd;i<wr;i++)if(events[i%256].type==type){*e=events[i%256];for(unsigned j=i;j+1<wr;j++)events[j%256]=events[(j+1)%256];wr--;return True;}return False;}
Status XSendEvent(Display*d,Window w,Bool prop,long mask,XEvent*e){(void)d;(void)w;(void)prop;(void)mask;queue(e);return 1;}
int XLookupString(XKeyEvent*e,char*b,int size,KeySym*s,XComposeStatus*c){(void)c;int k=(int)e->keycode-8;KeySym sym=(k>0&&k<15)?keysyms[k]:0;if(s)*s=sym;if(sym>=32&&sym<127&&size>0){b[0]=(char)sym;return 1;}return 0;}
KeySym XKeycodeToKeysym(Display*d,KeyCode k,int i){(void)d;(void)i;return k>=9&&k<23?keysyms[k-8]:0;}KeyCode XKeysymToKeycode(Display*d,KeySym s){(void)d;int k=fromsym((int)s);return k?k+8:0;}
Bool XQueryPointer(Display*d,Window w,Window*r,Window*c,int*rx,int*ry,int*x,int*y,unsigned*m){(void)d;(void)w;if(r)*r=1;if(c)*c=current;if(rx)*rx=mousex;if(ry)*ry=mousey;if(x)*x=mousex;if(y)*y=mousey;if(m)*m=(buttons&2?Button1Mask:0)|(buttons&4?Button2Mask:0)|(buttons&8?Button3Mask:0);return True;}
Atom XInternAtom(Display*d,const char*n,Bool only){(void)d;for(int i=1;i<natoms;i++)if(!strcmp(atoms[i],n))return i;if(only||natoms>=256)return None;atoms[natoms]=strdup(n);return natoms++;}
Status XGetWindowProperty(Display*d,Window w,Atom p,long off,long len,Bool del,Atom req,Atom*actual,int*format,unsigned long*count,unsigned long*left,unsigned char**data){(void)d;(void)w;(void)p;(void)off;(void)len;(void)del;(void)req;*actual=None;*format=0;*count=*left=0;*data=NULL;return Success;}
int XChangeProperty(Display*d,Window w,Atom p,Atom t,int format,int mode,const unsigned char*data,int n){(void)d;(void)w;(void)p;(void)t;(void)format;(void)mode;(void)data;(void)n;return Success;}
Colormap XCreateColormap(Display*d,Window w,Visual*v,int a){(void)d;(void)w;(void)v;(void)a;return 1;}
XSizeHints*XAllocSizeHints(void){return calloc(1,sizeof(XSizeHints));}XWMHints*XAllocWMHints(void){return calloc(1,sizeof(XWMHints));}XClassHint*XAllocClassHint(void){return calloc(1,sizeof(XClassHint));}
int XFree(void*p){free(p);return 0;}int XSelectInput(Display*d,Window w,long m){(void)d;(void)w;selected=m;return 1;}
Bool XTranslateCoordinates(Display*d,Window s,Window dest,int x,int y,int*xx,int*yy,Window*c){(void)d;(void)s;(void)dest;*xx=x;*yy=y;*c=None;return True;}
int XStoreName(Display*d,Window w,const char*n){(void)d;(void)w;(void)n;return 1;}
Status XQueryTree(Display*d,Window w,Window*r,Window*p,Window**children,unsigned*n){(void)d;(void)w;*r=1;*p=1;*children=NULL;*n=0;return 1;}
int XGetInputFocus(Display*d,Window*w,int*r){(void)d;*w=current;*r=RevertToParent;return 1;}
int XGrabPointer(Display*d,Window w,Bool owner,unsigned mask,int pm,int km,Window conf,Cursor cur,Time t){(void)d;(void)w;(void)owner;(void)mask;(void)pm;(void)km;(void)conf;(void)cur;(void)t;return GrabSuccess;}
int XWarpPointer(Display*d,Window src,Window dest,int sx,int sy,unsigned sw,unsigned sh,int dx,int dy){(void)d;(void)src;(void)dest;(void)sx;(void)sy;(void)sw;(void)sh;mousex=dx;mousey=dy;return 1;}
int XGetErrorText(Display*d,int code,char*b,int n){(void)d;(void)code;if(n)snprintf(b,n,"CLARC Wayland bridge");return 0;}
XErrorHandler XSetErrorHandler(XErrorHandler h){static XErrorHandler old;XErrorHandler prior=old;old=h;return prior;}
Status XSetWMProtocols(Display*d,Window w,Atom*a,int n){(void)d;(void)w;(void)a;(void)n;return 1;}
void XSetWMProperties(Display*d,Window w,XTextProperty*a,XTextProperty*b,char**argv,int argc,XSizeHints*s,XWMHints*h,XClassHint*c){(void)d;(void)w;(void)a;(void)b;(void)argv;(void)argc;(void)s;(void)h;(void)c;}
Status XStringListToTextProperty(char**s,int n,XTextProperty*p){p->value=(unsigned char*)strdup(n?s[0]:"");p->encoding=XA_STRING;p->format=8;p->nitems=strlen((char*)p->value);return 1;}
int XSetClassHint(Display*d,Window w,XClassHint*h){(void)d;(void)w;(void)h;return 1;}
Window XGetSelectionOwner(Display*d,Atom a){(void)d;(void)a;return None;}
int XSetSelectionOwner(Display*d,Atom a,Window w,Time t){(void)d;(void)a;(void)w;(void)t;return 1;}
int XConvertSelection(Display*d,Atom a,Atom b,Atom c,Window w,Time t){(void)d;(void)a;(void)b;(void)c;(void)w;(void)t;return 1;}
int XInitThreads(void){return 1;}int XFlush(Display*d){(void)d;return 1;}int XSync(Display*d,Bool discard){(void)d;if(discard)rd=wr;return 1;}
Bool XQueryExtension(Display*d,const char*n,int*o,int*e,int*error){(void)d;(void)n;*o=*e=*error=0;return False;}
void XLockDisplay(Display*d){(void)d;}void XUnlockDisplay(Display*d){(void)d;}
Bool XkbSetDetectableAutoRepeat(Display*d,Bool repeat,Bool*s){(void)d;(void)repeat;if(s)*s=True;return True;}
/* A disabled ARB extension must report an error if Unity probes it anyway.
 * Do not enter GL4ES's assembly translator and do not report fake success. */
static unsigned bridge_gl_error;
void glProgramStringARB(unsigned target,unsigned format,int length,const void*source){
 static int disabled=-1;static void(*real)(unsigned,unsigned,int,const void*);
 if(disabled<0){const char*value=getenv("LIBGL_NOARBPROGRAM");disabled=value&&strcmp(value,"0");}
 if(disabled){bridge_gl_error=0x502;return;}
 if(!real)real=dlsym(RTLD_NEXT,"glProgramStringARB");
 if(real){static unsigned logged;if(logged++<8)fprintf(stderr,"CLARC ARB: target=%x format=%x bytes=%d context_refs=%u\n",target,format,length,context_refs);real(target,format,length,source);}else bridge_gl_error=0x502;
}
unsigned glGetError(void){static unsigned(*real)(void);if(bridge_gl_error){unsigned error=bridge_gl_error;bridge_gl_error=0;return error;}if(!real)real=dlsym(RTLD_NEXT,"glGetError");return real?real():0;}
static void scale_rect(int*x,int*y,int*w,int*h){if(!window)return;int fbo=0,ww=width,hh=height;static void(*get)(unsigned,int*);if(!get)get=dlsym(RTLD_NEXT,"glGetIntegerv");if(get)get(0x8ca6,&fbo);if(fbo)return;SDL_GL_GetDrawableSize(window,&ww,&hh);if(ww>0&&hh>0){*x=*x*ww/width;*y=*y*hh/height;*w=*w*ww/width;*h=*h*hh/height;}}
void glViewport(int x,int y,int w,int h){static void(*real)(int,int,int,int);if(!real)real=dlsym(RTLD_NEXT,"glViewport");scale_rect(&x,&y,&w,&h);if(real)real(x,y,w,h);}
void glScissor(int x,int y,int w,int h){static void(*real)(int,int,int,int);if(!real)real=dlsym(RTLD_NEXT,"glScissor");scale_rect(&x,&y,&w,&h);if(real)real(x,y,w,h);}
/* GLX is intentionally not sent to a nonexistent X server. */
int glXQueryVersion(void*d,int*major,int*minor){(void)d;*major=1;*minor=3;return 1;}
const char*glXQueryExtensionsString(void*d,int s){(void)d;(void)s;return "GLX_ARB_get_proc_address GLX_EXT_swap_control";}
void**glXChooseFBConfig(void*d,int screen,const int*attrs,int*n){(void)d;(void)screen;(void)attrs;void**r=malloc(sizeof(void*));r[0]=(void*)1;*n=1;return r;}
XVisualInfo*glXGetVisualFromFBConfig(void*d,void*c){(void)d;(void)c;XVisualInfo*v=calloc(1,sizeof(*v));v->visual=&visual;v->visualid=1;v->screen=0;v->depth=24;v->class=TrueColor;v->red_mask=visual.red_mask;v->green_mask=visual.green_mask;v->blue_mask=visual.blue_mask;v->colormap_size=256;v->bits_per_rgb=8;return v;}
int glXGetFBConfigAttrib(void*d,void*c,int a,int*v){(void)d;(void)c;switch(a){case 5:case 0x8010:case 0x8011:case 0x8012:case 0x8013:case 0x800b:*v=1;break;case 0x22:*v=0x8002;break;case 0x20:*v=0x8000;break;case 2:*v=32;break;case 12:*v=24;break;case 13:*v=8;break;case 8:case 9:case 10:case 11:*v=8;break;default:*v=0;}return 0;}
void*glXCreateNewContext(void*d,void*c,int type,void*share,int direct){(void)d;(void)c;(void)type;(void)share;(void)direct;if(context){context_refs++;fprintf(stderr,"CLARC GLX: reused context refs=%u\n",context_refs);return context;}SDL_GL_SetAttribute(17,2);SDL_GL_SetAttribute(18,0);SDL_GL_SetAttribute(21,4);SDL_GL_SetAttribute(5,1);SDL_GL_SetAttribute(6,24);SDL_GL_SetAttribute(7,8);window=SDL_CreateWindow("CLARC RGDS+",0,0,width,height,0x1007);if(!window){fprintf(stderr,"CLARC window: %s\n",SDL_GetError());return NULL;}context=SDL_GL_CreateContext(window);if(!context)fprintf(stderr,"CLARC context: %s\n",SDL_GetError());else{context_refs=1;SDL_ShowWindow(window);SDL_GL_SetSwapInterval(1);}return context;}
int glXMakeCurrent(void*d,unsigned long w,void*c){(void)d;(void)w;return SDL_GL_MakeCurrent(window,c)==0;}void*glXGetCurrentContext(void){return context;}unsigned long glXGetCurrentDrawable(void){return current;}
void glXSwapBuffers(void*d,unsigned long w){(void)d;(void)w;SDL_GL_SwapWindow(window);}void glXDestroyContext(void*d,void*c){(void)d;if(c&&c==context&&context_refs){context_refs--;fprintf(stderr,"CLARC GLX: released context refs=%u\n",context_refs);if(!context_refs){SDL_GL_DeleteContext(c);context=NULL;}}}void glXWaitGL(void){}
void*glXGetProcAddressARB(const unsigned char*n){static void*(*get)(const unsigned char*);static void*lib;if(!n)return NULL;
#define OWN(f) if(!strcmp((const char*)n,#f))return (void*)f
 OWN(glGetError);OWN(glProgramStringARB);OWN(glViewport);OWN(glScissor);OWN(glXChooseFBConfig);OWN(glXGetVisualFromFBConfig);OWN(glXGetFBConfigAttrib);OWN(glXCreateNewContext);OWN(glXMakeCurrent);OWN(glXSwapBuffers);OWN(glXDestroyContext);OWN(glXQueryVersion);OWN(glXQueryExtensionsString);OWN(glXGetCurrentContext);OWN(glXGetCurrentDrawable);OWN(glXWaitGL);OWN(glXGetProcAddressARB);
#undef OWN
 if(!get){lib=dlopen("libGL.so.1",RTLD_LAZY|RTLD_GLOBAL);if(lib)get=dlsym(lib,"glXGetProcAddressARB");if((void*)get==(void*)glXGetProcAddressARB)get=dlsym(RTLD_NEXT,"glXGetProcAddressARB");}return get?get(n):NULL;}
void*glXGetProcAddress(const unsigned char*n){return glXGetProcAddressARB(n);}
/* Resources Unity uses only for its desktop cursor/selector. */
Pixmap XCreateBitmapFromData(Display*d,Drawable w,const char*b,unsigned ww,unsigned hh){(void)d;(void)w;(void)b;(void)ww;(void)hh;return nextid++;}
Pixmap XCreatePixmap(Display*d,Drawable w,unsigned ww,unsigned hh,unsigned dep){(void)d;(void)w;(void)ww;(void)hh;(void)dep;return nextid++;}
GC XCreateGC(Display*d,Drawable w,unsigned long m,XGCValues*v){(void)d;(void)w;(void)m;(void)v;return calloc(1,sizeof(struct _XGC));}
Cursor XCreateFontCursor(Display*d,unsigned shape){(void)d;(void)shape;return nextid++;}
Cursor XCreatePixmapCursor(Display*d,Pixmap a,Pixmap b,XColor*f,XColor*back,unsigned x,unsigned y){(void)d;(void)a;(void)b;(void)f;(void)back;(void)x;(void)y;return nextid++;}
char**XGetFontPath(Display*d,int*n){(void)d;*n=0;return calloc(1,sizeof(char*));}char**XListFonts(Display*d,const char*p,int max,int*n){(void)d;(void)p;(void)max;*n=0;return calloc(1,sizeof(char*));}
int XFreeFontPath(char**p){free(p);return 1;}int XFreeFontNames(char**p){free(p);return 1;}
int XBell(Display*d,int p){(void)d;(void)p;return 1;}int XForceScreenSaver(Display*d,int m){(void)d;(void)m;return 1;}
int XFreeCursor(Display*d,Cursor c){(void)d;(void)c;return 1;}int XFreePixmap(Display*d,Pixmap p){(void)d;(void)p;return 1;}
int XDefineCursor(Display*d,Window w,Cursor c){(void)d;(void)w;(void)c;return 1;}int XUndefineCursor(Display*d,Window w){(void)d;(void)w;return 1;}
int XSetInputFocus(Display*d,Window w,int r,Time t){(void)d;(void)w;(void)r;(void)t;return 1;}int XUngrabPointer(Display*d,Time t){(void)d;(void)t;return 1;}
#define SIMPLE(name) int name(Display*d,Window w){(void)d;(void)w;return 1;}
SIMPLE(XLowerWindow) SIMPLE(XRaiseWindow) SIMPLE(XUnmapWindow)
int XMoveWindow(Display*d,Window w,int x,int y){(void)d;(void)w;(void)x;(void)y;return 1;}
int XResizeWindow(Display*d,Window w,unsigned x,unsigned y){(void)d;(void)w;(void)x;(void)y;return 1;}
int XMoveResizeWindow(Display*d,Window w,int x,int y,unsigned ww,unsigned hh){(void)d;(void)w;(void)x;(void)y;(void)ww;(void)hh;return 1;}
int XReparentWindow(Display*d,Window w,Window p,int x,int y){(void)d;(void)w;(void)p;(void)x;(void)y;return 1;}
void*XcursorImageCreate(int w,int h){struct image{unsigned version,size,width,height,xhot,yhot,delay;unsigned*pixels;};struct image*p=calloc(1,sizeof(*p));p->version=1;p->size=sizeof(*p);p->width=w;p->height=h;p->pixels=calloc((size_t)w*h,4);return p;}
void XcursorCursorsDestroy(void*p){free(p);}void*XcursorCursorsCreate(Display*d,int size){(void)d;return calloc(1,64+(size_t)size*4);}void XcursorImageDestroy(void*p){struct image{unsigned version,size,width,height,xhot,yhot,delay;unsigned*pixels;};if(p){free(((struct image*)p)->pixels);free(p);}}Cursor XcursorImageLoadCursor(Display*d,const void*i){(void)d;(void)i;return 1;}
void clarc_bridge_marker(void){}
static int image_destroy(XImage*i){if(i){free(i->data);free(i);}return 1;}static unsigned long image_get(XImage*i,int x,int y){return ((uint32_t*)(i->data+y*i->bytes_per_line))[x];}static int image_put(XImage*i,int x,int y,unsigned long v){((uint32_t*)(i->data+y*i->bytes_per_line))[x]=(uint32_t)v;return 1;}
XImage*XCreateImage(Display*d,Visual*v,unsigned dep,int format,int off,char*data,unsigned w,unsigned h,int pad,int line){(void)d;(void)v;XImage*i=calloc(1,sizeof(*i));i->width=w;i->height=h;i->xoffset=off;i->format=format;i->data=data;i->byte_order=LSBFirst;i->bitmap_unit=32;i->bitmap_bit_order=LSBFirst;i->bitmap_pad=pad;i->depth=dep;i->bits_per_pixel=32;i->bytes_per_line=line?line:(int)w*4;i->red_mask=0xff0000;i->green_mask=0xff00;i->blue_mask=0xff;i->f.destroy_image=image_destroy;i->f.get_pixel=image_get;i->f.put_pixel=image_put;return i;}
int XPutImage(Display*d,Drawable w,GC gc,XImage*i,int sx,int sy,int dx,int dy,unsigned ww,unsigned hh){(void)d;(void)w;(void)gc;(void)i;(void)sx;(void)sy;(void)dx;(void)dy;(void)ww;(void)hh;return 1;}
int XFreeGC(Display*d,GC gc){(void)d;if(gc!=(GC)1)free(gc);return 1;}
/* Redirect Unity 4's legacy HOME lookup inside the translated game only.
 * The launcher and system environment are not changed. */
char*getenv(const char*name){static char*(*real)(const char*);if(!real)real=dlsym(RTLD_NEXT,"getenv");if(!real)return NULL;if(!strcmp(name,"HOME")){char*p=real("CLARC_SAVED_HOME");if(p&&*p)return p;}return real(name);}
