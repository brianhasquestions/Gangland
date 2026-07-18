#include "gangland.h"
#include <shlwapi.h>

#define MAX_FIGHTERS 24
#define MAX_ROUNDS 15
#define FIGHTER_PLAYER 0
#define FIGHTER_UNIT 1
#define FIGHTER_HEIR 2
#define FIGHTER_ENEMY 3
#define COP_JOIN_HEAT 50
#define BLAZING_BONUS 15
#define CAREFUL_OUR_PENALTY 0
#define CAREFUL_ENEMY_PENALTY 20
#define SNEAK_ENEMY_PENALTY 10
#define DEFENSE_ENEMY_PENALTY 10
#define UNIT_XP_STEP 80
#define VETERAN_LEVEL 3
#define RAID_CASH_LOSS_PERCENT 30
#define RAID_WIDOW_CHANCE 20
#define MISSION_TAILOR_BIZ 9
#define TAILOR_REWARD 300
#define MISSION2_REWARD 500
#define MISSION3_REWARD 1500
#define MISSION5_REWARD 3000
#define AMMO_DRY_PENALTY 15
#define DRIVEBY_KILLS 2
#define DRIVEBY_BONUS 5
#define SUCCESSION_DESERT_CHANCE 30
#define RESCUE_FAIL_DEATH_CHANCE 50
#define PLAYER_BASE_ACC 50
#define PLAYER_ACC_PER_LEVEL 3
#define PLAYER_ACC_PER_TIER 3
#define PLAYER_DMG_LOW_BASE 12
#define PLAYER_DMG_LOW_PER_TIER 2
#define PLAYER_DMG_HIGH_BASE 20
#define PLAYER_DMG_HIGH_PER_TIER 3
#define WEAPON_TIER_TOMMY 2
#define WEAPON_TIER_CUSTOM 3
#define UNIT_ACC_PER_LEVEL 2
#define HEIR_FIGHTER_HP_BASE 120
#define HEIR_FIGHTER_HP_DIVISOR 2
#define HEIR_FIGHTER_ACC_BASE 60
#define HEIR_FIGHTER_ACC_DIVISOR 5
#define HEIR_FIGHTER_DMG_LOW 15
#define HEIR_FIGHTER_DMG_HIGH 25
#define HEIR_FIGHTER_ATTACKS 2
#define SUCCESSOR_HP_BASE 90
#define SUCCESSOR_HP_DIVISOR 2
#define SUCCESSOR_LEVEL_DIVISOR 25
#define COP_RAID_BASE 3
#define COP_RAID_DAY_DIVISOR 10
#define COP_RAID_EXTRA_CAP 5
#define RIVAL_RAID_BASE 2
#define RIVAL_RAID_DAY_DIVISOR 12
#define RIVAL_RAID_VETERAN_DAY 25
#define CAPO_LOOT 800
#define FIGHT_MEDPACK_HEAL 60
#define MEDPACK_TRIGGER_PERCENT 30
#define HIDEOUT_DOWN_DAYS 6
#define HIDEOUT_LOOT_BASE 300
#define HIDEOUT_LOOT_SPAN 501
#define HIDEOUT_AMMO 4
#define ANGER_HIDEOUT 10
#define ANGER_EXTORT 12
#define ANGER_TAKEOVER 16
#define ANGER_CONTRACT 10
#define ANGER_VENDETTA 15
#define ANGER_CAPO_FALLEN 20

typedef struct Fighter
{
    int kind;
    int refIndex;
    int hp;
    int accuracy;
    int damageLow;
    int damageHigh;
    int attacks;
    const WCHAR* title;
} Fighter;

typedef struct Battle
{
    int ourCount;
    int foeCount;
    int ourBonus;
    int foeBonus;
    int defensive;
    Fighter ours[MAX_FIGHTERS];
    Fighter foes[MAX_FIGHTERS];
} Battle;

typedef struct EnemySpec
{
    const WCHAR* title;
    int hp;
    int accuracy;
    int damageLow;
    int damageHigh;
    int attacks;
} EnemySpec;

static const EnemySpec ENEMY_HOODLUM = { L"hoodlum", 60, 35, 8, 16, 1 };
static const EnemySpec ENEMY_SOLDIER = { L"soldier", 80, 50, 12, 22, 1 };
static const EnemySpec ENEMY_VETERAN = { L"veteran", 100, 65, 15, 25, 1 };
static const EnemySpec ENEMY_BOSS = { L"boss", 160, 75, 18, 30, 2 };
static const EnemySpec ENEMY_COP = { L"cop", 70, 45, 10, 20, 1 };

