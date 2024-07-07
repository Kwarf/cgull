#pragma once

#include <optional>

#define INCBIN_SILENCE_BITCODE_WARNING
#include <incbin.h>
#include <raylib-cpp.hpp>

#include "Constants.h"
#include "Scene.h"
#include "ShaderWrapper.h"
#include "TimeSource.h"

INCBIN(ridges_fs, "../src/scenes/Ridges.fs");

class Ridges : public Scene {
public:
  Ridges(const TimeSource &timeSource)
      : timeSource(timeSource),
#ifdef NDEBUG
        shaderWrapper(gridges_fsData, gridges_fsSize),
#else
        shaderWrapper("../src/scenes/Ridges.fs"),
#endif
        locResolution(shaderWrapper.getShader().GetLocation("resolution")),
        locTime(shaderWrapper.getShader().GetLocation("time")){};

  void render() override {
    auto &shader = shaderWrapper.getShader();

    const float resolution[] = {RENDER_WIDTH, RENDER_HEIGHT};
    shader.SetValue(locResolution, &resolution, SHADER_UNIFORM_VEC2);
    const float time = timeSource.now();
    shader.SetValue(locTime, &time, SHADER_UNIFORM_FLOAT);

    shader.BeginMode();
    DrawRectangle(0, 0, RENDER_WIDTH, RENDER_HEIGHT, WHITE);
    shader.EndMode();
  }

private:
  const TimeSource &timeSource;
  ShaderWrapper shaderWrapper;
  int locResolution;
  int locTime;
};
