#version 330

uniform vec4 color;

layout(location = 0) out vec4 fcolor;

void main(void) {
    fcolor = color;
}
