#pragma once

#include "bagel.h"
#include <string>
#include <vector>

namespace PokemonGame
{
    using bagel::ent_type;
    using bagel::Storage;
    using bagel::id_type;


    // --- Component Definitions (Data-only structs) ---

    // Identity: Used to store names or unique tags. (Storage: Tag/Sparse)
    // Fixed-size char buffer (POD) so bagel's raw-memory storage stays valid.
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
    struct PoisonTag {};
    struct BurnTag   {};
    struct ParalyzeTag {};

    ent_type createTrainer(const std::string& name);
    ent_type createBattleArena();



    class BattleSystem {};
    /**
 * @brief Pure ECS system that processes recurring tick damage from active status conditions.
 * * Iterates sequentially through all active registry entities in the world world.
 * Filters for entities with valid HealthComponents, evaluates active bitmask state flags
 * using TaggedStorage markers (PoisonTag or BurnTag), and inflicts fixed tick damage.
 * * @see PokemonGame::PoisonTag, PokemonGame::BurnTag, PokemonGame::HealthComponent
 */
    void runStatusSystem();
    void applyStatusEffect(bagel::Entity entity, uint8_t statusMask, int durationTurns);
    /**
 * @brief Probabilistic AI decision engine for the enemy active combatant.
 * * Analyzes the enemy's current health pool ratio dynamically.
 * If health is >= 50%, rolls a 70% chance to cast a status move (Slot 1) and a 30% strike chance (Slot 0).
 * If health drops below 50%, shifts behaviors to an 80% strike panic chance and a 20% status chance.
 * * @param enemy The active enemy entity making the tactical choice.
 * @return int The exact framework Entity ID of the selected move from the entity's moveset pool.
 * @see PokemonGame::MovesetComponent, PokemonGame::HealthComponent
 */
    int AiEnemy(bagel::Entity enemy);
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

    struct MoveStats {
        char name[24];
        int power;
        int type;   // 0 = Normal, 1 = Fire, 2 = Poison
        int max_pp; // Static maximum PP property for this move
    };

    inline const MoveStats MOVE_DATABASE[] = {
        {"TACKLE", 20, 0},        // ID 0
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
    // Mapping the status tags to Bagel's TaggedStorage system
        template <> struct Storage<PokemonGame::PoisonTag>   : NoInstance { using type = TaggedStorage<PokemonGame::PoisonTag>; };
        template <> struct Storage<PokemonGame::BurnTag>     : NoInstance { using type = TaggedStorage<PokemonGame::BurnTag>; };
        template <> struct Storage<PokemonGame::ParalyzeTag> : NoInstance { using type = TaggedStorage<PokemonGame::ParalyzeTag>; };

    }


