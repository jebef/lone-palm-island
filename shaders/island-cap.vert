#version 330 core 
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normal;
uniform vec4 clip_plane;

out VS_OUT {
    vec3 wPos;
    vec3 wNorm;
    vec2 TexCoords;
} vs_out;

void main() {
    // transform position to world space
    vec4 wPos = model * vec4(aPos, 1.0f);

    // set geometry shader inputs 
    vs_out.wPos = vec3(wPos);
    vs_out.wNorm = normalize(normal * aNorm);
    vs_out.TexCoords = aTexCoords;

    // compute clip distance 
    gl_ClipDistance[0] = dot(wPos, clip_plane);
    
    // transform position to clip space 
    gl_Position = projection * view * wPos;
}