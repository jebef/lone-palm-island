#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>

#include "utils/core.h"
#include "utils/shader.h"
#include "utils/camera.h"
#include "utils/stb_image.h"
#include "utils/model.h"
#include "utils/water_frame_buffers.h"

#include "utils/stb_image.h"

/*
    1. Setup window
    2. Register callback functions 
    3. Compile shaders
    4. Load models 
    5. Set uniforms 
    6. Render 
*/

// ----------- FUNCTION HEADERS ----------- //
void RenderScene(const std::vector<Model> models, Shader shader, glm::vec3 camera_pos, glm::mat4 view, glm::mat4 projection);
void RenderWater(Shader shader, Model model, glm::mat4 view, glm::mat4 projection, unsigned int refl_tex_id, unsigned int refr_tex_id, unsigned int dudv_map_id, unsigned int normal_map_id);
void RenderWaterGui(Shader shader, unsigned int VAO, unsigned int texture_id, unsigned int index_offset);
void RenderDebugAxes(Shader shader, unsigned int VAO);
void RenderLightOrbs(Shader shader, Model model, glm::mat4 model_mat, glm::mat4 view, glm::mat4 projection, glm::vec3 light_color);

int main() 
{
    // ----------- CONTEXT INIT AND SETUP ----------- //
    // init glfw and create a window 
    GLFWwindow* g_window = CreateWindow(3, 3, GLFW_OPENGL_CORE_PROFILE, g_screen_width, g_screen_height, "ISLAND DEMO");
    if (g_window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(g_window);

    // determine screen unit pixel ratio
    float width_scale, height_scale;
    glfwGetWindowContentScale(g_window, &width_scale, &height_scale);
    g_screen_width_p = static_cast<unsigned int>(g_screen_width * width_scale);
    g_screen_height_p = static_cast<unsigned int>(g_screen_height * height_scale);

    // capture cursor 
    glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // register glfw callback functions here 
    glfwSetFramebufferSizeCallback(g_window, FramebufferSizeCallback);
    glfwSetCursorPosCallback(g_window, MouseCallback);
    glfwSetScrollCallback(g_window, ScrollCallback);

    // load opengl function pointers with glad 
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to retrieve OpenGL function pointers with GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    //stbi_set_flip_vertically_on_load(true);

    // ----------- CONSTRUCT SHADER PROGRAMS ----------- //
    Shader terrain_shader = Shader("shaders/terrain.vert", "shaders/terrain.frag");
    Shader water_shader = Shader("shaders/water.vert", "shaders/water.frag");
    Shader island_cap_refraction_shader = Shader("shaders/island-cap.vert", "shaders/island-cap.frag", "shaders/island-cap-refract.geom");
    Shader island_cap_reflection_shader = Shader("shaders/island-cap.vert", "shaders/island-cap.frag", "shaders/island-cap-reflect.geom");
    Shader gui_debug_shader = Shader("shaders/gui.vert", "shaders/gui.frag");
    Shader axes_debug_shader("shaders/axes.vert", "shaders/axes.frag");
    Shader light_orb_shader("shaders/light_orb.vert", "shaders/light_orb.frag");

    const std::vector<Shader> lit_shaders = { terrain_shader, water_shader, island_cap_reflection_shader, island_cap_refraction_shader };

    // ----------- LOAD MODELS ----------- //
    Model palm_tree("assets/models/palm_tree/palm-tree.obj");
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));
    model = glm::translate(model, glm::vec3(-4.0f, 2.0f, 2.0f));
    palm_tree.SetModelMatrix(model);
    palm_tree.SetSpecularIntensity(1.0f);

    Model island("assets/models/island/island.obj");
    model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(2.0f));
    model = glm::translate(model, glm::vec3(0.0f, -10.0f, 0.0f));
    island.SetModelMatrix(model);
    island.SetSpecularIntensity(0.0f);

    Model water("assets/models/water/water.obj");
    model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(10.0f));
    water.SetModelMatrix(model);
    water.SetSpecularIntensity(1.0f);

    Model light_orb("assets/models/light_orb/light_orb.obj");

    const std::vector<Model> terrain_models = { island, palm_tree };
    

    // ----------- DEFINE LIGHTING UNIFORMS ----------- // 

    // global properties 
    bool directional_only = false; // default is false
    float reflectivity = 32.0f;
    // directional light 
    glm::vec3 dl_direction = glm::vec3(0.0f, -1.0f, 0.0f);

    glm::vec3 dl_ambient = glm::vec3(0.1f, 0.1f, 0.1f);
    glm::vec3 dl_diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
    glm::vec3 dl_specular = glm::vec3(1.0f, 1.0f, 1.0f);
    // glm::vec3 dl_ambient = glm::vec3(0.0f);
    // glm::vec3 dl_diffuse = glm::vec3(0.0f);
    // glm::vec3 dl_specular = glm::vec3(0.0f);

    // point lights
    glm::vec3 light_position = glm::vec3(3.0f, 3.0f, -3.0f);
    glm::vec3 pl_position = light_position;
    light_orb_shader.use();
    model = glm::mat4(1.0f);
    model = glm::translate(model, pl_position);
    light_orb_shader.setMat4("model", model);

    glm::vec3 pl_ambient = glm::vec3(0.1f, 0.1f, 0.1f);
    glm::vec3 pl_diffuse = glm::vec3(1.0f, 0.2f, 0.0f);
    glm::vec3 pl_specular = glm::vec3(1.0f, 1.0f, 1.0f);
    // glm::vec3 pl_ambient = glm::vec3(0.0f);
    // glm::vec3 pl_diffuse = glm::vec3(0.0f);
    // glm::vec3 pl_specular = glm::vec3(0.0f);

    float pl_constant = 1.0f;
    float pl_linear = 0.09f;
    float pl_quadratic = 0.032f;
    // spot lights 
    glm::vec3 sl_position = glm::vec3(3.0f, 1.5f, 3.0f);
    glm::vec3 sl_direction = glm::vec3(-1.0f, -0.1f, -1.0f);

    // glm::vec3 sl_ambient = glm::vec3(0.1f);
    // glm::vec3 sl_diffuse = glm::vec3(1.0f, 0.8f, 0.8f);
    // glm::vec3 sl_specular = glm::vec3(1.0f);
    glm::vec3 sl_ambient = glm::vec3(0.0f);
    glm::vec3 sl_diffuse = glm::vec3(0.0f);
    glm::vec3 sl_specular = glm::vec3(0.0f);

    float sl_constant = 1.0f;
    float sl_linear = 0.2f;
    float sl_quadratic = 0.032f;
    float sl_inner_cut_off = glm::cos(glm::radians(12.5f));
    float sl_outer_cut_off = glm::cos(glm::radians(15.0f));


    // ----------- SET LIGHTING UNIFORMS ----------- // 
    for (const Shader& shader : lit_shaders) {
        // activate shader 
        shader.use();
        // global
        shader.setBool("directional_only", directional_only);
        shader.setFloat("material.shininess", reflectivity);
        // directional
        shader.setVec3("directional_light.direction", dl_direction);
        shader.setVec3("directional_light.ambient", dl_ambient);
        shader.setVec3("directional_light.diffuse", dl_diffuse);
        shader.setVec3("directional_light.specular", dl_specular);
        // point lights
        shader.setVec3("point_light[0].position", pl_position);
        shader.setVec3("point_light[0].ambient", pl_ambient);
        shader.setVec3("point_light[0].diffuse", pl_diffuse);
        shader.setVec3("point_light[0].specular", pl_specular);
        shader.setFloat("point_light[0].constant", pl_constant);
        shader.setFloat("point_light[0].linear", pl_linear);
        shader.setFloat("point_light[0].quadratic", pl_quadratic);
        // spot lights
        shader.setVec3("spot_light[0].position", sl_position);
        shader.setVec3("spot_light[0].direction", sl_direction);
        shader.setVec3("spot_light[0].ambient", sl_ambient);
        shader.setVec3("spot_light[0].diffuse", sl_diffuse);
        shader.setVec3("spot_light[0].specular", sl_specular);
        shader.setFloat("spot_light[0].constant", sl_constant);
        shader.setFloat("spot_light[0].linear", sl_linear);
        shader.setFloat("spot_light[0].quadratic", sl_quadratic);
        shader.setFloat("spot_light[0].inner_cut_off", sl_inner_cut_off);
        shader.setFloat("spot_light[0].outer_cut_off", sl_outer_cut_off);
    }


    // ----------- LOAD/SET WATER TEXTURES/BUFFERS ----------- // 
    // set texure unit uniforms
    water_shader.use();
    water_shader.setInt("reflection_texture", 0);
    water_shader.setInt("refraction_texture", 1);
    water_shader.setInt("dudv_map", 2);
    water_shader.setInt("normal_map", 3);
    // load dudv and normal textures 
    unsigned int water_dudv = TextureFromFile("assets/textures/water/dudv.png", ".");
    unsigned int water_normal = TextureFromFile("assets/textures/water/normal.png", ".");
    // init water frame buffers 
    WaterFrameBuffers(water_fbos);
    // set static model matrix uniform 
    water_shader.setMat4("model", water.model_matrix);


    // ----------- DEBUG WATER GUI ----------- //
    float vd_gui[] = {
        // reflection gui
        -1.0f, 1.0f, 0.0f,  0.0f, 1.0f, // top left
        -0.5f, 1.0f, 0.0f,  1.0f, 1.0f, // top right
        -0.5f, 0.5f, 0.0f,  1.0f, 0.0f, // bottom right
        -1.0f, 0.5f, 0.0f,  0.0f, 0.0f, // bottom left
        // refraction gui
         0.5f, -0.5f, 0.0f,  0.0f, 1.0f, // top left
         1.0f, -0.5f, 0.0f,  1.0f, 1.0f, // top right
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f, // bottom right
         0.5f, -1.0f, 0.0f,  0.0f, 0.0f // bottom left
    };
    unsigned int i_gui[] = {
        // reflection gui 
        0, 1, 2, // first triangle
        0, 2, 3, // second triangle
        // refraction gui 
        4, 5, 6, // first triangle
        4, 6, 7 // second triangle
    };
    // generate buffers
    unsigned int VAO_WGUI, VBO_WGUI, EBO_WGUI;
    glGenVertexArrays(1, &VAO_WGUI);
    glGenBuffers(1, &VBO_WGUI);
    glGenBuffers(1, &EBO_WGUI);
    glBindVertexArray(VAO_WGUI);
    // configure vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO_WGUI);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vd_gui), vd_gui, GL_STATIC_DRAW);
    // configure vertex array
    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coordinates
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // configure element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_WGUI);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(i_gui), i_gui, GL_STATIC_DRAW);
    // unbind VAO_WGUI
    glBindVertexArray(0);
    // set texture unit uniform 
    gui_debug_shader.use();
    gui_debug_shader.setInt("texture_id", 0);


    // ----------- DEBUG AXES ----------- //
    float vd_axes[] = {
        // x axis
        -1.0f,  0.0f,  0.0f, 
         1.0f,  0.0f,  0.0f, 
        // y axis 
         0.0f, -1.0f,  0.0f, 
         0.0f,  1.0f,  0.0f, 
         // z axis 
         0.0f,  0.0f, -1.0f, 
         0.0f,  0.0f,  1.0f
    };
    // generate buffers 
    unsigned int VAO_AX, VBO_AX;
    glGenVertexArrays(1, &VAO_AX);
    glGenBuffers(1, &VBO_AX);
    // bind vertex array object 
    glBindVertexArray(VAO_AX);
    // configure vertex buffer 
    glBindBuffer(GL_ARRAY_BUFFER, VBO_AX);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vd_axes), vd_axes, GL_STATIC_DRAW);
    // configure vertex array attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); 
    glEnableVertexAttribArray(0);
    // unbind 
    glBindVertexArray(0);
    // set static model uniform 
    axes_debug_shader.use();
    glm::mat4 model_axes = glm::mat4(1.0f);
    model_axes = glm::scale(model_axes, glm::vec3(10.0f));
    axes_debug_shader.setMat4("model", model_axes);


    // ----------- SET GLOBAL STATES ----------- //
    glEnable(GL_DEPTH_TEST);

    float osc = 0.0f;
    float day_speed = 0.25f;
    glm::vec3 sky_color = glm::vec3(0.7f, 0.7f, 1.0f);

    // ----------- MAIN RENDER LOOP ----------- //
    while (!glfwWindowShouldClose(g_window)) {
        // per-frame time logic
        g_current_frame = static_cast<float>(glfwGetTime());
        g_delta_time = g_current_frame - g_last_frame;
        g_last_frame = g_current_frame;

        // handle user input 
        ProcessInput(g_window);

        // update float oscillator
        osc = (cos(g_current_frame * day_speed) + 1.0f) / 2.0f;

        // day simulation
        for (const Shader& shader : lit_shaders) {
            shader.use();
            // update directional light output 
            glm::vec3 dl_new_ambient = dl_ambient * osc;
            glm::vec3 dl_new_diffuse = dl_diffuse * osc;
            glm::vec3 dl_new_specular = dl_specular * osc;
            shader.setVec3("directional_light.ambient", dl_new_ambient);
            shader.setVec3("directional_light.diffuse", dl_new_diffuse);
            shader.setVec3("directional_light.specular", dl_new_specular);
            // update point light output 
            glm::vec3 pl_new_ambient = pl_ambient * (1.0f - osc);
            glm::vec3 pl_new_diffuse = pl_diffuse * (1.0f - osc);
            glm::vec3 pl_new_specular = pl_specular * (1.0f - osc);
            shader.setVec3("poin_light[0].ambient", pl_new_ambient);
            shader.setVec3("point_light[0].diffuse", pl_new_diffuse);
            shader.setVec3("point_light[0].specular", pl_new_specular);
        }

        // update position of point light 
        pl_position.y = light_position.y - (osc * 6.0f);
        glm::mat4 light_orb_model_mat = glm::mat4(1.0f);
        light_orb_model_mat = glm::translate(light_orb_model_mat, pl_position);
      

        // enable clipping 
        glEnable(GL_CLIP_DISTANCE0);

        // retrieve view matrix 
        glm::mat4 view_mat = g_camera.GetViewMatrix();
        // retrieve projection matrix
        glm::mat4 projection_mat = glm::perspective(glm::radians(g_camera.zoom_), (float)g_screen_width / (float)g_screen_height, 0.1f, 100.0f);

        
        // --- RENDER SCENE TO REFLECTION BUFFER --- //
        // activate terrain shader 
        terrain_shader.use();
        // calculate reflected camera position 
        glm::vec3 reflected_camera_pos = glm::vec3(g_camera.position_.x, -g_camera.position_.y, g_camera.position_.z);
        // calculate reflected camera target/front 
        glm::vec3 reflected_target = glm::vec3(g_camera.position_ + g_camera.front_);
        reflected_target.y = -reflected_target.y;
        // calculate reflected view matrix 
        glm::mat4 reflection_view_mat = glm::lookAt(reflected_camera_pos, reflected_target, glm::vec3(0.0f, 1.0f, 0.0f));
        // define reflection clip plane and set uniform
        glm::vec4 reflection_clip_plane = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
        terrain_shader.setVec4("clip_plane", reflection_clip_plane);
        // bind reflection framebuffer and render terrain
        water_fbos.BindReflectionFrameBuffer();
        //glClearColor(sky_color.r * osc, sky_color.g * osc, sky_color.b * osc, 1.0f);
        //glClearColor(sky_color.r, sky_color.g, sky_color.b, 1.0f);
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderScene(terrain_models, terrain_shader, reflected_camera_pos, reflection_view_mat, projection_mat);
        // render island reflection cap
        island_cap_reflection_shader.use();
        island_cap_reflection_shader.setVec4("clip_plane", reflection_clip_plane);
        RenderScene({island}, island_cap_reflection_shader, reflected_camera_pos, reflection_view_mat, projection_mat);
        // unbind reflection framebuffer  
        water_fbos.UnbindCurrentFrameBuffer();

        // --- RENDER SCENE TO REFRACTION BUFFER --- //
        // activate terrain shader 
        terrain_shader.use();
        // define refraction clip plane and set uniform
        glm::vec4 refraction_clip_plane = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
        terrain_shader.setVec4("clip_plane", refraction_clip_plane);
        // bind refraction framebuffer and render terrain
        water_fbos.BindRefractionFrameBuffer();
        //glClearColor(sky_color.r * osc, sky_color.g * osc, sky_color.b * osc, 1.0f);
        //glClearColor(sky_color.r, sky_color.g, sky_color.b, 1.0f);
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderScene(terrain_models, terrain_shader, g_camera.position_, view_mat, projection_mat);
        // render island refraction cap
        island_cap_refraction_shader.use();
        island_cap_refraction_shader.setVec4("clip_plane", refraction_clip_plane);
        RenderScene({island}, island_cap_refraction_shader, g_camera.position_, view_mat, projection_mat);
        // unbind refraction framebuffer 
        water_fbos.UnbindCurrentFrameBuffer();

        // disable clipping 
        glDisable(GL_CLIP_DISTANCE0);

        // --- RENDER SCENE --- //
        glClearColor(sky_color.r * osc, sky_color.g * osc, sky_color.b * osc, 1.0f); // day simulation - sky color
        // glClearColor(sky_color.r, sky_color.g, sky_color.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderScene(terrain_models, terrain_shader, g_camera.position_, view_mat, projection_mat);
        
        // --- RENDER WATER --- //
        RenderWater(water_shader, water, view_mat, projection_mat, water_fbos.GetReflectionTexture(), water_fbos.GetRefractionTexture(), water_dudv, water_normal);

        // --- RENDER LIGHT ORBS --- //
        if (!directional_only)
            RenderLightOrbs(light_orb_shader, light_orb, light_orb_model_mat, view_mat, projection_mat, pl_diffuse);

        // DEBUG - water texture guis and axes 
        // RenderWaterGui(gui_debug_shader, VAO_WGUI, water_fbos.GetReflectionTexture(), 0);
        // RenderWaterGui(gui_debug_shader, VAO_WGUI, water_fbos.GetRefractionTexture(), 6);
        // RenderDebugAxes(axes_debug_shader, VAO_AX);

        // swap frame and output buffers
        glfwSwapBuffers(g_window);
        // check for I/O events 
        glfwPollEvents();
    }
    // ----------- FREE RESOURCES ----------- //
    glDeleteVertexArrays(1, &VAO_WGUI);
    glDeleteVertexArrays(1, &VAO_AX);
    glDeleteBuffers(1, &VBO_WGUI);
    glDeleteBuffers(1, &VBO_AX);
    water_fbos.CleanUp();
    // free glfw resources 
    glfwTerminate();
    return 0; 
}

