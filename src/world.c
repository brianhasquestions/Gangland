#include "gangland.h"
#include <shlwapi.h>

#define START_CASH 500
#define START_HEALTH 100
#define START_LEADERSHIP 3
#define GUNPLAY_ACC_BONUS 3
#define GUNPLAY_HP_BONUS 10
#define LEVEL_XP_STEP 100
#define WAGE_PER_UNIT 15
#define OWNED_UPKEEP 25
#define MANAGER_DEAD_PENALTY_PERCENT 50
#define EXTORT_VIG_PERCENT 35
#define LAWYER_BOOST_PERCENT 50
#define DISTILLERY_SYNERGY 40
#define CAPO_STIPEND 150
#define CHIEF_DEMAND 3000
#define CHIEF_INTERVAL 10
#define CHIEF_FIRST_DAY 9
#define CHIEF_GRACE_DAYS 2
#define HEAT_DECAY 3
#define DAILY_REST_HEAL 15
#define LAWYER_HEAT_DECAY 3
#define HEAT_RAID_THRESHOLD 80
#define HEAT_RAID_CHANCE 30
#define BOUNTY_AMBUSH_CHANCE 25
#define RIVAL_RAID_CHANCE 12
#define PHONE_CHANCE 30
#define FAVOR_CHANCE 18
#define GESTATION_DAYS 5
#define WIFE_RAID_DEATH_CHANCE 20
#define SAVE_MAGIC 0x474E4147
#define SAVE_VERSION 6
#define VINCENZO_STIPEND_RESCUED 300
#define VINCENZO_TIP_CHANCE 12
#define HIDEOUT_RESPAWN_DAYS 6
#define CORNER_INCOME_LOW 60
#define CORNER_INCOME_SPAN 61
#define CORNER_SHAKEDOWN_CHANCE 12
#define CORNER_SHAKEDOWN_COST 80
#define CORNER_TROUBLE_CHANCE 6
#define CORNER_TROUBLE_HURT 30
#define RIVAL_ANGER_DECAY 2
#define RIVAL_START_STRENGTH 20
#define RIVAL_STRENGTH_CAP 90
#define RIVAL_RAID_ANGER_FLOOR 15
#define DISTRICT_EVENT_CHANCE 18
#define DISTRICT_EVENT_DAYS 2
#define STATUS_BUFFER_CHARS 1024
#define PATH_BUFFER_CHARS 64
#define START_AMMO 12
#define AMMO_FROM_STORE 3
#define MISTRESS_UPKEEP 60
#define MISTRESS_TIP_CHANCE 20
#define SUSPICION_LIMIT 100
#define SUSPICION_COOL 5
#define INLAW_ASK_CHANCE 10
#define INLAW_ASK_COST 300
#define INLAW_TRIBUTE_INCOME 45
#define KIDNAP_CHANCE 6
#define KIDNAP_DAYS 3
#define COUNTER_EXTORT_CHANCE 8
#define INFORMANT_CHANCE 5
#define INFORMANT_HEAT_FLOOR 60
#define INFORMANT_HEAT_SPIKE 5
#define NEWS_BOUNTY_CHANCE 10
#define NEWS_TIP_CHANCE 6
#define BOUNTY_DAYS_LOW 3
#define BOUNTY_DAYS_HIGH 5
#define HEIR_QUALITY_BASE 30
#define HEIR_QUALITY_PER_TRAIT 10
#define HEIR_QUALITY_SPAN 21
#define HEIR_WILDCARD_CHANCE 40
#define HEIR_DUD_QUALITY 50
#define SUSPICION_PER_DAY_BASE 3
#define SUSPICION_PER_DAY_SPAN 8
#define SUSPICION_VISIBLE 60
#define KIDNAP_RANSOM_BASE 1500
#define KIDNAP_RANSOM_SPAN 2501
#define PHONE_UNIT_PRICE_LOW 55
#define PHONE_UNIT_PRICE_HIGH 80
#define PHONE_HIT_REWARD_LOW 400
#define PHONE_HIT_REWARD_HIGH 1200
#define MISTRESS_HEAT_RELIEF 2

static Rival* AngriestRival(GameState* game);

static const UnitSpec UNIT_SPECS[NUM_UNIT_TYPES] =
{
    { L"Nobody", 0, 0, 0, 0, 0, 0 },
    { L"Bouncer", 300, 120, 35, 20, 35, 1 },
    { L"Gunman", 500, 80, 55, 15, 25, 1 },
    { L"Henchman", 900, 90, 65, 12, 22, 2 },
    { L"Street girl", 200, 50, 40, 10, 18, 1 },
    { L"Scout", 400, 60, 30, 8, 14, 1 },
    { L"Sniper", 1400, 70, 85, 40, 60, 1 },
    { L"Bomber", 1600, 70, 30, 10, 18, 1 },
    { L"Enforcer", 0, 150, 80, 15, 25, 2 }
};

static const WCHAR* const DISTRICT_NAMES[NUM_DISTRICTS] = { L"Little Italy", L"Downtown", L"The Docks", L"Market Quarter", L"The Suburbs" };

static const WCHAR* const RECRUIT_NAMES[] = { L"Sal", L"Tony", L"Enzo", L"Luca", L"Paulie", L"Nico", L"Franco", L"Gio", L"Marco", L"Dino", L"Rocco", L"Carlo" };
#define NUM_RECRUIT_NAMES 12

typedef struct BusinessTemplate
{
    const WCHAR* title;
    int type;
    int district;
    int income;
} BusinessTemplate;

static const BusinessTemplate BUSINESS_TEMPLATES[MAX_BUSINESSES] =
{
    { L"Mama Rosa's Trattoria", BIZ_RESTAURANT, DISTRICT_LITTLE_ITALY, 220 },
    { L"Santoro's Tailors", BIZ_CLOTHES, DISTRICT_LITTLE_ITALY, 160 },
    { L"Lucky Pawn", BIZ_PAWN, DISTRICT_LITTLE_ITALY, 180 },
    { L"Grand Jewelers", BIZ_JEWELRY, DISTRICT_DOWNTOWN, 340 },
    { L"Club Paradiso", BIZ_RESTAURANT, DISTRICT_DOWNTOWN, 240 },
    { L"Flash Photo Studio", BIZ_PHOTO, DISTRICT_DOWNTOWN, 150 },
    { L"Harbor Arms & Ammo", BIZ_AMMO, DISTRICT_DOCKS, 260 },
    { L"Gullo's Distillery", BIZ_DISTILLERY, DISTRICT_DOCKS, 300 },
    { L"Dockside Diner", BIZ_RESTAURANT, DISTRICT_DOCKS, 200 },
    { L"Bellini's Tailors", BIZ_CLOTHES, DISTRICT_MARKET, 170 },
    { L"Market Butchers", BIZ_RESTAURANT, DISTRICT_MARKET, 180 },
    { L"Verdi Jewelry", BIZ_JEWELRY, DISTRICT_MARKET, 300 },
    { L"Corner Pawnshop", BIZ_PAWN, DISTRICT_MARKET, 170 },
    { L"Roadside Distillery", BIZ_DISTILLERY, DISTRICT_SUBURBS, 280 }
};

const WCHAR* DistrictName(int district)
{
    const WCHAR* name = L"Nowhere";

    if (0 <= district && NUM_DISTRICTS > district)
    {
        name = DISTRICT_NAMES[district];
    }

    return name;
}

