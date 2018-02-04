#include "vmf_defines.h"


#ifndef REFLECTED
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

#endif



static inline
Vec2 v2(f32 x, f32 y)
{
	Vec2 v;
	v.x = x;
	v.y = y;
	return v;
}

static inline
Vec2 v2_add(Vec2 a, Vec2 b)
{
	Vec2 v;
	v.x = a.x + b.x;
	v.y = a.y + b.y;
	return v;
}

static inline
Vec2 v2_add_scaled(Vec2 a, Vec2 b, f32 f)
{
	Vec2 v;
	v.x = a.x + b.x * f;
	v.y = a.y + b.y * f;
	return v;
}

static inline
Vec2 v2_negate(Vec2 a)
{
	Vec2 v;
	v.x = -a.x;
	v.y = -a.y;
	return v;
}

static inline
Vec2 v2_sub(Vec2 a, Vec2 b)
{
	Vec2 v;
	v.x = a.x - b.x;
	v.y = a.y - b.y;
	return v;
}

static inline
Vec2 v2_scale(Vec2 a, f32 f)
{
	Vec2 v;
	v.x = a.x * f;
	v.y = a.y * f;
	return v;
}

static inline
Vec2 v2_mul(Vec2 a, Vec2 b)
{
	Vec2 v;
	v.x = a.x * b.x;
	v.y = a.y * b.y;
	return v;
}

static inline
f32 v2_mag2(Vec2 a)
{
	return a.x * a.x + a.y * a.y;
}

static inline
f32 v2_mag(Vec2 a)
{
	return sqrtf(v2_mag2(a));
}

static inline
f32 v2_dot(Vec2 a, Vec2 b)
{
	return a.x * b.x + a.y * b.y;
}

static inline
f32 v2_cross(Vec2 a, Vec2 b)
{
	return a.x * b.y - a.y * b.x;
}

static inline
f32 v2_cross_origin(Vec2 a, Vec2 b, Vec2 o)
{
	a = v2_sub(a, o);
	b = v2_sub(b, o);
	return a.x * b.y - a.y * b.x;
}

static inline
Vec2 v2_normalize(Vec2 a)
{
	Vec2 v;
	f32 mag = v2_mag(a);
	v.x = a.x / mag;
	v.y = a.y / mag;
	return v;
}

static inline
Vec2 v2_from_angle(f32 angle, f32 mag)
{
	Vec2 v;
	v.x = cosf(angle) * mag;
	v.y = sinf(angle) * mag;
	return v;
}

static inline
f32 v2_to_angle(Vec2 a)
{
	return atan2f(a.y, a.x);
}

static inline 
Vec2 v2_perpendicular(Vec2 a)
{
	Vec2 v;
	v.x = -a.y;
	v.y = a.x;
	return v;
}

static inline
Vec2 v2_abs(Vec2 a)
{
	Vec2 v;
	v.x = fabsf(a.x);
	v.y = fabsf(a.y);
	return v;
}

static inline
Vec2 v2_min(Vec2 a, Vec2 b)
{
	Vec2 v;
	v.x = Min(a.x, b.x);
	v.y = Min(a.y, b.y);
	return v;
}

static inline
Vec2 v2_max(Vec2 a, Vec2 b)
{
	Vec2 v;
	v.x = Max(a.x, b.x);
	v.y = Max(a.y, b.y);
	return v;
}



static inline
Vec2 v2_swap_xy(Vec2 a)
{
	Vec2 v;
	v.x = a.y;
	v.y = a.x;
	return v;
}

static inline
Vec2 v2_rotate(Vec2 a, Vec2 rot)
{
	Vec2 v;
	v.x = rot.x * a.x + rot.y * a.y;
	v.y = -rot.y * a.x + rot.x * a.y;
	return v;
}

static inline
Vec2 v2_round(Vec2 a)
{
	Vec2 v;
	v.x = roundf(a.x);
	v.y = roundf(a.y);
	return v;
}

static inline
Vec2 v2_floor(Vec2 a)
{
	Vec2 v;
	v.x = floorf(a.x);
	v.y = floorf(a.y);
	return v;
}

//warning: uses float ==
//for assignment comparisons only
static inline
i32 v2_eq(Vec2 a, Vec2 b)
{
	return a.x == b.x && a.y == b.y;
}

/* "ip" or "In-Place" functions modify the first parameter */

static inline
void v2_add_ip(Vec2* a, Vec2 b)
{
	a->x += b.x;
	a->y += b.y;
}