/*
    Render specified models to the actvive frame buffer.
*/
void RenderScene(const std::vector<Model> models, Shader shader, glm::vec3 camera_pos, glm::mat4 view, glm::mat4 projection) {

    shader.use();

    shader.setVec3("camera_pos", camera_pos);

    shader.setMat4("view", view);

    shader.setMat4("projection", projection);

    // render models 
    for (const Model& model : models) {
        // set model matrix uniform 
        shader.setMat4("model", model.model_matrix);
        // compute/set normal matrix uniform 
        glm::mat3 normal = glm::mat3(glm::transpose(glm::inverse(model.model_matrix)));
        shader.setMat3("normal", normal);

        shader.setFloat("specular_intenstiy", model.specular_intensity);

        // draw model
        model.Draw(shader);
    }
}

/*
    Render water model to the active frame buffer.
*/
void RenderWater(Shader shader, Model model, glm::mat4 view, glm::mat4 projection,
                 unsigned int refl_tex_id, unsigned int refr_tex_id, unsigned int dudv_map_id, unsigned int normal_map_id) {

    shader.use();

    shader.setVec3("camera_pos", g_camera.position_);

    shader.setMat4("view", view);

    shader.setMat4("projection", projection);

    // bind reflection texture 
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, refl_tex_id);
    shader.setInt("reflection_texture", 0);
    // bind refraction texture 
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, refr_tex_id);
    shader.setInt("refraction_texture", 1);
    // bind dudv map texture 
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, dudv_map_id);
    shader.setInt("dudv_map", 2);
    // bind normal map texture 
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, normal_map_id);
    shader.setInt("normal_map", 3);
    // update dudv/normal sampling offset 
    g_movement_factor = fmod(g_current_frame * g_wave_speed, 1.0f);
    shader.setFloat("sampling_offset", g_movement_factor);

    shader.setFloat("specular_intenstiy", model.specular_intensity);

    // render water 
    model.Draw(shader);
}

