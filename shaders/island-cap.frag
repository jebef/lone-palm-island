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
vec3 CalcDirLight(DirLight light, vec3 camera_dir);
vec3 CalcPointLight(PointLight light, vec3 camera_dir);
vec3 CalcSpotLight(SpotLight light, vec3 camera_dir);

out vec4 FragColor;

// UNIFORMS 
uniform vec3 camera_pos;
uniform Material material;
uniform DirLight directional_light;
uniform PointLight point_light[NR_POINT_LIGHTS];
uniform SpotLight spot_light[NR_SPOT_LIGHTS];
uniform bool directional_only;

in vec3 wPos;
in vec3 wNorm;
in vec2 TexCoords;

void main() {
    // get camera/view direction 
    vec3 frag_to_light = normalize(camera_pos - wPos);
    // directional light 
    vec3 result = CalcDirLight(directional_light, frag_to_light);
    if (!directional_only) {
        // point lights 
        for (int i = 0; i < NR_POINT_LIGHTS; i++) {
            result += CalcPointLight(point_light[i], frag_to_light);
        }
        // spot lights
        for (int i = 0; i < NR_SPOT_LIGHTS; i++)
            result += CalcSpotLight(spot_light[i], frag_to_light);
    }
    FragColor = vec4(result, 1.0f);
}

vec3 CalcDirLight(DirLight light, vec3 camera_dir) {

    vec3 frag_to_light = normalize(-light.direction);

    // diffuse component 
    float diff_imp = max(dot(wNorm, frag_to_light), 0.0f);

    // specular component
    vec3 refl_dir = reflect(-frag_to_light, wNorm);
    float spec_imp = pow(max(dot(camera_dir, refl_dir), 0.0f), material.shininess);

    // scale components
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff_imp * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec_imp * vec3(texture(material.specular, TexCoords));

    // combine results
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 camera_dir) {
    // points from fragment to light source
    vec3 light_dir = normalize(light.position - wPos);
    // diffuse component
    float diff_imp = max(dot(wNorm, light_dir), 0.0f);
    // specular component
    vec3 refl_dir = reflect(-light_dir, wNorm);
    float spec_imp = pow(max(dot(camera_dir, refl_dir), 0.0f), material.shininess);
    // attenuation
    float dist = length(light.position - wPos);
    float attenuation = 1.0f / (light.constant + light.linear * dist + light.quadratic * (dist * dist));
    // scale components
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff_imp * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec_imp * vec3(texture(material.specular, TexCoords));
    // attenuate components
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    // combine results
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 camera_dir) {
    // points from fragment to light source
    vec3 light_dir = normalize(light.position - wPos);
    // diffuse component
    float diff_imp = max(dot(wNorm, light_dir), 0.0f);
    // specular component
    vec3 refl_dir = reflect(-light_dir, wNorm);
    float spec_imp = pow(max(dot(camera_dir, refl_dir), 0.0f), material.shininess);
    // attenuation
    float dist = length(light.position - wPos);
    float attenuation = 1.0f / (light.constant + light.linear * dist + light.quadratic * (dist * dist));
    // spotlight intensity
    float theta = dot(light_dir, normalize(-light.direction));
    float epsilon = light.inner_cut_off - light.outer_cut_off;
    float spot_intensity = clamp((theta - light.outer_cut_off) / epsilon, 0.0f, 1.0f);
    // scale components
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff_imp * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec_imp * vec3(texture(material.specular, TexCoords));
    // attenuate components
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    // combine results
    return (ambient + diffuse + specular);
}