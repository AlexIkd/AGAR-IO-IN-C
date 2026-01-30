/*  
    Jogo Geométrico 3D — OpenGL + (free)GLUT (estilo Agar.io 3D) + CENÁRIO
    Adicionado: menu inicial com opções Iniciar, Controles, Sair
*/

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <algorithm>

#if defined(__APPLE__)
  #include <GLUT/glut.h>
#else
  #if defined(FREEGLUT)
    #include <GL/freeglut.h>
  #else
    #include <GL/glut.h>
  #endif
#endif

//============================ Utilidades ============================//
struct Vec3 { float x=0, y=0, z=0; };
static float frand(float a, float b) { return a + (b - a) * (std::rand() / (float)RAND_MAX); }
static float clampf(float x, float a, float b) { return std::max(a, std::min(b, x)); }
static float dist2(const Vec3& a, const Vec3& b) { float dx=a.x-b.x, dy=a.y-b.y, dz=a.z-b.z; return dx*dx + dy*dy + dz*dz; }

//============================= Jogo =================================//
struct Obj {
    Vec3 pos;         // posição
    float r=0.5f;     // raio p/ colisão (define o TAMANHO do cubo vermelho / pellet)
    float rot=0.f;    // rotação visual
    int kind=0;       // 0: pellet (pirâmide) | 1: inimigo (cubo vermelho)
    Vec3 vel;         // para inimigos móveis
};

// Mundo / Cenário
static const float WORLD_HALF = 25.0f;   // limites +- no X e Z (área jogável)
static const float GROUND_Y   = 0.0f;
static const float WALL_THICK = 0.6f;
static const float WALL_H     = 3.0f;
static const float GATE_W     = 8.0f;   // largura do portão no lado +Z (norte)

// Jogador
static Vec3 player{0.f, 0.6f, 0.f};
static float playerR = 0.6f;   // raio colisor base do jogador
static float playerYaw = 0.f;  // orientação visual
static float baseSpeed = 9.0f; // m/s base
static float mass = 1.0f;      // cresce ao comer
static float dashCd = 0.f;     // recarga do dash
static Vec3 playerCurrentVel = {0.f, 0.f, 0.f}; // Velocidade atual do jogador
static float decelerationFactor = 0.95f; // Fator de desaceleração (0.0 a 1.0, mais próximo de 1.0 = mais lento para parar)
static float maxPlayerSpeed = 9.0f; // Velocidade máxima do jogador
static float accelerationFactor = 0.1f; // Fator de aceleração (0.0 a 1.0, controla quão rápido atinge a velocidade máxima)


// Sistema de jogo
static int score = 0;
static int lives = 1;          // **apenas 1 vida**
static int level = 1;
static bool started = false;
static bool paused  = false;
static bool gameOver= false;

// Menu
static bool menuActive = true;
static bool showControlsMenu = false;
static int menuIndex = 0;
static const int MENU_ITEMS = 3;
static const char* menuLabels[MENU_ITEMS] = {"Iniciar", "Controles", "Sair"};

static std::vector<Obj> pellets;    // pirâmides pequenas (comida)
static std::vector<Obj> enemies;    // cubos vermelhos (tamanhos variados)

struct Particle {
    Vec3 pos;
    Vec3 vel;
    float life;
    float r, g, b;
};
static std::vector<Particle> particles;

// Entrada
static bool keys[256] = {false};
static bool skey[256] = {false};
static bool mouseFollow = true; // modo padrão tipo agar.io
static int mouseX = 0, mouseY = 0; // posição do cursor

// Delta time
static int lastTicks = 0; // ms


//======================= Render helpers =============================//
static void setLight()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat pos[]  = { 10.f, 20.f, 10.f, 1.f };
    GLfloat diff[] = { 1.f, 1.f, 1.f, 1.f };
    GLfloat amb[]  = { 0.25f, 0.25f, 0.25f, 1.f };
    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  diff);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  amb);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glShadeModel(GL_SMOOTH);
}

static void drawCube(float s=1.f) { glutSolidCube(s); }

