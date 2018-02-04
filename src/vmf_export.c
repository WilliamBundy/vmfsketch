
#ifndef REFLECTED

struct VmfContext
{
	string name;
	FILE* file;

	i32 next_id;
	i32 next_side_id;
	i32 indent;

};
struct Plane
{
	Vec3 points[3];
	Vec4 uv[2];
	i32 vert;
}
#endif

Vec3 v3ify(Vec2 v, f32 z, Vec3 offset)
{
	return v3_add(v3(v.x, v.y, z), offset);
}

isize sprite_get_planes(Sprite* s, f32 depth, f32 rise, Vec3 offset,  Plane* planes)
{
	Vec2* corners;
	Vec2 corner_list[4];
	sprite_corners(s, corner_list);
	v2_sort_on_x(corner_list, 4);
	Vec2 convex[8];
	i32 count = 0;
	convex_hull(4, corner_list, &count, convex);
	count--;
	f32 heights[4];
	if(s->low_side.x == 0 && s->low_side.y == 0) {
		heights[0] = 1;
		heights[1] = 1;
		heights[2] = 1;
		heights[3] = 1;
	} else {
		sprite_get_corner_heights(s, count, convex, heights);
	}
	corners = (void*)convex;
	if (count == 4) {
		isize i = 0;
		//top
		Plane p;
		p.vert = 0;
		p.points[0] = v3ify(corners[3], rise * heights[3], offset);
		p.points[1] = v3ify(corners[2], rise * heights[2], offset);
		p.points[2] = v3ify(corners[1], rise * heights[1], offset);
		p.uv[0] = v4(1, 0, 0, 0);
		p.uv[1] = v4(0, -1, 0, 0);
		planes[i++] = p;
		//bottom
		p.vert = 0;
		p.points[0] = v3ify(corners[0], -depth, offset);
		p.points[1] = v3ify(corners[1], -depth, offset);
		p.points[2] = v3ify(corners[2], -depth, offset);
		p.uv[0] = v4(1, 0, 0, 0);
		p.uv[1] = v4(0, -1, 0, 0);
		planes[i++] = p;
		//left
		p.vert = 1;
		p.points[0] = v3ify(corners[3], rise * heights[3], offset);
		p.points[1] = v3ify(corners[0], rise * heights[0], offset);
		p.points[2] = v3ify(corners[0], -depth, offset);
		p.uv[0] = v4(0, 1, 0, 0);
		p.uv[1] = v4(0, 0, -1, 0);
		planes[i++] = p;
		//right
		p.vert = 1;
		p.points[0] = v3ify(corners[2], -depth, offset);
		p.points[1] = v3ify(corners[1], -depth, offset);
		p.points[2] = v3ify(corners[1], rise * heights[1], offset);
		p.uv[0] = v4(0, 1, 0, 0);
		p.uv[1] = v4(0, 0, -1, 0);
		planes[i++] = p;
		//down
		p.vert = 1;
		p.points[0] = v3ify(corners[2], rise * heights[2], offset);
		p.points[1] = v3ify(corners[3], rise * heights[3], offset);
		p.points[2] = v3ify(corners[3], -depth, offset);
		p.uv[0] = v4(1, 0, 0, 0);
		p.uv[1] = v4(0, 0, -1, 0);
		planes[i++] = p;
		//up
		p.vert = 1;
		p.points[0] = v3ify(corners[1], -depth, offset);
		p.points[1] = v3ify(corners[0], -depth, offset);
		p.points[2] = v3ify(corners[0], rise * heights[0], offset);
		p.uv[0] = v4(1, 0, 0, 0);
		p.uv[1] = v4(0, 0, -1, 0);
		planes[i++] = p;
		return i;
	} else if(count == 3) {
		i32 i = 0;
		Plane p;
		p.vert = 0;
		p.points[0] = v3ify(corners[0], rise * heights[0], offset);
		p.points[1] = v3ify(corners[2], rise * heights[2], offset);
		p.points[2] = v3ify(corners[1], rise * heights[1], offset);
		p.uv[0] = v4(1, 0, 0, -256);
		p.uv[1] = v4(0, -1, 0, -256);
		planes[i++] = p;

		//bottom
		p.vert = 0;
		p.points[0] = v3ify(corners[1], -depth, offset);
		p.points[1] = v3ify(corners[2], -depth, offset);
		p.points[2] = v3ify(corners[0], -depth, offset);
		p.uv[0] = v4(1, 0, 0, -256);
		p.uv[1] = v4(0, -1, 0, -256);
		planes[i++] = p;
		//
		//left
		p.vert = 1;
		p.points[0] = v3ify(corners[0], -depth, offset);
		p.points[1] = v3ify(corners[2], -depth, offset);
		p.points[2] = v3ify(corners[2], rise * heights[2], offset);
		p.uv[0] = v4(0, 1, 0, 256);
		p.uv[1] = v4(0, 0, -1, -256);
		planes[i++] = p;
		//right
		p.vert = 1;
		p.points[0] = v3ify(corners[2], -depth, offset);
		p.points[1] = v3ify(corners[1], -depth, offset);
		p.points[2] = v3ify(corners[1], rise * heights[1], offset);
		p.uv[0] = v4(0, 1, 0, 256);
		p.uv[1] = v4(0, 0, -1, -256);
		planes[i++] = p;
		//down
		p.vert = 1;
		p.points[0] = v3ify(corners[1], -depth, offset);
		p.points[1] = v3ify(corners[0], -depth, offset);
		p.points[2] = v3ify(corners[0], rise * heights[0], offset);
		p.uv[0] = v4(1, 0, 0, -256);
		p.uv[1] = v4(0, 0, -1, -256);
		planes[i++] = p;
		return i;
	}
	return 0;

}


