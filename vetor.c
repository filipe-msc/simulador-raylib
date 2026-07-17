#include "raylib.h"
#include "raymath.h" // Incluído para funções matemáticas de vetores

int main(void)
{
    const int larguraTela = 800;
    const int alturaTela = 450;
    InitWindow(larguraTela, alturaTela, "Camera Orbitavel Manual");

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 5.0f, 10.0f }; // Posição inicial
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };     // Foco no cubo
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };         
    camera.fovy = 45.0f;                               
    camera.projection = CAMERA_PERSPECTIVE;            

    // Variáveis para controlar a rotação (em radianos)
    float anguloHorizontal = 0.0f;
    float anguloVertical = 0.5f;
    float raio = 12.0f; // Distância até o cubo (Zoom)

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // 1. Controle do Zoom (Scroll do Mouse)
        raio -= GetMouseWheelMove() * 1.0f;
        if (raio < 2.0f) raio = 2.0f;     // Limite máximo de aproximação
        if (raio > 30.0f) raio = 30.0f;   // Limite máximo de afastamento

        // 2. Controle de Rotação (Clique esquerdo + Arrastar)
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            Vector2 deltaMouse = GetMouseDelta(); // Pega o quanto o mouse se moveu neste frame
            
            // Sensibilidade do movimento
            anguloHorizontal -= deltaMouse.x * 0.005f;
            anguloVertical += deltaMouse.y * 0.005f;

            // Limita o ângulo vertical para a câmera não virar de ponta-cabeça
            if (anguloVertical > 1.4f) anguloVertical = 1.4f;
            if (anguloVertical < 0.1f) anguloVertical = 0.1f;
        }

        // 3. Atualiza a posição da câmera usando coordenadas esféricas
        camera.position.x = camera.target.x + raio * cosf(anguloVertical) * sinf(anguloHorizontal);
        camera.position.y = camera.target.y + raio * sinf(anguloVertical);
        camera.position.z = camera.target.z + raio * cosf(anguloVertical) * cosf(anguloHorizontal);

        // Desenho
        BeginDrawing();
            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
                DrawCube((Vector3){ 0.0f, 0.0f, 0.0f }, 2.0f, 2.0f, 2.0f, RED);
                DrawCubeWires((Vector3){ 0.0f, 0.0f, 0.0f }, 2.0f, 2.0f, 2.0f, BLACK);
                DrawGrid(10, 1.0f);
            EndMode3D();

            DrawText("Clique com o botao esquerdo e arraste para orbitar!", 10, 10, 20, DARKGRAY);
            DrawText("Use a roda do mouse para Zoom.", 10, 35, 16, GRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