static void AddEnemies(Battle* battle, const EnemySpec* spec, int count)
{
    int index = 0;
    Fighter* fighter = NULL;

    for (index = 0; count > index; index++)
    {
        if (MAX_FIGHTERS <= battle->foeCount)
        {
            goto Cleanup;
        }
        fighter = &battle->foes[battle->foeCount];
        fighter->kind = FIGHTER_ENEMY;
        fighter->hp = spec->hp;
        fighter->accuracy = spec->accuracy;
        fighter->damageLow = spec->damageLow;
        fighter->damageHigh = spec->damageHigh;
        fighter->attacks = spec->attacks;
        fighter->title = spec->title;
        battle->foeCount = battle->foeCount + 1;
    }

Cleanup:

    return;
}

static void AddPlayerFighter(GameState* game, Battle* battle)
{
    Fighter* fighter = NULL;

    fighter = &battle->ours[battle->ourCount];
    fighter->kind = FIGHTER_PLAYER;
    fighter->hp = game->playerHealth;
    fighter->accuracy = PLAYER_BASE_ACC + (game->gunplayLevel * PLAYER_ACC_PER_LEVEL) + (game->weaponTier * PLAYER_ACC_PER_TIER);
    fighter->damageLow = PLAYER_DMG_LOW_BASE + (game->weaponTier * PLAYER_DMG_LOW_PER_TIER);
    fighter->damageHigh = PLAYER_DMG_HIGH_BASE + (game->weaponTier * PLAYER_DMG_HIGH_PER_TIER);
    fighter->attacks = 1;
    if (1 <= game->weaponTier)
    {
        fighter->attacks = 2;
    }
    if (WEAPON_TIER_CUSTOM == game->weaponTier)
    {
        fighter->attacks = 3;
    }
    fighter->title = game->playerName;
    battle->ourCount = battle->ourCount + 1;
}

static int RemoveNonBossFoe(App* app, Battle* battle, const WCHAR* format)
{
    int removed = 0;
    int index = 0;

    for (index = 0; battle->foeCount > index; index++)
    {
        if (ENEMY_BOSS.title == battle->foes[index].title)
        {
            continue;
        }
        UiLogFmt(app, format, battle->foes[index].title);
        battle->foes[index] = battle->foes[battle->foeCount - 1];
        battle->foeCount = battle->foeCount - 1;
        removed = 1;
        break;
    }

    return removed;
}

static void SpendAmmo(App* app, Battle* battle)
{
    GameState* game = NULL;

    game = app->game;
    if (game->ammo >= battle->ourCount)
    {
        game->ammo = game->ammo - battle->ourCount;
        goto Cleanup;
    }
    game->ammo = 0;
    battle->ourBonus = battle->ourBonus - AMMO_DRY_PENALTY;
    UiLog(app, L"The crew counts shells and comes up short. Dry guns make careful shooters - and dead ones.");

Cleanup:

    return;
}

static void SniperOpeningShot(App* app, Battle* battle)
{
    GameState* game = NULL;
    int index = 0;
    int hasSniper = 0;

    game = app->game;
    for (index = 0; MAX_CREW > index; index++)
    {
        if (UNIT_SNIPER == game->crew[index].type && 0 != game->crew[index].alive && ASSIGN_SQUAD == game->crew[index].assignment)
        {
            hasSniper = 1;
        }
    }
    if (0 == hasSniper)
    {
        goto Cleanup;
    }
    RemoveNonBossFoe(app, battle, L"A single rifle crack from the rooftop across the street, and a %s folds before anyone hears it.");

Cleanup:

    return;
}

static int TrySuccession(App* app)
{
    GameState* game = NULL;
    int succeeded = 0;
    int heirIndex = -1;
    int index = 0;
    Heir* heir = NULL;

    game = app->game;
    for (index = 0; MAX_HEIRS > index; index++)
    {
        if (0 != game->heirs[index].exists)
        {
            heirIndex = index;
            break;
        }
    }
    if (0 > heirIndex)
    {
        goto Cleanup;
    }
    heir = &game->heirs[heirIndex];
    SoundPlay(app, SOUND_DEATH);
    UiLog(app, L"");
    UiLogFmt(app, L"%s dies in the gutter of Paradise City. But the Mangano name does not die with him.", game->playerName);
    wnsprintfW(game->playerName, NAME_CHARS, L"%s Mangano", heir->name);
    game->generation = game->generation + 1;
    game->playerMaxHealth = SUCCESSOR_HP_BASE + (heir->quality / SUCCESSOR_HP_DIVISOR);
    game->playerHealth = game->playerMaxHealth;
    game->gunplayLevel = heir->quality / SUCCESSOR_LEVEL_DIVISOR;
    game->gunplayXp = 0;
    if (HEIR_ENFORCER == heir->type && WEAPON_TIER_TOMMY > game->weaponTier)
    {
        game->weaponTier = WEAPON_TIER_TOMMY;
    }
    heir->exists = 0;
    game->mistress = 0;
    game->mistressDays = 0;
    game->wifeConfront = 0;
    game->wifeSuspicion = 0;
    if (WIFE_MARRIED == game->wife.status)
    {
        UiLogFmt(app, L"%s puts on black and sails for the old country. Only blood can lead this family - and now the blood is %s.", game->wife.name, game->playerName);
        game->wife.status = WIFE_LEFT;
    }
    for (index = 0; MAX_CREW > index; index++)
    {
        if (UNIT_NONE != game->crew[index].type && 0 != game->crew[index].alive && SUCCESSION_DESERT_CHANCE > RandomRange(game, 100))
        {
            UiLogFmt(app, L"%s drifts away in the confusion. Some men serve a boss, not a family.", game->crew[index].name);
            game->crew[index].type = UNIT_NONE;
        }
    }
    UiLogFmt(app, L"Generation %d. %s picks up the rings, the ledger, and the vendetta. The city starts learning a new first name.", game->generation, game->playerName);
    succeeded = 1;

Cleanup:

    return succeeded;
}

