#include <raylib-cpp.hpp>

int main() {
    raylib::Window window(960, 540, "Cgull");
    while (!window.ShouldClose()) {
        BeginDrawing();
        ClearBackground(WHITE);
        EndDrawing();
    }
}
