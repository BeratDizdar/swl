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
    LARGE_INTEGER f,l,c;
    float dt;
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

    QueryPerformanceFrequency(&_w.f);
    QueryPerformanceCounter(&_w.l);
}

GPUAPI void*swl_GetWindowPtr(){return (void*)_w.handler;}
GPUAPI void swl_CloseWindow(){DestroyWindow(_w.handler);}
GPUAPI void swl_SendQuitEvent(){_w.should_close=1;}

GPUAPI int swl_ShouldClose() { return _w.should_close; }

GPUAPI void swl_PollEvents() {
    for(MSG m={0};m.message!=WM_QUIT && PeekMessageW(&m,0,0,0,1)>0;){
        TranslateMessage(&m);
        DispatchMessageW(&m);
    }
    for(int i=0;i<256;i++)_w.pk[i]=_w.k[i];
    GetKeyboardState(_w.k);
    QueryPerformanceCounter(&_w.c);
    _w.dt=(float)(_w.c.QuadPart-_w.l.QuadPart)/(float)_w.f.QuadPart;
    _w.l=_w.c;
}

GPUAPI float swl_GetFrameTime() { return _w.dt; }

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

GPUAPI void* swl_LoadLibrary(const char* name) {
    return LoadLibraryA(name);
}

GPUAPI void* swl_GetFunction(void* lib, const char* func) {
    return GetProcAddress(lib, func);
}

GPUAPI void swl_FreeLibrary(void* lib) {
    FreeLibrary(lib);
}

GPUAPI void swl_GL_CreateContext(int major, int minor, int zbuf, int sbuf) {
    PIXELFORMATDESCRIPTOR pfd = { 
        sizeof(PIXELFORMATDESCRIPTOR),    // size of this pfd  
        1,                                // version number  
        PFD_DRAW_TO_WINDOW |              // support window  
        PFD_SUPPORT_OPENGL |              // support OpenGL  
        PFD_DOUBLEBUFFER,                 // double buffered  
        PFD_TYPE_RGBA,                    // RGBA type  
        24,                               // 24-bit color depth  
        0, 0, 0, 0, 0, 0,                 // color bits ignored  
        0,                                // no alpha buffer  
        0,                                // shift bit ignored  
        0,                                // no accumulation buffer  
        0, 0, 0, 0,                       // accum bits ignored  
        zbuf,                             // z-buffer      
        sbuf,                             // stencil buffer  
        0,                                // no auxiliary buffer  
        PFD_MAIN_PLANE,                   // main layer  
        0,                                // reserved  
        0, 0, 0                           // layer masks ignored  
    };
    _w.dc = GetDC(_w.handler);
    int iPixelFormat = ChoosePixelFormat(_w.dc, &pfd);
    SetPixelFormat(_w.dc, iPixelFormat, &pfd);

    HGLRC dummy_rc = wglCreateContext(_w.dc);
    wglMakeCurrent(_w.dc, dummy_rc);

    // driver zaten 4.6 sağlıyor ama işte dümenden lazım
    int is_core_profile = (major > 3) || (major == 3 && minor >= 2);
    if (!is_core_profile) {
        _w.rc = dummy_rc;
        return;
    }

    void*(*wglARBctx)(HDC, HGLRC, int*) = (void*)wglGetProcAddress("wglCreateContextAttribsARB");
    if (!wglARBctx) { _w.rc = dummy_rc; return; }
    int attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, major,
        WGL_CONTEXT_MINOR_VERSION_ARB, minor,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        #ifdef _DEBUG
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
        #else
        WGL_CONTEXT_OPENGL_NO_ERROR_ARB, 1,   // sadece release build'de
        #endif
        0 // liste sonu
    };
    
    _w.rc = wglARBctx(_w.dc, 0, attribs);
    if (_w.rc != NULL) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(dummy_rc);
        wglMakeCurrent(_w.dc, _w.rc);
    }
    else {
        _w.rc = dummy_rc;
    }
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
