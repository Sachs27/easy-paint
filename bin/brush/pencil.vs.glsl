#version 330 core

uniform float point_size;

layout(location = 0) in vec2 vposition;     // [-1, 1]

out vec2        texcoord;

float rand(vec2 co){
    return sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453;
}

void main(void) {
    gl_Position = vec4(vposition, 0, 1);
    gl_PointSize = point_size;
    texcoord = (vposition + 1) / 2;
}
