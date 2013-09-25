#version 330 core

layout(location = 0) in vec2 vposition;     // [-1, 1]
layout(location = 1) in vec4 vcolor;

out vec2 texcoord;
out vec4 color;

void main(void) {
    gl_Position = vec4(vposition, 0, 1);
    texcoord = (vposition + 1) / 2;         // [0, 1]
    color = vcolor;
}
