#version 330 core

uniform sampler2D tex0;

in vec2 texcoord;

layout(location = 0) out vec4 fcolor;

void main(void) {
    fcolor = texture(tex0, texcoord);
}
