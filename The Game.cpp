#include "SFML\Graphics.hpp"
#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- VARIÁVEIS GLOBAIS DE ANIMAÇÃO ---
// Texturas para cada direção do Isaac
std::vector<sf::Texture> textures_walk_down;
std::vector<sf::Texture> textures_walk_up;
std::vector<sf::Texture> textures_walk_left;
std::vector<sf::Texture> textures_walk_right;

// Texturas para o Inimigo (Demon)
std::vector<sf::Texture> textures_enemy_walk_down;
std::vector<sf::Texture> textures_enemy_walk_up;
std::vector<sf::Texture> textures_enemy_walk_left;
std::vector<sf::Texture> textures_enemy_walk_right;

// Texturas para o Inimigo Bishop
std::vector<sf::Texture> textures_bishop_animation;


// Variáveis de animação do Isaac
int current_frame = 0;
float animation_time = 0.0f;
const float frame_duration = 0.100f;          // Duração de cada frame (Isaac)

// Variáveis de animação do Inimigo (Demon)
int enemy_current_frame = 0;
float enemy_animation_time = 0.0f;
const float enemy_frame_duration = 0.12f;    // Duração de cada frame (Inimigo)

// Variáveis de animação do Bishop
int bishop_current_frame = 0;
float bishop_animation_time = 0.0f;
const float bishop_frame_duration = 0.08f;       // Duração para frames rápidos (B2-B8, B10-B14)
sf::Clock bishop_cooldown_clock;                 // Clock para o ciclo de 7 segundos
const sf::Time bishop_cooldown_time = sf::seconds(7.0f);
bool is_bishop_animating = false;                // Flag para controlar se está em animação
const int BISHOP_TOTAL_FRAMES = 14;              // ATUALIZADO: B1 a B14 (14 frames)

// Cooldown para a pausa longa apenas no B9
const float bishop_long_pause_duration = 1.0f;   // 1 segundo no B9
float bishop_long_pause_time = 0.0f;             // Temporizador para a pausa

// Variáveis de Efeito de Dano (Red Flash)
sf::Clock isaacHitClock;
const sf::Time hitFlashDuration = sf::milliseconds(100); // 0.1 segundos de flash de DANO
bool isIsaacHit = false;

sf::Clock enemyHitClock;
bool isEnemyHit = false;

// Variáveis de Efeito de Cura (Green Flash) - ATUALIZADO
sf::Clock enemyHealClock;
const sf::Time healFlashDuration = sf::seconds(2.0f); // 2.0 segundos de flash de CURA
bool isEnemyHealed = false;
// --------------------------------------

// Estrutura para definir os parâmetros de cada animação
struct AnimationSet {
    std::string folderName;
    std::string prefix;
    int totalFrames;
    std::vector<sf::Texture>* textureVector;
};

// Estrutura para os Projéteis
struct Projectile {
    sf::Sprite sprite;
    sf::Vector2f direction;
    float distanceTraveled;
};
// --------------------------------------

// Função para calcular colisão entre dois retângulos
bool checkCollision(const sf::FloatRect& a, const sf::FloatRect& b)
{
    return (a.position.x < b.position.x + b.size.x &&
        a.position.x + a.size.x > b.position.x &&
        a.position.y < b.position.y + b.size.y &&
        a.position.y + b.size.y > b.position.y);
}

// ANGULO ENTRE 2 PONTOS
float calculateAngle(const sf::Vector2f& p1, const sf::Vector2f& p2)
{
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    return static_cast<float>(std::atan2(dy, dx) * 180.0f / M_PI);
}

