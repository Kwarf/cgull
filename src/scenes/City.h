#pragma once

#include <optional>

#define INCBIN_SILENCE_BITCODE_WARNING
#include <incbin.h>
#include <raylib-cpp.hpp>

#include "Constants.h"
#include "Rocket.h"
#include "Scene.h"
#include "ShaderWrapper.h"
#include "TimeSource.h"

INCBIN(city_fs, "../src/scenes/City.fs");

class City : public Scene {
public:
    City(const TimeSource &timeSource, const Rocket &rocket)
        : timeSource(timeSource), rocket(rocket),
#ifdef NDEBUG
          shaderWrapper(gcity_fsData, gcity_fsSize),
#else
          shaderWrapper("../../src/scenes/City.fs"),
#endif
          locResolution(shaderWrapper.getShader().GetLocation("resolution")),
          locTime(shaderWrapper.getShader().GetLocation("time")),
          locCameraPosition(shaderWrapper.getShader().GetLocation("cameraPosition")),
          locCameraTarget(shaderWrapper.getShader().GetLocation("cameraTarget")),
          cameraPositionX(rocket.track("camera_position:x")), cameraPositionY(rocket.track("camera_position:y")),
          cameraPositionZ(rocket.track("camera_position:z")), cameraTargetX(rocket.track("camera_target:x")),
          cameraTargetY(rocket.track("camera_target:y")), cameraTargetZ(rocket.track("camera_target:z")){};

    void render() override {
        auto &shader = shaderWrapper.getShader();

        const float resolution[] = {RENDER_WIDTH, RENDER_HEIGHT};
        shader.SetValue(locResolution, &resolution, SHADER_UNIFORM_VEC2);
        const float time = timeSource.now();
        shader.SetValue(locTime, &time, SHADER_UNIFORM_FLOAT);

        float vec3[3];
        vec3[0] = cameraPositionX.value();
        vec3[1] = cameraPositionY.value();
        vec3[2] = cameraPositionZ.value();
        shader.SetValue(locCameraPosition, &vec3, SHADER_UNIFORM_VEC3);
        vec3[0] = cameraTargetX.value();
        vec3[1] = cameraTargetY.value();
        vec3[2] = cameraTargetZ.value();
        shader.SetValue(locCameraTarget, &vec3, SHADER_UNIFORM_VEC3);

        shader.BeginMode();
        DrawRectangle(0, 0, RENDER_WIDTH, RENDER_HEIGHT, WHITE);
        shader.EndMode();
    }

private:
    const TimeSource &timeSource;
    const Rocket &rocket;
    ShaderWrapper shaderWrapper;

    int locResolution;
    int locTime;
    int locCameraPosition;
    int locCameraTarget;

    Rocket::Track cameraPositionX;
    Rocket::Track cameraPositionY;
    Rocket::Track cameraPositionZ;
    Rocket::Track cameraTargetX;
    Rocket::Track cameraTargetY;
    Rocket::Track cameraTargetZ;
};
