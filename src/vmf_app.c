#define MouseWheel 15

#ifndef REFLECTED
typedef enum ButtonState
{
	Button_JustReleased = -1,
	Button_Released,
	Button_Pressed,
	Button_JustPressed
} ButtonState;

struct vmfSettings
{
	struct {
		string title;
		Vec2i size;
		f32 scale;
		string icon_name;
	} window;

	f32 desired_frame_time;
	i32 vsync;

	string archive_name;
	string vert_shader;
	string frag_shader;
	string texture_file;

	string title_font;
	i32 title_font_size;

	string body_font;
	i32 body_font_size;

	string fixed_font;
	i32 fixed_font_size;
	i32 small_font_size;

};

struct vmfAppMain
{
	i32 state;
	
	char* base_path;
	SDL_Window* window;
	struct {
		MemoryArena* base;
		MemoryArena* render;
		MemoryArena* frame;
	} memory;

	mz_zip_archive assets;

	i32* keys;
	i32* mouse;

	Vec2 mousepos;

	Vec2 size;
	Vec2i window_size;
	f32 scale;

	struct {
		f32 last[8];
		i32 next;
		f32 desired;
	} frame_time;
	f32 elapsed;


	struct {
		struct nk_font* title;
		struct nk_font* body;
		struct nk_font* fixed;
		struct nk_font* small;
	} fonts;
};
#endif 

vmfAppMain vmfApp;

Renderer* vmfRenderer;
RenderGroup* vmfCurrent;

//nk_context nk_ctx;
nk_context* nk;

RenderGroup* vmfOverlay;
i32 overlay_mode = 0;
u32 splash_texture = 0, controls_texture = 0;
Rect2 splash_t, controls_t;
f32 splash_timer = 2;

void* get_asset(string asset_name, isize* size_out)
{
	usize len = 0;
	void* asset = mz_zip_reader_extract_file_to_heap(&vmfApp.assets, asset_name, &len, 0);
	if(asset == NULL) {
		log_error("Asset was null %s", asset_name);
	}
	*size_out = len;
	return asset;
}

void app_set_scale(f32 scale)
{
	vmfApp.scale = scale;
	vmfApp.size.x = vmfApp.window_size.x / scale;
	vmfApp.size.y = vmfApp.window_size.y / scale;
}


void update_mousepos()
{
	i32 mx, my;
	SDL_GetMouseState(&mx, &my);
	f32 scale = vmfApp.scale * vmfCurrent->scale;
	vmfApp.mousepos = v2(mx, my);
	vmfApp.mousepos = v2_scale(vmfApp.mousepos, 1.0f / scale);
	vmfApp.mousepos = v2_add(vmfApp.mousepos, vmfCurrent->offset);
}

