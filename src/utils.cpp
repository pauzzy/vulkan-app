#include "utils.h"

std::string readTextFile(const std::string_view filepath)
{
    std::ifstream file(filepath.data());

    if(file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        const std::string output = buffer.str();
        file.close();
        return output;
    }

    return std::string();
}