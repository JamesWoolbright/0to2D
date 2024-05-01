#include <Windows.h>
#include <stdint.h>

static bool Running;

static BITMAPINFO bitMapInfo;
static void *BitMapMemory;
static int BitMapWidth;
static int BitMapHeight;
static int BytesPerPixel = 4;

static void
RenderWeirdGradient(int XOffSet, int YOffSet)
{
	int Width = BitMapWidth;
	int Height = BitMapHeight;


	int Pitch = Width*BytesPerPixel;
	uint8_t *Row = (uint8_t *)BitMapMemory;
	for (int Y = 0; Y < BitMapHeight; ++Y) {
		uint8_t *Pixel = (uint8_t *)Row;
		for (int X = 0; X < BitMapWidth; ++X) {
			*Pixel = (uint8_t)(X + XOffSet); // B
			++Pixel;
			*Pixel = (uint8_t)(Y + YOffSet); // G
			++Pixel;
			*Pixel = 255; // R
			++Pixel;
			*Pixel = 0;
			++Pixel;
		}
		Row += Pitch;
	}
}

static void ResizeDIBSection(int Width, int Height)
{

	if(BitMapMemory) {
		VirtualFree(BitMapMemory, 0, MEM_RELEASE);
	}

	BitMapWidth = Width;
	BitMapHeight = Height;

	bitMapInfo.bmiHeader.biSize = sizeof(bitMapInfo.bmiHeader);
	bitMapInfo.bmiHeader.biWidth = BitMapWidth;
	bitMapInfo.bmiHeader.biHeight = -BitMapHeight;
	bitMapInfo.bmiHeader.biPlanes = 1;
	bitMapInfo.bmiHeader.biBitCount = 32;
	bitMapInfo.bmiHeader.biCompression = BI_RGB;
	
	int BytesPerPixel = 4;
	int BitMapMemorySize = (BitMapWidth * BitMapHeight)*BytesPerPixel;
	BitMapMemory = VirtualAlloc(0, BitMapMemorySize, MEM_COMMIT, PAGE_READWRITE);


}

static void UpdateScreen(HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height)
{
	int WindowWidth = ClientRect->right - ClientRect->left;
	int WindowHeight = ClientRect->bottom - ClientRect->top;
	StretchDIBits(DeviceContext,
		/*X, Y, Width, Height,
		X, Y, Width, Height,*/
		0, 0, BitMapWidth, BitMapHeight,
		0, 0, WindowWidth, WindowHeight,
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

			RECT ClientRect;
			GetClientRect(Window, &ClientRect);

			UpdateScreen(DeviceContext, &ClientRect, X, Y, Width, Height);
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
	int XOffset = 0;
	int YOffset = 0;
	while(Running) {

		MSG Message;
		while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
			if (Message.message == WM_QUIT) { Running = false; }
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
		RenderWeirdGradient(XOffset, YOffset);
		HDC DeviceContext = GetDC(WindowHandle);
		RECT ClientRect;
		GetClientRect(WindowHandle, &ClientRect);
		int WindowWidth = ClientRect.right - ClientRect.left;
		int WindowHeight = ClientRect.bottom - ClientRect.top;
		UpdateScreen(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);	
		ReleaseDC(WindowHandle, DeviceContext);
		++XOffset;
		++YOffset;
	}
	
	return 0;
}
