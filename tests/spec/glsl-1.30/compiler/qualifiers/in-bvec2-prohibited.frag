// [config]
// expect_result: fail
// glsl_version: 1.30
// [end config]
//
// Check that a bvec2 can't be used as a varying in GLSL 1.30.
//
// From section 4.3.4 ("Inputs") of the GLSL 1.30 spec:
//     Fragment inputs can only be signed and unsigned integers and
//     integer vectors, float, floating-point vectors, matrices, or
//     arrays of these.  Structures cannot be input.

#version 130

flat in bvec2 foo;

void main()
{
  gl_FragColor = vec4(foo, 0.0, 0.0);
}
