#include "raylib.h"

int main(void)
{
    // 1. Inicialização da janela
    const int larguraTela = 800;
    const int alturaTela = 450;
    InitWindow(larguraTela, alturaTela, "Camera 3D Orbitavel com o Mouse");

    // 2. Configuração da Câmera 3D
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 10.0f, 10.0f }; // Posição inicial
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };     // Foco no cubo (centro)
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };         // Eixo Y para cima
    camera.fovy = 45.0f;                               // Campo de visão
    camera.projection = CAMERA_PERSPECTIVE;            // Projeção 3D

    SetTargetFPS(60);

    // Loop principal do jogo
    while (!WindowShouldClose())
    {
        // 3. Atualização da Câmera
        // O modo CAMERA_ORBITAL faz o mouse girar ao redor do target (o cubo)
        UpdateCamera(&camera, CAMERA_ORBITAL);

        // 4. Desenho
        BeginDrawing();
            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                // Desenha o mesmo cubo vermelho no centro
                DrawCube((Vector3){ 0.0f, 0.0f, 0.0f }, 2.0f, 2.0f, 2.0f, RED);
                DrawCubeWires((Vector3){ 0.0f, 0.0f, 0.0f }, 2.0f, 2.0f, 2.0f, BLACK);

                // Grade para referência de movimento
                DrawGrid(10, 1.0f);

            EndMode3D();

            // Instruções na tela
            DrawText("Mova o mouse para girar em volta do cubo!", 10, 10, 20, DARKGRAY);
            DrawText("Use a roda do mouse (Scroll) para dar Zoom.", 10, 35, 16, GRAY);

        EndDrawing();
    }

    // 5. Desinicialização
    CloseWindow();

    return 0;
}