static void drawPyramid(float base=0.7f, float h=0.7f)
{
    float b = base * 0.5f;
    glBegin(GL_TRIANGLES);
      glNormal3f(0.f, h, b);
      glVertex3f( 0.f, h, 0.f);
      glVertex3f(-b, 0.f, b);
      glVertex3f( b, 0.f, b);
      glNormal3f(b, h, 0.f);
      glVertex3f( 0.f, h, 0.f);
      glVertex3f( b, 0.f,  b);
      glVertex3f( b, 0.f, -b);
      glNormal3f(0.f, h, -b);
      glVertex3f( 0.f, h, 0.f);
      glVertex3f( b, 0.f, -b);
      glVertex3f(-b, 0.f, -b);
      glNormal3f(-b, h, 0.f);
      glVertex3f( 0.f, h, 0.f);
      glVertex3f(-b, 0.f, -b);
      glVertex3f(-b, 0.f,  b);
    glEnd();
    glBegin(GL_QUADS);
      glNormal3f(0.f,-1.f,0.f);
      glVertex3f(-b,0.f, b);
      glVertex3f( b,0.f, b);
      glVertex3f( b,0.f,-b);
      glVertex3f(-b,0.f,-b);
    glEnd();
}

//======================= CENÁRIO ====================================//
static void drawCheckerFloor(float half, float tile=1.0f)
{
    glBegin(GL_QUADS);
    for (float x=-half; x<half; x+=tile) {
        for (float z=-half; z<half; z+=tile) {
            bool dark = ( ((int)std::floor((x+half)/tile) + (int)std::floor((z+half)/tile)) % 2 )==0;
            glColor3f(dark?0.10f:0.20f, dark?0.10f:0.20f, dark?0.14f:0.26f);
            glNormal3f(0,1,0);
            glVertex3f(x,   GROUND_Y, z);
            glVertex3f(x+tile, GROUND_Y, z);
            glVertex3f(x+tile, GROUND_Y, z+tile);
            glVertex3f(x,   GROUND_Y, z+tile);
        }
    }
    glEnd();
}

static void drawScaledCubeAt(float cx, float cy, float cz, float sx, float sy, float sz)
{
    glPushMatrix();
    glTranslatef(cx, cy, cz);
    glScalef(sx, sy, sz);
    drawCube(1.0f);
    glPopMatrix();
}

static void drawWallsAndGate()
{
    // Paredes como cubos esticados posicionados levemente fora da área jogável
    float len = 2*WORLD_HALF + WALL_THICK; // comprimento cobrindo toda a borda

    glColor3f(0.30f, 0.32f, 0.42f);
    // Sul (-Z)
    drawScaledCubeAt(0, WALL_H*0.5f, -WORLD_HALF - WALL_THICK*0.5f, len, WALL_H, WALL_THICK);

    // Norte (+Z) com portão no meio
    float gap = GATE_W;
    float halfSpan = len*0.5f;
    float seg = halfSpan - gap*0.5f;
    // segmento esquerdo
    drawScaledCubeAt(-seg*0.5f - gap*0.5f, WALL_H*0.5f, WORLD_HALF + WALL_THICK*0.5f, seg, WALL_H, WALL_THICK);
    // segmento direito
    drawScaledCubeAt( seg*0.5f + gap*0.5f, WALL_H*0.5f, WORLD_HALF + WALL_THICK*0.5f, seg, WALL_H, WALL_THICK);

    // Oeste (-X)
    drawScaledCubeAt(-WORLD_HALF - WALL_THICK*0.5f, WALL_H*0.5f, 0, WALL_THICK, WALL_H, len);
    // Leste (+X)
    drawScaledCubeAt( WORLD_HALF + WALL_THICK*0.5f, WALL_H*0.5f, 0, WALL_THICK, WALL_H, len);

    // Portão: 2 pilares + viga
    glColor3f(0.50f, 0.52f, 0.62f);
    float poleW=1.2f, poleH=WALL_H+1.8f, poleZ= WORLD_HALF + WALL_THICK*0.5f - 0.01f;
    drawScaledCubeAt(-gap*0.5f - poleW*0.5f, poleH*0.5f, poleZ, poleW, poleH, WALL_THICK*1.25f);
    drawScaledCubeAt( gap*0.5f + poleW*0.5f, poleH*0.5f, poleZ, poleW, poleH, WALL_THICK*1.25f);
    // viga no topo
    drawScaledCubeAt(0, poleH + 0.4f, poleZ, gap + 1.0f, 0.6f, WALL_THICK*1.3f);
}

