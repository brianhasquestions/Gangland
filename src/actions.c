#include "gangland.h"

#define CHIEF_DEMAND 3000
#define NEWSMAN_FEE 20
#define MANAGER_HIRE_COST 200
#define MEDPACK_COST 120
#define MEDPACK_HEAL 60
#define BOMB_COST 500
#define BOMB_HEAT 40
#define BRIBE_COST 800
#define BRIBE_HEAT_RELIEF 20
#define COURT_MIN_CASH 2000
#define DOWRY_BASE 3000
#define DOWRY_PER_TRAIT 800
#define DOWRY_MIN 4000
#define DOWRY_MAX 18000
#define GESTATION_DAYS 5
#define WEDDING_AMBUSH_CHANCE 30
#define WEDDING_WIDOW_CHANCE 40
#define TRAVEL_AMBUSH_CHANCE 15
#define CHURCH_FIND_CHANCE 35
#define CRATE_AMBUSH_CHANCE 40
#define CRATE_REWARD 250
#define TAILOR_REWARD 300
#define MISSION_TAILOR_BIZ 9
#define WEAPON_COST_TIER1 800
#define WEAPON_COST_TIER2 2000
#define WEAPON_COST_TIER3 4500
#define DESK_DISCOUNT_PERCENT 20
#define MISTRESS_MEET_COST 500
#define MISTRESS_UPKEEP 60
#define MISTRESS_GIFT_COST 500
#define MISTRESS_GIFT_RELIEF 30
#define SWEAR_GIFT_COST 1000
#define DENY_LEAVE_CHANCE 50
#define DISMISS_LEAVE_CHANCE 65
#define INLAW_TRIBUTE_UPFRONT 400
#define INLAW_TRIBUTE_SUSPICION 25
#define CAR_COST_JALOPY 600
#define CAR_COST_SEDAN 1500
#define CAR_COST_ARMORED 3500
#define CAR_STEAL_CHANCE 60
#define AMMO_BOX_COST 100
#define AMMO_BOX_ROUNDS 5
#define BIZ_BOMB_COST 300
#define INTIMIDATION_DAYS 5
#define TRAVEL_AMBUSH_CAR_CHANCE 7
#define RICH_BIZ_INCOME 250
#define DOWRY_JITTER 1000
#define WOO_COST 150
#define WOO_MIN 300
#define WOO_SPAN 401
#define DOWRY_FLOOR 2000
#define SITDOWN_BASE 500
#define SITDOWN_PER_ANGER 10
#define SITDOWN_RELIEF 45
#define ANGER_EXTORT_INTIMIDATED 8
#define ANGER_BOMB 25
#define FEAST_COST 300
#define FEAST_HEAT_RELIEF 15
#define FEAST_SUSPICION_RELIEF 10
#define FEAST_HEAL 20
#define GALA_REWARD 500
#define CARGO_SUCCESS_CHANCE 70
#define CARGO_REWARD_LOW 400
#define CARGO_REWARD_SPAN 501
#define VENDOR_COST 200
#define VENDOR_INTIMIDATION 3
#define MOONSHINE_COST 350
#define MOONSHINE_RETURN 900
#define MOONSHINE_AMMO 2
#define ARM_COST 600
#define TUTOR_COST 200
#define TUTOR_GAIN_BASE 2
#define TUTOR_GAIN_SPAN 3
#define TUTOR_QUALITY_CAP 95

static const WCHAR* const CANDIDATE_NAMES[] = { L"Lucia Ferrante", L"Teresa Bonanno", L"Gina Castellano", L"Sofia Marchetti", L"Rosa DiNapoli", L"Bianca Greco" };
#define NUM_CANDIDATE_NAMES 6

static int HeirPlaced(GameState* game, int type, int placement)
{
    int found = 0;
    int index = 0;

    for (index = 0; MAX_HEIRS > index; index++)
    {
        if (0 != game->heirs[index].exists && type == game->heirs[index].type && placement == game->heirs[index].placement)
        {
            found = 1;
        }
    }

    return found;
}

static void DoLook(App* app)
{
    GameState* game = NULL;
    int here = 0;
    int index = 0;
    Business* biz = NULL;

    game = app->game;
    here = game->location;
    UiLog(app, L"");
    UiLogFmt(app, L"You take a slow walk through %s.", DistrictName(here));
    if (0 == game->policePresence[here])
    {
        UiLog(app, L"Not a badge in sight out here. A man could get away with murder.");
    }
    if (3 <= game->policePresence[here])
    {
        UiLog(app, L"Cops on every corner. Anything loud here will bring the law down fast.");
    }
    if (1 == game->policePresence[here] || 2 == game->policePresence[here])
    {
        UiLog(app, L"A patrolman strolls past now and then. Manageable, if you are quick.");
    }
    for (index = 0; MAX_BUSINESSES > index; index++)
    {
        biz = &game->businesses[index];
        if (biz->district == here && BIZ_DESTROYED != biz->status)
        {
            UiLogFmt(app, L"  %s - takes in about $%d a day.", biz->name, biz->baseIncome);
        }
    }
    if (DISTRICT_SUBURBS == here && 0 == game->churchFound && CHURCH_FIND_CHANCE > RandomRange(game, 100))
    {
        game->churchFound = 1;
        UiLog(app, L"Between the quiet houses you spot a small white church. Good to know, for a man thinking about family.");
    }
}

static void DoShowJobs(App* app)
{
    GameState* game = NULL;

    game = app->game;
    UiLog(app, L"");
    UiLog(app, L"YOUR OBJECTIVES:");
    if (0 == game->missionStage)
    {
        UiLog(app, L"- Fetch Vincenzo's ammunition crate from the Docks and bring it home.");
    }
    if (1 == game->missionStage)
    {
        UiLog(app, L"- Vincenzo wants Bellini's Tailors in the Market Quarter paying protection. Extort or seize it.");
    }
    if (2 == game->missionStage)
    {
        UiLog(app, L"- Three of Romano's hoodlums are squeezing Vincenzo's friend at the Docks. Deal with them.");
    }
    if (3 == game->missionStage && 0 == game->romanoKnown)
    {
        UiLog(app, L"- Find Romano. Somebody in this city knows where he sleeps - rats talk on the phone, and the newspaper man hears things.");
    }
    if (3 == game->missionStage && 0 != game->romanoKnown)
    {
        UiLog(app, L"- Romano runs his crew from a warehouse at the Docks. Pay him a visit.");
    }
    if (5 == game->missionStage && 0 == game->angeloKnown)
    {
        UiLog(app, L"- Angelo took your uncle. Find out where he holds court.");
    }
    if (5 == game->missionStage && 0 != game->angeloKnown)
    {
        UiLog(app, L"- Angelo holds court above Club Paradiso, Downtown. Bring plenty of guns.");
    }
    if (6 == game->missionStage && 0 == game->sonnyKnown)
    {
        UiLog(app, L"- Only Sonny is left. He has gone to ground somewhere.");
    }
    if (6 == game->missionStage && 0 != game->sonnyKnown)
    {
        UiLog(app, L"- Sonny waits in a fortified villa in the Suburbs. Finish it.");
    }
    if (7 <= game->missionStage)
    {
        UiLog(app, L"- Chico is avenged. Paradise City is yours to run as you see fit.");
    }
    if (0 <= game->favorDistrict)
    {
        UiLogFmt(app, L"- FAVOR: %d thugs troubling a citizen in %s. Kill them and the citizen joins you.", game->favorEnemies, DistrictName(game->favorDistrict));
    }
    if (0 <= game->contractBusiness)
    {
        UiLogFmt(app, L"- CONTRACT: $%d waiting for the death of the manager of %s.", game->contractReward, game->businesses[game->contractBusiness].name);
    }
}

static void DoTravel(App* app, int district)
{
    GameState* game = NULL;
    int ambushChance = 0;

    game = app->game;
    game->location = district;
    UiLog(app, L"");
    if (CAR_NONE == game->carTier)
    {
        UiLogFmt(app, L"You make your way to %s on foot.", DistrictName(district));
    }
    else
    {
        UiLogFmt(app, L"The %s into %s.", (CAR_JALOPY == game->carTier) ? L"jalopy rattles" : L"sedan glides", DistrictName(district));
    }
    ambushChance = (CAR_NONE == game->carTier) ? TRAVEL_AMBUSH_CHANCE : TRAVEL_AMBUSH_CAR_CHANCE;
    if (CAR_ARMORED == game->carTier)
    {
        ambushChance = ambushChance / 2;
    }
    if (0 < game->bountyDays && ambushChance > RandomRange(game, 100))
    {
        UiLog(app, L"Bounty hunters step out of an alley with guns drawn.");
        ResolveRaid(app, RAID_BOUNTY);
    }
    game->menuId = MENU_MAIN;
}

