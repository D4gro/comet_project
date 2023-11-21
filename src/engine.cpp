
#include "input.h"
#define APIENTRY
#define GL_GLEXT_PROTOTYPES
#include "glcorearb.h"
// WINDOWS
#ifdef _WIN32
#include "win32_platform.cpp"

#endif

#include "gl_renderer.cpp"

int main() {
	BumpAllocator transientStorage = make_bump_allocator(MB(50));
  platform_create_window(1280, 720, L"Comet");
	input.screenSizeX = 1280;
	input.screenSizeY = 720;

	gl_init(&transientStorage);
  while (running) {
    platform_update_window();
		gl_render();
		platform_swap_buffers();
  }
  return 0;
}