#version 330

uniform mat4 mprojection;

layout(location = 0) in vec2 vposition;

void main(void) {
    gl_Position = mprojection * vec4(vposition, 0, 1);
}