static void drawCornerTowers()
{
    glColor3f(0.36f, 0.38f, 0.50f);
    float tH = 6.0f; float tW = 1.6f;
    float off = WORLD_HALF - 1.6f;
    drawScaledCubeAt(-off, tH*0.5f, -off, tW, tH, tW);
    drawScaledCubeAt( off, tH*0.5f, -off, tW, tH, tW);
    drawScaledCubeAt(-off, tH*0.5f,  off, tW, tH, tW);
    drawScaledCubeAt( off, tH*0.5f,  off, tW, tH, tW);
}


static void drawArena()
{
    drawCheckerFloor(WORLD_HALF, 1.0f);
    drawWallsAndGate();
    drawCornerTowers();}

//======================= Entidades ==================================
static void drawPlayer()
{
    glPushMatrix();
    glTranslatef(player.x, player.y, player.z);
    glRotatef(playerYaw, 0,1,0);
    float scale = std::cbrt(mass);
    glScalef(scale, scale, scale);
    glColor3f(0.2f, 0.75f, 1.0f);
    drawCube(1.0f);
    glPopMatrix();
}

static void drawEnemy(const Obj& o)
{
    glPushMatrix();
    glTranslatef(o.pos.x, o.pos.y, o.pos.z);
    glRotatef(o.rot, 0,1,0);
    float s = 2.0f * o.r; // cubo com aresta ~ 2r
    glScalef(s, s, s);
    glColor3f(1.f, 0.2f, 0.2f);
    drawCube(1.0f);
    glPopMatrix();
}

static void drawPellet(const Obj& o)
{
    glPushMatrix();
    glTranslatef(o.pos.x, o.pos.y, o.pos.z);
    glRotatef(o.rot, 0,1,0);
    glColor3f(1.f, 0.9f, 0.2f);
    drawPyramid(0.6f, 0.6f);
    glPopMatrix();
}

