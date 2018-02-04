#include "vmf_defines.h"

const f32 SpriteAnchorX[] = {
	0.0f,
	-0.5f,
	0.0f,
	0.5f, 
	0.5f,
	0.5f, 
	0.0f, 
	-0.5f,
	-0.5f
};

const f32 SpriteAnchorY[] = {
	0.0f,
	-0.5f,
	-0.5f,
	-0.5f,
	0.0f,
	0.5f,
	0.5f,
	0.5f,
	0.0f
};

#ifndef REFLECTED
typedef enum SpriteFlags
{
	Anchor_Center = 0,
	Anchor_TopLeft = 1,
	Anchor_Top = 2,
	Anchor_TopRight = 3,
	Anchor_Right = 4,
	Anchor_BottomRight = 5,
	Anchor_Bottom = 6,
	Anchor_BottomLeft = 7,
	Anchor_Left = 8,
	SpriteFlag_FlipHoriz = Flag(4),
	SpriteFlag_FlipVert = Flag(5),
	SpriteFlag_Primitive = Flag(6),
	SpriteFlag_Null = Flag(10)
} SpriteFlags;


struct Sprite
{
	u32 flags;
	i32 sort;

	Vec2 pos;
	Vec2 size;
	Vec2 center;
	Vec2 rotation;
	Rect2 texture;
	Vec4 color;
	Vec2 corners[4];
	Vec2 low_side;
};

union SpriteFreeList
{
	struct
	{
		u32 is_link;
		i32 next;
	} link;
	Sprite s;
};


typedef enum RenderGroupKind
{
	Group_RetainedStream,
	Group_RetainedDynamic,
	Group_Immediate
} RenderGroupKind;


struct RenderGroup
{
	i32 index;
	i32 kind;

	u32 vao;
	u32 vbo;
	u32 texture;
	i32 texture_width;
	i32 texture_height;
	Vec2 inv_texture_size;


	u32 draw_mode;

	Vec2 offset;
	f32 scale;

	f32 ortho[16];
	Vec4 tint;

	SpriteFreeList* free_list;
	isize last_filled;
	
	i32 dirty;
	i32 always_dirty;

	isize count, capacity;
	Sprite* sprites;
};
 
struct Renderer
{
	u32 shaders;
	u32 vbo;
	u32 vao;

	isize u_texture_size;
	isize u_ortho_matrix;
	isize u_scale;
	isize u_index_mode;
	isize u_tint;

	u32 texture;
	i32 texture_width;
	i32 texture_height;
	Vec2 inv_texture_size;

	MemoryPool* group_pool;
	RenderGroup** groups;
	isize group_count, group_capacity;
};
#endif


void sprite_init(Sprite* s)
{
	s->flags = 0;
	s->sort = 0;
	s->pos = v2(0, 0);
	s->size = v2(16, 16);
	s->center = v2(0, 0);
	s->rotation = v2(1, 0);
	s->texture = rect2(0, 0, 1, 1);
	//s->color = color_from_rgba(0xFFFFFFFF);
	s->color = v4(1, 1, 1, 1);
	//order:
	// tl, tr, bl, br
	s->corners[0] = v2(1, 1); 
	s->corners[1] = v2(1, 1); 
	s->corners[2] = v2(1, 1);	
	s->corners[3] = v2(1, 1); 
	s->low_side = v2(0, 0);
}

Sprite sprite_make(Vec2 pos, Rect2 texture, u32 flags)
{
	Sprite s;
	sprite_init(&s);
	s.pos = pos;
	s.texture = texture;
	s.size = texture.size;
	s.flags = flags;
	return s;
}

