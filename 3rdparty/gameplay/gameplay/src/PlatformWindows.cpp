#ifdef WIN32

#include "Base.h"
#include "Platform.h"
#include "FileSystem.h"
#include "Game.h"
#include "Vector2.h"
#include <GL/wglew.h>
#include <windowsx.h>
#include <Commdlg.h>
#include <shellapi.h>

using gameplay::print;

// Window defaults
#define DEFAULT_RESOLUTION_X 1024
#define DEFAULT_RESOLUTION_Y 768
#define DEFAULT_COLOR_BUFFER_SIZE 32
#define DEFAULT_DEPTH_BUFFER_SIZE 24
#define DEFAULT_STENCIL_BUFFER_SIZE 8

static double __timeTicksPerMillis;
static double __timeStart;
static double __timeAbsolute;
static bool __vsync = WINDOW_VSYNC;
static HINSTANCE __hinstance = 0;
static HWND __hwnd = 0;
static HDC __hdc = 0;
static HGLRC __hrc = 0;
static bool __mouseCaptured = false;
static POINT __mouseCapturePoint = {0, 0};
static bool __multiSampling = false;
static bool __cursorVisible = true;
static unsigned int __gamepadsConnected = 0;

#ifdef GP_USE_GAMEPAD
static const unsigned int XINPUT_BUTTON_COUNT = 14;
static const unsigned int XINPUT_JOYSTICK_COUNT = 2;
static const unsigned int XINPUT_TRIGGER_COUNT = 2;

static XINPUT_STATE __xInputState;
static bool __connectedXInput[4];

static float normalizeXInputJoystickAxis(int axisValue, int deadZone)
{
    int absAxisValue = abs(axisValue);

    if (absAxisValue < deadZone)
    {
        return 0.0f;
    }
    else
    {
        int value = axisValue;
        int maxVal;
        if (value < 0)
        {
            value = -1;
            maxVal = 32768;
        }
        else if (value > 0)
        {
            value = 1;
            maxVal = 32767;
        }
        else
        {
            return 0;
        }

        return value * (absAxisValue - deadZone) / (float)(maxVal - deadZone);
    }
}
#endif

static void UpdateCapture(LPARAM lParam)
{
    if( (lParam & MK_LBUTTON) || (lParam & MK_MBUTTON) || (lParam & MK_RBUTTON) )
        SetCapture(__hwnd);
    else
        ReleaseCapture();
}


static void WarpMouse(int clientX, int clientY)
{
    POINT p = {clientX, clientY};
    ClientToScreen(__hwnd, &p);
    SetCursorPos(p.x, p.y);
}


/**
 * Gets the width and height of the screen in pixels.
 */
static void getDesktopResolution(int& width, int& height)
{
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    // Get the size of screen to the variable desktop
    GetWindowRect(hDesktop, &desktop);
    width = desktop.right;
    height = desktop.bottom;
}