static void WifeLeaves(App* app)
{
    GameState* game = NULL;

    game = app->game;
    game->wife.status = WIFE_LEFT;
    game->wifeConfront = 0;
    game->wifeSuspicion = 0;
    game->inLawTribute = 0;
    game->inLawBusiness = -1;
    if (game->bountyDays < 3)
    {
        game->bountyDays = 3;
    }
    UiLogFmt(app, L"%s packs one suitcase and walks out into the rain. By morning, half the city knows your routes and your habits - a scorned wife talks, and rivals pay well to listen.", game->wife.name);
}

static void DoMistressMeet(App* app)
{
    GameState* game = NULL;

    game = app->game;
    if (DISTRICT_DOWNTOWN != game->location)
    {
        UiLog(app, L"That kind of company is found at the Downtown social club, behind the velvet rope.");
        goto Cleanup;
    }
    if (MISTRESS_MEET_COST > game->cash)
    {
        UiLogFmt(app, L"Champagne, a private booth, a first gift - it takes $%d to even begin. You are short.", MISTRESS_MEET_COST);
        goto Cleanup;
    }
    game->cash = game->cash - MISTRESS_MEET_COST;
    game->mistress = 1;
    game->mistressDays = 0;
    UiLog(app, L"");
    UiLogFmt(app, L"A singer from the club takes your arm and does not let go. She costs $%d a day, hears everything worth hearing - and if %s ever finds out, God help you.", MISTRESS_UPKEEP, game->wife.name);

Cleanup:

    return;
}

static void DoConfront(App* app, int choice)
{
    GameState* game = NULL;

    game = app->game;
    if (ACT_CONFRONT_SWEAR == choice)
    {
        if (SWEAR_GIFT_COST > game->cash)
        {
            UiLog(app, L"You swear it is over, but your empty hands say otherwise. She looks right through you.");
            WifeLeaves(app);
            goto Cleanup;
        }
        game->cash = game->cash - SWEAR_GIFT_COST;
        game->mistress = 0;
        game->mistressDays = 0;
        game->wifeSuspicion = 0;
        game->wifeConfront = 0;
        UiLogFmt(app, L"You end the affair that night and put $%d of penance jewelry on the kitchen table. %s says nothing for three days - then sets your plate at dinner again.", SWEAR_GIFT_COST, game->wife.name);
        goto Cleanup;
    }
    if (ACT_CONFRONT_DENY == choice)
    {
        if (DENY_LEAVE_CHANCE > RandomRange(game, 100))
        {
            WifeLeaves(app);
            goto Cleanup;
        }
        game->wifeSuspicion = 40;
        game->wifeConfront = 0;
        UiLogFmt(app, L"You lie beautifully. %s wants to believe you, and that is enough - for now. The collar goes into the stove, but not the memory.", game->wife.name);
        goto Cleanup;
    }
    if (DISMISS_LEAVE_CHANCE > RandomRange(game, 100))
    {
        WifeLeaves(app);
        goto Cleanup;
    }
    game->wifeSuspicion = 80;
    game->wifeConfront = 0;
    UiLogFmt(app, L"'Mind your place,' you tell her. %s stays - this is the life she married into - but something behind her eyes closes for good.", game->wife.name);

Cleanup:
    game->menuId = MENU_MAIN;

    return;
}

static void DoInLawTribute(App* app, int startTribute)
{
    GameState* game = NULL;

    game = app->game;
    if (0 != startTribute)
    {
        game->inLawTribute = 1;
        game->cash = game->cash + INLAW_TRIBUTE_UPFRONT;
        game->wifeSuspicion = game->wifeSuspicion + INLAW_TRIBUTE_SUSPICION;
        UiLogFmt(app, L"You send two men to your in-laws at %s. Family or not, everyone pays. They hand over $%d and a weekly envelope - and word of it reaches %s before supper.", game->businesses[game->inLawBusiness].name, INLAW_TRIBUTE_UPFRONT, game->wife.name);
        goto Cleanup;
    }
    game->inLawTribute = 0;
    game->wifeSuspicion = (15 < game->wifeSuspicion) ? (game->wifeSuspicion - 15) : 0;
    UiLogFmt(app, L"You call off the envelope. %s's people fly the family colors for free again, and the dinner table thaws a little.", game->wife.name);

Cleanup:

    return;
}

static void DoRansomPay(App* app)
{
    GameState* game = NULL;

    game = app->game;
    if (game->cash < game->kidnapRansom)
    {
        UiLogFmt(app, L"The ransom is $%d and you cannot raise it. Find the money, or find the hideout.", game->kidnapRansom);
        goto Cleanup;
    }
    game->cash = game->cash - game->kidnapRansom;
    game->statRansomsPaid = game->statRansomsPaid + 1;
    UiLogFmt(app, L"The exchange happens under the harbor bridge at midnight. %s comes home shaken but whole. The money is gone; the insult is not forgotten.", KidnapVictimName(game));
    game->kidnapVictim = KIDNAP_NOBODY;
    SoundPlay(app, SOUND_BELLS);

Cleanup:

    return;
}

static void DoRescue(App* app)
{
    GameState* game = NULL;
    int index = 0;
    int hasScout = 0;

    game = app->game;
    for (index = 0; MAX_CREW > index; index++)
    {
        if (UNIT_SCOUT == game->crew[index].type && 0 != game->crew[index].alive)
        {
            hasScout = 1;
        }
    }
    if (0 == hasScout)
    {
        UiLog(app, L"Nobody knows where they are holding the family. A scout could find the hideout.");
        goto Cleanup;
    }
    game->location = game->kidnapDistrict;
    UiLogFmt(app, L"Your scout traces the sedan to a boarded-up building in %s. Lamplight through the slats. Voices. It is now or never.", DistrictName(game->kidnapDistrict));
    memset(&game->pending, 0, sizeof(PendingCombat));
    game->pending.kind = FIGHT_RESCUE;
    game->pending.soldiers = 2;
    game->pending.veterans = 1;
    if (3000 < game->kidnapRansom)
    {
        game->pending.soldiers = 3;
    }
    QueueCombat(app, L"storm the kidnappers' hideout");

Cleanup:

    return;
}

static void DoCarBuy(App* app, int tier)
{
    GameState* game = NULL;
    int cost = 0;

    game = app->game;
    cost = CAR_COST_JALOPY;
    if (CAR_SEDAN == tier)
    {
        cost = CAR_COST_SEDAN;
    }
    if (CAR_ARMORED == tier)
    {
        cost = CAR_COST_ARMORED;
    }
    if (game->cash < cost)
    {
        UiLog(app, L"The dealer pats the hood and names a price your pockets cannot meet.");
        goto Cleanup;
    }
    game->cash = game->cash - cost;
    game->carTier = tier;
    UiLogFmt(app, L"%s", (CAR_JALOPY == tier) ? L"A rusty jalopy, but it runs. Fewer ambushes catch a moving target." : ((CAR_SEDAN == tier) ? L"A black sedan with a deep back seat. Built for drive-bys and quick exits." : L"An armored sedan - steel plates in the doors, glass an inch thick. Let them try."));

Cleanup:

    return;
}

static void DoCarSteal(App* app)
{
    GameState* game = NULL;

    game = app->game;
    if (CAR_STEAL_CHANCE > RandomRange(game, 100))
    {
        game->carTier = CAR_JALOPY;
        UiLog(app, L"A parked jalopy, a bent wire, thirty seconds. The city provides for those who take.");
        AddHeat(app, 2);
        goto Cleanup;
    }
    UiLog(app, L"The engine coughs and dies, a whistle blows somewhere, and you walk away fast with your collar up.");
    AddHeat(app, 1);

Cleanup:

    return;
}

