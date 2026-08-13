#include "swl.h"
#include "dlfcn.h"
#include <Cocoa/Cocoa.h>
#include <Foundation/Foundation.h>
#include <Quartz/Quartz.h>

typedef struct {
    NSWindow* window;
    int should_close;
    unsigned char k[256], pk[256];
    CFTimeInterval last_time;
    float dt;
} MAC_App;
static MAC_App a = {0};

@interface SWLWindowDelegate : NSObject <NSWindowDelegate>
@end
@implementation SWLWindowDelegate
- (BOOL)windowShouldClose:(NSWindow *)sender {
    a.should_close = 1;
    return YES;
}
@end

void swl_CreateWindow(const char *title, int width, int height) {
    NSApp = [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp activateIgnoringOtherApps:YES];

    a.window = [[NSWindow alloc]
        initWithContentRect:NSMakeRect(0, 0, width, height)
        styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable) 
        backing:NSBackingStoreBuffered
        defer:NO];
    [a.window setTitle:[NSString stringWithUTF8String:title]];
    [a.window center];
    static SWLWindowDelegate *delegate;
    delegate = [[SWLWindowDelegate alloc] init];
    [a.window setDelegate:delegate];

    [a.window makeKeyAndOrderFront:nil];

    a.last_time = CACurrentMediaTime();
}

void* swl_GetWindowPtr(void) {
    return (__bridge void*)a.window;
}

void swl_CloseWindow(void) {
    [a.window close];
}

void swl_SendQuitEvent(void) {
    a.should_close = 1;
}

int swl_ShouldClose(void) {
    return a.should_close;
}

void swl_PollEvents(void) {
    memcpy(a.pk, a.k, 256);

    @autoreleasepool {
        NSEvent *event;
        while ((event = [NSApp 
            nextEventMatchingMask:NSEventMaskAny
            untilDate:[NSDate distantPast]
            inMode:NSDefaultRunLoopMode
            dequeue:YES])) {
            if (event.type == NSEventTypeKeyDown) {
                if (event.keyCode < 256) a.k[event.keyCode] = 1;
            } else if (event.type == NSEventTypeKeyUp) {
                if (event.keyCode < 256) a.k[event.keyCode] = 0;
            }
            [NSApp sendEvent:event];
        }
    }

    CFTimeInterval now = CACurrentMediaTime();
    a.dt = (float)(now - a.last_time);
    a.last_time = now;
}

float swl_GetFrameTime(void) {
    return a.dt;
}

int swl_IsKeyDown(int y) {
    return a.k[y];
}

int swl_IsKeyPressed(int y) {
    return a.k[y] && !a.pk[y];
}

int swl_IsKeyReleased(int y) {
    return !a.k[y] && a.pk[y];
}

void swl_GetMousePos(int *x, int *y) {
    NSPoint p = [a.window mouseLocationOutsideOfEventStream];
    NSRect contentRect = [[a.window contentView] frame];
    *x = (int)p.x;
    *y = (int)(contentRect.size.height - p.y);
}

void  swl_PassScheduler() {
    usleep(1);
}

void *swl_LoadLibrary(const char *name) {
    return dlopen(name, RTLD_LAZY);
}

void *swl_GetFunction(void *lib, const char *symbol) {
    return dlsym(lib, symbol);
}

void swl_FreeLibrary(void *lib) {
    dlclose(lib);
}