static inline
void v2_add_scaled_ip(Vec2* a, Vec2 b, f32 f)
{
	a->x += b.x * f;
	a->y += b.y * f;
}

static inline
void v2_negate_ip(Vec2* a)
{
	a->x *= -1;
	a->y *= -1;
}

static inline
void v2_sub_ip(Vec2* a, Vec2 b)
{
	a->x -= b.x;
	a->y -= b.y;
}

static inline
void v2_scale_ip(Vec2* a, f32 f)
{
	a->x *= f;
	a->y *= f;
}

static inline
void v2_mul_ip(Vec2* a, Vec2 b)
{
	a->x *= b.x;
	a->y *= b.y;
}

static inline
void v2_normalize_ip(Vec2* a)
{
	f32 mag = v2_mag(*a);
	a->x /= mag;
	a->y /= mag;
}

static inline
void v2_perpendicular_ip(Vec2* a)
{
	f32 x = a->x;
	a->x = -a->y;
	a->y = x;
}

static inline
void v2_rotate_ip(Vec2* b, Vec2 rot)
{
	Vec2 a = *b;
	b->x = rot.x * a.x + rot.y * a.y;
	b->y = -rot.y * a.x + rot.x * a.y;
}

static inline
void v2_round_ip(Vec2* a)
{
	a->x = roundf(a->x);
	a->y = roundf(a->y);
}

static inline
void v2_floor_ip(Vec2* a)
{
	a->x = floorf(a->x);
	a->y = floorf(a->y);
}

static inline
void v2_int_ip(Vec2* a)
{
	a->x = (i32)a->x;
	a->y = (i32)a->y;
}

#define _v2_y(v) (v.y)
#define _v2_x(v) (v.x)
GenerateInsertionSortForType(v2_sort_on_y, Vec2, _v2_y)
GenerateInsertionSortForType(v2_sort_on_x, Vec2, _v2_x)

Vec2 v2_to_local(Vec2 pt, Vec2 offset, f32 scale)
{
	v2_scale_ip(&pt, 1.0f/scale);
	v2_add_ip(&pt, offset);
	return pt;
}


Vec3 v3(f32 x, f32 y, f32 z)
{
	Vec3 v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

Vec3 v3_add(Vec3 a, Vec3 b)
{
	Vec3 v;
	v.x = a.x + b.x;
	v.y = a.y + b.y;
	v.z = a.z + b.z;
	return v;
}

Vec4 v4(f32 x, f32 y, f32 z, f32 w)
{
	Vec4 v;
	v.x = x;
	v.y = y;
	v.z = z;
	v.w = w;
	return v;
}

Vec2i v2i(i32 x, i32 y)
{
	Vec2i v;
	v.x = x;
	v.y = y;
	return v;
}


static inline
Color color_make(f32 r, f32 g, f32 b, f32 a)
{
	Color c;
	c.r = (u8)(r * 255);
	c.g = (u8)(g * 255);
	c.b = (u8)(b * 255);
	c.a = (u8)(a * 255);
	return c;
}

static inline
Color color_from_rgba(u32 rgba)
{
	Color c;
	c.r = (rgba >> 24) & 0xFF;
	c.g = (rgba >> 16) & 0xFF;
	c.b = (rgba >> 8) & 0xFF;
	c.a = (rgba >> 0) & 0xFF;
	return c;
}

static inline
u32 color_to_rgba(Color c)
{
	return (c.r << 24) | (c.g << 16) | (c.b << 8) | c.a;
}

/* For simplicity, "Rect2" and "AABB" share the same type, but are treated differently.
 * functions marked aabb_ will treat a Rect2 as a center and half-extents
 * functions marked rect_ will treat a Rect2 as the topleft and full width/height
 */
#ifndef REFLECTED
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
	i32 x, y, w, h;
};
#endif

static inline
Rect2 rect2(f32 x, f32 y, f32 w, f32 h)
{
	Rect2 r;
	r.pos = v2(x, y);
	r.size = v2(w, h);
	return r;
}

static inline
Rect2 rect2_v(Vec2 pos, Vec2 size)
{
	Rect2 r;
	r.pos = pos;
	r.size = size;
	return r;
}

static inline
Rect2 rect2_aligned(i32 x, i32 y, f32 align)
{
	return rect2(x * align, y * align, align, align);
}

static inline 
Rect2i rect2i(i32 x, i32 y, i32 w, i32 h)
{
	Rect2i r;
	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	return r;
}