const UnitSpec* GetUnitSpec(int type)
{
    const UnitSpec* spec = NULL;

    spec = &UNIT_SPECS[0];
    if (0 <= type && NUM_UNIT_TYPES > type)
    {
        spec = &UNIT_SPECS[type];
    }

    return spec;
}

int CountCrew(GameState* game)
{
    int count = 0;
    int index = 0;

    for (index = 0; MAX_CREW > index; index++)
    {
        if (UNIT_NONE != game->crew[index].type && 0 != game->crew[index].alive)
        {
            count = count + 1;
        }
    }

    return count;
}

int LeadershipCap(GameState* game)
{
    int cap = 0;

    cap = START_LEADERSHIP + game->businessLevel;
    if (RANK_CAPO <= game->rank)
    {
        cap = cap + 2;
    }
    if (RANK_DON == game->rank)
    {
        cap = cap + 3;
    }
    if (MAX_CREW < cap)
    {
        cap = MAX_CREW;
    }

    return cap;
}

int ControlsBusinessType(GameState* game, int type)
{
    int found = 0;
    int index = 0;

    for (index = 0; MAX_BUSINESSES > index; index++)
    {
        if (type == game->businesses[index].type && BIZ_INDEPENDENT != game->businesses[index].status && BIZ_DESTROYED != game->businesses[index].status)
        {
            found = 1;
            break;
        }
    }

    return found;
}

void RollHideout(GameState* game, int rivalIndex)
{
    Rival* rival = NULL;

    rival = &game->rivals[rivalIndex];
    rival->hideoutHoodlums = 1 + RandomRange(game, 3);
    rival->hideoutSoldiers = RandomRange(game, 2) + (rival->strength / 40);
    rival->hideoutScouted = 0;
    rival->hideoutDownDays = 0;
}

void TagMap(App* app, int district, int kind)
{
    GameState* game = NULL;

    game = app->game;
    if (0 > district || NUM_DISTRICTS <= district)
    {
        goto Cleanup;
    }
    game->mapPing[district] = kind;
    game->mapPingDay[district] = game->day;

Cleanup:

    return;
}

Rival* RivalInDistrict(GameState* game, int district)
{
    Rival* rival = NULL;
    int index = 0;

    for (index = 0; NUM_RIVALS > index; index++)
    {
        if (0 != game->rivals[index].alive && district == game->rivals[index].district)
        {
            rival = &game->rivals[index];
            break;
        }
    }

    return rival;
}

void RaiseRivalAnger(App* app, int district, int amount)
{
    GameState* game = NULL;
    Rival* rival = NULL;

    game = app->game;
    rival = RivalInDistrict(game, district);
    if (NULL == rival)
    {
        goto Cleanup;
    }
    rival->anger = rival->anger + amount;
    if (100 < rival->anger)
    {
        rival->anger = 100;
    }
    if (RIVAL_ANGER_WAR <= rival->anger)
    {
        UiLogFmt(app, L"%s has had enough. His district is at war with the Mangano name.", rival->name);
        goto Cleanup;
    }
    if (RIVAL_ANGER_TENSE <= rival->anger)
    {
        UiLogFmt(app, L"Word is %s has been asking pointed questions about you. The %s is getting tense.", rival->name, DistrictName(district));
    }

Cleanup:

    return;
}

void RecruitUnit(App* app, int type, int silent)
{
    GameState* game = NULL;
    int index = 0;
    int slot = -1;
    const UnitSpec* spec = NULL;
    Unit* unit = NULL;

    game = app->game;
    for (index = 0; MAX_CREW > index; index++)
    {
        if (UNIT_NONE == game->crew[index].type || 0 == game->crew[index].alive)
        {
            slot = index;
            break;
        }
    }
    if (0 > slot)
    {
        goto Cleanup;
    }
    spec = GetUnitSpec(type);
    unit = &game->crew[slot];
    unit->type = type;
    unit->alive = 1;
    unit->level = 1;
    unit->xp = 0;
    unit->health = spec->health;
    unit->maxHealth = spec->health;
    unit->accuracy = spec->accuracy;
    unit->assignment = ASSIGN_SQUAD;
    unit->assignedBusiness = -1;
    CopyText(unit->name, RECRUIT_NAMES[RandomRange(game, NUM_RECRUIT_NAMES)], NAME_CHARS);
    if (0 == silent)
    {
        UiLogFmt(app, L"%s the %s joins your crew.", unit->name, spec->title);
    }

Cleanup:

    return;
}

void GainGunplayXp(App* app, int amount)
{
    GameState* game = NULL;

    game = app->game;
    game->gunplayXp = game->gunplayXp + amount;
    while (game->gunplayXp >= (game->gunplayLevel + 1) * LEVEL_XP_STEP)
    {
        game->gunplayXp = game->gunplayXp - ((game->gunplayLevel + 1) * LEVEL_XP_STEP);
        game->gunplayLevel = game->gunplayLevel + 1;
        game->playerMaxHealth = game->playerMaxHealth + GUNPLAY_HP_BONUS;
        game->playerHealth = game->playerMaxHealth;
        UiLogFmt(app, L"Your gunplay sharpens. Gunplay level %d: +%d accuracy, +%d health.", game->gunplayLevel, GUNPLAY_ACC_BONUS, GUNPLAY_HP_BONUS);
    }
}

void GainBusinessXp(App* app, int amount)
{
    GameState* game = NULL;

    game = app->game;
    game->businessXp = game->businessXp + amount;
    while (game->businessXp >= (game->businessLevel + 1) * LEVEL_XP_STEP)
    {
        game->businessXp = game->businessXp - ((game->businessLevel + 1) * LEVEL_XP_STEP);
        game->businessLevel = game->businessLevel + 1;
        UiLogFmt(app, L"Your head for business grows. Business level %d: leadership cap is now %d.", game->businessLevel, LeadershipCap(game));
    }
}

void AddHeat(App* app, int amount)
{
    GameState* game = NULL;
    int scaled = 0;

    game = app->game;
    scaled = amount * game->policePresence[game->location];
    if (0 >= scaled)
    {
        goto Cleanup;
    }
    game->heat = game->heat + scaled;
    if (100 < game->heat)
    {
        game->heat = 100;
    }
    UiLogFmt(app, L"Word of the job spreads. Heat rises to %d.", game->heat);

Cleanup:

    return;
}

