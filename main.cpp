#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <cmath>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int GAME_MARGIN_TOP = 80;
const int GAME_MARGIN_BOTTOM = 60;
const int GAME_MARGIN_SIDES = 40;
const int GAME_WIDTH = WINDOW_WIDTH - (GAME_MARGIN_SIDES * 2);
const int GAME_HEIGHT = WINDOW_HEIGHT - GAME_MARGIN_TOP - GAME_MARGIN_BOTTOM;
const int PADDLE_WIDTH = 20;
const int PADDLE_HEIGHT = 80; // Más pequeño para el área reducida
const int BALL_SIZE = 12; // Más pequeño
const float PADDLE_SPEED = 250.0f;
const float BALL_SPEED = 200.0f;

enum GameMode {
    MENU,
    SINGLE_PLAYER,
    MULTIPLAYER
};

class AudioManager {
private:
    Mix_Music* backgroundMusic;
    Mix_Chunk* paddleSound;
    Mix_Chunk* scoreSound;
    bool musicEnabled;
    int musicVolume;
    
public:
    AudioManager() : backgroundMusic(nullptr), paddleSound(nullptr), 
                     scoreSound(nullptr), musicEnabled(false), musicVolume(64) {}
    
    bool init() {
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            std::cout << "Error inicializando SDL_mixer: " << Mix_GetError() << std::endl;
            return false;
        }
        
        // Cargar música de fondo
        backgroundMusic = Mix_LoadMUS("assets/Funk It - Dyalla.mp3");
        if (!backgroundMusic) {
            std::cout << "Advertencia: No se pudo cargar la música: " << Mix_GetError() << std::endl;
            std::cout << "El juego funcionará sin música de fondo." << std::endl;
        } else {
            std::cout << "Música cargada exitosamente: Funk It - Dyalla" << std::endl;
        }
        
        return true;
    }
    
    void toggleMusic() {
        musicEnabled = !musicEnabled;
        if (musicEnabled) {
            if (backgroundMusic) {
                if (Mix_PlayMusic(backgroundMusic, -1) == -1) {
                    std::cout << "Error reproduciendo música: " << Mix_GetError() << std::endl;
                } else {
                    Mix_VolumeMusic(musicVolume);
                    std::cout << "♪ Música activada: Funk It - Dyalla (Volumen: " << (musicVolume * 100 / 128) << "%)" << std::endl;
                }
            } else {
                std::cout << "No hay música disponible para reproducir" << std::endl;
                musicEnabled = false;
            }
        } else {
            Mix_HaltMusic();
            std::cout << "♪ Música desactivada" << std::endl;
        }
    }
    
    bool isMusicEnabled() const {
        return musicEnabled;
    }
    
    bool isMusicPlaying() const {
        return Mix_PlayingMusic();
    }
    
    void setMusicVolume(int volume) {
        // Volumen entre 0-128
        musicVolume = volume;
        if (musicVolume < 0) musicVolume = 0;
        if (musicVolume > 128) musicVolume = 128;
        Mix_VolumeMusic(musicVolume);
        
        if (musicEnabled) {
            std::cout << "♪ Volumen: " << (musicVolume * 100 / 128) << "%" << std::endl;
        }
    }
    
    void increaseVolume() {
        setMusicVolume(musicVolume + 16);
    }
    
    void decreaseVolume() {
        setMusicVolume(musicVolume - 16);
    }
    
    void cleanup() {
        if (backgroundMusic) {
            Mix_FreeMusic(backgroundMusic);
        }
        if (paddleSound) {
            Mix_FreeChunk(paddleSound);
        }
        if (scoreSound) {
            Mix_FreeChunk(scoreSound);
        }
        Mix_CloseAudio();
    }
};

class Paddle {
public:
    float x, y;
    float speed;
    bool isAI;
    
    Paddle(float startX, float startY, bool aiControlled = false) : 
        x(startX), y(startY), speed(PADDLE_SPEED), isAI(aiControlled) {}
    
    void update(float deltaTime, bool upPressed, bool downPressed) {
        if (upPressed && y > GAME_MARGIN_TOP) {
            y -= speed * deltaTime;
        }
        if (downPressed && y < GAME_MARGIN_TOP + GAME_HEIGHT - PADDLE_HEIGHT) {
            y += speed * deltaTime;
        }
    }
    
