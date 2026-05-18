#include "bagel.h"
#include "PokemonModel.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h> // פותר את שגיאת ה-Stack Overflow ב-SDL3 סטטי
#include <SDL3_image/SDL_image.h>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>

namespace PokemonGame {
    using bagel::ent_type;
    using bagel::Storage;
    using bagel::id_type;

    enum BattleState {
        ATTACK_SELECT,
        START_IDLE,
        TACKLE_MOVE,
        EMBER_MOVE,
        POISON_POWDER_MOVE,
        ENEMY_HIT,
        PLAYER_HIT,
        AI_ATTACK,
        MESSAGE_PLAYER_ATTACK,
        MESSAGE_ENEMY_ATTACK,
        MESSAGE_POISONED,
        POISON_TURN_TEXT,
        STATUS_TICK,
        FINAL_IDLE
    };

    enum AttackType {
        ATTACK_TACKLE,
        ATTACK_EMBER
    };

    std::unordered_map<char, SDL_FRect> fontMap;

    void InitFont() {
        fontMap.clear();
        float upperX = 170.0f;
        float upperY = 122.0f;
        float upperW = 6.0f;
        float upperH = 11.0f;
        float upperStepX = 7.0f;

        std::string upperRow = "ABCDEFGHIJKLMNOPQRSTUVWXYZ.,";
        for (int i = 0; i < (int)upperRow.length(); ++i) {
            char c = upperRow[i];
            fontMap[c] = { upperX + (i * upperStepX), upperY, upperW, upperH };
        }

        float numX = 170.0f;
        float numY = 157.0f;
        float numW = 6.0f;
        float numH = 10.0f;
        float numStepX = 7.0f;

        std::string numRow = "0123456789";
        for (int i = 0; i < (int)numRow.length(); ++i) {
            char c = numRow[i];
            fontMap[c] = { numX + (i * numStepX), numY, numW, numH };
        }
    }

    void RenderText(SDL_Renderer* ren, SDL_Texture* tex, const std::string& text, float x, float y, float scale) {
        if (!tex) return;
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

    void UpdateHP(float& currentHP, float startHP, float targetHP, float stateTimer, float duration) {
        if (stateTimer >= duration) {
            currentHP = targetHP;
        } else {
            float progress = stateTimer / duration;
            currentHP = startHP + (targetHP - startHP) * progress;
        }
    }
}

// ==========================================
// MAIN EXECUTION
// ==========================================
int main(int argc, char* argv[]) {
    using namespace PokemonGame;

    if (!SDL_Init(SDL_INIT_VIDEO)) return -1;

    SDL_Window* win;
    SDL_Renderer* ren;
    if (!SDL_CreateWindowAndRenderer("Pokemon Battle POC", 800, 600, 0, &win, &ren)) return -1;

    // Load Textures
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
        SDL_SetSurfaceColorKey(mSurf, true, SDL_MapSurfaceRGB(mSurf, 34, 177, 76));
        movesTex = SDL_CreateTextureFromSurface(ren, mSurf);
        SDL_DestroySurface(mSurf);
    }

    // Setup Rectangles
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
    SDL_FRect StatusPoisonDst = { 500.0f, 345.0f, 45.0f, 20.0f };
    SDL_FRect StatusPoisonSrc = { 5.0f, 80.0f, 21.0f, 8.0f };

    SDL_FRect dotSrc = { 266.0f, 2.0f, 9.0f, 12.0f };
    SDL_FRect dotDst = { 30.0f, 475.0f, 20.0f, 18.0f };

    SDL_FRect enemyDstBase = { 483.0f, 64.0f, 160.0f, 160.0f };
    SDL_FRect enemySrc = { 1185.0f, 55.0f, 50.0f, 45.0f };

