#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

#undef CurrentTime /* Defined by X.h */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <exec/memory.h>
#include <clib/exec_protos.h>
#include "intuition_intern.h"
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

static struct MsgPort * intuiReplyPort;

extern Display * sysDisplay;
extern long sysCMap[];
extern unsigned long sysPlaneMask;
extern Cursor sysCursor;

static struct Screen WB;

Display * GetSysDisplay (void);
int	  GetSysScreen (void);
extern void SetGC (struct RastPort * rp, GC gc);
extern GC GetGC (struct RastPort * rp);
extern void SetXWindow (struct RastPort * rp, int win);

struct IntWindow
{
    struct Window iw_Window;
    int 	  iw_XWindow;
    Region	  iw_Region;
};

struct _keytable
{
	KeySym keysym;
	WORD amiga;
	UWORD amiga_qual;
	char * normal, * shifted;
	ULONG keycode;
} keytable[] =
{
    {XK_Return, 0x44, 0, "\012", "\012", 0 },
    {XK_Right, 0x4e, 0, "\233C", "\233 A", 0 },
    {XK_Up, 0x4c, 0, "\233A", "\233T",0 },
    {XK_Left, 0x4f, 0, "\233D", "\233 @",0 },
    {XK_Down, 0x4d, 0, "\233B", "\233S",0 },
    {XK_Help, 0x5f, 0, "\233?~", "\233?~",0 },
    {XK_KP_Enter, 0x43, IEQUALIFIER_NUMERICPAD, "\015", "\015",0 },
    {XK_KP_Separator, 0x3c, IEQUALIFIER_NUMERICPAD, ".", ".",0 },
    {XK_KP_Subtract, 0x4a, IEQUALIFIER_NUMERICPAD, "-", "-",0 },
    {XK_KP_Decimal, 0x3c, IEQUALIFIER_NUMERICPAD, ".", ".",0 },
    {XK_KP_0, 0x0f, IEQUALIFIER_NUMERICPAD, "0", "0",0 },
    {XK_KP_1, 0x1d, IEQUALIFIER_NUMERICPAD, "1", "1",0 },
    {XK_KP_2, 0x1e, IEQUALIFIER_NUMERICPAD, "2", "2",0 },
    {XK_KP_3, 0x1f, IEQUALIFIER_NUMERICPAD, "3", "3",0 },
    {XK_KP_4, 0x2d, IEQUALIFIER_NUMERICPAD, "4", "4",0 },
    {XK_KP_5, 0x2e, IEQUALIFIER_NUMERICPAD, "5", "5",0 },
    {XK_KP_6, 0x2f, IEQUALIFIER_NUMERICPAD, "6", "6",0 },
    {XK_KP_7, 0x3d, IEQUALIFIER_NUMERICPAD, "7", "7",0 },
    {XK_KP_8, 0x3e, IEQUALIFIER_NUMERICPAD, "8", "8",0 },
    {XK_KP_9, 0x3f, IEQUALIFIER_NUMERICPAD, "9", "9",0 },
    {XK_F1, 0x50, 0, "\2330~", "\23310~",0 },
    {XK_F2, 0x51, 0, "\2331~", "\23311~",0 },
    {XK_F3, 0x52, 0, "\2332~", "\23312~",0 },
    {XK_F4, 0x53, 0, "\2333~", "\23313~",0 },
    {XK_F5, 0x54, 0, "\2334~", "\23314~",0 },
    {XK_F6, 0x55, 0, "\2335~", "\23315~",0 },
    {XK_F7, 0x56, 0, "\2336~", "\23316~",0 },
    {XK_F8, 0x57, 0, "\2337~", "\23317~",0 },
    {XK_F9, 0x58, 0, "\2338~", "\23318~",0 },
    {XK_F10, 0x59, 0, "\2339~", "\23319~",0 },
    {0, -1, 0, },
};

#define SHIFT	(IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT)
#define ALT	(IEQUALIFIER_LALT | IEQUALIFIER_RALT)
#define CTRL	IEQUALIFIER_CONTROL
#define CAPS	IEQUALIFIER_CAPSLOCK


int intui_init (struct IntuitionBase * IntuitionBase)
{
    int t;

    memset (&WB, 0, sizeof(struct Screen));

    WB.Width = DisplayWidth (GetSysDisplay (), GetSysScreen ());
    WB.Height = DisplayHeight (GetSysDisplay (), GetSysScreen ());

    IntuitionBase->FirstScreen = IntuitionBase->ActiveScreen = &WB;

    for (t=0; keytable[t].amiga != -1; t++)
	keytable[t].keycode = XKeysymToKeycode (sysDisplay,
		keytable[t].keysym);

    intuiReplyPort = CreateMsgPort ();

    return True;
}