// v hold 4 vecs
void sprite_corners(Sprite* s, Vec2* v)
{
	Vec2 pos = s->pos;
	Vec2 size = s->size;
	Vec2 center = s->center;
	f32 ax = SpriteAnchorX[s->flags & 0xF];
	f32 ay = SpriteAnchorY[s->flags & 0xF];
	Vec2 anchor = v2(ax, ay);

	Vec2 tl = v2_mul(v2(-0.5, -0.5), s->corners[0]);
	Vec2 tr = v2_mul(v2(0.5, -0.5), s->corners[1]);
	Vec2 bl = v2_mul(v2(-0.5, 0.5), s->corners[2]);
	Vec2 br = v2_mul(v2(0.5, 0.5), s->corners[3]);

	v2_sub_ip(&tl, anchor);
	v2_sub_ip(&tr, anchor);
	v2_sub_ip(&bl, anchor);
	v2_sub_ip(&br, anchor);

	v2_mul_ip(&tl, size);
	v2_mul_ip(&tr, size);
	v2_mul_ip(&bl, size);
	v2_mul_ip(&br, size);

	v2_sub_ip(&tl, center);
	v2_sub_ip(&tr, center);
	v2_sub_ip(&bl, center);
	v2_sub_ip(&br, center);

	v2_rotate_ip(&tl, s->rotation);
	v2_rotate_ip(&tr, s->rotation);
	v2_rotate_ip(&bl, s->rotation);
	v2_rotate_ip(&br, s->rotation);

	v2_add_ip(&tl, pos);
	v2_add_ip(&tr, pos);
	v2_add_ip(&bl, pos);
	v2_add_ip(&br, pos);
	
	v[0] = tl;
	v[1] = tr;
	v[2] = br;
	v[3] = bl;
}

void sprite_rotate_cw90(Sprite* s)
{
	Vec2 temp = s->corners[3];     //store br
	s->corners[3] = s->corners[1]; //put tr into br
	s->corners[1] = s->corners[0]; //put tl into tr
	s->corners[0] = s->corners[2]; //put bl into tl
	s->corners[2] = temp;          //put br into bl
	for(isize i = 0; i < 4; ++i) {
		s->corners[i] = v2_swap_xy(s->corners[i]);
	}

	s->low_side = v2_rotate(s->low_side, v2(0, -1));

	f32 tx = s->size.x;
	s->size.x = s->size.y;
	s->size.y = tx;
}

void sprite_rotate_ccw90(Sprite* s)
{
	Vec2 temp = s->corners[3];     //store br
	s->corners[3] = s->corners[2]; //put tl into br
	s->corners[2] = s->corners[0]; //put tl into tr
	s->corners[0] = s->corners[1]; //put bl into tl
	s->corners[1] = temp;          //put br into bl
	for(isize i = 0; i < 4; ++i) {
		s->corners[i] = v2_swap_xy(s->corners[i]);
	}
	s->low_side = v2_rotate(s->low_side, v2(0, 1));

	f32 tx = s->size.x;
	s->size.x = s->size.y;
	s->size.y = tx;
}

void sprite_flip_horiz(Sprite* s)
{
	Vec2 temp = s->corners[3];   
	s->corners[3] = s->corners[2];
	s->corners[2] = temp;
	temp = s->corners[0];
	s->corners[0] = s->corners[1];
	s->corners[1] = temp;
}

void sprite_flip_vert(Sprite* s)
{
	Vec2 temp = s->corners[3];   
	s->corners[3] = s->corners[1];
	s->corners[1] = temp;
	temp = s->corners[0];
	s->corners[0] = s->corners[2];
	s->corners[2] = temp;
}




i32 sprite_is_aabb(Sprite* s) 
{
	i32 rotation = 0;
	if(s->rotation.x == 1 && s->rotation.y == 0) {
		rotation = 1;
	}

	i32 corners = 1;
	for(isize i = 0; i < 4; ++i) {
		if(s->corners[i].x != 1) {
			corners = 0;
			break;
		}
		if(s->corners[i].y != 1) {
			corners = 0;
			break;
		}
	}

	return rotation && corners;
}

