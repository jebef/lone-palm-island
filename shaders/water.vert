#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTexCoords;

out vec3 wPos;
out vec4 cPos;
out vec3 wNorm;
out vec2 TiledTexCoords;

uniform mat4 model;
uniform mat4 view; 
uniform mat4 projection;
uniform mat3 normal;

const float tiling_factor = 3.0f;

void main () {
    // transform vertex's position to world space 
    wPos = vec3(model * vec4(aPos, 1.0f));

    // transform vertex's position to clip space 
    cPos = projection * view * vec4(wPos, 1.0f);

    // transform vertex's normal to world space 
    wNorm = normalize(normal * aNorm);

    // compute tiled texture coordinates for sampling dudv and normal maps 
    TiledTexCoords = aTexCoords * tiling_factor;

    // set fragment's position attribute 
    gl_Position = cPos;
}