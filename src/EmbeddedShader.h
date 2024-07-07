#pragma once

#include <raylib-cpp.hpp>

class EmbeddedShader {
public:
  static raylib::Shader fromIncbinData(const unsigned char data[],
                                       const unsigned int size) {
    std::vector<char> buffer(size + 1, 0);
    std::copy(data, data + size, buffer.begin());
    return raylib::Shader::LoadFromMemory(0, buffer.data());
  }
};
