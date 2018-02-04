/* ===========================================================================
 *	Todo:
 *		- Layers panel
 *	Plans:
 *    - File menu 
 *    - Undo
 *    - Cutting Tool
 *    - Drawing tool
 *    - Layer organization panel
 *    - Layer stuff in general
 *    - Properties panel
 *    - VMF Tools?
 *    	- Build a tree of elements
 *    	- VMF importer, try to make auto layers?
 *    	-
 *
 * Thoughts:
 * 		At some point I'm going to want to modularize the tools and program flow for
 * 		panels/windows, modes, actions, tools, and data in general. I think the path for 
 * 		modularazing modes/tools/actions is pretty straightforward.
 * 			- Actions are basically an invertible procedure call that modify editor state
 * 			- Tools define how the mouse works: what actions it performs, how click/drag work
 * 			- Modes define what tools and actions are available
 * 				- The editor listens to keypresses and invokes actions based on
 * 				  the current mode
 * 				- Editing basics, such as selecting objects, can be customized by 
 * 				  the current handles used by the mode
 * 			- Tools and modes aren't very distinct, especially because implemented modes only
 * 			  have one tool at their disposal. I'm thinking that, say if there's a "layers" mode, 
 * 			  there could be multiple tools involved, such as <rotate>, <move rotation anchor> etc
 * 			  	- Better example for tools: create/move/resize in edit mode! Which tool you're using
 * 			  	  depends on where you click
 * 			  	- I think the correct design is something like "currentMode->onClickEmpty(currentMode, editor)"
 * 			  	  which then invokes "editor->startTool(editor, editor->tool)" depending on what happens
 * 			- There's several things that should probably be reserved and always active, such as
 * 			  WASD for movement, brackets for grid, Z for undo.
 * 			- I want to avoid duplicating code, since that's how messes happen
 *
 *
 */   

#ifndef REFLECTED
typedef enum EditorAction
{
	Action_SelectOnlyBrush,
	Action_SelectAddBrush,
	Action_SelectGroup,
	Action_DeselectBrush,
	Action_DeselectAll,

	Action_CopySelection,
	Action_CutSelection,
	Action_Paste,

	Action_CreateBrush,
	Action_MoveSelected,
	Action_ResizeBrush,
	Action_RotateSelected
} EditorAction;

typedef enum EditorFocus
{
	Focus_None,
	Focus_Editor,
	Focus_Panel
} EditorFocus;

typedef enum EditorMode
{
	Mode_Null=-1,
	Mode_Normal=0,
	Mode_Edit,
	Mode_Pan,
	Mode_Vertex,
	Mode_Cut,
} EditorMode;


struct EditorKeys
{
	struct {
		i32 bigger, smaller;
	} grid;

	struct {
		i32 up, down, left, right;
		i32 center, grid;
	} pan;

	struct {
		i32 back, edit, pan, vert, cut, layer;
	} mode;

	struct {
		i32 undo, cut, copy, paste;
		i32 delete;
		i32 rotate, flip; 
		i32 layer;
	} action;
}

struct EditorModeData
{
	struct {
		Vec2 start;
		Vec2i start_cell;
		i32 down;
	} normal;
	
	struct {
		Vec2 mousepos;
		i32 down;
		f32 speed;
		f32 shiftmod;
	} pan;

	struct {
		Vec2 start;
		Sprite* resizing;
		f32 mouse_offset;
		Vec2i start_cell;
		i32 active_handle;
		i32 create_down;
		i32 resize_down;
		i32 move_down;
	} edit;

	struct {
		i32 low_size_hover;
		i32 low_side_down;
		i32 down;
		i32 handle;
		i32 cindex;
		Vec2 start;
		Vec2 offset;
		Vec2 uv;
	} vert;

};


struct EditorBrush
{
	Vec2 pos;
	Vec2 size;
};

struct EditorLayer
{
	char name[64];
	i32 name_len;
	Vec3 pos;
	f32 rotation;
	f32 depth;
	f32 rise;
	i32 id;
	Vec4 color;

	RenderGroup* brushes;
};

struct Editor
{
	struct {
		RenderGroup* grid;
		RenderGroup* overlay;
		RenderGroup* layers;
		RenderGroup* itext;
		RenderGroup* stext;

		i32 layer_count;
	} groups;

	Rect2 screen_bounds;
	Vec2 camera;
	i32 grid_size;
	f32 zoom;
	f32 zoom_z;

	Vec2[16] anchors;
	i32 anchor_count;

	Sprite** selected;
	Vec2 selected_center;
	Vec2 selected_size;
	i32 selected_count;

	Sprite* clipboard;
	Vec2 clipboard_center;
	i32 clipboard_count;

	i32 input_block;
	i32 mode;
	i32 last_mode;
	EditorModeData modes;
	i32 focus;

	struct {
		RenderGroup* bg;
		RenderGroup* grid;
		RenderGroup* layers;
		RenderGroup* overlay;

		Rect2 bounds;
		i32 open;
		f32 height;

		Vec2 camera;
		f32 zoom;

	} layer_menu; 

	EditorLayer* current;
	EditorLayer* layers;
	i32 layer_count, layer_capacity;

	i32 confirm;
	i32 reset_windows_waiting;

	EditorKeys keys;
};

#endif

void editor_layer_init(Editor* ed, EditorLayer* l)
{
	l->name_len = snprintf(l->name, 64, "Layer %d", l->id);

	l->pos = v3(0, 0, 0);
	l->rotation = 0;
	l->depth = 64;
	l->brushes = render_add_group(vmfRenderer, Group_RetainedDynamic);
	l->brushes->tint.w = 0.5f;
	l->color = v4(1, 1, 1, 1);
}


char save_filename[4096];
i32 save_filename_len;
char export_filename[4096];
i32 export_filename_len;

#define EditorSelectedMax 2048
void editor_start(Editor* ed, MemoryArena* arena)
{
	ed->selected = arena_push(arena, sizeof(Sprite*) * EditorSelectedMax);
	ed->selected_count = 0;

	ed->clipboard = arena_push(arena, sizeof(Sprite*) * EditorSelectedMax);
	ed->clipboard_count = 0;

	ed->zoom_z = 2;
	ed->zoom = 0.5f;
	ed->focus = 1;

	ed->modes.pan.shiftmod = 2.0f;
	ed->modes.pan.speed = 16.0f;

	ed->groups.grid = render_add_group(vmfRenderer, Group_Immediate);
	ed->groups.grid->always_dirty = 1;
	ed->groups.grid->draw_mode = GL_LINE_LOOP;

	ed->groups.overlay = render_add_group(vmfRenderer, Group_Immediate);
	ed->groups.stext = render_add_group(vmfRenderer, Group_Immediate);

	//bound gets set first-thing in the update function
	ed->layer_menu.bounds = rect2(0, 0, 1280, 0);
	ed->layer_menu.open = 0;
	ed->layer_menu.height = 512;
	ed->layer_menu.zoom = 1;
	ed->layer_menu.camera = v2(0, 0);
	ed->layer_menu.bg = render_add_group(vmfRenderer, Group_Immediate);
	ed->layer_menu.grid = render_add_group(vmfRenderer, Group_Immediate);
	ed->layer_menu.layers = render_add_group(vmfRenderer, Group_RetainedDynamic);
	ed->layer_menu.overlay = render_add_group(vmfRenderer, Group_Immediate);

	editor_change_grid_size(ed, 64);

	//TODO(will): load config from file
	//TODO(will): Use rewrite the keys thing to support sdlk instead
	EditorKeys* keys = &ed->keys;
	keys->pan.up = SDL_SCANCODE_W;
	keys->pan.down = SDL_SCANCODE_S;
	keys->pan.left = SDL_SCANCODE_A;
	keys->pan.right = SDL_SCANCODE_D;

	keys->pan.center = SDL_SCANCODE_F;
	keys->pan.grid = SDL_SCANCODE_G;

	keys->mode.back = SDL_SCANCODE_Q;
	keys->mode.edit = SDL_SCANCODE_E;
	keys->mode.pan = SDL_SCANCODE_SPACE;
	keys->mode.vert = SDL_SCANCODE_T;
	keys->mode.layer = SDL_SCANCODE_TAB;

	keys->action.undo = SDL_SCANCODE_Z;
	keys->action.cut = SDL_SCANCODE_X;
	keys->action.copy = SDL_SCANCODE_C;
	keys->action.paste = SDL_SCANCODE_V;
	keys->action.delete = SDL_SCANCODE_DELETE;

	keys->action.rotate = SDL_SCANCODE_R;
	keys->action.flip = SDL_SCANCODE_F;

	keys->action.layer = SDL_SCANCODE_G;

	keys->grid.bigger = SDL_SCANCODE_2;//SDL_SCANCODE_RIGHTBRACKET;
	keys->grid.smaller = SDL_SCANCODE_1;//SDL_SCANCODE_LEFTBRACKET;

	ed->layers = arena_push(arena, sizeof(EditorLayer) * 2048);
	ed->layer_capacity = 2048;
	ed->layer_count = 1;
	ed->current = ed->layers;
	editor_layer_init(ed, ed->current);


	save_filename_len = sizeof("untitled.mapsketch") - 1;
	memcpy(save_filename, "untitled.mapsketch", sizeof("untitled.mapsketch"));
	memcpy(export_filename, "sketch.vmf", sizeof("sketch.vmf"));
	export_filename_len = sizeof("sketch.vmf") - 1;

}

#include "vmf_saveload.c"
#include "vmf_editor_helper.c"