void NewGame(App* app)
{
    GameState* game = NULL;
    unsigned int seed = 0;
    int index = 0;

    game = app->game;
    seed = GetTickCount();
    memset(game, 0, sizeof(GameState));
    game->rngSeed = seed;
    game->day = 1;
    game->cash = START_CASH;
    game->playerHealth = START_HEALTH;
    game->playerMaxHealth = START_HEALTH;
    game->location = DISTRICT_LITTLE_ITALY;
    game->vincenzoAlive = 1;
    game->chiefNextDay = CHIEF_FIRST_DAY;
    game->favorDistrict = -1;
    game->contractBusiness = -1;
    game->generation = 1;
    game->inLawBusiness = -1;
    game->kidnapVictim = KIDNAP_NOBODY;
    game->ammo = START_AMMO;
    game->soundOn = 1;
    CopyText(game->playerName, L"Mario Mangano", NAME_CHARS);
    game->policePresence[DISTRICT_LITTLE_ITALY] = 1;
    game->policePresence[DISTRICT_DOWNTOWN] = 3;
    game->policePresence[DISTRICT_DOCKS] = 2;
    game->policePresence[DISTRICT_MARKET] = 2;
    game->policePresence[DISTRICT_SUBURBS] = 0;
    game->rivals[0].alive = 1;
    game->rivals[0].district = DISTRICT_DOCKS;
    game->rivals[0].strength = RIVAL_START_STRENGTH;
    CopyText(game->rivals[0].name, L"Capo Ferro", NAME_CHARS);
    game->rivals[1].alive = 1;
    game->rivals[1].district = DISTRICT_DOWNTOWN;
    game->rivals[1].strength = RIVAL_START_STRENGTH;
    CopyText(game->rivals[1].name, L"Capo Rizzi", NAME_CHARS);
    game->rivals[2].alive = 1;
    game->rivals[2].district = DISTRICT_MARKET;
    game->rivals[2].strength = RIVAL_START_STRENGTH;
    CopyText(game->rivals[2].name, L"Capo Greco", NAME_CHARS);
    for (index = 0; NUM_RIVALS > index; index++)
    {
        RollHideout(game, index);
    }
    for (index = 0; MAX_BUSINESSES > index; index++)
    {
        game->businesses[index].type = BUSINESS_TEMPLATES[index].type;
        game->businesses[index].district = BUSINESS_TEMPLATES[index].district;
        game->businesses[index].baseIncome = BUSINESS_TEMPLATES[index].income;
        game->businesses[index].status = BIZ_INDEPENDENT;
        game->businesses[index].managerAlive = 1;
        CopyText(game->businesses[index].name, BUSINESS_TEMPLATES[index].title, NAME_CHARS);
    }
    RecruitUnit(app, UNIT_GUNMAN, 1);
    SetWindowTextW(app->windowLog, L"");
    UiLog(app, L"PARADISE CITY, 1932.");
    UiLog(app, L"");
    UiLog(app, L"You are Mario Mangano, grandson of Don Mangano of Sicily. Your brother Chico came to this city and came home in a box. Three brothers put him there: Romano, Angelo and Sonny.");
    UiLog(app, L"");
    UiLog(app, L"Your uncle Vincenzo runs a modest outfit out of Little Italy. For now you are his errand boy. Your friend Sal carries a pistol and walks with you.");
    UiLog(app, L"");
    UiLog(app, L"Vincenzo leans over his desk, flanked by two bodyguards. 'Mario. Before you spill blood, you carry crates. A shipment of top-grade ammunition waits at the Docks. Bring it to me.'");
    UiLog(app, L"");
    UiLog(app, L"OBJECTIVE: Travel to the Docks and pick up the ammunition crate.");
}

static int BusinessDailyTake(GameState* game, int index, int hasDistillery)
{
    Business* biz = NULL;
    int take = 0;

    biz = &game->businesses[index];
    if (BIZ_EXTORTED == biz->status)
    {
        take = (biz->baseIncome * EXTORT_VIG_PERCENT) / 100;
    }
    if (BIZ_OWNED == biz->status)
    {
        take = biz->baseIncome;
        if (BIZ_RESTAURANT == biz->type && 0 != hasDistillery)
        {
            take = take + DISTILLERY_SYNERGY;
        }
        if (0 != biz->hasLawyer)
        {
            take = take + ((take * LAWYER_BOOST_PERCENT) / 100);
        }
        if (0 == biz->managerAlive)
        {
            take = (take * MANAGER_DEAD_PENALTY_PERCENT) / 100;
        }
        take = take - OWNED_UPKEEP;
    }
    if (0 > take)
    {
        take = 0;
    }

    return take;
}

static void CollectIncome(App* app)
{
    GameState* game = NULL;
    int index = 0;
    int total = 0;
    int hasDistillery = 0;

    game = app->game;
    hasDistillery = ControlsBusinessType(game, BIZ_DISTILLERY);
    for (index = 0; MAX_BUSINESSES > index; index++)
    {
        total = total + BusinessDailyTake(game, index, hasDistillery);
    }
    if (0 != game->inLawTribute)
    {
        total = total + INLAW_TRIBUTE_INCOME;
    }
    if (0 < total)
    {
        game->cash = game->cash + total;
        UiLogFmt(app, L"The runners bring in $%d from your rackets and businesses.", total);
        GainBusinessXp(app, total / 20);
    }
    if (RANK_CAPO <= game->rank && 0 != game->vincenzoAlive)
    {
        index = (0 != game->vincenzoRescued) ? VINCENZO_STIPEND_RESCUED : CAPO_STIPEND;
        game->cash = game->cash + index;
        UiLogFmt(app, L"Vincenzo pays your cut of the family profits: $%d.%s", index, (0 != game->vincenzoRescued) ? L" A man remembers who pulled him out of a basement." : L"");
    }
    if (0 != ControlsBusinessType(game, BIZ_AMMO))
    {
        game->ammo = game->ammo + AMMO_FROM_STORE;
        UiLogFmt(app, L"Crates from your ammunition store keep the crew supplied (+%d ammo).", AMMO_FROM_STORE);
    }
}

static void CornerTick(App* app)
{
    GameState* game = NULL;
    Unit* unit = NULL;
    int index = 0;
    int earnings = 0;
    int girls = 0;

    game = app->game;
    for (index = 0; MAX_CREW > index; index++)
    {
        unit = &game->crew[index];
        if (UNIT_STREET_GIRL != unit->type || 0 == unit->alive || ASSIGN_CORNER != unit->assignment)
        {
            continue;
        }
        girls = girls + 1;
        earnings = earnings + CORNER_INCOME_LOW + RandomRange(game, CORNER_INCOME_SPAN);
        if (CORNER_SHAKEDOWN_CHANCE > RandomRange(game, 100))
        {
            if (CORNER_SHAKEDOWN_COST <= game->cash)
            {
                game->cash = game->cash - CORNER_SHAKEDOWN_COST;
                UiLogFmt(app, L"A beat cop leans on %s's corner. $%d makes him forget the address.", unit->name, CORNER_SHAKEDOWN_COST);
            }
            else
            {
                game->heat = (100 - 2 > game->heat) ? (game->heat + 2) : 100;
                UiLogFmt(app, L"A beat cop runs %s off her corner and writes your name in his little book.", unit->name);
            }
        }
        if (CORNER_TROUBLE_CHANCE > RandomRange(game, 100))
        {
            unit->health = (CORNER_TROUBLE_HURT < unit->health) ? (unit->health - CORNER_TROUBLE_HURT) : 10;
            unit->assignment = ASSIGN_SQUAD;
            UiLogFmt(app, L"%s comes home the worse for a bad customer. She is off the corner until she says otherwise - and somebody should pay for that.", unit->name);
        }
    }
    if (0 < earnings)
    {
        game->cash = game->cash + earnings;
        game->heat = (100 > game->heat) ? (game->heat + girls) : 100;
        UiLogFmt(app, L"The corners bring in $%d, quiet money with loud consequences.", earnings);
        GainBusinessXp(app, earnings / 20);
    }
}

static void PayWages(App* app)
{
    GameState* game = NULL;
    int wages = 0;
    int index = 0;

    game = app->game;
    wages = CountCrew(game) * WAGE_PER_UNIT;
    if (0 == wages)
    {
        goto Cleanup;
    }
    if (game->cash >= wages)
    {
        game->cash = game->cash - wages;
        UiLogFmt(app, L"You pay the crew their wages: $%d.", wages);
        goto Cleanup;
    }
    for (index = 0; MAX_CREW > index; index++)
    {
        if (UNIT_NONE != game->crew[index].type && 0 != game->crew[index].alive)
        {
            UiLogFmt(app, L"You cannot make payroll. %s spits at your feet and walks.", game->crew[index].name);
            game->crew[index].type = UNIT_NONE;
            game->statMenLost = game->statMenLost + 1;
            break;
        }
    }

Cleanup:

    return;
}

