
#version 330 core
in vec2 f_texcoords;
in vec4 f_color;
in float f_invalid;
in float f_primitive;

uniform vec4 u_tint;
uniform vec2 u_texture_size;
uniform sampler2D u_texture0;
uniform float u_scale;

out vec4 final_color;
vec2 subpixel_aa(vec2 uv, vec2 texture_size, float zoom)
{
    vec2 pixel = uv * texture_size;
    return (floor(pixel) + 1.5 - clamp((1.0 - fract(pixel)) * zoom, 0.0, 1.0)) / texture_size;
}

void main()
{
	vec4 color = f_color;
	if(f_invalid > 0) {
		color = vec4(0, 0, 0, 0);
	}
	//vec2 uv = subpixel_aa(f_texcoords, u_texture_size, u_scale);
	vec2 uv = f_texcoords;

	if(f_primitive > 0.5) {
		final_color = color * u_tint;
	} else {
		final_color = texture(u_texture0, uv) * color * u_tint;
	}
}




