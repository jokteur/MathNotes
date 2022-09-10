#pragma once

#include <vector>
#include <tempo.h>

using ARGB_Image = std::vector<unsigned char>;
using ARGB_Imageptr = std::shared_ptr<ARGB_Image>;
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
    int m_height = 0;
    int m_samples = 4;

    bool m_success = false;

    ARGB_Imageptr m_data = nullptr;

    void load_texture(Filtering filtering);
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
     * Set image from memory (makes a local copy of data)
     * @param data ARGB array
     * @return if successful or not
     */
    bool setImage(unsigned char* data, int width, int height, Filtering filtering = FILTER_NEAREST, Format format = ARGB);

    /**
     * Set image from memory (makes no copy)
     * @param data ARGB array
     * @return if successful or not
     */
    bool setImage(ARGB_Imageptr data_ptr, int width, int height, Filtering filtering = FILTER_NEAREST, Format format = ARGB);

    /**
     * Erases any content in the image
     * After this function, isImageSet will return false
     */
    void reset();

    /**
     * @return if the image has been successfully loaded in memory
     */
    bool isImageSet() const { return m_success; }

    /**
     * @return width of image as stored in memory
     */
    int width() const { return m_width; }
    /**
     * @return height of image as stored in memory
     */
    int height() const { return m_height; }
    /**
     * @return GL texture of image
     * This function is not safe. Always check if image is set with isImageSet()
     */
    void* texture() const { return (void*)(intptr_t)texture_; }

};
