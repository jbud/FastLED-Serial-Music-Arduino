// Winamp test visualization library v1.0
// Copyright (C) 1997-1998, Justin Frankel/Nullsoft
// Feel free to base any plugins on this "framework"...

#include <windows.h>
#include "..\winamp\wa_ipc.h"
#include <cmath>
#include "vis.h"
#using <System.dll>
#include <vcclr.h>
#include <string>
using namespace System;
using namespace System::IO::Ports;
using namespace System::ComponentModel;


char szAppName[] = "SimpleVis"; // Our window class, etc

unsigned char outputarray[15];
int roundrobin[16][3];
int rrindex = 0;
float redch = 0;
float greench = 0;
float bluech = 0;
int yellowch = 0;
int specavg1old = 0;
float oldhue = 0;
float oldsaturation = 0;
float oldvalue = 0;
bool succss = false;

float specbins[10] = {0};


DWORD dwWritten = 0;
// configuration declarations
char * overrideCom = NULL;
int config_x=50, config_y=50;	// screen X position and Y position, repsectively
void config_read(struct winampVisModule *this_mod);		// reads the configuration
void config_write(struct winampVisModule *this_mod);	// writes the configuration
void config_getinifn(struct winampVisModule *this_mod, char *ini_file); // makes the .ini file filename

// returns a winampVisModule when requested. Used in hdr, below
winampVisModule *getModule(int which);

// "member" functions
void config(struct winampVisModule *this_mod); // configuration dialog
int init(struct winampVisModule *this_mod);	   // initialization for module
int render1(struct winampVisModule *this_mod);  // rendering for module 1
int render2(struct winampVisModule *this_mod);  // rendering for module 2
int render3(struct winampVisModule *this_mod);  // rendering for module 3
int render4(struct winampVisModule *this_mod);  // rendering for module 4
int render5(struct winampVisModule *this_mod);  // rendering for module 5
void quit(struct winampVisModule *this_mod);   // deinitialization for module
std::string makeStd(String ^in);
std::string getComPort();
int random_presets_flag = 0;

// our window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
HWND hMainWnd; // main window handle

// Double buffering data
HDC memDC;		// memory device context
HBITMAP	memBM,  // memory bitmap (for memDC)
		oldBM;  // old bitmap (from memDC)


// Module header, includes version, description, and address of the module retriever function
winampVisHeader hdr = { VIS_HDRVER, "Nullsoft Test Visualization Library v1.0", getModule };

// first module (oscilliscope)
winampVisModule mod1 =
{
	"Oscilloscope",
	NULL,	// hwndParent
	NULL,	// hDllInstance
	0,		// sRate
	0,		// nCh
	25,		// latencyMS
	25,		// delayMS
	0,		// spectrumNch
	2,		// waveformNch
	{ 0, },	// spectrumData
	{ 0, },	// waveformData
	config,
	init,
	render1, 
	quit
};

// second module (spectrum analyser)
winampVisModule mod2 =
{
	"Spectrum Analyser",
	NULL,	// hwndParent
	NULL,	// hDllInstance
	0,		// sRate
	0,		// nCh
	25,		// latencyMS
	25,		// delayMS
	2,		// spectrumNch
	0,		// waveformNch
	{ 0, },	// spectrumData
	{ 0, },	// waveformData
	config,
	init,
	render2, 
	quit
};

// third module (VU meter)
winampVisModule mod3 =
{
	"VU Meter",
	NULL,	// hwndParent
	NULL,	// hDllInstance
	0,		// sRate
	0,		// nCh
	25,		// latencyMS
	25,		// delayMS
	0,		// spectrumNch
	2,		// waveformNch
	{ 0, },	// spectrumData
	{ 0, },	// waveformData
	config,
	init,
	render3, 
	quit
};

// fourth module (RS232 output)
winampVisModule mod4 =
{
	"RS232 RGB",
	NULL,	// hwndParent
	NULL,	// hDllInstance
	0,		// sRate
	0,		// nCh
	25,		// latencyMS
	50,		// delayMS
	2,		// spectrumNch
	2,		// waveformNch
	{ 0, },	// spectrumData
	{ 0, },	// waveformData
	config,
	init,
	render4, 
	quit
};