    // State Variables
    BattleState currentState = ATTACK_SELECT;
    AttackType currentAttack = ATTACK_TACKLE;
    bool playerTurnPending = false;
    bool enemyTurnPending = false;
    bool poisonTargetIsEnemy = false;
    bool poisonTickTargetIsEnemy = false;
    bool enemyPoisonChecked = false;
    bool playerPoisonChecked = false;
    int activeMoveId = 0;
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

    // Create ECS Entities
    auto tackleMove = bagel::Entity::create();
    tackleMove.add(MoveStats{"TACKLE", 20, 0, 30});

    auto emberMove = bagel::Entity::create();
    emberMove.add(MoveStats{"EMBER", 30, 1, 15});

    auto poisonMove = bagel::Entity::create();
    poisonMove.add(MoveStats{"POISON POWDER", 0, 2, 20});

    auto playerEntity = bagel::Entity::create();
    playerEntity.addAll(
        IdentityComponent{"CHARMANDER"},
        HealthComponent{100, 100},
        StatsComponent{15, 10, 12},
        MovesetComponent{{tackleMove.entity().id, emberMove.entity().id, 0, 0}, {30, 15, 0, 0}}
    );

    auto enemyEntity = bagel::Entity::create();
    enemyEntity.addAll(
        IdentityComponent{"CATERPIE"},
        HealthComponent{60, 60},
        StatsComponent{10, 8, 10},
        MovesetComponent{{tackleMove.entity().id, poisonMove.entity().id, 0, 0}, {30, 20, 0, 0}}
    );

    // Pokemon sprites attached to the same entities
    playerEntity.add(SpriteComponent{playerTex, playerSrc, playerDstBase, 5, true});
    enemyEntity.add(SpriteComponent{enemyTex, enemySrc, enemyDstBase, 5, true});

    // Create Visual Entities (background + static UI)
    auto bgEntity = bagel::Entity::create();
    bgEntity.add(SpriteComponent{bgTex, bgSrc, bgDst, 0, true});

    auto enemyHpFrameEntity = bagel::Entity::create();
    enemyHpFrameEntity.add(SpriteComponent{uiTex, enemyHpSrc, enemyHpDst, 10, true});

    auto playerHpFrameEntity = bagel::Entity::create();
    playerHpFrameEntity.add(SpriteComponent{uiTex, playerHpSrc, playerHpDst, 10, true});

    auto msgBoxEntity = bagel::Entity::create();
    msgBoxEntity.add(SpriteComponent{uiTex, msgBoxSrc, msgBoxDst, 10, true});

    auto msgBoxAltEntity = bagel::Entity::create();
    msgBoxAltEntity.add(SpriteComponent{uiTex, msg2BoxSrc, msg2BoxDst, 10, false});

    auto playerBannerEntity = bagel::Entity::create();
    playerBannerEntity.add(SpriteComponent{playerText, playerTextSrc, playerTextDst, 20, true});

    auto enemyBannerEntity = bagel::Entity::create();
    enemyBannerEntity.add(SpriteComponent{enemyText, EnemyTextSrc, EnemyTextDst, 20, true});

    auto typeNormalEntity = bagel::Entity::create();
    typeNormalEntity.add(SpriteComponent{TypesTex, TypeNormalSrc, TypeNormalDst, 20, true});

    auto typeFireEntity = bagel::Entity::create();
    typeFireEntity.add(SpriteComponent{TypesTex, TypeFireSrc, TypeFireDst, 20, false});

    auto dotEntity = bagel::Entity::create();
    dotEntity.add(SpriteComponent{uiTex, dotSrc, dotDst, 20, true});

    // Status icons: visibility synced to PoisonTag by statusIconSystem
    auto enemyPoisonIconEntity = bagel::Entity::create();
    enemyPoisonIconEntity.add(SpriteComponent{TypesTex, StatusPoisonSrc, StatusPoisonDst, 15, false});
    enemyPoisonIconEntity.add(StatusIconComponent{enemyEntity.entity().id});