Rect2 sprite_aabb(Sprite* s)
{
	i32 rotation = 0;
	if(s->rotation.x == 1 && s->rotation.y == 0) {
		rotation = 1;
	}

	i32 corners = 1;
	for(isize i = 0; i < 4; ++i) {
		if(s->corners[i].x != 1) {
			corners = 0;
			break;
		}
		if(s->corners[i].y != 1) {
			corners = 0;
			break;
		}
	}

	if(rotation && corners) {
		//only do anchor
		
		f32 ax = SpriteAnchorX[s->flags & 0xF];
		f32 ay = SpriteAnchorY[s->flags & 0xF];
		Vec2 anchor = v2(ax, ay);
		return rect2_v(v2_sub(s->pos, v2_mul(s->size, anchor)), s->size);
	} else {
		//have to calculate from corners
		Vec2 vc[4];
		sprite_corners(s, vc);
		Vec2 p1, p2;
		{
			f32 top = Min(vc[0].x, vc[1].x);
			f32 bottom = Min(vc[2].x, vc[3].x);
			p1.x = Min(top, bottom);
			top = Min(vc[0].y, vc[1].y);
			bottom = Min(vc[2].y, vc[3].y);
			p1.y = Min(top, bottom);
		}

		{
			f32 top = Max(vc[0].x, vc[1].x);
			f32 bottom = Max(vc[2].x, vc[3].x);
			p2.x = Max(top, bottom);
			top = Max(vc[0].y, vc[1].y);
			bottom = Max(vc[2].y, vc[3].y);
			p2.y = Max(top, bottom);
		}
		
		Vec2 size = v2_sub(p2, p1);
		Vec2 pos = v2_add_scaled(p1, size, 0.5f);
		return rect2_v(pos, size);
	}

}

void sprite_validate(Sprite* s)
{
	Vec2 corners[4];
	i32 count = 0;
	sprite_corners(s, corners);
	Sprite t = points_to_sprite(corners);
	Vec4 color = s->color;
	u32 flags = s->flags;
	Vec2 low_side = s->low_side;
	*s = t;
	s->color = color;
	s->flags = flags;
	s->low_side = low_side;
}

//TODO(will): fix this broken function
Sprite points_to_sprite(Vec2* points)
{
	Vec2 corners[4];
	i32 count = 0;
	for(isize i = 0; i < 4; ++i) {
		corners[i] = points[i];
	}

	v2_sort_on_x(corners, 4);
	Vec2 convex[8];
	convex_hull(4, corners, &count, convex);
	count--;
	if(count < 4) {
		//find the longest side
		//insert a point in the midpoint
		isize index = -1;
		f32 longest = 0;
		for(isize i = 0; i < count; ++i) {
			isize j = (i + 1) % count;
			f32 dist = v2_mag(v2_sub(convex[j], convex[i]));
			if(dist > longest) {
				index = i;
				longest = dist;
			}
		}

		Vec2 mid = v2_scale(v2_add(convex[index], convex[(index+1)%count]), 0.5f);
		for(isize i = 3; i > index; --i) {
			convex[i] = convex[i - 1];
		}
		convex[index + 1] = mid;
	}

	//Vec2 center = v2(0, 0);
	Vec2 min_pt = v2(FLT_MAX, FLT_MAX), max_pt = v2(-FLT_MAX, -FLT_MAX);
	Vec2 ysort[4];
	for(isize i = 0; i < 4; ++i) {
		//v2_add_ip(&center, convex[i]);
		min_pt = v2_min(min_pt, convex[i]);
		max_pt = v2_max(max_pt, convex[i]);
		ysort[i] = convex[i];
	}
	//v2_scale_ip(&center, 0.25f);

	//This can pick weird points to be br
	//use a hybrid approach: quadrants when easy, this thing when hard?
	v2_sort_on_y(ysort, 4);
	Vec2 br = ysort[2].x > ysort[3].x ? ysort[2] : ysort[3];
	i32 br_index = 0;
	for(isize i = 0; i < 4; ++i) {
		if(v2_eq(convex[i], br)) {
			br_index = i;
			break;
		}
	}
	corners[3] = convex[br_index++];
	corners[2] = convex[(br_index++)%4];
	corners[0] = convex[(br_index++)%4];
	corners[1] = convex[(br_index++)%4];

	const Vec2 sign[] = {
		{-1, -1}, {1, -1},
		{-1, 1},  {1, 1}
	};
	Vec2 size = v2_sub(max_pt, min_pt);
	if(size.x == 0) {
		size.x = 16;
	} 
	if(size.y == 0) {
		size.y = 16;
	}
	Vec2 hs = v2_scale(size, 0.5);
	Vec2 inv_size = v2(1.0/hs.x, 1.0/hs.y);
	Vec2 center = v2_add(min_pt, hs);
	Sprite s = {0};
	sprite_init(&s);
	s.pos = center;
	s.size = size;
	for(isize i = 0; i < 4; ++i) {
		v2_sub_ip(corners + i, center);
		v2_mul_ip(corners + i, inv_size);
		s.corners[i] = v2_mul(corners[i], sign[i]);
	}
	return s;
}

