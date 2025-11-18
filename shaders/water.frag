#version 330 core 

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight { 
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};
#define NR_POINT_LIGHTS 1

struct SpotLight {
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
    float inner_cut_off;
    float outer_cut_off;
};
#define NR_SPOT_LIGHTS 1

// FUNCTION HEADERS
vec3 CalcDirLightSpec(DirLight light, vec3 camera_dir, vec4 frag_color, vec3 normal);
vec3 CalcPointLightSpec(PointLight light, vec3 camera_dir, vec4 frag_color, vec3 normal);
vec3 CalcSpotLightSpec(SpotLight light, vec3 camera_dir, vec4 frag_color, vec3 normal);

// OUT VARS FOR ALPHA TEST AND BLENDING STAGE
out vec4 FragColor;

// UNIFORMS 
uniform vec3 camera_pos;
uniform Material material;
uniform DirLight directional_light;
uniform PointLight point_light[NR_POINT_LIGHTS];
uniform SpotLight spot_light[NR_SPOT_LIGHTS];
uniform bool directional_only;

uniform sampler2D reflection_texture;
uniform sampler2D refraction_texture;
uniform sampler2D dudv_map;
uniform sampler2D normal_map;
uniform float sampling_offset;

uniform float specular_intenstiy;

// IN VARS FROM VERTEX SHADER
in vec3 wPos;
in vec4 cPos;
in vec3 wNorm;
in vec2 TiledTexCoords;

const float distortion_strength = 0.01;

void main() {

    //----- REFLECTION/REFRACTION TEXTURES -----//

    // convert clip space position to normalized device coordinates [0,1]
    vec2 nPos = (cPos.xy / cPos.w) / 2.0f + 0.5f;
    
    // calculate reflection/refraction texture coordinates 
    vec2 refl_tex_coords = vec2(nPos.x, -nPos.y);
    vec2 refr_tex_coords = vec2(nPos.x, nPos.y);

    // dynamically update sampling coordinates - this can be played with to achieve desired look 
    vec2 distor_coords_0 = vec2(TiledTexCoords.x + sampling_offset, TiledTexCoords.y);
    vec2 distor_coords_1 = vec2(-TiledTexCoords.x, TiledTexCoords.y + sampling_offset);

    // sample distortion values from dudv map - convert from [0,1] to [-1,1] for positive and negative distortions
    vec2 distor_0 = texture(dudv_map, distor_coords_0).rg * 2.0f -1.0f;
    vec2 distor_1 = texture(dudv_map, distor_coords_1).rg * 2.0f -1.0f;
    vec2 total_distortion = (distor_0 + distor_1) * distortion_strength;

    // sample normal map 
    // red -> x | green -> z | blue -> y
    vec4 raw_norm_0 = texture(normal_map, distor_coords_0);
    vec3 norm_0 = vec3(raw_norm_0.r * 2.0f - 1.0f, raw_norm_0.b, raw_norm_0.g * 2.0f - 1.0f); 
    vec4 raw_norm_1 = texture(normal_map, distor_coords_1);
    vec3 norm_1 = vec3(raw_norm_1.r * 2.0f - 1.0f, raw_norm_1.b, raw_norm_1.g * 2.0f - 1.0f); 
    vec3 normal = normalize(norm_0 + norm_1);

    // offset reflection/refraction texture coordinates with distortion
    refl_tex_coords += total_distortion;
    refl_tex_coords.x = clamp(refl_tex_coords.x, 0.001f, 0.999f);
    refl_tex_coords.y = clamp(refl_tex_coords.y, -0.999f, -0.001f);
    refr_tex_coords += total_distortion;
    refr_tex_coords = clamp(refr_tex_coords, 0.001f, 0.999f);

     // compute direction vector from fragment to camera 
    vec3 frag_to_camera = normalize(camera_pos - wPos);

    // calculate reflection factor - Fresnel effect 
    float refr_factor = dot(frag_to_camera, vec3(0.0f, 1.0f, 0.0f));
    // tune refraction factor
    refr_factor = pow(refr_factor, 1.5f); 

    // combine reflection and refraction textures 
    vec4 texture_result = mix(texture(reflection_texture, refl_tex_coords), texture(refraction_texture, refr_tex_coords), refr_factor);

    //----- SPECULAR HIGHLIGHTS -----//

    // directional light 
    vec3 lighting_result = CalcDirLightSpec(directional_light, frag_to_camera, texture_result, normal);

    // compute light values from point and spot lights if any 
    if (!directional_only) {
        // point lights 
        for (int i = 0; i < NR_POINT_LIGHTS; i++) {
            lighting_result += CalcPointLightSpec(point_light[i], frag_to_camera, texture_result, normal);
        }
        // spot lights
        for (int i = 0; i < NR_SPOT_LIGHTS; i++)
            lighting_result += CalcSpotLightSpec(spot_light[i], frag_to_camera, texture_result, normal);
    }

    // combine results 
    FragColor = mix(texture_result + vec4(lighting_result, 1.0f), vec4(0.0f, 0.2f, 0.5f, 1.0f), 0.3f);

    // REFLECTIONS AND REFRACTIONS ONLY - WRITEUP
    // refl_tex_coords = vec2(nPos.x, -nPos.y);
    // refr_tex_coords = vec2(nPos.x, nPos.y);
    // FragColor = mix(mix(texture(reflection_texture, refl_tex_coords), texture(refraction_texture, refr_tex_coords), 0.5f), vec4(0.4f, 0.5f, 1.0f, 1.0f), 0.2f);
}