static int PickIndependentTarget(GameState* game)
{
    int picked = -1;
    int index = 0;
    int candidates[MAX_BUSINESSES] = { 0 };
    int candidateCount = 0;

    for (index = 0; MAX_BUSINESSES > index; index++)
    {
        if (BIZ_INDEPENDENT == game->businesses[index].status && DISTRICT_LITTLE_ITALY != game->businesses[index].district)
        {
            candidates[candidateCount] = index;
            candidateCount = candidateCount + 1;
        }
    }
    if (0 < candidateCount)
    {
        picked = candidates[RandomRange(game, candidateCount)];
    }

    return picked;
}

static void RollPhoneOffer(App* app)
{
    GameState* game = NULL;
    int roll = 0;
    int target = -1;

    game = app->game;
    if (0 != game->phone.active || PHONE_CHANCE <= RandomRange(game, 100))
    {
        goto Cleanup;
    }
    game->phone.active = 1;
    roll = RandomRange(game, 3);
    if (0 == roll)
    {
        game->phone.kind = PHONE_UNIT;
        game->phone.unitType = RandomBetween(game, UNIT_BOUNCER, UNIT_BOMBER);
        game->phone.price = (GetUnitSpec(game->phone.unitType)->cost * RandomBetween(game, PHONE_UNIT_PRICE_LOW, PHONE_UNIT_PRICE_HIGH)) / 100;
    }
    if (1 == roll)
    {
        target = PickIndependentTarget(game);
        if (0 > target)
        {
            game->phone.active = 0;
            goto Cleanup;
        }
        game->phone.kind = PHONE_HIT;
        game->phone.targetBusiness = target;
        game->phone.reward = RandomBetween(game, PHONE_HIT_REWARD_LOW, PHONE_HIT_REWARD_HIGH);
    }
    if (2 == roll)
    {
        game->phone.kind = PHONE_TIP;
    }
    UiLog(app, L"The telephone at the safe house is ringing.");

Cleanup:

    return;
}

static void RollFavorJob(App* app)
{
    GameState* game = NULL;

    game = app->game;
    if (0 <= game->favorDistrict || 1 > game->missionStage || FAVOR_CHANCE <= RandomRange(game, 100))
    {
        goto Cleanup;
    }
    game->favorDistrict = RandomRange(game, NUM_DISTRICTS);
    game->favorEnemies = RandomBetween(game, 1, 2);
    UiLogFmt(app, L"A citizen slips into your office, hat in hand. 'Some men in %s are bleeding my family dry. Deal with them and I am yours.' (%d thug%s)", DistrictName(game->favorDistrict), game->favorEnemies, (1 == game->favorEnemies) ? L"" : L"s");

Cleanup:

    return;
}

static void ChiefBusiness(App* app)
{
    GameState* game = NULL;

    game = app->game;
    if (0 == game->policePresence[DISTRICT_DOWNTOWN])
    {
        goto Cleanup;
    }
    if (0 < game->chiefDeadline)
    {
        game->chiefDeadline = game->chiefDeadline - 1;
        if (0 == game->chiefDeadline)
        {
            game->copsHostile = 1;
            UiLog(app, L"You stiffed the Chief of Police. Every badge in Paradise City is now hunting you, and they know where you sleep.");
        }
        goto Cleanup;
    }
    if (game->day >= game->chiefNextDay)
    {
        game->chiefDeadline = CHIEF_GRACE_DAYS;
        game->chiefNextDay = game->day + CHIEF_INTERVAL;
        UiLogFmt(app, L"The Chief of Police strolls into your office, smiling like a shark. 'Insurance is due, Mangano. $%d. You have %d days.'", CHIEF_DEMAND, CHIEF_GRACE_DAYS);
    }

Cleanup:

    return;
}

static void FamilyTick(App* app)
{
    GameState* game = NULL;
    int index = 0;
    int heirSlot = -1;
    int bestTrait = 0;
    int heirType = 0;
    int roll = 0;
    Heir* heir = NULL;

    game = app->game;
    if (WIFE_MARRIED != game->wife.status || 0 >= game->wife.gestationDays)
    {
        goto Cleanup;
    }
    game->wife.gestationDays = game->wife.gestationDays - 1;
    if (0 < game->wife.gestationDays)
    {
        goto Cleanup;
    }
    for (index = 0; MAX_HEIRS > index; index++)
    {
        if (0 == game->heirs[index].exists)
        {
            heirSlot = index;
            break;
        }
    }
    if (0 > heirSlot)
    {
        goto Cleanup;
    }
    bestTrait = game->wife.athletic;
    heirType = HEIR_ENFORCER;
    if (game->wife.clever > bestTrait)
    {
        bestTrait = game->wife.clever;
        heirType = HEIR_LAWYER;
    }
    if (game->wife.charm > bestTrait)
    {
        bestTrait = game->wife.charm;
        heirType = HEIR_SEDUCTRESS;
    }
    roll = RandomRange(game, 100);
    if (HEIR_WILDCARD_CHANCE > roll)
    {
        heirType = RandomRange(game, 3);
    }
    heir = &game->heirs[heirSlot];
    heir->exists = 1;
    heir->type = heirType;
    heir->placement = PLACE_HOME;
    heir->placedBusiness = -1;
    if (HEIR_ENFORCER == heirType)
    {
        heir->quality = HEIR_QUALITY_BASE + (game->wife.athletic * HEIR_QUALITY_PER_TRAIT) + RandomRange(game, HEIR_QUALITY_SPAN);
        CopyText(heir->name, L"Vito", NAME_CHARS);
    }
    if (HEIR_LAWYER == heirType)
    {
        heir->quality = HEIR_QUALITY_BASE + (game->wife.clever * HEIR_QUALITY_PER_TRAIT) + RandomRange(game, HEIR_QUALITY_SPAN);
        CopyText(heir->name, L"Carmela", NAME_CHARS);
    }
    if (HEIR_SEDUCTRESS == heirType)
    {
        heir->quality = HEIR_QUALITY_BASE + (game->wife.charm * HEIR_QUALITY_PER_TRAIT) + RandomRange(game, HEIR_QUALITY_SPAN);
        CopyText(heir->name, L"Isabella", NAME_CHARS);
    }
    if (0 < heirSlot)
    {
        index = lstrlenW(heir->name);
        heir->name[index] = (WCHAR)(L'0' + heirSlot + 1);
        heir->name[index + 1] = L'\0';
    }
    SoundPlay(app, SOUND_BELLS);
    TagMap(app, DISTRICT_LITTLE_ITALY, PING_FAMILY);
    UiLogFmt(app, L"%s gives birth. In time the child grows into %s, an %s of quality %d. Only blood can lead this family.", game->wife.name, heir->name, (HEIR_ENFORCER == heirType) ? L"enforcer" : ((HEIR_LAWYER == heirType) ? L"attorney (lawyer)" : L"operator (seductress)"), heir->quality);
    if (HEIR_DUD_QUALITY > heir->quality)
    {
        UiLog(app, L"The child is, frankly, a disappointment. Blood is a gamble.");
    }

Cleanup:

    return;
}

const WCHAR* KidnapVictimName(GameState* game)
{
    const WCHAR* name = L"nobody";

    if (KIDNAP_WIFE == game->kidnapVictim)
    {
        name = game->wife.name;
    }
    if (0 <= game->kidnapVictim && MAX_HEIRS > game->kidnapVictim)
    {
        name = game->heirs[game->kidnapVictim].name;
    }

    return name;
}