    void updateAI(float deltaTime, float ballY, float ballVelocityX) {
        if (!isAI) return;
        
        // Solo reacciona si la pelota se acerca
        if (ballVelocityX > 0) {
            float paddleCenter = y + PADDLE_HEIGHT / 2;
            
            // Agregar algo de imprecisión para hacer la IA más realista
            float difficulty = 0.8f; // 0.0 = muy fácil, 1.0 = perfecto
            float aiSpeed = speed * difficulty;
            
            // Zona muerta para evitar temblores
            float deadZone = 10.0f;
            
            if (paddleCenter < ballY - deadZone) {
                // Mover hacia abajo
                if (y < GAME_MARGIN_TOP + GAME_HEIGHT - PADDLE_HEIGHT) {
                    y += aiSpeed * deltaTime;
                }
            } else if (paddleCenter > ballY + deadZone) {
                // Mover hacia arriba
                if (y > GAME_MARGIN_TOP) {
                    y -= aiSpeed * deltaTime;
                }
            }
        }
    }
    
    SDL_Rect getRect() const {
        return {(int)x, (int)y, PADDLE_WIDTH, PADDLE_HEIGHT};
    }
};

class Ball {
public:
    float x, y;
    float velocityX, velocityY;
    
    Ball() : x(WINDOW_WIDTH / 2), y(GAME_MARGIN_TOP + GAME_HEIGHT / 2), 
             velocityX(BALL_SPEED), velocityY(BALL_SPEED) {}
    
    void update(float deltaTime) {
        x += velocityX * deltaTime;
        y += velocityY * deltaTime;
        
        // Rebote en paredes superior e inferior (dentro del área de juego)
        if (y <= GAME_MARGIN_TOP || y >= GAME_MARGIN_TOP + GAME_HEIGHT - BALL_SIZE) {
            velocityY = -velocityY;
        }
    }
    
    void reset() {
        x = WINDOW_WIDTH / 2;
        y = GAME_MARGIN_TOP + GAME_HEIGHT / 2;
        velocityX = (velocityX > 0) ? -BALL_SPEED : BALL_SPEED;
        velocityY = BALL_SPEED;
    }
    
    bool checkCollision(const Paddle& paddle) {
        SDL_Rect ballRect = {(int)x, (int)y, BALL_SIZE, BALL_SIZE};
        SDL_Rect paddleRect = paddle.getRect();
        
        if (SDL_HasIntersection(&ballRect, &paddleRect)) {
            velocityX = -velocityX;
            
            // Añadir efecto según donde golpee la pelota
            float paddleCenter = paddle.y + PADDLE_HEIGHT / 2;
            float ballCenter = y + BALL_SIZE / 2;
            float hitPos = (ballCenter - paddleCenter) / (PADDLE_HEIGHT / 2);
            velocityY = hitPos * BALL_SPEED;
            
            return true;
        }
        return false;
    }
    
    SDL_Rect getRect() {
        return {(int)x, (int)y, BALL_SIZE, BALL_SIZE};
    }
};

class Game {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    GameMode currentMode;
    Paddle player1, player2;
    Ball ball;
    int score1, score2;
    Uint32 lastTime;
    AudioManager audioManager;
    int selectedMenuOption;
    
public:
    Game() : window(nullptr), renderer(nullptr), running(true),
             currentMode(MENU),
             player1(GAME_MARGIN_SIDES + 20, GAME_MARGIN_TOP + GAME_HEIGHT / 2 - PADDLE_HEIGHT / 2, false),
             player2(WINDOW_WIDTH - GAME_MARGIN_SIDES - 20 - PADDLE_WIDTH, GAME_MARGIN_TOP + GAME_HEIGHT / 2 - PADDLE_HEIGHT / 2, true),
             score1(0), score2(0), lastTime(SDL_GetTicks()), selectedMenuOption(0) {}
    