winampVisModule mod5 =
{
	"Mega Wall",
	NULL,	// hwndParent
	NULL,	// hDllInstance
	0,		// sRate
	0,		// nCh
	25,		// latencyMS
	30,		// delayMS
	2,		// spectrumNch
	0,		// waveformNch
	{ 0, },	// spectrumData
	{ 0, },	// waveformData
	config,
	init,
	render5, 
	quit
};

// this is the only exported symbol. returns our main header.
// if you are compiling C++, the extern "C" { is necessary, so we just #ifdef it
#ifdef __cplusplus
extern "C" {
#endif
__declspec( dllexport ) winampVisHeader *winampVisGetHeader()
{
	return &hdr;
}
#ifdef __cplusplus
}
#endif

	DCB PortDCB;
	HANDLE hComPort;
	
	char * lpszPortName;

	
	
// getmodule routine from the main header. Returns NULL if an invalid module was requested,
// otherwise returns either mod1, mod2 or mod3 depending on 'which'.
winampVisModule *getModule(int which)
{
	switch (which)
	{
		case 0: return &mod5;
		//case 1: return &mod2;
		//case 2: return &mod3;
		//case 3: return &mod4;
		//case 4: return &mod5;
		default:return NULL;
	}
}

// configuration. Passed this_mod, as a "this" parameter. Allows you to make one configuration
// function that shares code for all your modules (you don't HAVE to use it though, you can make
// config1(), config2(), etc...)
void config(struct winampVisModule *this_mod)
{
	//std::string porta = getComPort();
	//lpszPortName =  const_cast<char *>(porta.c_str());
	if (succss) {
		MessageBox(this_mod->hwndParent, lpszPortName, "Serial Port", MB_OK);
	}
	else
	{
		MessageBox(this_mod->hwndParent, lpszPortName, "Port", MB_OK);
	}
}

// This function will convert a System^ string into a std::string
std::string makeStd(String ^in){
	array<Byte, 1> ^chars = System::Text::Encoding::ASCII->GetBytes(in);
	pin_ptr<Byte> charsPointer = &(chars[0]);
	char *nativeCharsPointer = reinterpret_cast<char *>(static_cast<unsigned char *>(charsPointer));
	std::string native(nativeCharsPointer, chars->Length);
	return nativeCharsPointer;
}

// This function will grab the second indexed com port and return it in an easily convertable std::string instead of String^
std::string getComPort(){
	array<String^, 1> ^serialPorts = nullptr;
	LPTSTR out;
	try {
		serialPorts = SerialPort::GetPortNames();
	}
	catch (Win32Exception^ ex)
	{
		MessageBox(NULL, "Failed!","Config Failed!", MB_OK);
	}
		
	std::string portb = makeStd(serialPorts[1]);
	out = const_cast<char *>(portb.c_str());
	MessageBox(NULL, out, "Com port!", MB_OK);
	return portb;
}
embedWindowState myWindowState;

winampVisModule *g_mod = NULL;

int width;
int height;
std::string portx;