void RenderWaterGui(Shader shader, unsigned int VAO, unsigned int texture_id, unsigned int index_offset) {
    glDisable(GL_DEPTH_TEST);
    shader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(index_offset * sizeof(GLuint)));
    glEnable(GL_DEPTH_TEST);
}

void RenderDebugAxes(Shader shader, unsigned int VAO) {
    // bind axes shader program
    shader.use();
    // update dynamic matrix uniforms 
    glm::mat4 projection = glm::perspective(glm::radians(g_camera.zoom_), (float)g_screen_width / (float)g_screen_height, 0.1f, 100.0f);
    shader.setMat4("projection", projection);
    glm::mat4 view = g_camera.GetViewMatrix();
    shader.setMat4("view", view);
    // x axis
    shader.setVec3("axisColor", 1.0f, 0.0f, 0.0f);
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, 2);
    // y axis 
    shader.setVec3("axisColor", 0.0f, 1.0f, 0.0f);
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 2, 2);
    // z axis 
    shader.setVec3("axisColor", 0.0f, 0.0f, 1.0f);
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 4, 2);
}

void RenderLightOrbs(Shader shader, Model model, glm::mat4 model_mat, glm::mat4 view, glm::mat4 projection, glm::vec3 light_color) {
    shader.use();
    shader.setMat4("model", model_mat);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setVec3("light_color", light_color);
    model.Draw(shader);
}