    bool init() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
            std::cout << "Error inicializando SDL: " << SDL_GetError() << std::endl;
            return false;
        }
        
        if (!audioManager.init()) {
            std::cout << "Advertencia: Audio no disponible" << std::endl;
        }
        
        window = SDL_CreateWindow("Pong Game - Menú Principal", 
                                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                WINDOW_WIDTH, WINDOW_HEIGHT, 
                                SDL_WINDOW_SHOWN);
        
        if (!window) {
            std::cout << "Error creando ventana: " << SDL_GetError() << std::endl;
            return false;
        }
        
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            std::cout << "Error creando renderer: " << SDL_GetError() << std::endl;
            return false;
        }
        
        return true;
    }
    
    void handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            
            if (currentMode == MENU) {
                handleMenuEvents(event);
            } else {
                handleGameEvents(event);
            }
        }
    }
    
    void handleMenuEvents(SDL_Event& event) {
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_UP:
                    selectedMenuOption = (selectedMenuOption - 1 + 2) % 2;
                    break;
                case SDLK_DOWN:
                    selectedMenuOption = (selectedMenuOption + 1) % 2;
                    break;
                case SDLK_RETURN:
                    switch (selectedMenuOption) {
                        case 0: // Multijugador
                            currentMode = MULTIPLAYER;
                            player2.isAI = false;
                            resetGame();
                            SDL_SetWindowTitle(window, "Pong - Multijugador");
                            break;
                        case 1: // Vs IA
                            currentMode = SINGLE_PLAYER;
                            player2.isAI = true;
                            resetGame();
                            SDL_SetWindowTitle(window, "Pong - Vs IA");
                            break;
                    }
                    break;
                case SDLK_ESCAPE:
                    running = false;
                    break;
            }
        }
    }
    
    void handleGameEvents(SDL_Event& event) {
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                currentMode = MENU;
                SDL_SetWindowTitle(window, "Pong Game - Menú Principal");
            } else if (event.key.keysym.sym == SDLK_m) {
                audioManager.toggleMusic();
            } else if (event.key.keysym.sym == SDLK_PLUS || event.key.keysym.sym == SDLK_EQUALS) {
                audioManager.increaseVolume();
            } else if (event.key.keysym.sym == SDLK_MINUS) {
                audioManager.decreaseVolume();
            }
        }
    }
    
    void resetGame() {
        score1 = 0;
        score2 = 0;
        player1.x = GAME_MARGIN_SIDES + 20;
        player1.y = GAME_MARGIN_TOP + GAME_HEIGHT / 2 - PADDLE_HEIGHT / 2;
        player2.x = WINDOW_WIDTH - GAME_MARGIN_SIDES - 20 - PADDLE_WIDTH;
        player2.y = GAME_MARGIN_TOP + GAME_HEIGHT / 2 - PADDLE_HEIGHT / 2;
        ball.reset();
    }
    
    void update() {
        if (currentMode == MENU) {
            return; // No hay lógica de juego en el menú
        }
        
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        
        // Input para jugadores
        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        
        // Jugador 1 (siempre humano - W/S)
        player1.update(deltaTime, keystate[SDL_SCANCODE_W], keystate[SDL_SCANCODE_S]);
        
        // Jugador 2 depende del modo
        if (currentMode == SINGLE_PLAYER) {
            // IA
            player2.updateAI(deltaTime, ball.y + BALL_SIZE / 2, ball.velocityX);
        } else if (currentMode == MULTIPLAYER) {
            // Segundo jugador humano (Flechas)
            player2.update(deltaTime, keystate[SDL_SCANCODE_UP], keystate[SDL_SCANCODE_DOWN]);
        }
        
        // Actualizar pelota
        ball.update(deltaTime);
        
        // Colisiones con paletas
        ball.checkCollision(player1);
        ball.checkCollision(player2);
        
        // Verificar puntuación (cuando la pelota sale del área de juego)
        if (ball.x < GAME_MARGIN_SIDES) {
            score2++;
            ball.reset();
            if (currentMode == SINGLE_PLAYER) {
                std::cout << "Jugador: " << score1 << " - IA: " << score2 << std::endl;
            } else {
                std::cout << "Jugador 1: " << score1 << " - Jugador 2: " << score2 << std::endl;
            }
        }
        if (ball.x > WINDOW_WIDTH - GAME_MARGIN_SIDES) {
            score1++;
            ball.reset();
            if (currentMode == SINGLE_PLAYER) {
                std::cout << "Jugador: " << score1 << " - IA: " << score2 << std::endl;
            } else {
                std::cout << "Jugador 1: " << score1 << " - Jugador 2: " << score2 << std::endl;
            }
        }
    }
    
    void render() {
        // Limpiar pantalla
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        if (currentMode == MENU) {
            renderMenu();
        } else {
            renderGame();
        }
        
        SDL_RenderPresent(renderer);
    }
    
    void renderMenu() {
        // Fondo del menú
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        // Título simple y claro
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        drawSimpleTitle();
        
        // Solo 2 opciones principales
        const char* options[] = {
            "MULTIJUGADOR",
            "JUGAR vs IA"
        };
        
        int startY = WINDOW_HEIGHT / 2 - 50;
        
        for (int i = 0; i < 2; i++) {
            bool selected = (i == selectedMenuOption);
            drawSimpleMenuOption(options[i], i, WINDOW_WIDTH / 2 - 120, startY + i * 60, selected); // Más a la izquierda para compensar el espaciado
        }
        
        // Instrucciones simples
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        drawSimpleText("Flechas: Navegar", WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT - 80);
        drawSimpleText("ENTER: Seleccionar", WINDOW_WIDTH / 2 - 90, WINDOW_HEIGHT - 60);
        drawSimpleText("ESC: Salir", WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT - 40);
    }
    
    void drawSimpleTitle() {
        // Título "PONG" simple y grande
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        
        // P
        SDL_Rect P[] = {
            {WINDOW_WIDTH / 2 - 120, 100, 25, 5},   // Top
            {WINDOW_WIDTH / 2 - 120, 100, 5, 60},   // Left
            {WINDOW_WIDTH / 2 - 120, 125, 20, 5},   // Middle
            {WINDOW_WIDTH / 2 - 100, 100, 5, 30}    // Right top
        };
        for (int i = 0; i < 4; i++) SDL_RenderFillRect(renderer, &P[i]);
        
        // O
        SDL_Rect O[] = {
            {WINDOW_WIDTH / 2 - 80, 100, 25, 5},    // Top
            {WINDOW_WIDTH / 2 - 80, 155, 25, 5},    // Bottom
            {WINDOW_WIDTH / 2 - 80, 100, 5, 60},    // Left
            {WINDOW_WIDTH / 2 - 60, 100, 5, 60}     // Right
        };
        for (int i = 0; i < 4; i++) SDL_RenderFillRect(renderer, &O[i]);
        
        // N
        SDL_Rect N[] = {
            {WINDOW_WIDTH / 2 - 40, 100, 5, 60},    // Left
            {WINDOW_WIDTH / 2 - 20, 100, 5, 60},    // Right
            {WINDOW_WIDTH / 2 - 40, 110, 25, 5}     // Diagonal (simplified)
        };
        for (int i = 0; i < 3; i++) SDL_RenderFillRect(renderer, &N[i]);
        
        // G
        SDL_Rect G[] = {
            {WINDOW_WIDTH / 2 + 20, 100, 25, 5},    // Top
            {WINDOW_WIDTH / 2 + 20, 155, 25, 5},    // Bottom
            {WINDOW_WIDTH / 2 + 20, 100, 5, 60},    // Left
            {WINDOW_WIDTH / 2 + 40, 125, 5, 35},    // Right bottom
            {WINDOW_WIDTH / 2 + 35, 125, 10, 5}     // Middle right
        };
        for (int i = 0; i < 5; i++) SDL_RenderFillRect(renderer, &G[i]);
    }
    
    void drawSimpleMenuOption(const char* text, int index, int x, int y, bool selected) {
        // Fondo para opción seleccionada
        if (selected) {
            SDL_SetRenderDrawColor(renderer, 0, 100, 200, 255);
            SDL_Rect background = {x - 20, y - 10, 320, 40}; // Más ancho para el espaciado
            SDL_RenderFillRect(renderer, &background);
            
            // Flecha indicadora simple
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_Rect arrow = {x - 15, y + 10, 10, 5};
            SDL_RenderFillRect(renderer, &arrow);
        }
        
        // Color del texto
        SDL_SetRenderDrawColor(renderer, selected ? 255 : 180, selected ? 255 : 180, selected ? 255 : 180, 255);
        
        // Dibujar texto usando la función mejorada
        drawMenuText(text, x, y);
    }
    
    void drawMenuText(const char* text, int x, int y) {
        // Sistema básico para dibujar texto carácter por carácter
        int startX = x;
        int charWidth = 12;
        int charHeight = 20;
        int charSpacing = 16; // Más espacio entre caracteres
        
        for (int i = 0; text[i] != '\0' && i < 25; i++) {
            char c = text[i];
            int charX = startX + i * charSpacing;
            
            // Dibujar cada carácter usando rectángulos
            switch (c) {
                case 'M':
                    // M
                    drawChar_M(charX, y, charWidth, charHeight);
                    break;
                case 'U':
                    // U
                    drawChar_U(charX, y, charWidth, charHeight);
                    break;
                case 'L':
                    // L
                    drawChar_L(charX, y, charWidth, charHeight);
                    break;
                case 'T':
                    // T
                    drawChar_T(charX, y, charWidth, charHeight);
                    break;
                case 'I':
                    // I
                    drawChar_I(charX, y, charWidth, charHeight);
                    break;
                case 'J':
                    // J
                    drawChar_J(charX, y, charWidth, charHeight);
                    break;
                case 'G':
                    // G
                    drawChar_G(charX, y, charWidth, charHeight);
                    break;
                case 'A':
                    // A
                    drawChar_A(charX, y, charWidth, charHeight);
                    break;
                case 'R':
                    // R
                    drawChar_R(charX, y, charWidth, charHeight);
                    break;
                case 'O':
                    // O
                    drawChar_O(charX, y, charWidth, charHeight);
                    break;
                case 'D':
                    // D
                    drawChar_D(charX, y, charWidth, charHeight);
                    break;
                case 'v':
                case 'V':
                    // V
                    drawChar_V(charX, y, charWidth, charHeight);
                    break;
                case 's':
                case 'S':
                    // S
                    drawChar_S(charX, y, charWidth, charHeight);
                    break;
                case ' ':
                    // Espacio - no dibujar nada
                    break;
                default:
                    // Para caracteres no implementados, dibujar un rectángulo simple
                    SDL_Rect defaultChar = {charX + 2, y + 5, charWidth - 4, charHeight - 10};
                    SDL_RenderFillRect(renderer, &defaultChar);
                    break;
            }
        }
    }
    
    // Funciones para dibujar caracteres individuales
    void drawChar_M(int x, int y, int w, int h) {
        SDL_Rect parts[] = {
            {x, y, 3, h},           // Left vertical
            {x + w - 3, y, 3, h},   // Right vertical
            {x + 3, y, w/3, 3},     // Top left diagonal
            {x + w/2, y, w/3, 3}    // Top right diagonal
        };
        for (int i = 0; i < 4; i++) SDL_RenderFillRect(renderer, &parts[i]);
    }
    
    void drawChar_U(int x, int y, int w, int h) {
        SDL_Rect parts[] = {
            {x, y, 3, h - 3},       // Left vertical
            {x + w - 3, y, 3, h - 3}, // Right vertical
            {x, y + h - 3, w, 3}    // Bottom horizontal
        };
        for (int i = 0; i < 3; i++) SDL_RenderFillRect(renderer, &parts[i]);
    }
    
    void drawChar_L(int x, int y, int w, int h) {
        SDL_Rect parts[] = {
            {x, y, 3, h},           // Vertical
            {x, y + h - 3, w, 3}    // Bottom horizontal
        };
        for (int i = 0; i < 2; i++) SDL_RenderFillRect(renderer, &parts[i]);
    }
    
    void drawChar_T(int x, int y, int w, int h) {
        SDL_Rect parts[] = {
            {x, y, w, 3},           // Top horizontal
            {x + w/2 - 1, y, 3, h} // Center vertical
        };
        for (int i = 0; i < 2; i++) SDL_RenderFillRect(renderer, &parts[i]);
    }
    
    void drawChar_I(int x, int y, int w, int h) {
        SDL_Rect parts[] = {
            {x, y, w, 3},           // Top
            {x + w/2 - 1, y, 3, h}, // Center vertical
            {x, y + h - 3, w, 3}    // Bottom
        };
        for (int i = 0; i < 3; i++) SDL_RenderFillRect(renderer, &parts[i]);
    }
    
    void drawChar_J(int x, int y, int w, int h) {
        SDL_Rect parts[] = {
            {x + w - 3, y, 3, h - 3}, // Right vertical
            {x, y + h - 3, w, 3},     // Bottom
            {x, y + h - 6, 3, 6}      // Bottom left curve
        };
        for (int i = 0; i < 3; i++) SDL_RenderFillRect(renderer, &parts[i]);
    }
    
    void drawChar_G(int x, int y, int w, int h) {
        SDL_Rect parts[] = {
            {x, y, w, 3},           // Top
            {x, y, 3, h},           // Left vertical
            {x, y + h - 3, w, 3},   // Bottom
            {x + w/2, y + h/2, w/2, 3}, // Middle right
            {x + w - 3, y + h/2, 3, h/2} // Right vertical (bottom half)
        };
        for (int i = 0; i < 5; i++) SDL_RenderFillRect(renderer, &parts[i]);
    }
    
    void drawChar_A(int x, int y, int w, int h) {
        SDL_Rect parts[] = {
            {x, y + 3, 3, h - 3},   // Left vertical
            {x + w - 3, y + 3, 3, h - 3}, // Right vertical
            {x, y, w, 3},           // Top
            {x, y + h/2, w, 3}      // Middle horizontal
        };
        for (int i = 0; i < 4; i++) SDL_RenderFillRect(renderer, &parts[i]);
    }
    
    void drawChar_R(int x, int y, int w, int h) {
        SDL_Rect parts[] = {
            {x, y, 3, h},           // Left vertical
            {x, y, w - 3, 3},       // Top
            {x, y + h/2 - 1, w - 3, 3}, // Middle
            {x + w - 3, y, 3, h/2 + 1}, // Right top
            {x + w/2, y + h/2, w/2 - 3, h/2} // Diagonal
        };
        for (int i = 0; i < 5; i++) SDL_RenderFillRect(renderer, &parts[i]);
    }
    
    void drawChar_O(int x, int y, int w, int h) {
        SDL_Rect parts[] = {
            {x, y, w, 3},           // Top
            {x, y + h - 3, w, 3},   // Bottom
            {x, y, 3, h},           // Left
            {x + w - 3, y, 3, h}    // Right
        };
        for (int i = 0; i < 4; i++) SDL_RenderFillRect(renderer, &parts[i]);
    }
    
    void drawChar_D(int x, int y, int w, int h) {
        SDL_Rect parts[] = {
            {x, y, 3, h},           // Left vertical
            {x, y, w - 3, 3},       // Top
            {x, y + h - 3, w - 3, 3}, // Bottom
            {x + w - 3, y + 3, 3, h - 6} // Right vertical
        };
        for (int i = 0; i < 4; i++) SDL_RenderFillRect(renderer, &parts[i]);
    }
    
    void drawChar_V(int x, int y, int w, int h) {
        SDL_Rect parts[] = {
            {x, y, 3, h - 5},       // Left diagonal (simplified)
            {x + w - 3, y, 3, h - 5}, // Right diagonal (simplified)
            {x + w/2 - 1, y + h - 5, 3, 5} // Bottom point
        };
        for (int i = 0; i < 3; i++) SDL_RenderFillRect(renderer, &parts[i]);
    }
    
    void drawChar_S(int x, int y, int w, int h) {
        SDL_Rect parts[] = {
            {x, y, w, 3},           // Top
            {x, y, 3, h/2},         // Left top
            {x, y + h/2 - 1, w, 3}, // Middle
            {x + w - 3, y + h/2, 3, h/2}, // Right bottom
            {x, y + h - 3, w, 3}    // Bottom
        };
        for (int i = 0; i < 5; i++) SDL_RenderFillRect(renderer, &parts[i]);
    }
    
    void drawSimpleText(const char* text, int x, int y) {
        (void)text; // Suprimir warning
        
        // Texto pequeño simple
        SDL_Rect textRect = {x, y, 100, 15};
        SDL_RenderFillRect(renderer, &textRect);
    }
    
    void drawTextLine(const char* text, int x, int y) {
        // Función simple para mostrar texto como rectángulos
        int length = strlen(text);
        for (int i = 0; i < length && i < 20; i++) {
            SDL_Rect charRect = {x + i * 8, y, 6, 12};
            SDL_RenderFillRect(renderer, &charRect);
        }
    }
    
    void drawSmallText(const char* text, int x, int y) {
        // Texto más pequeño para etiquetas
        int length = strlen(text);
        for (int i = 0; i < length && i < 15; i++) {
            SDL_Rect charRect = {x + i * 6, y, 4, 8};
            SDL_RenderFillRect(renderer, &charRect);
        }
    }
    
    void renderGame() {
        // Fondo negro
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        // Dibujar marco del área de juego
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_Rect gameArea = {GAME_MARGIN_SIDES, GAME_MARGIN_TOP, GAME_WIDTH, GAME_HEIGHT};
        SDL_RenderDrawRect(renderer, &gameArea);
        
        // Fondo del área de juego
        SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
        SDL_RenderFillRect(renderer, &gameArea);
        
        // Dibujar línea central dentro del área de juego
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        int centerX = WINDOW_WIDTH / 2;
        for (int i = GAME_MARGIN_TOP; i < GAME_MARGIN_TOP + GAME_HEIGHT; i += 15) {
            SDL_Rect lineSegment = {centerX - 1, i, 2, 8};
            SDL_RenderFillRect(renderer, &lineSegment);
        }
        
        // Dibujar paletas con efecto 3D
        drawPaddle(player1.getRect(), true);  // Jugador 1
        drawPaddle(player2.getRect(), false); // Jugador 2
        
        // Dibujar pelota con efecto
        drawBall(ball.getRect());
        
        // Mostrar puntuación en la parte superior
        drawScoreBoard();
        
        // Mostrar estado de la música si está activa
        if (audioManager.isMusicEnabled() && audioManager.isMusicPlaying()) {
            drawMusicIndicator();
        }
        
        // Mostrar controles en la parte inferior
        renderGameInstructions();
    }
    
    void drawPaddle(SDL_Rect paddleRect, bool isPlayer1) {
        // Efecto 3D para las paletas
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &paddleRect);
        
        // Borde oscuro para efecto 3D
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderDrawRect(renderer, &paddleRect);
        
        // Líneas decorativas en el medio
        int centerY = paddleRect.y + paddleRect.h / 2;
        for (int i = -1; i <= 1; i++) {
            SDL_Rect line = {paddleRect.x + 2, centerY + i * 6, paddleRect.w - 4, 1};
            SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
            SDL_RenderFillRect(renderer, &line);
        }
        
        // Indicador de jugador
        SDL_SetRenderDrawColor(renderer, isPlayer1 ? 100 : 255, isPlayer1 ? 255 : 100, 100, 255);
        SDL_Rect indicator = {
            isPlayer1 ? paddleRect.x - 8 : paddleRect.x + paddleRect.w + 3,
            paddleRect.y + paddleRect.h / 2 - 3,
            5, 6
        };
        SDL_RenderFillRect(renderer, &indicator);
    }
    
    void drawBall(SDL_Rect ballRect) {
        // Pelota con efecto brillante
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &ballRect);
        
        // Borde
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderDrawRect(renderer, &ballRect);
        
        // Punto brillante en el centro
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect highlight = {
            ballRect.x + ballRect.w / 2 - 1,
            ballRect.y + ballRect.h / 2 - 1,
            2, 2
        };
        SDL_RenderFillRect(renderer, &highlight);
    }
    
    void drawScoreBoard() {
        // Fondo del marcador en la parte superior
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_Rect scoreBackground = {WINDOW_WIDTH / 2 - 100, 15, 200, 50};
        SDL_RenderFillRect(renderer, &scoreBackground);
        
        // Marco del marcador
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderDrawRect(renderer, &scoreBackground);
        
        // Separador central
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect separator = {WINDOW_WIDTH / 2 - 1, 20, 2, 40};
        SDL_RenderFillRect(renderer, &separator);
        
        // Etiquetas de jugadores más pequeñas
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        if (currentMode == SINGLE_PLAYER) {
            drawSmallText("JUGADOR", WINDOW_WIDTH / 2 - 90, 25);
            drawSmallText("IA", WINDOW_WIDTH / 2 + 50, 25);
        } else {
            drawSmallText("P1", WINDOW_WIDTH / 2 - 70, 25);
            drawSmallText("P2", WINDOW_WIDTH / 2 + 50, 25);
        }
        
        // Puntuación jugador 1 (izquierda)
        SDL_SetRenderDrawColor(renderer, 100, 255, 100, 255);
        drawLargeDigit(score1, WINDOW_WIDTH / 2 - 60, 35);
        
        // Puntuación jugador 2 (derecha)  
        SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);
        drawLargeDigit(score2, WINDOW_WIDTH / 2 + 30, 35);
    }
    
    void drawLargeDigit(int number, int x, int y) {
        // Convertir número a string y dibujar cada dígito
        if (number > 9) {
            drawDigitalDigit(number / 10, x - 15, y);
            drawDigitalDigit(number % 10, x + 5, y);
        } else {
            drawDigitalDigit(number, x, y);
        }
    }
    
    void drawDigitalDigit(int digit, int x, int y) {
        // Patrón simplificado para dígitos 0-9
        bool segments[7]; // 7 segmentos: top, top-right, bottom-right, bottom, bottom-left, top-left, middle
        
        // Inicializar todos los segmentos como falsos
        for (int i = 0; i < 7; i++) segments[i] = false;
        
        // Configurar segmentos según el dígito
        switch (digit) {
            case 0: segments[0] = segments[1] = segments[2] = segments[3] = segments[4] = segments[5] = true; break;
            case 1: segments[1] = segments[2] = true; break;
            case 2: segments[0] = segments[1] = segments[6] = segments[4] = segments[3] = true; break;
            case 3: segments[0] = segments[1] = segments[6] = segments[2] = segments[3] = true; break;
            case 4: segments[5] = segments[6] = segments[1] = segments[2] = true; break;
            case 5: segments[0] = segments[5] = segments[6] = segments[2] = segments[3] = true; break;
            case 6: segments[0] = segments[5] = segments[4] = segments[3] = segments[2] = segments[6] = true; break;
            case 7: segments[0] = segments[1] = segments[2] = true; break;
            case 8: for (int i = 0; i < 7; i++) segments[i] = true; break;
            case 9: segments[0] = segments[1] = segments[2] = segments[3] = segments[5] = segments[6] = true; break;
        }
        
        // Dibujar segmentos activos
        if (segments[0]) { // top
            SDL_Rect seg = {x, y, 15, 2};
            SDL_RenderFillRect(renderer, &seg);
        }
        if (segments[1]) { // top-right
            SDL_Rect seg = {x + 13, y, 2, 8};
            SDL_RenderFillRect(renderer, &seg);
        }
        if (segments[2]) { // bottom-right
            SDL_Rect seg = {x + 13, y + 10, 2, 8};
            SDL_RenderFillRect(renderer, &seg);
        }
        if (segments[3]) { // bottom
            SDL_Rect seg = {x, y + 16, 15, 2};
            SDL_RenderFillRect(renderer, &seg);
        }
        if (segments[4]) { // bottom-left
            SDL_Rect seg = {x, y + 10, 2, 8};
            SDL_RenderFillRect(renderer, &seg);
        }
        if (segments[5]) { // top-left
            SDL_Rect seg = {x, y, 2, 8};
            SDL_RenderFillRect(renderer, &seg);
        }
        if (segments[6]) { // middle
            SDL_Rect seg = {x, y + 8, 15, 2};
            SDL_RenderFillRect(renderer, &seg);
        }
    }
    
    void drawMusicIndicator() {
        // Indicador de música activa en la esquina superior derecha
        SDL_SetRenderDrawColor(renderer, 100, 255, 100, 255);
        SDL_Rect musicIcon = {WINDOW_WIDTH - 30, 10, 20, 15};
        SDL_RenderDrawRect(renderer, &musicIcon);
        
        // Notas musicales simuladas
        SDL_Rect note1 = {WINDOW_WIDTH - 28, 12, 3, 3};
        SDL_Rect note2 = {WINDOW_WIDTH - 20, 15, 3, 3};
        SDL_Rect note3 = {WINDOW_WIDTH - 12, 12, 3, 3};
        SDL_RenderFillRect(renderer, &note1);
        SDL_RenderFillRect(renderer, &note2);
        SDL_RenderFillRect(renderer, &note3);
    }
    
    void renderGameInstructions() {
        // Fondo para las instrucciones en la parte inferior
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_Rect instructionBg = {10, GAME_MARGIN_TOP + GAME_HEIGHT + 10, WINDOW_WIDTH - 20, 40};
        SDL_RenderFillRect(renderer, &instructionBg);
        
        // Marco
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderDrawRect(renderer, &instructionBg);
        
        // Instrucciones compactas según el modo
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        int textY = GAME_MARGIN_TOP + GAME_HEIGHT + 20;
        
        if (currentMode == SINGLE_PLAYER) {
            drawSmallText("W/S: Mover", 20, textY);
            drawSmallText("M: Musica", 150, textY);
            drawSmallText("+/-: Volumen", 250, textY);
            drawSmallText("ESC: Menu", 380, textY);
        } else {
            drawSmallText("P1: W/S", 20, textY);
            drawSmallText("P2: Flechas", 120, textY);
            drawSmallText("M: Musica", 250, textY);
            drawSmallText("ESC: Menu", 350, textY);
        }
    }
    
    void run() {
        while (running) {
            handleEvents();
            update();
            render();
            SDL_Delay(16); // ~60 FPS
        }
    }
    
    void cleanup() {
        audioManager.cleanup();
        if (renderer) {
            SDL_DestroyRenderer(renderer);
        }
        if (window) {
            SDL_DestroyWindow(window);
        }
        SDL_Quit();
    }
};

int main(int argc, char* argv[]) {
    (void)argc; // Suprimir warning de parámetro no usado
    (void)argv; // Suprimir warning de parámetro no usado
    
    Game game;
    
    if (!game.init()) {
        return -1;
    }
    
    std::cout << "¡Bienvenido a Pong!" << std::endl;
    std::cout << "=== MENÚ PRINCIPAL ===" << std::endl;
    std::cout << "Usa las flechas para navegar" << std::endl;
    std::cout << "ENTER para seleccionar" << std::endl;
    std::cout << "ESC para salir" << std::endl;
    std::cout << std::endl;
    std::cout << "=== CONTROLES DE JUEGO ===" << std::endl;
    std::cout << "Modo IA: W/S para mover tu paleta" << std::endl;
    std::cout << "Multijugador: Jugador 1 (W/S), Jugador 2 (Flechas)" << std::endl;
    std::cout << "M: Activar/desactivar música" << std::endl;
    std::cout << "+/-: Subir/bajar volumen" << std::endl;
    std::cout << "ESC: Volver al menú" << std::endl;
    std::cout << std::endl;
    std::cout << "♪ Música: Funk It - Dyalla" << std::endl;
    
    game.run();
    game.cleanup();
    
    return 0;
}
