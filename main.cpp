#include <Windows.h>
#include <stdint.h>
#include <Xinput.h>
#include <dsound.h>

struct win32_bufferinfo {
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel = 4;
};

struct win32_window_dimension {
    int Width;
    int Height;
};


win32_window_dimension GetWindowDimension(HWND Window) {
    
    win32_window_dimension Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return(Result);

}

static void InitDSound(HWND Window, int32_t BufferSize, int32_t SamplesPerSecond) {
    LPDIRECTSOUND DirectSound;
    if (SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0))) {
        WAVEFORMATEX WaveFormat = {};
        WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
        WaveFormat.nChannels = 2;
        WaveFormat.nSamplesPerSec = SamplesPerSecond;
        WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
        WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
        WaveFormat.wBitsPerSample = 16;
        WaveFormat.cbSize = 8;

        if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY))) {
            DSBUFFERDESC BufferDescription = {};
            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

            LPDIRECTSOUNDBUFFER PrimaryBuffer;
            if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0))) {


                if(SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat))) {

                } else {
                    // ANOTHA ERROR THING
                }
            } else {
                // Error stuff
            }
        } else {
            // Error Stuff
        }

        DSBUFFERDESC BufferDescription = {};
        BufferDescription.dwSize = sizeof(BufferDescription);
        BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

        LPDIRECTSOUNDBUFFER SecondaryBuffer;
        if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &SecondaryBuffer, 0))) {

        } else {
            // Error Stuff
        }
    } else {
        //Error Stuff
    }
}



static bool Running;
static win32_bufferinfo BackBuffer;
static void
RenderWeirdGradient(win32_bufferinfo *Buffer,int XOffSet, int YOffSet)
{

    uint8_t *Row = (uint8_t *)Buffer->Memory;
    for (int Y = 0; Y < Buffer->Height; ++Y) {
        uint32_t *Pixel = (uint32_t *)Row;
        for (int X = 0; X < Buffer->Width; ++X) {
            uint8_t Red = (X + XOffSet);
            uint8_t Blue = (Y + YOffSet);


            *Pixel++ = (Red << 16 | Blue);
        }
        Row += Buffer->Pitch;
    }
}

static void ResizeDIBSection(win32_bufferinfo *Buffer,int Width, int Height)
{

    if(Buffer->Memory) {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;
    
    int BitMapMemorySize = (Buffer->Width * Buffer->Height)*Buffer->BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitMapMemorySize, MEM_COMMIT, PAGE_READWRITE);


    Buffer->Pitch = Width*Buffer->BytesPerPixel;
}

static void CopyBufferToWindow(HDC DeviceContext, int WindowWidth, int WindowHeight, win32_bufferinfo *Buffer, int X, int Y, int Width, int Height)
{
    StretchDIBits(DeviceContext,
        /*X, Y, Width, Height,
        X, Y, Width, Height,*/
        0, 0, WindowWidth, WindowHeight,
        0, 0, Buffer->Width, Buffer->Height,
        Buffer->Memory, &Buffer->Info, DIB_RGB_COLORS,
        SRCCOPY
    );
}

LRESULT CALLBACK WindowProcedure(HWND Window, UINT Message, WPARAM WParameter, LPARAM LParameter) {
    switch(Message) {
        case WM_CLOSE: {
            Running = false;
        } break;
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP: {
                uint32_t VkCode = WParameter;
                bool WasDown = ((LParameter & (1 << 30)) != 0);
                bool IsDown = ((LParameter & (1 << 31)) == 0);

                switch(VkCode) {
                    case 'W':
                    {

                    } break; 
                    case 'S':
                    {

                    } break; 
                    case 'A':
                    {

                    } break;     
                    case 'D':
                    {

                    } break;

                    case VK_UP:
                    {

                    } break;

                    case VK_LEFT:
                    {

                    } break;

                    case VK_RIGHT:
                    {

                    } break;

                    case VK_DOWN:
                    {

                    } break;
                    case VK_ESCAPE:
                    {

                    } break;
                    case VK_SPACE:
                    {

                    } break;
                }
        } break;
        case WM_PAINT: {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

            win32_window_dimension Dimension = GetWindowDimension(Window);
            CopyBufferToWindow(DeviceContext, Dimension.Width, Dimension.Height, &BackBuffer,X, Y, Width, Height);
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

    ResizeDIBSection(&BackBuffer, 1280, 720);

    Window_Class.style = CS_OWNDC | CS_HREDRAW;
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
    InitDSound(WindowHandle, 48000 * sizeof(int32_t) * 2, 48000);
    HDC DeviceContext = GetDC(WindowHandle);
    while(Running) {

        MSG Message;
        while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
            if (Message.message == WM_QUIT) { Running = false; }
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }

        for(DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ++ControllerIndex) {
            XINPUT_STATE ControllerState;
            if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS) {
                // Controller is plugged up
                XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

                bool Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                bool Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                bool Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
                bool Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
                bool lShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                bool rShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                bool gamepadX = (Pad->wButtons & XINPUT_GAMEPAD_X);
                bool gamepadA = (Pad->wButtons & XINPUT_GAMEPAD_A);
                bool gamepadB = (Pad->wButtons & XINPUT_GAMEPAD_B);
                bool gamepadY = (Pad->wButtons & XINPUT_GAMEPAD_Y);

                int16_t ThumbStickX = Pad->sThumbLX;
                int16_t ThumbStickY = Pad->sThumbLY;

                
            }       
        }           
           
        RenderWeirdGradient(&BackBuffer, XOffset, YOffset);    
        /*DirectSound Output tezt!
        DWORD WritePointer = ;
        DWORD BytesToWrite = ;

        VOID *Region1;
        DWORD Region1Size;
        VOID *Region2;
        DWORD Region2Size;
        SecondaryBuffer->Lock(WritePointer, BytesToWrite, &Region1, &Region1Size, &Region2, &Region2Size, 0);

        for (DWORD SampleIndex) */
        win32_window_dimension Dimension = GetWindowDimension(WindowHandle);
        CopyBufferToWindow(DeviceContext, Dimension.Width, Dimension.Height, &BackBuffer, 0, 0, Dimension.Width, Dimension.Height);	
        

        ReleaseDC(WindowHandle, DeviceContext);
    }
    
    return 0;
}
