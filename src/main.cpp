#include <iostream>

#include <raylib-cpp.hpp>

#include "Rocket.h"
#include "TimeSource.h"
#include "scenes/Ridges.h"

int main() {
  raylib::Window window(960, 540, "Cgull");

  TimeSource timeSource;
  Rocket rocket(timeSource);
  const auto rocketScene = rocket.track("scene");

#ifdef __APPLE__
  const int targetFrameRate = GetMonitorRefreshRate(GetCurrentMonitor());
#endif

  std::unique_ptr<Scene> scenes[] = {
      std::make_unique<Ridges>(timeSource),
  };

  float windowWidth = GetScreenWidth();
  float windowHeight = GetScreenHeight();
  raylib::RenderTexture renderTexture(RENDER_WIDTH, RENDER_HEIGHT);

  while (!window.ShouldClose()) {
    if (IsWindowResized()) {
      windowWidth = GetScreenWidth();
      windowHeight = GetScreenHeight();
    }
    timeSource.tick();
    rocket.update();

    const int sceneIdx = rocketScene.value();
    if (sceneIdx < 0 || sceneIdx >= sizeof(scenes) / sizeof(scenes[0])) {
      window.Close();
      continue;
    }

    renderTexture.BeginMode();
    const auto &currentScene = scenes[sceneIdx];
    currentScene->render();
    renderTexture.EndMode();

    BeginDrawing();
    // NOTE: Render texture must be y-flipped due to default OpenGL coordinates
    // (left-bottom)
    DrawTexturePro(renderTexture.texture,
                   {0, 0, static_cast<float>(renderTexture.texture.width),
                    static_cast<float>(-renderTexture.texture.height)},
                   {0, 0, windowWidth, windowHeight}, {0, 0}, 0, WHITE);
    EndDrawing();

    SwapScreenBuffer();
    PollInputEvents();

#ifdef __APPLE__
    // VSync is broken in glfw on macOS,
    // https://github.com/glfw/glfw/issues/2249
    const double elapsed = glfwGetTime() - timeSource.now();
    const double target = 1.0 / targetFrameRate;
    WaitTime(std::min(target - elapsed, 0.02));
#endif
  }
}