Vec2 get_centroid_from_corners(i32 count, Vec2* corners)
{
	Vec2 res = v2(0, 0);
	for(isize i = 0; i < count; ++i) {
		v2_add_ip(&res, corners[i]);
	}
	v2_scale_ip(&res, 1.0f/count);
	return res;
}

Vec2 sprite_get_centroid(Sprite* s)
{
	Vec2 corners[4];
	sprite_corners(s, corners);
	v2_sort_on_x(corners, 4);
	Vec2 convex[8];
	i32 count;
	convex_hull(4, corners, &count, convex);
	count--;
	return get_centroid_from_corners(count, convex);
}

i32 sprite_get_low_side(Sprite* s, Vec2* a, Vec2* b)
{
	Vec2 corners[4];
	sprite_corners(s, corners);
	v2_sort_on_x(corners, 4);
	Vec2 convex[8];
	i32 count;
	convex_hull(4, corners, &count, convex);
	count--;

	Vec2 line[2];
	line[0] = get_centroid_from_corners(count, convex);
	line[1] = v2_add(line[0], s->low_side);
	i32 indices[4];
	Vec2 result[5];
	i32 intersect = poly_intersect_out(2, line, count, convex, result, indices);
	if(intersect) {
		*a = result[2];
		*b = result[3];
	}
	return intersect;
}

void sprite_get_corner_heights(Sprite* s, i32 count, Vec2* convex, f32* heights)
{
	Vec2 line[2];
	line[0] = get_centroid_from_corners(count, convex);
	line[1] = v2_add(line[0], s->low_side);
	i32 indices[4];
	Vec2 result[5];
	i32 intersect = poly_intersect_out(2, line, count, convex, result, indices);
	heights[0] = 1;
	heights[1] = 1;
	heights[2] = 1;
	heights[3] = 1;
	if(intersect) {
		heights[indices[2]] = 0;
		heights[indices[3]] = 0;
	}
}


void render_group_init(RenderGroup* group, Sprite* sprites, isize capacity, i32 kind, Renderer* r)
{
	group->sprites = sprites;
	group->capacity = capacity;
	group->free_list = (SpriteFreeList*)sprites;
	group->free_list->link.next = 1;
	group->count = 0;
	group->last_filled = 0;
	group->kind = kind;

	group->texture = r->texture;
	group->texture_width = r->texture_width;
	group->texture_height = r->texture_height; 
	group->inv_texture_size = r->inv_texture_size;

	group->draw_mode = GL_TRIANGLE_STRIP;


	group->offset = v2(0, 0);
	group->scale = 1.0f;
	group->always_dirty = 1;

	group->index = r->group_count;
	group->vbo = r->vbo;
	group->vao = r->vao;

	group->tint = v4(1, 1, 1, 1);

	if(kind <= Group_RetainedDynamic) {
		_render_group_retained_init(group, r);
	} /*else if(kind == Group_Immediate) {
	}*/
}

