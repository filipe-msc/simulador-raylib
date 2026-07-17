#include "raylib.h"
#include "raymath.h"

int main(void)
{
    const int larguraTela = 800;
    const int alturaTela = 450;
    InitWindow(larguraTela, alturaTela, "Simulacao: Passos Fixos (Deterministico)");

    // Configuração da Câmera
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 5.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };         
    camera.fovy = 45.0f;                               
    camera.projection = CAMERA_PERSPECTIVE;            

    float anguloHorizontal = 0.0f;
    float anguloVertical = 0.5f;
    float raio = 12.0f;

    // --- VARIÁVEIS DO ACUMULADOR DE TEMPO ---
    float tempoAcumulado = 0.0f;
    const float dt = 0.01f; // Passo fixo da física (100 Hz). Mude para 0.001f se quiser mais precisão.
    float tempoSimulacao = 0.0f; // Tempo total simulado da física

    SetTargetFPS(144); // Forçando 144 FPS para testar o desacoplamento

    while (!WindowShouldClose())
    {
        // 1. Captura o tempo real decorrido desde o último frame de renderização
        float frameTime = GetFrameTime();
        
        // Evita o "espirro da morte" se o jogo travar por um segundo
        if (frameTime > 0.25f) frameTime = 0.25f; 
        
        tempoAcumulado += frameTime;

        // 2. LOOP DA FÍSICA (Passos Fixos e Determinísticos)
        // Enquanto houver tempo acumulado suficiente, executamos um passo de física de tamanho fixo 'dt'
        while (tempoAcumulado >= dt)
        {
            // --------------------------------------------------------
            // TODO: Inserir os cálculos de física aqui!
            // Aqui você aplicará Euler ou RK4 usando o 'dt' fixo.
            // Exemplo: posicao += velocidade * dt;
            // --------------------------------------------------------
            
            tempoSimulacao += dt;
            tempoAcumulado -= dt; // Consome o tempo do acumulador
        }

        // 3. Controles da Câmera (Mouse)
        raio -= GetMouseWheelMove() * 1.0f;
        if (raio < 2.0f) raio = 2.0f;
        if (raio > 30.0f) raio = 30.0f;

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            Vector2 deltaMouse = GetMouseDelta();
            anguloHorizontal -= deltaMouse.x * 0.005f;
            anguloVertical += deltaMouse.y * 0.005f;

            if (anguloVertical > 1.4f) anguloVertical = 1.4f;
            if (anguloVertical < 0.1f) anguloVertical = 0.1f;
        }

        camera.position.x = camera.target.x + raio * cosf(anguloVertical) * sinf(anguloHorizontal);
        camera.position.y = camera.target.y + raio * sinf(anguloVertical);
        camera.position.z = camera.target.z + raio * cosf(anguloVertical) * cosf(anguloHorizontal);

        // 4. Desenho (Renderiza o estado mais recente)
        BeginDrawing();
            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
                DrawCube((Vector3){ 0.0f, 0.0f, 0.0f }, 2.0f, 2.0f, 2.0f, RED);
                DrawCubeWires((Vector3){ 0.0f, 0.0f, 0.0f }, 2.0f, 2.0f, 2.0f, BLACK);
                DrawGrid(10, 1.0f);
            EndMode3D();

            // Telemetria na tela
            DrawText(TextFormat("FPS Real: %i", GetFPS()), 10, 10, 20, LIME);
            DrawText(TextFormat("Tempo Simulado: %.2fs", tempoSimulacao), 10, 35, 18, DARKGRAY);
            DrawText(TextFormat("Passo de Fisica (dt): %.3fs", dt), 10, 60, 16, GRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
