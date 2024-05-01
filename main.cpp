#include <Windows.h>

static bool Running;
static BITMAPINFO bitMapInfo;
static void *BitMapMemory;
static HBITMAP BitMapHandle;
static HDC BitMapDeviceContext;

static void ResizeDIBSection(int Width, int Height)
{
	if (BitMapHandle) {
		DeleteObject(BitMapHandle);
	}
	
	if (!BitMapDeviceContext) {
		BitMapDeviceContext = CreateCompatibleDC(0);
	}

	bitMapInfo.bmiHeader.biSize = sizeof(bitMapInfo.bmiHeader);
	bitMapInfo.bmiHeader.biWidth = Width;
	bitMapInfo.bmiHeader.biHeight = Height;
	bitMapInfo.bmiHeader.biPlanes = 1;
	bitMapInfo.bmiHeader.biBitCount = 32;
	bitMapInfo.bmiHeader.biCompression = BI_RGB;


	BitMapHandle = CreateDIBSection(
		BitMapDeviceContext,
		&bitMapInfo,
		DIB_RGB_COLORS,
		&BitMapMemory,
		0,
		0);

}

static void UpdateScreen(HDC DeviceContext, int X, int Y, int Width, int Height)
{
	StretchDIBits(DeviceContext,
		X, Y, Width, Height,
		X, Y, Width, Height,
		BitMapMemory, &bitMapInfo, DIB_RGB_COLORS, 
		SRCCOPY
	);
}

LRESULT CALLBACK WindowProcedure(HWND Window, UINT Message, WPARAM WParameter, LPARAM LParameter) {
	switch(Message) {
		case WM_CLOSE: {
			Running = false;
		} break;
		case WM_SIZE: {
			RECT ClientRect;
			GetClientRect(Window, &ClientRect);
			int Width = ClientRect.right - ClientRect.left;
			int Height = ClientRect.bottom - ClientRect.top;
			ResizeDIBSection(Width, Height);
		} break;
		case WM_PAINT: {
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(Window, &Paint);
			int X = Paint.rcPaint.left;
			int Y = Paint.rcPaint.top;
			int Width = Paint.rcPaint.right - Paint.rcPaint.left;
			int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
			UpdateScreen(DeviceContext, X, Y, Width, Height);
			EndPaint(Window, &Paint);
		} break;
		default: {
			return DefWindowProcA(Window, Message, WParameter, LParameter);
		} break;	
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR pCmdLine, int nCmdShow) {

	WNDCLASSA Window_Class = {};
	Window_Class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	Window_Class.lpfnWndProc = WindowProcedure;
	Window_Class.hInstance = Instance;
	Window_Class.lpszClassName = "Window With Vim";

	if (!RegisterClassA(&Window_Class)) {return 0;}

	HWND WindowHandle = CreateWindowExA(
	  0,
	  Window_Class.lpszClassName,
	  "Window",
	  WS_OVERLAPPEDWINDOW | WS_VISIBLE,
	  CW_USEDEFAULT,
	  CW_USEDEFAULT,
	  CW_USEDEFAULT,
	  CW_USEDEFAULT,
	  0,
	  0,
	  Instance,
	  0); 

	if (!WindowHandle) { return 0; }
	Running = true;
	while(Running) {
		MSG Message;
		BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
		if (MessageResult > 0) {
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		} else {
			break;
		}
	}
	
	return 0;
}