static void drawParticles()
{
    glDisable(GL_LIGHTING);
    glPointSize(4.0f);
    glBegin(GL_POINTS);
    for (const auto& p : particles) {
        glColor4f(p.r, p.g, p.b, p.life);
        glVertex3f(p.pos.x, p.pos.y, p.pos.z);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

// HUD 2D
static void drawBitmapText(const std::string& s, float x, float y)
{ glRasterPos2f(x, y); for (char c : s) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c); }

static void drawHUD(int w, int h)
{
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
    glDisable(GL_LIGHTING);

    // --- Menu inicial ---
    if (menuActive) {
        // caixa semi-transparente
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.f, 0.f, 0.f, 0.65f);
        float boxW = 420.f, boxH = 260.f;
        float cx = w*0.5f, cy = h*0.5f;
        float left = cx - boxW*0.5f, right = cx + boxW*0.5f;
        float top = cy + boxH*0.5f, bottom = cy - boxH*0.5f;
        glBegin(GL_QUADS);
          glVertex2f(left, bottom);
          glVertex2f(right, bottom);
          glVertex2f(right, top);
          glVertex2f(left, top);
        glEnd();
        glDisable(GL_BLEND);

        // Texto do menu
        glDisable(GL_DEPTH_TEST); // Desabilita o teste de profundidade para o texto 2D
        glDisable(GL_LIGHTING); // Desabilita a iluminação para o texto 2D
        glColor3f(1,1,1);
        drawBitmapText("JOGO GEOMETRICO 3D", cx - 120, cy + 90);
        for (int i=0;i<MENU_ITEMS;i++) {
            std::string t = (menuIndex==i ? "> " : "  ");
            t += menuLabels[i];
            drawBitmapText(t, cx - 60, cy + 30 - i*36);
        }

        glMatrixMode(GL_MODELVIEW); glPopMatrix();
        glMatrixMode(GL_PROJECTION); glPopMatrix();
        glEnable(GL_LIGHTING); // Reabilita a iluminação
        glEnable(GL_DEPTH_TEST); // Reabilita o teste de profundidade
        return;
    }

    // --- Tela de CONTROLES (quando ativada) ---
    if (showControlsMenu) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.f, 0.f, 0.f, 0.6f);
        float boxW = 560.f, boxH = 360.f;
        float cx = w*0.5f, cy = h*0.5f;
        float left = cx - boxW*0.5f, right = cx + boxW*0.5f;
        float top = cy + boxH*0.5f, bottom = cy - boxH*0.5f;
        glBegin(GL_QUADS);
          glVertex2f(left, bottom);
          glVertex2f(right, bottom);
          glVertex2f(right, top);
          glVertex2f(left, top);
        glEnd();
        glDisable(GL_BLEND);

        glDisable(GL_DEPTH_TEST); // Desabilita o teste de profundidade para o texto 2D
        glDisable(GL_LIGHTING); // Desabilita a iluminação para o texto 2D
        glColor3f(1,1,1);
        drawBitmapText("CONTROLES", cx - 60, cy + 140);
        drawBitmapText("W / S / A / D    -> mover no plano XZ (modo teclado)", cx - 240, cy + 90);
        drawBitmapText("Mouse             -> mover em rumo ao cursor", cx - 240, cy + 60);
        drawBitmapText("M                 -> alterna mouse / WASD", cx - 240, cy + 30);
        drawBitmapText("Espaço            -> dash curto à frente", cx - 240, cy + 0);
        drawBitmapText("P                 -> pausar / R -> reiniciar / ESC -> sair", cx - 240, cy - 30);
        drawBitmapText("Pressione ou ESC para voltar ao menu", cx - 200, cy - 100);

        glMatrixMode(GL_MODELVIEW); glPopMatrix();
        glMatrixMode(GL_PROJECTION); glPopMatrix();
        glEnable(GL_LIGHTING); // Reabilita a iluminação
        glEnable(GL_DEPTH_TEST); // Reabilita o teste de profundidade
        return;
    }

    // Texto de HUD normal
    glColor3f(1,1,1);
    drawBitmapText("Score: "+std::to_string(score)+
                   "  Lives: "+std::to_string(lives)+
                   "  Level: "+std::to_string(level)+
                   "  Mass: "+std::to_string((int)mass)+
                   (mouseFollow?"  Mode: Mouse":"  Mode: WASD"), 10, h-24);

    if (!started && !gameOver) {
        drawBitmapText("Mova o mouse ou WASD para começar (M alterna modo)", 10, h/2 + 10);
        drawBitmapText("Coma vermelhos MENORES; toque num MAIOR = Game Over", 10, h/2 - 12);
    }
    if (paused && !gameOver) drawBitmapText("PAUSADO (P para continuar)", 10, h/2);
    if (gameOver) drawBitmapText("GAME OVER — pressione R para tentar novamente", 10, h/2);

    glEnable(GL_LIGHTING);
    glMatrixMode(GL_MODELVIEW); glPopMatrix();
    glMatrixMode(GL_PROJECTION); glPopMatrix();
}

//======================= Spawns e lógica ============================//
static void respawnInside(Vec3& p)
{ p.x = clampf(p.x, -WORLD_HALF, WORLD_HALF); p.z = clampf(p.z, -WORLD_HALF, WORLD_HALF); }

static Obj makeEnemy()
{
    Obj e; e.kind=1; e.pos={frand(-WORLD_HALF, WORLD_HALF), 0.6f, frand(-WORLD_HALF, WORLD_HALF)}; e.rot=frand(0,360);
    e.r = frand(0.35f, 2.2f); // **tamanho variado**
    float v = frand(3.0f, 6.0f) * (1.9f - 0.30f * e.r);
    v = std::max(v, 1.2f); // maiores andam mais devagar
    e.vel={frand(-v,v), 0, frand(-v,v)};
    return e;
}

