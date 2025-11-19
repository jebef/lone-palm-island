#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include "core.h"
#include "camera.h"
#include "stb_image_write.h"

// ----------- INIT GLOBAL VARIABLES ----------- // 
// Screen // 
GLFWwindow* g_window = nullptr;
const unsigned int g_screen_width = 1200;
const unsigned int g_screen_height= 900;
unsigned int g_screen_width_p = 0; // set dynamically in main
unsigned int g_screen_height_p = 0;
// Camera // 
glm::vec3 g_camera_position = glm::vec3(5.0f, 5.0f, 10.0f);
Camera g_camera(g_camera_position);
// Mouse // 
float g_last_x = g_screen_width / 2.0f;
float g_last_y = g_screen_height / 2.0f;
bool g_first_mouse = true;
// Time // 
float g_current_frame = 0.0f;
float g_delta_time = 0.0f; 
float g_last_frame = 0.0f;
// Water // 
float g_wave_speed = 0.05f;
float g_movement_factor = 0.0f;
// Out Image //
int g_out_file_index = 0;

// ----------- UTILITY FUNCTIONS ----------- // 
/*
    Initializes/configures GLFW. Returns a refernce to a
    GLFW window object. Does not check for success.
*/
GLFWwindow* CreateWindow(int version_major, int version_minor, int profile, int width, int height, const char *title)
{
    // init and configure glfw 
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, version_major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, version_minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, profile);
#ifdef __APPLE__ 
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    // create window 
    return glfwCreateWindow(width, height, title, NULL, NULL);
}

/*
    Captures a still of the scene and saves it to a png file. 
*/
void SaveScreenshot() {
    // construct output file name 
    std::string filename = "img" + std::to_string(g_out_file_index++) + ".png";
    const char* c_filename = filename.c_str();
    // init pixel data array 
    unsigned char* pixels = new unsigned char[g_screen_width_p * g_screen_height_p * 3]; 
    // read in pixel values from frame buffer 
    glReadPixels(0, 0, g_screen_width_p, g_screen_height_p, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    // flip vertical pixel data 
    unsigned char* flipped = new unsigned char[g_screen_width_p * g_screen_height_p * 3];
    for (int y = 0; y < g_screen_height_p; y++) {
        memcpy(flipped + (g_screen_height_p - 1 - y) * g_screen_width_p * 3, pixels + y * g_screen_width_p * 3, g_screen_width_p * 3);
    }
    // write pixel values to output file 
    stbi_write_png(c_filename, g_screen_width_p, g_screen_height_p, 3, flipped, g_screen_width_p * 3);
    std::cout << "Image saved!" << std::endl;
    // cleanup
    delete[] pixels;
    delete[] flipped;
}

/*
    Gets called every frame, handles user input.
*/
void ProcessInput(GLFWwindow* window) 
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        g_camera.ProcessKeyboard(FORWARD, g_delta_time);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        g_camera.ProcessKeyboard(BACKWARD, g_delta_time);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        g_camera.ProcessKeyboard(LEFT, g_delta_time);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        g_camera.ProcessKeyboard(RIGHT, g_delta_time);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        SaveScreenshot();
}

//----------- CALLBACK FUNCTIONS -----------//
/*
    Called whenever the window dimensions change. Resizes the viewport match the window.
*/
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

/*
    Called whenever the cursor position changes. Updates camera values accordingly.
*/
void MouseCallback(GLFWwindow* window, double x_pos_d, double y_pos_d) {
    float x_pos = static_cast<float>(x_pos_d);
    float y_pos = static_cast<float>(y_pos_d);

    if (g_first_mouse) {
        g_last_x = x_pos;
        g_last_y = y_pos;
        g_first_mouse = false;
    }

    float x_offset = x_pos - g_last_x;
    float y_offset = g_last_y - y_pos; // y-coords go from bottom to top

    g_last_x = x_pos;
    g_last_y = y_pos;

    g_camera.ProcessMouseMovement(x_offset, y_offset);
}

/*
    Called whenever the scroll wheel is moved. Updates camera zoom attribute. 
*/
void ScrollCallback(GLFWwindow* window, double x_offset, double y_offset) {
    g_camera.ProcessMouseScroll(static_cast<float>(y_offset));
}

