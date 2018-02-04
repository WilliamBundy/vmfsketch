
/*
 * So, this is a very lazy serialization strategy
 * but it'll work, lol
 *
 */


#define RW_Pair_For_Type(type) \
void write_##type(type v, FILE* f) \
{ \
	fwrite(&v, sizeof(type), 1, f); \
} \
f32 read_get_##type(FILE* f) \
{ \
	type v; \
	fread(&v, sizeof(type), 1, f); \
	return v; \
}
typedef int64_t i64;

RW_Pair_For_Type(f32)
RW_Pair_For_Type(u32)
RW_Pair_For_Type(i32)
RW_Pair_For_Type(i64)
RW_Pair_For_Type(u8)
#define read_f32(lvalue, file) lvalue = read_get_f32(file)
#define read_u32(lvalue, file) lvalue = read_get_u32(file)
#define read_i32(lvalue, file) lvalue = read_get_i32(file)
#define read_i64(lvalue, file) lvalue = read_get_i64(file)
#define read_u8(lvalue, file) lvalue = read_get_u8(file)


void write_layer_to_file(EditorLayer* l, FILE* f)
{
	write_i32(l->name_len, f);
	fwrite(l->name, sizeof(char), 64, f);
	write_f32(l->pos.x, f);
	write_f32(l->pos.y, f);
	write_f32(l->pos.z, f);
	write_f32(l->rotation, f);
	write_f32(l->depth, f);
	write_f32(l->rise, f);
	write_f32(l->color.x, f);
	write_f32(l->color.y, f);
	write_f32(l->color.z, f);
	write_f32(l->color.w, f);

	write_i64(l->brushes->last_filled, f);
	for(isize i = 0; i <= l->brushes->last_filled; ++i) {
		write_sprite_to_file(l->brushes->sprites + i, f);
	}
}

void write_sprite_to_file(Sprite* s, FILE* f)
{
	write_u32(s->flags, f);
	write_i32(s->sort, f);
	write_f32(s->pos.x, f);
	write_f32(s->pos.y, f);
	write_f32(s->size.x, f);
	write_f32(s->size.y, f);
	write_f32(s->center.x, f);
	write_f32(s->center.y, f);
	write_f32(s->rotation.x, f);
	write_f32(s->rotation.y, f);
	write_f32(s->texture.pos.x, f);
	write_f32(s->texture.pos.y, f);
	write_f32(s->texture.size.x, f);
	write_f32(s->texture.size.y, f);
	write_f32(s->color.x, f);
	write_f32(s->color.y, f);
	write_f32(s->color.z, f);
	write_f32(s->color.w, f);
	write_f32(s->corners[0].x, f);
	write_f32(s->corners[0].y, f);
	write_f32(s->corners[1].x, f);
	write_f32(s->corners[1].y, f);
	write_f32(s->corners[2].x, f);
	write_f32(s->corners[2].y, f);
	write_f32(s->corners[3].x, f);
	write_f32(s->corners[3].y, f);
	write_f32(s->low_side.x, f);
	write_f32(s->low_side.y, f);
}

void read_layer(EditorLayer* l, FILE* f)
{
	read_i32(l->name_len, f);
	fread(l->name, sizeof(char), 64, f);
	read_f32(l->pos.x, f);
	read_f32(l->pos.y, f);
	read_f32(l->pos.z, f);
	read_f32(l->rotation, f);
	read_f32(l->depth, f);
	read_f32(l->rise, f);
	read_f32(l->color.x, f);
	read_f32(l->color.y, f);
	read_f32(l->color.z, f);
	read_f32(l->color.w, f);

	read_i64(l->brushes->last_filled, f);
	for(isize i = 0; i <= l->brushes->last_filled; ++i) {
		Sprite s; 
		read_sprite(&s, f);
		if(!HasFlag(s.flags, SpriteFlag_Null)) {
			Sprite* sp = render_get_sprite(l->brushes);
			*sp = s;
		}
	}
}

void read_sprite(Sprite* s, FILE* f)
{
	read_u32(s->flags, f);
	read_i32(s->sort, f);
	read_f32(s->pos.x, f);
	read_f32(s->pos.y, f);
	read_f32(s->size.x, f);
	read_f32(s->size.y, f);
	read_f32(s->center.x, f);
	read_f32(s->center.y, f);
	read_f32(s->rotation.x, f);
	read_f32(s->rotation.y, f);
	read_f32(s->texture.pos.x, f);
	read_f32(s->texture.pos.y, f);
	read_f32(s->texture.size.x, f);
	read_f32(s->texture.size.y, f);
	read_f32(s->color.x, f);
	read_f32(s->color.y, f);
	read_f32(s->color.z, f);
	read_f32(s->color.w, f);
	read_f32(s->corners[0].x, f);
	read_f32(s->corners[0].y, f);
	read_f32(s->corners[1].x, f);
	read_f32(s->corners[1].y, f);
	read_f32(s->corners[2].x, f);
	read_f32(s->corners[2].y, f);
	read_f32(s->corners[3].x, f);
	read_f32(s->corners[3].y, f);
	read_f32(s->low_side.x, f);
	read_f32(s->low_side.y, f);
}
