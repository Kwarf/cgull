#pragma once

#include <stdint.h>

constexpr uint64_t BPM = 175;
constexpr uint64_t SYNC_ROWS_PER_BEAT = 4;
constexpr double SYNC_ROW_RATE = (static_cast<double>(BPM) / 60) * SYNC_ROWS_PER_BEAT;

#ifdef NDEBUG
constexpr uint64_t RENDER_WIDTH = 1920;
constexpr uint64_t RENDER_HEIGHT = 1080;
#else
constexpr uint64_t RENDER_WIDTH = 640;
constexpr uint64_t RENDER_HEIGHT = 360;
#endif
