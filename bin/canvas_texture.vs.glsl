#version 330 core

layout(location = 0) in vec2 vposition;
layout(location = 1) in vec4 vcolor;

out vec4 color;

void main(void) {
    gl_Position = vec4(vposition, 0, 1);
    color = vcolor;
}
