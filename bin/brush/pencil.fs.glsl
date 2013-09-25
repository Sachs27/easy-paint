#version 330 core

uniform sampler2D   tex0;

in vec2 texcoord;
in vec4 color;

layout(location = 0) out vec4 fcolor;

void main(void) {
    vec4 dst_color = texture(tex0, texcoord);

    fcolor = vec4(color.rgb * color.a + color.rgb * (1 - color.a),
                  color.a + dst_color.a);
}