static void AddOurSide(GameState* game, Battle* battle, int includeGuards)
{
    int index = 0;
    Unit* unit = NULL;
    Heir* heir = NULL;
    Fighter* fighter = NULL;

    AddPlayerFighter(game, battle);
    for (index = 0; MAX_CREW > index; index++)
    {
        unit = &game->crew[index];
        if (UNIT_NONE == unit->type || 0 == unit->alive || MAX_FIGHTERS <= battle->ourCount)
        {
            continue;
        }
        if (ASSIGN_SQUAD != unit->assignment && (0 == includeGuards || ASSIGN_SAFEHOUSE != unit->assignment))
        {
            continue;
        }
        fighter = &battle->ours[battle->ourCount];
        fighter->kind = FIGHTER_UNIT;
        fighter->refIndex = index;
        fighter->hp = unit->health;
        fighter->accuracy = unit->accuracy + (unit->level * UNIT_ACC_PER_LEVEL);
        fighter->damageLow = GetUnitSpec(unit->type)->damageLow;
        fighter->damageHigh = GetUnitSpec(unit->type)->damageHigh;
        fighter->attacks = GetUnitSpec(unit->type)->attacks;
        if (VETERAN_LEVEL <= unit->level)
        {
            fighter->attacks = fighter->attacks + 1;
        }
        if (0 != unit->armed)
        {
            fighter->attacks = fighter->attacks + 1;
        }
        fighter->title = unit->name;
        battle->ourCount = battle->ourCount + 1;
    }
    for (index = 0; MAX_HEIRS > index; index++)
    {
        heir = &game->heirs[index];
        if (0 == heir->exists || HEIR_ENFORCER != heir->type || PLACE_FIELD != heir->placement || MAX_FIGHTERS <= battle->ourCount)
        {
            continue;
        }
        fighter = &battle->ours[battle->ourCount];
        fighter->kind = FIGHTER_HEIR;
        fighter->refIndex = index;
        fighter->hp = HEIR_FIGHTER_HP_BASE + (heir->quality / HEIR_FIGHTER_HP_DIVISOR);
        fighter->accuracy = HEIR_FIGHTER_ACC_BASE + (heir->quality / HEIR_FIGHTER_ACC_DIVISOR);
        fighter->damageLow = HEIR_FIGHTER_DMG_LOW;
        fighter->damageHigh = HEIR_FIGHTER_DMG_HIGH;
        fighter->attacks = HEIR_FIGHTER_ATTACKS;
        fighter->title = heir->name;
        battle->ourCount = battle->ourCount + 1;
    }
}

static void SeductressConversion(App* app, Battle* battle)
{
    GameState* game = NULL;
    int index = 0;
    int present = 0;
    int target = -1;

    game = app->game;
    for (index = 0; MAX_HEIRS > index; index++)
    {
        if (0 != game->heirs[index].exists && HEIR_SEDUCTRESS == game->heirs[index].type && PLACE_HOME != game->heirs[index].placement)
        {
            present = 1;
        }
    }
    if (0 == present)
    {
        goto Cleanup;
    }
    for (index = 0; battle->foeCount > index; index++)
    {
        if (ENEMY_COP.title == battle->foes[index].title)
        {
            target = index;
            break;
        }
        if (0 > target && ENEMY_BOSS.title != battle->foes[index].title)
        {
            target = index;
        }
    }
    if (0 > target)
    {
        goto Cleanup;
    }
    UiLogFmt(app, L"Your seductress murmurs a few words, and a %s decides he was never here at all.", battle->foes[target].title);
    battle->foes[target] = battle->foes[battle->foeCount - 1];
    battle->foeCount = battle->foeCount - 1;

Cleanup:

    return;
}

static int CountLiving(const Fighter* list, int count)
{
    int living = 0;
    int index = 0;

    for (index = 0; count > index; index++)
    {
        if (0 < list[index].hp)
        {
            living = living + 1;
        }
    }

    return living;
}

