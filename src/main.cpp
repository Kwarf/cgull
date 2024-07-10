#include <iostream>
#include <memory>

#include <raylib-cpp.hpp>

#include "Rocket.h"
#include "TimeSource.h"
#include "scenes/City.h"

INCBIN(music_ogg, "../assets/music.ogg");
INCBIN(syncdata, "../build/sync.bin");

int main() {
#ifdef NDEBUG
    raylib::Window window(1920, 1080, "Cgull", FLAG_FULLSCREEN_MODE | FLAG_VSYNC_HINT);
    raylib::AudioDevice audioDevice;
    raylib::Music music(".ogg", (unsigned char*)gmusic_oggData, gmusic_oggSize);
    HideCursor();
#else
    raylib::Window window(960, 540, "Cgull", FLAG_VSYNC_HINT);
    raylib::AudioDevice audioDevice;
    raylib::Music music("../../assets/music.ogg");
#endif

    TimeSource timeSource;
    Rocket rocket(timeSource, music);
#ifdef NDEBUG
    auto tracks = Rocket::Track::fromData(gsyncdataData, gsyncdataSize);
    rocket.device()->num_tracks = tracks.size();
	rocket.device()->tracks = (sync_track**)malloc(sizeof(sync_track*) * tracks.size());
	for (int i = 0; i < rocket.device()->num_tracks; ++i)
	{
		rocket.device()->tracks[i] = &tracks[i];
	}
#endif
    const auto rocketScene = rocket.track("scene");

#ifdef __APPLE__
    const int targetFrameRate = GetMonitorRefreshRate(GetCurrentMonitor());
#endif

    std::unique_ptr<Scene> scenes[] = {
        std::make_unique<City>(timeSource, rocket),
    };

    float windowWidth = GetScreenWidth();
    float windowHeight = GetScreenHeight();
    raylib::RenderTexture renderTexture(RENDER_WIDTH, RENDER_HEIGHT);

    music.Play();
    while (!window.ShouldClose()) {
        if (IsWindowResized()) {
            windowWidth = GetScreenWidth();
            windowHeight = GetScreenHeight();
        }
        timeSource.tick();
        rocket.update();
        music.Update();

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
        DrawTexturePro(
            renderTexture.texture,
            {0, 0, static_cast<float>(renderTexture.texture.width), static_cast<float>(-renderTexture.texture.height)},
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
