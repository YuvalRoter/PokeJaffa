#include "PokemonModel.h"
#include <cstring>

namespace PokemonGame
{
    ent_type createTrainer(const std::string& name) {
        ent_type trainer = bagel::World::createEntity();

        IdentityComponent id{};
        std::strncpy(id.name, name.c_str(), sizeof(id.name) - 1);

        Storage<IdentityComponent>::type::add(trainer, id);
        Storage<PartyComponent>::type::add(trainer, {});
        Storage<ControllerComponent>::type::add(trainer, {}); // Tag component

        return trainer;
    }


    ent_type createBattleArena() {
        ent_type arena = bagel::World::createEntity();

        Storage<IdentityComponent>::type::add(arena, {"Battle Arena"});
        Storage<CombatantsComponent>::type::add(arena, {});

        return arena;
    }

    void battleSystemUpdate() {
        // Future logic for turn-based combat will be implemented here
    }

    void runStatusSystem() {
        for (auto e = bagel::Entity::first(); !e.eof(); e.next()) {
            if (!e.has<HealthComponent>()) continue;
            auto& health = e.get<HealthComponent>();

            if (e.has<PoisonTag>()) {
                health.hp -= 5;
            }
            if (e.has<BurnTag>()) {
                health.hp -= 10;
            }
            if (health.hp < 0) health.hp = 0;
        }
    }
    int AiEnemy(bagel::Entity enemy) {
        auto& health = enemy.get<HealthComponent>();
        auto& moves  = enemy.get<MovesetComponent>();

        float hpPercent = static_cast<float>(health.hp) / health.max_hp;
        int randomValue = std::rand() % 100;

        if (hpPercent >= 0.5f) {
            // High health behavior: 70% chance for status move (stored in slot 1)
            if (randomValue < 70) return moves.move_ids[1]; // Returns the actual Poison Powder Entity ID
            return moves.move_ids[0]; // Returns the actual Tackle Entity ID
        } else {
            // Low health panic behavior: 80% chance for direct attack (stored in slot 0)
            if (randomValue < 80) return moves.move_ids[0];
            return moves.move_ids[1];
        }
    }
    bool determineIfPlayerGoesFirst(bagel::Entity player, bagel::Entity enemy) {
        auto& playerStats = player.get<StatsComponent>();
        auto& enemyStats  = enemy.get<StatsComponent>();

        int playerRoll = (std::rand() % 6) + 1; // 1d6
        int enemyRoll  = (std::rand() % 6) + 1; // 1d6

        int playerScore = playerRoll + playerStats.speed;
        int enemyScore  = enemyRoll + enemyStats.speed;

        if (playerScore == enemyScore) {
            return (std::rand() % 2) == 0; // 50/50 Coin toss tie-breaker
        }
        return playerScore > enemyScore;
    }
    void calculateAndApplyDamage(bagel::Entity attacker, bagel::Entity defender, int moveEntityId) {
        // Reconstruct the move Entity from its ID to fetch data
        bagel::Entity moveEntity = bagel::Entity(bagel::ent_type{moveEntityId});
        if (!moveEntity.has<MoveStats>()) return;

        auto& moveData       = moveEntity.get<PokemonGame::MoveStats>();
        auto& attackerStats  = attacker.get<StatsComponent>();
        auto& defenderStats  = defender.get<StatsComponent>();
        auto& defenderHealth = defender.get<HealthComponent>();

        // Damage Formula: (Attacker Attack + Move Power) - Defender Defense
        int damageDealt = (attackerStats.attack + moveData.power) - defenderStats.defense;
        if (damageDealt < 1) damageDealt = 1; // Ensure at least 1 damage is dealt

        defenderHealth.hp -= damageDealt;
        if (defenderHealth.hp < 0) defenderHealth.hp = 0;
    }
}