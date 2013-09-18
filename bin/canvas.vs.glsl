#version 330 core

uniform mat4 mmvp;

layout(location = 0) in vec4 vposition;
layout(location = 1) in vec4 vcolor;

out vec4 color;

void main(void) {
    gl_Position = mmvp * vposition;

    color = vcolor;
}
