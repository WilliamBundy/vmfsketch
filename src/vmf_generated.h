typedef struct Vec2 Vec2;
typedef struct Vec3 Vec3;
typedef struct Vec4 Vec4;
typedef struct Vec2i Vec2i;
typedef struct Color Color;
typedef struct Rect2 Rect2;
typedef struct Rect3 Rect3;
typedef struct Rect2i Rect2i;
typedef struct MemoryArena MemoryArena;
typedef struct PoolHandle PoolHandle;
typedef struct MemoryPool MemoryPool;
typedef struct Sprite Sprite;
typedef union SpriteFreeList SpriteFreeList;
typedef struct RenderGroup RenderGroup;
typedef struct Renderer Renderer;
typedef struct vmfSettings vmfSettings;
typedef struct vmfAppMain vmfAppMain;
typedef struct EditorKeys EditorKeys;
typedef struct EditorModeData EditorModeData;
typedef struct EditorBrush EditorBrush;
typedef struct EditorLayer EditorLayer;
typedef struct Editor Editor;
typedef struct VmfContext VmfContext;
typedef struct Plane Plane;
typedef enum SpriteFlags SpriteFlags;
typedef enum RenderGroupKind RenderGroupKind;
typedef enum ButtonState ButtonState;
typedef enum EditorAction EditorAction;
typedef enum EditorFocus EditorFocus;
typedef enum EditorMode EditorMode;
typedef const char* string;
typedef const char* string;
typedef const char* string;
typedef enum SpriteFlags{ Anchor_Center=0, Anchor_TopLeft=1, Anchor_Top=2, Anchor_TopRight=3, Anchor_Right=4, Anchor_BottomRight=5, Anchor_Bottom=6, Anchor_BottomLeft=7, Anchor_Left=8, SpriteFlag_FlipHoriz= Flag(4), SpriteFlag_FlipVert= Flag(5), SpriteFlag_Primitive= Flag(6), SpriteFlag_Null= Flag(10)} SpriteFlags;
typedef enum RenderGroupKind{ Group_RetainedStream, Group_RetainedDynamic, Group_Immediate} RenderGroupKind;
typedef enum ButtonState{ Button_JustReleased=-1, Button_Released, Button_Pressed, Button_JustPressed} ButtonState;
typedef enum EditorAction{ Action_SelectOnlyBrush, Action_SelectAddBrush, Action_SelectGroup, Action_DeselectBrush, Action_DeselectAll, Action_CopySelection, Action_CutSelection, Action_Paste, Action_CreateBrush, Action_MoveSelected, Action_ResizeBrush, Action_RotateSelected} EditorAction;
typedef enum EditorFocus{ Focus_None, Focus_Editor, Focus_Panel} EditorFocus;
typedef enum EditorMode{ Mode_Null=-1, Mode_Normal=0, Mode_Edit, Mode_Pan, Mode_Vertex, Mode_Cut,} EditorMode;
typedef int64_t i64;
typedef const unsigned char* glGetStringProc( unsigned int name);
struct Vec2
{
	f32 x;
	f32 y;
};
struct Vec3
{
	f32 x;
	f32 y;
	f32 z;
};
struct Vec4
{
	f32 x;
	f32 y;
	f32 z;
	f32 w;
};
struct Vec2i
{
	i32 x;
	i32 y;
};
struct Color
{
	u8 r;
	u8 g;
	u8 b;
	u8 a;
};
struct Rect2
{
	Vec2 pos;
	Vec2 size;
};
struct Rect3
{
	Vec3 pos;
	Vec3 size;
};
struct Rect2i
{
	i32 x;
	i32 y;
	i32 w;
	i32 h;
};
struct MemoryArena
{
	string name;
	u8 *data;
	u8 *head;
	u8 *temp_head;
	isize size;
};
struct PoolHandle
{
	struct PoolHandle *next;
};
struct MemoryPool
{
	isize element_size;
	void *slots;
	isize count;
	isize capacity;
	string name;
	PoolHandle *free_list;
};
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
	struct {
		u32 is_link;
		i32 next;
	} link;
	Sprite s;
};
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
	SpriteFreeList *free_list;
	isize last_filled;
	i32 dirty;
	i32 always_dirty;
	isize count;
	isize capacity;
	Sprite *sprites;
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
	MemoryPool *group_pool;
	RenderGroup **groups;
	isize group_count;
	isize group_capacity;
};
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
	char *base_path;
	SDL_Window *window;
	struct {
		MemoryArena *base;
		MemoryArena *render;
		MemoryArena *frame;
	} memory;
	mz_zip_archive assets;
	i32 *keys;
	i32 *mouse;
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
		struct nk_font *title;
		struct nk_font *body;
		struct nk_font *fixed;
		struct nk_font *small;
	} fonts;
};
struct EditorKeys
{
	struct {
		i32 bigger;
		i32 smaller;
	} grid;
	struct {
		i32 up;
		i32 down;
		i32 left;
		i32 right;
		i32 center;
		i32 grid;
	} pan;
	struct {
		i32 back;
		i32 edit;
		i32 pan;
		i32 vert;
		i32 cut;
		i32 layer;
	} mode;
	struct {
		i32 undo;
		i32 cut;
		i32 copy;
		i32 paste;
		i32 delete;
		i32 rotate;
		i32 flip;
		i32 layer;
	} action;
};
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
		Sprite *resizing;
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
	RenderGroup *brushes;
};
struct Editor
{
	struct {
		RenderGroup *grid;
		RenderGroup *overlay;
		RenderGroup *layers;
		RenderGroup *itext;
		RenderGroup *stext;
		i32 layer_count;
	} groups;
	Rect2 screen_bounds;
	Vec2 camera;
	i32 grid_size;
	f32 zoom;
	f32 zoom_z;
	Vec2 anchors[16];
	i32 anchor_count;
	Sprite **selected;
	Vec2 selected_center;
	Vec2 selected_size;
	i32 selected_count;
	Sprite *clipboard;
	Vec2 clipboard_center;
	i32 clipboard_count;
	i32 input_block;
	i32 mode;
	i32 last_mode;
	EditorModeData modes;
	i32 focus;
	struct {
		RenderGroup *bg;
		RenderGroup *grid;
		RenderGroup *layers;
		RenderGroup *overlay;
		Rect2 bounds;
		i32 open;
		f32 height;
		Vec2 camera;
		f32 zoom;
	} layer_menu;
	EditorLayer *current;
	EditorLayer *layers;
	i32 layer_count;
	i32 layer_capacity;
	i32 confirm;
	i32 reset_windows_waiting;
	EditorKeys keys;
};
struct VmfContext
{
	string name;
	FILE *file;
	i32 next_id;
	i32 next_side_id;
	i32 indent;
};
struct Plane
{
	Vec3 points[3];
	Vec4 uv[2];
	i32 vert;
};
Vec3 v3(f32 x, f32 y, f32 z);
Vec3 v3_add(Vec3 a, Vec3 b);
Vec4 v4(f32 x, f32 y, f32 z, f32 w);
Vec2i v2i(i32 x, i32 y);
i32 segment_intersect(Vec2 a1, Vec2 a2, Vec2 b1, Vec2 b2, Vec2* result);
i32 f_segment_intersect(Vec2 a1, Vec2 a2, Vec2 b1, Vec2 b2, Vec2* result);
i32 poly_intersect(i32 p1c, Vec2* p1, i32 p2c, Vec2* p2);
i32 poly_intersect_out(i32 p1c, Vec2* p1, i32 p2c, Vec2* p2, Vec2* edges, i32* indices);
i32 poly_contains_point(Vec2 p, i32 side_count, Vec2* poly);
void convex_hull(i32 count, Vec2* points, i32* out_count, Vec2* out);
void arena_init(MemoryArena* arena, string name, isize size);
MemoryArena* arena_bootstrap(string name, isize size);
void* arena_push(MemoryArena* arena, isize size);
void arena_start_temp(MemoryArena* arena);
void arena_end_temp(MemoryArena* arena);
void arena_clear(MemoryArena** arena);
void* arena_alloc_wrapper(isize size, void* userdata);
Allocator arena_allocator(MemoryArena* arena);
MemoryArena arena_free(MemoryArena* arena);
void pool_init(MemoryPool* pool, string name, isize element_size, isize capacity, MemoryArena* arena);
MemoryPool* pool_new(string name, isize element_size, isize capacity, MemoryArena* arena);
void* pool_retrieve(MemoryPool* pool);
void pool_release(MemoryPool* pool, void** iptr);
void pool_print(MemoryPool* pool);
void pool_clear(MemoryPool* pool, i8 clear);
void sprite_init(Sprite* s);
Sprite sprite_make(Vec2 pos, Rect2 texture, u32 flags);
void sprite_corners(Sprite* s, Vec2* v);
void sprite_rotate_cw90(Sprite* s);
void sprite_rotate_ccw90(Sprite* s);
void sprite_flip_horiz(Sprite* s);
void sprite_flip_vert(Sprite* s);
i32 sprite_is_aabb(Sprite* s);
Rect2 sprite_aabb(Sprite* s);
void sprite_validate(Sprite* s);
Sprite points_to_sprite(Vec2* points);
Vec2 get_centroid_from_corners(i32 count, Vec2* corners);
Vec2 sprite_get_centroid(Sprite* s);
i32 sprite_get_low_side(Sprite* s, Vec2* a, Vec2* b);
void sprite_get_corner_heights(Sprite* s, i32 count, Vec2* convex, f32* heights);
void render_group_init(RenderGroup* group, Sprite* sprites, isize capacity, i32 kind, Renderer* r);
void _render_group_retained_init(RenderGroup* g, Renderer* r);
void render_set_texture(Renderer* r, u32 texture, i32 w, i32 h);
void ogl_create_vao_vbo(u32* vao, u32* vbo);
void renderer_init(Renderer* render, string vert_source, string frag_source, i32 group_count, MemoryArena* arena);
RenderGroup* render_add_group(Renderer* r, i32 kind);
void render_remove_group(Renderer* r, RenderGroup* g);
RenderGroup* get_group(Renderer* r, i32 i);
Sprite* render_get_sprite(RenderGroup* group);
Sprite* render_create_sprite(RenderGroup* group, Vec2 pos, Rect2 texture);
Sprite* render_create_primitive(RenderGroup* group, Vec2 pos, Vec2 size, Vec4 color, u32 flags);
Sprite* render_group_query(RenderGroup* g, Vec2 local_pt);
isize render_group_query_aabb(RenderGroup* g, Rect2 q, isize max_out, Sprite** out);
void render_remove_sprite(RenderGroup* group, Sprite* s);
void render_calculate_ortho_matrix(f32* ortho, Vec4 screen, float nearplane, float farplane);
void render_clear(RenderGroup* group);
void render_draw(Renderer* r, RenderGroup* group, Vec2 size, f32 scale);
GLuint ogl_add_texture(u8* data, isize w, isize h);
GLuint ogl_load_texture(char* filename, isize* w_o, isize* h_o);
GLuint ogl_load_texture_from_memory(u8* buf, i32 size, i32* x, i32* y);
Sprite sprite_make_box(Vec2 pos, Vec2 size, Vec4 color, u32 flags);
Sprite sprite_make_line(Vec2 start, Vec2 end, Vec4 color, i32 thickness, u32 flags);
Vec2 size_text(nk_font* font, i32 len, string text);
void render_text(RenderGroup* group, nk_font* nkfont, Vec2 pos, i32 len, string text);
void render_string(RenderGroup* group, nk_font* font, Vec2 pos, string text);
void render_string_c(RenderGroup* group, nk_font* font, Vec2 pos, string text);
void* get_asset(string asset_name, isize* size_out);
void app_set_scale(f32 scale);
void update_mousepos();
i32 init_app(vmfSettings settings);
void app_run();
void editor_layer_init(Editor* ed, EditorLayer* l);
void editor_start(Editor* ed, MemoryArena* arena);
void write_sprite_to_file(Sprite* s, FILE* f);
void read_layer(EditorLayer* l, FILE* f);
void read_sprite(Sprite* s, FILE* f);
void render_clip(Rect2 viewport);
void render_end_clip();
void editor_clear_focus(Editor* ed);
void editor_save_to_file(Editor* ed, string filename);
void editor_open_from_file(Editor* ed, string filename);
void editor_add_from_file(Editor* ed, string filename);
void editor_change_grid_size(Editor* ed, i32 new_grid);
EditorLayer* editor_get_layer(Editor* ed);
void editor_remove_layer(Editor* ed, EditorLayer* l);
void editor_assign_layer_ids(Editor* ed);
Vec2 editor_round_to_grid(Editor* ed, Vec2 pt);
Vec2 editor_floor_to_grid(Editor* ed, Vec2 pt);
Vec2 editor_int_to_grid(Editor* ed, Vec2 pt);
Vec2 editor_get_screen_pt(Editor* ed, Vec2 pt);
i32 editor_brush_is_selected(Editor* ed, Sprite* s);
void editor_select(Editor* ed, Sprite* s);
void editor_select_only(Editor* ed, Sprite* s);
void editor_deselect(Editor* ed, Sprite* s);
void editor_clear_selected(Editor* ed);
void editor_copy_selected_to_clipboard(Editor* ed);
void editor_calc_selected_center(Editor* ed);
void editor_paste_clipboard(Editor* ed);
void editor_get_resize_handles(Editor* ed, Sprite* s, Rect2* r);
i32 editor_check_resize_handles(Editor* ed, Sprite* s);
void editor_get_vertex_handles(Editor* ed, Sprite* s, Rect2* r);
Rect2 editor_get_low_side_handle(Editor* ed, Sprite* s);
i32 editor_check_vertex_handles(Editor* ed, Sprite* s);
void editor_draw_pos_size_text(Editor* ed, Vec2 pos, Vec2 size);
void editor_update(Editor* ed);
void editor_update(Editor* ed);
void editor_change_grid_size(Editor* ed, i32 new_grid);
Vec3 v3ify(Vec2 v, f32 z, Vec3 offset);
isize sprite_get_planes(Sprite* s, f32 depth, f32 rise, Vec3 offset, Plane* planes);
void vmfctx_init(VmfContext* ctx, string name);
void vmfctx_write_indent(VmfContext* ctx);
void vmfctx_add_line(VmfContext* ctx, string name);
void vmfctx_add_property(VmfContext* ctx, string name, string value);
void vmfctx_add_iproperty(VmfContext* ctx, string name, int value);
void vmfctx_add_plane(VmfContext* ctx, string name, Plane* p);
void vmfctx_add_v4uv(VmfContext* ctx, string name, Vec4 uv);
void vmfctx_start(VmfContext* ctx, EditorLayer* layers, isize layer_count);
void vmfctx_add_world_geom(VmfContext* ctx, EditorLayer* layers, i32 count);
void vmfctx_end(VmfContext* ctx);
void start();
void update();
int main(int argc, char** argv);
