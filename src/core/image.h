#pragma once

#include <tempo.h>


/**
 * Image class for holding images in memory to be drawn to Dear ImGui
 */
class Image {
public:
    enum Filtering { FILTER_NEAREST, FILTER_BILINEAR };
    enum Format { RGBA, ARGB };
private:
    GLuint texture_ = -1;
    int m_width = 0;
    int height_ = 0;
    int m_samples = 4;

    bool m_success = false;

    void load_texture(unsigned char* data, int width, int height, Filtering filtering);
    void load_texture_from_file(const char* filename, Filtering filtering);
    void load_texture_from_memory(unsigned char* data, int width, int height, Filtering filtering);
public:
    Image() = default;
    ~Image();

    /**
     * Set image from disk
     * @param filename path of the image (jpg, png)
     * @return if successful or not
     */
    bool setImage(const char* filename, Filtering filtering = FILTER_NEAREST);


    /**
     * Set image from memory
     * @param data RGB array
     * @return if successful or not
     */
    bool setImage(unsigned char* data, int width, int height, Filtering filtering = FILTER_NEAREST, Format format = ARGB);

    /**
     * Erases any content in the image
     * After this function, isImageSet will return false
     */
    void reset();

    /**
     * @return if the image has been successfully loaded in mage
     */
    bool isImageSet() const { return m_success; }

    /**
     * @return width of image as stored in memory
     */
    int width() const { return m_width; }
    /**
     * @return height of image as stored in memory
     */
    int height() const { return height_; }
    /**
     * @return GL texture of image
     */
    void* texture() const { return (void*)(intptr_t)texture_; }

};
