#pragma once

#include "bagel.h"
#include <SDL3/SDL.h>
#include <string>
#include <vector>

namespace PokemonGame
{
    using bagel::ent_type;
    using bagel::Storage;
    using bagel::id_type;


    // --- Component Definitions (Data-only structs) ---

    // Identity: Used to store names or unique tags. (Storage: Tag/Sparse)
    struct IdentityComponent {
        char name[24] = "Unknown";
    };

    // Health: Current and maximum hit points. (Storage: Sparse)
    struct HealthComponent {
        int hp = 100;
        int max_hp = 100;
    };

    // Stats: Base attributes like attack and defense. (Storage: Sparse)
    struct StatsComponent {
        int attack = 10;
        int defense = 10;
        int speed = 10;
    };

    struct StatusEffectComponent {
        static constexpr uint8_t Poison = 1;
        uint8_t mask = 0;
        int durationTimer = 0;
    };
    // Moveset: List of available moves for battle. (Storage: Packed)
    struct MovesetComponent {
        int move_ids[4] = {0, 0, 0, 0};
        int pp_current[4] = {0, 0, 0, 0};
    };

    // Party: Entities (Pokemon) owned by a trainer. (Storage: Sparse)
   struct PartyComponent {std::vector<id_type> members;};

    // Combatants: Active participants in a battle arena. (Storage: Packed)
    struct CombatantsComponent {id_type active_ids[2];};

    // StatusEffect: Temporary conditions like poison/paralysis. (Storage: Packed)

    // Tags: Empty components used as markers (Controller, Catchable, etc.)
    struct ControllerComponent {}; // Indicates if entity can be controlled
  //  struct CatchComponent {};      // Indicates if entity can be caught
    // Status tags
    struct PoisonTag { int turnsLeft = 3; }; // 10% max HP per round, 3 rounds total
    struct BurnTag   {};
    struct ParalyzeTag {};

    // Sprite: drawable rect bound to a texture. (Storage: Packed)
    struct SpriteComponent {
        SDL_Texture* tex = nullptr;
        SDL_FRect src{};
        SDL_FRect dst{};
        int z = 0;          // draw order, low first
        bool visible = true;
    };

    // Animation: drives frame stepping on a SpriteComponent's src rect. (Storage: Packed)
    struct AnimationComponent {
        float frameTime = 0.1f;   // seconds per frame
        float timer = 0.0f;
        int currentFrame = 0;
        int totalFrames = 1;
        float srcStartY = 0.0f;
        float srcStepY = 0.0f;    // pixels per frame on the sheet
        float startDelay = 0.0f;
        bool loop = false;
        bool done = false;
        // Optional falling motion (poison powder drops)
        float dstStartY = 0.0f;
        float dstFallDistance = 0.0f; // 0 means no fall
    };

    // StatusIcon: links a status badge sprite to its owner Pokemon. (Storage: Sparse)
    struct StatusIconComponent {
        id_type ownerId = -1;
    };

    ent_type createTrainer(const std::string& name);
    ent_type createBattleArena();



    class BattleSystem {};
    /**
 * @brief Pure ECS system that processes recurring tick damage from active status conditions.
 * * Iterates all live entities with HealthComponent. PoisonTag carries a turnsLeft counter
 * and inflicts 10% of max HP per round for 3 rounds, then removes itself. BurnTag still
 * deals fixed 10 HP per tick.
 * * @see PokemonGame::PoisonTag, PokemonGame::BurnTag, PokemonGame::HealthComponent
 */
    void runStatusSystem();
    void applyStatusEffect(bagel::Entity entity, uint8_t statusMask, int durationTurns);
    /**
 * @brief Probabilistic AI decision engine for the enemy active combatant.
 * * If the target already has PoisonTag, the AI does not reapply poison and returns Tackle (slot 0).
 * Otherwise: HP >= 50% rolls 70% status (slot 1) / 30% strike (slot 0); HP < 50% panics with
 * 80% strike / 20% status.
 * * @param enemy The active enemy entity making the tactical choice.
 * @param target The opposing entity (checked for existing poison).
 * @return int The framework Entity ID of the selected move from the entity's moveset pool.
 * @see PokemonGame::MovesetComponent, PokemonGame::HealthComponent, PokemonGame::PoisonTag
 */
    int AiEnemy(bagel::Entity enemy, bagel::Entity target);
    /**
 * @brief Determines which entity takes their turn first in a 1v1 combat round.
 * * Combines a random 1d6 dice roll with the entity's speed statistic.
 * Resolves identical score ties using a 50/50 random coin toss.
 * * @param player The player's Pokemon entity to evaluate.
 * @param enemy The enemy's Pokemon entity to evaluate.
 * @return true If the player wins initiative and attacks first.
 * @return false If the enemy wins initiative and attacks first.
 * @see PokemonGame::StatsComponent
 */
    bool determineIfPlayerGoesFirst(bagel::Entity player, bagel::Entity enemy);
    /**
 * @brief Calculates and inflicts combat damage from an attacker to a defender entity.
 * * Reconstructs the temporary move entity from its ID to fetch its base power data.
 * Applies the core battle equation formula: (Attacker Attack + Move Power) - Defender Defense.
 * Guarantees a minimum threshold of 1 damage and safely clamps target health at 0.
 * * @param attacker The entity executing the active move.
 * @param defender The target entity receiving the incoming damage.
 * @param moveEntityId The dynamic system Entity ID of the static move being cast.
 * @see PokemonGame::HealthComponent, PokemonGame::StatsComponent, PokemonGame::MoveStats
 */
    void calculateAndApplyDamage(bagel::Entity attacker, bagel::Entity defender, int moveEntityId);
 //   class LevelingSystem {};