void _render_group_retained_init(RenderGroup* g, Renderer* r)
{
	SpriteFreeList* handle = NULL;
	for(isize i = 0; i < g->capacity - 1; ++i) {
		handle = (SpriteFreeList*)(g->sprites + i);
		handle->link.is_link = SpriteFlag_Null;
		handle->link.next = i + 1;
	}
	handle = (SpriteFreeList*)(g->sprites + g->capacity - 1);
	handle->link.is_link = SpriteFlag_Null;
	handle->link.next = -1;
	if(g->kind == Group_RetainedDynamic) {
		g->always_dirty = 0;
		g->dirty = 1;

		ogl_create_vao_vbo(&g->vao, &g->vbo);
	}
}


void render_set_texture(Renderer* r, u32 texture, i32 w, i32 h)
{
	r->texture = texture;
	r->texture_width = w;
	r->texture_height = h;
	r->inv_texture_size = v2(1.0f/w, 1.0f/h);
}

void ogl_create_vao_vbo(u32* vao, u32* vbo)
{
	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);
	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);

	usize stride = sizeof(Sprite);
	usize vertex_count = 1;
	isize array_index = 0;

#define Normalize GL_FALSE
#define _vertexAttrib(size, type, name) \
	do { glVertexAttribPointer(array_index, size, type, Normalize, stride, &((Sprite*)NULL)->name); \
	glEnableVertexAttribArray(array_index); \
	glVertexAttribDivisor(array_index, vertex_count); \
	array_index++; } while(0)

	//You have to use the special "IPointer" variant for integers
	glVertexAttribIPointer(array_index, 1, GL_UNSIGNED_INT, stride, &((Sprite*)NULL)->flags);
	glEnableVertexAttribArray(array_index);
	glVertexAttribDivisor(array_index, vertex_count);
	array_index++;

	//_vertexAttrib(1, GL_FLOAT, sort);
	_vertexAttrib(2, GL_FLOAT, pos);
	_vertexAttrib(2, GL_FLOAT, size);
	_vertexAttrib(2, GL_FLOAT, center);
	_vertexAttrib(2, GL_FLOAT, rotation);
	_vertexAttrib(4, GL_FLOAT, texture);
	_vertexAttrib(4, GL_FLOAT, color);
	_vertexAttrib(4, GL_FLOAT, corners);
	_vertexAttrib(4, GL_FLOAT, corners[2]);
#undef Normalize
#undef _vertexAttrib

	glBindVertexArray(0);
}


void renderer_init(Renderer* render, string vert_source, string frag_source, i32 group_count, MemoryArena* arena)
{
	render->group_pool = pool_new("GroupPool", sizeof(RenderGroup) + sizeof(Sprite) * 8192, group_count, arena);
	render->groups = arena_push(arena, sizeof(RenderGroup*) * group_count); 
	render->group_count = 0;
	render->group_capacity = group_count;

	ogl_create_vao_vbo(&render->vao, &render->vbo);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	u32 vert_shader = glCreateShader(GL_VERTEX_SHADER);
	{
		u32 shader = vert_shader;
		string src = vert_source;

		glShaderSource(shader, 1, &src, NULL);
		glCompileShader(shader);
		u32 success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		i32 log_size;
		char shader_log[4096];
		glGetShaderInfoLog(shader, 4096, &log_size, shader_log);
		if(!success) {
			log_error("Error: could not compile vertex shader\n\n%s\n\n", shader_log);
		} else {
			printf("Vertex shader compiled successfully \n");
		}
	}

	u32 frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	{
		u32 shader = frag_shader;
		string src = frag_source;

		glShaderSource(shader, 1, &src, NULL);
		glCompileShader(shader);
		u32 success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		i32 log_size;
		char shader_log[4096];
		glGetShaderInfoLog(shader, 4096, &log_size, shader_log);
		if(!success) {
			log_error("Error: could not compile fragment shader\n\n%s\n\n", shader_log);
		} else {
			printf("Frag shader compiled successfully \n");
		}
		
	}
	render->shaders = glCreateProgram();
	glAttachShader(render->shaders, vert_shader);
	glAttachShader(render->shaders, frag_shader);
	glLinkProgram(render->shaders);
	glUseProgram(render->shaders);
	glDeleteShader(vert_shader);
	glDeleteShader(frag_shader);

	render->u_texture_size = glGetUniformLocation(render->shaders, "u_texture_size");
	render->u_ortho_matrix = glGetUniformLocation(render->shaders, "u_ortho_matrix");
	render->u_scale = glGetUniformLocation(render->shaders, "u_scale");
	render->u_index_mode = glGetUniformLocation(render->shaders, "u_index_mode");
	render->u_tint = glGetUniformLocation(render->shaders, "u_tint");
}


