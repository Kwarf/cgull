#pragma once

#include <sstream>
#include <string>

#include <device.h>
#include <raylib-cpp.hpp>
#include <sync.h>
#include <track.h>

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

#ifdef NDEBUG
        static std::vector<sync_track> fromData(const unsigned char *data, size_t size) {
            std::istringstream stream(std::string(data, data + size), std::ios::binary);

            uint8_t numTracks;
            stream.read(reinterpret_cast<char*>(&numTracks), sizeof(numTracks));
            std::vector<sync_track> tracks(numTracks);

            for (auto &track : tracks) {
                uint8_t trackNameLength;
                stream.read(reinterpret_cast<char*>(&trackNameLength), sizeof(trackNameLength));
                track.name = new char[trackNameLength];
                stream.read(track.name, trackNameLength);

                uint16_t keyCount;
                stream.read(reinterpret_cast<char*>(&keyCount), sizeof(keyCount));
                track.keys = new track_key[keyCount];
                track.num_keys = keyCount;
                stream.read(reinterpret_cast<char*>(track.keys), sizeof(track_key) * keyCount);
            }

            return tracks;
        }
#endif

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

    sync_device *device() {
        return rocket;
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
