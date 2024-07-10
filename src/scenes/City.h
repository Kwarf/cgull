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

INCBIN(city_fs, "../src/scenes/City.min.fs");

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
          locFadeToBlack(shaderWrapper.getShader().GetLocation("fadeToBlack")),
          locCameraPosition(shaderWrapper.getShader().GetLocation("cameraPosition")),
          locCameraRotation(shaderWrapper.getShader().GetLocation("cameraRotation")),
          locSeagullPosition(shaderWrapper.getShader().GetLocation("seagullPosition")),
          fadeToBlack(rocket.track("ftb")), cameraPositionX(rocket.track("camera_position:x")),
          cameraPositionY(rocket.track("camera_position:y")), cameraPositionZ(rocket.track("camera_position:z")),
          cameraRotationX(rocket.track("camera_rotation:x")), cameraRotationY(rocket.track("camera_rotation:y")),
          cameraRotationZ(rocket.track("camera_rotation:z")), seagullX(rocket.track("seagull:x")),
          seagullY(rocket.track("seagull:y")), seagullZ(rocket.track("seagull:z")){};

    void render() override {
        auto &shader = shaderWrapper.getShader();

        const float resolution[] = {RENDER_WIDTH, RENDER_HEIGHT};
        shader.SetValue(locResolution, &resolution, SHADER_UNIFORM_VEC2);
        const float time = timeSource.now();
        shader.SetValue(locTime, &time, SHADER_UNIFORM_FLOAT);

        const float fadeToBlackValue = fadeToBlack.value();
        shader.SetValue(locFadeToBlack, &fadeToBlackValue, SHADER_UNIFORM_FLOAT);
        float vec3[3];
        vec3[0] = cameraPositionX.value();
        vec3[1] = cameraPositionY.value();
        vec3[2] = cameraPositionZ.value();
        shader.SetValue(locCameraPosition, &vec3, SHADER_UNIFORM_VEC3);
        vec3[0] = cameraRotationX.value();
        vec3[1] = cameraRotationY.value();
        vec3[2] = cameraRotationZ.value();
        shader.SetValue(locCameraRotation, &vec3, SHADER_UNIFORM_VEC3);
        vec3[0] = seagullX.value();
        vec3[1] = seagullY.value();
        vec3[2] = seagullZ.value();
        shader.SetValue(locSeagullPosition, &vec3, SHADER_UNIFORM_VEC3);

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
    int locFadeToBlack;
    int locCameraPosition;
    int locCameraRotation;
    int locSeagullPosition;

    Rocket::Track fadeToBlack;
    Rocket::Track cameraPositionX;
    Rocket::Track cameraPositionY;
    Rocket::Track cameraPositionZ;
    Rocket::Track cameraRotationX;
    Rocket::Track cameraRotationY;
    Rocket::Track cameraRotationZ;
    Rocket::Track seagullX;
    Rocket::Track seagullY;
    Rocket::Track seagullZ;
};