static void DoSitdown(App* app, int rivalIndex)
{
    GameState* game = NULL;
    Rival* rival = NULL;
    int cost = 0;

    game = app->game;
    rival = &game->rivals[rivalIndex];
    cost = SITDOWN_BASE + (rival->anger * SITDOWN_PER_ANGER);
    if (game->cash < cost)
    {
        UiLogFmt(app, L"A sit-down with %s means wine, a private room, and $%d in respect money. You are short.", rival->name, cost);
        goto Cleanup;
    }
    game->cash = game->cash - cost;
    rival->anger = (SITDOWN_RELIEF < rival->anger) ? (rival->anger - SITDOWN_RELIEF) : 0;
    UiLogFmt(app, L"You break bread with %s in the back of a restaurant that officially seats nobody. $%d changes hands, old insults are toasted away, and the soldiers stand down - for now.", rival->name, cost);

Cleanup:

    return;
}

static void DoHitCapo(App* app, int rivalIndex)
{
    GameState* game = NULL;
    Rival* rival = NULL;

    game = app->game;
    rival = &game->rivals[rivalIndex];
    memset(&game->pending, 0, sizeof(PendingCombat));
    game->pending.kind = FIGHT_CAPO;
    game->pending.arg = rivalIndex;
    game->pending.soldiers = 2 + (rival->strength / 30);
    game->pending.veterans = 1 + (rival->strength / 45);
    game->pending.bosses = 1;
    UiLogFmt(app, L"%s holds court in a back room in %s. He is expecting trouble - just not this much of it.", rival->name, DistrictName(rival->district));
    QueueCombat(app, L"take the capo off the board");
}

static void DoDistrictEvent(App* app)
{
    GameState* game = NULL;
    int district = 0;
    int index = 0;
    int reward = 0;

    game = app->game;
    district = game->districtEvent.district;
    if (0 == game->districtEvent.active || district != game->location)
    {
        goto Cleanup;
    }
    if (DISTRICT_LITTLE_ITALY == district)
    {
        if (FEAST_COST > game->cash)
        {
            goto Cleanup;
        }
        game->cash = game->cash - FEAST_COST;
        game->heat = (FEAST_HEAT_RELIEF < game->heat) ? (game->heat - FEAST_HEAT_RELIEF) : 0;
        game->wifeSuspicion = (FEAST_SUSPICION_RELIEF < game->wifeSuspicion) ? (game->wifeSuspicion - FEAST_SUSPICION_RELIEF) : 0;
        for (index = 0; MAX_CREW > index; index++)
        {
            if (UNIT_NONE != game->crew[index].type && 0 != game->crew[index].alive)
            {
                game->crew[index].health = game->crew[index].health + FEAST_HEAL;
                if (game->crew[index].health > game->crew[index].maxHealth)
                {
                    game->crew[index].health = game->crew[index].maxHealth;
                }
            }
        }
        UiLogFmt(app, L"You march at the head of the Feast of San Gennaro with $%d pinned to the saint. The neighborhood eats, the cops look kindly on you, and the crew comes home fat and happy.", FEAST_COST);
    }
    if (DISTRICT_DOWNTOWN == district)
    {
        game->cash = game->cash + GALA_REWARD;
        GainBusinessXp(app, 30);
        UiLogFmt(app, L"Your men in rented tuxedos keep the gala's jewels on the right wrists. The host presses $%d into your palm and asks no questions about your tailor.", GALA_REWARD);
    }
    if (DISTRICT_DOCKS == district)
    {
        if (CARGO_SUCCESS_CHANCE > RandomRange(game, 100))
        {
            reward = CARGO_REWARD_LOW + RandomRange(game, CARGO_REWARD_SPAN);
            game->cash = game->cash + reward;
            GainBusinessXp(app, 25);
            UiLogFmt(app, L"The freighter's cargo walks off the pier and into your warehouse before the harbormaster finishes his coffee. Resale: $%d.", reward);
        }
        else
        {
            UiLog(app, L"A harbor patrol rounds the pier mid-heist. You leave the crates and melt into the fog, whistling.");
            AddHeat(app, 3);
        }
    }
    if (DISTRICT_MARKET == district)
    {
        if (VENDOR_COST > game->cash)
        {
            goto Cleanup;
        }
        game->cash = game->cash - VENDOR_COST;
        game->intimidation[DISTRICT_MARKET] = game->intimidation[DISTRICT_MARKET] + VENDOR_INTIMIDATION;
        GainBusinessXp(app, 20);
        UiLog(app, L"You back the bigger vendor family with cash and a visible show of muscle. The feud ends in an afternoon, and every stall in the Market Quarter takes note of whose street this is.");
    }
    if (DISTRICT_SUBURBS == district)
    {
        if (MOONSHINE_COST > game->cash)
        {
            goto Cleanup;
        }
        game->cash = (game->cash - MOONSHINE_COST) + MOONSHINE_RETURN;
        game->ammo = game->ammo + MOONSHINE_AMMO;
        UiLogFmt(app, L"You buy the moonshiner's stock for $%d and wholesale it to thirsty restaurants by sundown for $%d. He throws in a crate of shells as a tip.", MOONSHINE_COST, MOONSHINE_RETURN);
    }
    game->districtEvent.active = 0;
    TagMap(app, district, PING_MONEY);
    SoundPlay(app, SOUND_CASH);

Cleanup:

    return;
}

static void DoBizBomb(App* app)
{
    GameState* game = NULL;
    Business* biz = NULL;

    game = app->game;
    biz = &game->businesses[game->ctxBusiness];
    if (BIZ_BOMB_COST > game->cash)
    {
        UiLogFmt(app, L"Dynamite costs $%d. Come back with money to burn.", BIZ_BOMB_COST);
        goto Cleanup;
    }
    game->cash = game->cash - BIZ_BOMB_COST;
    biz->status = BIZ_DESTROYED;
    game->intimidation[biz->district] = INTIMIDATION_DAYS;
    SoundPlay(app, SOUND_GUNFIGHT);
    UiLogFmt(app, L"%s goes up in a ball of fire that rattles every window in %s. For days, no shopkeeper in the district will need more than a look from you.", biz->name, DistrictName(biz->district));
    RaiseRivalAnger(app, biz->district, ANGER_BOMB);
    TagMap(app, biz->district, PING_FIGHT);
    if (game->ctxBusiness == game->inLawBusiness)
    {
        game->inLawBusiness = -1;
        game->inLawTribute = 0;
        game->wifeSuspicion = game->wifeSuspicion + 50;
        UiLogFmt(app, L"It was %s's family's place. She will never forgive this. Never.", game->wife.name);
    }
    AddHeat(app, 8);
    game->menuId = MENU_RACKETS;

Cleanup:

    return;
}

static void DoExtort(App* app, int takeover)
{
    GameState* game = NULL;
    Business* biz = NULL;

    game = app->game;
    biz = &game->businesses[game->ctxBusiness];
    if (0 == takeover && 0 < game->intimidation[biz->district])
    {
        biz->status = BIZ_EXTORTED;
        UiLogFmt(app, L"The owner of %s saw what was left of the last place that said no. He agrees to the arrangement before you finish your sentence.", biz->name);
        GainBusinessXp(app, 40);
        game->statBizSeized = game->statBizSeized + 1;
        RaiseRivalAnger(app, biz->district, ANGER_EXTORT_INTIMIDATED);
        TagMap(app, biz->district, PING_MONEY);
        if (1 == game->missionStage && MISSION_TAILOR_BIZ == game->ctxBusiness)
        {
            game->missionStage = 2;
            game->cash = game->cash + TAILOR_REWARD;
            UiLogFmt(app, L"Vincenzo is pleased. '$%d for a job done clean. One more thing: three of Romano's hoodlums are squeezing an old friend of mine at the Docks. Send them to the bottom of the harbor.'", TAILOR_REWARD);
        }
        game->menuId = MENU_RACKETS;
        goto Cleanup;
    }
    memset(&game->pending, 0, sizeof(PendingCombat));
    game->pending.kind = (0 != takeover) ? FIGHT_TAKEOVER : FIGHT_EXTORT;
    game->pending.arg = game->ctxBusiness;
    game->pending.hoodlums = RandomBetween(game, 1, 2);
    if (0 != takeover)
    {
        game->pending.hoodlums = 2;
        game->pending.soldiers = 1;
        if (RICH_BIZ_INCOME <= biz->baseIncome)
        {
            game->pending.soldiers = 2;
        }
    }
    if (3 <= game->missionStage && (DISTRICT_DOCKS == biz->district || DISTRICT_DOWNTOWN == biz->district))
    {
        game->pending.soldiers = game->pending.soldiers + 1;
    }
    QueueCombat(app, (0 != takeover) ? L"seize the place" : L"shake down the owner");

Cleanup:

    return;
}