// initialization. Registers our window class, creates our window, etc. Again, this one works for
// both modules, but you could make init1() and init2()...
// returns 0 on success, 1 on failure.
int init(struct winampVisModule *this_mod)
{
  int styles;
  HWND parent = NULL;
  HWND (*e)(embedWindowState *v);

	width = (this_mod == &mod3)?256:288; // width and height are the same for mod1 and mod2,
	height = (this_mod == &mod3)?32:256; // but mod3 is shaped differently

  g_mod = this_mod;

	config_read(this_mod);

	//std::string port = getComPort();
	//lpszPortName = const_cast<char *>(port.c_str());

	// This would normally work but for some reason causes corruption of the string and doesn't connect properly:
	portx = getComPort();
	if (overrideCom == NULL || overrideCom[0] == 0){
		lpszPortName = const_cast<char *>(portx.c_str());
		succss = true;
	}
	else
	{
		lpszPortName = overrideCom;
	}
	

  // uncomment this line if your plugin draws to the screen using directx OVERLAY mode
  // myWindowState.flags |= EMBED_FLAGS_NOTRANSPARENCY; 
  
  myWindowState.r.left = config_x;
  myWindowState.r.top = config_y;
  myWindowState.r.right = config_x + width;
  myWindowState.r.bottom = config_y + height;
     
  *(void**)&e = (void *)SendMessage(this_mod->hwndParent,WM_WA_IPC,(LPARAM)0,IPC_GET_EMBEDIF);

  if (!e)
  {
		MessageBox(this_mod->hwndParent,"This plugin requires Winamp 5.0+","blah",MB_OK);
		return 1;
  }

  parent = e(&myWindowState);

  SetWindowText(myWindowState.me, this_mod->description); // set our window title
	
	{	// Register our window class
		WNDCLASS wc;
		memset(&wc,0,sizeof(wc));
		wc.lpfnWndProc = WndProc;				// our window procedure
		wc.hInstance = this_mod->hDllInstance;	// hInstance of DLL
		wc.lpszClassName = szAppName;			// our window class name
	
		if (!RegisterClass(&wc)) 
		{
			MessageBox(this_mod->hwndParent,"Error registering window class","blah",MB_OK);
			return 1;
		}
	}

  styles = WS_VISIBLE|WS_CHILDWINDOW|WS_OVERLAPPED|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;


	hMainWnd = CreateWindowEx(
		0,	// these exstyles put a nice small frame, 
											// but also a button in the taskbar
		szAppName,							// our window class name
		NULL,				// no title, we're a child
		styles,				                   // do not make the window visible 
		config_x,config_y,					// screen position (read from config)
		width,height,						// width & height of window (need to adjust client area later)
		parent,				// parent window (winamp main window)
		NULL,								// no menu
		this_mod->hDllInstance,				// hInstance of DLL
		0); // no window creation data

	if (!hMainWnd) 
	{
		MessageBox(this_mod->hwndParent,"Error creating window","blah",MB_OK);
		return 1;
	}


	SetWindowLong(hMainWnd,GWL_USERDATA,(LONG)this_mod); // set our user data to a "this" pointer

/*	{	// adjust size of window to make the client area exactly width x height
		RECT r;
		GetClientRect(hMainWnd,&r);
		SetWindowPos(hMainWnd,0,0,0,width*2-r.right,height*2-r.bottom,SWP_NOMOVE|SWP_NOZORDER);
	}*/

  SendMessage(this_mod->hwndParent, WM_WA_IPC, (int)hMainWnd, IPC_SETVISWND);

	// create our doublebuffer
	memDC = CreateCompatibleDC(NULL);
	memBM = CreateCompatibleBitmap(memDC,width,height);
	oldBM = (HBITMAP)SelectObject(memDC,memBM);

  {
    RECT r={0,0,width,height};
    FillRect(memDC, &r, (HBRUSH)GetStockObject(WHITE_BRUSH));
  }

hComPort = CreateFile(lpszPortName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
PortDCB.DCBlength = sizeof(DCB);
GetCommState(hComPort, &PortDCB);
PortDCB.BaudRate = 57600;
PortDCB.ByteSize = 8;
PortDCB.Parity = NOPARITY;
PortDCB.StopBits = ONESTOPBIT;

SetCommState(hComPort, &PortDCB);



	// show the window
	ShowWindow(parent,SW_SHOWNORMAL);
	return 0;
}

// render function for oscilliscope. Returns 0 if successful, 1 if visualization should end.
int render1(struct winampVisModule *this_mod)
{
	int x, y;
	// clear background
	Rectangle(memDC,0,0,288,256);
	// draw oscilliscope
	for (y = 0; y < this_mod->nCh; y ++)
	{
		MoveToEx(memDC,0,(y*256)>>(this_mod->nCh-1),NULL);
		for (x = 0; x < 288; x ++)
		{
			LineTo(memDC,x,(y*256 + this_mod->waveformData[y][x]^128)>>(this_mod->nCh-1));
		}
	}
	{ // copy doublebuffer to window
		HDC hdc = GetDC(hMainWnd);
		BitBlt(hdc,0,0,288,256,memDC,0,0,SRCCOPY);
		ReleaseDC(hMainWnd,hdc);
	}
	return 0;
}

// render function for analyser. Returns 0 if successful, 1 if visualization should end.
int render2(struct winampVisModule *this_mod)
{
	int x, y;
	// clear background
	Rectangle(memDC,0,0,288,256);
	// draw analyser
	for (y = 0; y < this_mod->nCh; y ++)
	{
		for (x = 0; x < 288; x ++)
		{
			MoveToEx(memDC,x,(y*256+256)>>(this_mod->nCh-1),NULL);
			LineTo(memDC,x,(y*256 + 256 - this_mod->spectrumData[y][x])>>(this_mod->nCh-1));
		}
	}
	{ // copy doublebuffer to window
		HDC hdc = GetDC(hMainWnd);
		BitBlt(hdc,0,0,288,256,memDC,0,0,SRCCOPY);
		ReleaseDC(hMainWnd,hdc);
	}
	return 0;
}

// render function for VU meter. Returns 0 if successful, 1 if visualization should end.
int render3(struct winampVisModule *this_mod)
{
	int x, y;
	// clear background
	Rectangle(memDC,0,0,256,32);
	// draw VU meter
	for (y = 0; y < 2; y ++)
	{
		int last=this_mod->waveformData[y][0];
		int total=0;
		for (x = 1; x < 576; x ++)
		{
			total += abs(last - this_mod->waveformData[y][x]);
			last = this_mod->waveformData[y][x];
		}
		total /= 288;
		if (total > 127) total = 127;
		if (y) Rectangle(memDC,128,0,128+total,32);
		else Rectangle(memDC,128-total,0,128,32);
	}
	{ // copy doublebuffer to window
		HDC hdc = GetDC(hMainWnd);
		BitBlt(hdc,0,0,256,32,memDC,0,0,SRCCOPY);
		ReleaseDC(hMainWnd,hdc);
	}
	return 0;
}

int render4(struct winampVisModule *this_mod)
{

	OVERLAPPED osWrite = {0};

	outputarray[0] = 255;
	outputarray[1] = 0;
	outputarray[2] = 48;
	outputarray[3] = 48;
	outputarray[4] = 48;
	outputarray[5] = 48;
	outputarray[6] = 48;
	outputarray[7] = 48;
	outputarray[8] = 48;
	outputarray[9] = 48;
	outputarray[10] = 48;
	outputarray[11] = 48;
	outputarray[12] = 48;
	outputarray[13] = 13;

	char convhex[3];
	int x, y;
	int rightvu = 0;
	int leftvu = 0;

	for (y = 0; y < 2; y ++)
	{
		int last=this_mod->waveformData[y][0];
		int total=0;
		for (x = 1; x < 576; x ++)
		{
			total += abs(last - this_mod->waveformData[y][x]);
			last = this_mod->waveformData[y][x];
		}
		total /= 576;
		if (total > 255) total = 255;
		if (total < 1) total = 1;

		//if (y == 0) leftvu = int (log((float)total) * 47.0);
		//if (y == 1) rightvu = int (log((float)total) * 47.0);


		if (y == 0){
			
				leftvu = int (log((float)total) * 22.0);		
		}

		if (y == 1) {
		
			if (total > rightvu) {
				rightvu = int (log((float)total) * 26.0);
			} else {
				if (rightvu > 5) {
					rightvu -= 5;
				} else {
					rightvu = 0;
				}
			}

		} 
	}
		int specavg1=0;
		int specavg2=0;
		int specavg3=0;
		int specavg4=0;

		
		
		for (x = 1; x < 14; x++) {
				specavg1 += this_mod->spectrumData[0][x];
		}
		
		specavg1 = specavg1/8 - 20;

		if (specavg1 > 254) specavg1 = 254;
		if (specavg1 < 0) specavg1 = 0;
		

		for (x = 150; x < 250; x++) {
			specavg4 += this_mod->spectrumData[y][x];
		}

		specavg4 /= 100;
		if (specavg4 > 254) specavg4 = 254;

		float hue = .90 * oldhue + .1 * (((float) specavg1)/254.0);
		float saturation = 1; //.90 * oldsaturation + .10 * (float) specavg1/254.0;;
		float value =  1; // * oldvalue + .08 * specavg4/255.0;

		oldhue = hue;
		oldsaturation = saturation;
		oldvalue = value;


       if (saturation == 0) {
           redch = 0;
           greench = 0;
           bluech = 0;
       } else {
           int i = (int) floor(hue * 6);
           float f = (hue * 6) - i;
           float p = value * (1.0 - saturation);
           float q = value * (1.0 - (saturation * f));
           float t = value * (1.0 - (saturation * (1.0 - f)));
           switch (i) {
               case 1: redch = q; greench = value; bluech = p; break;
               case 2: redch = p; greench = value; bluech = t; break;
               case 3: redch = p; greench = q; bluech = value; break;
               case 4: redch = t; greench = p; bluech = value; break;
               case 5: redch = value; greench = p; bluech = q; break;
               case 6: // fall through
               case 0: redch = value; greench = t; bluech = p; break;
           }
       }


	   if (leftvu > 100) leftvu = 100;

			outputarray[2] = leftvu;
			outputarray[3] = (int) (redch*254.0);
			outputarray[4] = (int) (greench*254.0);
			outputarray[5] = (int) (bluech*254.0);		
			outputarray[6] = yellowch;

			outputarray[8] = convhex[1];
			
				
			outputarray[9] = convhex[0];
			outputarray[10] = convhex[1];

			
			outputarray[11] = convhex[0];
			outputarray[12] = convhex[1];

	WriteFile(hComPort, outputarray, 7, &dwWritten, &osWrite);
	return 0;
}

int render5(struct winampVisModule *this_mod)
{
	int x, y;
	// clear background
	Rectangle(memDC,0,0,288,256);
	// draw analyser


	int specbands[10] = {4,6,8,16,24,34,54,64,176,288};
	unsigned char specvalues[11] = {0};

	OVERLAPPED osWrite = {0};

	//for (y = 0; y < this_mod->nCh; y ++)
	for (y = 0; y < 1; y ++)
	{

		float regionavg=0;
		int region=0;
		int regioncount=0;

		for (x = 0; x < 289; x++)
		{
			if (x == specbands[region]) {

				if (region > 0) {
					regionavg /= specbands[region] - specbands[region-1];
				} else {
					regionavg /= specbands[region]-1;
				}

				//regionavg = ((float) regionavg * exp((float) x/110));


				specbins[region] = (0.8 * specbins[region] + 0.2 * regionavg);

				unsigned int specval = (log(specbins[region]/16+1)*49)*1.7;
				if (specval > 254) specval = 254;
				if (specval < 0) specval = 0;

				specvalues[region+1] = specval;

				Rectangle(memDC,region*28,256 - specvalues[region+1],region*28+28,256);
				//Rectangle(memDC,region*28,128 - (int)specbins[region]+y*128,region*28+28,128+y*128);
				
				
				region++;
				regionavg=0;

			}

		regionavg += ((float) this_mod->spectrumData[y][x] * exp((float) x/70) * (sin((float)x/91+0.4)*1.6+0.65));
		
		}

			
	}
	
	{ // copy doublebuffer to window
		HDC hdc = GetDC(hMainWnd);
		BitBlt(hdc,0,0,288,256,memDC,0,0,SRCCOPY);
		ReleaseDC(hMainWnd,hdc);
	}


	specvalues[0] = 255;
	WriteFile(hComPort, specvalues, 11, &dwWritten, &osWrite);

	return 0;
}

// cleanup (opposite of init()). Destroys the window, unregisters the window class
void quit(struct winampVisModule *this_mod)
{
  SendMessage(this_mod->hwndParent, WM_WA_IPC, 0, IPC_SETVISWND);

	config_write(this_mod);		// write configuration
	SelectObject(memDC,oldBM);	// delete our doublebuffer
	DeleteObject(memDC);
	DeleteObject(memBM);	
  // delete our window
  if (myWindowState.me) 
  {
    SetForegroundWindow(g_mod->hwndParent);
    DestroyWindow(myWindowState.me);
  }
	UnregisterClass(szAppName,this_mod->hDllInstance); // unregister window class

	CloseHandle(hComPort);
}

void next_preset()
{
}

void previous_preset()
{
}

void load_random_preset()
{
}

void set_random(int r)
{
  random_presets_flag = r;
}

void go_fullscreen()
{
  if (SendMessage(g_mod->hwndParent,WM_WA_IPC,0,IPC_IS_PLAYING_VIDEO)>1) 
  {
    MessageBox(g_mod->hwndParent, "Can't go fullscreen while video is playing", "SVis", 0);
  }
  else
  {
    SendMessage(g_mod->hwndParent,WM_WA_IPC,1,IPC_SET_VIS_FS_FLAG);

    // ... now do the work of actually going fullscreen ...

  }
}

void go_windowed()
{
  SendMessage(g_mod->hwndParent,WM_WA_IPC,0,IPC_SET_VIS_FS_FLAG);
}

void open_configuration()
{
}

void open_popup_menu()
{
}

// window procedure for our window
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:		return 0;
		case WM_ERASEBKGND: return 0;
		case WM_PAINT:
			{ // update from doublebuffer
				PAINTSTRUCT ps;
				RECT r;
				HDC hdc = BeginPaint(hwnd,&ps);
				GetClientRect(hwnd,&r);
				BitBlt(hdc,0,0,r.right,r.bottom,memDC,0,0,SRCCOPY);
        {
          RECT x={r.left+width, r.top, r.right, r.bottom};
          RECT y={r.left, r.top+height, r.right, r.bottom};
          FillRect(hdc, &x, (HBRUSH)GetStockObject(WHITE_BRUSH));
          FillRect(hdc, &y, (HBRUSH)GetStockObject(WHITE_BRUSH));
        }
				EndPaint(hwnd,&ps);
			}
		return 0;
		case WM_DESTROY: PostQuitMessage(0); return 0;
		case WM_KEYDOWN: // pass keyboard messages to main winamp window (for processing)
		case WM_KEYUP:
			{	// get this_mod from our window's user data
				winampVisModule *this_mod = (winampVisModule *) GetWindowLong(hwnd,GWL_USERDATA);
				PostMessage(this_mod->hwndParent,message,wParam,lParam);
			}
		return 0;
		case WM_WINDOWPOSCHANGING:
			{	// get config_x and config_y for configuration
				RECT r;
				GetWindowRect(myWindowState.me,&r);
				config_x = r.left;
				config_y = r.top;
			}
		return 0;
    case WM_COMMAND: {
      int id = LOWORD(wParam);
      switch (id) {

        // user clicked on 'next' preset button
        case ID_VIS_NEXT: next_preset(); break;

        // user clicked on 'previous' preset button
        case ID_VIS_PREV: previous_preset(); break;

        // user clicked on 'random' togglebutton
        case ID_VIS_RANDOM: {
          // determine if we're switching random on or off or if Winamp is asking us about the state of our random flag
          int v = HIWORD(wParam) ? 1 : 0; 

          // are we being asked about the state of our random flag ?
          if (wParam >> 16 == 0xFFFF) {
            // tell winamp about our state
            SendMessage(g_mod->hwndParent,WM_WA_IPC,random_presets_flag,IPC_CB_VISRANDOM);
            break;
          }
      
          // changes random_preset_flag 
          set_random(v); 

          // if we are turning random on, we should switch to a new random preset right away
          if (v) load_random_preset();

          break;
        }
        case ID_VIS_FS: go_fullscreen(); break;
        case ID_VIS_CFG: open_configuration(); break;
        case ID_VIS_MENU: open_popup_menu(); break;
      }
      break;
    }
	}
	return DefWindowProc(hwnd,message,wParam,lParam);
}