    auto playerPoisonIconEntity = bagel::Entity::create();
    playerPoisonIconEntity.add(SpriteComponent{TypesTex, StatusPoisonSrc, StatusPoisonDst, 15, false});
    playerPoisonIconEntity.add(StatusIconComponent{playerEntity.entity().id});

    // Ember FX entity (pre-created, invisible until state entry)
    auto emberFxEntity = bagel::Entity::create();
    {
        SDL_FRect emberSrc = { 86.0f, 270.0f, 30.0f, 30.0f };
        emberFxEntity.add(SpriteComponent{movesTex, emberSrc, enemyDstBase, 6, false});
        AnimationComponent ea;
        ea.frameTime = 0.2f;
        ea.totalFrames = 5;
        ea.srcStartY = 270.0f;
        ea.srcStepY = 30.0f;
        ea.done = true; // start completed
        emberFxEntity.add(ea);
    }

    // Poison powder drops (8 pre-created, invisible until state entry)
    struct PoisonDrop { float xOffsetPct; float delay; };
    const PoisonDrop poisonDrops[8] = {
        {0.15f, 0.0f}, {0.40f, 0.2f}, {0.85f, 0.1f},
        {0.30f, 0.5f}, {0.70f, 0.3f},
        {0.20f, 1.0f}, {0.50f, 0.7f}, {0.80f, 0.9f}
    };
    bagel::Entity poisonDropEntities[8] = {
        bagel::Entity::create(), bagel::Entity::create(),
        bagel::Entity::create(), bagel::Entity::create(),
        bagel::Entity::create(), bagel::Entity::create(),
        bagel::Entity::create(), bagel::Entity::create()
    };
    for (int i = 0; i < 8; ++i) {
        SDL_FRect dropSrc = { 511.0f, 684.0f, 8.0f, 16.0f };
        SDL_FRect dropDst = { 0.0f, 0.0f, 8.0f * 2.5f, 16.0f * 2.5f };
        poisonDropEntities[i].add(SpriteComponent{movesTex, dropSrc, dropDst, 6, false});
        AnimationComponent pa;
        pa.frameTime = 2.0f / 7.0f;
        pa.totalFrames = 7;
        pa.srcStartY = 684.0f;
        pa.srcStepY = 17.0f;
        pa.done = true;
        poisonDropEntities[i].add(pa);
    }

    SDL_Event ev;
    InitFont();

    // Round-end Poison Handling Routine
    auto CheckPoisonTicks = [&]() {
        if (!enemyPoisonChecked && enemyEntity.has<PokemonGame::PoisonTag>()) {
            enemyPoisonChecked = true;
            poisonTickTargetIsEnemy = true;
            messageText = "CATERPIE IS POISONED";
            currentState = POISON_TURN_TEXT;
            stateTimer = 0.0f;
        } else if (!playerPoisonChecked && playerEntity.has<PokemonGame::PoisonTag>()) {
            playerPoisonChecked = true;
            poisonTickTargetIsEnemy = false;
            messageText = "CHARMANDER IS POISONED";
            currentState = POISON_TURN_TEXT;
            stateTimer = 0.0f;
        } else {
            enemyPoisonChecked = false;
            playerPoisonChecked = false;
            currentState = ATTACK_SELECT;
            stateTimer = 0.0f;
        }
    };

