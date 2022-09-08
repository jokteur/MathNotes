#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


void Image::reset() {
    if (m_success) {
        m_success = false;
        m_width = 0;
        height_ = 0;
        glDeleteTextures(1, &texture_);
    }
}
Image::~Image() {
    reset();
}
void Image::load_texture(unsigned char* data, int width, int height, Filtering filtering) {
    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    int gl_filter = 0;
    if (filtering == FILTER_NEAREST) {
        gl_filter = GL_NEAREST;
    }
    else if (filtering == FILTER_BILINEAR) {
        gl_filter = GL_LINEAR;
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter);

    // Upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    texture_ = image_texture;
}

void Image::load_texture_from_file(const char* filename, Filtering filtering) {
    reset();

    // Load from file
    unsigned char* image_data = stbi_load(filename, &m_width, &height_, NULL, 4);
    if (image_data == nullptr)
        return;

    load_texture(image_data, m_width, height_, filtering);

    stbi_image_free(image_data);
    m_success = true;
}

void Image::load_texture_from_memory(unsigned char* data, int width, int height, Filtering filtering) {
    reset();

    m_width = width;
    height_ = height;

    load_texture(data, m_width, height_, filtering);
    m_success = true;

}

bool Image::setImage(const char* filename, Filtering filtering) {
    load_texture_from_file(filename, filtering);
    return m_success;
}

bool Image::setImage(unsigned char* data, int width, int height, Filtering filtering, Format format) {
    unsigned char* out_data = data;
    // if (format == RGBA) {
        // out_data = new unsigned char[width * height * 4];
        // for (int j = 0; j < height; j++) {
        //     for (int i = 0; i < width; i++) {
        //         out_data[j * (width * 4) + i * 4] = data[j * (width * 4) + i * 4 + 3];
        //         out_data[j * (width * 4) + i * 4 + 1] = data[j * (width * 4) + i * 4 + 2];
        //         out_data[j * (width * 4) + i * 4 + 2] = data[j * (width * 4) + i * 4 + 1];
        //         out_data[j * (width * 4) + i * 4 + 3] = data[j * (width * 4) + i * 4];
        //     }
        // }
    // }
    load_texture_from_memory(out_data, width, height, filtering);
    // if (format == RGBA) {
    //     delete[]out_data;
    // }
    return m_success;
}