#include "raylib.h"
#include "raymath.h"
#include <math.h>

typedef enum { METODO_EULER = 0, METODO_RK4 } MetodoIntegracao;

// Agora o estado precisa de Posição e Velocidade em 2D (Eixos X e Z para a câmera 3D)
typedef struct {
    Vector2 pos; // pos.x e pos.y (mapeado para Z no 3D)
    Vector2 vel; 
} Estado2D;

// Configurações do corpo massivo central fixo no ponto (0,0)
const float G = 1.0f;           // Constante gravitacional simplificada
const float massaCentro = 30.0f; // Massa do corpo central

// Calcula a aceleração gravitacional com base na posição atual
Vector2 CalcularAceleracaoGravitacional(Vector2 posicao)
{
    // Vetor do cubo até o centro (0,0)
    Vector2 direcao = { 0.0f - posicao.x, 0.0f - posicao.y };
    float distancia = sqrtf(direcao.x * direcao.x + direcao.y * direcao.y);
    
    // Evita divisão por zero se o cubo colidir perfeitamente com o centro
    if (distancia < 0.2f) distancia = 0.2f;
    
    // Vetor unitário (normalizado) apontando para o centro
    Vector2 dirNormalizada = { direcao.x / distancia, direcao.y / distancia };
    
    // Lei da gravitação: a = G * M / r^2
    float aceleracaoMag = (G * massaCentro) / (distancia * distancia);
    
    Vector2 aceleracao;
    aceleracao.x = dirNormalizada.x * aceleracaoMag;
    aceleracao.y = dirNormalizada.y * aceleracaoMag;
    
    return aceleracao;
}