void editor_update(Editor* ed)
{
	//
	//
	// Nuklear 
	//
	//
	nk->style.window.padding = nk_vec2(4, 8);
	nk->style.window.spacing = nk_vec2(0, 8);
	nk->style.button.border = 0;
	nk->style.button.rounding = 0;
	//nk_color c = {255, 255, 255, 255};
	//nk->style.text.color = c;
	//
	ed->input_block = 0;
	if(nk_begin(nk, "Save/Open/Export", nk_rect(84 + 16, 16, 256, 256), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE | NK_WINDOW_SCALABLE)) {
		if(ed->reset_windows_waiting) {
			nk_window_set_position(nk, nk_vec2(84 + 16, 16));
		}
		nk_layout_row_dynamic(nk, 25, 4);
		if(nk_button_label(nk, "Save")) {
			save_filename[save_filename_len] = '\0';
			editor_save_to_file(ed, save_filename);
		}

		if(nk_button_label(nk, "Open")) {
			save_filename[save_filename_len] = '\0';
			editor_open_from_file(ed, save_filename);
		}

		if(nk_button_label(nk, "Add")) {
			save_filename[save_filename_len] = '\0';
			editor_add_from_file(ed, save_filename);
		}

		if(nk_button_label(nk, "Export")) {
			export_filename[export_filename_len] = '\0';
			VmfContext ctx;
			vmfctx_init(&ctx, export_filename);
			vmfctx_start(&ctx, ed->layers, ed->layer_count);
			vmfctx_add_world_geom(&ctx, ed->layers, ed->layer_count);
			vmfctx_end(&ctx);
		}

		nk_layout_row_dynamic(nk, 25, 1);
		nk_label(nk, "Save File", NK_TEXT_LEFT);
		nk_edit_string(nk, NK_EDIT_SIMPLE, save_filename, &save_filename_len, 4096, nk_filter_default);
		editor_clear_focus(ed);

		nk_label(nk, "Export File", NK_TEXT_LEFT);
		nk_edit_string(nk, NK_EDIT_SIMPLE, export_filename, &export_filename_len, 4096, nk_filter_default);

		editor_clear_focus(ed);
	}
	nk_end(nk);


	if(nk_begin(nk, "Layers", nk_rect(vmfApp.size.x - 256 - 16, 16, 256, 384), 
				NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE
				| NK_WINDOW_SCALABLE)) {
		if(ed->reset_windows_waiting) {
			nk_window_set_position(nk, nk_vec2(vmfApp.size.x - 256 - 16, 16));
		}
		nk_layout_row_dynamic(nk, 25, 2);
		if(nk_button_label(nk, "Add Layer")) {
			ed->confirm = 0;
			ed->current = editor_get_layer(ed);
			editor_clear_selected(ed);
		}

		if(nk_button_label(nk, ed->confirm ? "Confirm?" : "Delete Layer")) {
			if(!ed->confirm) {
				ed->confirm = 1;
			} else if(ed->layer_count > 1) {
				ed->confirm = 0;
				i32 id = ed->current->id;
				id--;
				if(id < 0) id = ed->layer_count - 1;
				editor_remove_layer(ed, ed->current);
				ed->current = ed->layers + id;
			}
		}

		if(nk_button_label(nk, "Add Above")) {
			ed->confirm = 0;
			EditorLayer* l = editor_get_layer(ed);
			editor_clear_selected(ed);
			l->pos.z = ed->current->pos.z + ed->current->rise;
			ed->current = l;
		}

		if(nk_button_label(nk, "Add Below")) {
			ed->confirm = 0;
			EditorLayer* l = editor_get_layer(ed);
			editor_clear_selected(ed);
			l->pos.z = ed->current->pos.z - ed->current->depth;
			ed->current = l;
		}

		if(nk_button_label(nk, "Move Up")) {
			i32 id = ed->current->id;
			id--;
			if(id < 0) id = ed->layer_count - 1;
			id %= ed->layer_count;
			EditorLayer temp = *ed->current;
			*ed->current = ed->layers[id];
			ed->layers[id] = temp;
			ed->current = ed->layers + id;

			editor_assign_layer_ids(ed);
		}

		if(nk_button_label(nk, "Move Down")) {
			i32 id = ed->current->id;
			id++;
			if(id < 0) id = ed->layer_count - 1;
			id %= ed->layer_count;
			EditorLayer temp = *ed->current;
			*ed->current = ed->layers[id];
			ed->layers[id] = temp;
			ed->current = ed->layers + id;

			editor_assign_layer_ids(ed);
		}


		nk_layout_row_dynamic(nk, 25, 1);
		i32 layer_id = ed->current->id;
		for(isize i = 0; i < ed->layer_count; ++i) {
			EditorLayer* l = ed->layers + i;
			l->name[63] = '\0';
			char buf[256];
			snprintf(buf, 256, "%s (Z:%.0f, Rise:%.0f)", l->name, l->pos.z, l->rise);
			layer_id = nk_option_label(nk, buf, l->id == layer_id) ? l->id : layer_id;
		}
		if(layer_id != ed->current->id) {
			editor_clear_selected(ed);
		}
		ed->current = ed->layers + layer_id;

	}
	nk_end(nk);

	if(nk_begin(nk, "Layer Properties", nk_rect(vmfApp.size.x - 256 - 16, vmfApp.size.y - 16 - 256, 256, 256), 
				NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE
				| NK_WINDOW_SCALABLE)) {

		if(ed->reset_windows_waiting) {
			nk_window_set_position(nk, nk_vec2(vmfApp.size.x - 256 - 16, vmfApp.size.y - 256 - 16));
		}
		nk_layout_row_dynamic(nk, 25, 2);
		nk_label(nk, "Name", NK_TEXT_LEFT);
		nk_edit_string(nk, NK_EDIT_SIMPLE, ed->current->name, &ed->current->name_len, 64, nk_filter_default);
		ed->current->name[ed->current->name_len] = '\0';
		editor_clear_focus(ed);


		nk_layout_row_dynamic(nk, 25, 1);
		char buf[256];
		snprintf(buf, 256, "Position: %.0f %.0f %.0f", ed->current->pos.x, 
				ed->current->pos.y, ed->current->pos.z);
		i32 gs = ed->grid_size;
		i32 gs8 = gs / 8;
		if(nk_combo_begin_label(nk, buf, nk_vec2(256, 256))) {
			nk_layout_row_dynamic(nk, 25, 1);
			nk_property_float(nk, "#X:", -(1<<14), &ed->current->pos.x, 1<<14, gs, gs8);
			nk_property_float(nk, "#Y:", -(1<<14), &ed->current->pos.y, 1<<14, gs, gs8);
			nk_property_float(nk, "#Z:", -(1<<14), &ed->current->pos.z, 1<<14, gs, gs8);
			nk_combo_end(nk);
		}

		nk_property_float(nk, "#Depth:", 0, &ed->current->depth, 4096, gs, gs8);
		nk_property_float(nk, "#Rise:", 0, &ed->current->rise, 4096, gs, gs8);
		if(nk_button_label(nk, "Reset Position and Size")) {
			ed->current->pos = v3(0, 0, 0);
			ed->current->depth = 64;
			ed->current->rise = 0;
		}

		snprintf(buf, 256, "Color: %.2f %.2f %.2f %.2f", ed->current->color.x, 
				ed->current->color.y, ed->current->color.z, ed->current->color.w);
		if(nk_combo_begin_label(nk, buf, nk_vec2(256, 256))) {
			nk_layout_row_dynamic(nk, 25, 1);
			nk_property_float(nk, "#R:", 0, &ed->current->color.x, 1, 0.1, 1/256.0);
			nk_property_float(nk, "#G:", 0, &ed->current->color.y, 1, 0.1, 1/256.0);
			nk_property_float(nk, "#B:", 0, &ed->current->color.z, 1, 0.1, 1/256.0);
			nk_property_float(nk, "#A:", 0, &ed->current->color.w, 1, 0.1, 1/256.0);
			nk_combo_end(nk);
		}


	}
	nk_end(nk);
	ed->reset_windows_waiting = 0;

	if(nk_begin(nk, "Menu", nk_rect(-2, 0, 86, vmfApp.size.y), NK_WINDOW_NO_SCROLLBAR)) {
		nk_layout_row_dynamic(nk, 24, 1);

		struct nk_style_button style = nk->style.button;
		//Normal button
		if(ModeIs(Normal)) {
			style.normal = style.active;
		} else {
			style.normal = nk->style.button.normal;
		}
		if(nk_button_label_styled(nk, &style, "Normal")) {
			ed->last_mode = Mode_Normal;
			ed->mode = Mode_Normal;
		}

		//Pan
		if(ModeIs(Pan)) {
			style.normal = style.active;
		} else {
			style.normal = nk->style.button.normal;
		}
		if(nk_button_label_styled(nk, &style, "Pan")) {
		}

		//Edit button
		if(ModeIs(Edit)) {
			style.normal = style.active;
		} else {
			style.normal = nk->style.button.normal;
		}
		if(nk_button_label_styled(nk, &style, "Edit")) {
			ed->last_mode = Mode_Normal;
			ed->mode = Mode_Edit;
		}

		//Vert button
		if(ModeIs(Vertex)) {
			style.normal = style.active;
		} else {
			style.normal = nk->style.button.normal;
		}
		if(nk_button_label_styled(nk, &style, "Vertex")) {
			ed->last_mode = Mode_Normal;
			ed->mode = Mode_Vertex;
		}

#if 0
		//Cut button
		if(ModeIs(Cut)) {
			style.normal = style.active;
		} else {
			style.normal = nk->style.button.normal;
		}
		if(nk_button_label_styled(nk, &style, "Cut")) {
			printf("Pressed button 2!\n");
		}
#endif



		if(nk_button_label(nk, "Recenter")) {
			ed->reset_windows_waiting = 1;

			ed->camera = v2(0, 0);
			ed->zoom = 0.5f;
			ed->zoom_z = 2;
			ed->grid_size = 64;
		}

		nk_layout_row_dynamic(nk, 8, 1);
		nk->style.window.spacing = nk_vec2(0, 2);
		nk_style_push_font(nk, &vmfApp.fonts.small->handle);
		nk_labelf_wrap(nk, "X %.0f", ed->camera.x);
		nk_labelf_wrap(nk, "Y %.0f", ed->camera.y);
		nk_labelf_wrap(nk, "Layer %d", ed->current->id);
		nk_labelf_wrap(nk, "Grid %dhu", ed->grid_size);
		nk_labelf_wrap(nk, "Scale %.2f", ed->zoom);
		nk_labelf_wrap(nk, "Mode %d", ed->mode);
		nk_labelf_wrap(nk, "dt %.4f", vmfApp.elapsed * 1000);
		nk_style_pop_font(nk);

	}
	nk_end(nk);

	ed->input_block |= nk_window_is_any_hovered(nk);// | nk_item_is_any_active(nk);
	if(!ed->input_block && vmfApp.mouse[MouseLeft] >= Button_Pressed) {
		ed->confirm = 0;
	}

	//
	//
	// Mode Select
	//
	//

	//TODO(will): add a mode_finished for each mode, call it when modes change
	// currently we're small-scale, so it's more-or-less fine, but at some point
	// it might be good to do Shawn style codegen for modes and actions.
	if(EditorFocusKeyL(mode.edit)) {
		ed->mode = Mode_Edit;
	} else if(EditorFocusKeyL(mode.vert)) {
		ed->mode = Mode_Vertex;
	} else if(EditorFocusKeyR(ed->keys.mode.back, Button_JustPressed)) {
		ed->mode = Mode_Normal;
	}


	//
	//
	// Ui controls
	//
	//

	update_mousepos();


	// TODO(will): This stuff needs to be sent to the panel that 
	// has focus, rather than the global editor

	if(EditorFocusKeyL(grid.bigger)) {
		ed->grid_size *= 2;
	}
	if(EditorFocusKeyL(grid.smaller)) {
		ed->grid_size /= 2;
	}

	if(ed->grid_size < 16) {
		ed->grid_size = 16;
	} else if(ed->grid_size > 512) {
		ed->grid_size = 512;
	}
	editor_change_grid_size(ed, ed->grid_size);
	if(!ed->input_block && vmfApp.mouse[MouseWheel]) {
		f32 zoom_factor = expf(vmfApp.mouse[MouseWheel] * 0.1f);
		ed->zoom *= zoom_factor;
		if(ed->zoom > 2) ed->zoom = 2;
		else if(ed->zoom < 1.0/32.0) ed->zoom = 1.0/32.0;
		ed->zoom_z = 1.0/ed->zoom;
	} 

	//mode selection

	if(EditorFocusKeyL(action.layer)) {
		i32 id = ed->current->id;
		if(vmfApp.keys[SDL_SCANCODE_LSHIFT] >= Button_Pressed) {
			id--;
			//go down a layer
		} else {
			id++;
			//go up a layer
		}
		if(id < 0) id = ed->layer_count - 1;
		id %= ed->layer_count;
		
		if(AnySelected && id != ed->current->id) {
			EditorLayer* next = ed->layers + id;
			for(isize i = 0; i < ed->selected_count; ++i) {
				Sprite* sel = ed->selected[i];
				Sprite* dst = render_get_sprite(next->brushes);
				*dst = *sel;
				render_remove_sprite(ed->current->brushes, sel);
			}

			editor_clear_selected(ed);
		}

		ed->current = ed->layers + id;
	}


	if(vmfApp.keys[ed->keys.mode.layer] == Button_JustPressed) {
		ed->layer_menu.open = !ed->layer_menu.open;
	}

	f32 layer_menu_height = 0;
	if(ed->layer_menu.open) {
		layer_menu_height = ed->layer_menu.height;
	}

	ed->screen_bounds = rect2(84, 0, vmfApp.size.x - 84, vmfApp.size.y - layer_menu_height);
	ed->layer_menu.bounds = rect2(84, vmfApp.size.y - layer_menu_height, vmfApp.size.x - 84, 
			layer_menu_height);
	EditorModeData* modes = &ed->modes;

	{

		Vec2 v = v2(0, 0);
		f32 move_speed = ed->modes.pan.speed;

		if(EditorFocusKeyPressed(pan.up)) {
			v.y -= move_speed;
		}

		if(EditorFocusKeyPressed(pan.down)) {
			v.y += move_speed;
		}

		if(EditorFocusKeyPressed(pan.left)) {
			v.x -= move_speed;
		}
		if(EditorFocusKeyPressed(pan.right)) {
			v.x += move_speed;
		}

		if(vmfApp.keys[SDL_SCANCODE_LSHIFT] >= Button_Pressed) {
			v2_scale_ip(&v, ed->modes.pan.shiftmod);
		}

		if(fabsf(v.x * v.y) > 0.001) {
			v.x *= Math_InvSqrt2;
			v.y *= Math_InvSqrt2;
		}
		v2_add_ip(&ed->camera, v2_scale(v, 1.0/ed->zoom));

		if(EditorFocusKey(SPACE, Button_JustPressed)) {
			ed->last_mode = ed->mode;
			ed->mode = Mode_Pan;
		}
	}

	if(ModeIs(Pan)) {
		vmfCurrent = get_group(vmfRenderer, 0);
		update_mousepos();
		if(EditorJustClicked(MouseLeft)) {
			modes->pan.down = 1;
			modes->pan.mousepos = vmfApp.mousepos;
		} else if(vmfApp.mouse[MouseLeft] == Button_JustReleased){
			modes->pan.down = 0;
			modes->pan.mousepos = v2(0, 0);
		}
		i32 ctmns = (rect_contains_point(ed->screen_bounds, vmfApp.mousepos));
		if(modes->pan.down && vmfApp.mouse[MouseLeft] == Button_Pressed) {
			Vec2 diff = v2_sub(vmfApp.mousepos, modes->pan.mousepos);
			v2_sub_ip(&ed->camera, v2_scale(diff, 1.0f/ed->zoom));
			modes->pan.mousepos = vmfApp.mousepos;
		}

		if(vmfApp.keys[SDL_SCANCODE_SPACE] <= Button_Released)  {
			ed->mode = ed->last_mode;
		}

		if(EditorFocusKeyL(pan.center)) {
			ed->camera = v2(0, 0);
			ed->zoom = 0.5f;
			ed->zoom_z = 2;
		}

		if(EditorFocusKeyL(pan.grid)) {
			ed->grid_size = 64;
		}

	} else {
		modes->pan.mousepos = v2(0, 0);
		modes->pan.down = 0;
	}

	Vec2 final_camera = v2_sub(ed->camera, v2_scale(vmfApp.size, 0.5 * 1.0/ed->zoom));
	ed->groups.grid->offset = final_camera;
	ed->groups.grid->scale = ed->zoom;
	render_draw(vmfRenderer, ed->groups.grid, vmfApp.size, vmfApp.scale);


	vmfCurrent = get_group(vmfRenderer, 0);
	update_mousepos();
	Vec2 screen_mousepos = vmfApp.mousepos;
	vmfCurrent = ed->groups.grid;
	update_mousepos();
	Vec2 local_mousepos = vmfApp.mousepos;

	if(!ModeIs(Pan)) {
		Sprite* s;
		if(EditorJustClickedM(MouseLeft, screen_mousepos)) {
			if(s = render_group_query(ed->current->brushes, vmfApp.mousepos)) {
				i32 block = 0;
				i32 hit_handle = 0;

				if(EditorFocusKey(LCTRL, Button_Pressed)) {
					block = 1;
				}

				if(ModeIs(Vertex)) {
					if(ed->selected_count >= 1) {
						hit_handle = editor_check_vertex_handles(ed, ed->selected[0]);
					}
				}

				if(ModeIs(Edit)) {
					for(isize i = 0; i < ed->selected_count; ++i) {
						hit_handle = editor_check_resize_handles(ed, ed->selected[i]);
						if(hit_handle) break ;
					}
				}

				if(!hit_handle) {
					if(EditorFocusKey(LCTRL, Button_Pressed)) {
						block = 1;
						if(editor_brush_is_selected(ed, s)) {
							editor_deselect(ed, s);
						} else {
							editor_select(ed, s);
						}
					} else {
						if(!editor_brush_is_selected(ed, s)) {
							editor_select_only(ed, s);
						}
					}
				}

				if(!hit_handle && ModeIs(Edit)) {
					hit_handle = editor_check_resize_handles(ed, s);

					if(!hit_handle && !block) {
						editor_calc_selected_center(ed);
						modes->edit.start = v2_sub(ed->selected_center, local_mousepos);
						modes->edit.move_down = 1;
					} 
				} 
				editor_calc_selected_center(ed);
			} else if(AnySelected) {
				{
					i32 hit_handle = 0;
					if(ModeIs(Edit)) {
						for(isize i = 0; i < ed->selected_count; ++i) {
							Sprite* t = ed->selected[i];
							hit_handle = editor_check_resize_handles(ed, t);
							if(hit_handle) {
								break;
							}
						}
					} else if(ModeIs(Vertex)) {
						hit_handle = editor_check_vertex_handles(ed, ed->selected[0]);
					}

					if(!hit_handle)  {
						ed->input_block = 1;
						editor_clear_selected(ed);


					}
				}
			}
			if(ModeIs(Normal)) {
				modes->normal.start = local_mousepos;
				modes->normal.down = 1;
			}
		}

		if(ModeIs(Normal) && modes->normal.down) {
			if(vmfApp.mouse[MouseLeft] >= Button_Pressed) {
				render_create_primitive(ed->groups.overlay,
						modes->normal.start, v2_sub(local_mousepos, modes->normal.start), 
						v4(1, 0.5, 0, 0.5), Anchor_TopLeft);
			} else {
				//select everything in region
				Vec2 end = local_mousepos;
				Vec2 start = modes->normal.start;
				ed->selected_count = render_group_query_aabb(ed->current->brushes, aabb_from_extents(start, end), EditorSelectedMax, ed->selected);

				modes->normal.down = 0;
				modes->normal.start = v2(0, 0);
			}
		}


		if(!ModeIs(Vertex)) {
			for(isize i = 0; i < ed->selected_count; ++i) {
				s = ed->selected[i];
				Rect2 aabb = sprite_aabb(s);
				if(ModeIs(Edit)) {
					if(modes->edit.move_down) {
						Sprite* t = render_get_sprite(ed->groups.overlay);
						*t = *s;
						Vec2 hs = v2_scale(aabb.size, 0.5f);
						Vec2 tl = v2_sub(aabb.pos, hs);
						v2_sub_ip(&tl, ed->selected_center);
						v2_add_ip(&tl, editor_round_to_grid(ed, v2_add(modes->edit.start, local_mousepos)));
						v2_add_ip(&tl, hs);
						t->pos = tl;

						editor_draw_pos_size_text(ed, t->pos, t->size);

						t->color.w = 0.5f;
						t->color.x = 0.9;
						Sprite* v = render_get_sprite(ed->groups.overlay);
						*v = *t;
						v->size = v2_scale(v2(8, 8), 1.0f/ed->zoom);
						v->color.w = 1;

						s->color.w = 0.1f;
					} else {
						s->color.w = 0.75f;
					}
				}

				Sprite* selected_box = render_create_primitive(ed->groups.overlay, aabb.pos, aabb.size, v4(1, 0.5, 0, 0.25), 0);
				if(ModeIs(Vertex)) {
					selected_box->color.w = 0.1;
				}
				if(ModeIs(Edit)) { //resize controls
					Rect2 handles[4];
					editor_get_resize_handles(ed, s, handles);
					Vec4 control_color = v4(0.5, 1, 0.2, 0.5);
					for(isize i = 0; i < 4; ++i) {
						Rect2 handle = handles[i];
						Vec4 c = control_color;
						if(s == modes->edit.resizing) {
							selected_box->color.w = 0.75;
							if(i == modes->edit.active_handle) {
								c = v4(1, 1, 1, 1);
								//Interpret mouse pointer
								f32 mouse_diff = 0;
								switch(modes->edit.active_handle) {
									case 0: //top
									case 1: //bottom
										mouse_diff = local_mousepos.y + modes->edit.mouse_offset;
										handle.pos.y = mouse_diff;
										break;
									case 2: //left
									case 3: //right
										mouse_diff = local_mousepos.x + modes->edit.mouse_offset;
										handle.pos.x = mouse_diff;
										break;
								}

								//recalculate AABB based on active handle's position
								Vec2 extent = v2(0, 0); 
								Vec2 tl = v2_sub(aabb.pos, v2_scale(aabb.size, 0.5));
								Vec2 br = v2_add(aabb.pos, v2_scale(aabb.size, 0.5));
								switch(modes->edit.active_handle) {
									case 0: //top
										extent.y = handle.pos.y;
										extent = editor_round_to_grid(ed, extent);
										tl.y = extent.y;
										break;
									case 1: //bottom
										extent.y = handle.pos.y + handle.size.y;
										extent = editor_round_to_grid(ed, extent);
										br.y = extent.y;
										break;
									case 2: //left
										extent.x = handle.pos.x;
										extent = editor_round_to_grid(ed, extent);
										tl.x = extent.x;
										break;
									case 3: //right
										extent.x = handle.pos.x + handle.size.x;
										extent = editor_round_to_grid(ed, extent);
										br.x = extent.x;
										break;
								}

								aabb = aabb_from_extents(tl, br);

							}
						} else if(rect_contains_point(handle, local_mousepos)) {
							c = v4(0.8, 1, 0.4, 1);
						}
						render_create_primitive(ed->groups.overlay,
								handle.pos, handle.size,
								c, Anchor_TopLeft);
					}
					selected_box->pos = aabb.pos;
					selected_box->size = aabb.size;
				}
				editor_draw_pos_size_text(ed, aabb.pos, aabb.size);
				render_create_primitive(ed->groups.overlay, aabb.pos, v2_scale(v2(8, 8), 1.0f/ed->zoom), v4(1, 1, 1, 1), 0);
			}
		}
	}
	
	/*========================================================================
	 * Vertex Mode
	 *======================================================================*/

	if(ModeIs(Vertex)) {
		if(AnySelected) {
			i32 clicked = EditorJustClickedM(MouseLeft, screen_mousepos);
			ed->selected_count = 1;
			Sprite* s = ed->selected[0];

			if(!modes->vert.down) {
				Sprite* t = render_get_sprite(ed->groups.overlay);
				*t = *s;
				t->color = v4(1, 0.5, 0, 0.1);
			}

			Rect2 handles[4];
			editor_get_vertex_handles(ed, s, handles);
			for(isize i = 0; i < 4; ++i) {
				Rect2 handle = handles[i];
				i32 mouse_over = rect_contains_point(handle, local_mousepos);
				Vec4 color = v4(1, 0.2, 0.2, 0.75);
				if(mouse_over) {
					color.w = 1;
					if(clicked) {
						modes->vert.down = 1;
						modes->vert.handle = i;
						modes->vert.start = local_mousepos;
						modes->vert.offset = v2_sub(v2_add_scaled(
									handle.pos, handle.size, 0.5f),
								local_mousepos);
						modes->vert.uv = v2(1, 1);
					}
				}

				Sprite* t = render_create_primitive(ed->groups.overlay, 
						handle.pos, handle.size, 
						color, Anchor_TopLeft);
			}
			
			{ //draw center handle
				Rect2 handle = editor_get_low_side_handle(ed, s);
				Vec4 color = v4(0.2, 0.2, 1, 0.75);
				i32 mouse_over = rect_contains_point(handle, local_mousepos);
				if(!modes->vert.down && mouse_over) {
					color.w = 1;
					if(clicked) {
						modes->vert.low_side_down = 1;
					}
				}
				Sprite* h = render_create_primitive(ed->groups.overlay, 
						handle.pos, handle.size, 
						color, Anchor_TopLeft);
			}

			if(!modes->vert.down && modes->vert.low_side_down && vmfApp.mouse[MouseLeft] >= Button_Pressed) {
				ed->selected[0]->low_side = v2_sub(local_mousepos, sprite_get_centroid(ed->selected[0]));
			} else {
				modes->vert.low_side_down = 0;
			}

			if(!modes->vert.low_side_down && modes->vert.down && vmfApp.mouse[MouseLeft] >= Button_Pressed) {
				//get ready
				Vec2 end = v2_add(local_mousepos, modes->vert.offset);
				end = editor_round_to_grid(ed, end);
				Vec4 color = v4(1, 0.2, 0.2, 0.75);


				Sprite* t = render_create_primitive(ed->groups.overlay, 
						end, v2_scale(v2(32, 32), 1.0/ed->zoom), 
						color, Anchor_Center);

				v2_sub_ip(&end, s->pos);
				i32 cindex;
				Vec2 hs = v2_scale(s->size, 0.5);
				Vec2 tl = v2_scale(hs, -1);
				Vec2 br = hs;
				Vec2 corner;
				switch(modes->vert.handle) {
					case 0: //tl
						cindex = 0;
						corner = tl;
						break;
					case 1: //tr
						cindex = 1;
						corner = v2(br.x, tl.y);
						break;
					case 2: //br
						cindex = 3;
						corner = br;
						break;
					case 3: //bl
						cindex = 2;
						corner = v2(tl.x, br.y);
						break;
				}
				Vec2 uv = v2(end.x / corner.x, end.y / corner.y);
				//if(uv.x < 0) uv.x = 0;
				//if(uv.y < 0) uv.y = 0;
				modes->vert.uv = uv;
				modes->vert.cindex = cindex;
				{
					Sprite* t = render_get_sprite(ed->groups.overlay);
					*t = *s;
					t->corners[cindex] = modes->vert.uv;
					t->color = v4(1, 0.5, 0, 0.5);
				}
			} else if(!modes->vert.low_side_down && modes->vert.down && vmfApp.mouse[MouseLeft] <= Button_Released) {
				s->corners[modes->vert.cindex] = modes->vert.uv;
				sprite_validate(s);
				ed->current->brushes->dirty = 1;
				modes->vert.down = 0; 
			}
		}
	}

	/*========================================================================
	 * Edit Mode
	 *======================================================================*/

	if(ModeIs(Edit)) {
		if(!AnySelected && EditorJustClickedM(MouseLeft, screen_mousepos)) {
			modes->edit.create_down = 1;
			modes->edit.start = editor_floor_to_grid(ed, local_mousepos);
			Vec2 start = modes->edit.start;
			modes->edit.start_cell = v2i(start.x / ed->grid_size, start.y / ed->grid_size);
		}

		if(modes->edit.move_down) {
			ed->current->brushes->dirty = 1;
			if(vmfApp.mouse[MouseLeft] <= Button_Released) {
				for(isize i = 0; i < ed->selected_count; ++i) {
					Sprite* t = ed->selected[i];
					Rect2 aabb = sprite_aabb(t);
					Vec2 hs = v2_scale(aabb.size, 0.5f);
					Vec2 tl = v2_sub(aabb.pos, hs);
					v2_sub_ip(&tl, ed->selected_center);
					v2_add_ip(&tl, editor_round_to_grid(ed, v2_add(modes->edit.start, local_mousepos)));
					v2_add_ip(&tl, hs);
					if(vmfApp.keys[SDL_SCANCODE_LSHIFT] >= Button_Pressed) {
						Sprite* s = render_get_sprite(ed->current->brushes);
						*s = *t;
						s->pos = tl;
						s->color.w = 0.75;
						editor_select(ed, s);
						editor_deselect(ed, t);
					} else {
						t->pos = tl;
					}

					t->color.w = 0.75;
				}
				ed->selected_center = v2(0, 0);
				for(isize i = 0; i < ed->selected_count; ++i) {
					Sprite* t = ed->selected[i];
					v2_add_ip(&ed->selected_center, v2_scale(t->pos, 1.0f/ed->selected_count));
				}
				ed->selected_center = editor_floor_to_grid(ed, ed->selected_center);
				EditModeEnd;
			} 
		} else if(EditorFocusKeyL(action.paste)) {
			editor_paste_clipboard(ed);
		} else if(modes->edit.resize_down) {
			if(vmfApp.mouse[MouseLeft] <= Button_Released) {
				Sprite* s = modes->edit.resizing;
				Rect2 handles[4];
				editor_get_resize_handles(ed, s, handles);
				Rect2 handle = handles[modes->edit.active_handle];
				Rect2 aabb = sprite_aabb(s);
				f32 mouse_diff = 0;
				switch(modes->edit.active_handle) {
					case 0: //top
					case 1: //bottom
						mouse_diff = local_mousepos.y + modes->edit.mouse_offset;
						handle.pos.y = mouse_diff;
						break;
					case 2: //left
					case 3: //right
						mouse_diff = local_mousepos.x + modes->edit.mouse_offset;
						handle.pos.x = mouse_diff;
						break;
				}

				//recalculate AABB based on active handle's position
				Vec2 extent = v2(0, 0); 
				switch(modes->edit.active_handle) {
					case 0: //top
						extent.y = handle.pos.y;
						break;
					case 1: //bottom
						extent.y = handle.pos.y + handle.size.y;
						break;
					case 2: //left
						extent.x = handle.pos.x;
						break;
					case 3: //right
						extent.x = handle.pos.x + handle.size.x;
						break;
				}
				extent = editor_round_to_grid(ed, extent);

				//okay, now we know the new side
				//we need to recalculate the new size
				//and then recenter the aabb wrt that 
				Vec2 tl = v2_sub(aabb.pos, v2_scale(aabb.size, 0.5));
				Vec2 br = v2_add(aabb.pos, v2_scale(aabb.size, 0.5));
				switch(modes->edit.active_handle) {
					case 0: //top
						tl.y = extent.y;
						break;
					case 1: //bottom
						br.y = extent.y;
						break;
					case 2: //left
						tl.x = extent.x;
						break;
					case 3: //right
						br.x = extent.x;
						break;
				}
				aabb = aabb_from_extents(tl, br);
				s->pos = aabb.pos;
				s->size = aabb.size;
				ed->current->brushes->dirty = 1;

				EditModeEnd;
			}
		}

		if(AnySelected) {
			if(EditorFocusKeyL(action.delete)) {
				for(isize i = 0; i < ed->selected_count; ++i) {
					render_remove_sprite(ed->current->brushes, ed->selected[i]);
				}
				editor_clear_selected(ed);
			} else if(EditorFocusKeyL(action.cut)) {
				if(vmfApp.keys[SDL_SCANCODE_LSHIFT] < Button_Pressed) {
					editor_copy_selected_to_clipboard(ed);
				}
				for(isize i = 0; i < ed->selected_count; ++i) {
					render_remove_sprite(ed->current->brushes, ed->selected[i]);
				}
				editor_clear_selected(ed);
			} else if(EditorFocusKeyL(action.copy)) {
				editor_copy_selected_to_clipboard(ed);
			} else if(EditorFocusKeyL(action.rotate)) {
				if(EditorFocusKey(LSHIFT, Button_Pressed)) {
					if(ed->selected_count > 1) {
						editor_calc_selected_center(ed);
						for(isize i = 0; i < ed->selected_count; ++i) {
							Sprite* s = ed->selected[i];
							v2_sub_ip(&s->pos, ed->selected_center);
							v2_rotate_ip(&s->pos, v2(0, -1));
							v2_add_ip(&s->pos, ed->selected_center);
							sprite_rotate_ccw90(s);
						}
					} else {
						sprite_rotate_ccw90(ed->selected[0]);
					}
				} else {
					if(ed->selected_count > 1) {
						editor_calc_selected_center(ed);
						for(isize i = 0; i < ed->selected_count; ++i) {
							Sprite* s = ed->selected[i];
							v2_sub_ip(&s->pos, ed->selected_center);
							v2_rotate_ip(&s->pos, v2(0, -1));
							v2_add_ip(&s->pos, ed->selected_center);
							sprite_rotate_cw90(s);
						}
					} else {
						sprite_rotate_cw90(ed->selected[0]);
					}
				}
				ed->current->brushes->dirty = 1;
			} else if(EditorFocusKeyL(action.flip)) {
				//TODO(will): there's a bug, flipping things moves the center?
				if(EditorFocusKey(LSHIFT, Button_Pressed)) {
					//vertical flip
					if(ed->selected_count > 1) {
						editor_calc_selected_center(ed);
						for(isize i = 0; i < ed->selected_count; ++i) {
							Sprite* s = ed->selected[i];

							v2_sub_ip(&s->pos, ed->selected_center);
							s->pos.y *= -1;
							v2_add_ip(&s->pos, ed->selected_center);

							sprite_flip_vert(s);
						}
					} else {
						sprite_flip_vert(ed->selected[0]);
					}
				} else {
					if(ed->selected_count > 1) {
						editor_calc_selected_center(ed);
						for(isize i = 0; i < ed->selected_count; ++i) {
							Sprite* s = ed->selected[i];

							v2_sub_ip(&s->pos, ed->selected_center);
							s->pos.x *= -1;
							v2_add_ip(&s->pos, ed->selected_center);

							sprite_flip_horiz(s);
						}
					} else {
						sprite_flip_horiz(ed->selected[0]);
					}
				}
				ed->current->brushes->dirty = 1;

			}

		} else if(modes->edit.create_down) {
			if(vmfApp.mouse[MouseLeft] >= Button_Pressed) {
				Vec2 grid = v2_scale(v2(ed->grid_size, ed->grid_size), 0.5f);;
				Vec2 start = v2_scale(v2(modes->edit.start_cell.x, modes->edit.start_cell.y), ed->grid_size);
				v2_add_ip(&start, grid);
				//start is the top left corner
				//cases: mouse is
				//	- pointing br (+, +)
				//	- pointing bl (-, +)
				//	- pointing tr (+, -)
				//	- pointing tl (-, -)
				//We always want to include the cell we started dragging from 
				//in the brush.
				Vec2 diff = v2_sub(local_mousepos, start);
				i32 mode = 0;
				if(diff.x > 0) {
					mode |= 1;
				}
				if(diff.y > 0) {
					mode |= 2;
				}

				switch(mode) {
					case 0: // 0b00 tl
						v2_add_ip(&start, grid);
						break;
					case 1: // 0b01 tr
						v2_add_ip(&start, v2(-grid.x, grid.y));
						break;
					case 2: // 0b10 bl
						v2_add_ip(&start, v2(grid.x, -grid.y));
						break;
					case 3: // 0b11 br
						v2_sub_ip(&start, grid);
						break;
				}
				modes->edit.start = start;
			}

			if(vmfApp.mouse[MouseLeft] == Button_JustReleased) {
				Vec2 end = editor_round_to_grid(ed, local_mousepos);
				Vec2 start = modes->edit.start;
				Vec2 p1 = v2(Min(end.x, start.x), Min(end.y, start.y));
				Vec2 p2 = v2(Max(end.x, start.x), Max(end.y, start.y));
				Rect2 area = rect2_v(p1, v2_sub(p2, p1));

				if(fabsf(area.size.x * area.size.y) > 0.0001) {
					Sprite* s = render_get_sprite(ed->current->brushes);
					s->flags = SpriteFlag_Primitive;
					s->pos = v2_add(area.pos, v2_scale(area.size, 0.5f));
					s->size = area.size;
					s->color = v4(1, 1, 1, 0.75);
				}

				EditModeEnd;
			}

			if(vmfApp.mouse[MouseLeft] >= Button_Pressed) {
				Vec2 end = editor_round_to_grid(ed, local_mousepos);
				Vec2 start = modes->edit.start;

				editor_draw_pos_size_text(ed, v2_scale(v2_add(start, end), 0.5f), v2_abs(v2_sub(end, start)));

				render_create_primitive(ed->groups.overlay, 
						start, 
						v2_sub(end, start), 
						v4(1, 1, 1, 0.75), 
						Anchor_TopLeft);
			} else {
				EditModeEnd;
			}
		}
		if(vmfApp.mouse[MouseRight] == Button_JustPressed) {
			EditModeEnd;
		}
	} else {
		EditModeEnd;
	}

	for(isize i = 0; i <= ed->current->brushes->last_filled; ++i) {
		Sprite* s = ed->current->brushes->sprites + i;
		if(HasFlag(s->flags, SpriteFlag_Null)) continue;

		if(s->low_side.x != 0 || s->low_side.y != 0) {
			Rect2 handle = editor_get_low_side_handle(ed, s);
			Sprite* h = render_create_primitive(ed->groups.overlay, 
					handle.pos, handle.size, 
					v4(0.5, 0.1, 1, 0.5), Anchor_TopLeft);
			Sprite* q = render_get_sprite(ed->groups.overlay);
			*q = sprite_make_line(sprite_get_centroid(s), v2_add_scaled(h->pos, handle.size, 0.5f), h->color, 4.0f/ed->zoom, SpriteFlag_Primitive);
			Vec2 start, end;
			if(sprite_get_low_side(s, &start, &end)) {
				Sprite* u = render_get_sprite(ed->groups.overlay);
				Vec4 c = h->color;
				c.w = 0.75;
				*u = sprite_make_line(start, end, c, 8.0f/ed->zoom, SpriteFlag_Primitive);
				
			}
		}
	}

	ed->current->brushes->offset = final_camera;
	ed->current->brushes->scale = ed->zoom;
	ed->current->brushes->tint = ed->current->color;
	ed->current->brushes->tint.w *= 0.5;
	ed->current->brushes->draw_mode = GL_TRIANGLE_STRIP;
	render_draw(vmfRenderer, ed->current->brushes, vmfApp.size, vmfApp.scale);
	ed->current->brushes->tint = ed->current->color;
	ed->current->brushes->draw_mode = GL_LINE_LOOP;
	render_draw(vmfRenderer, ed->current->brushes, vmfApp.size, vmfApp.scale);

	for(isize i = 0; i < ed->layer_count; ++i) {
		EditorLayer* l = ed->layers + i;
		l->brushes->offset = v2_add(final_camera, v2(l->pos.x, l->pos.y));
		l->brushes->scale = ed->zoom;
		l->brushes->tint = l->color;
		l->brushes->tint.w *= 0.25;
		l->brushes->draw_mode = GL_TRIANGLE_STRIP;
		render_draw(vmfRenderer, l->brushes, vmfApp.size, vmfApp.scale);

	}

	ed->groups.overlay->offset = final_camera;
	ed->groups.overlay->scale = ed->zoom;
	//ed->groups.overlay->tint.w = 1.0;
	ed->groups.overlay->draw_mode = GL_TRIANGLE_STRIP;
	render_draw(vmfRenderer, ed->groups.overlay, vmfApp.size, vmfApp.scale);
	//	render_clear(ed->groups.overlay);
	ed->groups.stext->offset = v2(0, 0);
	ed->groups.stext->scale = 1;
	render_draw(vmfRenderer, ed->groups.stext, vmfApp.size, vmfApp.scale);


}



