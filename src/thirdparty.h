/* Copyright (C) William Bundy - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 *
 *
 * Much of this work is in the public domain or under the unlicense.
 *
 * TODO(will): ship licenses as appropriate with app, or rip out glad ;)
 * 	- khrplatform.h -- MIT
 */


#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "thirdparty/stb_image.h"

#include "thirdparty/khrplatform.h"
#include "thirdparty/glad.h"
#include "thirdparty/glad.c"

#include "thirdparty/miniz.h"
#include "thirdparty/miniz.c"


#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_BUTTON_TRIGGER_ON_RELEASE
#define NK_IMPLEMENTATION
#define NK_SDL_GL3_IMPLEMENTATION
#include "thirdparty/nuklear.h"
#include "thirdparty/nuklear_sdl_gl3.h"

typedef struct nk_buffer nk_buffer;
typedef struct nk_allocator nk_allocator;
typedef struct nk_command_buffer nk_command_buffer;
typedef struct nk_draw_command nk_draw_command;
typedef struct nk_convert_config nk_convert_config;
typedef struct nk_style_item nk_style_item;
typedef struct nk_text_edit nk_text_edit;
typedef struct nk_draw_list nk_draw_list;
typedef struct nk_user_font nk_user_font;
typedef struct nk_font nk_font;
typedef struct nk_panel nk_panel;
typedef struct nk_context nk_context;
typedef struct nk_draw_vertex_layout_element nk_draw_vertex_layout_element;
typedef struct nk_style_button nk_style_button;
typedef struct nk_style_toggle nk_style_toggle;
typedef struct nk_style_selectable nk_style_selectable;
typedef struct nk_style_slide nk_style_slide;
typedef struct nk_style_progress nk_style_progress;
typedef struct nk_style_scrollbar nk_style_scrollbar;
typedef struct nk_style_edit nk_style_edit;
typedef struct nk_style_property nk_style_property;
typedef struct nk_style_chart nk_style_chart;
typedef struct nk_style_combo nk_style_combo;
typedef struct nk_style_tab nk_style_tab;
typedef struct nk_style_window_header nk_style_window_header;
typedef struct nk_style_window nk_style_window;

typedef struct nk_color nk_color;
typedef struct nk_colorf nk_colorf;
typedef struct nk_cursor nk_cursor;
typedef struct nk_scroll nk_scroll;