static int PickLiving(GameState* game, const Fighter* list, int count)
{
    int picked = -1;
    int living = 0;
    int index = 0;
    int chosen = 0;

    living = CountLiving(list, count);
    if (0 == living)
    {
        goto Cleanup;
    }
    chosen = RandomRange(game, living);
    for (index = 0; count > index; index++)
    {
        if (0 >= list[index].hp)
        {
            continue;
        }
        if (0 == chosen)
        {
            picked = index;
            break;
        }
        chosen = chosen - 1;
    }

Cleanup:

    return picked;
}

static void AttackOnce(App* app, Fighter* attacker, Fighter* target)
{
    GameState* game = NULL;
    int damage = 0;

    game = app->game;
    if (0 >= attacker->hp || NULL == target || 0 >= target->hp)
    {
        goto Cleanup;
    }
    if (attacker->accuracy <= RandomRange(game, 100))
    {
        goto Cleanup;
    }
    damage = RandomBetween(game, attacker->damageLow, attacker->damageHigh);
    target->hp = target->hp - damage;
    if (0 >= target->hp)
    {
        if (FIGHTER_ENEMY == target->kind)
        {
            game->statKills = game->statKills + 1;
            UiLogFmt(app, L"%s drops a %s.", attacker->title, target->title);
        }
        else
        {
            UiLogFmt(app, L"%s goes down under a %s's fire!", target->title, attacker->title);
        }
        goto Cleanup;
    }
    if (FIGHTER_ENEMY != target->kind)
    {
        UiLogFmt(app, L"%s is hit by a %s (-%d).", target->title, attacker->title, damage);
    }

Cleanup:

    return;
}

static void SideAttacks(App* app, Battle* battle, int enemiesActing)
{
    GameState* game = NULL;
    Fighter* attackers = NULL;
    Fighter* targets = NULL;
    int attackerCount = 0;
    int targetCount = 0;
    int bonus = 0;
    int index = 0;
    int strike = 0;
    int targetIndex = 0;
    int savedAccuracy = 0;

    game = app->game;
    attackers = (0 != enemiesActing) ? battle->foes : battle->ours;
    attackerCount = (0 != enemiesActing) ? battle->foeCount : battle->ourCount;
    targets = (0 != enemiesActing) ? battle->ours : battle->foes;
    targetCount = (0 != enemiesActing) ? battle->ourCount : battle->foeCount;
    bonus = (0 != enemiesActing) ? battle->foeBonus : battle->ourBonus;
    for (index = 0; attackerCount > index; index++)
    {
        for (strike = 0; attackers[index].attacks > strike; strike++)
        {
            targetIndex = PickLiving(game, targets, targetCount);
            if (0 > targetIndex)
            {
                goto Cleanup;
            }
            savedAccuracy = attackers[index].accuracy;
            attackers[index].accuracy = savedAccuracy + bonus;
            AttackOnce(app, &attackers[index], &targets[targetIndex]);
            attackers[index].accuracy = savedAccuracy;
        }
    }

Cleanup:

    return;
}

static void WriteBackCasualties(App* app, Battle* battle)
{
    GameState* game = NULL;
    int index = 0;
    Fighter* fighter = NULL;
    Unit* unit = NULL;

    game = app->game;
    for (index = 0; battle->ourCount > index; index++)
    {
        fighter = &battle->ours[index];
        if (FIGHTER_PLAYER == fighter->kind)
        {
            game->playerHealth = (0 < fighter->hp) ? fighter->hp : 0;
        }
        if (FIGHTER_UNIT == fighter->kind)
        {
            unit = &game->crew[fighter->refIndex];
            unit->health = (0 < fighter->hp) ? fighter->hp : 0;
            if (0 >= fighter->hp)
            {
                unit->alive = 0;
                unit->type = UNIT_NONE;
                game->statMenLost = game->statMenLost + 1;
                UiLogFmt(app, L"%s will not be coming home. You carry what is left to the undertaker.", fighter->title);
            }
        }
        if (FIGHTER_HEIR == fighter->kind && 0 >= fighter->hp)
        {
            game->heirs[fighter->refIndex].placement = PLACE_HOME;
            UiLogFmt(app, L"%s is carried home bleeding but alive. Blood protects blood - but it was close.", fighter->title);
        }
    }
}