void KidnapVictimDies(App* app)
{
    GameState* game = NULL;

    game = app->game;
    if (KIDNAP_WIFE == game->kidnapVictim)
    {
        game->wife.status = WIFE_DEAD;
        UiLogFmt(app, L"A fisherman finds %s in the harbor at dawn. The ransom went unpaid, and the city collects its debts.", game->wife.name);
    }
    if (0 <= game->kidnapVictim && MAX_HEIRS > game->kidnapVictim)
    {
        UiLogFmt(app, L"They leave %s's coat on your doorstep, folded. Nothing else. Only blood could lead this family - and you let blood slip away.", game->heirs[game->kidnapVictim].name);
        game->heirs[game->kidnapVictim].exists = 0;
    }
    game->kidnapVictim = KIDNAP_NOBODY;
    SoundPlay(app, SOUND_DEATH);
}

static void KidnapTick(App* app)
{
    GameState* game = NULL;
    int candidates[MAX_HEIRS + 1] = { 0 };
    int candidateCount = 0;
    int index = 0;

    game = app->game;
    if (KIDNAP_NOBODY != game->kidnapVictim)
    {
        game->kidnapDaysLeft = game->kidnapDaysLeft - 1;
        if (0 >= game->kidnapDaysLeft)
        {
            KidnapVictimDies(app);
        }
        else
        {
            UiLogFmt(app, L"The kidnappers still hold %s. %d day%s before their patience runs out.", KidnapVictimName(game), game->kidnapDaysLeft, (1 == game->kidnapDaysLeft) ? L"" : L"s");
        }
        goto Cleanup;
    }
    if (3 > game->missionStage || KIDNAP_CHANCE <= RandomRange(game, 100))
    {
        goto Cleanup;
    }
    if (WIFE_MARRIED == game->wife.status)
    {
        candidates[candidateCount] = KIDNAP_WIFE;
        candidateCount = candidateCount + 1;
    }
    for (index = 0; MAX_HEIRS > index; index++)
    {
        if (0 != game->heirs[index].exists)
        {
            candidates[candidateCount] = index;
            candidateCount = candidateCount + 1;
        }
    }
    if (0 == candidateCount)
    {
        goto Cleanup;
    }
    game->kidnapVictim = candidates[RandomRange(game, candidateCount)];
    game->kidnapRansom = KIDNAP_RANSOM_BASE + RandomRange(game, KIDNAP_RANSOM_SPAN);
    game->kidnapDaysLeft = KIDNAP_DAYS;
    game->kidnapDistrict = (0 == RandomRange(game, 2)) ? DISTRICT_DOCKS : DISTRICT_SUBURBS;
    TagMap(app, game->kidnapDistrict, PING_ALERT);
    UiLogFmt(app, L"A black sedan. A sack over the head. They have taken %s. A note demands $%d in %d days - or you send a scout to sniff out the hideout and take the family back with guns.", KidnapVictimName(game), game->kidnapRansom, KIDNAP_DAYS);

Cleanup:

    return;
}

static void MistressTick(App* app)
{
    GameState* game = NULL;

    game = app->game;
    if (0 == game->mistress)
    {
        if (SUSPICION_COOL <= game->wifeSuspicion)
        {
            game->wifeSuspicion = game->wifeSuspicion - SUSPICION_COOL;
        }
        goto Cleanup;
    }
    if (MISTRESS_UPKEEP > game->cash)
    {
        game->mistress = 0;
        game->mistressDays = 0;
        UiLog(app, L"Your mistress does not wait around for broke men. She leaves a perfumed note and an empty jewelry box.");
        goto Cleanup;
    }
    game->cash = game->cash - MISTRESS_UPKEEP;
    game->mistressDays = game->mistressDays + 1;
    if (MISTRESS_TIP_CHANCE > RandomRange(game, 100))
    {
        if (3 == game->missionStage && 0 == game->romanoKnown)
        {
            game->romanoKnown = 1;
            UiLog(app, L"Between cigarettes, your mistress mentions a warehouse at the Docks where Romano's men drink for free.");
        }
        else if (5 == game->missionStage && 0 == game->angeloKnown)
        {
            game->angeloKnown = 1;
            UiLog(app, L"Your mistress hears things. 'Angelo? Above Club Paradiso, darling. Everyone knows.'");
        }
        else if (6 == game->missionStage && 0 == game->sonnyKnown)
        {
            game->sonnyKnown = 1;
            UiLog(app, L"Your mistress traces a road on your chest. 'Out past the Suburbs. A villa. That is where Sonny hides.'");
        }
        else
        {
            game->heat = (MISTRESS_HEAT_RELIEF < game->heat) ? (game->heat - MISTRESS_HEAT_RELIEF) : 0;
            UiLog(app, L"Your mistress whispers which patrolmen can be avoided this week. Useful company.");
        }
    }
    if (WIFE_MARRIED != game->wife.status)
    {
        goto Cleanup;
    }
    game->wifeSuspicion = game->wifeSuspicion + SUSPICION_PER_DAY_BASE + RandomRange(game, SUSPICION_PER_DAY_SPAN);
    if (SUSPICION_LIMIT <= game->wifeSuspicion && 0 == game->wifeConfront)
    {
        game->wifeConfront = 1;
        UiLogFmt(app, L"You come home to find %s awake at the kitchen table, a single lipstick-stained collar in front of her. She knows. She is waiting.", game->wife.name);
    }

Cleanup:

    return;
}

static void InLawTick(App* app)
{
    GameState* game = NULL;

    game = app->game;
    if (WIFE_MARRIED != game->wife.status || 0 > game->inLawBusiness)
    {
        goto Cleanup;
    }
    if (0 == game->inLawTribute && INLAW_ASK_CHANCE > RandomRange(game, 100))
    {
        if (INLAW_ASK_COST <= game->cash)
        {
            game->cash = game->cash - INLAW_ASK_COST;
            game->wifeSuspicion = (SUSPICION_COOL < game->wifeSuspicion) ? (game->wifeSuspicion - SUSPICION_COOL) : 0;
            UiLogFmt(app, L"%s's cousin comes around with troubles and an open palm. You peel off $%d. Family is family - and your wife notices the kindness.", game->wife.name, INLAW_ASK_COST);
        }
        else
        {
            game->wifeSuspicion = game->wifeSuspicion + 10;
            UiLogFmt(app, L"%s's cousin asks for help and you turn him out empty-handed. Dinner is very quiet that night.", game->wife.name);
        }
    }

Cleanup:

    return;
}