vec3 CalcDirLightSpec(DirLight light, vec3 camera_dir, vec4 frag_color, vec3 normal) {

    vec3 light_dir = normalize(-light.direction);
  
    vec3 refl_dir = reflect(-light_dir, normal);

    float spec_imp = pow(max(dot(camera_dir, refl_dir), 0.0f), material.shininess);

    vec3 specular = light.specular * spec_imp * vec3(frag_color);

    return specular * specular_intenstiy;
}


vec3 CalcPointLightSpec(PointLight light, vec3 camera_dir, vec4 frag_color, vec3 normal) {

    // points from fragment to light source
    vec3 light_dir = normalize(light.position - wPos);

    // specular component
    vec3 refl_dir = reflect(-light_dir, normal);
    float spec_imp = pow(max(dot(camera_dir, refl_dir), 0.0f), material.shininess);

    // attenuation
    float dist = length(light.position - wPos);
    float attenuation = 1.0f / (light.constant + light.linear * dist + light.quadratic * (dist * dist));

    // scale components
    vec3 specular = light.specular * spec_imp * vec3(frag_color);

    // attenuate components
    specular *= attenuation;

    // combine results
    return specular * specular_intenstiy;
}


vec3 CalcSpotLightSpec(SpotLight light, vec3 camera_dir, vec4 frag_color, vec3 normal) {

    // points from fragment to light source
    vec3 light_dir = normalize(light.position - wPos);

    // specular component
    vec3 refl_dir = reflect(-light_dir, normal);
    float spec_imp = pow(max(dot(camera_dir, refl_dir), 0.0f), material.shininess);

    // attenuation
    float dist = length(light.position - wPos);
    float attenuation = 1.0f / (light.constant + light.linear * dist + light.quadratic * (dist * dist));

    // spotlight intensity
    float theta = dot(light_dir, normalize(-light.direction));
    float epsilon = light.inner_cut_off - light.outer_cut_off;
    float spot_intensity = clamp((theta - light.outer_cut_off) / epsilon, 0.0f, 1.0f);

    // scale components
    vec3 specular = light.specular * spec_imp * vec3(frag_color);

    // attenuate components
    specular *= attenuation;

    // combine results
    return specular * specular_intenstiy;
}