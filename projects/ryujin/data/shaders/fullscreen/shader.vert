#version 450

layout (location = 0) out vec2 vertUV;

vec2 uvs[3] = vec2[](
    vec2(0.0, 0.0),
    vec2(2.0, 0.0),
    vec2(0.0, 2.0)
);

vec4 positions[3] = vec4[](
    vec4(-1.0, -1.0, 0.0, 1.0),
    vec4(3.0, -1.0, 0.0, 1.0),
    vec4(-1.0, 3.0, 0.0, 1.0)
);

void main(void) {
    vertUV = uvs[gl_VertexIndex];
    gl_Position = positions[gl_VertexIndex];
}