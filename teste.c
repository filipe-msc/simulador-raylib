#include "raylib.h"
#include "raymath.h"
#include <math.h>

typedef enum { METODO_EULER = 0, METODO_RK4 } MetodoIntegracao;

// Estrutura para representar o estado do sistema (Posição e Velocidade no eixo X)
typedef struct {
    float x;
    float v;
} Estado;

// Equação diferencial do Oscilador Harmônico (Mola): a = -k * x / m
// Para simplificar, assumimos que a constante da mola (k) e a massa (m) valem 1.
// Então: aceleração = -posição
float CalcularAceleracao(float posicao) 
{
    return -posicao; 
}

int main(void)
{
    const int larguraTela = 800;
    const int alturaTela = 450;
    InitWindow(larguraTela, alturaTela, "Mola Ideal: Euler vs RK4 (Divergencia de Energia)");

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 6.0f, 12.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };         
    camera.fovy = 45.0f;                               
    camera.projection = CAMERA_PERSPECTIVE;            

    float anguloHorizontal = 0.0f;
    float anguloVertical = 0.5f;
    float raio = 14.0f;

    // --- CONFIGURAÇÃO DA FÍSICA ---
    float tempoAcumulado = 0.0f;
    const float dt = 0.01f; // Passo fixo da física

    MetodoIntegracao metodoAtual = METODO_EULER;

    // Estado inicial: o cubo começa puxado na posição X = 2.0 e parado (Velocidade = 0)
    Estado estado = { 2.0f, 0.0f }; 

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // Alterna o método e RESETA o cubo para a posição inicial para podermos comparar de forma justa
        if (IsKeyPressed(KEY_SPACE))
        {
            if (metodoAtual == METODO_EULER) metodoAtual = METODO_RK4;
            else metodoAtual = METODO_EULER;
            
            // Reseta o cubo
            estado.x = 2.0f;
            estado.v = 0.0f;
        }

        float frameTime = GetFrameTime();
        if (frameTime > 0.25f) frameTime = 0.25f; 
        tempoAcumulado += frameTime;

        // Loop determinístico da física
        while (tempoAcumulado >= dt)
        {
            if (metodoAtual == METODO_EULER)
            {
                // --- MÉTODO DE EULER EXPLÍCITO ---
                float a = CalcularAceleracao(estado.x);
                estado.x += estado.v * dt;
                estado.v += a * dt;
            }
            else
            {
                // --- MÉTODO DE RUNGE-KUTTA 4ª ORDEM (RK4) ---
                // Passo 1
                float x1 = estado.x;
                float v1 = estado.v;
                float a1 = CalcularAceleracao(x1);

                // Passo 2
                float x2 = estado.x + v1 * dt * 0.5f;
                float v2 = estado.v + a1 * dt * 0.5f;
                float a2 = CalcularAceleracao(x2);

                // Passo 3
                float x3 = estado.x + v2 * dt * 0.5f;
                float v3 = estado.v + a2 * dt * 0.5f;
                float a3 = CalcularAceleracao(x3);

                // Passo 4
                float x4 = estado.x + v3 * dt;
                float v4 = estado.v + a3 * dt;
                float a4 = CalcularAceleracao(x4);

                // Combinação ponderada dos 4 passos
                estado.x += (dt / 6.0f) * (v1 + 2.0f * v2 + 2.0f * v3 + v4);
                estado.v += (dt / 6.0f) * (a1 + 2.0f * a2 + 2.0f * a3 + a4);
            }
            
            tempoAcumulado -= dt;
        }

        // Cálculo da Energia Cinética (E = 0.5 * m * v^2). Como m=1, fica 0.5 * v^2
        float energiaCinetica = 0.5f * estado.v * estado.v;

        // Controles de câmera
        raio -= GetMouseWheelMove() * 1.0f;
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

        // Desenho
        BeginDrawing();
            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
                // Desenha a mola/linha guia do centro até o cubo
                DrawLine3D((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ estado.x, 0.0f, 0.0f }, GRAY);
                
                // O cubo agora se move de verdade no eixo X baseado no resultado da física!
                DrawCube((Vector3){ estado.x, 0.0f, 0.0f }, 1.5f, 1.5f, 1.5f, RED);
                DrawCubeWires((Vector3){ estado.x, 0.0f, 0.0f }, 1.5f, 1.5f, 1.5f, BLACK);
                
                DrawGrid(10, 1.0f);
            EndMode3D();

            // Interface Gráfica (Telemetria)
            DrawRectangle(10, 10, 420, 140, Fade(SKYBLUE, 0.3f));
            DrawRectangleLines(10, 10, 420, 140, BLUE);

            DrawText(TextFormat("FPS Atual: %i", GetFPS()), 20, 20, 20, DARKGRAY);

            if (metodoAtual == METODO_EULER) {
                DrawText("Metodo: EULER (Ganha energia!)", 20, 50, 20, RED);
            } else {
                DrawText("Metodo: RK4 (Estavel / Conservativo)", 20, 50, 20, LIME);
            }

            DrawText(TextFormat("Energia Cinetica: %.4f J", energiaCinetica), 20, 80, 20, MAROON);
            DrawText("Pressione [ESPACO] para alternar e resetar", 20, 120, 14, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
