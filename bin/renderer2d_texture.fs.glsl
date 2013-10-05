#version 330

uniform sampler2D texture0;
uniform bool      iscoloring = false;
uniform vec3      color = vec3(0, 0, 0);

in vec2 texcoord;

layout(location = 0) out vec4 fcolor;

void main(void) {
    vec4 texcolor = texture(texture0, texcoord);

    if (iscoloring) {
        fcolor = vec4(color.rgb * texcolor.a,
                      texcolor.a);
    } else {
        fcolor = texcolor;
    }
}
