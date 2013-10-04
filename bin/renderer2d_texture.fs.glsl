#version 330

uniform sampler2D texture0;

in vec2 texcoord;

layout(location = 0) out vec4 fcolor;

void main(void) {
    fcolor = texture(texture0, texcoord);
}
