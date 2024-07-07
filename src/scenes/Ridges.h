#pragma once

#include <optional>

#define INCBIN_SILENCE_BITCODE_WARNING
#include <incbin.h>
#include <raylib-cpp.hpp>

#include "Constants.h"
#include "EmbeddedShader.h"
#include "Scene.h"
#include "TimeSource.h"

INCBIN(ridges_fs, "../src/scenes/Ridges.fs");

class Ridges : public Scene {
public:
  Ridges(const TimeSource &timeSource)
      : timeSource(timeSource),
        shader(EmbeddedShader::fromIncbinData(gridges_fsData, gridges_fsSize)),
        locResolution(shader.GetLocation("resolution")),
        locTime(shader.GetLocation("time")){};

  void render() override {
    const float resolution[] = {RENDER_WIDTH, RENDER_HEIGHT};
    shader.SetValue(locResolution, &resolution, SHADER_UNIFORM_VEC2);
    const float time = timeSource.now();
    shader.SetValue(locTime, &time, SHADER_UNIFORM_FLOAT);

    shader.BeginMode();
    DrawRectangle(0, 0, RENDER_WIDTH, RENDER_HEIGHT, WHITE);
    DrawLine(0, 0, RENDER_WIDTH / 2, RENDER_HEIGHT / 2, BLUE);
    shader.EndMode();
  }

private:
  const TimeSource &timeSource;
  raylib::Shader shader;
  int locResolution;
  int locTime;
};