static void DoMission(App* app)
{
    GameState* game = NULL;

    game = app->game;
    memset(&game->pending, 0, sizeof(PendingCombat));
    game->pending.kind = FIGHT_MISSION;
    game->pending.arg = game->missionStage;
    if (2 == game->missionStage)
    {
        game->pending.hoodlums = 3;
        QueueCombat(app, L"hit Romano's hoodlums");
        goto Cleanup;
    }
    if (3 == game->missionStage)
    {
        game->pending.soldiers = 2;
        game->pending.veterans = 1;
        game->pending.bosses = 1;
        QueueCombat(app, L"raid Romano's warehouse");
        goto Cleanup;
    }
    if (5 == game->missionStage)
    {
        game->pending.soldiers = 4;
        game->pending.veterans = 2;
        game->pending.bosses = 1;
        QueueCombat(app, L"storm Club Paradiso");
        goto Cleanup;
    }
    if (6 == game->missionStage)
    {
        game->pending.soldiers = 2;
        game->pending.veterans = 5;
        game->pending.bosses = 1;
        QueueCombat(app, L"assault Sonny's villa");
    }

Cleanup:

    return;
}

static void DoFavor(App* app)
{
    GameState* game = NULL;

    game = app->game;
    memset(&game->pending, 0, sizeof(PendingCombat));
    game->pending.kind = FIGHT_FAVOR;
    game->pending.hoodlums = game->favorEnemies;
    QueueCombat(app, L"settle the citizen's problem");
}

static void DoContract(App* app)
{
    GameState* game = NULL;

    game = app->game;
    memset(&game->pending, 0, sizeof(PendingCombat));
    game->pending.kind = FIGHT_CONTRACT;
    game->pending.arg = game->contractBusiness;
    game->pending.hoodlums = 2;
    game->pending.soldiers = 1;
    QueueCombat(app, L"carry out the contract");
}

static void DoCratePickup(App* app)
{
    GameState* game = NULL;

    game = app->game;
    game->carryingCrate = 1;
    UiLog(app, L"");
    UiLog(app, L"The dockworker hands over a heavy crate of top-grade ammunition and looks the other way.");
    if (CRATE_AMBUSH_CHANCE > RandomRange(game, 100))
    {
        UiLog(app, L"Two hoodlums block the alley. 'Nice crate. Leave it.'");
        memset(&game->pending, 0, sizeof(PendingCombat));
        game->pending.kind = FIGHT_CRATE_AMBUSH;
        game->pending.hoodlums = 2;
        QueueCombat(app, L"protect the crate");
        goto Cleanup;
    }
    UiLog(app, L"You get it to the car without trouble. Vincenzo is waiting in Little Italy.");

Cleanup:

    return;
}

static void DoCrateDeliver(App* app)
{
    GameState* game = NULL;

    game = app->game;
    game->carryingCrate = 0;
    game->missionStage = 1;
    game->cash = game->cash + CRATE_REWARD;
    GainBusinessXp(app, 30);
    UiLog(app, L"");
    UiLogFmt(app, L"Vincenzo pries the crate open and smiles. 'Good boy, Mario.' He counts out $%d.", CRATE_REWARD);
    UiLog(app, L"'Now something with teeth. Bellini's Tailors in the Market Quarter refuses to pay respect. Lean on them - a cut of their take, or take the whole shop, I leave the method to you.'");
}

static void DoCourt(App* app)
{
    GameState* game = NULL;
    int index = 0;
    Candidate* candidate = NULL;

    game = app->game;
    if (DISTRICT_DOWNTOWN != game->location)
    {
        UiLog(app, L"The social club is Downtown. Powerful men are seen there; go where the eyes are.");
        goto Cleanup;
    }
    if (RANK_CAPO > game->rank || COURT_MIN_CASH > game->cash)
    {
        UiLog(app, L"The women at the club look straight through you. Come back when you have a name and money to burn.");
        goto Cleanup;
    }
    UiLog(app, L"");
    UiLog(app, L"You take a table at the social club. Three women take an interest in the young capo from Little Italy.");
    for (index = 0; MAX_CANDIDATES > index; index++)
    {
        candidate = &game->candidates[index];
        candidate->active = 1;
        candidate->athletic = RandomBetween(game, 1, 5);
        candidate->clever = RandomBetween(game, 1, 5);
        candidate->charm = RandomBetween(game, 1, 5);
        candidate->dowry = DOWRY_BASE + ((candidate->athletic + candidate->clever + candidate->charm) * DOWRY_PER_TRAIT) + RandomRange(game, DOWRY_JITTER);
        if (DOWRY_MIN > candidate->dowry)
        {
            candidate->dowry = DOWRY_MIN;
        }
        if (DOWRY_MAX < candidate->dowry)
        {
            candidate->dowry = DOWRY_MAX;
        }
        CopyText(candidate->name, CANDIDATE_NAMES[RandomRange(game, NUM_CANDIDATE_NAMES)], NAME_CHARS);
    }
    UiLog(app, L"Each expects to be won properly - courtship in this city is paid in cash. Choose under Family matters. Who you marry shapes the children you will raise.");

Cleanup:

    return;
}

static void DoCourtWoo(App* app, int index)
{
    GameState* game = NULL;
    Candidate* candidate = NULL;
    int charmed = 0;

    game = app->game;
    candidate = &game->candidates[index];
    if (DISTRICT_DOWNTOWN != game->location)
    {
        UiLog(app, L"Courting is done at the social club Downtown, over wine and a slow band.");
        goto Cleanup;
    }
    if (game->courtedDay == game->day)
    {
        UiLog(app, L"One evening, one woman. Even a capo only has so much charm in a day.");
        goto Cleanup;
    }
    if (WOO_COST > game->cash)
    {
        UiLogFmt(app, L"Dinner, dancing and a corsage run $%d, and you are short.", WOO_COST);
        goto Cleanup;
    }
    game->cash = game->cash - WOO_COST;
    game->courtedDay = game->day;
    charmed = WOO_MIN + RandomRange(game, WOO_SPAN);
    candidate->wooed = candidate->wooed + charmed;
    if (DOWRY_FLOOR > (candidate->dowry - candidate->wooed))
    {
        candidate->wooed = candidate->dowry - DOWRY_FLOOR;
    }
    UiLogFmt(app, L"An evening of wine and dancing with %s. Her price in gold falls as her opinion of you rises - the family will now settle for $%d.", candidate->name, candidate->dowry - candidate->wooed);

Cleanup:

    return;
}

static void DoCourtPay(App* app, int index)
{
    GameState* game = NULL;
    Candidate* candidate = NULL;
    int askingPrice = 0;

    game = app->game;
    candidate = &game->candidates[index];
    askingPrice = candidate->dowry - candidate->wooed;
    if (game->cash < askingPrice)
    {
        UiLogFmt(app, L"You cannot raise the $%d she expects. She smiles politely and turns away. Perhaps a few more evenings together would soften the family's asking price.", askingPrice);
        goto Cleanup;
    }
    game->cash = game->cash - askingPrice;
    game->wife.status = WIFE_ENGAGED;
    game->wife.athletic = candidate->athletic;
    game->wife.clever = candidate->clever;
    game->wife.charm = candidate->charm;
    CopyText(game->wife.name, candidate->name, NAME_CHARS);
    memset(game->candidates, 0, sizeof(game->candidates));
    UiLog(app, L"");
    UiLogFmt(app, L"%s accepts your proposal. She is precious now - and fragile. Hostile turf is no place for a bride. You will need a church, and the Suburbs are said to have one.", game->wife.name);

Cleanup:

    return;
}