// --- FUNÇÃO PARA CARREGAR TODAS AS TEXTURAS DE ANIMAÇÃO ---
bool loadAnimationTextures() {
    std::string base_path = "Images/";

    // Configuração de cada animação: Pasta | Prefixo | Frames | Vector
    std::vector<AnimationSet> sets = {
        // Sets do Isaac
        {"Front_Isaac", "F", 9, &textures_walk_down},
        {"Back_Isaac" , "B", 9, &textures_walk_up},
        {"Left_Isaac" , "L", 6,  &textures_walk_left},
        {"Right_Isaac", "R", 6,  &textures_walk_right},

        // Sets do Inimigo
        {"Front_Demon", "F", 8, &textures_enemy_walk_down},
        {"Back_Demon",  "B", 8, &textures_enemy_walk_up},
        {"Left_Demon",  "L", 8, &textures_enemy_walk_left},
        {"Right_Demon", "R", 8, &textures_enemy_walk_right},

        // Set do Bishop (usa BISHOP_TOTAL_FRAMES = 14)
        {"Bishop", "B", BISHOP_TOTAL_FRAMES, &textures_bishop_animation}
    };

    for (const auto& set : sets) {
        set.textureVector->resize(set.totalFrames);
        for (int i = 0; i < set.totalFrames; ++i) {
            std::string filename;

            // Lógica para o Bishop (B1.png, B2.png, ...)
            if (set.folderName == "Bishop") {
                // Formato: Images/Bishop/B1.png
                filename = base_path + set.folderName + "/" +
                    set.prefix + std::to_string(i + 1) + ".png";
            }
            // Lógica para o prefixo do Demónio (sufixo 'D', Ex: F1D.png)
            else if (set.folderName.find("Demon") != std::string::npos) {
                filename = base_path + set.folderName + "/" +
                    set.prefix + std::to_string(i + 1) + "D.png";
            }
            else {
                // Lógica para o prefixo do Isaac (sufixo 'V1', Ex: F1V1.png)
                filename = base_path + set.folderName + "/" +
                    set.prefix + std::to_string(i + 1) + "V1.png";
            }

            if (!set.textureVector->at(i).loadFromFile(filename)) {
                std::cerr << "Erro ao carregar a textura da animação: " << filename << std::endl;
                return false;
            }
        }
    }
    return true;
}
// -----------------------------------------------------

enum class GameState {
    menu,
    firstRoom,
    playing,
    sttings,
    exiting,
};

