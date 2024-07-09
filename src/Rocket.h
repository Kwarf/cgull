#pragma once

#include <string>

#include <raylib-cpp.hpp>
#include <sync.h>

#include "Constants.h"
#include "TimeSource.h"

class Rocket {
public:
    class Track {
    public:
        Track(const TimeSource &timeSource, const sync_track *track) : timeSource(timeSource), track(track) {}

        double value() const {
            return sync_get_val(track, timeSource.now() * SYNC_ROW_RATE);
        }

    private:
        const TimeSource &timeSource;
        const sync_track *track;
    };

    Rocket(TimeSource &timeSource, raylib::Music &music)
        : timeSource(timeSource), music(music)
#ifndef NDEBUG
          ,
          callbacks{reinterpret_cast<void (*)(void *, int)>(setPaused), reinterpret_cast<void (*)(void *, int)>(setRow),
                    reinterpret_cast<int (*)(void *)>(isPlaying)}
#endif
    {
        rocket = sync_create_device("Rocket");
#ifndef NDEBUG
        if (sync_tcp_connect(rocket, "localhost", SYNC_DEFAULT_PORT) < 0) {
            TraceLog(LOG_WARNING, "Failed to connect to Rocket client");
        }
#endif
    }
    ~Rocket() {
        sync_destroy_device(rocket);
    }

    void update() {
#ifndef NDEBUG
        sync_update(rocket, timeSource.now() * SYNC_ROW_RATE, &callbacks, this);
#endif
    }

    Track track(const std::string &name) const {
        return Track(timeSource, sync_get_track(rocket, name.c_str()));
    }

private:
#ifndef NDEBUG
    static void setPaused(Rocket *rocket, int paused) {
        rocket->timeSource.setPaused(paused);
        if (paused) {
            rocket->music.Pause();
        } else {
            rocket->music.Resume();
        }
    }

    static void setRow(Rocket *rocket, int row) {
        rocket->music.Seek(row / SYNC_ROW_RATE);
        rocket->timeSource.setTime(row / SYNC_ROW_RATE);
    }

    static int isPlaying(Rocket *rocket) {
        return !rocket->timeSource.isPaused();
    }
#endif

    TimeSource &timeSource;
    raylib::Music &music;
    sync_device *rocket;
#ifndef NDEBUG
    sync_cb callbacks;
#endif
};