static void DoScoutChurch(App* app)
{
    GameState* game = NULL;
    int index = 0;
    int hasScout = 0;

    game = app->game;
    for (index = 0; MAX_CREW > index; index++)
    {
        if (UNIT_SCOUT == game->crew[index].type && 0 != game->crew[index].alive)
        {
            hasScout = 1;
        }
    }
    if (0 == hasScout)
    {
        UiLog(app, L"You have no scout. Hire one, or walk the Suburbs yourself and hope.");
        goto Cleanup;
    }
    game->churchFound = 1;
    UiLog(app, L"Your scout comes back before nightfall: a small white church in the Suburbs, quiet street, no law anywhere near it. Perfect.");

Cleanup:

    return;
}

static void DoWedding(App* app)
{
    GameState* game = NULL;
    int ambushChance = 0;
    int candidates[MAX_BUSINESSES] = { 0 };
    int candidateCount = 0;
    int index = 0;

    game = app->game;
    game->location = DISTRICT_SUBURBS;
    ambushChance = WEDDING_AMBUSH_CHANCE;
    if (0 < game->bountyDays)
    {
        ambushChance = ambushChance + 25;
    }
    if (0 != game->churchFound)
    {
        ambushChance = ambushChance - 10;
    }
    if (CAR_NONE != game->carTier)
    {
        ambushChance = ambushChance - 10;
    }
    UiLog(app, L"");
    UiLogFmt(app, L"You escort %s across the city with your squad around her, watching every rooftop.", game->wife.name);
    if (ambushChance > RandomRange(game, 100))
    {
        UiLog(app, L"Gunfire erupts near the church steps - somebody sold out the wedding route!");
        ResolveRaid(app, RAID_BOUNTY);
        if (0 != game->gameOver)
        {
            goto Cleanup;
        }
        if (WEDDING_WIDOW_CHANCE > RandomRange(game, 100))
        {
            game->wife.status = WIFE_DEAD;
            UiLogFmt(app, L"When the smoke clears, %s lies still on the church steps in her white dress. Some debts cannot be collected in money.", game->wife.name);
            goto Cleanup;
        }
        UiLogFmt(app, L"Your men shielded %s with their bodies. Shaken, she still takes your hand at the altar.", game->wife.name);
    }
    game->wife.status = WIFE_MARRIED;
    SoundPlay(app, SOUND_BELLS);
    TagMap(app, DISTRICT_SUBURBS, PING_FAMILY);
    UiLogFmt(app, L"Bells ring over the Suburbs. %s Mangano leaves the church on your arm. The family has a future - if you can protect it.", game->wife.name);
    for (index = 0; MAX_BUSINESSES > index; index++)
    {
        if (BIZ_INDEPENDENT == game->businesses[index].status)
        {
            candidates[candidateCount] = index;
            candidateCount = candidateCount + 1;
        }
    }
    if (0 < candidateCount)
    {
        game->inLawBusiness = candidates[RandomRange(game, candidateCount)];
    }
    if (0 <= game->inLawBusiness)
    {
        UiLogFmt(app, L"Her people run %s in %s. In-laws now - you can shield them for nothing, or squeeze them like anyone else. Blood makes it complicated.", game->businesses[game->inLawBusiness].name, DistrictName(game->businesses[game->inLawBusiness].district));
    }

Cleanup:

    return;
}

static void DoRomance(App* app)
{
    GameState* game = NULL;
    int index = 0;
    int freeSlot = 0;

    game = app->game;
    for (index = 0; MAX_HEIRS > index; index++)
    {
        if (0 == game->heirs[index].exists)
        {
            freeSlot = 1;
        }
    }
    if (0 == freeSlot)
    {
        UiLog(app, L"Three children is enough for any dynasty. The house is full.");
        goto Cleanup;
    }
    game->wife.gestationDays = GESTATION_DAYS;
    UiLogFmt(app, L"You spend a quiet evening with %s, away from the guns. In %d days you may hear good news.", game->wife.name, GESTATION_DAYS);

Cleanup:

    return;
}

static void DoHeirPlace(App* app, int placement)
{
    GameState* game = NULL;
    Heir* heir = NULL;

    game = app->game;
    heir = &game->heirs[game->ctxUnit];
    if (PLACE_BUSINESS == heir->placement && 0 <= heir->placedBusiness)
    {
        game->businesses[heir->placedBusiness].hasLawyer = 0;
        heir->placedBusiness = -1;
    }
    heir->placement = placement;
    UiLogFmt(app, L"%s is now %s.", heir->name, (PLACE_HOME == placement) ? L"home at the safe house" : ((PLACE_FIELD == placement) ? L"working in the field" : ((PLACE_DESK == placement) ? L"minding the phone desk" : L"working the courthouse")));
    game->menuId = MENU_FAMILY;
}

static void DoPlaceLawyer(App* app, int heirIndex)
{
    GameState* game = NULL;
    Heir* heir = NULL;

    game = app->game;
    heir = &game->heirs[heirIndex];
    heir->placement = PLACE_BUSINESS;
    heir->placedBusiness = game->ctxBusiness;
    game->businesses[game->ctxBusiness].hasLawyer = 1;
    UiLogFmt(app, L"%s sets up an office in the back of %s. The books get creative; the revenue climbs.", heir->name, game->businesses[game->ctxBusiness].name);
    game->menuId = MENU_RACKETS;
}

static void DoAnswerPhone(App* app)
{
    GameState* game = NULL;
    int discount = 0;

    game = app->game;
    if (0 != HeirPlaced(game, HEIR_SEDUCTRESS, PLACE_DESK))
    {
        discount = 1;
    }
    UiLog(app, L"");
    if (PHONE_TIP == game->phone.kind)
    {
        UiLog(app, L"A whispering voice - one of your rats. 'Word travels, Mangano. Listen close.'");
        if (3 == game->missionStage && 0 == game->romanoKnown)
        {
            game->romanoKnown = 1;
            UiLog(app, L"'Romano beds down in a warehouse at the Docks. You did not hear it from me.'");
        }
        else if (5 == game->missionStage && 0 == game->angeloKnown)
        {
            game->angeloKnown = 1;
            UiLog(app, L"'Angelo runs everything from above Club Paradiso, Downtown. Bring friends.'");
        }
        else if (6 == game->missionStage && 0 == game->sonnyKnown)
        {
            game->sonnyKnown = 1;
            UiLog(app, L"'Sonny bought himself a villa in the Suburbs. No cops out there - and no witnesses either.'");
        }
        else
        {
            UiLogFmt(app, L"'The Chief comes sniffing for his envelope around day %d. Keep $%d where you can reach it.'", game->chiefNextDay, CHIEF_DEMAND);
        }
        game->phone.active = 0;
        goto Cleanup;
    }
    if (PHONE_UNIT == game->phone.kind)
    {
        if (0 != discount)
        {
            game->phone.price = (game->phone.price * (100 - DESK_DISCOUNT_PERCENT)) / 100;
            UiLogFmt(app, L"Isabella purrs into the receiver before handing it over. The price drops to $%d.", game->phone.price);
        }
        UiLogFmt(app, L"A gravelly voice: 'Got a %s looking for work. $%d and he is yours, no questions.'", GetUnitSpec(game->phone.unitType)->title, game->phone.price);
        game->menuId = MENU_PHONE;
        goto Cleanup;
    }
    if (0 != discount)
    {
        game->phone.reward = (game->phone.reward * (100 + DESK_DISCOUNT_PERCENT)) / 100;
    }
    UiLogFmt(app, L"A cold voice: 'The manager of %s has become inconvenient. $%d when he stops breathing.'", game->businesses[game->phone.targetBusiness].name, game->phone.reward);
    game->menuId = MENU_PHONE;

Cleanup:

    return;
}

static void DoPhoneAccept(App* app)
{
    GameState* game = NULL;

    game = app->game;
    if (PHONE_UNIT == game->phone.kind)
    {
        if (game->cash < game->phone.price)
        {
            UiLog(app, L"You cannot cover the price. The line goes dead.");
            goto Cleanup;
        }
        if (LeadershipCap(game) <= CountCrew(game))
        {
            UiLog(app, L"You cannot lead another man yet. The line goes dead.");
            goto Cleanup;
        }
        game->cash = game->cash - game->phone.price;
        RecruitUnit(app, game->phone.unitType, 0);
        goto Cleanup;
    }
    game->contractBusiness = game->phone.targetBusiness;
    game->contractReward = game->phone.reward;
    UiLogFmt(app, L"'Good. The manager of %s. Make it quiet or make it loud, just make it final.'", game->businesses[game->contractBusiness].name);

Cleanup:
    memset(&game->phone, 0, sizeof(PhoneOffer));
    game->menuId = MENU_MAIN;

    return;
}