int intui_open (struct IntuitionBase * IntuitionBase)
{
    return True;
}

void intui_close (struct IntuitionBase * IntuitionBase)
{
    return;
}

void intui_expunge (struct IntuitionBase * IntuitionBase)
{
    return;
}

void intui_SetWindowTitles (struct Window * win, char * text, char * screen)
{
    XSizeHints hints;
    struct IntWindow * w;

    w = (struct IntWindow *)win;

    hints.x	 = win->LeftEdge;
    hints.y	 = win->TopEdge;
    hints.width  = win->Width;
    hints.height = win->Height;
    hints.flags  = PPosition | PSize;

    if (screen == (char *)-1)
	screen = "Amiga";

    XSetStandardProperties (sysDisplay, w->iw_XWindow, text, screen,
	    None, NULL, 0, &hints);
}

struct Window * intui_OpenWindow (struct NewWindow * nw,
	struct IntuitionBase * IntuitionBase)
{
    struct Window * w;
    struct IntWindow * iw;
    struct RastPort * rp;
    XGCValues gcval;
    GC gc;

    iw = AllocMem (sizeof (struct IntWindow), MEMF_CLEAR);
    rp = AllocMem (sizeof (struct RastPort), MEMF_CLEAR);

    w = &iw->iw_Window;

    if (nw->IDCMPFlags)
    {
	w->UserPort = CreateMsgPort ();

	if (!w->UserPort)
	{
	    FreeMem (iw, sizeof (struct IntWindow));
	    FreeMem (rp, sizeof (struct RastPort));

	    return NULL;
	}
    }

    w->LeftEdge = nw->LeftEdge;
    w->TopEdge = nw->TopEdge;
    w->Width = nw->Width;
    w->Height = nw->Height;
    w->RPort = rp;
    w->IDCMPFlags = nw->IDCMPFlags;

    if (nw->DetailPen == 0xff) nw->DetailPen = 1;
    if (nw->BlockPen == 0xff) nw->BlockPen = 0;

    iw->iw_XWindow = XCreateSimpleWindow (GetSysDisplay (),
	    DefaultRootWindow (GetSysDisplay ()),
	    nw->LeftEdge, nw->TopEdge, nw->Width, nw->Height, 15,
	    sysCMap[1], sysCMap[0]);

    intui_SetWindowTitles (w, nw->Title, (STRPTR)-1);

    gcval.plane_mask = sysPlaneMask;

    gc = XCreateGC (sysDisplay, iw->iw_XWindow, GCPlaneMask, &gcval);

    SetGC (rp, gc);
    SetXWindow (rp, iw->iw_XWindow);

    XSetGraphicsExposures (sysDisplay, gc, TRUE);

    w->BorderLeft = 2;
    w->BorderTop = GfxBase->DefaultFont->tf_YSize + 3;
    w->BorderRight = 2;
    w->BorderBottom = 2;

    w->WScreen = &WB;

    SetFont (rp, GfxBase->DefaultFont);

    w->MinWidth  = nw->MinWidth;
    w->MinHeight = nw->MinHeight;
    w->MaxWidth  = nw->MaxWidth;
    w->MaxHeight = nw->MaxHeight;

    /*TODO __SetSizeHints (w); */

    SetAPen (rp, nw->DetailPen);
    SetBPen (rp, nw->BlockPen);
    SetDrMd (rp, JAM2);

    w->Parent = NULL;
    w->NextWindow = w->Descendant = WB.FirstWindow;
    WB.FirstWindow = (struct Window *)iw;

    if (nw->Flags & ACTIVATE)
	IntuitionBase->ActiveWindow = (struct Window *)iw;

    iw->iw_Region = XCreateRegion ();

    XSelectInput (sysDisplay, iw->iw_XWindow, ExposureMask |
	ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
	KeyPressMask | KeyReleaseMask |
	EnterWindowMask | LeaveWindowMask |
	StructureNotifyMask);

    ModifyIDCMP (w, w->IDCMPFlags);

    XMapRaised (sysDisplay, iw->iw_XWindow);
    XDefineCursor (sysDisplay, iw->iw_XWindow, sysCursor);