LRESULT CALLBACK __WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static gameplay::Game* game = gameplay::Game::getInstance();

    if( !game->isInitialized() || hwnd != __hwnd )
    {
        // Ignore messages that are not for our game window.
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    static bool shiftDown = false;
    static bool capsOn = false;

    switch( msg )
    {
        case WM_CLOSE:
#ifdef GP_USE_MEM_LEAK_DETECTION
                DestroyWindow(__hwnd);
#else
            exit(0);
#endif
            return 0;

        case WM_DESTROY:
            gameplay::Platform::shutdownInternal();
            PostQuitMessage(0);
            return 0;

        case WM_SETFOCUS:
            break;

        case WM_KILLFOCUS:
            break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}


namespace gameplay
{
    struct WindowCreationParams
    {
        RECT rect;
        std::string windowName;
        bool fullscreen;
        bool resizable;
        int samples;
    };


    extern void print(const char* format, ...)
    {
        va_list argptr;
        va_start(argptr, format);
        int sz = vfprintf(stderr, format, argptr);
        if( sz > 0 )
        {
            char* buf = new char[sz + 1];
            vsprintf(buf, format, argptr);
            buf[sz] = 0;
            OutputDebugStringA(buf);
            SAFE_DELETE_ARRAY(buf);
        }
        va_end(argptr);
    }


    extern int strcmpnocase(const char* s1, const char* s2)
    {
        return _strcmpi(s1, s2);
    }


    Platform::Platform(Game* game)
        : _game(game)
    {
    }


    Platform::~Platform()
    {
        if( __hwnd )
        {
            DestroyWindow(__hwnd);
            __hwnd = 0;
        }
    }


    bool createWindow(WindowCreationParams* params, HWND* hwnd, HDC* hdc)
    {
        bool fullscreen = false;
        bool resizable = false;
        RECT rect = {CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT};
        std::string windowName;
        if( params )
        {
            windowName = params->windowName;
            memcpy(&rect, &params->rect, sizeof(RECT));
            fullscreen = params->fullscreen;
            resizable = params->resizable;
        }

        // Set the window style.
        DWORD style, styleEx;
        if( fullscreen )
        {
            style = WS_POPUP;
            styleEx = WS_EX_APPWINDOW;
        }
        else
        {
            if( resizable )
                style = WS_OVERLAPPEDWINDOW;
            else
                style = WS_POPUP | WS_BORDER | WS_CAPTION | WS_SYSMENU;
            styleEx = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        }
        style |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

        // Adjust the window rectangle so the client size is the requested size.
        AdjustWindowRectEx(&rect, style, FALSE, styleEx);

        // Create the native Windows window.
        *hwnd = CreateWindowEx(styleEx, "gameplay", windowName.c_str(), style, 0, 0, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, __hinstance, NULL);
        if( *hwnd == NULL )
        {
            GP_ERROR("Failed to create window.");
            return false;
        }

        // Get the drawing context.
        *hdc = GetDC(*hwnd);
        if( *hdc == NULL )
        {
            GP_ERROR("Failed to get device context.");
            return false;
        }

        // Center the window
        GetWindowRect(*hwnd, &rect);
        const int screenX = (GetSystemMetrics(SM_CXSCREEN) - rect.right) / 2;
        const int screenY = (GetSystemMetrics(SM_CYSCREEN) - rect.bottom) / 2;
        SetWindowPos(*hwnd, *hwnd, screenX, screenY, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

        return true;
    }


    bool initializeGL(WindowCreationParams* params)
    {
        // Create a temporary window and context to we can initialize GLEW and get access
        // to additional OpenGL extension functions. This is a neccessary evil since the
        // function for querying GL extensions is a GL extension itself.
        HWND hwnd = NULL;
        HDC hdc = NULL;

        if( params )
        {
            if( !createWindow(params, &hwnd, &hdc) )
                return false;
        }
        else
        {
            hwnd = __hwnd;
            hdc = __hdc;
        }

        PIXELFORMATDESCRIPTOR pfd;
        memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = DEFAULT_COLOR_BUFFER_SIZE;
        pfd.cDepthBits = DEFAULT_DEPTH_BUFFER_SIZE;
        pfd.cStencilBits = DEFAULT_STENCIL_BUFFER_SIZE;
        pfd.iLayerType = PFD_MAIN_PLANE;

        int pixelFormat = ChoosePixelFormat(hdc, &pfd);
        if( pixelFormat == 0 )
        {
            DestroyWindow(hwnd);
            GP_ERROR("Failed to choose a pixel format.");
            return false;
        }

        if( !SetPixelFormat(hdc, pixelFormat, &pfd) )
        {
            DestroyWindow(hwnd);
            GP_ERROR("Failed to set the pixel format.");
            return false;
        }

        HGLRC tempContext = wglCreateContext(hdc);
        if( !tempContext )
        {
            DestroyWindow(hwnd);
            GP_ERROR("Failed to create temporary context for initialization.");
            return false;
        }
        wglMakeCurrent(hdc, tempContext);

        // Initialize GLEW
        if( GLEW_OK != glewInit() )
        {
            wglDeleteContext(tempContext);
            DestroyWindow(hwnd);
            GP_ERROR("Failed to initialize GLEW.");
            return false;
        }

        if( wglChoosePixelFormatARB && wglCreateContextAttribsARB )
        {
            // Choose pixel format using wglChoosePixelFormatARB, which allows us to specify
            // additional attributes such as multisampling.
            //
            // Note: Keep multisampling attributes at the start of the attribute lists since code below
            // assumes they are array elements 0 through 3.
            int attribList[] = {
                WGL_SAMPLES_ARB, params ? params->samples : 0,
                WGL_SAMPLE_BUFFERS_ARB, params ? (params->samples > 0 ? 1 : 0) : 0,
                WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
                WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
                WGL_COLOR_BITS_ARB, DEFAULT_COLOR_BUFFER_SIZE,
                WGL_DEPTH_BITS_ARB, DEFAULT_DEPTH_BUFFER_SIZE,
                WGL_STENCIL_BITS_ARB, DEFAULT_STENCIL_BUFFER_SIZE,
                0
            };
            __multiSampling = params && params->samples > 0;

            UINT numFormats;
            if( !wglChoosePixelFormatARB(hdc, attribList, NULL, 1, &pixelFormat, &numFormats) || numFormats == 0 )
            {
                bool valid = false;
                if( params && params->samples > 0 )
                {
                    GP_WARN("Failed to choose pixel format with WGL_SAMPLES_ARB == %d. Attempting to fallback to lower samples setting.", params->samples);
                    while( params->samples > 0 )
                    {
                        params->samples /= 2;
                        attribList[1] = params->samples;
                        attribList[3] = params->samples > 0 ? 1 : 0;
                        if( wglChoosePixelFormatARB(hdc, attribList, NULL, 1, &pixelFormat, &numFormats) && numFormats > 0 )
                        {
                            valid = true;
                            GP_WARN("Found pixel format with WGL_SAMPLES_ARB == %d.", params->samples);
                            break;
                        }
                    }

                    __multiSampling = params->samples > 0;
                }

                if( !valid )
                {
                    wglDeleteContext(tempContext);
                    DestroyWindow(hwnd);
                    GP_ERROR("Failed to choose a pixel format.");
                    return false;
                }
            }

            // Create new/final window if needed
            if( params )
            {
                DestroyWindow(hwnd);
                hwnd = NULL;
                hdc = NULL;

                if( !createWindow(params, &__hwnd, &__hdc) )
                {
                    wglDeleteContext(tempContext);
                    return false;
                }
            }

            // Set final pixel format for window
            if( !SetPixelFormat(__hdc, pixelFormat, &pfd) )
            {
                GP_ERROR("Failed to set the pixel format: %d.", (int)GetLastError());
                return false;
            }

            // Create our new GL context
            int attribs[] =
                {
                    WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
                    WGL_CONTEXT_MINOR_VERSION_ARB, 1,
                    0
                };

            if( !(__hrc = wglCreateContextAttribsARB(__hdc, 0, attribs)) )
            {
                wglDeleteContext(tempContext);
                GP_ERROR("Failed to create OpenGL context.");
                return false;
            }

            // Delete the old/temporary context and window
            wglDeleteContext(tempContext);

            // Make the new context current
            if( !wglMakeCurrent(__hdc, __hrc) || !__hrc )
            {
                GP_ERROR("Failed to make the window current.");
                return false;
            }
        }
        else // fallback to OpenGL 2.0 if wglChoosePixelFormatARB or wglCreateContextAttribsARB is NULL.
        {
            // Context is already here, just use it.
            __hrc = tempContext;
            __hwnd = hwnd;
            __hdc = hdc;
        }

        // Vertical sync.
        if( wglSwapIntervalEXT )
        wglSwapIntervalEXT(__vsync ? 1 : 0);
        else
            __vsync = false;

        // Some old graphics cards support EXT_framebuffer_object instead of ARB_framebuffer_object.
        // Patch ARB_framebuffer_object functions to EXT_framebuffer_object ones since semantic is same.
        if( !GLEW_ARB_framebuffer_object && GLEW_EXT_framebuffer_object )
        {
            glBindFramebuffer = glBindFramebufferEXT;
            glBindRenderbuffer = glBindRenderbufferEXT;
            glBlitFramebuffer = glBlitFramebufferEXT;
            glCheckFramebufferStatus = glCheckFramebufferStatusEXT;
            glDeleteFramebuffers = glDeleteFramebuffersEXT;
            glDeleteRenderbuffers = glDeleteRenderbuffersEXT;
            glFramebufferRenderbuffer = glFramebufferRenderbufferEXT;
            glFramebufferTexture1D = glFramebufferTexture1DEXT;
            glFramebufferTexture2D = glFramebufferTexture2DEXT;
            glFramebufferTexture3D = glFramebufferTexture3DEXT;
            glFramebufferTextureLayer = glFramebufferTextureLayerEXT;
            glGenFramebuffers = glGenFramebuffersEXT;
            glGenRenderbuffers = glGenRenderbuffersEXT;
            glGenerateMipmap = glGenerateMipmapEXT;
            glGetFramebufferAttachmentParameteriv = glGetFramebufferAttachmentParameterivEXT;
            glGetRenderbufferParameteriv = glGetRenderbufferParameterivEXT;
            glIsFramebuffer = glIsFramebufferEXT;
            glIsRenderbuffer = glIsRenderbufferEXT;
            glRenderbufferStorage = glRenderbufferStorageEXT;
            glRenderbufferStorageMultisample = glRenderbufferStorageMultisampleEXT;
        }

        return true;
    }


    Platform* Platform::create(Game* game)
    {
        GP_ASSERT(game);

        FileSystem::setResourcePath("./");
        Platform* platform = new Platform(game);

        // Get the application module handle.
        __hinstance = ::GetModuleHandle(NULL);

        // Read window settings from config.
        WindowCreationParams params;
        params.fullscreen = false;
        params.resizable = false;
        params.rect.left = 0;
        params.rect.top = 0;
        params.rect.right = 0;
        params.rect.bottom = 0;
        params.samples = 0;
        if( game->getConfig() )
        {
            Properties* config = game->getConfig()->getNamespace("window", true);
            if( config )
            {
                // Read window title.
                const char* title = config->getString("title");
                if( title )
                {
                    params.windowName = title;
                }

                // Read fullscreen state.
                params.fullscreen = config->getBool("fullscreen");
                // Read resizable state.
                params.resizable = config->getBool("resizable");
                // Read multisampling state.
                params.samples = config->getInt("samples");

                // Read window rect.
                int x = config->getInt("x");
                if( x != 0 )
                    params.rect.left = x;
                int y = config->getInt("y");
                if( y != 0 )
                    params.rect.top = y;
                int width = config->getInt("width");
                int height = config->getInt("height");

                if( width == 0 && height == 0 && params.fullscreen )
                    getDesktopResolution(width, height);

                if( width != 0 )
                    params.rect.right = params.rect.left + width;
                if( height != 0 )
                    params.rect.bottom = params.rect.top + height;
            }
        }

        // If window size was not specified, set it to a default value
        if( params.rect.right == 0 )
            params.rect.right = params.rect.left + DEFAULT_RESOLUTION_X;
        if( params.rect.bottom == 0 )
            params.rect.bottom = params.rect.top + DEFAULT_RESOLUTION_Y;
        int width = params.rect.right - params.rect.left;
        int height = params.rect.bottom - params.rect.top;

        if( params.fullscreen )
        {
            // Enumerate all supposed display settings
            bool modeSupported = false;
            DWORD modeNum = 0;
            DEVMODE devMode;
            memset(&devMode, 0, sizeof(DEVMODE));
            devMode.dmSize = sizeof(DEVMODE);
            devMode.dmDriverExtra = 0;
            while( EnumDisplaySettings(NULL, modeNum++, &devMode) != 0 )
            {
                // Is mode supported?
                if( devMode.dmPelsWidth == width &&
                    devMode.dmPelsHeight == height &&
                    devMode.dmBitsPerPel == DEFAULT_COLOR_BUFFER_SIZE )
                {
                    modeSupported = true;
                    break;
                }
            }

            // If the requested mode is not supported, fall back to a safe default
            if( !modeSupported )
            {
                width = DEFAULT_RESOLUTION_X;
                height = DEFAULT_RESOLUTION_Y;
                params.rect.right = params.rect.left + width;
                params.rect.bottom = params.rect.top + height;
            }
        }

        // Register our window class.
        WNDCLASSEX wc;
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = (WNDPROC)__WndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = __hinstance;
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wc.hIconSm = NULL;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = NULL; // No brush - we are going to paint our own background
        wc.lpszMenuName = NULL; // No default menu
        wc.lpszClassName = "gameplay";

        if( !::RegisterClassEx(&wc) )
        {
            GP_ERROR("Failed to register window class.");
            goto error;
        }

        if( params.fullscreen )
        {
            DEVMODE dm;
            memset(&dm, 0, sizeof(dm));
            dm.dmSize = sizeof(dm);
            dm.dmPelsWidth = width;
            dm.dmPelsHeight = height;
            dm.dmBitsPerPel = DEFAULT_COLOR_BUFFER_SIZE;
            dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

            // Try to set selected mode and get results. NOTE: CDS_FULLSCREEN gets rid of start bar.
            if( ChangeDisplaySettings(&dm, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL )
            {
                params.fullscreen = false;
                GP_ERROR("Failed to start game in full-screen mode with resolution %dx%d.", width, height);
                goto error;
            }
        }

        if( !initializeGL(&params) )
            goto error;

        // Show the window.
        ShowWindow(__hwnd, SW_SHOW);

#ifdef GP_USE_GAMEPAD
        // Initialize XInputGamepads.
    for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
    {
        if (XInputGetState(i, &__xInputState) == NO_ERROR)
        {
            if (!__connectedXInput[i])
            {
        // Gamepad is connected.
                Platform::gamepadEventConnectedInternal(i, XINPUT_BUTTON_COUNT, XINPUT_JOYSTICK_COUNT, XINPUT_TRIGGER_COUNT, "Microsoft XBox360 Controller");
                __connectedXInput[i] = true;
            }
        }
    }
#endif

        return platform;

    error:

        exit(0);
        return NULL;
    }


    int Platform::enterMessagePump()
    {
        GP_ASSERT(_game);

        // Get the initial time.
        LARGE_INTEGER tps;
        QueryPerformanceFrequency(&tps);
        __timeTicksPerMillis = (double)(tps.QuadPart / 1000L);
        LARGE_INTEGER queryTime;
        QueryPerformanceCounter(&queryTime);
        GP_ASSERT(__timeTicksPerMillis);
        __timeStart = queryTime.QuadPart / __timeTicksPerMillis;

        SwapBuffers(__hdc);

        if( _game->getState() != Game::RUNNING )
            _game->run();

        // Enter event dispatch loop.
        MSG msg;
        while( true )
        {
            if( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);

                if( msg.message == WM_QUIT )
                {
                    gameplay::Platform::shutdownInternal();
                    return msg.wParam;
                }
            }
            else
            {
#ifdef GP_USE_GAMEPAD
                // Check for connected XInput gamepads.
            for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
            {
                if (XInputGetState(i, &__xInputState) == NO_ERROR && !__connectedXInput[i])
                {
                // Gamepad was just connected.
                    Platform::gamepadEventConnectedInternal(i, XINPUT_BUTTON_COUNT, XINPUT_JOYSTICK_COUNT, XINPUT_TRIGGER_COUNT, "Microsoft XBox360 Controller");
                    __connectedXInput[i] = true;
                }
                else if (XInputGetState(i, &__xInputState) != NO_ERROR && __connectedXInput[i])
                {
                // Gamepad was just disconnected.
                    __connectedXInput[i] = false;
                    Platform::gamepadEventDisconnectedInternal(i);
                }
            }
#endif
                _game->frame();
                SwapBuffers(__hdc);
            }

            // If we are done, then exit.
            if( _game->getState() == Game::UNINITIALIZED )
                break;
        }
        return 0;
    }


    void Platform::signalShutdown()
    {
        // nothing to do
    }


    bool Platform::canExit()
    {
        return true;
    }


    unsigned int Platform::getDisplayWidth()
    {
        static RECT rect;
        GetClientRect(__hwnd, &rect);
        return rect.right;
    }


    unsigned int Platform::getDisplayHeight()
    {
        static RECT rect;
        GetClientRect(__hwnd, &rect);
        return rect.bottom;
    }


    double Platform::getAbsoluteTime()
    {
        LARGE_INTEGER queryTime;
        QueryPerformanceCounter(&queryTime);
        GP_ASSERT(__timeTicksPerMillis);
        __timeAbsolute = queryTime.QuadPart / __timeTicksPerMillis;

        return __timeAbsolute - __timeStart;
    }


    void Platform::setAbsoluteTime(double time)
    {
        __timeAbsolute = time;
    }


    bool Platform::isVsync()
    {
        return __vsync;
    }


    void Platform::setVsync(bool enable)
    {
        __vsync = enable;

        if( wglSwapIntervalEXT )
        wglSwapIntervalEXT(__vsync ? 1 : 0);
        else
            __vsync = false;
    }


    void Platform::swapBuffers()
    {
        if( __hdc )
            SwapBuffers(__hdc);
    }


    void Platform::sleep(long ms)
    {
        Sleep(ms);
    }


    void Platform::setMultiSampling(bool enabled)
    {
        if( enabled == __multiSampling )
        {
            return;
        }

        if( enabled )
        {
            glEnable(GL_MULTISAMPLE);
        }
        else
        {
            glDisable(GL_MULTISAMPLE);
        }

        __multiSampling = enabled;
    }


    bool Platform::isMultiSampling()
    {
        return __multiSampling;
    }


    void Platform::getArguments(int* argc, char*** argv)
    {
        if( argc )
            *argc = __argc;
        if( argv )
            *argv = __argv;
    }


    void Platform::shutdownInternal()
    {
        Game::getInstance()->shutdown();
    }
}

#endif
