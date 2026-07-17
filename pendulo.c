#include "raylib.h"
#include "raymath.h"
#include <math.h>

typedef enum { METODO_EULER = 0, METODO_RK4 } MetodoIntegracao;

// Estado em 3D completo para o pêndulo mover-se em qualquer direção
typedef struct {
    Vector3 pos;
    Vector3 vel;
} Estado3D;

// Configurações do Pêndulo
const Vector3 pontoFixacao = { 0.0f, 5.0f, 0.0f }; // Onde o topo da corda está preso
const float comprimentoCorda = 5.0f;               // Comprimento natural da corda
const float rigidezCorda = 150.0f;                 // Mola muito rígida para agir como corda
const float massaCubo = 1.0f;
const Vector3 gravidade = { 0.0f, -9.81f, 0.0f };  // Gravidade realista para baixo

// Calcula todas as forças (Gravidade + Tração da Corda) e retorna a aceleração resultante
Vector3 CalcularAceleracaoPendulo(Vector3 posicao, Vector3 velocidade)
{
    // 1. Força da Gravidade: F = m * g
    Vector3 F_gravidade = Vector3Scale(gravidade, massaCubo);

    // 2. Força de Tração (Mola rígida): puxa na direção do ponto de fixação
    Vector3 direcaoCorda = Vector3Subtract(pontoFixacao, posicao);
    float distancia = Vector3Length(direcaoCorda);
    
    Vector3 F_tracao = { 0.0f, 0.0f, 0.0f };
    if (distancia > 0.001f)
    {
        Vector3 dirNormalizada = Vector3Scale(direcaoCorda, 1.0f / distancia);
        // Só puxa se a corda esticar além do comprimento nominal
        float deformacao = distancia - comprimentoCorda;
        
        // Lei de Hooke: F = k * x
        F_tracao = Vector3Scale(dirNormalizada, rigidezCorda * deformacao);
        
        // Amortecimento leve apenas para estabilizar a corda elástica (opcional)
        Vector3 V_relativa = velocidade; 
        float amortecimento = 1.5f;
        Vector3 F_amortecimento = Vector3Scale(dirNormalizada, -amortecimento * Vector3DotProduct(V_relativa, dirNormalizada));
        F_tracao = Vector3Add(F_tracao, F_amortecimento);
    }

    // Força Total = Gravidade + Tração
    Vector3 forcaTotal = Vector3Add(F_gravidade, F_tracao);

    // Segunda Lei de Newton: a = F / m
    return Vector3Scale(forcaTotal, 1.0f / massaCubo);
}

