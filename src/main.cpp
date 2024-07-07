#include <iostream>

#include <raylib-cpp.hpp>

#include "Rocket.h"
#include "TimeSource.h"

int main() {
  raylib::Window window(960, 540, "Cgull");

  TimeSource timeSource;
  Rocket rocket(timeSource);
  auto track = rocket.track("scene");

#ifdef __APPLE__
    const int targetFrameRate = GetMonitorRefreshRate(GetCurrentMonitor());
#endif

  while (!window.ShouldClose()) {
    timeSource.tick();
    rocket.update();

    BeginDrawing();
    ClearBackground(WHITE);
    EndDrawing();

    SwapScreenBuffer();
    PollInputEvents();

#ifdef __APPLE__
    // VSync is broken in glfw on macOS,
    // https://github.com/glfw/glfw/issues/2249
    const double elapsed = glfwGetTime() - timeSource.now();
    const double target = 1.0 / targetFrameRate;
    WaitTime(target - elapsed);
#endif
  }
}