#if 0
void editor_update(Editor* ed)
{
	if(EditorFocusKeyL(grid.bigger)) {
		ed->grid_size *= 2;
	}
	if(EditorFocusKeyL(grid.smaller)){
		ed->grid_size /= 2;
	}

	if(ed->grid_size < 16) {
		ed->grid_size = 16;
	} else if(ed->grid_size > 512) {
		ed->grid_size = 512;
	}
	editor_change_grid_size(ed, ed->grid_size);
	if(vmfApp.mouse[MouseWheel]) {
		f32 zoom_factor = expf(vmfApp.mouse[MouseWheel] * 0.1f);
		ed->zoom *= zoom_factor;
		if(ed->zoom > 2) ed->zoom = 2;
		else if(ed->zoom < 1.0/32.0) ed->zoom = 1.0/32.0;
		ed->zoom_z = 1.0/ed->zoom;
	} 

	ed->input_block = 0;
	//mode selection

	if(vmfApp.keys[ed->keys.mode.layer] == Button_JustPressed) {
		ed->layer_menu.open = !ed->layer_menu.open;
	}

	f32 layer_menu_height = 0;
	if(ed->layer_menu.open) {
		layer_menu_height = ed->layer_menu.height;
		printf("%.0f\n", layer_menu_height);
	}


	ed->screen_bounds = rect2(84, 0, vmfApp.size.x - 84, vmfApp.size.y - layer_menu_height);
	ed->layer_menu.bounds = rect2(84, vmfApp.size.y - layer_menu_height, vmfApp.size.x - 84, 
			layer_menu_height);
	EditorModeData* modes = &ed->modes;


	/*========================================================================
	 * Layer Menu
	 *======================================================================*/
	if(ed->layer_menu.open) {
		render_create_primitive(ed->layer_menu.bg, 
				ed->layer_menu.bounds.pos, ed->layer_menu.bounds.size,
				v4(0.25, 0.25, 0.25, 1), Anchor_TopLeft);
	}


	/*========================================================================
	 * Mode Select (mostly)
	 *======================================================================*/

	//TODO(will): add a mode_finished for each mode, call it when modes change
	// currently we're small-scale, so it's more-or-less fine, but at some point
	// it might be good to do Shawn style codegen for modes and actions.
	if(EditorFocusKeyL(mode.edit)) {
		ed->mode = Mode_Edit;
	} else if(EditorFocusKeyL(mode.vert)) {
		ed->mode = Mode_Vertex;
	} else if(EditorFocusKeyR(ed->keys.mode.back, Button_JustPressed)) {
		ed->mode = Mode_Normal;
	}

	/*========================================================================
	 * Panning
	 *======================================================================*/
	{
		Vec2 v = v2(0, 0);
		f32 move_speed = ed->modes.pan.speed;

		if(vmfApp.keys[ed->keys.pan.up] >= Button_Pressed) {
			v.y -= move_speed;
		}

		if(vmfApp.keys[ed->keys.pan.down] >= Button_Pressed) {
			v.y += move_speed;
		}

		if(vmfApp.keys[ed->keys.pan.left] >= Button_Pressed) {
			v.x -= move_speed;
		}
		if(vmfApp.keys[ed->keys.pan.right] >= Button_Pressed) {
			v.x += move_speed;
		}

		if(vmfApp.keys[SDL_SCANCODE_LSHIFT] >= Button_Pressed) {
			v2_scale_ip(&v, ed->modes.pan.shiftmod);
		}

		if(fabsf(v.x * v.y) > 0.001) {
			v.x *= Math_InvSqrt2;
			v.y *= Math_InvSqrt2;
		}
		v2_add_ip(&ed->camera, v2_scale(v, 1.0/ed->zoom));

		if(EditorFocusKey(SPACE, Button_JustPressed)) {
			ed->last_mode = ed->mode;
			ed->mode = Mode_Pan;
		}
	}

	if(ModeIs(Pan)) {
		vmfCurrent = get_group(vmfRenderer, 0);
		update_mousepos();
		if(EditorJustClicked(MouseLeft)) {
			modes->pan.down = 1;
			modes->pan.mousepos = vmfApp.mousepos;
		} else if(vmfApp.mouse[MouseLeft] == Button_JustReleased){
			modes->pan.down = 0;
			modes->pan.mousepos = v2(0, 0);
		}
		i32 ctmns = (rect_contains_point(ed->screen_bounds, vmfApp.mousepos));
		if(modes->pan.down && vmfApp.mouse[MouseLeft] == Button_Pressed) {
			Vec2 diff = v2_sub(vmfApp.mousepos, modes->pan.mousepos);
			v2_sub_ip(&ed->camera, v2_scale(diff, 1.0f/ed->zoom));
			modes->pan.mousepos = vmfApp.mousepos;
		}

		if(vmfApp.keys[SDL_SCANCODE_SPACE] <= Button_Released)  {
			ed->mode = ed->last_mode;
		}

		if(EditorFocusKeyL(pan.center)) {
			ed->camera = v2(0, 0);
			ed->zoom = 0.5f;
			ed->zoom_z = 2;
		}

		if(EditorFocusKeyL(pan.grid)) {
			ed->grid_size = 64;
		}

	} else {
		modes->pan.mousepos = v2(0, 0);
		modes->pan.down = 0;
	}

	Vec2 final_camera = v2_sub(ed->camera, v2_scale(vmfApp.size, 0.5 * 1.0/ed->zoom));
	ed->groups.grid->offset = final_camera;
	ed->groups.grid->scale = ed->zoom;
	render_draw(vmfRenderer, ed->groups.grid, vmfApp.size, vmfApp.scale);

	/*========================================================================
	 * Selections
	 *======================================================================*/

	vmfCurrent = get_group(vmfRenderer, 0);
	update_mousepos();
	Vec2 screen_mousepos = vmfApp.mousepos;
	vmfCurrent = ed->groups.grid;
	update_mousepos();
	Vec2 local_mousepos = vmfApp.mousepos;


	if(!ModeIs(Pan)) {
		Sprite* s;
		if(EditorJustClickedM(MouseLeft, screen_mousepos)) {
			if(s = render_group_query(ed->current->brushes, vmfApp.mousepos)) {
				i32 block = 0;
				i32 hit_handle = 0;
				if(ModeIs(Vertex)) {
					hit_handle = editor_check_vertex_handles(ed, ed->selected[0]);
				}

				if(!hit_handle) {
					if(EditorFocusKey(LCTRL, Button_Pressed)) {
						block = 1;
						if(editor_brush_is_selected(ed, s)) {
							editor_deselect(ed, s);
						} else {
							editor_select(ed, s);
						}
					} else {
						if(!editor_brush_is_selected(ed, s)) {
							editor_select_only(ed, s);
						}
					}
				}

				if(ModeIs(Edit)) {
					hit_handle = editor_check_resize_handles(ed, s);

					if(!hit_handle && !block) {
						editor_calc_selected_center(ed);
						modes->edit.start = v2_sub(ed->selected_center, local_mousepos);
						modes->edit.move_down = 1;
					} 
				} 
				editor_calc_selected_center(ed);
			} else if(AnySelected) {
				{
					i32 hit_handle = 0;
					if(ModeIs(Edit)) {
						for(isize i = 0; i < ed->selected_count; ++i) {
							Sprite* t = ed->selected[i];
							hit_handle = editor_check_resize_handles(ed, t);
							if(hit_handle) {
								break;
							}
						}
					} else if(ModeIs(Vertex)) {
						hit_handle = editor_check_vertex_handles(ed, ed->selected[0]);
					}

					if(!hit_handle)  {
						ed->input_block = 1;
						editor_clear_selected(ed);

					}
				}
			}
			if(ModeIs(Normal)) {
				modes->normal.start = local_mousepos;
				modes->normal.down = 1;
			}
		}

		if(ModeIs(Normal) && modes->normal.down) {
			if(vmfApp.mouse[MouseLeft] >= Button_Pressed) {
				render_create_primitive(ed->groups.overlay,
						modes->normal.start, v2_sub(local_mousepos, modes->normal.start), 
						v4(1, 0.5, 0, 0.5), Anchor_TopLeft);
			} else {
				//select everything in region
				Vec2 end = local_mousepos;
				Vec2 start = modes->normal.start;
				ed->selected_count = render_group_query_aabb(ed->current->brushes, aabb_from_extents(start, end), EditorSelectedMax, ed->selected);

				modes->normal.down = 0;
				modes->normal.start = v2(0, 0);
			}
		}

		if(!ModeIs(Vertex))
			for(isize i = 0; i < ed->selected_count; ++i) {
				s = ed->selected[i];
				Rect2 aabb = sprite_aabb(s);
				if(ModeIs(Edit)) {
					if(modes->edit.move_down) {
						Sprite* t = render_get_sprite(ed->groups.overlay);
						*t = *s;
						Vec2 hs = v2_scale(aabb.size, 0.5f);
						Vec2 tl = v2_sub(aabb.pos, hs);
						v2_sub_ip(&tl, ed->selected_center);
						v2_add_ip(&tl, editor_round_to_grid(ed, v2_add(modes->edit.start, local_mousepos)));
						v2_add_ip(&tl, hs);
						t->pos = tl;

						editor_draw_pos_size_text(ed, t->pos, t->size);

						t->color.w = 0.5f;
						t->color.x = 0.9;
						Sprite* v = render_get_sprite(ed->groups.overlay);
						*v = *t;
						v->size = v2_scale(v2(8, 8), 1.0f/ed->zoom);
						v->color.w = 1;

						s->color.w = 0.1f;
					} else {
						s->color.w = 0.75f;
					}
				}

				Sprite* selected_box = render_create_primitive(ed->groups.overlay, aabb.pos, aabb.size, v4(1, 0.5, 0, 0.25), 0);
				if(ModeIs(Vertex)) {
					selected_box->color.w = 0.1;
				}
				if(ModeIs(Edit)) { //resize controls
					Rect2 handles[4];
					editor_get_resize_handles(ed, s, handles);
					Vec4 control_color = v4(0.5, 1, 0.2, 0.5);
					for(isize i = 0; i < 4; ++i) {
						Rect2 handle = handles[i];
						Vec4 c = control_color;
						if(s == modes->edit.resizing) {
							selected_box->color.w = 0.75;
							if(i == modes->edit.active_handle) {
								c = v4(1, 1, 1, 1);
								//Interpret mouse pointer
								f32 mouse_diff = 0;
								switch(modes->edit.active_handle) {
									case 0: //top
									case 1: //bottom
										mouse_diff = local_mousepos.y + modes->edit.mouse_offset;
										handle.pos.y = mouse_diff;
										break;
									case 2: //left
									case 3: //right
										mouse_diff = local_mousepos.x + modes->edit.mouse_offset;
										handle.pos.x = mouse_diff;
										break;
								}

								//recalculate AABB based on active handle's position
								Vec2 extent = v2(0, 0); 
								Vec2 tl = v2_sub(aabb.pos, v2_scale(aabb.size, 0.5));
								Vec2 br = v2_add(aabb.pos, v2_scale(aabb.size, 0.5));
								switch(modes->edit.active_handle) {
									case 0: //top
										extent.y = handle.pos.y;
										extent = editor_round_to_grid(ed, extent);
										tl.y = extent.y;
										break;
									case 1: //bottom
										extent.y = handle.pos.y + handle.size.y;
										extent = editor_round_to_grid(ed, extent);
										br.y = extent.y;
										break;
									case 2: //left
										extent.x = handle.pos.x;
										extent = editor_round_to_grid(ed, extent);
										tl.x = extent.x;
										break;
									case 3: //right
										extent.x = handle.pos.x + handle.size.x;
										extent = editor_round_to_grid(ed, extent);
										br.x = extent.x;
										break;
								}

								aabb = aabb_from_extents(tl, br);

							}
						} else if(rect_contains_point(handle, local_mousepos)) {
							c = v4(0.8, 1, 0.4, 1);
						}
						render_create_primitive(ed->groups.overlay,
								handle.pos, handle.size,
								c, Anchor_TopLeft);
					}
					selected_box->pos = aabb.pos;
					selected_box->size = aabb.size;
				}
				editor_draw_pos_size_text(ed, aabb.pos, aabb.size);
				render_create_primitive(ed->groups.overlay, aabb.pos, v2_scale(v2(8, 8), 1.0f/ed->zoom), v4(1, 1, 1, 1), 0);
			}

	}

	/*========================================================================
	 * Vertex Mode
	 *======================================================================*/

	if(ModeIs(Vertex)) {
		if(AnySelected) {
			i32 clicked = EditorJustClickedM(MouseLeft, screen_mousepos);
			ed->selected_count = 1;
			Sprite* s = ed->selected[0];

			if(!modes->vert.down) {
				Sprite* t = render_get_sprite(ed->groups.overlay);
				*t = *s;
				t->color = v4(1, 0.5, 0, 0.1);
			}

			Rect2 handles[4];
			editor_get_vertex_handles(ed, s, handles);
			for(isize i = 0; i < 4; ++i) {
				Rect2 handle = handles[i];
				i32 mouse_over = rect_contains_point(handle, local_mousepos);
				Vec4 color = v4(1, 0.2, 0.2, 0.75);
				if(mouse_over) {
					color.w = 1;
					if(clicked) {
						modes->vert.down = 1;
						modes->vert.handle = i;
						modes->vert.start = local_mousepos;
						modes->vert.offset = v2_sub(v2_add_scaled(
									handle.pos, handle.size, 0.5f),
								local_mousepos);
						modes->vert.uv = v2(1, 1);
					}
				}

				Sprite* t = render_create_primitive(ed->groups.overlay, 
						handle.pos, handle.size, 
						color, Anchor_TopLeft);
			}

			if(modes->vert.down && vmfApp.mouse[MouseLeft] >= Button_Pressed) {
				//get ready
				Vec2 end = v2_sub(local_mousepos, modes->vert.offset);
				end = editor_round_to_grid(ed, end);
				Vec4 color = v4(1, 0.2, 0.2, 0.75);


				Sprite* t = render_create_primitive(ed->groups.overlay, 
						end, v2_scale(v2(32, 32), 1.0/ed->zoom), 
						color, Anchor_Center);

				v2_sub_ip(&end, s->pos);
				i32 cindex;
				Vec2 hs = v2_scale(s->size, 0.5);
				Vec2 tl = v2_scale(hs, -1);
				Vec2 br = hs;
				Vec2 corner;
				switch(modes->vert.handle) {
					case 0: //tl
						cindex = 0;
						corner = tl;
						break;
					case 1: //tr
						cindex = 1;
						corner = v2(br.x, tl.y);
						break;
					case 2: //br
						cindex = 3;
						corner = br;
						break;
					case 3: //bl
						cindex = 2;
						corner = v2(tl.x, br.y);
						break;
				}
				Vec2 uv = v2(end.x / corner.x, end.y / corner.y);
				//if(uv.x < 0) uv.x = 0;
				//if(uv.y < 0) uv.y = 0;
				modes->vert.uv = uv;
				modes->vert.cindex = cindex;
				{
					Sprite* t = render_get_sprite(ed->groups.overlay);
					*t = *s;
					t->corners[cindex] = modes->vert.uv;
					t->color = v4(1, 0.5, 0, 0.5);
				}
			} else if(modes->vert.down && vmfApp.mouse[MouseLeft] <= Button_Released) {
				s->corners[modes->vert.cindex] = modes->vert.uv;
				sprite_validate(s);
				ed->current->brushes->dirty = 1;
				modes->vert.down = 0; 
			}
		}
	}

	/*========================================================================
	 * Edit Mode
	 *======================================================================*/

	if(ModeIs(Edit)) {
#		define EditModeEnd do {\
	modes->edit.start = v2(0, 0);\
	modes->edit.create_down = 0;\
	modes->edit.resize_down = 0;\
	modes->edit.move_down = 0;\
	modes->edit.active_handle = -1; \
	modes->edit.resizing = NULL; \
	modes->edit.mouse_offset = 0; \
} while(0)


	if(!AnySelected && EditorJustClickedM(MouseLeft, screen_mousepos)) {
		modes->edit.create_down = 1;
		modes->edit.start = editor_floor_to_grid(ed, local_mousepos);
		Vec2 start = modes->edit.start;
		modes->edit.start_cell = v2i(start.x / ed->grid_size, start.y / ed->grid_size);
	}

if(modes->edit.move_down) {
	ed->current->brushes->dirty = 1;
	if(vmfApp.mouse[MouseLeft] <= Button_Released) {
		for(isize i = 0; i < ed->selected_count; ++i) {
			Sprite* t = ed->selected[i];
			Rect2 aabb = sprite_aabb(t);
			Vec2 hs = v2_scale(aabb.size, 0.5f);
			Vec2 tl = v2_sub(aabb.pos, hs);
			v2_sub_ip(&tl, ed->selected_center);
			v2_add_ip(&tl, editor_round_to_grid(ed, v2_add(modes->edit.start, local_mousepos)));
			v2_add_ip(&tl, hs);
			if(vmfApp.keys[SDL_SCANCODE_LSHIFT] >= Button_Pressed) {
				Sprite* s = render_get_sprite(ed->current->brushes);
				*s = *t;
				s->pos = tl;
				s->color.w = 0.75;
				editor_select(ed, s);
				editor_deselect(ed, t);
			} else {
				t->pos = tl;
			}

			t->color.w = 0.75;
		}
		ed->selected_center = v2(0, 0);
		for(isize i = 0; i < ed->selected_count; ++i) {
			Sprite* t = ed->selected[i];
			v2_add_ip(&ed->selected_center, v2_scale(t->pos, 1.0f/ed->selected_count));
		}
		ed->selected_center = editor_floor_to_grid(ed, ed->selected_center);
		EditModeEnd;
	} 
} else if(EditorFocusKeyL(action.paste)) {
	editor_paste_clipboard(ed);
} else if(modes->edit.resize_down) {
	if(vmfApp.mouse[MouseLeft] <= Button_Released) {
		Sprite* s = modes->edit.resizing;
		Rect2 handles[4];
		editor_get_resize_handles(ed, s, handles);
		Rect2 handle = handles[modes->edit.active_handle];
		Rect2 aabb = sprite_aabb(s);
		f32 mouse_diff = 0;
		switch(modes->edit.active_handle) {
			case 0: //top
			case 1: //bottom
				mouse_diff = local_mousepos.y + modes->edit.mouse_offset;
				handle.pos.y = mouse_diff;
				break;
			case 2: //left
			case 3: //right
				mouse_diff = local_mousepos.x + modes->edit.mouse_offset;
				handle.pos.x = mouse_diff;
				break;
		}

		//recalculate AABB based on active handle's position
		Vec2 extent = v2(0, 0); 
		switch(modes->edit.active_handle) {
			case 0: //top
				extent.y = handle.pos.y;
				break;
			case 1: //bottom
				extent.y = handle.pos.y + handle.size.y;
				break;
			case 2: //left
				extent.x = handle.pos.x;
				break;
			case 3: //right
				extent.x = handle.pos.x + handle.size.x;
				break;
		}
		extent = editor_round_to_grid(ed, extent);

		//okay, now we know the new side
		//we need to recalculate the new size
		//and then recenter the aabb wrt that 
		Vec2 tl = v2_sub(aabb.pos, v2_scale(aabb.size, 0.5));
		Vec2 br = v2_add(aabb.pos, v2_scale(aabb.size, 0.5));
		switch(modes->edit.active_handle) {
			case 0: //top
				tl.y = extent.y;
				break;
			case 1: //bottom
				br.y = extent.y;
				break;
			case 2: //left
				tl.x = extent.x;
				break;
			case 3: //right
				br.x = extent.x;
				break;
		}
		aabb = aabb_from_extents(tl, br);
		s->pos = aabb.pos;
		s->size = aabb.size;
		ed->current->brushes->dirty = 1;

		EditModeEnd;
	}
}

if(AnySelected) {
	if(EditorFocusKeyL(action.cut)) {
		editor_copy_selected_to_clipboard(ed);
		for(isize i = 0; i < ed->selected_count; ++i) {
			render_remove_sprite(ed->current->brushes, ed->selected[i]);
		}
		editor_clear_selected(ed);
	} else if(EditorFocusKeyL(action.copy)) {
		editor_copy_selected_to_clipboard(ed);
	} else if(EditorFocusKeyL(action.rotate)) {
		if(EditorFocusKey(LSHIFT, Button_Pressed)) {
			if(ed->selected_count > 1) {
				editor_calc_selected_center(ed);
				for(isize i = 0; i < ed->selected_count; ++i) {
					Sprite* s = ed->selected[i];
					v2_sub_ip(&s->pos, ed->selected_center);
					v2_rotate_ip(&s->pos, v2(0, -1));
					v2_add_ip(&s->pos, ed->selected_center);
					sprite_rotate_ccw90(s);
				}
			} else {
				sprite_rotate_ccw90(ed->selected[0]);
			}
		} else {
			if(ed->selected_count > 1) {
				editor_calc_selected_center(ed);
				for(isize i = 0; i < ed->selected_count; ++i) {
					Sprite* s = ed->selected[i];
					v2_sub_ip(&s->pos, ed->selected_center);
					v2_rotate_ip(&s->pos, v2(0, -1));
					v2_add_ip(&s->pos, ed->selected_center);
					sprite_rotate_cw90(s);
				}
			} else {
				sprite_rotate_cw90(ed->selected[0]);
			}
		}
		ed->current->brushes->dirty = 1;
	} else if(EditorFocusKeyL(action.flip)) {
		//TODO(will): there's a bug, flipping things moves the center?
		if(EditorFocusKey(LSHIFT, Button_Pressed)) {
			//vertical flip
			if(ed->selected_count > 1) {
				editor_calc_selected_center(ed);
				for(isize i = 0; i < ed->selected_count; ++i) {
					Sprite* s = ed->selected[i];

					v2_sub_ip(&s->pos, ed->selected_center);
					s->pos.y *= -1;
					v2_add_ip(&s->pos, ed->selected_center);

					sprite_flip_vert(s);
				}
			} else {
				sprite_flip_vert(ed->selected[0]);
			}
		} else {
			if(ed->selected_count > 1) {
				editor_calc_selected_center(ed);
				for(isize i = 0; i < ed->selected_count; ++i) {
					Sprite* s = ed->selected[i];

					v2_sub_ip(&s->pos, ed->selected_center);
					s->pos.x *= -1;
					v2_add_ip(&s->pos, ed->selected_center);

					sprite_flip_horiz(s);
				}
			} else {
				sprite_flip_horiz(ed->selected[0]);
			}
		}
		ed->current->brushes->dirty = 1;

	}

} else if(modes->edit.create_down) {
	if(vmfApp.mouse[MouseLeft] >= Button_Pressed) {
		Vec2 grid = v2_scale(v2(ed->grid_size, ed->grid_size), 0.5f);;
		Vec2 start = v2_scale(v2(modes->edit.start_cell.x, modes->edit.start_cell.y), ed->grid_size);
		v2_add_ip(&start, grid);
		//start is the top left corner
		//cases: mouse is
		//	- pointing br (+, +)
		//	- pointing bl (-, +)
		//	- pointing tr (+, -)
		//	- pointing tl (-, -)
		//We always want to include the cell we started dragging from 
		//in the brush.
		Vec2 diff = v2_sub(local_mousepos, start);
		i32 mode = 0;
		if(diff.x > 0) {
			mode |= 1;
		}
		if(diff.y > 0) {
			mode |= 2;
		}

		switch(mode) {
			case 0: // 0b00 tl
				v2_add_ip(&start, grid);
				break;
			case 1: // 0b01 tr
				v2_add_ip(&start, v2(-grid.x, grid.y));
				break;
			case 2: // 0b10 bl
				v2_add_ip(&start, v2(grid.x, -grid.y));
				break;
			case 3: // 0b11 br
				v2_sub_ip(&start, grid);
				break;
		}
		modes->edit.start = start;
	}

	if(vmfApp.mouse[MouseLeft] == Button_JustReleased) {
		Vec2 end = editor_round_to_grid(ed, local_mousepos);
		Vec2 start = modes->edit.start;
		Vec2 p1 = v2(Min(end.x, start.x), Min(end.y, start.y));
		Vec2 p2 = v2(Max(end.x, start.x), Max(end.y, start.y));
		Rect2 area = rect2_v(p1, v2_sub(p2, p1));

		if(fabsf(area.size.x * area.size.y) > 0.0001) {
			Sprite* s = render_get_sprite(ed->current->brushes);
			s->flags = SpriteFlag_Primitive;
			s->pos = v2_add(area.pos, v2_scale(area.size, 0.5f));
			s->size = area.size;
		}

		EditModeEnd;
	}

	if(vmfApp.mouse[MouseLeft] >= Button_Pressed) {
		Vec2 end = editor_round_to_grid(ed, local_mousepos);
		Vec2 start = modes->edit.start;

		editor_draw_pos_size_text(ed, v2_scale(v2_add(start, end), 0.5f), v2_abs(v2_sub(end, start)));

		render_create_primitive(ed->groups.overlay, 
				start, 
				v2_sub(end, start), 
				v4(1, 1, 1, 0.75), 
				Anchor_TopLeft);
	} else {
		EditModeEnd;
	}
}
if(vmfApp.mouse[MouseRight] == Button_JustPressed) {
	EditModeEnd;
}
} else {
	EditModeEnd;
}

