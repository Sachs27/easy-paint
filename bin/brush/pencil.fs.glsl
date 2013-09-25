#version 330 core

uniform int         type;
uniform float       point_size;
uniform sampler2D   tex0;
uniform vec2        window_size;
uniform vec4        brush_color;

in vec2             texcoord;

layout(location = 0) out vec4 fcolor;


float get_alpha(vec2 pos) {
    float a0 = brush_color.a;
    float a1 = a0 * 0.5;
    float a2 = a1 * 0.5;

    switch (type) {
    case 0:
        /*
         * x x
         * o x
         */
        if (pos.x >= 0 && pos.x < 0.5) {
            if (pos.y >= 0 && pos.y < 0.5) {
                return a0;
            } else if (pos.y >= 0.5) {
                return a1;
            }
        } else if (pos.x >= 0.5) {
            if (pos.y >= 0 && pos.y < 0.5) {
                return a1;
            } else if (pos.y >= 0.5) {
                return a2;
            }
        }
        break;
    case 1:
        /*
         * o x
         * x x
         */
        if (pos.x >= 0 && pos.x < 0.5) {
            if (pos.y <= 0 && pos.y > -0.5) {
                return a0;
            } else if (pos.y <= -0.5) {
                return a1;
            }
        } else if (pos.x >= 0.5) {
            if (pos.y <= 0 && pos.y > -0.5) {
                return a1;
            } else if (pos.y < -0.5) {
                return a2;
            }
        }
        break;
    case 2:
        /*
         * x x
         * x o
         */
        if (pos.x <= 0 && pos.x > -0.5) {
            if (pos.y >= 0 && pos.y < 0.5) {
                return a0;
            } else if (pos.y >= 0.5) {
                return a1;
            }
        } else if (pos.x <= -0.5) {
            if (pos.y >= 0 && pos.y < 0.5) {
                return a1;
            } else if (pos.y >= 0.5) {
                return a2;
            }
        }
        break;
    case 3:
        /*
         * x o
         * x x
         */
        if (pos.x <= 0 && pos.x > -0.5) {
            if (pos.y <= 0 && pos.y > -0.5) {
                return a0;
            } else if (pos.y <= -0.5) {
                return a1;
            }
        } else if (pos.x <= -0.5) {
            if (pos.y <= 0 && pos.y > -0.5) {
                return a1;
            } else if (pos.y < -0.5) {
                return a2;
            }
        }
        break;
    case 4:
        /*
         *   x
         * x o x
         *   x
         */
        if (pos.x >= -0.33 && pos.x <= 0.33) {
            if (pos.y >= -0.33 && pos.y <= 0.33) {
                return a0;
            } else {
                return a1;
            }
        } else if (pos.y >= -0.33 && pos.y <= 0.33) {
            return a1;
        }
        break;
    }

    return 0;
}

void main(void) {
    vec2 pos = gl_PointCoord * 2 - 1;       // [-1, 1]
    vec2 off_texcoord = texcoord + pos * (point_size / 2.0) / window_size;
    vec4 dst_color = texture(tex0, off_texcoord);
    vec4 src_color;

    src_color.rgb = brush_color.rgb;
    src_color.a = get_alpha(pos);

    if (src_color.a == 0) {
        discard;
    }

    fcolor = vec4(src_color.rgb * src_color.a
                  + dst_color.rgb * (1 - src_color.a),
                  src_color.a + dst_color.a);
}
