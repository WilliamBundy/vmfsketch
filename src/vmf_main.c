
#define _CRT_SECURE_NO_WARNINGS
#include "vmf_defines.h"

#include "wb_sorting.c"
#include "wb_math.c"
#include "wb_memory.c"

#include "wb_renderer.c"
#include "wb_nk_spritefont.c"

#include "vmf_app.c"
#include "vmf_editor.c"
#include "vmf_export.c"

Editor vmfEditor_s;
Editor* vmfEditor = &vmfEditor_s;

i32 g_is_arg = 0;
string g_arg;
void start()
{
	editor_start(vmfEditor, vmfApp.memory.frame);
	if(g_is_arg)
		editor_open_from_file(vmfEditor, g_arg);
}

void update()
{
	editor_update(vmfEditor);
}

typedef const unsigned char* glGetStringProc(unsigned int name);
int main(int argc, char** argv)
{
	if(argc > 1) {
		g_is_arg = 1;
		g_arg = argv[argc-1];
	}
	vmfSettings s = { 0 };
	s.window.size = v2i(1280, 720);
	s.window.scale = 1.0f;
	s.window.title = "VMF Sketch";
	s.window.icon_name = "assets/tiny_icon.png";

	s.archive_name = "vmfsketch.dat";
	s.desired_frame_time = 1.0f / 60.0f;
	s.frag_shader = "src/shaders/frag.glsl";
	s.vert_shader = "src/shaders/vert.glsl";
	s.texture_file = "assets/graphics.png";
	s.body_font = "assets/DejaVuSansCondensed.ttf";
	s.body_font_size = 16;
	s.fixed_font = "assets/DejaVuSansMono.ttf";
	s.fixed_font_size = 16;
	s.small_font_size = 12;
	s.vsync = true;
	


	if(init_app(s)) {
		log_error("Failed to initialize program.");
		//return 1;
	}


	{
		HMODULE gldll = LoadLibraryA("opengl32.dll");
		glGetStringProc* getString = (void*)GetProcAddress(gldll, "glGetString");
		const unsigned char* str =getString(0x1f02);
		printf("%s\n", str);
	}


	app_run();

	return 0;

}