/*========================================================================
 * Renderinm
 *======================================================================*/

ed->current->brushes->offset = final_camera;
ed->current->brushes->scale = ed->zoom;

ed->current->brushes->tint.w = 0.25;
ed->current->brushes->draw_mode = GL_TRIANGLE_STRIP;
render_draw(vmfRenderer, ed->current->brushes, vmfApp.size, vmfApp.scale);

ed->current->brushes->tint.w = 1.0;
ed->current->brushes->draw_mode = GL_LINE_LOOP;
render_draw(vmfRenderer, ed->current->brushes, vmfApp.size, vmfApp.scale);

ed->groups.overlay->offset = final_camera;
ed->groups.overlay->scale = ed->zoom;
//ed->groups.overlay->tint.w = 1.0;
ed->groups.overlay->draw_mode = GL_TRIANGLE_STRIP;
render_draw(vmfRenderer, ed->groups.overlay, vmfApp.size, vmfApp.scale);
//	render_clear(ed->groups.overlay);
ed->groups.stext->offset = v2(0, 0);
ed->groups.stext->scale = 1;
render_draw(vmfRenderer, ed->groups.stext, vmfApp.size, vmfApp.scale);

render_draw(vmfRenderer, ed->layer_menu.bg, vmfApp.size, vmfApp.scale);




