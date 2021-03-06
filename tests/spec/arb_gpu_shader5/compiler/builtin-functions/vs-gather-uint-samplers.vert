// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_gpu_shader5 GL_ARB_texture_cube_map_array
// [end config]

#version 150
#extension GL_ARB_gpu_shader5: require
#extension GL_ARB_texture_cube_map_array: require

uniform usampler2D s2D;
uniform usampler2DArray s2DArray;
uniform usamplerCube sCube;
uniform usamplerCubeArray sCubeArray;
uniform usampler2DRect s2DRect;

void main()
{
	uvec4 res = uvec4(0);

	res += textureGather(s2D,		vec2(0));
	res += textureGather(s2DArray,		vec3(0));
	res += textureGather(sCube,		vec3(0));
	res += textureGather(sCubeArray,	vec4(0));
	res += textureGather(s2DRect,		vec2(0));

	gl_Position = vec4(res);
}
