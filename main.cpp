#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <unordered_map>
#include <string>

enum BattleState {
    ATTACK_SELECT,
    START_IDLE,
    TACKLE_MOVE,
    EMBER_MOVE,
    ENEMY_HIT,
    PLAYER_HIT,
    AI_ATTACK,
    MESSAGE_PLAYER_ATTACK,
    MESSAGE_ENEMY_ATTACK,
    FINAL_IDLE
};

enum AttackType {
    ATTACK_TACKLE,
    ATTACK_EMBER
};

std::unordered_map<char, SDL_FRect> fontMap;

void InitFont() {
    fontMap.clear();

    // --- Uppercase Alphabet Configuration ---
    float upperX = 170.0f;
    float upperY = 122.0f;
    float upperW = 6.0f;
    float upperH = 11.0f;
    float upperStepX = 7.0f;

    std::string upperRow = "ABCDEFGHIJKLMNOPQRSTUVWXYZ.,";

    for (int i = 0; i < (int)upperRow.length(); ++i) {
        char c = upperRow[i];
        fontMap[c] = {
            upperX + (i * upperStepX),
            upperY,
            upperW,
            upperH
        };
    }

    // --- Numbers Configuration ---
    float numX = 170.0f;
    float numY = 157.0f;
    float numW = 6.0f;
    float numH = 10.0f;
    float numStepX = 7.0f;

    std::string numRow = "0123456789";

    for (int i = 0; i < (int)numRow.length(); ++i) {
        char c = numRow[i];
        fontMap[c] = {
            numX + (i * numStepX),
            numY,
            numW,
            numH
        };
    }
}

// Helper function to render text using the dictionary
void RenderText(SDL_Renderer* ren, SDL_Texture* tex, const std::string& text, float x, float y, float scale) {
    float currentX = x;
    for (char c : text) {
        if (fontMap.count(c)) {
            SDL_FRect src = fontMap[c];
            SDL_FRect dst = { currentX, y, src.w * scale, src.h * scale };
            SDL_RenderTexture(ren, tex, &src, &dst);

            currentX += (src.w + 1.0f) * scale;
        } else if (c == ' ') {
            currentX += (6.0f * scale);
        }
    }
}

// Renders the Ember animation, cycling through 5 frames moving downwards on the sprite sheet
void PlayEmberAnimation(SDL_Renderer* ren, SDL_Texture* movesTex,SDL_FRect *Dst, float stateTimer) {
    if (!movesTex) return; // Safety check

    const int totalFrames = 5;
    const float duration = 1.0f;
    const float timePerFrame = duration / totalFrames;

    // Determine the current frame index (0 to 4) based on the state timer
    int currentFrame = static_cast<int>(stateTimer / timePerFrame);
    if (currentFrame >= totalFrames) {
        currentFrame = totalFrames - 1; // Clamp to the last frame to prevent overflow
    }

    // Define the Source Rectangle based on your specific coordinates
    SDL_FRect src;
    src.x = 242.0f;
    src.y = 1165.0f + (currentFrame * 30.0f); // Move down 30 pixels per frame
    src.w = 30.0f;
    src.h = 30.0f;

    SDL_RenderTexture(ren, movesTex, &src, Dst);
}