/*========================================================================
 * Toolbar
 *======================================================================*/

nk->style.window.padding = nk_vec2(6, 8);
nk->style.window.spacing = nk_vec2(0, 8);
nk->style.button.border = 0;
nk->style.button.rounding = 0;
//nk_color c = {255, 255, 255, 255};
//nk->style.text.color = c;

if(nk_begin(nk, "Menu", nk_rect(-2, 0, 86, vmfApp.size.y), NK_WINDOW_NO_SCROLLBAR)) {
	nk_layout_row_dynamic(nk, 24, 1);

	struct nk_style_button style = nk->style.button;
	//Normal button
	if(ModeIs(Normal)) {
		style.normal = style.active;
	} else {
		style.normal = nk->style.button.normal;
	}
	if(nk_button_label_styled(nk, &style, "Normal")) {
		ed->last_mode = Mode_Normal;
		ed->mode = Mode_Normal;
	}

	//Pan
	if(ModeIs(Pan)) {
		style.normal = style.active;
	} else {
		style.normal = nk->style.button.normal;
	}
	if(nk_button_label_styled(nk, &style, "Pan")) {
	}

	//Edit button
	if(ModeIs(Edit)) {
		style.normal = style.active;
	} else {
		style.normal = nk->style.button.normal;
	}
	if(nk_button_label_styled(nk, &style, "Edit")) {
		ed->last_mode = Mode_Normal;
		ed->mode = Mode_Edit;
	}

	//Vert button
	if(ModeIs(Vertex)) {
		style.normal = style.active;
	} else {
		style.normal = nk->style.button.normal;
	}
	if(nk_button_label_styled(nk, &style, "Vertex")) {
		ed->last_mode = Mode_Normal;
		ed->mode = Mode_Vertex;
	}

#if 0
	//Cut button
	if(ModeIs(Cut)) {
		style.normal = style.active;
	} else {
		style.normal = nk->style.button.normal;
	}
	if(nk_button_label_styled(nk, &style, "Cut")) {
		printf("Pressed button 2!\n");
	}
#endif

	if(nk_button_label(nk, "Export")) {
		VmfContext ctx;
		vmfctx_init(&ctx, "sketch.vmf");
		vmfctx_start(&ctx);
		vmfctx_add_world_geom(&ctx, ed->current, 1);
		vmfctx_end(&ctx);
	}

	nk_layout_row_dynamic(nk, 8, 1);
	nk->style.window.spacing = nk_vec2(0, 2);
	nk_style_push_font(nk, &vmfApp.fonts.small->handle);
	nk_labelf_wrap(nk, "X %.0f", ed->camera.x);
	nk_labelf_wrap(nk, "Y %.0f", ed->camera.y);
	nk_labelf_wrap(nk, "Grid %dhu", ed->grid_size);
	nk_labelf_wrap(nk, "Scale %.2f", ed->zoom);
	nk_labelf_wrap(nk, "Mode %d", ed->mode);
	nk_labelf_wrap(nk, "dt %.4f", vmfApp.elapsed * 1000);
	nk_style_pop_font(nk);

}
nk_end(nk);