static void GrantExperience(App* app, Battle* battle)
{
    GameState* game = NULL;
    int index = 0;
    Fighter* fighter = NULL;
    Unit* unit = NULL;
    int foes = 0;

    game = app->game;
    foes = battle->foeCount;
    GainGunplayXp(app, 15 * foes);
    for (index = 0; battle->ourCount > index; index++)
    {
        fighter = &battle->ours[index];
        if (FIGHTER_UNIT != fighter->kind || 0 >= fighter->hp)
        {
            continue;
        }
        unit = &game->crew[fighter->refIndex];
        unit->xp = unit->xp + 20 + (10 * foes);
        while (unit->xp >= unit->level * UNIT_XP_STEP)
        {
            unit->xp = unit->xp - (unit->level * UNIT_XP_STEP);
            unit->level = unit->level + 1;
            unit->accuracy = unit->accuracy + 3;
            unit->maxHealth = unit->maxHealth + 10;
            unit->health = unit->health + 10;
            if (VETERAN_LEVEL == unit->level)
            {
                UiLogFmt(app, L"%s has survived enough gunfights to be called a veteran. Faster, meaner, harder to kill.", unit->name);
            }
            else
            {
                UiLogFmt(app, L"%s reaches level %d.", unit->name, unit->level);
            }
        }
    }
}

static void FieldMedpack(App* app, Battle* battle)
{
    GameState* game = NULL;
    Fighter* boss = NULL;

    game = app->game;
    boss = &battle->ours[0];
    if (FIGHTER_PLAYER != boss->kind || 0 >= boss->hp || 0 >= game->medpacks)
    {
        goto Cleanup;
    }
    if (((game->playerMaxHealth * MEDPACK_TRIGGER_PERCENT) / 100) <= boss->hp)
    {
        goto Cleanup;
    }
    game->medpacks = game->medpacks - 1;
    boss->hp = boss->hp + FIGHT_MEDPACK_HEAL;
    if (game->playerMaxHealth < boss->hp)
    {
        boss->hp = game->playerMaxHealth;
    }
    UiLogFmt(app, L"Bleeding behind cover, you tear open a medpack with your teeth. Bandages, morphine, back into the fight (+%d).", FIGHT_MEDPACK_HEAL);

Cleanup:

    return;
}

static int RunBattle(App* app, Battle* battle)
{
    GameState* game = NULL;
    int won = 0;
    int round = 0;

    game = app->game;
    for (round = 1; MAX_ROUNDS >= round; round++)
    {
        SideAttacks(app, battle, 0);
        if (0 == CountLiving(battle->foes, battle->foeCount))
        {
            won = 1;
            break;
        }
        SideAttacks(app, battle, 1);
        FieldMedpack(app, battle);
        if (0 == CountLiving(battle->ours, battle->ourCount))
        {
            break;
        }
    }
    if (0 != won)
    {
        game->statFightsWon = game->statFightsWon + 1;
    }
    else
    {
        game->statFightsLost = game->statFightsLost + 1;
    }
    WriteBackCasualties(app, battle);
    if (0 >= game->playerHealth)
    {
        if (0 != battle->defensive)
        {
            game->playerHealth = 1;
            UiLog(app, L"You are dragged out through the coal chute, bleeding from a dozen places but breathing.");
        }
        else if (0 == TrySuccession(app))
        {
            game->playerHealth = 0;
            game->gameOver = 1;
            game->menuId = MENU_DEAD;
            SoundPlay(app, SOUND_DEATH);
            UiLog(app, L"");
            UiLogFmt(app, L"%s dies in the gutter of Paradise City, far from Sicily. There is no heir. The family name ends here.", game->playerName);
        }
    }
    if (0 != won && 0 == game->gameOver)
    {
        GrantExperience(app, battle);
    }

    return won;
}

void QueueCombat(App* app, const WCHAR* label)
{
    GameState* game = NULL;
    int total = 0;

    game = app->game;
    CopyText(game->pending.label, label, NAME_CHARS * 2);
    game->pending.active = 1;
    if (0 < game->policePresence[game->location] && (COP_JOIN_HEAT <= game->heat || 0 != game->copsHostile))
    {
        game->pending.cops = game->policePresence[game->location];
    }
    total = game->pending.hoodlums + game->pending.soldiers + game->pending.veterans + game->pending.bosses;
    UiLog(app, L"");
    UiLogFmt(app, L"You size up the job: about %d gun%s against you.", total, (1 == total) ? L"" : L"s");
    if (0 < game->pending.cops)
    {
        UiLogFmt(app, L"Your name is hot and the law is close - expect %d cop%s to join the party.", game->pending.cops, (1 == game->pending.cops) ? L"" : L"s");
    }
    game->menuId = MENU_APPROACH;
}