    XSync (sysDisplay, FALSE);

    return (w);
}

void intui_CloseWindow (struct Window * win,
	    struct IntuitionBase * IntuitionBase)
{
    struct IntWindow * iw;
    struct Window * wptr;

    iw = (struct IntWindow *)win;

    if (win->Descendant)
	win->Descendant->Parent = win->Parent;
    if (win->Parent)
	win->Parent->NextWindow = win->Parent->Descendant = win->Descendant;

    if (win == WB.FirstWindow)
	WB.FirstWindow = win->NextWindow;

    IntuitionBase->ActiveWindow = wptr;

    XDestroyWindow (sysDisplay, iw->iw_XWindow);
    XDestroyRegion (iw->iw_Region);
    XFreeGC (sysDisplay, GetGC(win->RPort));

    FreeMem (win->RPort, sizeof (struct RastPort));

    if (win->UserPort)
	DeleteMsgPort (win->UserPort);

    FreeMem (iw, sizeof (struct IntWindow));

    XSync (sysDisplay, FALSE);
}

void intui_WindowToFront (struct IntWindow * window)
{
    XRaiseWindow (sysDisplay, window->iw_XWindow);
}

long StateToQualifier (unsigned long state)
{
    long result;

    result = 0;

    if (state & ShiftMask)
	result |= SHIFT;

    if (state & ControlMask)
	result |= CTRL;

    if (state & LockMask)
	result |= CAPS;

    if (state & Mod1Mask)
	result |= ALT;

    if (state & Mod2Mask)
	result |= AMIGAKEYS;

    if (state & Button1Mask)
	result |= IEQUALIFIER_LEFTBUTTON;

    if (state & Button2Mask)
	result |= IEQUALIFIER_RBUTTON;

    if (state & Button3Mask)
	result |= IEQUALIFIER_MIDBUTTON;

    return (result);
}

long XKeyToAmigaCode (XKeyEvent * xk)
{
    char buffer[10];
    KeySym ks;
    int count;
    long result;
    short t;

    result = 0;

    if (xk->state & ShiftMask)
	result |= SHIFT;

    if (xk->state & ControlMask)
	result |= CTRL;

    if (xk->state & LockMask)
	result |= CAPS;

    if (xk->state & Mod1Mask)
	result |= ALT;

    if (xk->state & Mod2Mask)
	result |= AMIGAKEYS;

    if (xk->state & Button1Mask)
	result |= IEQUALIFIER_LEFTBUTTON;

    if (xk->state & Button2Mask)
	result |= IEQUALIFIER_RBUTTON;

    if (xk->state & Button3Mask)
	result |= IEQUALIFIER_MIDBUTTON;

    result <<= 16;

    xk->state = 0;
    count = XLookupString (xk, buffer, 10, &ks, NULL);

    for (t=0; keytable[t].amiga != -1; t++)
    {
	if (ks == keytable[t].keycode)
	{
	    result |= (keytable[t].amiga_qual << 16) | keytable[t].amiga;
	    return (result);
	}
    }

    result |= xk->keycode & 0xffff;

    return (result);
}

#undef SysBase /* The next function needs the global SysBase */
extern struct ExecBase * SysBase;

