#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core.h"
#include "water_frame_buffers.h"

#include <iostream>


// ----------- PUBLIC ----------- // 
/*
    Initialize buffers.
*/
WaterFrameBuffers::WaterFrameBuffers() {
    InitReflectionFrameBuffer();
    InitRefractionFrameBuffer();
}

/* 
    Free all resources - frame buffers, render buffers, and textures. 
*/
void WaterFrameBuffers::CleanUp() {
    // reflection data 
    glDeleteFramebuffers(1, &refl_frame_buffer);
    glDeleteTextures(1, &refl_texture);
    glDeleteRenderbuffers(1, &refl_depth_buffer);
    // refraction data
    glDeleteFramebuffers(1, &refr_frame_buffer);
    glDeleteTextures(1, &refr_texture);
    glDeleteTextures(1, &refr_depth_texture);
}

void WaterFrameBuffers::BindReflectionFrameBuffer() {
    BindFrameBuffer(refl_frame_buffer, kReflectionWidth, kReflectionHeight);
}

void WaterFrameBuffers::BindRefractionFrameBuffer() {
    BindFrameBuffer(refr_frame_buffer, kRefractionWidth, kRefractionHeight);
}

void WaterFrameBuffers::UnbindCurrentFrameBuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);                  // 0 is default frame buffer id 
    glViewport(0, 0, g_screen_width_p, g_screen_height_p); // YOU WILL REMEMBER THIS BUG (p)
}

unsigned int WaterFrameBuffers::GetReflectionTexture() {
    return refl_texture;
}

unsigned int WaterFrameBuffers::GetRefractionTexture() {
    return refr_texture;
}

unsigned int WaterFrameBuffers::GetRefractionDepthTexture() {
    return refr_depth_texture;
}

// ----------- PRIVATE ----------- // 
void WaterFrameBuffers::InitReflectionFrameBuffer() {
    refl_frame_buffer = CreateFrameBuffer();
    refl_texture = CreateTextureAttachment(kReflectionWidth, kReflectionHeight);
    refl_depth_buffer = CreateDepthBufferAttachment(kReflectionWidth, kReflectionHeight);
    // check that framebuffer is complete 
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer not complete!" << std::endl;
    }
    UnbindCurrentFrameBuffer();
}

void WaterFrameBuffers::InitRefractionFrameBuffer() {
    refr_frame_buffer = CreateFrameBuffer();
    refr_texture = CreateTextureAttachment(kRefractionWidth, kRefractionHeight);
    refr_depth_texture = CreateDepthTextureAttachment(kRefractionWidth, kRefractionHeight);
    // check that framebuffer is complete 
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer not complete!" << std::endl;
    }
    UnbindCurrentFrameBuffer();
}

/*
    Bind the frame buffer with id frame_buffer and set the viewport 
    resolution to width x height. 
*/
void WaterFrameBuffers::BindFrameBuffer(unsigned int frame_buffer, int width, int height) {
    glBindTexture(GL_TEXTURE_2D, 0); // unbind any currently bound textures
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    glViewport(0, 0, width, height);
}

/*
    Creates a frame buffer object and returns its id.
*/
unsigned int WaterFrameBuffers::CreateFrameBuffer() {
    unsigned int frame_buffer;
    glGenFramebuffers(1, &frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    glDrawBuffer(GL_COLOR_ATTACHMENT0); 
    return frame_buffer;
}

/*
    Creates 2D texture object and binds it to the currently active 
    frame buffer. Returns the texture id.
*/
unsigned int WaterFrameBuffers::CreateTextureAttachment(int width, int height) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);
    return texture;
}

/*
    Creates 2D depth texture object and binds it to the currently active 
    frame buffer. Returns the texture id.
*/
unsigned int WaterFrameBuffers::CreateDepthTextureAttachment(int width, int height) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, (void*)0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);
    return texture;
}

/*
    Creates a render buffer object that stores depth information. Binds 
    the depth buffer to the currently active frame buffer. Returns the 
    depth buffer id.
*/
unsigned int WaterFrameBuffers::CreateDepthBufferAttachment(int width, int height) {
    unsigned int depth_buffer;
    glGenRenderbuffers(1, &depth_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
    return depth_buffer;
}


