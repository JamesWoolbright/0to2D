#include <Windows.h>
#include <stdint.h>
#include <dsound.h>
#include <math.h>

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


static LPDIRECTSOUNDBUFFER SecondaryBuffer;

static void InitDSound(HWND Window, int32_t BufferSize, int32_t SamplesPerSecond) {
    LPDIRECTSOUND DirectSound;
    if (SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0))) {
        WAVEFORMATEX WaveFormat;
        WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
        WaveFormat.nChannels = 2;
        WaveFormat.wBitsPerSample = 16;
        WaveFormat.nSamplesPerSec = SamplesPerSecond;
        WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
        WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;

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
        BufferDescription.dwFlags = 0;
        BufferDescription.dwBufferBytes = BufferSize;
        BufferDescription.lpwfxFormat = &WaveFormat;

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

static int XOffset = 0;
static int YOffset = 0;



static void RenderWeirdGradient(win32_bufferinfo *Buffer, int XOffset, int YOffset)
{
    uint8_t *Row = (uint8_t *)Buffer->Memory;
    for (int Y = 0; Y < Buffer->Height; ++Y) {
        uint32_t *Pixel = (uint32_t *)Row;
        for (int X = 0; X < Buffer->Width; ++X) {
            // Calculate the Red component with a horizontal gradient effect
            uint8_t Red = ((X + XOffset) ^ (Y + YOffset)) + (Buffer->Width - X);

            uint8_t Green = (X - XOffset) ^ (Y - YOffset);
            uint8_t Blue = (X * Y) ^ (XOffset + YOffset);
        
            // Combine the color components into a single pixel value
            uint32_t Color = (Red << 16) | (Green << 8) | Blue;

            // Set the pixel value
            *Pixel++ = Color;
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
                        YOffset += 20;
                    } break; 
                    case 'S':
                    {
                        YOffset -= 20;
                    } break; 
                    case 'A':
                    {
                        XOffset += 20;
                    } break;     
                    case 'D':
                    {
                        XOffset -= 20;
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
    // Graphics Test tings


    //SoundTestThings
    int SamplesPerSecond = 48000;
    int hz = 256;
    uint32_t RunningSampleIndex = 0;
    int SquareWavePeriod = SamplesPerSecond/hz;
    int BytesPerSample = sizeof(int16_t) * 2;
    int BufferSize = SamplesPerSecond * BytesPerSample;



    InitDSound(WindowHandle, BufferSize, SamplesPerSecond);
    SecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
    HDC DeviceContext = GetDC(WindowHandle);

    int currentFrame = 0;

    while(Running) {

        MSG Message;
        while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
            if (Message.message == WM_QUIT) { Running = false; }
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }

        /*for(DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ++ControllerIndex) {
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

                XOffset += ThumbStickX << 14;
                YOffset -= ThumbStickY << 14;
            }       
        }           */
        RenderWeirdGradient(&BackBuffer, XOffset, YOffset);
        // DirectSound Output tezt!
        DWORD PlayCursor;
        DWORD WriteCursor;
        if(SUCCEEDED(SecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor))) {

            DWORD BytesToLock = RunningSampleIndex * BytesPerSample % BufferSize;
            DWORD BytesToWrite;
            if(BytesToLock > PlayCursor) {
                BytesToWrite = (BufferSize - BytesToLock);
                BytesToWrite += PlayCursor;
            } else {
                BytesToWrite = PlayCursor - BytesToLock;
            }

            VOID *Region1;
            DWORD Region1Size;
            VOID *Region2;
            DWORD Region2Size;

            

            if(SUCCEEDED(SecondaryBuffer->Lock(BytesToLock, BytesToWrite, &Region1, &Region1Size, &Region2, &Region2Size, 0))) {
                int16_t *SampleOut = (int16_t *)Region1;
                DWORD Region1SampleCount = Region1Size/BytesPerSample;
                for (DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex) {
                    int16_t SampleValue = ((RunningSampleIndex++ / (SquareWavePeriod / 2)) % 2) ? 1000 : -1000;
                    *SampleOut++ = SampleValue;
                    *SampleOut++ = SampleValue;
                }
                DWORD Region2SampleCount = Region2Size/BytesPerSample;
                SampleOut = (int16_t *)Region2;
                for (DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex) {
                    int16_t SampleValue = ((RunningSampleIndex++ / (SquareWavePeriod / 2)) % 2) ? 1000 : -1000;
                    *SampleOut++ = SampleValue;
                    *SampleOut++ = SampleValue;
                }

                SecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
            }
        }
        win32_window_dimension Dimension = GetWindowDimension(WindowHandle);
        CopyBufferToWindow(DeviceContext, Dimension.Width, Dimension.Height, &BackBuffer, 0, 0, Dimension.Width, Dimension.Height);	
        

        ReleaseDC(WindowHandle, DeviceContext);
    }
    
    return 0;
}