RenderGroup* render_add_group(Renderer* r, i32 kind)
{
	RenderGroup* g = pool_retrieve(r->group_pool);
	render_group_init(g, (void*)(g + 1), 4096, kind, r);
	r->groups[r->group_count++] = g;
	return g;
}

void render_remove_group(Renderer* r, RenderGroup* g)
{
	i32 index = g->index;
	if(g->vbo != r->vbo && g->vbo != 0) {
		glBindVertexArray(0);
		glDeleteBuffers(1, &g->vbo);
		glDeleteVertexArrays(1, &g->vao);

	}

	pool_release(r->group_pool, &g);
	r->group_count--;
	r->groups[index] = r->groups[r->group_count];
}


RenderGroup* get_group(Renderer* r, i32 i) 
{
	if(i < 0 || i >= r->group_count) return NULL;
	return r->groups[i];
}

Sprite* render_get_sprite(RenderGroup* group)
{
	if(group->kind == Group_Immediate) {
		group->last_filled = group->count;
		Sprite* s = group->sprites + group->count++;
		sprite_init(s);
		return s;
	} else {

		Sprite* s = &group->free_list->s;
		isize index = ((isize)group->free_list - (isize)group->sprites) / sizeof(Sprite);
		group->free_list = (SpriteFreeList*)group->sprites + group->free_list->link.next;

		sprite_init(s);
		group->count++;
		if(index > group->last_filled) {
			group->last_filled = index;
		}
		group->dirty = 1;

		return s;
	}
}

Sprite* render_create_sprite(RenderGroup* group, Vec2 pos, Rect2 texture)
{
	Sprite* s = render_get_sprite(group);
	*s = sprite_make(pos, texture, 0);
	v2_mul_ip(&s->texture.pos, group->inv_texture_size);
	v2_mul_ip(&s->texture.size, group->inv_texture_size);
	return s;
}

Sprite* render_create_primitive(RenderGroup* group, Vec2 pos, Vec2 size, Vec4 color, u32 flags)
{
	Sprite* s = render_get_sprite(group);
	*s = sprite_make(pos, rect2(0, 0, 0, 0), SpriteFlag_Primitive | flags);
	s->color = color;
	s->size = size;

	return s;
}

Sprite* render_group_query(RenderGroup* g, Vec2 local_pt)
{
	Vec2 corners[4];
	for(isize i = g->last_filled; i >= 0; --i) {
		Sprite* s = g->sprites + i;
		if(HasFlag(s->flags, SpriteFlag_Null)) continue;
		sprite_corners(s, corners);
		if(poly_contains_point(local_pt, 4, corners)) {
			return s;
		}
	}

	return NULL;
}

isize render_group_query_aabb(RenderGroup* g, Rect2 q, isize max_out, Sprite** out)
{
	v2_scale_ip(&q.size, 0.5f);
	Vec2 tl = v2_sub(q.pos, q.size), br = v2_add(q.pos, q.size);
	Vec2 qcorners[4];
	qcorners[0] = tl;
	qcorners[1] = v2(br.x, tl.y);
	qcorners[2] = br;
	qcorners[3] = v2(tl.x, br.y);
	isize len = 0;
	for(isize i = g->last_filled; i >= 0; --i) {
		Sprite* s = g->sprites + i;
		if(HasFlag(s->flags, SpriteFlag_Null)) continue;
		Rect2 paabb = sprite_aabb(s);
		v2_scale_ip(&paabb.size, 0.5f);
		if(aabb_intersect(&q, &paabb)) {
			if(sprite_is_aabb(s)) {
				if(len + 1 >= max_out) return len;
				out[len++] = s;
			} else if(aabb_contains(&q, &paabb)) {
				if(len + 1 >= max_out) return len;
				out[len++] = s;
			} else {
				Vec2 pcorners[4];
				sprite_corners(s, pcorners);

				if(poly_intersect(4, qcorners, 4, pcorners)) {
					if(len + 1 >= max_out) return len;
					out[len++] = s;
				}
			}
		}
	}
	return len;
}