static void spawnWorld()
{
    pellets.clear(); enemies.clear();
    // pellets
    for (int i=0;i<16;i++) {
        Obj o; o.kind=0; o.r=0.45f; o.pos={frand(-WORLD_HALF, WORLD_HALF), 0.5f, frand(-WORLD_HALF, WORLD_HALF)}; o.rot=frand(0,360); pellets.push_back(o);
    }
    // inimigos
    for (int i=0;i<12;i++) enemies.push_back(makeEnemy());
}

static void resetGame()
{
    score=0; lives=1; level=1; paused=false; gameOver=false; started=false;
    player={0.f,0.6f,0.f}; playerYaw=0.f; dashCd=0.f; playerCurrentVel={0.f,0.f,0.f}; maxPlayerSpeed=9.0f; mass=1.f;
    // abrir menu ao resetar
    menuActive = true;
    showControlsMenu = false;
    menuIndex = 0;
    spawnWorld();
}

static void spawnParticles(const Vec3& pos, int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.pos = pos;
        p.vel = {frand(-2.0f, 2.0f), frand(1.0f, 4.0f), frand(-2.0f, 2.0f)};
        p.life = frand(0.5f, 1.5f);
        p.r = 1.0f;
        p.g = frand(0.2f, 1.0f);
        p.b = 0.2f;
        particles.push_back(p);
    }
}

static void nextLevel()
{
    level++;
    enemies.push_back(makeEnemy());
    enemies.push_back(makeEnemy());
}

