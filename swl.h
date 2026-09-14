#pragma once
#include <stdint.h>
// key codeları ayırmak lazımmış platforma özel

void     swl_CreateWindow(const char* title, int width, int height);
void*    swl_GetWindowPtr();
void     swl_CloseWindow();
void     swl_SendQuitEvent();
int      swl_ShouldClose();
void     swl_PollEvents();
uint64_t swl_GetFrameTimeUs();
double   swl_GetFrameTimeSeconds();
int      swl_IsKeyDown(int y);
int      swl_IsKeyPressed(int y);
int      swl_IsKeyReleased(int y);
void     swl_GetMousePos(int* x, int* y);
void     swl_PassScheduler(); // sleep(1ms)
void     swl_GL_CreateLegacyContext();
void     swl_GL_CreateLegacyContextSingleBuffer();
void     swl_GL_DestroyContext();
void     swl_GL_SwapBuffers();
void*    swl_GL_GetProcAddress(const char* proc);