i32 init_app(vmfSettings settings)
{
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		log_error("Error: failed to init SDL: %s", SDL_GetError());
		return 1;
	}

	SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
			SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute (SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

	SDL_Window* window = SDL_CreateWindow(settings.window.title, 
			SDL_WINDOWPOS_CENTERED_DISPLAY(0), 
			SDL_WINDOWPOS_CENTERED_DISPLAY(0),
			settings.window.size.x, settings.window.size.y,
			SDL_WINDOW_OPENGL | 
			SDL_WINDOW_RESIZABLE |
			SDL_WINDOW_ALLOW_HIGHDPI |
			SDL_WINDOW_MOUSE_FOCUS |
			SDL_WINDOW_INPUT_FOCUS);

	if(window == NULL) {
		log_error("Error: could not create window: %s", SDL_GetError());
		return 1;
	}
	vmfApp.window = window;

	SDL_GLContext opengl_context = SDL_GL_CreateContext(window);
	
	if(opengl_context == NULL) {
		log_error("Error: SDL_GL_CreateContext returned null");
		log_error((char*)SDL_GetError());
	}

	SDL_GL_MakeCurrent(window, opengl_context);

	if(!gladLoadGL()) {
		log_error("%d %d", GLVersion.major, GLVersion.minor);
		log_error("Error: could not load OpenGL functions (glad)");
		return 1;
	}
	if(settings.vsync) {
		i32 error = SDL_GL_SetSwapInterval(-1);
		if(error == -1) {
			log_error("Couldn't get late swap tearing, trying vsync...");
			error = SDL_GL_SetSwapInterval(1);
			if(error == -1) {
				log_error("Couldn't get vsync.");
				error = SDL_GL_SetSwapInterval(0);
			}
		} else {
			//log_error("Using late-swap tearing");
		}
	}

	vmfApp.frame_time.desired = settings.desired_frame_time;

	vmfApp.scale = 1.0f;
	vmfApp.memory.base = arena_bootstrap("BaseArena", Megabytes(256));
	vmfApp.memory.render = arena_bootstrap("RenderArena", Megabytes(512));
	vmfApp.memory.frame = arena_bootstrap("FrameArena", Megabytes(256));

	vmfApp.keys = arena_push(vmfApp.memory.base, sizeof(i32) * SDL_NUM_SCANCODES);
	vmfApp.mouse = arena_push(vmfApp.memory.base, sizeof(i32) * 16);


	vmfApp.base_path = SDL_GetBasePath();
	char* vertex_src;
	char* frag_src;

	{
		mz_zip_archive zip = {0};
		char fn_buf[4096];
		snprintf(fn_buf, 4096, "%s%s", vmfApp.base_path, settings.archive_name);

		i32 result = mz_zip_reader_init_file(&zip, fn_buf, MZ_ZIP_FLAG_COMPRESSED_DATA);


		if(!result) {
			printf("%s\n", mz_zip_get_error_string(zip.m_last_error));
			log_error("Could not open archive %s. Please locate program archive and try again", settings.archive_name);
			return 1;
		}

		usize len = 0;

		vertex_src = mz_zip_reader_extract_file_to_heap(&zip, settings.vert_shader, &len, 0);
		vertex_src[len-1] = '\0';

		len = 0;
		frag_src = mz_zip_reader_extract_file_to_heap(&zip, settings.frag_shader, &len, 0);
		frag_src[len - 1] = '\0';
		vmfApp.assets = zip;
	}

	vmfRenderer = arena_push(vmfApp.memory.base, sizeof(Renderer));
	renderer_init(vmfRenderer, vertex_src, frag_src, 512, vmfApp.memory.render);
	{
		u32 texture;
		i32 w, h;
		isize newlen;
		u8* raw_image = mz_zip_reader_extract_file_to_heap(&vmfApp.assets, settings.texture_file, &newlen, 0);
		texture = ogl_load_texture_from_memory(raw_image, newlen, &w, &h);
		free(raw_image);
		render_set_texture(vmfRenderer, texture, w, h);
	}

	{
		i32 w, h;
		isize newlen;
		u8* raw_image = mz_zip_reader_extract_file_to_heap(&vmfApp.assets, "assets/splash.png",  &newlen, 0);
		splash_texture = ogl_load_texture_from_memory(raw_image, newlen, &w, &h);
		splash_t = rect2(0, 0, w, h);
		free(raw_image);
	}

	{
		i32 w, h;
		isize newlen;
		u8* raw_image = mz_zip_reader_extract_file_to_heap(&vmfApp.assets, "assets/controls.png", &newlen, 0);
		controls_texture = ogl_load_texture_from_memory(raw_image, newlen, &w, &h);
		controls_t = rect2(0, 0, w, h);
		free(raw_image);
	}


	{
		i32 w, h, n;
		isize newlen;
		u8* raw_image = mz_zip_reader_extract_file_to_heap(&vmfApp.assets, settings.window.icon_name, &newlen, 0);
		u8* data = stbi_load_from_memory(raw_image, newlen, &w, &h, &n, STBI_rgb_alpha);
		printf("%d\n", n);
		SDL_Surface* icon = SDL_CreateRGBSurfaceFrom(data, w, h, n * 8, n * w, 0xFF, 0xFF00, 0xFF0000, 0xFF000000);
		SDL_SetWindowIcon(vmfApp.window, icon);
		SDL_FreeSurface(icon);
		STBI_FREE(data);
		free(raw_image);
	}
	
	vmfCurrent = render_add_group(vmfRenderer, Group_RetainedStream);
	vmfOverlay = render_add_group(vmfRenderer, Group_Immediate);
	vmfOverlay->texture = splash_texture;
	vmfOverlay->texture_width = splash_t.size.x;
	vmfOverlay->texture_height = splash_t.size.y;
	vmfOverlay->inv_texture_size = v2(1/splash_t.size.x, 1/splash_t.size.y);
	
	i32 ttt = 0;
	nk = nk_sdl_init(window);
	{
		struct nk_font_atlas* atlas;
		nk_sdl_font_stash_begin(&atlas);

		string font_name;
		i32 font_size;

#if 0
		font_name = settings.title_font;
		font_size = settings.title_font_size;
 		{
			struct nk_font_config f = nk_font_config(font_size);
			f.oversample_h = 8;
			f.oversample_v = 8;
			isize size;
			u8* font_data = get_asset(font_name, &size);
			vmfApp.fonts.title = nk_font_atlas_add_from_memory(atlas, (void*)font_data, size, font_size, &f);
		}

#endif
		font_name = settings.body_font;
		font_size = settings.body_font_size;
 		{
			struct nk_font_config f = nk_font_config(font_size);
			f.oversample_h = 8;
			f.oversample_v = 8;
			isize size;
			u8* font_data = get_asset(font_name, &size);
			vmfApp.fonts.body = nk_font_atlas_add_from_memory(atlas, (void*)font_data, size, font_size, &f);
		}

#if 1
		font_name = settings.fixed_font;
		font_size = settings.fixed_font_size;
 		{
			struct nk_font_config f = nk_font_config(font_size);
			f.oversample_h = 8;
			f.oversample_v = 8;
			isize size;
			u8* font_data = get_asset(font_name, &size);
			vmfApp.fonts.fixed = nk_font_atlas_add_from_memory(atlas, (void*)font_data, size, font_size, &f);
			vmfApp.fonts.small = nk_font_atlas_add_from_memory(atlas, 
					(void*)font_data, size, settings.small_font_size, &f);
		}
#endif

		nk_sdl_font_stash_end();
		nk_style_set_font(nk, &vmfApp.fonts.body->handle);
	}

	return 0;
}


