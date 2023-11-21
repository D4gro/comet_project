#pragma once

// GLOBALS
static bool running = true;

// PLATFORM FUNCS
bool platform_create_window(int width, int height, wchar_t *title);
void platform_update_window();
void* platform_load_gl_function(char* funName);
void platform_swap_buffers();