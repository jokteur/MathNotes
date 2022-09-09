#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


void Image::reset() {
    if (m_success) {
        m_success = false;
        m_width = 0;
        m_height = 0;
        glDeleteTextures(1, &texture_);
    }
    m_data = std::make_shared<ARGB_Image>();
}
Image::~Image() {
    reset();
}
void Image::load_texture(Filtering filtering) {
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

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(*m_data)[0]);
    texture_ = image_texture;
}

void Image::load_texture_from_file(const char* filename, Filtering filtering) {
    reset();

    // Load from file
    unsigned char* image_data = stbi_load(filename, &m_width, &m_height, NULL, 4);
    if (image_data == nullptr)
        return;

    m_data = std::make_shared<ARGB_Image>();
    m_data->resize(m_width * m_height * 4);
    memcpy(&(*m_data)[0], image_data, sizeof(unsigned char) * m_width * m_height * 4);

    load_texture(filtering);

    stbi_image_free(image_data);
    m_success = true;
}

void Image::load_texture_from_memory(unsigned char* data, int width, int height, Filtering filtering) {
    reset();

    m_data = std::make_shared<ARGB_Image>();
    m_data->resize(width * height * 4);
    memcpy(&(*m_data)[0], data, sizeof(unsigned char) * width * height * 4);

    m_width = width;
    m_height = height;

    load_texture(filtering);
    m_success = true;

}

bool Image::setImage(const char* filename, Filtering filtering) {
    load_texture_from_file(filename, filtering);
    return m_success;
}

bool Image::setImage(unsigned char* data, int width, int height, Filtering filtering, Format format) {
    load_texture_from_memory(data, width, height, filtering);
    return m_success;
}
bool Image::setImage(ARGB_Imageptr data, int width, int height, Filtering filtering, Format format) {
    m_data = data;
    m_width = width;
    m_height = height;
    load_texture(filtering);
    return m_success;
}