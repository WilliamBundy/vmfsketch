
/* Some convenience functions around nk_... stuff
 * Exposes the nk internal font rendering to my rendering system
 * So I don't have to deal with nk_begin/end shenanigans for 
 * drawing a few words.
 *
 * To use: make an immediate mode group, w/ scale 1
 *	call render_string on it, get text
 */ 

Vec2 size_text(nk_font* font, i32 len, string text)
{
	i32 glyphs = 0;
	struct nk_vec2 size = nk_text_calculate_text_bounds(
			&font->handle,
			text, len,
			font->handle.height,
			NULL, NULL,
			&glyphs, 0
			);
	return v2(size.x, size.y);
}

void render_text(RenderGroup* group, nk_font* nkfont, Vec2 pos, i32 len, string text)
{
    int text_len = 0;
    nk_rune unicode = 0;
    nk_rune next = 0;
    int glyph_len = 0;
    int next_glyph_len = 0;
    struct nk_user_font_glyph g;

	/*
    NK_ASSERT(list);
    if (!list || !len || !text) return;
    if (!NK_INTERSECT(rect.x, rect.y, rect.w, rect.h,
        list->clip_rect.x, list->clip_rect.y, list->clip_rect.w, list->clip_rect.h)) return;
		*/

	const struct nk_user_font* font = &nkfont->handle;
	group->texture = font->texture.id;
	f32 font_height = font->height;
	//instead, assume that group->texture is the font texture
    //nk_draw_list_push_image(list, font->texture);
    f32 x = pos.x;
    glyph_len = nk_utf_decode(text, &unicode, len);
    if (!glyph_len) return;

    /* draw every glyph image */
    while (text_len < len && glyph_len) {
        float gx, gy, gh, gw;
        float char_width = 0;
        if (unicode == NK_UTF_INVALID) break;

        /* query currently drawn glyph information */
        next_glyph_len = nk_utf_decode(text + text_len + glyph_len, &next, (int)len - text_len);
        font->query(font->userdata, 32, &g, unicode,
                    (next == NK_UTF_INVALID) ? '\0' : next);

        /* calculate and draw glyph drawing rectangle and image */
        gx = x + g.offset.x/2;
        gy = pos.y + g.offset.y/2;
        gw = g.width; gh = g.height;
        char_width = g.xadvance;
        //nk_draw_list_push_rect_uv(list, nk_vec2(gx,gy), nk_vec2(gx + gw, gy+ gh),
        //   g.uv[0], g.uv[1], fg);
		Sprite* s = render_get_sprite(group);//, v2(gx, gy), )
		s->pos = v2(gx, gy);
		s->size = v2_scale(v2(gw, gh), 0.5f);
		s->texture = rect2(g.uv[0].x, g.uv[0].y, g.uv[1].x - g.uv[0].x, g.uv[1].y - g.uv[0].y);
		s->flags = Anchor_TopLeft;

        /* offset next glyph */
        text_len += glyph_len;
        x += char_width * 0.5f;
        glyph_len = next_glyph_len;
        unicode = next;
    }
}

#define Font(x) (vmfApp.fonts.x)
void render_string(RenderGroup* group, nk_font* font, Vec2 pos, string text)
{
	render_text(group, font, pos, strlen(text), text);
}

void render_string_c(RenderGroup* group, nk_font* font, Vec2 pos, string text)
{
	isize len = strlen(text);
	Vec2 size = size_text(font, len, text);
	size.y = 0;
	pos.x -= size.x/2;
	render_text(group, font, pos , len, text);
}