void intui_ProcessXEvents (void)
{
    struct IntuiMessage * im;
    struct Window * w;
    struct IntWindow * iw;
    char * ptr;
    static int mpos_x, mpos_y;
    XEvent event;

    if (!intuiReplyPort || !WB.FirstWindow) /* NOP if no intuition.library */
	return;

    /* Empty port */
    while ((im = (struct IntuiMessage *)GetMsg (intuiReplyPort)))
	FreeMem (im, sizeof (struct IntuiMessage));

    im = NULL;
    w = NULL;

    while (XPending (sysDisplay))
    {
	XNextEvent (sysDisplay, &event);

	if (event.type == MappingNotify)
	{
	    XRefreshKeyboardMapping ((XMappingEvent*)&event);
	    continue;
	}

	for (w=WB.FirstWindow; w; w=w->NextWindow)
	{
	    if (((struct IntWindow *)w)->iw_XWindow == event.xany.window)
		break;
	}

	if (!w)
	    continue;

	iw = (struct IntWindow *)w;

	if (!im)
	    im = AllocMem (sizeof (struct IntuiMessage), MEMF_CLEAR);

	im->Class	= 0L;
	im->IDCMPWindow = w;
	im->MouseX	= mpos_x;
	im->MouseY	= mpos_y;

	switch (event.type)
	{
	case GraphicsExpose:
	case Expose: {
	    XRectangle rect;
	    UWORD      count;

	    if (event.type == Expose)
	    {
		rect.x	    = event.xexpose.x;
		rect.y	    = event.xexpose.y;
		rect.width  = event.xexpose.width;
		rect.height = event.xexpose.height;
		count	    = event.xexpose.count;
	    }
	    else
	    {
		rect.x	    = event.xgraphicsexpose.x;
		rect.y	    = event.xgraphicsexpose.y;
		rect.width  = event.xgraphicsexpose.width;
		rect.height = event.xgraphicsexpose.height;
		count	    = event.xgraphicsexpose.count;
	    }

	    XUnionRectWithRegion (&rect, iw->iw_Region, iw->iw_Region);

	    if (count != 0)
		break;

	    im->Class = REFRESHWINDOW;
	    ptr       = "REFRESHWINDOW";
	} break;

	case ConfigureNotify:
	    if (w->Width != event.xconfigure.width ||
		    w->Height != event.xconfigure.height)
	    {
		w->Width  = event.xconfigure.width;
		w->Height = event.xconfigure.height;

		im->Class = NEWSIZE;
		ptr	  = "NEWSIZE";
	    }

	    break;

	case ButtonPress: {
	    XButtonEvent * xb = &event.xbutton;

	    im->Class = MOUSEBUTTONS;
	    im->Qualifier = StateToQualifier (xb->state);
	    im->MouseX = xb->x;
	    im->MouseY = xb->y;

	    switch (xb->button)
	    {
	    case Button1:
		im->Code = SELECTDOWN;
		break;

	    case Button2:
		im->Code = MIDDLEDOWN;
		break;

	    case Button3:
		im->Code = MENUDOWN;
		break;
	    }

	    ptr = "MOUSEBUTTONS";
	} break;

	case ButtonRelease: {
	    XButtonEvent * xb = &event.xbutton;

	    im->Class = MOUSEBUTTONS;
	    im->Qualifier = StateToQualifier (xb->state);
	    im->MouseX = xb->x;
	    im->MouseY = xb->y;

	    switch (xb->button)
	    {
	    case Button1:
		im->Code = SELECTUP;
		break;

	    case Button2:
		im->Code = MIDDLEUP;
		break;

	    case Button3:
		im->Code = MENUUP;
		break;
	    }

	    ptr = "MOUSEBUTTONS";
	} break;

	case KeyPress: {
	    XKeyEvent * xk = &event.xkey;
	    ULONG result;

	    im->Class = RAWKEY;
	    result = XKeyToAmigaCode(xk);
	    im->Code = xk->keycode;
	    im->Qualifier = result >> 16;

	    ptr = NULL;
	} break;

	case KeyRelease: {
	    XKeyEvent * xk = &event.xkey;
	    ULONG result;

	    im->Class = RAWKEY;
	    result = XKeyToAmigaCode(xk);
	    im->Code = xk->keycode | 0x8000;
	    im->Qualifier = result >> 16;

	    ptr = NULL;
	} break;

	case MotionNotify: {
	    XMotionEvent * xm = &event.xmotion;

	    im->Code = IECODE_NOBUTTON;
	    im->Class = MOUSEMOVE;
	    im->Qualifier = StateToQualifier (xm->state);
	    im->MouseX = xm->x;
	    im->MouseY = xm->y;

	    ptr = "MOUSEMOVE";
	} break;

	case EnterNotify: {
	    XCrossingEvent * xc = &event.xcrossing;

	    im->Class = ACTIVEWINDOW;
	    im->MouseX = xc->x;
	    im->MouseY = xc->y;

	    ptr = "ACTIVEWINDOW";
	} break;

	case LeaveNotify: {
	    XCrossingEvent * xc = &event.xcrossing;

	    im->Class = ACTIVEWINDOW;
	    im->MouseX = xc->x;
	    im->MouseY = xc->y;

	    ptr = "INACTIVEWINDOW";
	} break;

	default:
	break;
	} /* switch */

	mpos_x = im->MouseX;
	mpos_y = im->MouseY;

	if (im->Class)
	{
	    if ((im->Class & w->IDCMPFlags) && w->UserPort)
	    {
		im->ExecMessage.mn_ReplyPort = intuiReplyPort;

		PutMsg (w->UserPort, (struct Message *)im);
		im = NULL;
	    }
	    else
		im->Class = 0;
	}
   }

   if (im)
   {
      FreeMem (im, sizeof (struct IntuiMessage));
   }
}