    // ==========================================
    // MAIN GAME LOOP >:3
    // ==========================================
    while (running) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) running = false;

            if (currentState == ATTACK_SELECT && ev.type == SDL_EVENT_KEY_DOWN) {
                if (ev.key.key == SDLK_LEFT || ev.key.key == SDLK_A) {
                    currentAttack = ATTACK_TACKLE;
                } else if (ev.key.key == SDLK_RIGHT || ev.key.key == SDLK_D) {
                    currentAttack = ATTACK_EMBER;
                } else if (ev.key.key == SDLK_RETURN) {
                    auto& moves = playerEntity.get<MovesetComponent>();
                    int slotIdx = (currentAttack == ATTACK_TACKLE) ? 0 : 1;

                    if (moves.pp_current[slotIdx] > 0) {
                        moves.pp_current[slotIdx]--;

                        bool playerGoesFirst = determineIfPlayerGoesFirst(playerEntity, enemyEntity);

                        if (playerGoesFirst) {
                            activeMoveId = moves.move_ids[slotIdx];
                            bagel::Entity moveEnt(ent_type{activeMoveId});
                            messageText = std::string("CHARMANDER USES ") + moveEnt.get<MoveStats>().name;
                            currentState = MESSAGE_PLAYER_ATTACK;
                            enemyTurnPending = true;
                            playerTurnPending = false;
                        } else {
                            activeMoveId = AiEnemy(enemyEntity, playerEntity);
                            bagel::Entity moveEnt(ent_type{activeMoveId});
                            messageText = std::string("CATERPIE USES ") + moveEnt.get<MoveStats>().name;
                            currentState = MESSAGE_ENEMY_ATTACK;
                            playerTurnPending = true;
                            enemyTurnPending = false;
                        }
                        stateTimer = 0.0f;
                    }
                }
            }
        }

        SDL_FRect playerDst = playerDstBase;
        SDL_FRect enemyDst = enemyDstBase;
        enemyDst.y += enemyYOffset;
        stateTimer += 0.016f;

        // State entry detection
        BattleState stateBefore = currentState;

        switch (currentState) {
            case ATTACK_SELECT:
                idleTimer += 0.05f;
                playerDst.y += std::sin(idleTimer) * 4.0f;
                break;

            case START_IDLE:
                idleTimer += 0.05f;
                playerDst.y += std::sin(idleTimer) * 20.0f;
                if (stateTimer >= 0.5f) {
                    bagel::Entity moveEnt(ent_type{activeMoveId});
                    int moveType = moveEnt.get<PokemonGame::MoveStats>().type;

                    if (moveType == 0) currentState = TACKLE_MOVE;
                    else if (moveType == 1) currentState = EMBER_MOVE;
                    else if (moveType == 2) currentState = POISON_POWDER_MOVE;

                    stateTimer = 0.0f;
                }
                break;

            case TACKLE_MOVE:
                if (enemyTurnPending) {
                    // Player attacks: approach moving up and right towards enemy
                    playerDst.x += (stateTimer * 1200.0f);
                    playerDst.y -= (stateTimer * 400.0f);
                    if (playerDst.x > 380.0f) {
                        calculateAndApplyDamage(playerEntity, enemyEntity, activeMoveId);

                        enemyHPAtStartOfHit = enemyHP;
                        targetEnemyHP = static_cast<float>(enemyEntity.get<HealthComponent>().hp) / enemyEntity.get<HealthComponent>().max_hp;
                        playerHPAtStartOfHit = playerHP;
                        targetPlayerHP = static_cast<float>(playerEntity.get<HealthComponent>().hp) / playerEntity.get<HealthComponent>().max_hp;

                        currentState = ENEMY_HIT;
                        stateTimer = 0.0f;
                    }
                } else {
                    // Enemy attacks: approach moving down and left towards player
                    enemyDst.x -= (stateTimer * 1200.0f);
                    enemyDst.y += (stateTimer * 400.0f);
                    if (enemyDst.x < 171.0f) {
                        calculateAndApplyDamage(enemyEntity, playerEntity, activeMoveId);

                        enemyHPAtStartOfHit = enemyHP;
                        targetEnemyHP = static_cast<float>(enemyEntity.get<HealthComponent>().hp) / enemyEntity.get<HealthComponent>().max_hp;
                        playerHPAtStartOfHit = playerHP;
                        targetPlayerHP = static_cast<float>(playerEntity.get<HealthComponent>().hp) / playerEntity.get<HealthComponent>().max_hp;

                        currentState = PLAYER_HIT;
                        stateTimer = 0.0f;
                    }
                }
                break;

            case EMBER_MOVE:
                if (stateTimer >= 1.0f) {
                    if (enemyTurnPending) {
                        calculateAndApplyDamage(playerEntity, enemyEntity, activeMoveId);
                    } else {
                        calculateAndApplyDamage(enemyEntity, playerEntity, activeMoveId);
                    }

                    enemyHPAtStartOfHit = enemyHP;
                    targetEnemyHP = static_cast<float>(enemyEntity.get<PokemonGame::HealthComponent>().hp) / enemyEntity.get<PokemonGame::HealthComponent>().max_hp;
                    playerHPAtStartOfHit = playerHP;
                    targetPlayerHP = static_cast<float>(playerEntity.get<HealthComponent>().hp) / playerEntity.get<HealthComponent>().max_hp;

                    currentState = enemyTurnPending ? ENEMY_HIT : PLAYER_HIT;
                    stateTimer = 0.0f;
                }
                break;

            case POISON_POWDER_MOVE:
                if (stateTimer >= 3.0f) {
                    enemyHPAtStartOfHit = enemyHP;
                    targetEnemyHP = static_cast<float>(enemyEntity.get<HealthComponent>().hp) / enemyEntity.get<HealthComponent>().max_hp;
                    playerHPAtStartOfHit = playerHP;
                    targetPlayerHP = static_cast<float>(playerEntity.get<HealthComponent>().hp) / playerEntity.get<HealthComponent>().max_hp;

                    currentState = enemyTurnPending ? ENEMY_HIT : PLAYER_HIT;
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
                        bagel::Entity moveEnt(ent_type{activeMoveId});
                        int moveType = moveEnt.get<MoveStats>().type;
                        if (moveType == 2) {
                            messageText = "CATERPIE IS POISONED";
                            poisonTargetIsEnemy = true;
                            currentState = MESSAGE_POISONED;
                        } else if (enemyTurnPending) {
                            enemyTurnPending = false;
                            activeMoveId = AiEnemy(enemyEntity, playerEntity);
                            bagel::Entity mEnt(ent_type{activeMoveId});
                            messageText = std::string("CATERPIE USES ") + mEnt.get<MoveStats>().name;
                            currentState = MESSAGE_ENEMY_ATTACK;
                        } else {
                            enemyPoisonChecked = false;
                            playerPoisonChecked = false;
                            CheckPoisonTicks();
                        }
                    }
                    stateTimer = 0.0f;
                }
                break;

            case MESSAGE_PLAYER_ATTACK:
                if (stateTimer >= 1.0f) {
                    bagel::Entity moveEnt(ent_type{activeMoveId});
                    int moveType = moveEnt.get<MoveStats>().type;

                    if (moveType == 0) currentState = TACKLE_MOVE;
                    else if (moveType == 1) currentState = EMBER_MOVE;
                    else if (moveType == 2) currentState = POISON_POWDER_MOVE;

                    stateTimer = 0.0f;
                    idleTimer = 0.0f;
                }
                break;

            case MESSAGE_ENEMY_ATTACK:
                if (stateTimer >= 1.0f) {
                    bagel::Entity moveEnt(ent_type{activeMoveId});
                    int moveType = moveEnt.get<MoveStats>().type;

                    if (moveType == 0) currentState = TACKLE_MOVE;
                    else if (moveType == 1) currentState = EMBER_MOVE;
                    else if (moveType == 2) currentState = POISON_POWDER_MOVE;

                    stateTimer = 0.0f;
                }
                break;

            case MESSAGE_POISONED:
                if (stateTimer >= 1.0f) {
                    if (poisonTargetIsEnemy) {
                        if (!enemyEntity.has<PokemonGame::PoisonTag>()) {
                            enemyEntity.add(PokemonGame::PoisonTag{3});
                        }
                    } else {
                        if (!playerEntity.has<PokemonGame::PoisonTag>()) {
                            playerEntity.add(PokemonGame::PoisonTag{3});
                        }
                    }

                    if (enemyTurnPending) {
                        enemyTurnPending = false;
                        activeMoveId = AiEnemy(enemyEntity, playerEntity);
                        bagel::Entity mEnt(ent_type{activeMoveId});
                        messageText = std::string("CATERPIE USES ") + mEnt.get<MoveStats>().name;
                        currentState = MESSAGE_ENEMY_ATTACK;
                    } else if (playerTurnPending) {
                        playerTurnPending = false;
                        auto& moves = playerEntity.get<MovesetComponent>();
                        int slotIdx = (currentAttack == ATTACK_TACKLE) ? 0 : 1;
                        activeMoveId = moves.move_ids[slotIdx];
                        bagel::Entity mEnt(ent_type{activeMoveId});
                        messageText = std::string("CHARMANDER USES ") + mEnt.get<MoveStats>().name;
                        currentState = MESSAGE_PLAYER_ATTACK;
                    } else {
                        enemyPoisonChecked = false;
                        playerPoisonChecked = false;
                        CheckPoisonTicks();
                    }
                    stateTimer = 0.0f;
                }
                break;

            case POISON_TURN_TEXT:
                if (stateTimer >= 1.0f) {
                    enemyHPAtStartOfHit = enemyHP;
                    playerHPAtStartOfHit = playerHP;

                    if (poisonTickTargetIsEnemy) {
                        auto& hc = enemyEntity.get<HealthComponent>();
                        hc.hp -= 10;
                        if (hc.hp < 0) hc.hp = 0;
                        targetEnemyHP = static_cast<float>(hc.hp) / hc.max_hp;
                    } else {
                        auto& hc = playerEntity.get<HealthComponent>();
                        hc.hp -= 10;
                        if (hc.hp < 0) hc.hp = 0;
                        targetPlayerHP = static_cast<float>(hc.hp) / hc.max_hp;
                    }

                    currentState = STATUS_TICK;
                    stateTimer = 0.0f;
                }
                break;

            case AI_ATTACK:
                break;

            case PLAYER_HIT:
                UpdateHP(playerHP, playerHPAtStartOfHit, targetPlayerHP, stateTimer, 0.6f);
                playerVisible = (int(stateTimer * 100) % 2 == 0);
                if (stateTimer > 0.6f) {
                    playerDst = playerDstBase;

                    if (targetPlayerHP <= 0.0f) {
                        currentState = FINAL_IDLE;
                    } else {
                        bagel::Entity moveEnt(ent_type{activeMoveId});
                        int moveType = moveEnt.get<MoveStats>().type;
                        if (moveType == 2) {
                            messageText = "CHARMANDER IS POISONED";
                            poisonTargetIsEnemy = false;
                            currentState = MESSAGE_POISONED;
                        } else if (playerTurnPending) {
                            playerTurnPending = false;
                            auto& moves = playerEntity.get<MovesetComponent>();
                            int slotIdx = (currentAttack == ATTACK_TACKLE) ? 0 : 1;
                            activeMoveId = moves.move_ids[slotIdx];
                            bagel::Entity mEnt(ent_type{activeMoveId});
                            messageText = std::string("CHARMANDER USES ") + mEnt.get<MoveStats>().name;
                            currentState = MESSAGE_PLAYER_ATTACK;
                        } else {
                            enemyPoisonChecked = false;
                            playerPoisonChecked = false;
                            CheckPoisonTicks();
                        }
                    }
                    stateTimer = 0.0f;
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

            case STATUS_TICK:
                UpdateHP(playerHP, playerHPAtStartOfHit, targetPlayerHP, stateTimer, 0.6f);
                UpdateHP(enemyHP, enemyHPAtStartOfHit, targetEnemyHP, stateTimer, 0.6f);
                if (stateTimer > 0.6f) {
                    if (targetPlayerHP <= 0.0f || targetEnemyHP <= 0.0f) {
                        currentState = FINAL_IDLE;
                    } else {
                        CheckPoisonTicks();
                    }
                    stateTimer = 0.0f;
                }
                break;
        }

        // On state entry: configure animation entities for Ember / Poison Powder
        if (currentState != stateBefore) {
            if (stateBefore == EMBER_MOVE) {
                emberFxEntity.get<SpriteComponent>().visible = false;
            } else if (stateBefore == POISON_POWDER_MOVE) {
                for (int i = 0; i < 8; ++i) {
                    poisonDropEntities[i].get<SpriteComponent>().visible = false;
                }
            }

            if (currentState == EMBER_MOVE) {
                auto& es = emberFxEntity.get<SpriteComponent>();
                es.dst = enemyDst;
                es.src = { 86.0f, 270.0f, 30.0f, 30.0f };
                es.visible = true;
                auto& ea = emberFxEntity.get<AnimationComponent>();
                ea.timer = 0.0f;
                ea.currentFrame = 0;
                ea.done = false;
            } else if (currentState == POISON_POWDER_MOVE) {
                const float scale = 2.5f;
                SDL_FRect targetDstRect = enemyTurnPending ? enemyDst : playerDst;
                const float startY = targetDstRect.y - 15.0f;
                const float fallDistance = targetDstRect.h + 30.0f;
                for (int i = 0; i < 8; ++i) {
                    auto& ps = poisonDropEntities[i].get<SpriteComponent>();
                    ps.src = { 511.0f, 684.0f, 8.0f, 16.0f };
                    ps.dst.w = 8.0f * scale;
                    ps.dst.h = 16.0f * scale;
                    ps.dst.x = targetDstRect.x + (targetDstRect.w * poisonDrops[i].xOffsetPct) - (ps.dst.w / 2.0f);
                    ps.dst.y = startY;
                    ps.visible = true;
                    auto& pa = poisonDropEntities[i].get<AnimationComponent>();
                    pa.timer = 0.0f;
                    pa.currentFrame = 0;
                    pa.startDelay = poisonDrops[i].delay;
                    pa.dstStartY = startY;
                    pa.dstFallDistance = fallDistance;
                    pa.done = false;
                }
            }
        }

        // Pokemon SpriteComponents
        {
            auto& playerSprite = playerEntity.get<SpriteComponent>();
            auto& enemySprite = enemyEntity.get<SpriteComponent>();
            playerSprite.dst = playerDst;
            playerSprite.visible = playerVisible;
            enemySprite.dst = enemyDst;
            enemySprite.visible = enemyVisible;
        }

        // Dynamic UI toggles: message box variant, dot position, type icon, status icon
        {
            auto& mb = msgBoxEntity.get<SpriteComponent>();
            auto& mbAlt = msgBoxAltEntity.get<SpriteComponent>();
            bool inSelect = (currentState == ATTACK_SELECT);
            mb.visible = inSelect;
            mbAlt.visible = !inSelect;

            auto& dot = dotEntity.get<SpriteComponent>();
            dot.visible = inSelect;
            if (inSelect) dot.dst.x = (currentAttack == ATTACK_TACKLE) ? 30.0f : 300.0f;

            auto& tn = typeNormalEntity.get<SpriteComponent>();
            auto& tf = typeFireEntity.get<SpriteComponent>();
            tn.visible = inSelect && (currentAttack == ATTACK_TACKLE);
            tf.visible = inSelect && (currentAttack == ATTACK_EMBER);
        }

        // Calculate Poison Tint Pulse Animation Modifier
        bool playerPulse = (currentState == MESSAGE_POISONED && !poisonTargetIsEnemy) ||
                          (currentState == STATUS_TICK && !poisonTickTargetIsEnemy);
        bool enemyPulse = (currentState == MESSAGE_POISONED && poisonTargetIsEnemy) ||
                        (currentState == STATUS_TICK && poisonTickTargetIsEnemy);

        if (playerTex) {
            if (playerPulse) {
                float pulse = (std::sin(stateTimer * 8.0f) + 1.0f) / 2.0f;
                Uint8 greenValue = static_cast<Uint8>(80 + (175 * pulse));
                Uint8 redValue   = static_cast<Uint8>(180 + (75 * pulse));
                SDL_SetTextureColorMod(playerTex, redValue, greenValue, 255);
            } else {
                SDL_SetTextureColorMod(playerTex, 255, 255, 255);
            }
        }

        if (enemyTex) {
            if (enemyPulse) {
                float pulse = (std::sin(stateTimer * 8.0f) + 1.0f) / 2.0f;
                Uint8 greenValue = static_cast<Uint8>(80 + (175 * pulse));
                Uint8 redValue   = static_cast<Uint8>(180 + (75 * pulse));
                SDL_SetTextureColorMod(enemyTex, redValue, greenValue, 255);
            } else {
                SDL_SetTextureColorMod(enemyTex, 255, 255, 255);
            }
        }

        // Render: ECS-driven sprites first, then dynamic overlays (HP bars + text)
        SDL_RenderClear(ren);
        animationSystem(0.016f);
        statusIconSystem();
        renderSpriteSystem(ren);

        // Reset Texture Colors to Default State
        if (playerTex) SDL_SetTextureColorMod(playerTex, 255, 255, 255);
        if (enemyTex) SDL_SetTextureColorMod(enemyTex, 255, 255, 255);

        if (uiTex) {
            // enemy HP bar
            SDL_FRect enemyhpBarDst = { 149.0f, 89.0f, 134.0f, 12.0f};
            SDL_FRect enemyhpBlackBarDst = { 145.0f, 89.0f, 143.0f, 12.0f };
            SDL_RenderTexture(ren, uiTex, &hpBarBlackSrc, &enemyhpBlackBarDst);
            if (enemyHP > 0.0f) {
                SDL_FRect enemyhpGreenDst = enemyhpBarDst;
                enemyhpGreenDst.w *= enemyHP;
                SDL_RenderTexture(ren, uiTex, &hpBarGreenSrc, &enemyhpGreenDst);
            }

            // player HP bar
            SDL_FRect playerHpBarDst = { 599.6f, 347.0f, 137.0f, 12.0f };
            SDL_FRect playerHpBlackBarDst = { 597.0f, 347.0f, 144.0f, 12.0f };
            SDL_RenderTexture(ren, uiTex, &hpBarBlackSrc, &playerHpBlackBarDst);
            if (playerHP > 0.0f) {
                SDL_FRect playerHpGreenDst = playerHpBarDst;
                playerHpGreenDst.w *= playerHP;
                SDL_RenderTexture(ren, uiTex, &hpBarGreenSrc, &playerHpGreenDst);
            }
        }

        RenderText(ren, uiTex, "4", 268.0f, 53.0f, 2.3f);
        RenderText(ren, uiTex, "6", 721.0f, 315.0f, 2.3f);

        if (currentState == ATTACK_SELECT) {
            auto& moves = playerEntity.get<MovesetComponent>();
            int slotIdx = (currentAttack == ATTACK_TACKLE) ? 0 : 1;

            bagel::Entity moveEnt(bagel::ent_type{moves.move_ids[slotIdx]});
            auto& moveStats = moveEnt.get<PokemonGame::MoveStats>();

            RenderText(ren, uiTex, std::to_string(moves.pp_current[slotIdx]), 600.0f, 470.0f, 3.0f);
            RenderText(ren, uiTex, std::to_string(moveStats.max_pp), 720.0f, 470.0f, 3.0f);

            RenderText(ren, uiTex, "TACKLE", 50.0f, 470.0f, 3.0f);
            RenderText(ren, uiTex, "EMBER", 320.0f, 470.0f, 3.0f);
        } else if (currentState == MESSAGE_PLAYER_ATTACK || currentState == MESSAGE_ENEMY_ATTACK || currentState == MESSAGE_POISONED || currentState == POISON_TURN_TEXT || currentState == STATUS_TICK) {
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