static void FinishMission(App* app)
{
    GameState* game = NULL;
    int index = 0;

    game = app->game;
    if (2 == game->pending.arg)
    {
        game->missionStage = 3;
        game->rank = RANK_CAPO;
        game->cash = game->cash + MISSION2_REWARD;
        SoundPlay(app, SOUND_FANFARE);
        UiLog(app, L"");
        UiLogFmt(app, L"Vincenzo embraces you in front of the whole crew. 'No more errands, Mario. You are capo soto now.' He presses $%d into your hand - and from today, a cut of the family take is yours.", MISSION2_REWARD);
        UiLog(app, L"'Now the real work. Romano - the eldest of the three who killed Chico - is somewhere in this city. Find where he sleeps.'");
        goto Cleanup;
    }
    if (3 == game->pending.arg)
    {
        game->brothersDead[0] = 1;
        game->missionStage = 5;
        game->cash = game->cash + MISSION3_REWARD;
        UiLog(app, L"");
        UiLogFmt(app, L"Romano crawls behind his desk, bleeding. 'The Mangano kid...' You finish it for Chico. One of three. His strongbox holds $%d.", MISSION3_REWARD);
        goto Cleanup;
    }
    if (5 == game->pending.arg)
    {
        game->brothersDead[1] = 1;
        game->missionStage = 6;
        game->cash = game->cash + MISSION5_REWARD;
        UiLog(app, L"");
        UiLogFmt(app, L"Angelo dies at his own card table above Club Paradiso. Two of three. Of Vincenzo there is no trace - only an empty chair and a bloodied ring you know too well. His safe gives up $%d.", MISSION5_REWARD);
        goto Cleanup;
    }
    if (6 == game->pending.arg)
    {
        game->brothersDead[2] = 1;
        game->missionStage = 7;
        game->rank = RANK_DON;
        game->gameWon = 1;
        SoundPlay(app, SOUND_FANFARE);
        UiLog(app, L"");
        UiLog(app, L"Sonny empties his gun and runs out of house to run through. It ends in the villa garden, quietly, the way Chico would have wanted.");
        UiLog(app, L"Three of three. The vendetta is over. Word crosses the ocean to your grandfather, and the answer comes back in one line: 'Paradise City belongs to Don Mario Mangano.'");
        UiLog(app, L"You may keep playing - there is always another dollar, another district, another child to raise.");
    }
    for (index = 0; NUM_RIVALS > index; index++)
    {
        if (0 != game->rivals[index].alive)
        {
            game->rivals[index].anger = (100 - ANGER_VENDETTA > game->rivals[index].anger) ? (game->rivals[index].anger + ANGER_VENDETTA) : 100;
        }
    }

Cleanup:

    return;
}

static void HandleRescueSetback(App* app)
{
    GameState* game = NULL;

    game = app->game;
    if (KIDNAP_NOBODY == game->kidnapVictim)
    {
        goto Cleanup;
    }
    if (RESCUE_FAIL_DEATH_CHANCE > RandomRange(game, 100))
    {
        KidnapVictimDies(app);
        goto Cleanup;
    }
    UiLogFmt(app, L"By the time you regroup, the kidnappers have moved %s to a new hole. The clock is still running.", KidnapVictimName(game));

Cleanup:

    return;
}

