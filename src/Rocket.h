#pragma once

#include <string>

#include <raylib.h>
#include <sync.h>

#include "Constants.h"
#include "TimeSource.h"

class Rocket {
  class Track {
  public:
    Track(const sync_track *track) : track(track) {}

  private:
    const sync_track *track;
  };

public:
  Rocket(TimeSource &timeSource)
      : timeSource(timeSource),
        callbacks{reinterpret_cast<void (*)(void *, int)>(setPaused),
                  reinterpret_cast<void (*)(void *, int)>(setRow),
                  reinterpret_cast<int (*)(void *)>(isPlaying)} {
    rocket = sync_create_device("Rocket");
#ifndef NDEBUG
    if (sync_tcp_connect(rocket, "localhost", SYNC_DEFAULT_PORT) < 0) {
      TraceLog(LOG_WARNING, "Failed to connect to Rocket client");
    }
#endif
  }
  ~Rocket() { sync_destroy_device(rocket); }

  void update() {
    sync_update(rocket, timeSource.now() * SYNC_ROW_RATE, &callbacks, this);
  }

  Track track(const std::string &name) {
    return Track(sync_get_track(rocket, name.c_str()));
  }

private:
  static void setPaused(Rocket *rocket, int paused) {
    rocket->timeSource.setPaused(paused);
  }

  static void setRow(Rocket *rocket, int row) {
    rocket->timeSource.setTime(row / SYNC_ROW_RATE);
  }

  static int isPlaying(Rocket *rocket) {
    return !rocket->timeSource.isPaused();
  }

  TimeSource &timeSource;
  sync_device *rocket;
  sync_cb callbacks;
};
