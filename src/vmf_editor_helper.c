

void render_clip(Rect2 viewport)
{
	glEnable(GL_SCISSOR_TEST);
	glScissor((i32)viewport.pos.x, (i32)(viewport.size.y + viewport.pos.y),
			(isize)viewport.size.x, (isize)viewport.size.y);
}

void render_end_clip()
{
	glDisable(GL_SCISSOR_TEST);
}

#define ModeIs3(a, b, c) (ed->mode == Mode_##a || ed->mode == Mode_##b || ed->mode == Mode_##c)
#define ModeIs2(a, b) (ed->mode == Mode_##a || ed->mode == Mode_##b)
#define ModeIs(a) (ed->mode == Mode_##a)

#define EditorJustClicked(btn) (!ed->input_block && (vmfApp.mouse[btn] == Button_JustPressed) && (rect_contains_point(ed->screen_bounds, vmfApp.mousepos)))
#define EditorJustClickedM(btn, mousepos) (!ed->input_block && (vmfApp.mouse[btn] == Button_JustPressed) && (rect_contains_point(ed->screen_bounds, mousepos)))
#define EditorMouseCheck(btn, mousepos, state) (!ed->input_block && (vmfApp.mouse[btn] == state) && (rect_contains_point(ed->screen_bounds, mousepos)))

#define PanelJustClicked(btn) (!ed->input_block && (vmfApp.mouse[btn] == Button_JustPressed) && (rect_contains_point(ed->layer_menu.bounds, vmfApp.mousepos)))
#define PanelJustClickedM(btn, mousepos) (!ed->input_block && (vmfApp.mouse[btn] == Button_JustPressed) && (rect_contains_point(ed->layer_menu.bounds, mousepos)))
#define PanelMouseCheck(btn, mousepos, state) (!ed->input_block && (vmfApp.mouse[btn] == state) && (rect_contains_point(ed->layer_menu.bounds, mousepos)))

#define EditorFocusKey(key, state) (!ed->input_block && (ed->focus == Focus_Editor) && (vmfApp.keys[SDL_SCANCODE_##key] == state))
#define EditorFocusKeyR(key, state) (!ed->input_block && (ed->focus == Focus_Editor) && (vmfApp.keys[key] == state))
#define EditorFocusKeyL(u) (!ed->input_block && (ed->focus == Focus_Editor) && (vmfApp.keys[ed->keys.u] == Button_JustPressed))

#define EditorFocusKeyPressed(u) (!ed->input_block && (ed->focus == Focus_Editor) && (vmfApp.keys[ed->keys.u] >= Button_Pressed))

#define PanelFocusKey(key, state) ((ed->focus == Focus_Panel) && (vmfApp.keys[SDL_SCANCODE_##key] == state))
#define PanelFocusKeyR(key, state) ((ed->focus == Focus_Panel) && (vmfApp.keys[key] == state))
#define PanelFocusKeyL(u) ((ed->focus == Focus_Panel) && (vmfApp.keys[ed->keys.u] == Button_JustPressed))

#define Selected (ed->selected[0])
#define AnySelected (ed->selected_count > 0)

//TODO(will) Remove with modularization of modes
#define EditModeEnd do {\
	modes->edit.start = v2(0, 0);\
	modes->edit.create_down = 0;\
	modes->edit.resize_down = 0;\
	modes->edit.move_down = 0;\
	modes->edit.active_handle = -1; \
	modes->edit.resizing = NULL; \
	modes->edit.mouse_offset = 0; \
	} while(0)


void editor_clear_focus(Editor* ed)
{
	if((!nk_window_is_any_hovered(nk) && vmfApp.mouse[MouseLeft]) || vmfApp.keys[SDL_SCANCODE_RETURN] == Button_JustPressed) {
		//ed->input_block = 1;
		nk_edit_unfocus(nk);
	}
}

void editor_save_to_file(Editor* ed, string filename)
{
	FILE* fp = fopen(filename, "wb");
	if(fp) {
		write_u8('w', fp);
		write_u8('v', fp);
		write_u8('m', fp);
		write_u8('f', fp);
		write_i32(0x1000, fp);

		write_i32(ed->layer_count, fp);
		for(isize i = 0; i < ed->layer_count; ++i) {
			write_layer_to_file(ed->layers + i, fp);
		}
		fclose(fp);
	}
}