static void CounterExtortion(App* app)
{
    GameState* game = NULL;
    int index = 0;
    int chance = 0;
    int guarded[MAX_BUSINESSES] = { 0 };
    Unit* unit = NULL;
    Business* biz = NULL;
    Rival* rival = NULL;

    game = app->game;
    chance = COUNTER_EXTORT_CHANCE;
    rival = AngriestRival(game);
    if (NULL != rival)
    {
        chance = chance + (rival->anger / 10);
    }
    if (1 > game->missionStage || chance <= RandomRange(game, 100))
    {
        goto Cleanup;
    }
    for (index = 0; MAX_CREW > index; index++)
    {
        unit = &game->crew[index];
        if (0 != unit->alive && ASSIGN_BUSINESS == unit->assignment && 0 <= unit->assignedBusiness && MAX_BUSINESSES > unit->assignedBusiness)
        {
            guarded[unit->assignedBusiness] = 1;
        }
    }
    for (index = 0; MAX_BUSINESSES > index; index++)
    {
        biz = &game->businesses[index];
        if (BIZ_EXTORTED != biz->status || DISTRICT_LITTLE_ITALY == biz->district)
        {
            continue;
        }
        if (0 != guarded[index])
        {
            UiLogFmt(app, L"Rival soldiers came sniffing around %s, but your guard sent them off with a look and a lifted coat flap.", biz->name);
            goto Cleanup;
        }
        biz->status = BIZ_INDEPENDENT;
        rival = RivalInDistrict(game, biz->district);
        if (NULL != rival)
        {
            UiLogFmt(app, L"NEWS: %s's men leaned on the owner of %s overnight. He stops paying you. Post a guard next time, or lean on him again.", rival->name, biz->name);
        }
        else
        {
            UiLogFmt(app, L"NEWS: Rivals leaned on the owner of %s overnight. He stops paying you. Post a guard next time, or lean on him again.", biz->name);
        }
        TagMap(app, biz->district, PING_ALERT);
        goto Cleanup;
    }

Cleanup:

    return;
}

static void InformantTick(App* app)
{
    GameState* game = NULL;
    int index = 0;
    int living[MAX_CREW] = { 0 };
    int livingCount = 0;
    Unit* rat = NULL;

    game = app->game;
    if (INFORMANT_HEAT_FLOOR > game->heat || INFORMANT_CHANCE <= RandomRange(game, 100))
    {
        goto Cleanup;
    }
    for (index = 0; MAX_CREW > index; index++)
    {
        if (UNIT_NONE != game->crew[index].type && 0 != game->crew[index].alive)
        {
            living[livingCount] = index;
            livingCount = livingCount + 1;
        }
    }
    if (0 == livingCount)
    {
        goto Cleanup;
    }
    rat = &game->crew[living[RandomRange(game, livingCount)]];
    UiLogFmt(app, L"With the heat this high, somebody talked. You catch %s whispering to a plainclothes cop behind the trattoria. The harbor keeps its secrets, and now it keeps %s.", rat->name, rat->name);
    rat->type = UNIT_NONE;
    game->statMenLost = game->statMenLost + 1;
    game->heat = game->heat + INFORMANT_HEAT_SPIKE;

Cleanup:

    return;
}

static void RivalTick(App* app)
{
    GameState* game = NULL;
    int index = 0;
    Rival* rival = NULL;

    game = app->game;
    for (index = 0; NUM_RIVALS > index; index++)
    {
        rival = &game->rivals[index];
        if (0 < rival->hideoutDownDays)
        {
            rival->hideoutDownDays = rival->hideoutDownDays - 1;
            if (0 == rival->hideoutDownDays)
            {
                RollHideout(game, index);
                UiLogFmt(app, L"Word from %s: new faces have moved back into the old hideout. The well refills.", DistrictName(rival->district));
            }
        }
        if (0 == rival->alive)
        {
            continue;
        }
        rival->anger = (RIVAL_ANGER_DECAY < rival->anger) ? (rival->anger - RIVAL_ANGER_DECAY) : 0;
        if (RIVAL_STRENGTH_CAP > rival->strength)
        {
            rival->strength = rival->strength + 1;
        }
    }
}

static Rival* AngriestRival(GameState* game)
{
    Rival* angriest = NULL;
    int index = 0;

    for (index = 0; NUM_RIVALS > index; index++)
    {
        if (0 == game->rivals[index].alive)
        {
            continue;
        }
        if (NULL == angriest || game->rivals[index].anger > angriest->anger)
        {
            angriest = &game->rivals[index];
        }
    }

    return angriest;
}

static void DistrictEventTick(App* app)
{
    GameState* game = NULL;
    static const WCHAR* const EVENT_TEXT[NUM_DISTRICTS] =
    {
        L"NEWS: The Feast of San Gennaro fills Little Italy. A generous donation to the parade would buy a lot of goodwill.",
        L"NEWS: A society gala Downtown quietly needs discreet protection. Money for a made man who can look respectable.",
        L"NEWS: A freighter sits low and unwatched at the Docks tonight. Cargo like that walks away by itself.",
        L"NEWS: Two vendor families are feuding in the Market Quarter. Backing one side would show the district whose street it is.",
        L"NEWS: A moonshiner in the Suburbs is selling his stock cheap before the still runs dry. Quick money for a quick buyer."
    };

    game = app->game;
    if (0 != game->districtEvent.active)
    {
        game->districtEvent.daysLeft = game->districtEvent.daysLeft - 1;
        if (0 >= game->districtEvent.daysLeft)
        {
            game->districtEvent.active = 0;
            UiLog(app, L"The window on that opportunity has closed. The city moves on.");
        }
        goto Cleanup;
    }
    if (DISTRICT_EVENT_CHANCE <= RandomRange(game, 100))
    {
        goto Cleanup;
    }
    game->districtEvent.active = 1;
    game->districtEvent.district = RandomRange(game, NUM_DISTRICTS);
    game->districtEvent.daysLeft = DISTRICT_EVENT_DAYS;
    UiLog(app, EVENT_TEXT[game->districtEvent.district]);
    TagMap(app, game->districtEvent.district, PING_EVENT);

Cleanup:

    return;
}

static void ExpirePings(GameState* game)
{
    int index = 0;

    for (index = 0; NUM_DISTRICTS > index; index++)
    {
        if (PING_NONE != game->mapPing[index] && PING_LIFETIME_DAYS <= (game->day - game->mapPingDay[index]))
        {
            game->mapPing[index] = PING_NONE;
        }
    }
}

static void VincenzoEvent(App* app)
{
    GameState* game = NULL;

    game = app->game;
    if (0 == game->brothersDead[0] || 0 != game->vincenzoTaken)
    {
        goto Cleanup;
    }
    game->vincenzoTaken = 1;
    game->vincenzoAlive = 0;
    UiLog(app, L"");
    UiLog(app, L"TERRIBLE NEWS. In the night, Angelo's men stormed Vincenzo's club. His gang is wiped out, and your uncle has been dragged away in a black sedan. The stipend is gone. The family is you now.");
    UiLog(app, L"Whatever you build from this day forward, you build alone. Find Angelo.");

Cleanup:

    return;
}