static void updateGame(float dt)
{
    if (!started || paused || gameOver) return;

    dashCd = std::max(0.f, dashCd - dt);
    // Direção de movimento — mouse ou WASD
    Vec3 inputDir{0,0,0};
    if (mouseFollow) {
        float cx = (float)mouseX - 0.5f*(float)glutGet(GLUT_WINDOW_WIDTH);
        float cz = (float)mouseY - 0.5f*(float)glutGet(GLUT_WINDOW_HEIGHT);
        float len = std::sqrt(cx*cx + cz*cz);
        if (len > 8.f) { inputDir.x = cx/len; inputDir.z = cz/len; }
    } else {
        if (keys['w'] || skey[GLUT_KEY_UP])    inputDir.z -= 1.f;
        if (keys['s'] || skey[GLUT_KEY_DOWN])  inputDir.z += 1.f;
        if (keys['a'] || skey[GLUT_KEY_LEFT])  inputDir.x -= 1.f;
        if (keys['d'] || skey[GLUT_KEY_RIGHT]) inputDir.x += 1.f;
        float len = std::sqrt(inputDir.x*inputDir.x + inputDir.z*inputDir.z);
        if (len>0) { inputDir.x/=len; inputDir.z/=len; }
    }

    // Atualiza a orientação visual do jogador
    if (inputDir.x!=0.f || inputDir.z!=0.f)
        playerYaw = std::atan2(inputDir.x, -inputDir.z) * 180.f / 3.1415926f;

    // velocidade diminui com massa (agar.io feel)
    float currentMaxSpeed = maxPlayerSpeed / (1.0f + 0.08f*mass);
        if (keys[' '] && (inputDir.x!=0.f || inputDir.z!=0.f)) {
        if (dashCd<=0.f) {
            currentMaxSpeed *= 12.0f; // Dobra a distância do dash (6.0f * 2)
            dashCd = 0.6f;
            // Gera partículas brancas no rastro do dash
            for (int i = 0; i < 20; ++i) {
                Particle p;
                p.pos = player;
                p.vel = {frand(-1.0f, 1.0f), frand(0.5f, 2.0f), frand(-1.0f, 1.0f)};
                p.life = frand(0.3f, 0.8f);
                p.r = 1.0f; p.g = 1.0f; p.b = 1.0f; // Partículas brancas
                particles.push_back(p);
            }
        }
    }

    Vec3 targetVel = {0.f, 0.f, 0.f};
    if (inputDir.x != 0.f || inputDir.z != 0.f) {
        // Se há input, define a velocidade alvo na direção do input
        targetVel.x = inputDir.x * currentMaxSpeed;
        targetVel.z = inputDir.z * currentMaxSpeed;
    }

    // Interpola a velocidade atual em direção à velocidade alvo (aceleração/desaceleração adaptativa)
    playerCurrentVel.x = playerCurrentVel.x * (1.0f - accelerationFactor) + targetVel.x * accelerationFactor;
    playerCurrentVel.z = playerCurrentVel.z * (1.0f - accelerationFactor) + targetVel.z * accelerationFactor;

    // Aplica desaceleração adicional se não houver input e a velocidade alvo for zero
    if (inputDir.x == 0.f && inputDir.z == 0.f) {
        playerCurrentVel.x *= decelerationFactor;
        playerCurrentVel.z *= decelerationFactor;
    }

    // Parar completamente se a velocidade for muito baixa para evitar movimento residual
    if (std::abs(playerCurrentVel.x) < 0.1f) playerCurrentVel.x = 0.f;
    if (std::abs(playerCurrentVel.z) < 0.1f) playerCurrentVel.z = 0.f;

    player.x += playerCurrentVel.x * dt;
    player.z += playerCurrentVel.z * dt;
    respawnInside(player);

    // Inimigos se movem e rebatem
    for (auto& e : enemies) {
        e.pos.x += e.vel.x * dt; e.pos.z += e.vel.z * dt; e.rot += 30.f*dt;
        if (e.pos.x < -WORLD_HALF || e.pos.x > WORLD_HALF) e.vel.x *= -1.f;
        if (e.pos.z < -WORLD_HALF || e.pos.z > WORLD_HALF) e.vel.z *= -1.f;
        respawnInside(e.pos);
    }
    for (auto& p : pellets) p.rot += 60.f*dt;

    // Raio efetivo do jogador (cresce com a massa)
    float playerRad = playerR * std::cbrt(mass);

    // Comer pellets (crescimento leve)
    for (auto& p : pellets) {
        if (dist2(player, p.pos) <= (playerRad + p.r)*(playerRad + p.r)) {
            score += 1; mass += 0.15f;
            spawnParticles(p.pos, 5); // Adiciona 5 partículas no local do pellet
            p.pos = { frand(-WORLD_HALF, WORLD_HALF), 0.5f, frand(-WORLD_HALF, WORLD_HALF) };
            if (score>0 && score%12==0) nextLevel();
        }
    }

    // Interação com inimigos (cubo vermelho)
    for (auto& e : enemies) {
        float er = e.r; // raio do inimigo
        if (dist2(player, e.pos) <= (playerRad + er)*(playerRad + er)) {
            if (playerRad > er * 1.04f) {
                // Jogador come o inimigo MENOR
                score += (int)std::round(2 + er*2);
                mass += 0.25f + 0.35f*er; // cresce proporcional ao tamanho comido
                spawnParticles(e.pos, 15);
                e = makeEnemy();
                continue;
            } else if (playerRad < er * 0.96f) {
                // Inimigo MAIOR → morte instantânea
                lives = 0; gameOver = true; paused=false; started=true;
                break;
            } else {
                // tamanhos parecidos: empurra levemente
                Vec3 push{ player.x - e.pos.x, 0, player.z - e.pos.z };
                float len = std::sqrt(push.x*push.x + push.z*push.z) + 1e-5f;
                push.x/=len; push.z/=len;
                player.x += push.x * 0.6f; player.z += push.z * 0.6f;
                respawnInside(player);
            }
        }
    }

    // Atualiza partículas
    for (auto it = particles.begin(); it != particles.end(); ) {
        it->pos.x += it->vel.x * dt;
        it->pos.y += it->vel.y * dt;
        it->pos.z += it->vel.z * dt;
        it->life -= dt; // Partículas desaparecem com o tempo
        if (it->life <= 0.f) {
            it = particles.erase(it);
        } else {
            ++it;
        }
    }
}