void vmfctx_init(VmfContext* ctx, string name)
{
	ctx->name = name;
	ctx->file = fopen(name, "w");
	ctx->next_id = 0;
	ctx->next_side_id = 1;
	ctx->indent = 0;
}


void vmfctx_write_indent(VmfContext* ctx)
{
	for(isize i = 0; i < ctx->indent; ++i) {
		fwrite("\t", 1, 1, ctx->file);
	}
}
void vmfctx_add_line(VmfContext* ctx, string name)
{
	if(name[0] == '}') {
		ctx->indent--;
	}
	vmfctx_write_indent(ctx);
	fwrite(name, strlen(name), 1, ctx->file);
	fwrite("\n", 1, 1, ctx->file);
	if(name[0] == '{') {
		ctx->indent++;
	}
}
void vmfctx_add_property(VmfContext* ctx, string name, string value)
{
	vmfctx_write_indent(ctx);
	char buf[1024];
	isize len = snprintf(buf, 1024, "\"%s\" \"%s\"", name, value);
	fwrite(buf, len, 1, ctx->file);
	fwrite("\n", 1, 1, ctx->file);
}
void vmfctx_add_iproperty(VmfContext* ctx, string name, int value)
{
	vmfctx_write_indent(ctx);
	char buf[1024];
	isize len = snprintf(buf, 1024, "\"%s\" \"%d\"", name, value);
	fwrite(buf, len, 1, ctx->file);
	fwrite("\n", 1, 1, ctx->file);
}

void vmfctx_add_plane(VmfContext* ctx, string name, Plane* p)
{
	vmfctx_write_indent(ctx);
	char buf[1024];
	isize len = snprintf(buf, 1024, "\"%s\" \"(%.0f %.0f %.0f) (%.0f %.0f %.0f) (%.0f %.0f %.0f)\"", name, 
			p->points[0].x, p->points[0].y, p->points[0].z,
			p->points[1].x, p->points[1].y, p->points[1].z,
			p->points[2].x, p->points[2].y, p->points[2].z);
	fwrite(buf, len, 1, ctx->file);
	fwrite("\n", 1, 1, ctx->file);
}

void vmfctx_add_v4uv(VmfContext* ctx, string name, Vec4 uv)
{
	vmfctx_write_indent(ctx);
	char buf[1024];
	isize len = snprintf(buf, 1024, "\"%s\" \"[%.0f %.0f %0.f %0.f] 0.25\"", name, 
			uv.x, uv.y, uv.z, uv.w);
	fwrite(buf, len, 1, ctx->file);
	fwrite("\n", 1, 1, ctx->file);
}




void vmfctx_start(VmfContext* ctx, EditorLayer* layers, isize layer_count)
{
	vmfctx_add_line(ctx, "versioninfo");
	vmfctx_add_line(ctx, "{");
	vmfctx_add_property(ctx, "editorversion", "400");
	vmfctx_add_property(ctx, "editorbuild", "7417");
	vmfctx_add_property(ctx, "mapversion", "0");
	vmfctx_add_property(ctx, "formatversion", "100");
	vmfctx_add_property(ctx, "prefab", "0");
	vmfctx_add_line(ctx, "}");
#if 1
	vmfctx_add_line(ctx, "visgroups");
	vmfctx_add_line(ctx, "{");
	for(isize i = 0; i < layer_count; ++i) {
		vmfctx_add_line(ctx, "visgroup");
		vmfctx_add_line(ctx, "{");
		char buf[256];
		snprintf(buf, 256, "%s - %d", layers[i].name, layers[i].id);
		vmfctx_add_property(ctx, "name", buf);
		vmfctx_add_iproperty(ctx, "visgroupid", layers[i].id);
		Vec4 c = layers[i].color;
		snprintf(buf, 256, "%.0f %.0f %.0f", c.x * 255, c.y * 255, c.z * 255);
		vmfctx_add_property(ctx, "color", buf);
		vmfctx_add_line(ctx, "}");
	}
	vmfctx_add_line(ctx, "}");
#endif


	vmfctx_add_line(ctx, "viewsettings");
	vmfctx_add_line(ctx, "{");
	vmfctx_add_property(ctx, "bSnapToGrid", "1");
	vmfctx_add_property(ctx, "bShowGrid", "1");
	vmfctx_add_property(ctx, "bShowLogicalGrid", "0");
	vmfctx_add_property(ctx, "nGridSpacing", "64");
	vmfctx_add_property(ctx, "bShow3DGrid", "0");
	vmfctx_add_line(ctx, "}");

}

