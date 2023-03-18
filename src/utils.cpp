#include "utils.h"

void getFileContents(const std::string& filename, std::string& contents) {
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (in) {
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
    }
    else
        throw(errno);
}