void render_remove_sprite(RenderGroup* group, Sprite* s)
{
	if(group->kind == Group_Immediate) {
		return;
	}
	isize sp = (isize)s;
	isize sprites = (isize)group->sprites;
	if(sp < sprites || sp > sprites + group->capacity * isizeof(Sprite)) {
		return;
	}

	//okay, so we know s is in the pool
	group->count--;
	group->dirty = 1;
	isize index = (sp - sprites) / sizeof(Sprite);
	SpriteFreeList* l = (void*)s;
	l->link.is_link = SpriteFlag_Null;
	l->link.next = ((isize)group->free_list - sprites) / sizeof(Sprite);
	group->free_list = l;

	printf("Prev LF: %d ", group->last_filled);
	if(index == group->last_filled) {
		index--;
		while(group->sprites[index].flags & SpriteFlag_Null) index--;
		group->last_filled = index;
	}
	printf("Now: %d\n", group->last_filled);
}

void render_calculate_ortho_matrix(f32* ortho, Vec4 screen, float nearplane, float farplane)
{
	//v4 == x, y, z, w;
	//   == l, t, r, b
	ortho[0] = 2.0f / (screen.z - screen.x);
	ortho[1] = 0;
	ortho[2] = 0;
	ortho[3] = -1.0f * (screen.x + screen.z) / (screen.z - screen.x);

	ortho[4] = 0;
	ortho[5] = 2.0f / (screen.y - screen.w);
	ortho[6] = 0;
	ortho[7] = -1 * (screen.y + screen.w) / (screen.y - screen.w);

	ortho[8] = 0;
	ortho[9] = 0;
	ortho[10] = (-2.0f / (farplane - nearplane));
	ortho[11] = (-1.0f * (farplane + nearplane) / (farplane - nearplane));

	ortho[12] = 0;
	ortho[13] = 0;
	ortho[14] = 0;
	ortho[15] = 1.0f;
}


void render_clear(RenderGroup* group)
{
	group->count = 0;

	if(group->kind <= Group_RetainedDynamic) {
		SpriteFreeList* handle = group->free_list;
		for(isize i = 0; i < group->last_filled - 1; ++i) {
			sprite_init(&handle->s);
			handle->link.is_link = SpriteFlag_Null;
			handle->link.next = i + 1;
			handle++;
		}
		handle->link.next = -1;
		group->last_filled = 0;
	}
}

