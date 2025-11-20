#ifndef ISLAND_UTILS_WATER_FRAME_BUFFERS_H_
#define ISLAND_UTILS_WATER_FRAME_BUFFERS_H_
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class WaterFrameBuffers {
public:
    WaterFrameBuffers();

    void CleanUp();

    void BindReflectionFrameBuffer();
    void BindRefractionFrameBuffer();
    void UnbindCurrentFrameBuffer();

    unsigned int GetReflectionTexture();
    unsigned int GetRefractionTexture();
    unsigned int GetRefractionDepthTexture();

private:
    const unsigned int kReflectionWidth = 320;
    const unsigned int kReflectionHeight = 180;

    const unsigned int kRefractionWidth = 320;
    const unsigned int kRefractionHeight = 720;

    unsigned int refl_frame_buffer;
    unsigned int refl_texture;
    unsigned int refl_depth_buffer;

    unsigned int refr_frame_buffer;
    unsigned int refr_texture;
    unsigned int refr_depth_texture;

    void InitReflectionFrameBuffer();
    void InitRefractionFrameBuffer();

    void BindFrameBuffer(unsigned int frame_buffer, int width, int height);

    unsigned int CreateFrameBuffer();
    unsigned int CreateTextureAttachment(int width, int height);
    unsigned int CreateDepthTextureAttachment(int width, int height);
    unsigned int CreateDepthBufferAttachment(int width, int height);
};
    
#endif // ISLAND_UTILS_WATER_FRAME_BUFFERS_H_