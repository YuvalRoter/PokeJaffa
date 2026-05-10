#include "PokemonModel.h"

namespace PokemonGame
{
    // Factory: Creates a Trainer entity with controller and party components
    ent_type createTrainer(const std::string& name) {
        ent_type trainer = bagel::World::createEntity();

        Storage<IdentityComponent>::type::add(trainer, {name});
        Storage<PartyComponent>::type::add(trainer, {});
        Storage<ControllerComponent>::type::add(trainer, {}); // Tag component

        return trainer;
    }

    // Factory: Creates a Wild Pokemon with stats, health, and catchable status
    ent_type createWildPokemon(const std::string& species, int hp, int atk) {
        ent_type pokemon = bagel::World::createEntity();

        Storage<IdentityComponent>::type::add(pokemon, {species});
        Storage<HealthComponent>::type::add(pokemon, {hp, hp});
        Storage<StatsComponent>::type::add(pokemon, {atk, 10, 10});
        Storage<ExperienceComponent>::type::add(pokemon, {0, 5});
        Storage<StatusEffectComponent>::type::add(pokemon, {});
        Storage<CatchComponent>::type::add(pokemon, {}); // Marker for wild pokemon

        return pokemon;
    }

    // Factory: Creates a Battle Arena to manage active combatants
    ent_type createBattleArena() {
        ent_type arena = bagel::World::createEntity();

        Storage<IdentityComponent>::type::add(arena, {"Battle Arena"});
        Storage<CombatantsComponent>::type::add(arena, {});

        return arena;
    }

    // System mockup
    void battleSystemUpdate() {
        // Future logic for turn-based combat will be implemented here
    }
}