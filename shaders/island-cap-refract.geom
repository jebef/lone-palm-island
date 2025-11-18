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

    if (triangle_safe < 3) {
        for (int j = 0; j < 3; j++) {
            // vertex is getting clipped? clamp y value to zero to create "cap"
            if (gl_in[j].gl_ClipDistance[0] < 0.0f) {
                gl_Position = projection * view * vec4(gs_in[j].wPos.x, 0.0f, gs_in[j].wPos.z, 1.0f);
            } else {
                gl_Position = gl_in[j].gl_Position;
            }
            // set output variables 
            wPos = gs_in[j].wPos;
            wNorm = gs_in[j].wNorm;
            TexCoords = gs_in[j].TexCoords;
            EmitVertex();
        }
        EndPrimitive();
    }
}