static void NightRaids(App* app)
{
    GameState* game = NULL;
    Rival* angryCapo = NULL;
    int ownsRivalTurf = 0;
    int raidChance = 0;
    int index = 0;

    game = app->game;
    if (0 != game->copsHostile)
    {
        UiLog(app, L"Police wagons screech to a halt outside the safe house. They are coming through the doors.");
        ResolveRaid(app, RAID_COPS);
        game->copsHostile = 0;
        game->chiefDeadline = 0;
        game->chiefNextDay = game->day + (CHIEF_INTERVAL / 2);
        UiLog(app, L"The law considers the message delivered - for now. The Chief will come around again soon enough.");
        goto Cleanup;
    }
    if (0 < game->bountyDays && BOUNTY_AMBUSH_CHANCE > RandomRange(game, 100))
    {
        UiLog(app, L"The price on your head brings hired guns to your door in the dark.");
        ResolveRaid(app, RAID_BOUNTY);
        goto Cleanup;
    }
    for (index = 0; MAX_BUSINESSES > index; index++)
    {
        if (BIZ_INDEPENDENT != game->businesses[index].status && BIZ_DESTROYED != game->businesses[index].status && DISTRICT_LITTLE_ITALY != game->businesses[index].district)
        {
            ownsRivalTurf = 1;
        }
    }
    angryCapo = AngriestRival(game);
    raidChance = RIVAL_RAID_CHANCE;
    if (NULL != angryCapo && RIVAL_RAID_ANGER_FLOOR <= angryCapo->anger)
    {
        raidChance = raidChance + (angryCapo->anger / 5);
        ownsRivalTurf = 1;
    }
    if (1 <= game->missionStage && 0 != ownsRivalTurf && raidChance > RandomRange(game, 100))
    {
        if (NULL != angryCapo && RIVAL_ANGER_TENSE <= angryCapo->anger)
        {
            UiLogFmt(app, L"%s's soldiers storm your safe house to answer for what you have done in %s.", angryCapo->name, DistrictName(angryCapo->district));
        }
        else
        {
            UiLog(app, L"Rival soldiers storm your safe house to teach you a lesson about squeezing their turf.");
        }
        ResolveRaid(app, RAID_RIVALS);
        goto Cleanup;
    }
    if (HEAT_RAID_THRESHOLD <= game->heat && HEAT_RAID_CHANCE > RandomRange(game, 100))
    {
        UiLog(app, L"Your name is too loud on the street. A police squad kicks in the safe house door.");
        ResolveRaid(app, RAID_COPS);
    }

Cleanup:

    return;
}

static void NewsTicker(App* app)
{
    GameState* game = NULL;
    int roll = 0;

    game = app->game;
    if (0 < game->bountyDays)
    {
        game->bountyDays = game->bountyDays - 1;
        if (0 == game->bountyDays)
        {
            UiLog(app, L"NEWS: The bounty on your head has quietly lapsed. The hired guns drift away.");
        }
    }
    roll = RandomRange(game, 100);
    if (NEWS_BOUNTY_CHANCE > roll && 0 == game->bountyDays && 2 <= game->missionStage)
    {
        game->bountyDays = RandomBetween(game, BOUNTY_DAYS_LOW, BOUNTY_DAYS_HIGH);
        UiLogFmt(app, L"NEWS: A price has been put on your head. Expect visitors for the next %d nights.", game->bountyDays);
    }
    if (NEWS_BOUNTY_CHANCE > roll || (NEWS_BOUNTY_CHANCE + NEWS_TIP_CHANCE) <= roll)
    {
        goto Cleanup;
    }
    if (3 == game->missionStage && 0 == game->romanoKnown)
    {
        game->romanoKnown = 1;
        UiLog(app, L"NEWS: A rat whispers that Romano runs his crew from a warehouse at the Docks.");
    }
    if (5 == game->missionStage && 0 == game->angeloKnown)
    {
        game->angeloKnown = 1;
        UiLog(app, L"NEWS: A rat whispers that Angelo holds court above Club Paradiso, Downtown.");
    }
    if (6 == game->missionStage && 0 == game->sonnyKnown)
    {
        game->sonnyKnown = 1;
        UiLog(app, L"NEWS: Sonny has holed up in a fortified villa out in the Suburbs, far from any law.");
    }
    if (6 <= game->missionStage && 0 != game->vincenzoTaken && 0 == game->vincenzoFound && 0 == game->vincenzoRescued && VINCENZO_TIP_CHANCE > RandomRange(game, 100))
    {
        game->vincenzoFound = 1;
        TagMap(app, DISTRICT_DOCKS, PING_ALERT);
        UiLog(app, L"NEWS: A rat swears an old man is being kept alive in a basement off Pier 13 at the Docks. Angelo's men were paid through the month - and nobody told them to stop.");
    }

Cleanup:

    return;
}

void EndDay(App* app)
{
    GameState* game = NULL;
    int decay = 0;
    int index = 0;

    game = app->game;
    game->day = game->day + 1;
    UiLog(app, L"");
    UiLogFmt(app, L"--- Night falls. Day %d dawns over Paradise City. ---", game->day);
    game->location = DISTRICT_LITTLE_ITALY;
    CollectIncome(app);
    CornerTick(app);
    PayWages(app);
    decay = HEAT_DECAY;
    for (index = 0; MAX_HEIRS > index; index++)
    {
        if (0 != game->heirs[index].exists && HEIR_LAWYER == game->heirs[index].type && PLACE_COURTHOUSE == game->heirs[index].placement)
        {
            decay = decay + LAWYER_HEAT_DECAY;
        }
    }
    game->heat = game->heat - decay;
    if (0 > game->heat)
    {
        game->heat = 0;
    }
    for (index = 0; NUM_DISTRICTS > index; index++)
    {
        if (0 < game->intimidation[index])
        {
            game->intimidation[index] = game->intimidation[index] - 1;
        }
    }
    ExpirePings(game);
    RivalTick(app);
    ChiefBusiness(app);
    NightRaids(app);
    if (0 != game->gameOver)
    {
        goto Cleanup;
    }
    DistrictEventTick(app);
    MistressTick(app);
    InLawTick(app);
    KidnapTick(app);
    CounterExtortion(app);
    InformantTick(app);
    FamilyTick(app);
    VincenzoEvent(app);
    RollPhoneOffer(app);
    RollFavorJob(app);
    NewsTicker(app);
    if (game->playerHealth < game->playerMaxHealth)
    {
        game->playerHealth = game->playerHealth + DAILY_REST_HEAL;
        if (game->playerHealth > game->playerMaxHealth)
        {
            game->playerHealth = game->playerMaxHealth;
        }
    }
    SaveGame(app, AUTOSAVE_SLOT, 1);

Cleanup:

    return;
}

