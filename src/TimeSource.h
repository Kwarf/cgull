#pragma once

#include <GLFW/glfw3.h>

// "Tickable" time source, to be refreshed once per frame, with subsequent reads
// getting the same value.
class TimeSource {
public:
  TimeSource() : paused(false), lastTime(0) { glfwSetTime(0); }

  void tick() {
    if (paused) {
      glfwSetTime(lastTime);
    } else {
      lastTime = glfwGetTime();
    }
  }

  double now() const { return lastTime; }

#ifndef NDEBUG
  bool isPaused() const { return paused; }
  void setPaused(bool paused) { this->paused = paused; }
  void setTime(double time) { lastTime = time; }
#endif

private:
  bool paused;
  double lastTime;
};