static void DoNewsman(App* app)
{
    GameState* game = NULL;

    game = app->game;
    if (NEWSMAN_FEE > game->cash)
    {
        UiLog(app, L"Even the newspaper man will not extend you credit.");
        goto Cleanup;
    }
    game->cash = game->cash - NEWSMAN_FEE;
    UiLog(app, L"");
    UiLog(app, L"The newspaper man palms your coins and talks out of the side of his mouth:");
    if (0 < game->bountyDays)
    {
        UiLogFmt(app, L"'There is paper on your head, Mangano - another %d days of it. Sleep light.'", game->bountyDays);
    }
    if (0 == game->bountyDays)
    {
        UiLog(app, L"'Nobody is paying for your funeral this week. Enjoy it.'");
    }
    if (3 == game->missionStage && 0 == game->romanoKnown)
    {
        game->romanoKnown = 1;
        UiLog(app, L"'Romano? A warehouse at the Docks. Everybody knows except the cops, and they are paid not to.'");
    }
    if (5 == game->missionStage && 0 == game->angeloKnown)
    {
        game->angeloKnown = 1;
        UiLog(app, L"'Angelo sits above Club Paradiso like a king. Kings fall, kid.'");
    }
    if (6 == game->missionStage && 0 == game->sonnyKnown)
    {
        game->sonnyKnown = 1;
        UiLog(app, L"'Sonny ran to a villa in the Suburbs. No law out there. Nothing to stop you but his guns.'");
    }
    UiLogFmt(app, L"'The Chief collects again around day %d. And the suburbs are still sweet and lawless, if you need air.'", game->chiefNextDay);

Cleanup:

    return;
}

static void DoBombStation(App* app)
{
    GameState* game = NULL;

    game = app->game;
    if (BOMB_COST > game->cash)
    {
        UiLogFmt(app, L"Dynamite costs $%d and your pockets are light.", BOMB_COST);
        goto Cleanup;
    }
    game->cash = game->cash - BOMB_COST;
    game->policePresence[game->location] = 0;
    game->heat = game->heat + BOMB_HEAT;
    if (100 < game->heat)
    {
        game->heat = 100;
    }
    if (DISTRICT_DOWNTOWN == game->location)
    {
        game->copsHostile = 0;
        game->chiefDeadline = 0;
    }
    UiLog(app, L"");
    UiLogFmt(app, L"Your bomber whistles as he works. The %s police station goes up in a pillar of fire and paperwork. No law will walk this district again.", DistrictName(game->location));
    UiLog(app, L"NEWS: The whole city is talking about it. That kind of fame is its own kind of trouble.");

Cleanup:

    return;
}

static void DoRecruit(App* app, int type)
{
    GameState* game = NULL;
    const UnitSpec* spec = NULL;

    game = app->game;
    spec = GetUnitSpec(type);
    if (game->cash < spec->cost)
    {
        UiLogFmt(app, L"A %s costs $%d. Come back with real money.", spec->title, spec->cost);
        goto Cleanup;
    }
    if (LeadershipCap(game) <= CountCrew(game))
    {
        UiLogFmt(app, L"You can only lead %d. Grow your head for business, or earn a bigger title.", LeadershipCap(game));
        goto Cleanup;
    }
    game->cash = game->cash - spec->cost;
    RecruitUnit(app, type, 0);

Cleanup:

    return;
}

static void DoMedpack(App* app)
{
    GameState* game = NULL;
    int index = 0;
    int worst = -1;
    int worstRatio = 100;
    int ratio = 0;
    Unit* unit = NULL;

    game = app->game;
    if (0 >= game->medpacks)
    {
        goto Cleanup;
    }
    ratio = (game->playerHealth * 100) / game->playerMaxHealth;
    if (ratio < worstRatio)
    {
        worstRatio = ratio;
        worst = -1;
    }
    for (index = 0; MAX_CREW > index; index++)
    {
        unit = &game->crew[index];
        if (UNIT_NONE == unit->type || 0 == unit->alive)
        {
            continue;
        }
        ratio = (unit->health * 100) / unit->maxHealth;
        if (ratio < worstRatio)
        {
            worstRatio = ratio;
            worst = index;
        }
    }
    game->medpacks = game->medpacks - 1;
    if (0 > worst)
    {
        game->playerHealth = game->playerHealth + MEDPACK_HEAL;
        if (game->playerHealth > game->playerMaxHealth)
        {
            game->playerHealth = game->playerMaxHealth;
        }
        UiLog(app, L"You stitch yourself up with the medpack.");
        goto Cleanup;
    }
    unit = &game->crew[worst];
    unit->health = unit->health + MEDPACK_HEAL;
    if (unit->health > unit->maxHealth)
    {
        unit->health = unit->maxHealth;
    }
    UiLogFmt(app, L"You patch up %s with the medpack.", unit->name);

Cleanup:

    return;
}

static void DoUnitAssign(App* app, int arg)
{
    GameState* game = NULL;
    Unit* unit = NULL;
    int unitIndex = 0;
    int assignment = ASSIGN_SQUAD;
    const WCHAR* description = NULL;

    game = app->game;
    unitIndex = arg;
    if (MAX_CREW <= arg && (2 * MAX_CREW) > arg)
    {
        unitIndex = arg - MAX_CREW;
        assignment = ASSIGN_SAFEHOUSE;
    }
    if ((2 * MAX_CREW) <= arg)
    {
        unitIndex = arg - (2 * MAX_CREW);
        assignment = ASSIGN_CORNER;
    }
    unit = &game->crew[unitIndex];
    unit->assignment = assignment;
    unit->assignedBusiness = -1;
    description = L"walking at your side";
    if (ASSIGN_SAFEHOUSE == assignment)
    {
        description = L"watching the safe house doors and windows";
    }
    if (ASSIGN_CORNER == assignment)
    {
        description = L"working a corner - quiet money, loud consequences";
    }
    UiLogFmt(app, L"%s is now %s.", unit->name, description);
    game->menuId = MENU_MAIN;
}

static void DoLegend(App* app)
{
    GameState* game = NULL;
    int index = 0;
    int heirsRaised = 0;

    game = app->game;
    for (index = 0; MAX_HEIRS > index; index++)
    {
        if (0 != game->heirs[index].exists)
        {
            heirsRaised = heirsRaised + 1;
        }
    }
    UiLog(app, L"");
    UiLog(app, L"YOUR LEGEND, AS THE CITY TELLS IT:");
    UiLogFmt(app, L"- Generation %d of the Mangano family. Day %d in Paradise City.", game->generation, game->day);
    UiLogFmt(app, L"- %d enemies put in the ground. %d of your own buried, turned, or walked.", game->statKills, game->statMenLost);
    UiLogFmt(app, L"- %d gunfights won, %d lost or abandoned.", game->statFightsWon, game->statFightsLost);
    UiLogFmt(app, L"- %d businesses brought under the family's wing. %d rival capos retired permanently.", game->statBizSeized, game->statCaposKilled);
    UiLogFmt(app, L"- Ransoms paid: %d. Heirs alive: %d. Brothers avenged: %d of 3.", game->statRansomsPaid, heirsRaised, game->brothersDead[0] + game->brothersDead[1] + game->brothersDead[2]);
    UiLogFmt(app, L"- Cash on hand: $%d. Heat: %d. %s", game->cash, game->heat, (0 != game->gameWon) ? L"The vendetta is settled." : L"The vendetta is unfinished.");
}

static void DoArmCrew(App* app)
{
    GameState* game = NULL;
    Unit* unit = NULL;
    Unit* best = NULL;
    int index = 0;

    game = app->game;
    if (ARM_COST > game->cash)
    {
        UiLogFmt(app, L"A crew Tommy gun runs $%d. The dealer does not do layaway.", ARM_COST);
        goto Cleanup;
    }
    for (index = 0; MAX_CREW > index; index++)
    {
        unit = &game->crew[index];
        if (UNIT_NONE == unit->type || 0 == unit->alive || 0 != unit->armed)
        {
            continue;
        }
        if (NULL == best || unit->level > best->level)
        {
            best = unit;
        }
    }
    if (NULL == best)
    {
        UiLog(app, L"Every man you have is already carrying a Tommy gun.");
        goto Cleanup;
    }
    game->cash = game->cash - ARM_COST;
    best->armed = 1;
    UiLogFmt(app, L"%s walks out with a violin case and a new spring in his step. One more gun in every fight.", best->name);

Cleanup:

    return;
}