#ifdef TODO
void RefreshWindowFrame (w)
WIN * w;
{
      __SetSizeHints (w);
}

int IntuiTextLength (itext)
struct IntuiText * itext;
{
	return (100);
}

void SizeWindow (win, dx, dy)
WIN * win;
int dx, dy;
{
	XResizeWindow (sysDisplay, win->xwindow, win->Width+dx,
		win->Height + dy);
}

void ClearMenuStrip (win)
WIN * win;
{
	return;
}

void ActivateWindow (win)
WIN * win;
{
    XSetInputFocus (sysDisplay, win->xwindow, RevertToNone, CurrentTime);

    IntuitionBase->ActiveWindow = (struct Window *)win;
}

void BeginRefresh (win)
WIN * win;
{
   /* Region aufpropfen */
   XSetRegion (sysDisplay, win->RPort->gc, win->region);
}


void EndRefresh (win, free)
WIN * win;
BOOL free;
{
   Region region;
   XRectangle rect;

   /* Zuerst alte Region freigeben (Speicher sparen) */
   if (free)
   {
      XDestroyRegion (win->region);

      win->region = XCreateRegion ();
   }

   /* Dann loeschen wir das ClipRect wieder indem wir ein neues
      erzeugen, welches das ganze Fenster ueberdeckt. */
   region = XCreateRegion ();

   rect.x      = 0;
   rect.y      = 0;
   rect.width  = win->Width;
   rect.height = win->Height;

   XUnionRectWithRegion (&rect, region, region);

   /* und setzen */
   XSetRegion (sysDisplay, win->RPort->gc, region);
}
int AutoRequest (win, body, pos, neg, f_pos, f_neg, width, height)
WIN * win;
struct IntuiText * body, * pos, * neg;
ULONG f_pos, f_neg;
SHORT width, height;
{
	return (TRUE);
}

void GetScreenData (scr, size, type, screen)
struct Screen * scr, * screen;
long size, type;
{
	if (type != WBENCHSCREEN) {
		movmem (scr, screen, size);
	} else {
		WB.Width = DisplayWidth(sysDisplay, sys_screen);
		WB.Height = DisplayHeight (sysDisplay, sys_screen);
		movmem (scr, &WB, size);
	}
}

int RawKeyConvert (ie, buf, size, km)
IEV * ie;
char * buf;
long size;
struct KeyMap * km;
{
	XKeyEvent xk;
	char * ptr;
	int t;

	ie->ie_Code &= 0x7fff;

	for (t=0; keytable[t].amiga != -1; t++) {
		if (ie->ie_Code == keytable[t].keycode) {
			if (ie->ie_Qualifier & SHIFT)
				ptr = keytable[t].shifted;
			else	ptr = keytable[t].normal;

			t = strlen(ptr);
			if (t > size) t = size;

			strncpy (buf, ptr, t);

			goto ende;
		}
	}

	xk.keycode = ie->ie_Code;
	xk.display = sysDisplay;
	xk.state = 0;

	if (ie->ie_Qualifier & SHIFT)
		xk.state |= ShiftMask;

	if (ie->ie_Qualifier & CTRL)
		xk.state |= ControlMask;

	if (ie->ie_Qualifier & CAPS)
		xk.state |= LockMask;

	if (ie->ie_Qualifier & ALT)
		xk.state |= Mod1Mask;

	if (ie->ie_Qualifier & AMIGAKEYS)
		xk.state |= Mod2Mask;

	if (ie->ie_Qualifier & IEQUALIFIER_LEFTBUTTON)
		xk.state |= Button1Mask;

	if (ie->ie_Qualifier & IEQUALIFIER_MIDBUTTON)
		xk.state |= Button2Mask;

	if (ie->ie_Qualifier & IEQUALIFIER_RBUTTON)
		xk.state |= Button3Mask;

	t = XLookupString(&xk, buf, size, NULL, NULL);

	if (!*buf && t == 1) t = 0;
	if (!t) *buf = 0;

ende:	/*printf ("RawKeyConvert: In %02x %04x %04x Out : %d cs %02x '%c'\n",
		ie->ie_Code, ie->ie_Qualifier, xk.state, t, (ubyte)*buf,
		(ubyte)*buf);*/

	return (t);
}

#endif