// Function to smoothly decrease HP over time
void UpdateHP(float& currentHP, float startHP, float targetHP, float stateTimer, float duration) {
    if (stateTimer >= duration) {
        currentHP = targetHP;
    } else {
        // Linear Interpolation: current = start + (end - start) * (progress)
        float progress = stateTimer / duration;
        currentHP = startHP + (targetHP - startHP) * progress;
    }
}
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

    SDL_Texture* TypesTex = nullptr;
    SDL_Surface* tsSurf = IMG_Load("res/Type.png");
    if (tsSurf) {
        SDL_SetSurfaceColorKey(tsSurf, true, SDL_MapSurfaceRGB(tsSurf, 152, 152, 184));
        TypesTex = SDL_CreateTextureFromSurface(ren, tsSurf);
        SDL_DestroySurface(tsSurf);
    }

    SDL_Texture* movesTex = nullptr;
    SDL_Surface* mSurf = IMG_Load("res/Moves.png");
    if (mSurf) {
        // Set the color key to black (0, 0, 0) to make the background transparent
        SDL_SetSurfaceColorKey(mSurf, true, SDL_MapSurfaceRGB(mSurf, 0, 0, 0));
        movesTex = SDL_CreateTextureFromSurface(ren, mSurf);
        SDL_DestroySurface(mSurf);
    }
    

    SDL_FRect bgSrc = { 268.0f, 4.0f, 216.0f, 112.0f };
    SDL_FRect bgDst = { 0.0f, -80.0f, 800.0f, 500.0f };
    SDL_FRect msgBoxSrc = { 293.0f, 2.0f, 250.0f, 50.0f };
    SDL_FRect msgBoxDst = { -13.0f, 415.0f, 830.0f, 184.0f };
    SDL_FRect msg2BoxSrc = { 298.0f, 55.0f, 250.0f, 50.0f };
    SDL_FRect msg2BoxDst = { 2.0f, 415.0f, 834.0f, 184.0f };
    SDL_FRect enemyHpSrc = { 3.0f, 3.0f, 100.0f, 30.0f };
    SDL_FRect enemyHpDst = { 40.0f, 40.0f, 280.0f, 89.0f };
    SDL_FRect playerHpSrc = { 3.0f, 44.0f, 104.0f, 37.0f };
    SDL_FRect playerHpDst = { 460.0f, 300.0f, 300.0f, 106.0f };

    SDL_FRect hpBarGreenSrc = { 117.0f, 9.0f, 9.0f, 3.0f };
    SDL_FRect hpBarBlackSrc = { 117.0f, 21.0f, 9.0f, 3.0f };

    SDL_FRect playerDstBase = { 68.0f, 234.0f, 190.0f, 190.0f };
    SDL_FRect playerSrc = { 405.0f, 55.0f, 50.0f, 45.0f };
    SDL_FRect playerTextSrc = { 402.0f, 12.0f, 57.0f, 14.0f };
    SDL_FRect playerTextDst = { 499.0f, 312.0f, 114.0f, 28.0f };
    SDL_FRect EnemyTextSrc = { 1182.0f, 12.0f, 57.0f, 14.0f };
    SDL_FRect EnemyTextDst = { 55.0f, 53.0f, 114.0f, 28.0f };
    SDL_FRect TypeNormalDst = { 635.0f, 515.0f, 120.0f, 55.0f };
    SDL_FRect TypeNormalSrc = { 0.0f, 0.0f, 31.0f, 15.0f };
    SDL_FRect TypeFireDst = { 635.0f, 515.0f, 120.0f, 55.0f };
    SDL_FRect TypeFireSrc = { 64.0f, 32.0f, 31.0f, 15.0f };
    SDL_FRect StatusFireDst = { 49.0f, 80.0f, 45.0f, 20.0f };
    SDL_FRect StatusFireSrc = { 5.0f, 88.0f, 21.0f, 8.0f };

    //SDL_FRect menuSrc = { 144.0f, 2.0f, 124.0f, 56.0f };
    //SDL_FRect menuDst = { 430.0f, 412.0f, 380.0f, 210.0f };
    SDL_FRect dotSrc = { 266.0f, 2.0f, 9.0f, 12.0f };
    SDL_FRect dotDst = { 30.0f, 475.0f, 20.0f, 18.0f };

    SDL_FRect enemyDstBase = { 483.0f, 64.0f, 160.0f, 160.0f };
    SDL_FRect enemySrc = { 1185.0f, 55.0f, 50.0f, 45.0f };

    BattleState currentState = ATTACK_SELECT;
    AttackType currentAttack = ATTACK_TACKLE;
    float stateTimer = 0.0f;
    float idleTimer = 0.0f;
    float enemyHP = 1.0f;
    float enemyHPAtStartOfHit = 1.0f;
    float playerHPAtStartOfHit = 1.0f;
    float playerHP = 1.0f;
    float targetEnemyHP = enemyHP;
    float targetPlayerHP = playerHP;
    float enemyYOffset = 0.0f;
    bool enemyVisible = true;
    bool playerVisible = true;
    bool running = true;
    std::string messageText;
    
    // Attack Power Points (PP) system
    int tacklePPCurrent = 30;
    int tacklePPMax = 30;
    int emberPPCurrent = 15;
    int emberPPMax = 15;
    
    float playerDamageTaken = 0.5f;
    float enemyDamageTaken = 0.35f;
    
    SDL_Event ev;
    InitFont();
    while (running) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) running = false;
            
            // Handle attack selection input
            if (currentState == ATTACK_SELECT && ev.type == SDL_EVENT_KEY_DOWN) {
                if (ev.key.key == SDLK_LEFT || ev.key.key == SDLK_A) {
                    currentAttack = ATTACK_TACKLE;
                } else if (ev.key.key == SDLK_RIGHT || ev.key.key == SDLK_D) {
                    currentAttack = ATTACK_EMBER;
                } else if (ev.key.key == SDLK_RETURN) {
                    // Confirm attack selection
                    if (currentAttack == ATTACK_TACKLE && tacklePPCurrent > 0) {
                        messageText = "CHARMANDER USES TACKLE";
                        currentState = MESSAGE_PLAYER_ATTACK;
                        stateTimer = 0.0f;
                        tacklePPCurrent--;
                    } else if (currentAttack == ATTACK_EMBER && emberPPCurrent > 0) {
                        messageText = "CHARMANDER USES EMBER";
                        currentState = MESSAGE_PLAYER_ATTACK;
                        stateTimer = 0.0f;
                        emberPPCurrent--;
                    }
                }
            }
        }

        SDL_FRect playerDst = playerDstBase;
        SDL_FRect enemyDst = enemyDstBase;
        enemyDst.y += enemyYOffset;
        stateTimer += 0.016f;

        switch (currentState) {
            case ATTACK_SELECT:
                idleTimer += 0.05f;
                playerDst.y += std::sin(idleTimer) * 4.0f;
                break;

            case START_IDLE:
                idleTimer += 0.05f;
                playerDst.y += std::sin(idleTimer) * 20.0f;
                if (stateTimer >= 0.5f) {
                    // Transition to appropriate attack
                    if (currentAttack == ATTACK_TACKLE) {
                        currentState = TACKLE_MOVE;
                    } else if (currentAttack == ATTACK_EMBER) {
                        currentState = EMBER_MOVE;
                    }
                    stateTimer = 0.0f;
                }
                break;

            case TACKLE_MOVE:
                playerDst.x += (stateTimer * 1200.0f);
                playerDst.y -= (stateTimer * 400.0f);
                if (playerDst.x > 380.0f) {
                    enemyHPAtStartOfHit = enemyHP;
                    targetEnemyHP = enemyHP - enemyDamageTaken;
                    if (targetEnemyHP <= 0.0f) targetEnemyHP = 0.0f;

                    currentState = ENEMY_HIT;
                    stateTimer = 0.0f;
                }
                break;

            case EMBER_MOVE:

                if (stateTimer >= 1.0f) {
                    enemyHPAtStartOfHit = enemyHP;
                    targetEnemyHP = enemyHP - enemyDamageTaken;
                    if (targetEnemyHP <= 0.0f) targetEnemyHP = 0.0f;

                    currentState = ENEMY_HIT;
                    stateTimer = 0.0f;
                }
                break;

            case ENEMY_HIT:
                UpdateHP(enemyHP, enemyHPAtStartOfHit, targetEnemyHP, stateTimer, 0.6f);
                enemyVisible = (int(stateTimer * 100) % 2 == 0);
                enemyDst.x += (std::rand() % 10) - 5;
                if (stateTimer > 0.6f) {
                    enemyVisible = true;
                    playerDst = playerDstBase;
                    if (targetEnemyHP <= 0.0f) {
                        currentState = FINAL_IDLE;
                    } else {
                        messageText = "CATERPIE USES TACKLE";
                        currentState = MESSAGE_ENEMY_ATTACK;
                    }
                    stateTimer = 0.0f;
                }
                break;

            case MESSAGE_PLAYER_ATTACK:
                if (stateTimer >= 1.0f) {
                    currentState = START_IDLE;
                    stateTimer = 0.0f;
                    idleTimer = 0.0f;
                }
                break;

            case MESSAGE_ENEMY_ATTACK:
                if (stateTimer >= 1.0f) {
                    currentState = AI_ATTACK;
                    stateTimer = 0.0f;
                }
                break;

            case AI_ATTACK:
                enemyDst.x -= (stateTimer * 120.0f);
                if (stateTimer > 0.5f) {
                    // We will add playerHPAtStartOfHit in the next step
                    playerHPAtStartOfHit = playerHP;
                    targetPlayerHP = playerHP - playerDamageTaken;

                    if (targetPlayerHP <= 0.0f) {
                        targetPlayerHP = 0.0f;
                    }

                    // Always transition to PLAYER_HIT to allow the animation to play
                    currentState = PLAYER_HIT;
                    stateTimer = 0.0f;
                }
                break;

            case PLAYER_HIT:
                UpdateHP(playerHP, playerHPAtStartOfHit, targetPlayerHP, stateTimer, 0.6f);
                playerVisible = (int(stateTimer * 100) % 2 == 0);
                if (stateTimer > 0.6f) {
                    playerDst = playerDstBase;
                    if (targetPlayerHP <= 0.0f) {
                        currentState = FINAL_IDLE;
                    } else {
                        currentState = ATTACK_SELECT;
                        stateTimer = 0.0f;
                        idleTimer = 0.0f;
                    }
                }
                break;

            case FINAL_IDLE:
                if (targetPlayerHP <= 0.0f) {
                    playerDst.y += 5.0f;
                }

                if (targetEnemyHP <= 0.0f) {
                    if (enemyYOffset < 335.0f) {
                        enemyYOffset += 5.0f;
                    }
                }
                break;
        }

        SDL_RenderClear(ren);
        SDL_RenderTexture(ren, bgTex, &bgSrc, &bgDst);
        if (playerVisible) {
            SDL_RenderTexture(ren, playerTex, &playerSrc, &playerDst);
        }
        if (enemyVisible) {
            SDL_RenderTexture(ren, enemyTex, &enemySrc, &enemyDst);
        }
        if (currentState == EMBER_MOVE) {
            PlayEmberAnimation(ren, movesTex, &enemyDst, stateTimer);
        }
        SDL_RenderTexture(ren, uiTex, &enemyHpSrc, &enemyHpDst);
        SDL_RenderTexture(ren, uiTex, &playerHpSrc, &playerHpDst);

        //enemy HP bar
        SDL_FRect enemyhpBarDst = { 149.0f, 89.0f, 134.0f, 12.0f};
        SDL_FRect enemyhpBlackBarDst = { 145.0f, 89.0f, 143.0f, 12.0f };

        SDL_RenderTexture(ren, uiTex, &hpBarBlackSrc, &enemyhpBlackBarDst);
        if (enemyHP > 0.0f) {
            SDL_FRect enemyhpGreenDst = enemyhpBarDst;
            enemyhpGreenDst.w *= enemyHP;
            SDL_RenderTexture(ren, uiTex, &hpBarGreenSrc, &enemyhpGreenDst);
        }

        //player HP bar
        SDL_FRect playerHpBarDst = { 598.6f, 347.0f, 135.0f, 12.0f };
        SDL_FRect playerHpBlackBarDst = { 597.0f, 347.0f, 144.0f, 12.0f };
        SDL_RenderTexture(ren, uiTex, &hpBarBlackSrc, &playerHpBlackBarDst);
        if (playerHP > 0.0f) {
            SDL_FRect playerHpGreenDst = playerHpBarDst;
            playerHpGreenDst.w *= playerHP;
            SDL_RenderTexture(ren, uiTex, &hpBarGreenSrc, &playerHpGreenDst);
        }

        SDL_FRect currentMsgBoxDst = msgBoxDst;
        SDL_FRect currentMsgBoxSrc = msgBoxSrc;
        if (currentState != ATTACK_SELECT ) {

            currentMsgBoxDst = msg2BoxDst;
            currentMsgBoxSrc = msg2BoxSrc;
        }


        SDL_RenderTexture(ren, uiTex, &currentMsgBoxSrc, &currentMsgBoxDst);
        SDL_RenderTexture(ren, playerText, &playerTextSrc, &playerTextDst);
        SDL_RenderTexture(ren, enemyText, &EnemyTextSrc, &EnemyTextDst);
        RenderText(ren, uiTex, "4", 268.0f, 53.0f, 2.3f);
        RenderText(ren, uiTex, "6", 721.0f, 315.0f, 2.3f);
        
        // Render dot and attack names only in ATTACK_SELECT
        if (currentState == ATTACK_SELECT) {
            // Render dot based on selected attack
            SDL_FRect dotDstAdjusted = dotDst;
            if (currentAttack == ATTACK_TACKLE) {
                dotDstAdjusted.x = 30.0f;  // Tackle position (left)
            } else {
                dotDstAdjusted.x = 300.0f;
            }
            SDL_RenderTexture(ren, uiTex, &dotSrc, &dotDstAdjusted);
            
            // Render type based on selected attack
            if (currentAttack == ATTACK_TACKLE) {
                SDL_RenderTexture(ren, TypesTex, &TypeNormalSrc, &TypeNormalDst);
            } else {
                SDL_RenderTexture(ren, TypesTex, &TypeFireSrc, &TypeFireDst);
            }
            


            // Display PP for selected attack
            if (currentAttack == ATTACK_TACKLE) {
                RenderText(ren, uiTex, std::to_string(tacklePPCurrent), 600.0f, 470.0f, 3.0f);
                RenderText(ren, uiTex, std::to_string(tacklePPMax), 720.0f, 470.0f, 3.0f);
            } else {
                RenderText(ren, uiTex, std::to_string(emberPPCurrent), 600.0f, 470.0f, 3.0f);
                RenderText(ren, uiTex, std::to_string(emberPPMax), 720.0f, 470.0f, 3.0f);
            }
            
            RenderText(ren, uiTex, "TACKLE", 50.0f, 470.0f, 3.0f);
            RenderText(ren, uiTex, "EMBER", 320.0f, 470.0f, 3.0f);
        } else if (currentState == MESSAGE_PLAYER_ATTACK || currentState == MESSAGE_ENEMY_ATTACK) {
            // Render message text
            RenderText(ren, uiTex, messageText, 50.0f, 470.0f, 3.0f);
        }
        
        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(bgTex);
    SDL_DestroyTexture(uiTex);
    SDL_DestroyTexture(playerTex);
    SDL_DestroyTexture(enemyTex);
    SDL_DestroyTexture(enemyText);
    SDL_DestroyTexture(playerText);
    SDL_DestroyTexture(movesTex);
    SDL_DestroyTexture(TypesTex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
