#version 330 core

//flags first 4 bits: anchor, 1<<4+ flags
layout (location = 0) in uint v_flags;

//sort offset on sprite
//layout (location = 1) in float v_sort;

//Offset from origin in pixels (world space)
layout (location = 1) in vec2 v_translate;

//scale of the sprite
layout (location = 2) in vec2 v_size;

//Amount to translate sprite by locally
layout (location = 3) in vec2 v_center;

//cos,sin of sprite's angle
layout (location = 4) in vec2 v_rotation;

//x, y, w, h of texture rectangle in 0->1 form
layout (location = 5) in vec4 v_texcoords;

//rgba color, sent to frag shader
layout (location = 6) in vec4 v_color;

layout (location = 7) in vec4 v_topcorners;
layout (location = 8) in vec4 v_bottomcorners;

out vec2 f_texcoords;
out vec4 f_color;
out float f_invalid;
out float f_primitive;

uniform mat4 u_ortho_matrix;
uniform int u_index_mode;

void main()
{
	f_primitive = 0;
	f_color = v_color;
	f_texcoords = vec2(0, 0);
	f_invalid = 0;

	uint valid_flag = v_flags & uint(1<<10);
	if(valid_flag == uint(1<<10)) {
		f_invalid = 1.0;
		gl_Position = vec4(0, 0, 0, 1);
		return;
	}

	float[4] coords_arr = float[](
		//center
		-0.5, -0.5,
		0.5, 0.5
	);

	float[18] coords_displacement = float[](
		//center
		0.0, 0.0,
		//topleft
		0.5, 0.5,
		//top
		0.0, 0.5, 
		//topright
		-0.5, 0.5,
		//right
		-0.5, 0,
		//bottomright
		-0.5, -0.5,
		//bottom
		0.0, -0.5,
		//bottomleft
		0.5, -0.5,
		//left
		0.5, 0.0
	);

	uint vertex_x = uint(0);
	uint vertex_y = uint(0);

	if(u_index_mode == 0) {
		vertex_x = uint(gl_VertexID & 2);
		vertex_y = uint(((gl_VertexID & 1) << 1) ^ 3);
	} else if(u_index_mode == 1) {
		vertex_x = uint(gl_VertexID & 2);
		vertex_y = uint(((gl_VertexID+1) & 2) ^ 1);
	}

	uint v_anchor = v_flags & uint(0xF);

	uint v_fliphoriz = v_flags & uint(1<<4);
	uint v_flipvert = v_flags & uint(1<<5);

	vec2 coords = vec2(
		coords_arr[vertex_x],
		coords_arr[vertex_y]
	);

	if(u_index_mode == 0) {
		if(gl_VertexID == 0) { //bl
			coords *= v_bottomcorners.xy;
		} else if(gl_VertexID == 1) { //tl
			coords *= v_topcorners.xy;
		} else if(gl_VertexID == 2) { // br
			coords *= v_bottomcorners.zw;
		} else if(gl_VertexID == 3) { // tr
			coords *= v_topcorners.zw;
		}
	} else if(u_index_mode == 1) {
		if(gl_VertexID == 0) { //tl
			coords *= v_topcorners.xy;
		} else if(gl_VertexID == 1) { //bl
			coords *= v_bottomcorners.xy;
		} else if(gl_VertexID == 2) { // br
			coords *= v_bottomcorners.zw;
		} else if(gl_VertexID == 3) { // tr
			coords *= v_topcorners.zw;
		}

	}
	coords.x += coords_displacement[uint(2) * v_anchor];
	coords.y += coords_displacement[uint(2) * v_anchor + uint(1)];

	if((v_flags & uint(1<<6)) > uint(0)) {
		f_primitive = 1;
		f_texcoords = vec2(0, 0);
	} else {
		vec4 tex_rect = vec4(
				v_texcoords.x, v_texcoords.y,
				v_texcoords.x + v_texcoords.z, 
				v_texcoords.y + v_texcoords.w
				);

		if(v_fliphoriz >= uint(1)) {
			tex_rect = tex_rect.zyxw;
		}
		if(v_flipvert >= uint(1)) {
			tex_rect = tex_rect.xwzy;
		}

		float[4] texcoords_arr = float[](
				tex_rect.x, tex_rect.y,
				tex_rect.z, tex_rect.w
				);

		f_texcoords = vec2(
				texcoords_arr[gl_VertexID & 2],
				texcoords_arr[((gl_VertexID & 1) << 1) ^ 3]
				);
	}
	coords.x *= v_size.x;
	coords.y *= v_size.y;
	mat2 rotmat = mat2 (
		v_rotation.x, v_rotation.y,
		-v_rotation.y, v_rotation.x
	);
	coords -= v_center;
	coords *= rotmat;
	coords += v_translate;
	gl_Position = vec4(coords, 0, 1) * u_ortho_matrix;
}