update_mousepos();
#if 0
if(vmfApp.keys[SDL_SCANCODE_F1] == Button_JustPressed) {
	if(vmfCurrent->draw_mode == GL_LINE_LOOP) {
		vmfCurrent->draw_mode = GL_TRIANGLE_STRIP;
	} else {
		vmfCurrent->draw_mode = GL_LINE_LOOP;
	}
}
#endif

}

void editor_change_grid_size(Editor* ed, i32 new_grid)
{
	ed->grid_size = new_grid;
	//
	//Maps in Hammer can be as big as +/-16384, -32768 to 32767 in size
	//256 lines on the vert/horiz will work for 64hu or bigger
	//But with fewer we'll run out of lines on one side.
	f32 hwidth = 16384*2;
	i32 hgrid = new_grid / 2;
	i32 cells = hwidth / ed->grid_size / 2;
	for(isize i = 0; i < cells; ++i) {
		Vec4 color = v4(1, 1, 1, 0.1);
		render_create_primitive(ed->groups.grid, v2(0, new_grid * i), 
				v2(hwidth, new_grid), color, Anchor_Top);
		render_create_primitive(ed->groups.grid, v2(0, new_grid * -i),
				v2(hwidth, new_grid), color, Anchor_Bottom);

		render_create_primitive(ed->groups.grid, v2(new_grid * i, 0),
				v2(new_grid, hwidth), color, Anchor_Left);
		render_create_primitive(ed->groups.grid, v2(new_grid * -i,0), 
				v2(new_grid, hwidth), color, Anchor_Right);

		if(i % 8 == 0) {
			if(i % 128 == 0) {
				color = v4(1, 0.6, 1.0, 0.75);
			} else if(i % 32 == 0) {
				color = v4(1, 0.9, 0.5, 0.75);
			} else {
				color = v4(0.5, .8, 1, 0.75);
			}
			render_create_primitive(ed->groups.grid, v2(0, new_grid * i), v2(hwidth, 0), color, Anchor_Center);
			render_create_primitive(ed->groups.grid, v2(0, new_grid * -i), v2(hwidth, 0), color, Anchor_Center);
			render_create_primitive(ed->groups.grid, v2(new_grid * i, 0), v2(0, hwidth), color, Anchor_Center);
			render_create_primitive(ed->groups.grid, v2(new_grid * -i,0), v2(0, hwidth), color, Anchor_Center);

		}
	}

	render_create_primitive(ed->groups.grid, v2(0, 0), v2(hwidth, 1), v4(1.0, 0.5, 0.5, 1.0), 0);
	render_create_primitive(ed->groups.grid, v2(0, 0), v2(1, hwidth), v4(1.0, 0.5, 0.5, 1.0), 0);

}
#endif