void app_run()
{

	i32 running = 1;
	SDL_Event event;

	glClearColor(0, 0, 0, 1);

	i64 perf_freq = platform_get_perf_freq();
	vmfApp.frame_time.next = 0;
	vmfApp.elapsed = vmfApp.frame_time.desired; 
	for(isize i = 0; i < 8; ++i) vmfApp.frame_time.last[i] = vmfApp.frame_time.desired;
	i64 start_cycles = 0;
	i64 elapsed_cycles = 0;
	f32 elapsed_time = 0;

	start();

	while(running) {
		start_cycles = platform_get_perf_counter();

		for(isize i = 0; i < SDL_NUM_SCANCODES; ++i) {
			i32* t = vmfApp.keys + i;
			if(*t == Button_JustPressed) {
				*t = Button_Pressed;
			} else if(*t == Button_JustReleased) {
				*t = Button_Released;
			}
		}

		for(isize i = 0; i < 16; ++i) {
			i32* t = vmfApp.mouse + i;
			if(*t == Button_JustPressed) {
				*t = Button_Pressed;
			} else if(*t == Button_JustReleased) {
				*t = Button_Released;
			}
		}

		vmfApp.mouse[MouseWheel] = 0; 
		nk_input_begin(nk);
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_WINDOWEVENT:
					break;
				case SDL_QUIT:
					running = 0;
					break;
				case SDL_KEYDOWN:
					if(!event.key.repeat) {
						vmfApp.keys[event.key.keysym.scancode] = Button_JustPressed;
					}
					break;
				case SDL_KEYUP:
					if(!event.key.repeat) {
						vmfApp.keys[event.key.keysym.scancode] = Button_JustReleased;
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					vmfApp.mouse[event.button.button] = Button_JustPressed;
					break;
				case SDL_MOUSEBUTTONUP:
					vmfApp.mouse[event.button.button] = Button_JustReleased;
					break;
				case SDL_MOUSEWHEEL:
					vmfApp.mouse[MouseWheel] = event.wheel.y;
					break;
			}
			nk_sdl_handle_event(&event);
		}
		nk_input_end(nk);
		glClear(GL_COLOR_BUFFER_BIT);

		SDL_GetWindowSize(vmfApp.window, &vmfApp.window_size.x, &vmfApp.window_size.y);
		glViewport(0, 0, vmfApp.window_size.x, vmfApp.window_size.y);
		vmfApp.size = v2(vmfApp.window_size.x / vmfApp.scale, vmfApp.window_size.y / vmfApp.scale);

		update();
	

		if(vmfApp.keys[SDL_SCANCODE_F1] >= Button_Pressed) {
			splash_timer = 0.5;
			overlay_mode = 1;
			vmfOverlay->texture = controls_texture;
			vmfOverlay->texture_width = controls_t.size.x;
			vmfOverlay->texture_height = controls_t.size.y;
			vmfOverlay->inv_texture_size = v2(1/controls_t.size.x, 1/controls_t.size.y);
		}

		if(overlay_mode == 0) {
			if(splash_timer > 0) {
				Sprite* s = render_create_sprite(vmfOverlay, v2_scale(vmfApp.size, 0.5), splash_t);
				v2_scale_ip(&s->size, 0.75f);
				splash_timer -= vmfApp.elapsed;
				s->color.w = splash_timer / 2;
			}
		} else {
			if(splash_timer > 0) {
				Sprite* s = render_create_sprite(vmfOverlay, v2_scale(vmfApp.size, 0.5), controls_t);
				splash_timer -= vmfApp.elapsed;
				s->color.w = splash_timer / 0.5;
			}
		}
		
		nk_sdl_render(NK_ANTI_ALIASING_ON, Megabytes(2), Megabytes(1));

		render_draw(vmfRenderer, vmfOverlay, vmfApp.size, vmfApp.scale);

		SDL_GL_SwapWindow(vmfApp.window);
		elapsed_cycles = platform_get_perf_counter() - start_cycles;
		//We convert to microseconds first to preserve accuracy.
		elapsed_cycles *= 1000000;
		elapsed_cycles /= perf_freq;
		//Convert to seconds
		elapsed_time = elapsed_cycles / 1000000.0f;

		//Time sampling.
		f32 total_time = 0;//vmfApp.frame_time.desired;
		total_time += elapsed_time;
		for(isize i = 0; i < 8; ++i) {
			total_time += vmfApp.frame_time.last[i];
		}

		f32 avg_time = total_time / 9.0f;
		if(avg_time > vmfApp.frame_time.desired * 10) {
			avg_time = vmfApp.frame_time.desired * 10;
		}

		vmfApp.frame_time.last[vmfApp.frame_time.next] = elapsed_time;
		vmfApp.frame_time.next++;
		vmfApp.frame_time.next %= 8;
		vmfApp.elapsed = avg_time;
	}
}


