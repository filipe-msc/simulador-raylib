#include "raylib.h"
#include "raymath.h"
#include <math.h>

typedef enum { METODO_EULER = 0, METODO_RK4 } MetodoIntegracao;

// Estado em 3D simplificado para a queda vertical
typedef struct {
    Vector3 pos;
    Vector3 vel;
} Estado3D;

// Configurações do ambiente físico
const float massaCubo = 1.0f;
const Vector3 gravidade = { 0.0f, -9.81f, 0.0f }; // Aceleração da gravidade
const float kArrasto = 0.15f;                    // Coeficiente de resistência do ar (proporcional a v^2)

// Calcula a aceleração resultante da queda (Gravidade + Arrasto do Ar)
Vector3 CalcularAceleracaoQueda(Vector3 velocidade)
{
    // 1. Força da Gravidade: Fg = m * g
    Vector3 F_gravidade = Vector3Scale(gravidade, massaCubo);

    // 2. Força de Arrasto: Fa = -k * v * |v|
    float moduloVel = Vector3Length(velocidade);
    Vector3 F_arrasto = { 0.0f, 0.0f, 0.0f };
    
    if (moduloVel > 0.001f)
    {
        // Direção oposta ao movimento
        Vector3 dirOposta = Vector3Scale(velocidade, -1.0f / moduloVel);
        // Magnitude proporcional ao quadrado da velocidade
        float intensidadeArrasto = kArrasto * moduloVel * moduloVel;
        F_arrasto = Vector3Scale(dirOposta, intensidadeArrasto);
    }

    // Força Resultante
    Vector3 forcaTotal = Vector3Add(F_gravidade, F_arrasto);

    // Segunda Lei de Newton: a = F / m
    return Vector3Scale(forcaTotal, 1.0f / massaCubo);
}

