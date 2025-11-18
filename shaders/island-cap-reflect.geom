#version 330 core 
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec3 wPos;
    vec3 wNorm;
    vec2 TexCoords;
} gs_in[];

uniform mat4 projection;
uniform mat4 view;

out vec3 wPos;
out vec3 wNorm;
out vec2 TexCoords;

void main() {
    int triangle_safe = 0;

    for (int i = 0; i < 3; i++) {
        // check to see if the entire triangle is safe
        if (gl_in[i].gl_ClipDistance[0] > 0.0f) {
            triangle_safe += 1;
        } 
    }
    // if safe, clamp y values to just above 0 to hide palm tree base 
    if (triangle_safe == 3) {
        for (int j = 0; j < 3; j++) {
            gl_Position = projection * view * vec4(gs_in[j].wPos.x, 0.4f, gs_in[j].wPos.z, 1.0f);
            wPos = gs_in[j].wPos;
            wNorm = gs_in[j].wNorm;
            TexCoords = gs_in[j].TexCoords;
            EmitVertex();
        }
        EndPrimitive();
    }
}