void vmfctx_add_world_geom(VmfContext* ctx, EditorLayer* layers, i32 count)
{
	vmfctx_add_line(ctx, "world");
	vmfctx_add_line(ctx, "{");
	vmfctx_add_property(ctx, "id", "1");
	vmfctx_add_property(ctx, "mapversion", "0");
	vmfctx_add_property(ctx, "classname", "worldspawn");
	vmfctx_add_property(ctx, "detailvbsp", "detail_2fort.vbsp");
	vmfctx_add_property(ctx, "detailmaterial", "detail/detailsprites_2fort");
	vmfctx_add_property(ctx, "maxpropscreenwidth", "-1");
	vmfctx_add_property(ctx, "skyname", "sky_tf_04");
	ctx->next_id++;
	ctx->next_id++;


	for(isize layer_index = 0; layer_index < count; ++layer_index) {
		EditorLayer* layer = layers + layer_index;
		layer->pos.y *= -1;
		for(isize i = 0; i <= layer->brushes->last_filled; ++i) {
			Sprite brush = layer->brushes->sprites[i];
			if(brush.flags & SpriteFlag_Null) continue;
			vmfctx_add_line(ctx, "solid");
			vmfctx_add_line(ctx, "{");
			vmfctx_add_iproperty(ctx, "id", ctx->next_id++);
			Plane sides[6];
			brush.pos.y *= -1;
			brush.low_side.y *= -1;
			Vec2 temp = brush.corners[0];
			brush.corners[0] = brush.corners[2];
			brush.corners[2] = temp;
			temp = brush.corners[1];
			brush.corners[1] = brush.corners[3];
			brush.corners[3] = temp;
			//sprite_validate(&brush);
			isize len = sprite_get_planes(&brush, layer->depth, layer->rise, layer->pos, sides);
			for(isize j = 0; j < len; ++j) {
				Plane* p = sides + j;
				vmfctx_add_line(ctx, "side");
				vmfctx_add_line(ctx, "{");
				vmfctx_add_iproperty(ctx, "id", ctx->next_side_id++);
				vmfctx_add_plane(ctx, "plane", p);
				if(p->vert) {
					vmfctx_add_property(ctx, "material", "DEV/REFLECTIVITY_50");
				} else {
					vmfctx_add_property(ctx, "material", "DEV/DEV_BLENDMEASURE");
				}
				vmfctx_add_v4uv(ctx, "uaxis", p->uv[0]);
				vmfctx_add_v4uv(ctx, "vaxis", p->uv[1]);
				vmfctx_add_property(ctx, "rotation", "0");
				vmfctx_add_property(ctx, "lightmapscale", "16");
				vmfctx_add_property(ctx, "smoothing_groups", "0");
				vmfctx_add_line(ctx, "}");
			}
			vmfctx_add_line(ctx, "editor");
			vmfctx_add_line(ctx, "{");
			vmfctx_add_property(ctx, "color", "0 107 172");
			vmfctx_add_iproperty(ctx, "visgroupid", layer->id);
			vmfctx_add_property(ctx, "visgroupshown", "1");
			vmfctx_add_property(ctx, "visgroupautoshown", "1");
			vmfctx_add_line(ctx, "}");

			vmfctx_add_line(ctx, "}");
		}
		layer->pos.y *= -1;
	}


	vmfctx_add_line(ctx, "}");
}

void vmfctx_end(VmfContext* ctx)
{
#if 0
	vmfctx_add_line(ctx, "entity");
	vmfctx_add_line(ctx, "{");
	vmfctx_add_line(ctx, "}");

	vmfctx_add_line(ctx, "hidden");
	vmfctx_add_line(ctx, "{");
	vmfctx_add_line(ctx, "}");
#endif
	vmfctx_add_line(ctx, "cameras");
	vmfctx_add_line(ctx, "{");
	vmfctx_add_property(ctx, "activecamera", "-1");
	vmfctx_add_line(ctx, "}");

	vmfctx_add_line(ctx, "cordon");
	vmfctx_add_line(ctx, "{");

	vmfctx_add_property(ctx, "mins", "(-1024 -1024 -1024)");
	vmfctx_add_property(ctx, "maxs", "(1024 1024 1024)");
	vmfctx_add_property(ctx, "active", "0");
	vmfctx_add_line(ctx, "}");

	fclose(ctx->file);
}
