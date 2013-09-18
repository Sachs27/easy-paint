#version 330 core


layout(location = 0) in vec4 vposition;
layout(location = 1) in vec2 vtexcoord;

out vec2 texcoord;

void main(void) {
    gl_Position = vposition;
    texcoord = vtexcoord;
}
