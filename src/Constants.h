#include <stdint.h>

constexpr uint64_t BPM = 125;
constexpr uint64_t SYNC_ROWS_PER_BEAT = 8;
constexpr double SYNC_ROW_RATE = (static_cast<double>(BPM) / 60) * SYNC_ROWS_PER_BEAT;