int main(void)
{
    const int larguraTela = 800;
    const int alturaTela = 450;
    InitWindow(larguraTela, alturaTela, "Pendulo 3D: Teste de Conservacao de Energia");

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 4.0f, 12.0f };
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };         
    camera.fovy = 45.0f;                                
    camera.projection = CAMERA_PERSPECTIVE;            

    float anguloHorizontal = 0.0f;
    float anguloVertical = 0.3f;
    float raio = 12.0f;

    // --- CONFIGURAÇÃO DA FÍSICA ---
    float tempoAcumulado = 0.0f;
    const float dt = 0.002f; // Passo menor (substepping) porque a mola rígida exige alta precisão

    MetodoIntegracao metodoAtual = METODO_EULER;

    // Estado inicial: Puxado para o lado no eixo X (3.5) e ligeiramente para frente no Z (1.0)
    Vector3 posInicial = { 3.5f, 1.5f, 1.0f };
    Vector3 velInicial = { 0.0f, 0.0f, 0.0f };
    Estado3D estado = { posInicial, velInicial };

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // Altera o método e RESETA para a mesma posição de largada
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
                Vector3 a = CalcularAceleracaoPendulo(estado.pos, estado.vel);
                
                estado.pos = Vector3Add(estado.pos, Vector3Scale(estado.vel, dt));
                estado.vel = Vector3Add(estado.vel, Vector3Scale(a, dt));
            }
            else
            {
                // --- RUNGE-KUTTA 4ª ORDEM (RK4) ---
                Vector3 x1 = estado.pos;
                Vector3 v1 = estado.vel;
                Vector3 a1 = CalcularAceleracaoPendulo(x1, v1);

                Vector3 x2 = Vector3Add(estado.pos, Vector3Scale(v1, dt * 0.5f));
                Vector3 v2 = Vector3Add(estado.vel, Vector3Scale(a1, dt * 0.5f));
                Vector3 a2 = CalcularAceleracaoPendulo(x2, v2);

                Vector3 x3 = Vector3Add(estado.pos, Vector3Scale(v2, dt * 0.5f));
                Vector3 v3 = Vector3Add(estado.vel, Vector3Scale(a2, dt * 0.5f));
                Vector3 a3 = CalcularAceleracaoPendulo(x3, v3);

                Vector3 x4 = Vector3Add(estado.pos, Vector3Scale(v3, dt));
                Vector3 v4 = Vector3Add(estado.vel, Vector3Scale(a3, dt));
                Vector3 a4 = CalcularAceleracaoPendulo(x4, v4);

                // Atualização final baseada na média ponderada das inclinações
                Vector3 dx = Vector3Scale(Vector3Add(Vector3Add(v1, Vector3Scale(v2, 2.0f)), Vector3Add(Vector3Scale(v3, 2.0f), v4)), dt / 6.0f);
                Vector3 dv = Vector3Scale(Vector3Add(Vector3Add(a1, Vector3Scale(a2, 2.0f)), Vector3Add(Vector3Scale(a3, 2.0f), a4)), dt / 6.0f);
                
                estado.pos = Vector3Add(estado.pos, dx);
                estado.vel = Vector3Add(estado.vel, dv);
            }
            
            tempoAcumulado -= dt;
        }

        // --- CÁLCULO DE ENERGIA MECÂNICA NO HUD ---
        // Energia Cinética: Ec = 0.5 * m * v^2
        float vQuadrado = Vector3DotProduct(estado.vel, estado.vel);
        float energiaCinetica = 0.5f * massaCubo * vQuadrado;
        
        // Energia Potencial Gravitacional: Ep = m * g * h (considerando h a altura Y em relação ao chão 0)
        float energiaPotencial = massaCubo * 9.81f * estado.pos.y;
        
        // Energia Mecânica Total = Cinética + Potencial
        float energiaTotal = energiaCinetica + energiaPotencial;

        // Controles de câmera orbital
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
                // Desenha a estrutura de suporte fixa no topo
                DrawSphere(pontoFixacao, 0.2f, BLACK);
                
                // Desenha a corda/linha até o cubo
                DrawLine3D(pontoFixacao, estado.pos, DARKGRAY);
                
                // Desenha o Pêndulo (Cubo)
                DrawCube(estado.pos, 1.0f, 1.0f, 1.0f, BLUE);
                DrawCubeWires(estado.pos, 1.0f, 1.0f, 1.0f, BLACK);
                
                DrawGrid(10, 1.0f);
            EndMode3D();

            // Interface Gráfica / HUD de Telemetria
            DrawRectangle(10, 10, 460, 160, Fade(SKYBLUE, 0.2f));
            DrawRectangleLines(10, 10, 460, 160, BLUE);

            DrawText(TextFormat("FPS Atual: %i", GetFPS()), 20, 20, 20, DARKGRAY);

            if (metodoAtual == METODO_EULER) {
                DrawText("Metodo: EULER (Cria energia!)", 20, 50, 20, RED);
            } else {
                DrawText("Metodo: RK4 (Conserva Energia)", 20, 50, 20, LIME);
            }

            // Exibição detalhada das métricas de energia
            DrawText(TextFormat("Energia Cinetica:  %.2f J", energiaCinetica), 20, 80, 18, MAROON);
            DrawText(TextFormat("Energia Potencial: %.2f J", energiaPotencial), 20, 105, 18, DARKGREEN);
            DrawText(TextFormat("ENERGIA TOTAL:     %.2f J", energiaTotal), 20, 130, 20, BLACK);

            DrawText("Pressione [ESPACO] para alternar o metodo", 480, 20, 14, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
