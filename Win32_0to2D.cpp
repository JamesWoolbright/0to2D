/*
 * Notice: (C) Copyright 2024 by James Robert Woolbright. All rights Reserved
*/

#include <Windows.h>
#include <stdint.h>
#include <dsound.h>
#include <math.h>

#define Pi32 3.14159265359f

/*
    Work that needs to be done on this layer

    - Save state location
    - Getting a handle to executable file
    - Asset Loading path
    - Multi Threading
    - Raw Input for multiple keyboards
    - Sleep so that on laptops we don't melt the cpu
    - Clipcursor() for multimonitor support
    - Fullscreen support
    - WM_SETCURSOR (control what the cursor looks like/ visablity)
    - QueryCancelAutoPlay
    - WM_ACTIVATEAPP (for when we are not the active application)
    - Blit Speed Improvements (BitBlit)
    - HardWare Acceleration (OpenGL or Direct3D maybe both?)
    - GetKeyboardLayout (for French Keyboards, international wasd support)

    Just some of the stuff. 

*/

#include "0to2D.cpp"

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

struct win32_sound_output
{
    int SamplesPerSecond;
    int hz;
    uint32_t RunningSampleIndex;
    int WavePeriod;
    int BytesPerSample;
    int BufferSize;
    float tSine;
    int LatencySampleCount;
};

void LoadFile(void) {
    // Platform specfic code to load files
    return;
}

static void
Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD ByteToLock, DWORD BytesToWrite) {
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;

    if(SUCCEEDED(SecondaryBuffer->Lock(ByteToLock, BytesToWrite, &Region1, &Region1Size, &Region2, &Region2Size, 0))) {
        int16_t *SampleOut = (int16_t *)Region1;
        DWORD Region1SampleCount = Region1Size/SoundOutput->BytesPerSample;
        for (DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex) {

            float SineValue = sinf(SoundOutput->tSine);
            int16_t SampleValue = (int16_t)(SineValue * 1000);
            *SampleOut++ = SampleValue;
            *SampleOut++ = SampleValue;

            SoundOutput->tSine += 2.0f * Pi32 * 1.0f / (float)SoundOutput->WavePeriod;
            ++SoundOutput->RunningSampleIndex;
        }
        DWORD Region2SampleCount = Region2Size/SoundOutput->BytesPerSample;
        SampleOut = (int16_t *)Region2;
        for (DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex) {
            
            float SineValue = sinf(SoundOutput->tSine);
            int16_t SampleValue = (int16_t)(SineValue * 1000);
            *SampleOut++ = SampleValue;
            *SampleOut++ = SampleValue;
            
            SoundOutput->tSine += 2.0f * Pi32 * 1.0f / (float)SoundOutput->WavePeriod;
            ++SoundOutput->RunningSampleIndex;
        }

        SecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

/*{
    win32_sound_output SoundOutput = {};
    SoundOutput.SamplesPerSecond = 48000;
    SoundOutput.hz = 256;
    SoundOutput.RunningSampleIndex = 0;
    SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond/SoundOutput.hz;
    SoundOutput.BytesPerSample = sizeof(int16_t) * 2;
    SoundOutput.BufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;
} */

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
    win32_sound_output SoundOutput = {};

    SoundOutput.SamplesPerSecond = 48000;
    SoundOutput.hz = 256;
    SoundOutput.RunningSampleIndex = 0;
    SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond/SoundOutput.hz;
    SoundOutput.BytesPerSample = sizeof(int16_t) * 2;
    SoundOutput.BufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;
    SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond / 15;

    InitDSound(WindowHandle, SoundOutput.BufferSize, SoundOutput.SamplesPerSecond); 
 
    Win32FillSoundBuffer(&SoundOutput, 0, SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample);
    SecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

    bool SoundIsPlaying = true;

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

            DWORD ByteToLock = (SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.BufferSize;
            DWORD TargetCursor = ((PlayCursor + (SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample)) % SoundOutput.BufferSize);
            DWORD BytesToWrite;
            // Need a more accurate than ByteToLock == PlayCursor because some systems may not update playcursor frequently
            if(ByteToLock > TargetCursor) {
                BytesToWrite = (SoundOutput.BufferSize - ByteToLock);
                BytesToWrite += TargetCursor;
            } else {
                BytesToWrite = TargetCursor - ByteToLock;
            }

            Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite);

        }

        MainLoop();

        win32_window_dimension Dimension = GetWindowDimension(WindowHandle);
        CopyBufferToWindow(DeviceContext, Dimension.Width, Dimension.Height, &BackBuffer, 0, 0, Dimension.Width, Dimension.Height);	
        

        ReleaseDC(WindowHandle, DeviceContext);
    }
    
    return 0;
}