void render_draw(Renderer* r, RenderGroup* group, Vec2 size, f32 scale)
{
	if(group->count == 0) return;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(r->shaders);
	group->offset.x = roundf(group->offset.x);
	group->offset.y = roundf(group->offset.y);
	scale /= group->scale;
	v2_scale_ip(&size, scale);

	glUniform1f(r->u_scale, scale);
	glUniform2f(r->u_texture_size, group->texture_width, group->texture_height);
	glUniform4f(r->u_tint, group->tint.x, group->tint.y, group->tint.z, group->tint.w);

	Vec4 screen = v4(
		group->offset.x, group->offset.y,
		group->offset.x + size.x, group->offset.y + size.y);
	render_calculate_ortho_matrix(group->ortho, screen, 1, -1);
	glUniformMatrix4fv(r->u_ortho_matrix,
		1, GL_FALSE, group->ortho);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, group->texture);
	glBindVertexArray(group->vao);
	glBindBuffer(GL_ARRAY_BUFFER, group->vbo);

	if(group->always_dirty) group->dirty = 1;
	if(group->dirty) {
		u32 draw = group->kind == Group_RetainedDynamic ? GL_DYNAMIC_DRAW : GL_STREAM_DRAW;
		glBufferData(GL_ARRAY_BUFFER,
			(group->last_filled+1) * sizeof(Sprite),
			group->sprites,
			draw);
		group->dirty = 0;
	}

	if(group->draw_mode == GL_TRIANGLE_STRIP) {
		glUniform1i(r->u_index_mode, 0);
	} else {
		glUniform1i(r->u_index_mode, 1);
	}

	glDrawArraysInstanced(group->draw_mode, 0, 4, group->last_filled+1);

	glBindVertexArray(0);
	
	if(group->kind == Group_Immediate) {
		render_clear(group);
	}
}
GLuint ogl_add_texture(u8* data, isize w, isize h)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	if(w <= 0) w = 1;
	if(h <= 0) h = 1;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	u32 error = glGetError();
	if(error != 0) {
		printf("There was an error adding a texture: %d \n", error);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}

#if 0 
// we load everything from memory
GLuint ogl_load_texture(char* filename, isize* w_o, isize* h_o)
{
	int w, h, n;
	char file[4096];
	char* base_path = SDL_GetBasePath();
	isize len = snprintf(file, 4096, "%s%s", base_path, filename);
	u8* data = (u8*)stbi_load(file, &w, &h, &n, STBI_rgb_alpha);
	GLuint texture = ogl_add_texture(data, w, h);
	if(texture == 0) {
		log_error("Error: could not load %s \n", filename);
	}
	if(w_o != NULL) *w_o = w;
	if(h_o != NULL) *h_o = h;

	SDL_free(base_path);
	STBI_FREE(data);
	return texture;
}
#endif

GLuint ogl_load_texture_from_memory(u8* buf, i32 size, i32* x, i32* y)
{
	i32 w, h, n;
	u8* data = stbi_load_from_memory(buf, size, &w, &h, &n, STBI_rgb_alpha);
	//printf("%s\n", stbi_failure_reason());
	
	GLuint texture = ogl_add_texture(data, w, h);
	if(texture == 0) {
		log_error("Error loading texture\n");
	}
	if(x) *x = w;
	if(y) *y = h;

	STBI_FREE(data);
	return texture;
}

Sprite sprite_make_box(Vec2 pos, Vec2 size, Vec4 color, u32 flags)
{
	Sprite s;
	sprite_init(&s);
	s.pos = pos;
	s.size = size;
	s.color = color;
	s.flags = flags | SpriteFlag_Primitive;
	return s;
}


Sprite sprite_make_line(Vec2 start, Vec2 end, Vec4 color, i32 thickness, u32 flags)
{
	Vec2 dline;
	dline.x = end.x - start.x;
	dline.y = end.y - start.y;
	Sprite s;
	if(dline.y == 0) {
		if(dline.x < 0) {
			dline.x *= -1;
			Vec2 temp = end;
			end = start;
			start = temp;
		}
		Vec2 lstart = start;
		lstart.x += dline.x / 2.0f;
		s = sprite_make_box(lstart, v2(dline.x, thickness), color, flags);
	} else if(dline.x == 0) {
		if(dline.y < 0) {
			dline.y *= -1;
			Vec2 temp = end;
			end = start;
			start = temp;
		}
		Vec2 lstart = start;
		lstart.y += dline.y / 2;
		s = sprite_make_box(lstart, v2(thickness, dline.y), color, flags);
	} else {
		Vec2 lstart = start;
		f32 mag = v2_mag(dline);
		lstart.x += dline.x / 2.0f;
		lstart.y += dline.y / 2.0f;

		s = sprite_make_box(lstart, v2(mag, thickness), color, flags);
		f32 angle = atan2f(dline.y, dline.x);
		s.rotation = v2_from_angle(-angle, 1);
	}
	s.flags = flags;
	return s;
}
