#version 430 core

void main() {

// quad, 4 sides, aka 2 triangles
    vec2 vertices[6] = {
        // Top Left
        vec2(-0.5, 0.5),
        // Bottom Left
        vec2(-0.5, -0.5),
        // Top Right
        vec2(0.5, 0.5),
        // Top Right , 2???
        vec2(0.5, 0.5),
        // Bottom Left , 2???
        vec2(-0.5, -0.5),
        // Bottom Right
        vec2(0.5, -0.5)
    };

    gl_Position = vec4(vertices[gl_VertexID], 1.0f, 1.0f);
}