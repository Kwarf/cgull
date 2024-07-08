#pragma once

#include <optional>

#include <raylib-cpp.hpp>

#include "FileWatcher.h"

class ShaderWrapper {
public:
#ifdef NDEBUG
    ShaderWrapper(const unsigned char data[], const unsigned int size) {
        std::vector<char> buffer(size + 1, 0);
        std::copy(data, data + size, buffer.begin());
        shader = raylib::Shader::LoadFromMemory(0, buffer.data());
    }
#else
    ShaderWrapper(const std::string &fsFileName)
        : shader({0, fsFileName.c_str()}), watcher(
                                               fsFileName,
                                               [](void *context, const std::string &path) {
                                                   TraceLog(LOG_INFO, "Reloading shader %s", path.c_str());
                                                   ShaderWrapper *self = static_cast<ShaderWrapper *>(context);
                                                   self->shader = raylib::Shader::Load(0, path.c_str());
                                               },
                                               this) {}
#endif

    raylib::Shader &getShader() {
        return *shader;
    }

private:
    std::optional<raylib::Shader> shader;
#ifndef NDEBUG
    FileWatcher watcher;
#endif
};