static void DoScoutHideout(App* app, int rivalIndex)
{
    GameState* game = NULL;
    Rival* rival = NULL;
    int index = 0;
    int hasScout = 0;

    game = app->game;
    rival = &game->rivals[rivalIndex];
    for (index = 0; MAX_CREW > index; index++)
    {
        if (UNIT_SCOUT == game->crew[index].type && 0 != game->crew[index].alive)
        {
            hasScout = 1;
        }
    }
    if (0 == hasScout)
    {
        UiLog(app, L"You need a scout to count guns through a window without getting counted back.");
        goto Cleanup;
    }
    rival->hideoutScouted = 1;
    UiLogFmt(app, L"Your scout comes back chewing a matchstick. 'The hideout? %d hoodlum%s, %d soldier%s. Back door has a bad hinge.'", rival->hideoutHoodlums, (1 == rival->hideoutHoodlums) ? L"" : L"s", rival->hideoutSoldiers, (1 == rival->hideoutSoldiers) ? L"" : L"s");

Cleanup:

    return;
}

static void DoRaidHideout(App* app, int rivalIndex)
{
    GameState* game = NULL;
    Rival* rival = NULL;

    game = app->game;
    rival = &game->rivals[rivalIndex];
    memset(&game->pending, 0, sizeof(PendingCombat));
    game->pending.kind = FIGHT_HIDEOUT;
    game->pending.arg = rivalIndex;
    game->pending.hoodlums = rival->hideoutHoodlums;
    game->pending.soldiers = rival->hideoutSoldiers;
    if (0 == rival->hideoutScouted)
    {
        game->pending.soldiers = game->pending.soldiers + 1;
        UiLog(app, L"You go in blind. There are always more guns than the street admits.");
    }
    QueueCombat(app, L"clean out the hideout");
}

static void DoHeirTutor(App* app, int heirIndex)
{
    GameState* game = NULL;
    Heir* heir = NULL;
    int gain = 0;

    game = app->game;
    heir = &game->heirs[heirIndex];
    if (game->tutoredDay == game->day)
    {
        UiLog(app, L"One lesson a day. Even a Mangano child needs time to be a child.");
        goto Cleanup;
    }
    if (TUTOR_COST > game->cash)
    {
        UiLogFmt(app, L"Good teachers cost $%d, and yours is an expensive family already.", TUTOR_COST);
        goto Cleanup;
    }
    if (TUTOR_QUALITY_CAP <= heir->quality)
    {
        UiLogFmt(app, L"%s has nothing left to learn from hired teachers. The rest is blood and years.", heir->name);
        goto Cleanup;
    }
    game->cash = game->cash - TUTOR_COST;
    game->tutoredDay = game->day;
    gain = TUTOR_GAIN_BASE + RandomRange(game, TUTOR_GAIN_SPAN);
    heir->quality = heir->quality + gain;
    if (TUTOR_QUALITY_CAP < heir->quality)
    {
        heir->quality = TUTOR_QUALITY_CAP;
    }
    if (HEIR_ENFORCER == heir->type)
    {
        UiLogFmt(app, L"A retired heavyweight spends the afternoon teaching %s footwork and how to take a punch without blinking. Quality %d.", heir->name, heir->quality);
    }
    else if (HEIR_LAWYER == heir->type)
    {
        UiLogFmt(app, L"A disbarred judge tutors %s in contracts, loopholes, and which clerks drink. Quality %d.", heir->name, heir->quality);
    }
    else
    {
        UiLogFmt(app, L"A retired opera singer teaches %s how to enter a room so that nobody ever forgets it. Quality %d.", heir->name, heir->quality);
    }

Cleanup:

    return;
}

static void DoRescueVincenzo(App* app)
{
    GameState* game = NULL;

    game = app->game;
    memset(&game->pending, 0, sizeof(PendingCombat));
    game->pending.kind = FIGHT_VINCENZO;
    game->pending.soldiers = 3;
    game->pending.veterans = 2;
    UiLog(app, L"The basement door off Pier 13 is guarded by men whose employer is dead and whose pay ran out. Desperate men. Careless men.");
    QueueCombat(app, L"bring Vincenzo home");
}

static void GoBack(App* app)
{
    GameState* game = NULL;

    game = app->game;
    switch (game->menuId)
    {
    case MENU_BIZ:
        game->menuId = MENU_RACKETS;
        break;
    case MENU_BIZ_GUARD_PICK:
        game->menuId = MENU_BIZ;
        break;
    case MENU_UNIT:
        game->menuId = MENU_CREW;
        break;
    case MENU_RECRUIT:
        game->menuId = MENU_CREW;
        break;
    case MENU_HEIR:
        game->menuId = MENU_FAMILY;
        break;
    default:
        game->menuId = MENU_MAIN;
        break;
    }
}