    // Visual systems
    /**
 * @brief Renders every entity that has a SpriteComponent, in z-order (low first).
 * @param ren The active SDL_Renderer.
 */
    void renderSpriteSystem(SDL_Renderer* ren);
    /**
 * @brief Advances each AnimationComponent by dt seconds: steps src rects, and lerps dst.y
 * when dstFallDistance is non-zero (poison powder drops).
 * @param dt Seconds since last frame.
 */
    void animationSystem(float dt);
    /**
 * @brief Syncs each StatusIconComponent entity's sprite visibility to its owner's PoisonTag presence.
 */
    void statusIconSystem();

    struct MoveStats {
        char name[24];
        int power;
        int type;   // 0 = Normal, 1 = Fire, 2 = Poison
        int max_pp; // Static maximum PP property for this move
    };

    inline const MoveStats MOVE_DATABASE[] = {
        {"TACKLE", 15, 0},        // ID 0
        {"EMBER", 30, 1},         // ID 1
        {"POISON POWDER", 0, 2}   // ID 2
    };

}
    namespace bagel
    {

        template <> struct Storage<PokemonGame::IdentityComponent> : NoInstance { using type = SparseStorage<PokemonGame::IdentityComponent>; };
        template <> struct Storage<PokemonGame::HealthComponent> : NoInstance { using type = SparseStorage<PokemonGame::HealthComponent>; };
        template <> struct Storage<PokemonGame::StatsComponent> : NoInstance { using type = SparseStorage<PokemonGame::StatsComponent>; };
        template <> struct Storage<PokemonGame::PartyComponent> : NoInstance { using type = SparseStorage<PokemonGame::PartyComponent>; };
        template <> struct Storage<PokemonGame::MovesetComponent> : NoInstance { using type = PackedStorage<PokemonGame::MovesetComponent>; };
        template <> struct Storage<PokemonGame::CombatantsComponent> : NoInstance { using type = PackedStorage<PokemonGame::CombatantsComponent>; };
        template <> struct Storage<PokemonGame::StatusEffectComponent> : NoInstance { using type = PackedStorage<PokemonGame::StatusEffectComponent>; };
    // Mapping the status tags to Bagel's storage. PoisonTag now carries duration data.
        template <> struct Storage<PokemonGame::PoisonTag>   : NoInstance { using type = PackedStorage<PokemonGame::PoisonTag>; };
        template <> struct Storage<PokemonGame::BurnTag>     : NoInstance { using type = TaggedStorage<PokemonGame::BurnTag>; };
        template <> struct Storage<PokemonGame::ParalyzeTag> : NoInstance { using type = TaggedStorage<PokemonGame::ParalyzeTag>; };

    // Visual storages
        template <> struct Storage<PokemonGame::SpriteComponent>     : NoInstance { using type = PackedStorage<PokemonGame::SpriteComponent>; };
        template <> struct Storage<PokemonGame::AnimationComponent>  : NoInstance { using type = PackedStorage<PokemonGame::AnimationComponent>; };
        template <> struct Storage<PokemonGame::StatusIconComponent> : NoInstance { using type = SparseStorage<PokemonGame::StatusIconComponent>; };

    }