int main(void)
{
    const int larguraTela = 800;
    const int alturaTela = 450;
    InitWindow(larguraTela, alturaTela, "Teste Acido: Orbita Gravitacional (Euler vs RK4)");

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 12.0f, 15.0f }; // Câmera um pouco mais alta para ver de cima
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };         
    camera.fovy = 45.0f;                                
    camera.projection = CAMERA_PERSPECTIVE;            

    float anguloHorizontal = 0.0f;
    float anguloVertical = 0.8f;
    float raio = 18.0f;

    // --- CONFIGURAÇÃO DA FÍSICA ---
    float tempoAcumulado = 0.0f;
    const float dt = 0.01f; 

    MetodoIntegracao metodoAtual = METODO_EULER;

    // Estado inicial: posicionado à direita (X=4.0) com velocidade puramente tangencial para cima/frente (Y=2.0)
    Estado2D estado = { { 4.0f, 0.0f }, { 0.0f, 3.0f } }; 

    // --- SISTEMA DE RASTRO (TRAIL) ---
    #define MAX_RASTRO 1000
    Vector2 rastro[MAX_RASTRO];
    int totalPontosRastro = 0;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // Alterna o método e RESETA a órbita para comparar os comportamentos
        if (IsKeyPressed(KEY_SPACE))
        {
            if (metodoAtual == METODO_EULER) metodoAtual = METODO_RK4;
            else metodoAtual = METODO_EULER;
            
            // Reseta para as condições iniciais perfeitas
            estado.pos = (Vector2){ 4.0f, 0.0f };
            estado.vel = (Vector2){ 0.0f, 3.0f };
            totalPontosRastro = 0;
        }

        float frameTime = GetFrameTime();
        if (frameTime > 0.25f) frameTime = 0.25f; 
        tempoAcumulado += frameTime;

        // Loop determinístico da física
        while (tempoAcumulado >= dt)
        {
            if (metodoAtual == METODO_EULER)
            {
                // --- EULER EXPLÍCITO em 2D ---
                Vector2 a = CalcularAceleracaoGravitacional(estado.pos);
                
                estado.pos.x += estado.vel.x * dt;
                estado.pos.y += estado.vel.y * dt;
                
                estado.vel.x += a.x * dt;
                estado.vel.y += a.y * dt;
            }
            else
            {
                // --- RUNGE-KUTTA 4ª ORDEM (RK4) em 2D ---
                // Passo 1
                Vector2 x1 = estado.pos;
                Vector2 v1 = estado.vel;
                Vector2 a1 = CalcularAceleracaoGravitacional(x1);

                // Passo 2
                Vector2 x2 = { estado.pos.x + v1.x * dt * 0.5f, estado.pos.y + v1.y * dt * 0.5f };
                Vector2 v2 = { estado.vel.x + a1.x * dt * 0.5f, estado.vel.y + a1.y * dt * 0.5f };
                Vector2 a2 = CalcularAceleracaoGravitacional(x2);

                // Passo 3
                Vector2 x3 = { estado.pos.x + v2.x * dt * 0.5f, estado.pos.y + v2.y * dt * 0.5f };
                Vector2 v3 = { estado.vel.x + a2.x * dt * 0.5f, estado.vel.y + a2.y * dt * 0.5f };
                Vector2 a3 = CalcularAceleracaoGravitacional(x3);

                // Passo 4
                Vector2 x4 = { estado.pos.x + v3.x * dt, estado.pos.y + v3.y * dt };
                Vector2 v4 = { estado.vel.x + a3.x * dt, estado.vel.y + a3.y * dt };
                Vector2 a4 = CalcularAceleracaoGravitacional(x4);

                // Atualização final ponderada
                estado.pos.x += (dt / 6.0f) * (v1.x + 2.0f * v2.x + 2.0f * v3.x + v4.x);
                estado.pos.y += (dt / 6.0f) * (v1.y + 2.0f * v2.y + 2.0f * v3.y + v4.y);
                
                estado.vel.x += (dt / 6.0f) * (a1.x + 2.0f * a2.x + 2.0f * a3.x + a4.x);
                estado.vel.y += (dt / 6.0f) * (a1.y + 2.0f * a2.y + 2.0f * a3.y + a4.y);
            }
            
            // Alimenta o sistema de rastro
            if (totalPontosRastro < MAX_RASTRO)
            {
                rastro[totalPontosRastro] = estado.pos;
                totalPontosRastro++;
            }
            else
            {
                for (int i = 0; i < MAX_RASTRO - 1; i++) rastro[i] = rastro[i + 1];
                rastro[MAX_RASTRO - 1] = estado.pos;
            }

            tempoAcumulado -= dt;
        }

        // Energia cinética em 2D: E = 0.5 * (vx^2 + vy^2)
        float energiaCinetica = 0.5f * (estado.vel.x * estado.vel.x + estado.vel.y * estado.vel.y);

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
                
                // 1. Desenha o corpo massivo fixo no centro da órbita (0, 0, 0)
                DrawSphere((Vector3){ 0.0f, 0.0f, 0.0f }, 0.8f, DARKGRAY);
                
                // 2. Desenha a linha de rastro (Trail) no plano XZ
                for (int i = 0; i < totalPontosRastro - 1; i++)
                {
                    Vector3 p1 = { rastro[i].x, 0.0f, rastro[i].y };
                    Vector3 p2 = { rastro[i + 1].x, 0.0f, rastro[i + 1].y };
                    DrawLine3D(p1, p2, metodoAtual == METODO_EULER ? RED : LIME);
                }

                // 3. Desenha o Cubo orbitando (convertendo a pos.y da física para a coordenada Z do 3D)
                Vector3 posicaoCubo3D = { estado.pos.x, 0.0f, estado.pos.y };
                DrawCube(posicaoCubo3D, 1.0f, 1.0f, 1.0f, BLUE);
                DrawCubeWires(posicaoCubo3D, 1.0f, 1.0f, 1.0f, BLACK);
                
                DrawGrid(20, 1.0f);
            EndMode3D();

            // Interface Gráfica
            DrawRectangle(10, 10, 440, 140, Fade(SKYBLUE, 0.3f));
            DrawRectangleLines(10, 10, 440, 140, BLUE);

            DrawText(TextFormat("FPS Atual: %i", GetFPS()), 20, 20, 20, DARKGRAY);

            if (metodoAtual == METODO_EULER) {
                DrawText("Metodo: EULER (Espirala para fora!)", 20, 50, 20, RED);
            } else {
                DrawText("Metodo: RK4 (Orbita Elitica Estavel)", 20, 50, 20, LIME);
            }

            DrawText(TextFormat("Velocidade Escalar: %.4f", sqrtf(estado.vel.x * estado.vel.x + estado.vel.y * estado.vel.y)), 20, 80, 20, MAROON);
            DrawText("Pressione [ESPACO] para alternar o metodo", 20, 120, 14, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