void HandleAction(App* app, int packedAction)
{
    GameState* game = NULL;
    int action = 0;
    int arg = 0;

    game = app->game;
    action = packedAction % ACTION_ARG_BASE;
    arg = packedAction / ACTION_ARG_BASE;
    if (0 != game->gameOver && ACT_LOAD != action && ACT_NEW_GAME != action)
    {
        goto Cleanup;
    }
    switch (action)
    {
    case ACT_BACK:
        GoBack(app);
        break;
    case ACT_LOOK:
        DoLook(app);
        break;
    case ACT_TRAVEL_MENU:
        game->menuId = MENU_TRAVEL;
        break;
    case ACT_TRAVEL:
        DoTravel(app, arg);
        break;
    case ACT_SHOW_JOBS:
        DoShowJobs(app);
        break;
    case ACT_RACKETS_MENU:
        game->menuId = MENU_RACKETS;
        break;
    case ACT_BIZ_MENU:
        game->ctxBusiness = arg;
        game->menuId = MENU_BIZ;
        break;
    case ACT_BIZ_EXTORT:
        DoExtort(app, 0);
        break;
    case ACT_BIZ_TAKEOVER:
        DoExtort(app, 1);
        break;
    case ACT_BIZ_GUARD_MENU:
        game->menuId = MENU_BIZ_GUARD_PICK;
        break;
    case ACT_BIZ_GUARD:
        game->crew[arg].assignment = ASSIGN_BUSINESS;
        game->crew[arg].assignedBusiness = game->ctxBusiness;
        UiLogFmt(app, L"%s takes up a post at %s.", game->crew[arg].name, game->businesses[game->ctxBusiness].name);
        game->menuId = MENU_BIZ;
        break;
    case ACT_BIZ_HIRE_MANAGER:
        if (MANAGER_HIRE_COST <= game->cash)
        {
            game->cash = game->cash - MANAGER_HIRE_COST;
            game->businesses[game->ctxBusiness].managerAlive = 1;
            UiLog(app, L"A nervous new manager takes the keys. Try to keep this one breathing.");
        }
        break;
    case ACT_BIZ_PLACE_LAWYER:
        DoPlaceLawyer(app, arg);
        break;
    case ACT_CREW_MENU:
        game->menuId = MENU_CREW;
        break;
    case ACT_UNIT_MENU:
        game->ctxUnit = arg;
        game->menuId = MENU_UNIT;
        break;
    case ACT_UNIT_ASSIGN:
        DoUnitAssign(app, arg);
        break;
    case ACT_UNIT_DISMISS:
        UiLogFmt(app, L"%s takes the news quietly and disappears into the city.", game->crew[arg].name);
        game->crew[arg].type = UNIT_NONE;
        game->menuId = MENU_CREW;
        break;
    case ACT_UNIT_MEDPACK:
        DoMedpack(app);
        break;
    case ACT_RECRUIT_MENU:
        game->menuId = MENU_RECRUIT;
        break;
    case ACT_RECRUIT:
        DoRecruit(app, arg);
        break;
    case ACT_FAMILY_MENU:
        game->menuId = MENU_FAMILY;
        break;
    case ACT_COURT:
        DoCourt(app);
        break;
    case ACT_COURT_PAY:
        DoCourtPay(app, arg);
        break;
    case ACT_COURT_WOO:
        DoCourtWoo(app, arg);
        break;
    case ACT_SITDOWN:
        DoSitdown(app, arg);
        break;
    case ACT_HIT_CAPO:
        DoHitCapo(app, arg);
        break;
    case ACT_DISTRICT_EVENT:
        DoDistrictEvent(app);
        break;
    case ACT_LEGEND:
        DoLegend(app);
        break;
    case ACT_ARM_CREW:
        DoArmCrew(app);
        break;
    case ACT_SCOUT_HIDEOUT:
        DoScoutHideout(app, arg);
        break;
    case ACT_RAID_HIDEOUT:
        DoRaidHideout(app, arg);
        break;
    case ACT_HEIR_TUTOR:
        DoHeirTutor(app, arg);
        break;
    case ACT_RESCUE_VINCENZO:
        DoRescueVincenzo(app);
        break;
    case ACT_SCOUT_CHURCH:
        DoScoutChurch(app);
        break;
    case ACT_WEDDING:
        DoWedding(app);
        break;
    case ACT_ROMANCE:
        DoRomance(app);
        break;
    case ACT_HEIR_MENU:
        game->ctxUnit = arg;
        game->menuId = MENU_HEIR;
        break;
    case ACT_HEIR_PLACE:
        DoHeirPlace(app, arg);
        break;
    case ACT_MISTRESS_MEET:
        DoMistressMeet(app);
        break;
    case ACT_MISTRESS_END:
        game->mistress = 0;
        game->mistressDays = 0;
        UiLog(app, L"You send a final gift and stop coming around. The affair ends the way they all do - quietly, and not quite cleanly.");
        break;
    case ACT_MISTRESS_GIFT:
        if (MISTRESS_GIFT_COST <= game->cash)
        {
            game->cash = game->cash - MISTRESS_GIFT_COST;
            game->wifeSuspicion = (MISTRESS_GIFT_RELIEF < game->wifeSuspicion) ? (game->wifeSuspicion - MISTRESS_GIFT_RELIEF) : 0;
            UiLogFmt(app, L"Pearls for %s, delivered with flowers and an alibi. Suspicion softens - jewelry has a way of doing that.", game->wife.name);
        }
        break;
    case ACT_CONFRONT:
        game->menuId = MENU_CONFRONT;
        UiLogFmt(app, L"%s does not raise her voice. That is the frightening part. 'Tell me about her,' she says.", game->wife.name);
        break;
    case ACT_CONFRONT_SWEAR:
    case ACT_CONFRONT_DENY:
    case ACT_CONFRONT_DISMISS:
        DoConfront(app, action);
        break;
    case ACT_INLAW_TRIBUTE:
        DoInLawTribute(app, 1);
        break;
    case ACT_INLAW_PROTECT:
        DoInLawTribute(app, 0);
        break;
    case ACT_RANSOM_PAY:
        DoRansomPay(app);
        break;
    case ACT_RESCUE:
        DoRescue(app);
        break;
    case ACT_CAR_BUY:
        DoCarBuy(app, arg);
        break;
    case ACT_CAR_STEAL:
        DoCarSteal(app);
        break;
    case ACT_BUY_AMMO:
        if (AMMO_BOX_COST <= game->cash)
        {
            game->cash = game->cash - AMMO_BOX_COST;
            game->ammo = game->ammo + AMMO_BOX_ROUNDS;
            UiLogFmt(app, L"A crate of shells changes hands. +%d ammo.", AMMO_BOX_ROUNDS);
        }
        break;
    case ACT_BIZ_BOMB:
        DoBizBomb(app);
        break;
    case ACT_SOUND_TOGGLE:
        game->soundOn = (0 == game->soundOn) ? 1 : 0;
        UiLogFmt(app, L"Sound is now %s.", (0 != game->soundOn) ? L"on" : L"off");
        break;
    case ACT_LAWYER_BRIBE:
        if (BRIBE_COST <= game->cash)
        {
            game->cash = game->cash - BRIBE_COST;
            game->copsHostile = 0;
            game->heat = (BRIBE_HEAT_RELIEF < game->heat) ? (game->heat - BRIBE_HEAT_RELIEF) : 0;
            UiLog(app, L"Envelopes change hands at the courthouse. The police suddenly remember other priorities.");
            game->menuId = MENU_FAMILY;
        }
        break;
    case ACT_DEALER_MENU:
        game->menuId = MENU_DEALER;
        UiLog(app, L"The weapons dealer wipes his counter. 'For a Mangano? Anything on the wall.'");
        break;
    case ACT_DEALER_WEAPON:
        if (1 == arg && WEAPON_COST_TIER1 <= game->cash)
        {
            game->cash = game->cash - WEAPON_COST_TIER1;
            game->weaponTier = 1;
            UiLog(app, L"Twin pistols, oiled and mean. You feel faster already.");
        }
        else if (2 == arg && WEAPON_COST_TIER2 <= game->cash)
        {
            game->cash = game->cash - WEAPON_COST_TIER2;
            game->weaponTier = 2;
            UiLog(app, L"A Tommy gun in a violin case. The classic never goes out of style.");
        }
        else if (3 == arg && WEAPON_COST_TIER3 <= game->cash)
        {
            game->cash = game->cash - WEAPON_COST_TIER3;
            game->weaponTier = 3;
            UiLog(app, L"The custom Tommy - drum magazine, compensator, walnut grip. A work of art.");
        }
        else
        {
            UiLog(app, L"The dealer shakes his head at your wallet.");
        }
        break;
    case ACT_DEALER_MEDPACK:
        if (MEDPACK_COST <= game->cash)
        {
            game->cash = game->cash - MEDPACK_COST;
            game->medpacks = game->medpacks + 1;
            UiLog(app, L"You buy a field medpack. Bandages, morphine, a prayer.");
        }
        break;
    case ACT_ANSWER_PHONE:
        DoAnswerPhone(app);
        break;
    case ACT_PHONE_ACCEPT:
        DoPhoneAccept(app);
        break;
    case ACT_PHONE_DECLINE:
        memset(&game->phone, 0, sizeof(PhoneOffer));
        UiLog(app, L"You set the receiver down without a word.");
        game->menuId = MENU_MAIN;
        break;
    case ACT_PAY_CHIEF:
        if (CHIEF_DEMAND <= game->cash)
        {
            game->cash = game->cash - CHIEF_DEMAND;
            game->chiefDeadline = 0;
            game->copsHostile = 0;
            UiLogFmt(app, L"The Chief pockets your $%d without counting it. 'Pleasure doing business, Mangano.' The law goes back to sleep.", CHIEF_DEMAND);
        }
        else
        {
            UiLogFmt(app, L"You need $%d and you do not have it. Find the money before the Chief loses patience.", CHIEF_DEMAND);
        }
        break;
    case ACT_NEWSMAN:
        DoNewsman(app);
        break;
    case ACT_MISSION:
        DoMission(app);
        break;
    case ACT_FAVOR:
        DoFavor(app);
        break;
    case ACT_CONTRACT:
        DoContract(app);
        break;
    case ACT_BOMB_STATION:
        DoBombStation(app);
        break;
    case ACT_APPROACH:
        if (3 == arg)
        {
            memset(&game->pending, 0, sizeof(PendingCombat));
            UiLog(app, L"You wave the crew off. Another day.");
            game->menuId = MENU_MAIN;
        }
        else
        {
            ResolveCombat(app, arg);
        }
        break;
    case ACT_CRATE_PICKUP:
        DoCratePickup(app);
        break;
    case ACT_CRATE_DELIVER:
        DoCrateDeliver(app);
        break;
    case ACT_END_DAY:
        EndDay(app);
        break;
    case ACT_SAVE_MENU:
        game->menuId = MENU_SAVE;
        break;
    case ACT_SAVE:
        SaveGame(app, arg, 0);
        game->menuId = MENU_MAIN;
        break;
    case ACT_LOAD:
        LoadGame(app, arg);
        break;
    case ACT_NEW_GAME:
        NewGame(app);
        break;
    default:
        break;
    }

Cleanup:

    return;
}