int main(void)
{
    const int larguraTela = 800;
    const int alturaTela = 450;
    InitWindow(larguraTela, alturaTela, "Queda Livre com Resistencia do Ar");

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 15.0f, 15.0f, 20.0f };
    camera.target = (Vector3){ 0.0f, 12.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };         
    camera.fovy = 45.0f;                                
    camera.projection = CAMERA_PERSPECTIVE;            

    float anguloHorizontal = 0.6f;
    float anguloVertical = 0.4f;
    float raio = 25.0f;

    // --- CONFIGURAÇÃO DA FÍSICA ---
    float tempoAcumulado = 0.0f;
    const float dt = 0.01f; // Passo padrão de física

    MetodoIntegracao metodoAtual = METODO_EULER;

    // Estado inicial: Solto de uma altura elevada (Y = 30) e completamente parado
    Vector3 posInicial = { 0.0f, 30.0f, 0.0f };
    Vector3 velInicial = { 0.0f, 0.0f, 0.0f };
    Estado3D estado = { posInicial, velInicial };

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // Alterna o método e coloca o cubo de volta lá no topo
        if (IsKeyPressed(KEY_SPACE))
        {
            if (metodoAtual == METODO_EULER) metodoAtual = METODO_RK4;
            else metodoAtual = METODO_EULER;
            
            estado.pos = posInicial;
            estado.vel = velInicial;
        }

        float frameTime = GetFrameTime();
        if (frameTime > 0.25f) frameTime = 0.25f; 
        tempoAcumulado += frameTime;

        // Loop determinístico da física
        while (tempoAcumulado >= dt)
        {
            if (metodoAtual == METODO_EULER)
            {
                // --- EULER EXPLÍCITO ---
                Vector3 a = CalcularAceleracaoQueda(estado.vel);
                
                estado.pos = Vector3Add(estado.pos, Vector3Scale(estado.vel, dt));
                estado.vel = Vector3Add(estado.vel, Vector3Scale(a, dt));
            }
            else
            {
                // --- RUNGE-KUTTA 4ª ORDEM (RK4) ---
                // Passo 1
                Vector3 v1 = estado.vel;
                Vector3 a1 = CalcularAceleracaoQueda(v1);

                // Passo 2
                Vector3 v2 = Vector3Add(estado.vel, Vector3Scale(a1, dt * 0.5f));
                Vector3 a2 = CalcularAceleracaoQueda(v2);

                // Passo 3
                Vector3 v3 = Vector3Add(estado.vel, Vector3Scale(a2, dt * 0.5f));
                Vector3 a3 = CalcularAceleracaoQueda(v3);

                // Passo 4
                Vector3 v4 = Vector3Add(estado.vel, Vector3Scale(a3, dt));
                Vector3 a4 = CalcularAceleracaoQueda(v4);

                // Atualização final baseada nas inclinações ponderadas
                Vector3 dx = Vector3Scale(Vector3Add(Vector3Add(v1, Vector3Scale(v2, 2.0f)), Vector3Add(Vector3Scale(v3, 2.0f), v4)), dt / 6.0f);
                Vector3 dv = Vector3Scale(Vector3Add(Vector3Add(a1, Vector3Scale(a2, 2.0f)), Vector3Add(Vector3Scale(a3, 2.0f), a4)), dt / 6.0f);
                
                estado.pos = Vector3Add(estado.pos, dx);
                estado.vel = Vector3Add(estado.vel, dv);
            }
            
            // Se o cubo passar muito do chão virtual, faz ele reaparecer no topo automaticamente para o teste continuar rodando
            if (estado.pos.y < -5.0f)
            {
                estado.pos = posInicial;
                estado.vel = velInicial;
            }

            tempoAcumulado -= dt;
        }

        // Métricas de telemetria
        float velocidadeEscalar = Vector3Length(estado.vel);
        float forcaArrastoAtual = kArrasto * velocidadeEscalar * velocidadeEscalar;
        
        // Velocidade terminal teórica para conferirmos: vt = sqrt( (m * g) / k )
        float vTerminalTeorica = sqrtf((massaCubo * 9.81f) / kArrasto);

        // Controles da câmera orbital
        raio -= GetMouseWheelMove() * 1.0f;
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            Vector2 deltaMouse = GetMouseDelta();
            anguloHorizontal -= deltaMouse.x * 0.005f;
            anguloVertical += deltaMouse.y * 0.005f;
            if (anguloVertical > 1.4f) anguloVertical = 1.4f;
            if (anguloVertical < -1.4f) anguloVertical = -1.4f;
        }
        camera.position.x = camera.target.x + raio * cosf(anguloVertical) * sinf(anguloHorizontal);
        camera.position.y = camera.target.y + raio * sinf(anguloVertical);
        camera.position.z = camera.target.z + raio * cosf(anguloVertical) * cosf(anguloHorizontal);

        // Desenho
        BeginDrawing();
            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
                // Linha guia que mostra a trajetória de queda
                DrawLine3D((Vector3){ 0.0f, 30.0f, 0.0f }, (Vector3){ 0.0f, -5.0f, 0.0f }, LIGHTGRAY);

                // Desenha o Cubo em queda
                DrawCube(estado.pos, 1.2f, 1.2f, 1.2f, BLUE);
                DrawCubeWires(estado.pos, 1.2f, 1.2f, 1.2f, BLACK);
                
                DrawGrid(20, 1.0f);
            EndMode3D();

            // Interface Gráfica / HUD de Telemetria
            DrawRectangle(10, 10, 460, 160, Fade(SKYBLUE, 0.2f));
            DrawRectangleLines(10, 10, 460, 160, BLUE);

            DrawText(TextFormat("FPS Atual: %i", GetFPS()), 20, 20, 20, DARKGRAY);

            if (metodoAtual == METODO_EULER) {
                DrawText("Metodo: EULER EXPLÍCITO", 20, 50, 20, RED);
            } else {
                DrawText("Metodo: RK4 (Alta Estabilidade)", 20, 50, 20, LIME);
            }

            DrawText(TextFormat("Velocidade Atual:  %.4f m/s", velocidadeEscalar), 20, 80, 18, MAROON);
            DrawText(TextFormat("Força de Arrasto:  %.4f N", forcaArrastoAtual), 20, 105, 18, DARKGREEN);
            DrawText(TextFormat("V. Terminal Alvo:  ~%.2f m/s", vTerminalTeorica), 20, 130, 18, BLACK);

            DrawText("Pressione [ESPACO] para alternar", 480, 20, 14, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
