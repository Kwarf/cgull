#version 330

uniform vec2 resolution;
uniform float time;

out vec4 color;

void main() {
    color = vec4(1.0, 0.0, sin(time), 1.0);
}
