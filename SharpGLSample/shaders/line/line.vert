#version 330

uniform vec2 window;
uniform uint n;

in vec4 vec;
out vec3 angular_velocity;

void main() {
    float n = n;
    float z = (gl_VertexID - 1) / (n - 2);
    if (window.y > window.x) {
        gl_Position = vec4(vec.x, vec.y, z, 1.0);
    } else {
        gl_Position = vec4(vec.y, vec.x, z, 1.0);
    }
    angular_velocity = vec.zxw;
}
