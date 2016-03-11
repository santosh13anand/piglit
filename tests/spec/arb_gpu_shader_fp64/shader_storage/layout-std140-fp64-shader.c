/*
 * Copyright © 2016 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** @file layout-std140-fp64-shader.c
 *
 * Tests that shader storage block reads/writes in GLSL works correctly (offsets
 * and values) when interface packing qualifier is std140 and row_major, using
 * doubles.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.window_width = 100;
	config.window_height = 100;
	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END


#define XSTR(S) STR(S)
#define STR(S) #S

#define USE_DOUBLE

#define SSBO_SIZE 24
#define TOLERANCE 1e-5
#define DIFFER(a,b) ((a > b ? a - b : b - a) > TOLERANCE)


static const char vs_code[] =
	"#version 150\n"
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
        "#extension GL_ARB_gpu_shader_fp64 : require\n"
        "#extension GL_ARB_vertex_attrib_64bit : require\n"
	"\n"
	"layout(std140, row_major, binding=2) buffer ssbo {\n"
#ifdef USE_DOUBLE
        "       double u[" XSTR(SSBO_SIZE) "/4];\n"
#else
        "       float u[" XSTR(SSBO_SIZE) "/4];\n"
#endif
	"};\n"
#ifdef USE_DOUBLE
	"in dvec4 vertex;\n"
#else
	"in vec4 vertex;\n"
#endif
	"void main() {\n"
	"	gl_Position = vec4(1.0);\n"
#ifdef USE_DOUBLE
	"       u[gl_VertexID] = vertex.x;\n"
#else
	"       u[gl_VertexID] = vertex.x;\n"
#endif
        "}\n";

static const char fs_source[] =
	"#version 150\n"
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
        "#extension GL_ARB_gpu_shader_fp64 : require\n"
        "#extension GL_ARB_vertex_attrib_64bit : require\n"
	"out vec4 color;\n"
	"void main() {\n"
	"       color = vec4(0.0, 1.0, 0.0, 1.0);\n"
	"}\n";

GLuint prog;

#ifdef USE_DOUBLE
double ssbo_values[SSBO_SIZE] = { 0.0 };
#else
float ssbo_values[SSBO_SIZE] = { 0.0 };
#endif

#ifdef USE_DOUBLE
static const double verts[] = {
#else
static const float verts[] = {
#endif
  6.776277770192E-21, 0.0, 0.0, 0.0,
  0.0, 0.0, 0.0, 0.0,
};

void
piglit_init(int argc, char **argv)
{
	GLuint buffer;

	piglit_require_extension("GL_ARB_shader_storage_buffer_object");
	piglit_require_extension("GL_ARB_gpu_shader_fp64");
	piglit_require_extension("GL_ARB_vertex_attrib_64bit");
        piglit_require_GLSL_version(150);

	prog = piglit_build_simple_program_multiple_shaders(GL_VERTEX_SHADER, vs_code,
                                                            GL_FRAGMENT_SHADER, fs_source,
                                                            NULL);

	glUseProgram(prog);

	glClearColor(0, 0, 0, 0);

	glGenBuffers(1, &buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, buffer);

#ifdef USE_DOUBLE
	glBufferData(GL_SHADER_STORAGE_BUFFER, SSBO_SIZE*sizeof(double),
				&ssbo_values[0], GL_DYNAMIC_DRAW);
#else
	glBufferData(GL_SHADER_STORAGE_BUFFER, SSBO_SIZE*sizeof(float),
				&ssbo_values[0], GL_DYNAMIC_DRAW);
#endif

        glViewport(0, 0, piglit_width, piglit_height);


	piglit_check_gl_error(GL_NO_ERROR);
}

static const char *float_to_hex(float f)
{
  union {
    float f;
    unsigned int i;
  } b;

  b.f = f;
  char *s = (char *) malloc(100);
  sprintf(s, "0x%08X", b.i);
  return s;
}

static const char *double_to_hex(double d)
{
  union {
    double d;
    unsigned int i[2];
  } b;

  b.d = d;
  char *s = (char *) malloc(100);
  sprintf(s, "0x%08X%08x", b.i[1], b.i[0]);
  return s;
}

enum piglit_result piglit_display(void)
{
  GLuint vao, vbo;

        glClear(GL_COLOR_BUFFER_BIT);
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

        GLint attrib_location = glGetAttribLocation(prog, "vertex");

#ifdef USE_DOUBLE
        glVertexAttribLPointer(attrib_location, 4, GL_DOUBLE, 0, (const GLvoid *)0);
#else
        glVertexAttribPointer(attrib_location, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid  *)0);
#endif
        glEnableVertexAttribArray(attrib_location);
        glDrawArrays(GL_LINES, 0, 2);

#ifdef USE_DOUBLE
        double *map = (double *) glMapBuffer(GL_SHADER_STORAGE_BUFFER,  GL_READ_ONLY);
#else
        float *map = (float *) glMapBuffer(GL_SHADER_STORAGE_BUFFER,  GL_READ_ONLY);
#endif
        int i;
	for (i = 0; i < SSBO_SIZE; i+=4) {
#ifdef USE_DOUBLE
          printf("read[%d] = (%.14g [%s],  %.14g [%s],  %.14g [%s],  %.14g [%s])\n",
                 i,
                 map[i], double_to_hex(map[i]),
                 map[i+1], double_to_hex(map[i+1]),
                 map[i+2], double_to_hex(map[i+2]),
                 map[i+3], double_to_hex(map[i+3]));
#else
          printf("read[%d] = (%.14g [%s],  %.14g [%s],  %.14g [%s],  %.14g [%s])\n",
                 i,
                 map[i], float_to_hex(map[i]),
                 map[i+1], float_to_hex(map[i+1]),
                 map[i+2], float_to_hex(map[i+2]),
                 map[i+3], float_to_hex(map[i+3]));
#endif
        }

  return PIGLIT_PASS;
}
