#include "raylib.h"

int main(void)
{
    // 1. Inicialização da janela
    const int larguraTela = 800;
    const int alturaTela = 450;
    InitWindow(larguraTela, alturaTela, "Primeiro Passo 3D - Cubo Estático");

    // 2. Configuração da Câmera 3D
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 10.0f, 10.0f }; // Posição da câmera no espaço (X, Y, Z)
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };     // Para onde a câmera está olhando (centro)
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };         // Vetor que define "para cima" (eixo Y)
    camera.fovy = 45.0f;                               // Campo de visão (Field of View) em graus
    camera.projection = CAMERA_PERSPECTIVE;            // Projeção em perspectiva (3D real)

    SetTargetFPS(60);

    // Loop principal do jogo
    while (!WindowShouldClose())
    {
        // 3. Desenho
        BeginDrawing();
            ClearBackground(RAYWHITE);

            // Inicia o modo de renderização 3D usando a câmera configurada
            BeginMode3D(camera);

                // Desenha o cubo vermelho no centro (X:0, Y:0, Z:0) com tamanho 2x2x2
                DrawCube((Vector3){ 0.0f, 0.0f, 0.0f }, 2.0f, 2.0f, 2.0f, RED);
                
                // Desenha as bordas do cubo em preto para dar definição física
                DrawCubeWires((Vector3){ 0.0f, 0.0f, 0.0f }, 2.0f, 2.0f, 2.0f, BLACK);

                // Desenha uma grade no chão para ajudar na noção de espaço
                DrawGrid(10, 1.0f);

            EndMode3D(); // Finaliza o modo 3D e volta para o plano 2D da tela

            // Elementos em 2D (interface) são desenhados fora do BeginMode3D
            DrawText("Parabens! Ambiente 3D pronto para rodar.", 10, 10, 20, DARKGRAY);

        EndDrawing();
    }

    // 4. Desinicialização
    CloseWindow();

    return 0;
}