int main()
{
    sf::RenderWindow window(sf::VideoMode({ 1920, 1080 }), "Isaac test");
    GameState currentState = GameState::menu;
    //-----------------RETIRÁVEL DO MAIN (ACHO EU)------------------------
     //PlayButton
    sf::Texture pButton;
    pButton.loadFromFile("Images/playButton.png");
    sf::Sprite playButton(pButton);
    playButton.setPosition({ 150.0f, 170.0f });
    playButton.setScale(sf::Vector2f(0.5, 0.5));

    //PlayButton
    sf::Texture eButton;
    eButton.loadFromFile("Images/exitButton.png");
    sf::Sprite exitButton(eButton);
    exitButton.setPosition({ 150.0f, 750.0f });
    exitButton.setScale(sf::Vector2f(0.4900965, 0.537958));

    //Background Menu
    sf::Texture menuTexture;
    if (!menuTexture.loadFromFile("Images/backgroundMenu.png")) return -1;
    sf::Sprite menuGround(menuTexture);
    menuGround.setScale({ 1.25f, 1.05571847f });

    //--------------------------------------------------------------------

    while (window.isOpen() and currentState == GameState::menu) {
        auto mousePosition = sf::Vector2f(sf::Mouse::getPosition(window));
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) window.close();
        }

        if (playButton.getGlobalBounds().contains(mousePosition) and (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)))
        {
            currentState = GameState::playing;
        }
        if (exitButton.getGlobalBounds().contains(mousePosition) and (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)))
        {
            currentState = GameState::exiting;
        }

        //Draw
        window.clear();
        window.draw(menuGround);
        window.draw(playButton);
        window.draw(exitButton);
        window.display();
    }

    if (currentState == GameState::exiting)
    {
        window.close();
    }


    // CARREGA AS TEXTURAS DE ANIMAÇÃO
    if (!loadAnimationTextures()) return -1;

    // ISAAC
    sf::Sprite Isaac(textures_walk_down[0]);
    Isaac.setScale({ 3.f, 3.f });
    Isaac.setOrigin({ 13.5f, 17.f });
    int isaacHealth = 6;

    // Variável para rastrear a última animação usada (para manter a pose ao parar)
    std::vector<sf::Texture>* last_animation_set = &textures_walk_down;

    // --- TEXTURAS DOS CORAÇÕES (HUD) ---
    sf::Texture heartFullTexture;
    if (!heartFullTexture.loadFromFile("Images/UI/Life/Full.png")) return -1;

    sf::Texture heartHalfTexture;
    if (!heartHalfTexture.loadFromFile("Images/UI/Life/Half.png")) return -1;

    sf::Texture heartEmptyTexture;
    if (!heartEmptyTexture.loadFromFile("Images/UI/Life/Empty.png")) return -1;

    // DECLARAÇÃO DO SPRITE DE VIDA FORA DO LOOP (CORREÇÃO DO ERRO DO CONSTRUTOR)
    sf::Sprite heartSpriteF(heartFullTexture);
    sf::Sprite heartSpriteH(heartHalfTexture);
    sf::Sprite heartSpriteE(heartEmptyTexture);
    // ----------------------------------


    // INIMIGO
    sf::Sprite Enemy(textures_enemy_walk_down[0]); // Padrão inicial
    Enemy.setScale({ 3.f, 3.f });
    Enemy.setPosition({ 960.f, 540.f });
    Enemy.setOrigin({ 14.f, 16.f });
    int enemyHealth = 10; // Variável global de vida do Demon

    // BISHOP (Usando as texturas carregadas)
    sf::Sprite eBishop(textures_bishop_animation[0]); // Começa no B1.png
    eBishop.setScale({ 3.f, 3.f });
    eBishop.setPosition({ 960.f, 540.f });
    eBishop.setOrigin({ 14.f, 16.f });

    // Reinicia o clock do Bishop para começar o ciclo de 7 segundos
    bishop_cooldown_clock.restart();


    // BACKGROUND
    sf::Texture Basement_Corner;
    Basement_Corner.loadFromFile("Images/Background/Basement_sheet.png");

    float scaleX = 960.f / 234.f;
    float scaleY = 540.f / 156.f;

    // canto superior esquerdo (normal)
    sf::Sprite cornerTL(Basement_Corner);
    cornerTL.setTextureRect(sf::IntRect({ 0, 0 }, { 234, 156 }));
    cornerTL.setPosition({ 0.f, 0.f });
    cornerTL.setScale({ scaleX, scaleY });

    // canto superior direito (flip vertical)
    sf::Sprite cornerTR(Basement_Corner);
    cornerTR.setTextureRect(sf::IntRect({ 0, 0 }, { 234, 156 }));
    cornerTR.setPosition({ 1920.f, 0.f });
    cornerTR.setScale({ -scaleX, scaleY });

    // canto inferior esquerdo (flip horizontal)
    sf::Sprite cornerBL(Basement_Corner);
    cornerBL.setTextureRect(sf::IntRect({ 0, 0 }, { 234, 156 }));
    cornerBL.setPosition({ 0.f, 1080.f });
    cornerBL.setScale({ scaleX, -scaleY });

    // canto inferior direito (sem simetria perfeita)
    sf::Sprite cornerBR(Basement_Corner);
    cornerBR.setTextureRect(sf::IntRect({ 0, 0 }, { 234, 156 }));
    cornerBR.setPosition({ 1920.f, 1080.f });
    cornerBR.setScale({ -scaleX, -scaleY });

    // LIMITES INTERNOS (paredes dentro do background)
    float left = 213.33f;
    float top = 179.80f;
    float width = 1493.34f;
    float height = 720.40f;

    sf::FloatRect gameBounds({ left, top }, { width, height });

    // ARMAS
    sf::Texture HitTexture;
    if (!HitTexture.loadFromFile("Images/Hit.png")) return -1;
    sf::Texture EnemyHitTexture;
    if (!EnemyHitTexture.loadFromFile("Images/Enemy_Knife.png")) return -1;

    sf::Clock clock;


    std::vector<Projectile> isaacProjectiles;
    std::vector<Projectile> enemyProjectiles;

    float isaacHitSpeed = 600.f;
    float enemyHitSpeed = 950.f;
    float maxHitDistance = 600.f;

    // COOLDOWN
    sf::Clock isaacCooldownClock;
    sf::Time isaacCooldownTime = sf::seconds(0.3f);
    sf::Clock enemyCooldownClock;
    sf::Time enemyCooldownTime = sf::seconds(3.5f);

    while (window.isOpen())
    {
        sf::Time deltaTime = clock.restart();
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) window.close();
        }

        // MOVIMENTO ISAAC
        sf::Vector2f move(0.f, 0.f);
        float speed = 300.f;

        // NOVAS VARIÁVEIS para o controlo da animação do Isaac
        bool is_moving = false;
        std::vector<sf::Texture>* current_animation_set = nullptr;
        int current_total_frames = 0;

        // --- 1. PRIORIDADE DE ANIMAÇÃO DO ISAAC: ATAQUE (SETAS) ---
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Up)) {
            current_animation_set = &textures_walk_up;
            current_total_frames = 9; // 10 frames
            is_moving = true;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Down)) {
            current_animation_set = &textures_walk_down;
            current_total_frames = 9; // 10 frames
            is_moving = true;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Left)) {
            current_animation_set = &textures_walk_left;
            current_total_frames = 6; // 6 frames
            is_moving = true;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Right)) {
            current_animation_set = &textures_walk_right;
            current_total_frames = 6; // 6 frames
            is_moving = true;
        }

        // --- 2. MOVIMENTO E ANIMAÇÃO DE MOVIMENTO DO ISAAC (WASD) ---

        // Aplica o movimento WASD, independentemente de estar a atacar
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S)) move += {0.f, 1.f};
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W)) move += {0.f, -1.f};
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)) move += {-1.f, 0.f};
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)) move += {1.f, 0.f};

        // Se NÃO estiver a atacar (is_moving == false neste ponto), a animação é controlada pelo WASD.
        if (!is_moving) {

            // Prioridade vertical (S e W) para animação
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S)) {
                current_animation_set = &textures_walk_down;
                current_total_frames = 9;
                is_moving = true;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W)) {
                current_animation_set = &textures_walk_up;
                current_total_frames = 9;
                is_moving = true;
            }

            // Prioridade horizontal (A e D) para animação
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)) {
                if (!current_animation_set) {
                    current_animation_set = &textures_walk_left;
                    current_total_frames = 6;
                    is_moving = true;
                }
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)) {
                if (!current_animation_set && !sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)) {
                    current_animation_set = &textures_walk_right;
                    current_total_frames = 6;
                    is_moving = true;
                }
            }
        }

        // Aplica o movimento do Isaac
        Isaac.move(move * deltaTime.asSeconds() * speed);

        // --- LÓGICA DE ANIMAÇÃO DO ISAAC ---
        if (is_moving) {
            static std::vector<sf::Texture>* last_animation_checked_set = &textures_walk_down;
            if (current_animation_set != last_animation_checked_set) {
                current_frame = 0;
                animation_time = 0.0f;
                last_animation_checked_set = current_animation_set;
            }

            last_animation_set = current_animation_set;
            animation_time += deltaTime.asSeconds();

            if (animation_time >= frame_duration) {
                animation_time -= frame_duration;
                current_frame = (current_frame + 1) % current_total_frames;
                Isaac.setTexture(current_animation_set->at(current_frame));
            }
        }
        else {
            current_frame = 0;
            animation_time = 0.0f;
            Isaac.setTexture(last_animation_set->at(0));
            static std::vector<sf::Texture>* last_animation_checked_set = &textures_walk_down;
            last_animation_checked_set = last_animation_set;
        }
        // -----------------------------------

        // LIMITES ISAAC
        sf::Vector2f newPos = Isaac.getPosition();
        sf::FloatRect isaacBounds = Isaac.getGlobalBounds();

        newPos.x = std::min(std::max(newPos.x, gameBounds.position.x + isaacBounds.size.x / 2.f), gameBounds.position.x + gameBounds.size.x - isaacBounds.size.x / 2.f);
        newPos.y = std::min(std::max(newPos.y, gameBounds.position.y + isaacBounds.size.y / 2.f), gameBounds.position.y + gameBounds.size.y - isaacBounds.size.y / 2.f);
        Isaac.setPosition(newPos);

        // MOVIMENTO INIMIGO (Demon)
        sf::Vector2f dirToIsaac = Isaac.getPosition() - Enemy.getPosition();
        float dist = std::sqrt(dirToIsaac.x * dirToIsaac.x + dirToIsaac.y * dirToIsaac.y);
        if (dist != 0.f)
        {
            dirToIsaac /= dist;
            Enemy.move(dirToIsaac * 100.f * deltaTime.asSeconds());
        }

        // --- LÓGICA DE ANIMAÇÃO DO INIMIGO ---
        if (enemyHealth > 0)
        {
            std::vector<sf::Texture>* current_enemy_set = &textures_enemy_walk_down; // Padrão
            int current_enemy_total_frames = 8;

            // Prioridade de animação: Vertical ou Horizontal, baseada na direção dominante
            if (std::abs(dirToIsaac.y) > std::abs(dirToIsaac.x)) {
                // Movimento Vertical dominante
                if (dirToIsaac.y > 0) { // Baixo (Front)
                    current_enemy_set = &textures_enemy_walk_down;
                    current_enemy_total_frames = 8;
                }
                else { // Cima (Back)
                    current_enemy_set = &textures_enemy_walk_up;
                    current_enemy_total_frames = 8;
                }
            }
            else {
                // Movimento Horizontal dominante
                if (dirToIsaac.x < 0) { // Esquerda
                    current_enemy_set = &textures_enemy_walk_left;
                    current_enemy_total_frames = 8;
                }
                else { // Direita
                    current_enemy_set = &textures_enemy_walk_right;
                    current_enemy_total_frames = 8;
                }
            }

            // Animar os frames
            enemy_animation_time += deltaTime.asSeconds();

            if (enemy_animation_time >= enemy_frame_duration) {
                enemy_animation_time -= enemy_frame_duration;

                // Resetar o frame se o set de animação mudou
                static std::vector<sf::Texture>* last_enemy_set = current_enemy_set;
                if (current_enemy_set != last_enemy_set) {
                    enemy_current_frame = 0;
                    last_enemy_set = current_enemy_set;
                }

                // Avança o frame em loop
                enemy_current_frame = (enemy_current_frame + 1) % current_enemy_total_frames;

                // Aplica a nova textura
                Enemy.setScale({ 3.f, 3.f }); // Garante que o sprite não está 'flipped'
                Enemy.setTexture(current_enemy_set->at(enemy_current_frame));
            }
        }
        // ---------------------------------------------


        // LIMITES INIMIGO
        sf::Vector2f newEnemyPos = Enemy.getPosition();
        sf::FloatRect enemyBounds = Enemy.getGlobalBounds();

        newEnemyPos.x = std::min(std::max(newEnemyPos.x, gameBounds.position.x + enemyBounds.size.x / 2.f), gameBounds.position.x + gameBounds.size.x - enemyBounds.size.x / 2.f);
        newEnemyPos.y = std::min(std::max(newEnemyPos.y, gameBounds.position.y + enemyBounds.size.y / 2.f), gameBounds.position.y + gameBounds.size.y - enemyBounds.size.y / 2.f);
        Enemy.setPosition(newEnemyPos);

        // -----------------------------------------------------------------------------------
        // --- LÓGICA DE ANIMAÇÃO DO BISHOP (COM CURA E NOVO FLASH VERDE) ---
        // -----------------------------------------------------------------------------------

        if (!is_bishop_animating) {
            // 1. CHECA SE É HORA DE INICIAR A ANIMAÇÃO (B1 -> B2)
            if (bishop_cooldown_clock.getElapsedTime() >= bishop_cooldown_time) {
                is_bishop_animating = true;
                bishop_current_frame = 1;              // Começa no B2 (Índice 1)
                bishop_animation_time = 0.0f;
                bishop_long_pause_time = 0.0f;         // Reset do temporizador de pausa
                eBishop.setTexture(textures_bishop_animation[bishop_current_frame]); // Aplica B2
            }
            // Caso contrário, fica no frame B1 (Índice 0)
        }
        else {
            // 2. EXECUTAR A PAUSA OU A ANIMAÇÃO RÁPIDA

            // **A: Lógica de Pausa no B9**
            if (bishop_current_frame == 8) { // Frame B9 (Índice 8)
                bishop_long_pause_time += deltaTime.asSeconds();

                if (bishop_long_pause_time >= bishop_long_pause_duration) {
                    // Terminou a pausa de 1 segundo, avança para B10
                    bishop_current_frame++;                // Vai para B10 (Índice 9)
                    eBishop.setTexture(textures_bishop_animation[bishop_current_frame]);
                    bishop_animation_time = 0.0f;          // Prepara para a animação rápida de volta
                }
            }
            // **B: Animação Rápida (Frames B2-B8 e B10-B14)**
            else {
                bishop_animation_time += deltaTime.asSeconds();

                if (bishop_animation_time >= bishop_frame_duration) {
                    bishop_animation_time -= bishop_frame_duration;

                    // Avança o frame
                    bishop_current_frame++;

                    // LÓGICA DE CURA DO DEMON NO B8 (Índice 7)
                    if (bishop_current_frame == 8) {
                        if (enemyHealth > 0) {
                            enemyHealth += 2; // Demon recupera 2 de vida
                            // ATUALIZADO: Ativa o flash verde de 2.0s
                            isEnemyHealed = true;
                            enemyHealClock.restart();
                            // --------------------------------------------
                            std::cout << "Bishop chegou em B8! Demon recuperou 2 de vida. Nova vida: " << enemyHealth << std::endl;
                        }
                    }

                    // 3. Verifica se a animação chegou ao FIM (após B14)
                    if (bishop_current_frame >= BISHOP_TOTAL_FRAMES) { // Chegou após B14 (Índice 13)
                        // FIM DA ANIMAÇÃO, volta ao B1 e reinicia o ciclo
                        is_bishop_animating = false;
                        bishop_current_frame = 0;              // Garante que fica em B1
                        eBishop.setTexture(textures_bishop_animation[0]);
                        bishop_cooldown_clock.restart();         // Reinicia o ciclo de 7 segundos
                    }
                    // 4. Verifica se a animação chegou ao B9 (Para a Pausa)
                    else if (bishop_current_frame == 8) { // Chegou no B9 (Índice 8)
                        eBishop.setTexture(textures_bishop_animation[bishop_current_frame]);
                        bishop_long_pause_time = 0.0f;           // Inicia o contador de pausa
                        // Não faz mais nada, no próximo frame passará pelo bloco de Pausa (A)
                    }
                    // 5. Se não chegou ao fim nem à pausa, aplica a nova textura
                    else {
                        eBishop.setTexture(textures_bishop_animation[bishop_current_frame]);
                    }
                }
            }
        }
        // -----------------------------------------------------------------------------------

        // ATAQUE ISAAC
        if (isaacCooldownClock.getElapsedTime() >= isaacCooldownTime)
        {
            struct KeyDir { sf::Keyboard::Scancode key; sf::Vector2f dir; float rot; };
            KeyDir dirs[4] = {
                {sf::Keyboard::Scancode::Up, {0.f,-1.f}, -180.f},
                {sf::Keyboard::Scancode::Down, {0.f,1.f}, 0.f},
                {sf::Keyboard::Scancode::Left, {-1.f,0.f}, 90.f},
                {sf::Keyboard::Scancode::Right, {1.f,0.f}, -90.f}
            };
            for (auto& d : dirs)
            {
                if (sf::Keyboard::isKeyPressed(d.key))
                {
                    Projectile p = { sf::Sprite(HitTexture), d.dir, 0.f };
                    p.sprite.setScale({ 1.5f,1.5f });
                    p.sprite.setOrigin({ 7.5f,12.f });
                    p.sprite.setPosition(Isaac.getPosition());
                    p.sprite.setRotation(sf::degrees(d.rot));
                    isaacProjectiles.push_back(p);
                    isaacCooldownClock.restart();

                    break;
                }
            }
        }


        // ATAQUE INIMIGO
        if (enemyCooldownClock.getElapsedTime() >= enemyCooldownTime)
        {
            Projectile p = { sf::Sprite(EnemyHitTexture), dirToIsaac, 0.f };
            p.sprite.setOrigin({ 4.f / 2.f,74.f / 2.f });
            p.sprite.setPosition(Enemy.getPosition());
            float angle = calculateAngle(Enemy.getPosition(), Isaac.getPosition());
            p.sprite.setRotation(sf::degrees(angle - 90.f));
            enemyProjectiles.push_back(p);
            enemyCooldownClock.restart();
        }
        // CHECAR VIDA
        if (enemyHealth <= 0 || isaacHealth <= 0)
            window.close();

        // PROJETEIS ISAAC (VERIFICAÇÃO DE DANO NO INIMIGO)
        for (auto it = isaacProjectiles.begin(); it != isaacProjectiles.end(); )
        {
            it->sprite.move(it->direction * isaacHitSpeed * deltaTime.asSeconds());
            it->distanceTraveled += isaacHitSpeed * deltaTime.asSeconds();
            sf::FloatRect projBounds = it->sprite.getGlobalBounds();

            if (checkCollision(projBounds, Enemy.getGlobalBounds()))
            {
                enemyHealth--;
                // ATUALIZAÇÃO DANO INIMIGO (Red Flash)
                isEnemyHit = true;
                enemyHitClock.restart();
                // ---------------------------------
                std::cout << "Demon sofreu dano! Vida restante: " << enemyHealth << std::endl;
                it = isaacProjectiles.erase(it);
            }
            else if (it->distanceTraveled >= maxHitDistance ||
                !checkCollision(projBounds, gameBounds))
                it = isaacProjectiles.erase(it);
            else ++it;
        }

        // PROJETEIS INIMIGO (VERIFICAÇÃO DE DANO NO ISAAC)
        for (auto it = enemyProjectiles.begin(); it != enemyProjectiles.end(); )
        {
            it->sprite.move(it->direction * enemyHitSpeed * deltaTime.asSeconds());
            it->distanceTraveled += enemyHitSpeed * deltaTime.asSeconds();
            sf::FloatRect projBounds = it->sprite.getGlobalBounds();

            if (checkCollision(projBounds, Isaac.getGlobalBounds()))
            {
                isaacHealth -= 2;
                // ATUALIZAÇÃO DANO ISAAC (Red Flash)
                isIsaacHit = true;
                isaacHitClock.restart();
                // ------------------------------
                std::cout << "Isaac sofreu dano! Vida restante: " << isaacHealth << std::endl;
                it = enemyProjectiles.erase(it);
            }
            else if (it->distanceTraveled >= maxHitDistance ||
                !checkCollision(projBounds, gameBounds))
                it = enemyProjectiles.erase(it);
            else ++it;
        }

        // ---------------------------------------------
        // --- LÓGICA DO EFEITO DE DANO/CURA (FLASHES) ---

        // 1. Efeito do Isaac (Dano)
        if (isIsaacHit) {
            if (isaacHitClock.getElapsedTime() < hitFlashDuration) {
                // Fica vermelho durante 0.1s
                Isaac.setColor(sf::Color::Red);
            }
            else {
                // Retorna ao normal e reseta o flag
                Isaac.setColor(sf::Color::White);
                isIsaacHit = false;
            }
        }

        // 2. Efeito do Demon (Dano) - Prioridade sobre a Cura
        if (isEnemyHit) {
            if (enemyHitClock.getElapsedTime() < hitFlashDuration) {
                // Fica vermelho durante 0.1s
                Enemy.setColor(sf::Color::Red);
            }
            else {
                // O dano terminou. Desliga o flag. A cor será redefinida mais abaixo.
                isEnemyHit = false;
            }
        }

        // 3. Efeito do Demon (Cura) - Só é executado se NÃO estiver a piscar vermelho
        else if (isEnemyHealed) {
            if (enemyHealClock.getElapsedTime() < healFlashDuration) {
                // Fica VERDE durante 2.0s - CORREÇÃO APLICADA AQUI!
                Enemy.setColor(sf::Color::Green);
            }
            else {
                // A cura terminou. Desliga o flag.
                isEnemyHealed = false;
            }
        }

        // 4. Reset da Cor do Demon
        // A cor deve ser White APENAS se nenhum dos flashes (dano ou cura) estiver ativo.
        if (!isEnemyHit && !isEnemyHealed) {
            Enemy.setColor(sf::Color::White);
        }

        // ---------------------------------------------

        // DESENHO
        window.clear();
        window.draw(cornerTL);
        window.draw(cornerTR);
        window.draw(cornerBL);
        window.draw(cornerBR);

        // --- LÓGICA DE DESENHO DA VIDA (HUD) ---
        int currentHealth = isaacHealth;
        float xPosition = 0.f + 200.f;
        float yPosition = 90.f;
        const int maxHearts = 3;
        const float heartSpacing = 67.f; // Espaçamento entre corações

        // Aplica a escala uma única vez em todos os sprites fora do loop (melhor performance)
        heartSpriteF.setScale({ 3.f, 3.f });
        heartSpriteH.setScale({ 3.f, 3.f });
        heartSpriteE.setScale({ 3.f, 3.f });

        for (int i = 0; i < maxHearts; ++i)
        {
            // 1. Crie um ponteiro ou referência para o Sprite que vamos desenhar
            const sf::Sprite* spriteToDraw;

            // 2. Decide qual dos 3 sprites deve ser usado neste slot
            if (currentHealth >= 2) // Cheio (2 meias-vidas ou mais)
            {
                spriteToDraw = &heartSpriteF; // Aponta para o Sprite Cheio
            }
            else if (currentHealth == 1) // Meio (1 meia-vida)
            {
                spriteToDraw = &heartSpriteH; // Aponta para o Meio Coração
            }
            else // Vazio (0 meias-vidas)
            {
                spriteToDraw = &heartSpriteE; // Aponta para o Sprite Vazio
            }

            // 3. Define a posição do sprite (todos têm a mesma posição no loop)
            // Usamos a referência temporária para definir a posição
            const_cast<sf::Sprite*>(spriteToDraw)->setPosition({ xPosition, yPosition });

            // 4. Desenha o sprite na tela
            window.draw(*spriteToDraw);

            // 5. Prepara para o próximo slot
            xPosition += heartSpacing; // Move para a direita
            currentHealth -= 2;        // Deduz 2 (um coração completo)
        }
        // ----------------------------------------

        if (enemyHealth > 0) window.draw(Enemy);
        for (auto& p : isaacProjectiles) window.draw(p.sprite);
        for (auto& p : enemyProjectiles) window.draw(p.sprite);
        window.draw(Isaac);
        // O Bishop só é desenhado se existir
        window.draw(eBishop);
        window.display();
    }
    
    return 0;
}