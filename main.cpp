#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <cmath>
#include <cstdlib>
#include <vector>

enum BattleState {
    START_IDLE,
    TACKLE_MOVE,
    ENEMY_HIT,
    TACKLE_RETURN,
    FINAL_IDLE
};

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* win;
    SDL_Renderer* ren;
    SDL_CreateWindowAndRenderer("Pokemon Battle POC", 800, 600, 0, &win, &ren);

    SDL_Texture* bgTex = IMG_LoadTexture(ren, "res/BattleBack.png");

    SDL_Texture* uiTex = nullptr;
    SDL_Surface* uiSurf = IMG_Load("res/Battle.png");
    if (uiSurf) {
        SDL_SetSurfaceColorKey(uiSurf, true, SDL_MapSurfaceRGB(uiSurf, 255, 255, 255));
        uiTex = SDL_CreateTextureFromSurface(ren, uiSurf);
        SDL_DestroySurface(uiSurf);
    }

    SDL_Texture* playerTex = nullptr;
    SDL_Surface* pSurf = IMG_Load("res/Pokemon.png");
    if (pSurf) {
        SDL_SetSurfaceColorKey(pSurf, true, SDL_MapSurfaceRGB(pSurf, 200, 200, 168));
        playerTex = SDL_CreateTextureFromSurface(ren, pSurf);
        SDL_DestroySurface(pSurf);
    }

    SDL_Texture* enemyTex = nullptr;
    SDL_Surface* eSurf = IMG_Load("res/Pokemon.png");
    if (eSurf) {
        SDL_SetSurfaceColorKey(eSurf, true, SDL_MapSurfaceRGB(eSurf, 200, 200, 168));
        enemyTex = SDL_CreateTextureFromSurface(ren, eSurf);
        SDL_DestroySurface(eSurf);
    }

    SDL_Texture* enemyText = nullptr;
    SDL_Surface* etSurf = IMG_Load("res/Pokemon.png");
    if (etSurf) {
        SDL_SetSurfaceColorKey(etSurf, true, SDL_MapSurfaceRGB(etSurf, 152, 152, 184));
        enemyText = SDL_CreateTextureFromSurface(ren, etSurf);
        SDL_DestroySurface(etSurf);
    }

    SDL_Texture* playerText = nullptr;
    SDL_Surface* ptSurf = IMG_Load("res/Pokemon.png");
    if (ptSurf) {
        SDL_SetSurfaceColorKey(ptSurf, true, SDL_MapSurfaceRGB(ptSurf, 152, 152, 184));
        playerText = SDL_CreateTextureFromSurface(ren, ptSurf);
        SDL_DestroySurface(ptSurf);
    }

    SDL_FRect bgSrc = { 268.0f, 4.0f, 216.0f, 112.0f };
    SDL_FRect bgDst = { 0.0f, -80.0f, 800.0f, 500.0f };
    SDL_FRect msgBoxSrc = { 297.0f, 56.0f, 240.0f, 48.0f };
    SDL_FRect msgBoxDst = { 0.0f, 420.0f, 800.0f, 180.0f };
    SDL_FRect enemyHpSrc = { 3.0f, 3.0f, 100.0f, 30.0f };
    SDL_FRect enemyHpDst = { 40.0f, 40.0f, 280.0f, 84.0f };
    SDL_FRect playerHpSrc = { 3.0f, 44.0f, 104.0f, 37.0f };
    SDL_FRect playerHpDst = { 460.0f, 300.0f, 300.0f, 106.0f };

    SDL_FRect hpBarGreenSrc = { 117.0f, 9.0f, 9.0f, 3.0f };
    SDL_FRect hpBarBlackSrc = { 117.0f, 22.0f, 9.0f, 3.0f };

    SDL_FRect playerDstBase = { 68.0f, 234.0f, 190.0f, 190.0f };
    SDL_FRect playerSrc = { 405.0f, 55.0f, 50.0f, 45.0f };
    SDL_FRect playerTextSrc = { 402.0f, 12.0f, 57.0f, 14.0f };
    SDL_FRect playerTextDst = { 499.0f, 312.0f, 114.0f, 28.0f };
    SDL_FRect EnemyTextSrc = { 1182.0f, 12.0f, 57.0f, 14.0f };
    SDL_FRect EnemyTextDst = { 55.0f, 53.0f, 114.0f, 28.0f };
    SDL_FRect menuSrc = { 120.0f, 4.0f, 220.0f, 116.0f };
    SDL_FRect menuDst = { 472.0f, 432.0f, 220.0f, 116.0f };
    SDL_FRect digit4Src = { 92.0f, 206.0f, 61.0f, 61.0f };
    SDL_FRect digit6Src = { 199.0f, 246.0f, 45.0f, 21.0f };
    SDL_FRect enemyLvlDst = { 145.0f, 52.0f, 24.0f, 24.0f };
    SDL_FRect playerLvlDst = { 589.0f, 318.0f, 24.0f, 24.0f };
    SDL_FRect enemyDstBase = { 483.0f, 64.0f, 160.0f, 160.0f };
    SDL_FRect enemySrc = { 1185.0f, 55.0f, 50.0f, 45.0f };

    BattleState currentState = START_IDLE;
    float stateTimer = 0.0f;
    float idleTimer = 0.0f;
    float enemyHP = 1.0f;
    float enemyYOffset = 0.0f;
    bool enemyVisible = true;
    bool running = true;
    SDL_Event ev;

    while (running) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) running = false;
        }

        SDL_FRect playerDst = playerDstBase;
        SDL_FRect enemyDst = enemyDstBase;
        enemyDst.y += enemyYOffset;
        stateTimer += 0.016f;

        switch (currentState) {
            case START_IDLE:
                idleTimer += 0.05f;
                playerDst.y += std::sin(idleTimer) * 4.0f;
                if (stateTimer >= 1.0f) {
                    currentState = TACKLE_MOVE;
                    stateTimer = 0.0f;
                }
                break;

            case TACKLE_MOVE:
                playerDst.x += (stateTimer * 1200.0f);
                playerDst.y -= (stateTimer * 400.0f);
                if (playerDst.x > 380.0f) {
                    currentState = ENEMY_HIT;
                    stateTimer = 0.0f;
                }
                break;

            case ENEMY_HIT:
                enemyVisible = (int(stateTimer * 100) % 2 == 0);
                enemyDst.x += (std::rand() % 10) - 5;
                if (stateTimer > 0.6f) {
                    enemyVisible = true;
                    currentState = TACKLE_RETURN;
                    stateTimer = 0.0f;
                }
                break;

            case TACKLE_RETURN:
                playerDst.x = 380.0f - (stateTimer * 1000.0f);
                playerDst.y = (playerDstBase.y - 60.0f) + (stateTimer * 200.0f);

                    currentState = FINAL_IDLE;
                    stateTimer = 0.0f;

                break;

            case FINAL_IDLE:
                idleTimer += 0.05f;
                playerDst.y += std::sin(idleTimer) * 4.0f;
                if (enemyHP > 0.0f) {
                    enemyHP -= 0.005f;
                } else {
                    enemyHP = 0.0f;
                    if (enemyYOffset < 335.0f) {
                        enemyYOffset += 5.0f;
                    }
                }
                break;
        }

        SDL_RenderClear(ren);
        SDL_RenderTexture(ren, bgTex, &bgSrc, &bgDst);
        SDL_RenderTexture(ren, playerTex, &playerSrc, &playerDst);
        if (enemyVisible) {
            SDL_RenderTexture(ren, enemyTex, &enemySrc, &enemyDst);
        }
        SDL_RenderTexture(ren, uiTex, &enemyHpSrc, &enemyHpDst);

        SDL_FRect hpBarDst = { 148.6f, 88.2f, 136.4f, 8.4f};
        SDL_FRect hpBlackBarDst = { 141.6f, 88.2f, 146.4f, 15.4f };

        SDL_RenderTexture(ren, uiTex, &hpBarBlackSrc, &hpBlackBarDst);
        if (enemyHP > 0.0f) {
            SDL_FRect hpGreenDst = hpBarDst;
            hpGreenDst.w *= enemyHP;
            SDL_RenderTexture(ren, uiTex, &hpBarGreenSrc, &hpGreenDst);
        }

        SDL_RenderTexture(ren, uiTex, &playerHpSrc, &playerHpDst);
        SDL_RenderTexture(ren, uiTex, &msgBoxSrc, &msgBoxDst);
        SDL_RenderTexture(ren, uiTex, &menuSrc, &menuDst);
        SDL_RenderTexture(ren, playerText, &playerTextSrc, &playerTextDst);
        SDL_RenderTexture(ren, enemyText, &EnemyTextSrc, &EnemyTextDst);
        SDL_RenderTexture(ren, uiTex, &digit4Src, &enemyLvlDst);
        SDL_RenderTexture(ren, uiTex, &digit6Src, &playerLvlDst);
        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(bgTex);
    SDL_DestroyTexture(uiTex);
    SDL_DestroyTexture(playerTex);
    SDL_DestroyTexture(enemyTex);
    SDL_DestroyTexture(enemyText);
    SDL_DestroyTexture(playerText);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