//======================= GLUT callbacks ============================//
static int winW=1280, winH=720;

static void display()
{
    glClearColor(0.06f, 0.06f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    setLight();

    // Câmera isométrica/top-down leve — fixa
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(60.0, winW/(double)winH, 0.1, 600.0);

    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    Vec3 eye{ player.x, 18.0f, player.z + 16.0f };
    gluLookAt(eye.x, eye.y, eye.z, player.x, player.y, player.z, 0.0, 1.0, 0.0);
    setLight();

    // Desenha o cenário
    drawArena();

    // Desenha entidades do jogo
    for (const auto& p : pellets) drawPellet(p);
    for (const auto& e : enemies) drawEnemy(e);
    drawPlayer();
    drawParticles(); // Desenha as partículas

    // HUD 2D
    drawHUD(winW, winH);

    glutSwapBuffers();
}

static void reshape(int w, int h)
{ winW = std::max(1,w); winH = std::max(1,h); glViewport(0,0,winW,winH); }

static void timer(int)
{
    int t = glutGet(GLUT_ELAPSED_TIME);
    float dt = (t - lastTicks) / 1000.f; if (dt > 0.1f) dt = 0.1f; lastTicks = t;
    updateGame(dt);
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

static void keyDown(unsigned char k, int, int)
{
    keys[(unsigned char)std::tolower(k)] = true;

    // Se estamos no menu, tratar navegação/seleção
    if (menuActive) {
        if (k==27) { std::exit(0); } // ESC no menu fecha
        if (k=='w' || k=='W') { menuIndex = (menuIndex + MENU_ITEMS - 1) % MENU_ITEMS; return; }
        if (k=='s' || k=='S') { menuIndex = (menuIndex + 1) % MENU_ITEMS; return; }
        if (k==13 || k=='\r' || k=='\n') { // Enter
            if (menuIndex==0) { started=true; menuActive=false; showControlsMenu=false; }
            else if (menuIndex==1) { showControlsMenu=true; menuActive=false; }
            else if (menuIndex==2) { std::exit(0); }
            return;
        }
    }

    // Se estamos na tela de controles, voltar com B/ESC
    if (showControlsMenu) {
        if (k==27 || k=='b' || k=='B') { showControlsMenu=false; menuActive=true; }
        return;
    }

    if (!started && k!=27) started = true;
    if (k==27) std::exit(0);
    if (k=='p' || k=='P') { if (!gameOver) paused = !paused; }
    if (k=='r' || k=='R') { resetGame(); }
    if (k=='m' || k=='M') { mouseFollow = !mouseFollow; }
}

static void keyUp(unsigned char k, int, int) { keys[(unsigned char)std::tolower(k)] = false; }

static void skeyDown(int k, int, int) {
    skey[k] = true;
    if (menuActive) {
        if (k==GLUT_KEY_UP) { menuIndex = (menuIndex + MENU_ITEMS - 1) % MENU_ITEMS; return; }
        if (k==GLUT_KEY_DOWN) { menuIndex = (menuIndex + 1) % MENU_ITEMS; return; }
        // Enter via special key isn't typical; user can use Enter (ASCII).
        return;
    }
    if (!started) started = true;
}
static void skeyUp(int k, int, int) { skey[k] = false; }

static void mouseMove(int x, int y) { mouseX = x; mouseY = y; if (!started && !menuActive) started=true; }

//============================== main ================================//
int main(int argc, char** argv)
{
    std::srand((unsigned)std::time(nullptr));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(winW, winH);
    glutCreateWindow("Jogo Geométrico 3D — Arena (OpenGL/GLUT)");

    glEnable(GL_DEPTH_TEST);

    resetGame();
    lastTicks = glutGet(GLUT_ELAPSED_TIME);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
    glutSpecialFunc(skeyDown);
    glutSpecialUpFunc(skeyUp);
    glutPassiveMotionFunc(mouseMove);
    glutTimerFunc(16, timer, 0);

    glutMainLoop();
    return 0;
}