#define AABB_x1(b) ((b).pos.x - (b).size.x)
#define AABB_x2(b) ((b).pos.x + (b).size.x)
#define AABB_y1(b) ((b).pos.y - (b).size.y)
#define AABB_y2(b) ((b).pos.y + (b).size.y)

#define AABBp_x1(b) ((b)->pos.x - (b)->size.x)
#define AABBp_x2(b) ((b)->pos.x + (b)->size.x)
#define AABBp_y1(b) ((b)->pos.y - (b)->size.y)
#define AABBp_y2(b) ((b)->pos.y + (b)->size.y)

static inline
i32 rect_contains_point(Rect2 a, Vec2 p)
{
	return (p.x > a.pos.x) && (p.y > a.pos.y) && (p.x < (a.pos.x + a.size.x)) && (p.y < (a.pos.y + a.size.y));
}


static inline
i32 aabb_contains(const Rect2* a, const Rect2* b)
{
	Vec2 atl = v2_sub(a->pos, a->size), abr = v2_add(a->pos, a->size);
	Vec2 btl = v2_sub(b->pos, b->size), bbr = v2_add(b->pos, b->size);
	return (btl.y > atl.y) && (bbr.y < abr.y) && (btl.x > atl.x) && (bbr.x < abr.x);
}

static inline
i32 aabb_intersect(const Rect2* a, const Rect2* b)
{
	if(fabsf(b->pos.x - a->pos.x) > (b->size.x + a->size.x)) return false;
	if(fabsf(b->pos.y - a->pos.y) > (b->size.y + a->size.y)) return false;
	return true;
}

static inline
Vec2 aabb_overlap(const Rect2* a, const Rect2* b)
{
	f32 sx = (a->size.x + b->size.x) - fabsf(b->pos.x - a->pos.x);
	f32 sy = (a->size.y + b->size.y) - fabsf(b->pos.y - a->pos.y);
	if(sx > sy) {
		sx = 0;
		if(a->pos.y > b->pos.y) {
			sy *= -1;
		}
	} else {
		sy = 0;
		if(a->pos.x > b->pos.x) {
			sx *= -1;
		}
	}
	return v2(sx, sy);
}

static inline 
i32 vaabb_intersect(Vec2 apos, Vec2 asize, Vec2 bpos, Vec2 bsize)
{
	if(fabsf(bpos.x - apos.x) > (bsize.x + asize.x)) return false;
	if(fabsf(bpos.y - apos.y) > (bsize.y + asize.y)) return false;
	return true;
}

static inline 
Vec2 vaabb_overlap(Vec2 apos, Vec2 asize, Vec2 bpos, Vec2 bsize)
{
	f32 sx = (asize.x + bsize.x) - fabsf(bpos.x - apos.x);
	f32 sy = (asize.y + bsize.y) - fabsf(bpos.y - apos.y);
	if(sx > sy) {
		sx = 0;
		if(apos.y > bpos.y) {
			sy *= -1;
		}
	} else {
		sy = 0;
		if(apos.x > bpos.x) {
			sx *= -1;
		}
	}
	return v2(sx, sy);

}

static inline
Rect2 aabb_from_extents(Vec2 tl, Vec2 br)
{
	Rect2 aabb;
	aabb.pos = v2_scale(v2_add(tl, br), 0.5f);
	aabb.size = v2_scale(v2_sub(br, tl), 1);
	aabb.size.x = fabsf(aabb.size.x);
	aabb.size.y = fabsf(aabb.size.y);
	return aabb;
}


i32 segment_intersect(Vec2 a1, Vec2 a2, Vec2 b1, Vec2 b2, Vec2* result)
{
	Vec2 asize = v2_scale(v2_abs(v2_sub(a2, a1)), 0.5f);
	Vec2 bsize = v2_scale(v2_abs(v2_sub(b2, b1)), 0.5f);
	Vec2 apos = v2_scale(v2_add(a1, a2), 0.5f);
	Vec2 bpos = v2_scale(v2_add(b1, b2), 0.5f);

	if(vaabb_intersect(apos, asize, bpos, bsize)) {
		f_segment_intersect(a1, a2, b1, b2, result);
		f32 x = result->x;
		f32 y = result->y;
		Vec2 xy = v2(x, y);
		if(vaabb_intersect(apos, asize, xy, v2(0.01, 0.01))) {
			if(vaabb_intersect(bpos, bsize, xy, v2(0.01, 0.01))) {
				if(isnan(x) || isnan(y)) {
					result->x = (b1.x + b2.x) / 2;
					result->y = (b1.y + b2.y) / 2;
				}
				return 1;
			}
			return 0;
		}
		return 0;
	}
	return 0;
}

