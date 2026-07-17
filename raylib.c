#include "raylib.h"

int main(void)
{
    // Inicializa uma janela de 800x450 pixels
    InitWindow(800, 450, "Raylib Instalada com Sucesso!");

    SetTargetFPS(60);

    while (!WindowShouldClose()) // Loop do jogo
    {
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("Parabens! Raylib funcionando!", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
