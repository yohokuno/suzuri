#define STRICT
#define DIRECTINPUT_VERSION 0x0800
#include <windows.h>
#include <windowsx.h>
#include <wingdi.h>

#include <ddraw.h>
#include <dinput.h>
#include <dsound.h>
#include <dmusici.h>
#include <mmsystem.h>

#include <stdio.h>
#include <math.h>
#include <time.h>

#include <list>
#include <vector>
#include <algorithm>
using namespace std;

#include <crtdbg.h>
#define _CRTDBG_MAP_ALLOC
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)

#include "utility\\resource.h"
#include "utility\\loadpng.h"

#pragma comment(lib,"dxguid")
#pragma comment(lib,"winmm")
#pragma comment(lib,"gdi32")
#pragma comment(lib,"ddraw")
#pragma comment(lib,"dinput8")
#pragma comment(lib,"dsound")