i32 f_segment_intersect(Vec2 a1, Vec2 a2, Vec2 b1, Vec2 b2, Vec2* result)
{
	f32 dax = a2.x - a1.x;
	f32 dbx = b2.x - b1.x;

	i32 flipped = 0;
	if(dax * dbx == 0) {
		//TODO(will) handle perfectly axis-aligned lines
		flipped = 1;
		a1 = v2_swap_xy(a1);
		b1 = v2_swap_xy(b1);
		a2 = v2_swap_xy(a2);
		b2 = v2_swap_xy(b2);
	}

	f32 am = (a2.y - a1.y) / (a2.x - a1.x);
	f32 bm = (b2.y - b1.y) / (b2.x - b1.x);

	f32 x = am * a1.x - bm * b1.x + b1.y - a1.y;
	x /= am - bm;
	f32 y = am * (x - a1.x) + a1.y;

	if(result != NULL) {
		if(flipped) {
			f32 t = x;
			x = y;
			y = t;
		}
		result->x = x;
		result->y = y;
	}
	return 1;
}


//n^2 poly intersect
i32 poly_intersect(i32 p1c, Vec2* p1, i32 p2c, Vec2* p2)
{
	Vec2 res = v2(0, 0);
	for(isize i = 0; i < p1c; ++i) {
		isize j = (i + 1) % p1c;
		Vec2 p1a = p1[i];
		Vec2 p1b = p1[j];
		for(isize k = 0; k < p2c; ++k) {
			isize m = (k + 1) % p2c;
			Vec2 p2a = p2[k];
			Vec2 p2b = p2[m];
			if(segment_intersect(p1a, p1b, p2a, p2b, &res)) {
				return 1;
			}
		}
	}
	return 0;
}



//edges needs to be 5 wide
i32 poly_intersect_out(i32 p1c, Vec2* p1, i32 p2c, Vec2* p2, Vec2* edges, i32* indices)
{
	Vec2 res = v2(0, 0);
	for(isize i = 0; i < p1c; ++i) {
		isize j = (i + 1) % p1c;
		Vec2 p1a = p1[i];
		Vec2 p1b = p1[j];
		for(isize k = 0; k < p2c; ++k) {
			isize m = (k + 1) % p2c;
			Vec2 p2a = p2[k];
			Vec2 p2b = p2[m];
			if(segment_intersect(p1a, p1b, p2a, p2b, &res)) {
				edges[0] = p1a;
				edges[1] = p1b;
				edges[2] = p2a;
				edges[3] = p2b;
				edges[4] = res;
				indices[0] = i;
				indices[1] = j;
				indices[2] = k;
				indices[3] = m;
				return 1;
			}
		}
	}
	return 0;
}



i32 poly_contains_point(Vec2 p, i32 side_count, Vec2* poly)
{
	//does horizontal raycasts w/ edges of polygon
	//every time it crosses a line, it flips c
	//if c is even, it's outside, otherwise, inside
	i32 i, j, c = 0;
	for(i = 0, j = side_count - 1; i < side_count; j = i++) {
		Vec2* a = poly + i;
		Vec2* b = poly + j;
		//does this edge intersect the horiz ray?
		i32 t1 = a->y > p.y;
		i32 t2 = b->y > p.y;
		if(t1 != t2) {
			//line intersection
			f32 t3 = b->x - a->x;
			f32 t4 = p.y - a->y;
			f32 t5 = b->y - a->y;
			f32 edge_x = t3 * t4 / t5 + a->x;
			if(p.x < edge_x) {
				c = !c;
			}
		}
	}
	return c;
}

void convex_hull(i32 count, Vec2* points, i32* out_count, Vec2* out)
{
	isize k = 0;
	for(isize i = 0; i < count; ++i) {
		while(k >= 2 && v2_cross_origin(out[k-1], points[i], out[k-2]) <= 0) {
			--k;
		}
		out[k++] = points[i];
	}

	for(isize i = count-2, t = k+1; i >= 0; --i) {
		while(k >= t && v2_cross_origin(out[k-1], points[i], out[k-2]) <= 0) {
			--k;
		}
		out[k++] = points[i];
	}

	*out_count = k;
}
