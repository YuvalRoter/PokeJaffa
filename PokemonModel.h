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
    struct IdentityComponent {
        std::string name = "Unknown";
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


    // Moveset: List of available moves for battle. (Storage: Packed)
    struct MovesetComponent {
        int move_ids[4] = {0, 0, 0, 0};
    };

    // Party: Entities (Pokemon) owned by a trainer. (Storage: Sparse)
    struct PartyComponent {
        std::vector<id_type> members;
    };

    // Combatants: Active participants in a battle arena. (Storage: Packed)
    struct CombatantsComponent {
        id_type active_ids[2];
    };

    // StatusEffect: Temporary conditions like poison/paralysis. (Storage: Packed)
    struct StatusEffectComponent {
        int effect_type = 0; // 0 = None
        int duration = 0;
    };

    // Tags: Empty components used as markers (Controller, Catchable, etc.)
    struct ControllerComponent {}; // Indicates if entity can be controlled
    struct CatchComponent {};      // Indicates if entity can be caught

    ent_type createTrainer(const std::string& name);
    ent_type createBattleArena();

    class BattleSystem {};
    class LevelingSystem {};


}
  // Mapping components to their optimized storage types.
    namespace bagel
    {

        template <> struct Storage<PokemonGame::IdentityComponent> : NoInstance { using type = SparseStorage<PokemonGame::IdentityComponent>; };
        template <> struct Storage<PokemonGame::HealthComponent> : NoInstance { using type = SparseStorage<PokemonGame::HealthComponent>; };
        template <> struct Storage<PokemonGame::StatsComponent> : NoInstance { using type = SparseStorage<PokemonGame::StatsComponent>; };
        template <> struct Storage<PokemonGame::PartyComponent> : NoInstance { using type = SparseStorage<PokemonGame::PartyComponent>; };
        template <> struct Storage<PokemonGame::MovesetComponent> : NoInstance { using type = PackedStorage<PokemonGame::MovesetComponent>; };
        template <> struct Storage<PokemonGame::CombatantsComponent> : NoInstance { using type = PackedStorage<PokemonGame::CombatantsComponent>; };
        template <> struct Storage<PokemonGame::StatusEffectComponent> : NoInstance { using type = PackedStorage<PokemonGame::StatusEffectComponent>; };
    }