void config_getinifn(struct winampVisModule *this_mod, char *ini_file)
{	// makes a .ini file in the winamp directory named "plugin.ini"
	char *p;
	GetModuleFileName(this_mod->hDllInstance,ini_file,MAX_PATH);
	p=ini_file+strlen(ini_file);
	while (p >= ini_file && *p != '\\') p--;
	if (++p >= ini_file) *p = 0;
	strcat(ini_file,"plugin.ini");
}


void config_read(struct winampVisModule *this_mod)
{
	char ini_file[MAX_PATH];
	char* tResult = new char[255];
	config_getinifn(this_mod,ini_file);
	config_x = GetPrivateProfileInt(this_mod->description,"Screen_x",config_x,ini_file);
	config_y = GetPrivateProfileInt(this_mod->description,"Screen_y",config_y,ini_file);
	// Grab the overrideCom= param. This isn't currently used, but will be later implemented:
	GetPrivateProfileString(this_mod->description, "overrideCom", NULL, tResult, 255, ini_file);
	overrideCom = tResult;
}

void config_write(struct winampVisModule *this_mod)
{
	char string[32];
	char ini_file[MAX_PATH];

	config_getinifn(this_mod,ini_file);

	wsprintf(string,"%d",config_x);
	WritePrivateProfileString(this_mod->description,"Screen_x",string,ini_file);
	wsprintf(string,"%d",config_y);
	WritePrivateProfileString(this_mod->description,"Screen_y",string,ini_file);
	// We wont set the override here:
	//wsprintf(string, "%s", overrideCom);
	//WritePrivateProfileString(this_mod->description, "overrideCom", string, ini_file);
}