static void FinishCombat(App* app, int won)
{
    GameState* game = NULL;
    Business* biz = NULL;
    int index = 0;

    game = app->game;
    if (0 == won)
    {
        if (0 != game->gameOver)
        {
            goto Cleanup;
        }
        UiLog(app, L"You drag your people back out under fire. Nothing gained, and the street will talk.");
        if (FIGHT_RESCUE == game->pending.kind)
        {
            HandleRescueSetback(app);
        }
        goto Cleanup;
    }
    if (FIGHT_EXTORT == game->pending.kind || FIGHT_TAKEOVER == game->pending.kind)
    {
        biz = &game->businesses[game->pending.arg];
        if (FIGHT_EXTORT == game->pending.kind)
        {
            biz->status = BIZ_EXTORTED;
            UiLogFmt(app, L"The owner of %s wipes his bloody nose and agrees to a weekly arrangement. Protection, guaranteed by the Mangano name.", biz->name);
            GainBusinessXp(app, 40);
            RaiseRivalAnger(app, biz->district, ANGER_EXTORT);
        }
        else
        {
            biz->status = BIZ_OWNED;
            biz->managerAlive = 1;
            UiLogFmt(app, L"%s is yours now - keys, books, and back room. Keep the manager alive and post a guard, or the street will take it back.", biz->name);
            GainBusinessXp(app, 60);
            RaiseRivalAnger(app, biz->district, ANGER_TAKEOVER);
        }
        game->statBizSeized = game->statBizSeized + 1;
        TagMap(app, biz->district, PING_MONEY);
        if (1 == game->missionStage && MISSION_TAILOR_BIZ == game->pending.arg)
        {
            game->missionStage = 2;
            game->cash = game->cash + TAILOR_REWARD;
            UiLogFmt(app, L"Vincenzo is pleased. '$%d for a job done clean. One more thing: three of Romano's hoodlums are squeezing an old friend of mine at the Docks. Send them to the bottom of the harbor.'", TAILOR_REWARD);
        }
    }
    if (FIGHT_FAVOR == game->pending.kind)
    {
        game->favorDistrict = -1;
        UiLog(app, L"The citizen weeps with relief and swears his life to you - a favor is a debt, and he pays it in service.");
        RecruitUnit(app, (0 == RandomRange(game, 2)) ? UNIT_GUNMAN : UNIT_SCOUT, 0);
    }
    if (FIGHT_CONTRACT == game->pending.kind)
    {
        game->businesses[game->pending.arg].managerAlive = 0;
        game->cash = game->cash + game->contractReward;
        SoundPlay(app, SOUND_CASH);
        UiLogFmt(app, L"The manager of %s will not be opening tomorrow. An envelope with $%d arrives within the hour.", game->businesses[game->pending.arg].name, game->contractReward);
        RaiseRivalAnger(app, game->businesses[game->pending.arg].district, ANGER_CONTRACT);
        game->contractBusiness = -1;
        game->contractReward = 0;
    }
    if (FIGHT_HIDEOUT == game->pending.kind)
    {
        index = HIDEOUT_LOOT_BASE + RandomRange(game, HIDEOUT_LOOT_SPAN);
        game->cash = game->cash + index;
        game->ammo = game->ammo + HIDEOUT_AMMO;
        game->rivals[game->pending.arg].hideoutDownDays = HIDEOUT_DOWN_DAYS;
        game->rivals[game->pending.arg].hideoutScouted = 0;
        SoundPlay(app, SOUND_CASH);
        UiLogFmt(app, L"The hideout gives up a strongbox with $%d, %d crates of shells, and a lesson for the whole district about locks and hinges.", index, HIDEOUT_AMMO);
        RaiseRivalAnger(app, game->rivals[game->pending.arg].district, ANGER_HIDEOUT);
        TagMap(app, game->rivals[game->pending.arg].district, PING_MONEY);
    }
    if (FIGHT_VINCENZO == game->pending.kind)
    {
        game->vincenzoRescued = 1;
        game->vincenzoAlive = 1;
        SoundPlay(app, SOUND_FANFARE);
        TagMap(app, DISTRICT_DOCKS, PING_FAMILY);
        UiLog(app, L"");
        UiLog(app, L"The old man in the basement is thinner, grayer, and still Vincenzo. He grips your arm with both hands and does not speak for a long moment.");
        UiLog(app, L"'They told me you were an errand boy,' he says at last. 'Look at you now.' The family stipend returns - doubled, for as long as he draws breath.");
    }
    if (FIGHT_CAPO == game->pending.kind)
    {
        game->rivals[game->pending.arg].alive = 0;
        game->rivals[game->pending.arg].anger = 0;
        game->statCaposKilled = game->statCaposKilled + 1;
        game->cash = game->cash + CAPO_LOOT;
        SoundPlay(app, SOUND_CASH);
        UiLogFmt(app, L"%s dies with his espresso still warm. His district belongs to nobody now - which means it belongs to you. His strongbox held $%d.", game->rivals[game->pending.arg].name, CAPO_LOOT);
        for (index = 0; NUM_RIVALS > index; index++)
        {
            if (0 != game->rivals[index].alive)
            {
                game->rivals[index].anger = (100 - ANGER_CAPO_FALLEN > game->rivals[index].anger) ? (game->rivals[index].anger + ANGER_CAPO_FALLEN) : 100;
            }
        }
        UiLog(app, L"The other capos take note. Fear and fury travel the same streets.");
    }
    if (FIGHT_CRATE_AMBUSH == game->pending.kind)
    {
        UiLog(app, L"The crate is safe. Vincenzo is waiting in Little Italy.");
    }
    if (FIGHT_RESCUE == game->pending.kind)
    {
        SoundPlay(app, SOUND_BELLS);
        UiLogFmt(app, L"You kick through the last door and find %s tied to a chair, furious and alive. The kidnappers paid in full - just not in money.", KidnapVictimName(game));
        game->kidnapVictim = KIDNAP_NOBODY;
    }
    if (FIGHT_MISSION == game->pending.kind)
    {
        FinishMission(app);
    }

Cleanup:

    return;
}

