#version 330

uniform sampler2D texture;
uniform vec2 delta;

in vec2 coord;
layout(location = 0) out vec4 FragColor;

void main() {
    /* get vertex info */
    vec4 info = texture2D(texture, coord);
      
    /* update the normal */
    vec3 dx = vec3(delta.x, texture2D(texture, vec2(coord.x + delta.x, coord.y)).r - info.r, 0.0);
    vec3 dy = vec3(0.0, texture2D(texture, vec2(coord.x, coord.y + delta.y)).r - info.r, delta.y);
    info.ba = normalize(cross(dy, dx)).xz;
      
    FragColor = info;
}