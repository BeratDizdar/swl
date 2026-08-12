#include "swl.h"
#include "dlfcn.h"
#include <Cocoa/Cocoa.h>
#include <Foundation/Foundation.h>

typedef struct {
    NSWindow* window;
} MAC_App;
static MAC_App a = {0};

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
    [a.window makeKeyAndOrderFront:nil];
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