void editor_open_from_file(Editor* ed, string filename)
{
	FILE* fp = fopen(filename, "rb");
	if(fp) {
		char w, v, m, f;
		i32 ver;
		read_u8(w, fp);
		read_u8(v, fp);
		read_u8(m, fp);
		read_u8(f, fp);
		read_i32(ver, fp);
		if(w != 'w' || v != 'v' || m != 'm' || 'f' != f) {
			printf("Tried to open non-mapsketch file\n");
		} else if(ver != 0x1000) {
			printf("Tried to open mapsketch file from wrong version");
		} else {
			//alright, 1) we delete all layers
			i32 loaded_layers = 0;
			read_i32(loaded_layers, fp);
			if(loaded_layers > 0) {
				for(isize i = 0; i < ed->layer_count; ++i) {
					EditorLayer* l = ed->layers + i;
					render_clear(l->brushes);
					render_remove_group(vmfRenderer, l->brushes);
				}
				ed->layer_count = 0;
				for(isize i = 0; i < loaded_layers; ++i) {
					EditorLayer* l = editor_get_layer(ed);
					read_layer(l, fp);
				}
			}
		}
		fclose(fp);
	}
}

void editor_add_from_file(Editor* ed, string filename)
{
	FILE* fp = fopen(filename, "rb");
	if(fp) {
		char w, v, m, f;
		i32 ver;
		read_u8(w, fp);
		read_u8(v, fp);
		read_u8(m, fp);
		read_u8(f, fp);
		read_i32(ver, fp);
		if(w != 'w' || v != 'v' || m != 'm' || 'f' != f) {
			printf("Tried to open non-mapsketch file\n");
		} else if(ver != 0x1000) {
			printf("Tried to open mapsketch file from wrong version");
		} else {
			//alright, 1) we delete all layers
			i32 loaded_layers = 0;
			read_i32(loaded_layers, fp);
			if(loaded_layers > 0) {
				for(isize i = 0; i < loaded_layers; ++i) {
					EditorLayer* l = editor_get_layer(ed);
					read_layer(l, fp);
				}
			}
		}
		fclose(fp);
	}
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

EditorLayer* editor_get_layer(Editor* ed)
{
	if(ed->layer_count >= ed->layer_capacity) return NULL;
	EditorLayer* l = ed->layers + ed->layer_count++;
	l->id = ed->layer_count - 1;
	editor_layer_init(ed, l);
	return l;
}

void editor_remove_layer(Editor* ed, EditorLayer* l)
{
	render_clear(l->brushes);
	render_remove_group(vmfRenderer, l->brushes);
	ed->layers[l->id] = ed->layers[--ed->layer_count];
	editor_assign_layer_ids(ed);
}

void editor_assign_layer_ids(Editor* ed)
{
	for(isize i = 0; i < ed->layer_count; ++i) {
		ed->layers[i].id = i;
	}
}



Vec2 editor_round_to_grid(Editor* ed, Vec2 pt)
{
	v2_scale_ip(&pt, 1.0f/ed->grid_size);
	v2_round_ip(&pt);
	v2_scale_ip(&pt, ed->grid_size);
	return pt;
}

Vec2 editor_floor_to_grid(Editor* ed, Vec2 pt)
{
	v2_scale_ip(&pt, 1.0f/ed->grid_size);
	v2_floor_ip(&pt);
	v2_scale_ip(&pt, ed->grid_size);
	return pt;
}

Vec2 editor_int_to_grid(Editor* ed, Vec2 pt)
{
	v2_scale_ip(&pt, 1.0f/ed->grid_size);
	v2_int_ip(&pt);
	v2_scale_ip(&pt, ed->grid_size);
	return pt;
}


Vec2 editor_get_screen_pt(Editor* ed, Vec2 pt)
{
	v2_sub_ip(&pt, ed->camera); 
	v2_scale_ip(&pt, ed->zoom);
	v2_add_ip(&pt, v2_scale(vmfApp.size, 0.5f));
	return pt;
}

i32 editor_brush_is_selected(Editor* ed, Sprite* s)
{
	for(isize i = 0; i < ed->selected_count; ++i) {
		Sprite* t = ed->selected[i];
		if(s == t) {
			return 1;
		}
	}
	return 0;
}

void editor_select(Editor* ed, Sprite* s)
{
	if(ed->selected_count >= EditorSelectedMax) {
		return;
	}

	for(isize i = 0; i < ed->selected_count; ++i) {
		Sprite* t = ed->selected[i];
		if(s == t) {
			return;
		}
	}

	ed->selected[ed->selected_count++] = s;
}

void editor_select_only(Editor* ed, Sprite* s)
{
	editor_clear_selected(ed);
	ed->selected[ed->selected_count++] = s;
}


void editor_deselect(Editor* ed, Sprite* s)
{
	isize index = -1;
	for(isize i = 0; i < ed->selected_count; ++i) {
		Sprite* t = ed->selected[i];
		if(s == t) {
			index = i;
			break;
		}
	}

	if(index != -1) {
		ed->selected_count--;
		ed->selected[index] = ed->selected[ed->selected_count];
	}
}

void editor_clear_selected(Editor* ed) 
{
	ed->selected_count = 0;
}

void editor_copy_selected_to_clipboard(Editor* ed)
{
	ed->clipboard_count = 0;
	ed->clipboard_center = v2(0, 0);
	for(isize i = 0; i < ed->selected_count; ++i) {
		Sprite* t = ed->selected[i];
		Sprite* s = ed->clipboard + i;
		*s = *t;
		v2_add_ip(&ed->clipboard_center, v2_scale(s->pos, 1.0f/ed->selected_count));
		ed->clipboard_count++;
	}

	ed->clipboard_center = editor_round_to_grid(ed, ed->clipboard_center);

	for(isize i = 0; i < ed->clipboard_count; ++i)  {
		Sprite* s = ed->clipboard + i;
		v2_sub_ip(&s->pos, ed->clipboard_center);
	}
}

void editor_calc_selected_center(Editor* ed)
{
	ed->selected_center = v2(0, 0);
	Vec2 min_pt = v2(FLT_MAX, FLT_MAX), max_pt = v2(-FLT_MAX, -FLT_MAX);
	for(isize i = 0; i < ed->selected_count; ++i) {
		Sprite* t = ed->selected[i];
		v2_add_ip(&ed->selected_center, v2_scale(t->pos, 1.0f/ed->selected_count));
#if 0
		Vec2 corners[4];
		sprite_corners(t, corners);
		for(isize j = 0; j < 4; ++j) {
			min_pt = v2_min(min_pt, corners[j]);
			max_pt = v2_max(max_pt, corners[j]);
		}
#endif
	}
	ed->selected_center = editor_floor_to_grid(ed, ed->selected_center);
	ed->selected_size = v2_sub(max_pt, min_pt);
}


void editor_paste_clipboard(Editor* ed)
{
	editor_clear_selected(ed);
	for(isize i = 0; i < ed->clipboard_count; ++i) {
		Sprite* t = render_get_sprite(ed->current->brushes);
		Sprite* s = ed->clipboard + i;
		*t = *s;
		v2_add_ip(&t->pos, editor_round_to_grid(ed, ed->camera));
		editor_select(ed, t);
	}
	editor_calc_selected_center(ed);
}

void editor_get_resize_handles(Editor* ed, Sprite* s, Rect2* r)
{
	Rect2 aabb = sprite_aabb(s);
	Rect2 screen = rect2_v(editor_get_screen_pt(ed, aabb.pos), 
			v2_scale(aabb.size, ed->zoom));
	Vec2 control_size = v2_scale(v2(16, 16), 1.0f/ed->zoom);
	i32 x_on_outside = 0;
	i32 y_on_outside = 0;
	if(screen.size.x / 4 < 16) x_on_outside = 1;
	if(screen.size.y / 4 < 16) y_on_outside = 1;
	v2_scale_ip(&screen.size, 1.0f/ed->zoom);
	//Rect2's in top-left form
	Rect2 top = rect2_v(
			v2(aabb.pos.x - aabb.size.x / 2, 
				aabb.pos.y - aabb.size.y / 2 + 
				(y_on_outside ? -control_size.y : 0)), 
			v2(screen.size.x, control_size.y));
	Rect2 bottom = rect2_v(
			v2(aabb.pos.x - aabb.size.x / 2, 
				aabb.pos.y + aabb.size.y/2 + 
				(y_on_outside ? 0 : -control_size.y)), 
			v2(screen.size.x, control_size.y));
	Rect2 left = rect2_v(
			v2(aabb.pos.x - aabb.size.x/2 + 
				(x_on_outside ? -control_size.x : 0),
				aabb.pos.y - aabb.size.y/2), 
			v2(control_size.x, screen.size.y));
	Rect2 right = rect2_v(
			v2( aabb.pos.x + aabb.size.x/2 + 
				(x_on_outside ? 0: -control_size.x),
				aabb.pos.y - aabb.size.y/2), 
			v2(control_size.x, screen.size.y));
	r[0] = top;
	r[1] = bottom;
	r[2] = left;
	r[3] = right;
}

//ModeIs(Edit)
//This function handles some of the state of the edit mode too
//It probably shouldn't
i32 editor_check_resize_handles(Editor* ed, Sprite* s)
{
	EditorModeData* modes = &ed->modes;
	Rect2 handles[4];
	editor_get_resize_handles(ed, s, handles);
	Vec2 local_mousepos = vmfApp.mousepos;
	i32 hit_handle = 0;
	for(isize i = 0; i < 4; ++i) {
		Rect2 handle = handles[i];
		hit_handle = rect_contains_point(handle, local_mousepos);
		if(hit_handle) {
			modes->edit.active_handle = i;
			break;
		}
	}

	if(hit_handle) {
		modes->edit.resizing = s;
		modes->edit.start = local_mousepos;
		modes->edit.resize_down = 1;
		switch(modes->edit.active_handle) {
			case 0: //top
			case 1: //bottom
				modes->edit.mouse_offset = handles[modes->edit.active_handle].pos.y 
					- local_mousepos.y;
				break;
			case 2: //left
			case 3: //right
				modes->edit.mouse_offset = handles[modes->edit.active_handle].pos.x 
					- local_mousepos.x;
				break;
		}
	}

	return hit_handle;
}

void editor_get_vertex_handles(Editor* ed, Sprite* s, Rect2* r)
{
	Vec2 corners[4];
	sprite_corners(s, corners);

	Vec2 handle_size = v2(32, 32);
	v2_scale_ip(&handle_size, 1.0f/ed->zoom);
	Vec2 hs = v2_scale(handle_size, 0.5);
	for(isize i = 0; i < 4; ++i) {
		Rect2 handle = rect2_v(v2_sub(corners[i], hs), handle_size);
		r[i] = handle;
	}

}

Rect2 editor_get_low_side_handle(Editor* ed, Sprite* s)
{
	Vec2 centroid = sprite_get_centroid(s);
	Vec2 tl = v2_add(centroid, s->low_side);
	Vec2 size = v2_scale(v2(32, 32), 1.0f/ed->zoom);
	Rect2 r = rect2_v(v2_sub(tl, v2_scale(size, 0.5f)), size);
	return r;
}


i32 editor_check_vertex_handles(Editor* ed, Sprite* s)
{
	EditorModeData* modes = &ed->modes;
	Vec2 local_mousepos = vmfApp.mousepos;
	Rect2 handles[4];
	editor_get_vertex_handles(ed, s, handles);
	for(isize i = 0; i < 4; ++i) {
		Rect2 handle = handles[i];
		if(rect_contains_point(handle, local_mousepos)) { 
			modes->vert.handle = i;
			return 1;
		}
	}

	if(rect_contains_point(editor_get_low_side_handle(ed, s), local_mousepos)) {
		//modes->vert.low_side_hover = 1;
		return 1;
	}
	
	return 0;
}

void editor_draw_pos_size_text(Editor* ed, Vec2 pos, Vec2 size)
{
	char buf[256];
	isize len = snprintf(buf, 256, "(%.0f,%.0f) %.0fx%.0f", 
			pos.x, pos.y, size.x, size.y);
	render_string_c(ed->groups.stext, Font(fixed), editor_get_screen_pt(ed, v2_sub(pos, v2(0, size.y/2 + 48))), buf);
}