void ResolveCombat(App* app, int approach)
{
    GameState* game = NULL;
    Battle* battle = NULL;
    int won = 0;
    int index = 0;

    game = app->game;
    battle = (Battle*)AllocZeroed(sizeof(Battle));
    if (NULL == battle)
    {
        goto Cleanup;
    }
    UiLog(app, L"");
    SoundPlay(app, SOUND_GUNFIGHT);
    if (0 == approach)
    {
        UiLog(app, L"You kick the door and come in shooting.");
        battle->ourBonus = BLAZING_BONUS;
        battle->foeBonus = BLAZING_BONUS;
    }
    if (1 == approach)
    {
        UiLog(app, L"You move up slow, crew hugging cover, picking shots.");
        battle->ourBonus = 0 - CAREFUL_OUR_PENALTY;
        battle->foeBonus = 0 - CAREFUL_ENEMY_PENALTY;
    }
    AddEnemies(battle, &ENEMY_HOODLUM, game->pending.hoodlums);
    AddEnemies(battle, &ENEMY_SOLDIER, game->pending.soldiers);
    AddEnemies(battle, &ENEMY_VETERAN, game->pending.veterans);
    AddEnemies(battle, &ENEMY_BOSS, game->pending.bosses);
    AddEnemies(battle, &ENEMY_COP, game->pending.cops);
    if (2 == approach)
    {
        UiLog(app, L"Your scout slips in through a back window ahead of the crew.");
        battle->foeBonus = 0 - SNEAK_ENEMY_PENALTY;
        RemoveNonBossFoe(app, battle, L"A %s is found later in the alley with his throat cut.");
    }
    if (4 == approach)
    {
        UiLog(app, L"The sedan rolls past slow, windows down, Tommy guns talking. Then you come back around for what is left.");
        battle->ourBonus = DRIVEBY_BONUS;
        for (index = 0; DRIVEBY_KILLS > index; index++)
        {
            if (0 == RemoveNonBossFoe(app, battle, L"A %s is cut down on the sidewalk before the fight even starts."))
            {
                break;
            }
        }
        AddHeat(app, 2);
    }
    SniperOpeningShot(app, battle);
    SeductressConversion(app, battle);
    AddOurSide(game, battle, 0);
    SpendAmmo(app, battle);
    TagMap(app, game->location, PING_FIGHT);
    won = RunBattle(app, battle);
    switch (game->pending.kind)
    {
    case FIGHT_EXTORT:
        AddHeat(app, 2);
        break;
    case FIGHT_TAKEOVER:
        AddHeat(app, 3);
        break;
    case FIGHT_CONTRACT:
        AddHeat(app, 4);
        break;
    case FIGHT_MISSION:
        AddHeat(app, 3);
        break;
    default:
        AddHeat(app, 1);
        break;
    }
    FinishCombat(app, won);
    memset(&game->pending, 0, sizeof(PendingCombat));
    if (0 == game->gameOver)
    {
        game->menuId = MENU_MAIN;
    }

Cleanup:
    FreeMemory(battle);

    return;
}

void ResolveRaid(App* app, int kind)
{
    GameState* game = NULL;
    Battle* battle = NULL;
    int won = 0;
    int extra = 0;

    game = app->game;
    battle = (Battle*)AllocZeroed(sizeof(Battle));
    if (NULL == battle)
    {
        goto Cleanup;
    }
    battle->foeBonus = 0 - DEFENSE_ENEMY_PENALTY;
    battle->defensive = 1;
    if (RAID_COPS == kind)
    {
        extra = game->day / COP_RAID_DAY_DIVISOR;
        if (COP_RAID_EXTRA_CAP < extra)
        {
            extra = COP_RAID_EXTRA_CAP;
        }
        AddEnemies(battle, &ENEMY_COP, COP_RAID_BASE + extra);
    }
    if (RAID_RIVALS == kind)
    {
        AddEnemies(battle, &ENEMY_SOLDIER, RIVAL_RAID_BASE + (game->day / RIVAL_RAID_DAY_DIVISOR));
        if (RIVAL_RAID_VETERAN_DAY <= game->day)
        {
            AddEnemies(battle, &ENEMY_VETERAN, 1);
        }
    }
    if (RAID_BOUNTY == kind)
    {
        AddEnemies(battle, &ENEMY_SOLDIER, 2);
        AddEnemies(battle, &ENEMY_VETERAN, 1);
    }
    SoundPlay(app, SOUND_GUNFIGHT);
    SeductressConversion(app, battle);
    AddOurSide(game, battle, 1);
    SpendAmmo(app, battle);
    TagMap(app, game->location, PING_FIGHT);
    won = RunBattle(app, battle);
    if (0 != game->gameOver)
    {
        goto Cleanup;
    }
    if (0 != won)
    {
        UiLog(app, L"The attackers break and run, leaving their dead on your doorstep. The safe house holds.");
        goto Cleanup;
    }
    UiLog(app, L"They overrun the place. You escape over the rooftops with what you can carry.");
    game->cash = game->cash - ((game->cash * RAID_CASH_LOSS_PERCENT) / 100);
    if (RAID_BOUNTY != kind && WIFE_MARRIED == game->wife.status && RAID_WIDOW_CHANCE > RandomRange(game, 100))
    {
        game->wife.status = WIFE_DEAD;
        UiLogFmt(app, L"When you return at dawn, you find %s among the dead. The house is very quiet now.", game->wife.name);
    }

Cleanup:
    FreeMemory(battle);

    return;
}
