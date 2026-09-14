#include"swl.h"
#include<Windows.h>

#define GPUAPI __declspec(dllexport)

#define WGL_CONTEXT_MAJOR_VERSION_ARB      0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB      0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB       0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB   0x00000001
#define WGL_CONTEXT_FLAGS_ARB              0x2094
#define WGL_CONTEXT_DEBUG_BIT_ARB          0x0001
#define WGL_CONTEXT_OPENGL_NO_ERROR_ARB    0x31B3

typedef struct swl_Window {
    HGLRC rc;
    HDC dc;
    uint64_t freq;       // QPF frekansı (tick/sn)
    uint64_t last_time;  // Bir önceki frame'in QPC tick değeri
    uint64_t dt_us;      // Delta time (Mikrosaniye - µs)
    HWND handler;
    BYTE k[256], pk[256];
    int should_close;
} swl_Window;
static swl_Window _w = {0};

LRESULT CALLBACK _c(HWND h,UINT m,WPARAM w,LPARAM l){
    switch (m) {case WM_DESTROY: _w.should_close=1; PostQuitMessage(0); break;}
    return DefWindowProcA(h,m,w,l);
}

GPUAPI void swl_CreateWindow(const char* title, int width, int height){
    HINSTANCE i = GetModuleHandleA(0);
    HCURSOR c = LoadCursorA(0,(LPCSTR)IDC_ARROW);
    LPCSTR n = "w";
    int xy = CW_USEDEFAULT;
    int s = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    
    RegisterClassA(&(WNDCLASSA){ CS_OWNDC, _c, 0, 0, i, 0, c, 0, 0, n });

    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, s, 0);
    int win_w = rect.right - rect.left;
    int win_h = rect.bottom - rect.top;

    _w.handler = CreateWindowA(n, title, s, xy, xy, win_w, win_h, 0, 0, i, 0);

    LARGE_INTEGER li;
    QueryPerformanceFrequency(&li);
    _w.freq = (uint64_t)li.QuadPart;

    QueryPerformanceCounter(&li);
    _w.last_time = (uint64_t)li.QuadPart;
}

GPUAPI void*swl_GetWindowPtr(){return (void*)_w.handler;}
GPUAPI void swl_CloseWindow(){DestroyWindow(_w.handler);}
GPUAPI void swl_SendQuitEvent(){_w.should_close=1;}

GPUAPI int swl_ShouldClose() { return _w.should_close; }

GPUAPI void swl_PollEvents() {
    for (MSG m = {0}; m.message != WM_QUIT && PeekMessageW(&m, 0, 0, 0, 1) > 0;) {
        TranslateMessage(&m);
        DispatchMessageW(&m);
    }
    for (int i = 0; i < 256; i++) _w.pk[i] = _w.k[i];
    GetKeyboardState(_w.k);

    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    uint64_t current_time = (uint64_t)li.QuadPart;

    uint64_t elapsed_ticks = current_time - _w.last_time;
    _w.last_time = current_time;
    _w.dt_us = (elapsed_ticks * 1000000ULL) / _w.freq;
}

// Ham mikrosaniye (Örn: 16666)
GPUAPI uint64_t swl_GetFrameTimeUs() { 
    return _w.dt_us; 
}

// Fizik formülleri için saniye cinsinden (dt * hız)
GPUAPI double swl_GetFrameTimeSeconds() { 
    return (double)_w.dt_us / 1000000.0; 
}

GPUAPI int swl_IsKeyDown(int y){return _w.k[y]&128;}
GPUAPI int swl_IsKeyPressed(int y){return (_w.k[y]&128)&&!(_w.pk[y]&128);}
GPUAPI int swl_IsKeyReleased(int y){return !(_w.k[y]&128)&&(_w.pk[y]&128);}
GPUAPI void swl_GetMousePos(int* x, int* y){
    POINT p; 
    GetCursorPos(&p); 
    ScreenToClient(_w.handler, &p);
    *x = p.x; 
    *y = p.y;
}

GPUAPI void swl_PassScheduler() {
    Sleep(1);
}

static void _swl_GL_CreateLegacyContext(int doublebuffer) {
    // 32 color, 8 alpha, 24 depth, 8 stencil yaptın unutma

    PIXELFORMATDESCRIPTOR pfd = { 
        sizeof(PIXELFORMATDESCRIPTOR),    // size of this pfd  
        1,                                // version number  
        PFD_DRAW_TO_WINDOW |              // support window  
        PFD_SUPPORT_OPENGL |              // support OpenGL  
        (doublebuffer)?PFD_DOUBLEBUFFER:0,                 // double buffered  
        PFD_TYPE_RGBA,
        32, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 24, 8, 0,
        PFD_MAIN_PLANE, 0, 0, 0, 0
    };
    _w.dc = GetDC(_w.handler);
    int iPixelFormat = ChoosePixelFormat(_w.dc, &pfd);
    SetPixelFormat(_w.dc, iPixelFormat, &pfd);

    _w.rc = wglCreateContext(_w.dc);
    wglMakeCurrent(_w.dc, _w.rc);
}

GPUAPI void swl_GL_CreateLegacyContext() {
    _swl_GL_CreateLegacyContext(1);
}

GPUAPI void swl_GL_CreateLegacyContextSingleBuffer() {
    _swl_GL_CreateLegacyContext(0);
}

GPUAPI void swl_GL_DestroyContext() {
    if (_w.rc) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(_w.rc);
        _w.rc = NULL;
    }
}

GPUAPI void swl_GL_SwapBuffers() {
    SwapBuffers(_w.dc);
}

GPUAPI void*swl_GL_GetProcAddress(const char* proc) {
    void *p = (void*)wglGetProcAddress(proc);
    if(p == 0 ||
        (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
        (p == (void*)-1) )
    {
        HMODULE module = LoadLibraryA("opengl32.dll");
        p = (void*)GetProcAddress(module, proc);
    }
        
    return p;
}