void BuildStatusText(App* app)
{
    GameState* game = NULL;
    WCHAR* buffer = NULL;
    WCHAR* cursor = NULL;
    int remaining = 0;
    int written = 0;
    int rivalIndex = 0;

    game = app->game;
    buffer = (WCHAR*)AllocZeroed(STATUS_BUFFER_CHARS * sizeof(WCHAR));
    if (NULL == buffer)
    {
        goto Cleanup;
    }
    cursor = buffer;
    remaining = STATUS_BUFFER_CHARS;
    written = wnsprintfW(cursor, remaining, L"%s - %s\r\nDay %d   Location: %s\r\n", game->playerName, (RANK_ERRAND_BOY == game->rank) ? L"Errand Boy" : ((RANK_CAPO == game->rank) ? L"Capo Soto" : L"Don"), game->day, DistrictName(game->location));
    cursor = cursor + written;
    remaining = remaining - written;
    written = wnsprintfW(cursor, remaining, L"Cash: $%d   Heat: %d%s\r\nHealth: %d/%d   Medpacks: %d\r\nGunplay Lv %d   Business Lv %d\r\nCrew: %d / %d (leadership)\r\n", game->cash, game->heat, (0 != game->copsHostile) ? L" [COPS HOSTILE]" : L"", game->playerHealth, game->playerMaxHealth, game->medpacks, game->gunplayLevel, game->businessLevel, CountCrew(game), LeadershipCap(game));
    cursor = cursor + written;
    remaining = remaining - written;
    written = wnsprintfW(cursor, remaining, L"Weapon: %s   Ammo: %d\r\nCar: %s\r\n", (0 == game->weaponTier) ? L"Old pistol" : ((1 == game->weaponTier) ? L"Twin pistols" : ((2 == game->weaponTier) ? L"Tommy gun" : L"Custom Tommy gun")), game->ammo, (CAR_NONE == game->carTier) ? L"None (you walk)" : ((CAR_JALOPY == game->carTier) ? L"Rusty jalopy" : ((CAR_SEDAN == game->carTier) ? L"Black sedan" : L"Armored sedan")));
    cursor = cursor + written;
    remaining = remaining - written;
    if (WIFE_MARRIED == game->wife.status)
    {
        written = wnsprintfW(cursor, remaining, L"Wife: %s%s\r\n", game->wife.name, (0 != game->wifeConfront) ? L" [SHE KNOWS]" : ((SUSPICION_VISIBLE <= game->wifeSuspicion) ? L" [suspicious]" : L""));
        cursor = cursor + written;
        remaining = remaining - written;
    }
    if (0 != game->mistress)
    {
        written = wnsprintfW(cursor, remaining, L"A mistress waits Downtown ($%d/day)\r\n", MISTRESS_UPKEEP);
        cursor = cursor + written;
        remaining = remaining - written;
    }
    if (KIDNAP_NOBODY != game->kidnapVictim)
    {
        written = wnsprintfW(cursor, remaining, L"KIDNAPPED: %s ($%d ransom, %d days)\r\n", KidnapVictimName(game), game->kidnapRansom, game->kidnapDaysLeft);
        cursor = cursor + written;
        remaining = remaining - written;
    }
    if (0 < game->chiefDeadline)
    {
        written = wnsprintfW(cursor, remaining, L"CHIEF WANTS $%d (%d days left)\r\n", CHIEF_DEMAND, game->chiefDeadline);
        cursor = cursor + written;
        remaining = remaining - written;
    }
    if (0 < game->bountyDays)
    {
        written = wnsprintfW(cursor, remaining, L"BOUNTY ON YOUR HEAD (%d days)\r\n", game->bountyDays);
        cursor = cursor + written;
        remaining = remaining - written;
    }
    for (rivalIndex = 0; NUM_RIVALS > rivalIndex; rivalIndex++)
    {
        if (0 == game->rivals[rivalIndex].alive || RIVAL_ANGER_TENSE > game->rivals[rivalIndex].anger)
        {
            continue;
        }
        cursor = cursor + wnsprintfW(cursor, remaining, L"%s (%s): %s\r\n", game->rivals[rivalIndex].name, DistrictName(game->rivals[rivalIndex].district), (RIVAL_ANGER_WAR <= game->rivals[rivalIndex].anger) ? L"AT WAR" : L"tense");
        remaining = (int)(STATUS_BUFFER_CHARS - (cursor - buffer));
    }
    wnsprintfW(cursor, remaining, L"\r\nBrothers avenged: %d/3", game->brothersDead[0] + game->brothersDead[1] + game->brothersDead[2]);
    SetWindowTextW(app->windowStatus, buffer);

Cleanup:
    FreeMemory(buffer);

    return;
}

static void BuildSavePath(WCHAR* path, int slot)
{
    if (AUTOSAVE_SLOT == slot)
    {
        wnsprintfW(path, PATH_BUFFER_CHARS, L"gangland_autosave.sav");
    }
    else
    {
        wnsprintfW(path, PATH_BUFFER_CHARS, L"gangland_slot%d.sav", slot + 1);
    }
}

int PeekSave(int slot, GameState* peek)
{
    int valid = 0;
    WCHAR* path = NULL;
    HANDLE file = INVALID_HANDLE_VALUE;
    DWORD bytesRead = 0;
    DWORD header[2] = { 0 };
    BOOL ok = FALSE;

    path = (WCHAR*)AllocZeroed(PATH_BUFFER_CHARS * sizeof(WCHAR));
    if (NULL == path || NULL == peek)
    {
        goto Cleanup;
    }
    BuildSavePath(path, slot);
    file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == file)
    {
        goto Cleanup;
    }
    ok = ReadFile(file, header, sizeof(header), &bytesRead, NULL);
    if (FALSE == ok || sizeof(header) != bytesRead || SAVE_MAGIC != header[0] || SAVE_VERSION != header[1])
    {
        goto Cleanup;
    }
    ok = ReadFile(file, peek, sizeof(GameState), &bytesRead, NULL);
    if (FALSE == ok || sizeof(GameState) != bytesRead)
    {
        goto Cleanup;
    }
    valid = 1;

Cleanup:
    if (INVALID_HANDLE_VALUE != file)
    {
        CloseHandle(file);
    }
    FreeMemory(path);

    return valid;
}

void SaveGame(App* app, int slot, int silent)
{
    GameState* game = NULL;
    WCHAR* path = NULL;
    HANDLE file = INVALID_HANDLE_VALUE;
    DWORD bytesWritten = 0;
    DWORD header[2] = { SAVE_MAGIC, SAVE_VERSION };
    BOOL ok = FALSE;

    game = app->game;
    path = (WCHAR*)AllocZeroed(PATH_BUFFER_CHARS * sizeof(WCHAR));
    if (NULL == path)
    {
        goto Cleanup;
    }
    BuildSavePath(path, slot);
    file = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == file)
    {
        if (0 == silent)
        {
            UiLog(app, L"Could not open the save file.");
        }
        goto Cleanup;
    }
    ok = WriteFile(file, header, sizeof(header), &bytesWritten, NULL);
    if (FALSE != ok)
    {
        ok = WriteFile(file, game, sizeof(GameState), &bytesWritten, NULL);
    }
    if (FALSE == ok)
    {
        if (0 == silent)
        {
            UiLog(app, L"The save failed.");
        }
        goto Cleanup;
    }
    if (0 == silent)
    {
        UiLogFmt(app, L"Game saved to slot %d. You can save anywhere - this family learned from 1932's mistakes.", slot + 1);
    }

Cleanup:
    if (INVALID_HANDLE_VALUE != file)
    {
        CloseHandle(file);
    }
    FreeMemory(path);

    return;
}

void LoadGame(App* app, int slot)
{
    WCHAR* path = NULL;
    HANDLE file = INVALID_HANDLE_VALUE;
    DWORD bytesRead = 0;
    DWORD header[2] = { 0 };
    GameState* loaded = NULL;
    BOOL ok = FALSE;

    path = (WCHAR*)AllocZeroed(PATH_BUFFER_CHARS * sizeof(WCHAR));
    loaded = (GameState*)AllocZeroed(sizeof(GameState));
    if (NULL == path || NULL == loaded)
    {
        goto Cleanup;
    }
    BuildSavePath(path, slot);
    file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == file)
    {
        UiLogFmt(app, L"There is no saved game in slot %d.", slot + 1);
        goto Cleanup;
    }
    ok = ReadFile(file, header, sizeof(header), &bytesRead, NULL);
    if (FALSE == ok || sizeof(header) != bytesRead || SAVE_MAGIC != header[0] || SAVE_VERSION != header[1])
    {
        UiLog(app, L"That save file is from another life. It cannot be loaded.");
        goto Cleanup;
    }
    ok = ReadFile(file, loaded, sizeof(GameState), &bytesRead, NULL);
    if (FALSE == ok || sizeof(GameState) != bytesRead)
    {
        UiLog(app, L"The save file is damaged.");
        goto Cleanup;
    }
    memcpy(app->game, loaded, sizeof(GameState));
    app->game->menuId = MENU_MAIN;
    app->game->pending.active = 0;
    UiLog(app, L"");
    UiLogFmt(app, L"--- Save loaded. Day %d, %s. The story continues. ---", app->game->day, DistrictName(app->game->location));

Cleanup:
    if (INVALID_HANDLE_VALUE != file)
    {
        CloseHandle(file);
    }
    FreeMemory(path);
    FreeMemory(loaded);

    return;
}
