#include "gangland.h"
#include <shlwapi.h>
#include <stdarg.h>

#define LABEL_CHARS 128
#define CHIEF_DEMAND 3000
#define NEWSMAN_FEE 20
#define MANAGER_HIRE_COST 200
#define MEDPACK_COST 120
#define BOMB_COST 500
#define BRIBE_COST 800
#define COURT_MIN_CASH 2000
#define SWEAR_GIFT_COST 1000
#define MISTRESS_MEET_COST 500
#define MISTRESS_GIFT_COST 500
#define CAR_COST_JALOPY 600
#define CAR_COST_SEDAN 1500
#define CAR_COST_ARMORED 3500
#define AMMO_BOX_COST 100
#define AMMO_BOX_ROUNDS 5
#define WEAPON_COST_TIER1 800
#define WEAPON_COST_TIER2 2000
#define WEAPON_COST_TIER3 4500
#define SITDOWN_BASE 500
#define SITDOWN_PER_ANGER 10
#define ARM_COST 600
#define TUTOR_COST 200
#define TUTOR_QUALITY_CAP 95
#define SLOT_DESC_CHARS 64

static const WCHAR* const EVENT_LABELS[NUM_DISTRICTS] =
{
    L"[OPPORTUNITY] Sponsor the Feast of San Gennaro ($300)",
    L"[OPPORTUNITY] Provide discreet protection at the society gala",
    L"[OPPORTUNITY] Hijack the unwatched freighter's cargo",
    L"[OPPORTUNITY] Back a side in the vendor war ($200)",
    L"[OPPORTUNITY] Buy out the moonshiner's stock ($350)"
};

static void AddChoiceFmt(App* app, int packedAction, const WCHAR* format, ...)
{
    va_list args = NULL;

    if (NULL == app || NULL == app->formatBuffer)
    {
        goto Cleanup;
    }
    va_start(args, format);
    wvnsprintfW(app->formatBuffer, LABEL_CHARS, format, args);
    va_end(args);
    AddChoice(app, app->formatBuffer, packedAction);

Cleanup:

    return;
}

static const WCHAR* BusinessStatusWord(int status)
{
    const WCHAR* word = L"independent";

    if (BIZ_EXTORTED == status)
    {
        word = L"paying protection";
    }
    if (BIZ_OWNED == status)
    {
        word = L"yours";
    }
    if (BIZ_DESTROYED == status)
    {
        word = L"a smoking ruin";
    }

    return word;
}

static int SquadHasType(GameState* game, int type)
{
    int found = 0;
    int index = 0;

    for (index = 0; MAX_CREW > index; index++)
    {
        if (type == game->crew[index].type && 0 != game->crew[index].alive && ASSIGN_SQUAD == game->crew[index].assignment)
        {
            found = 1;
        }
    }

    return found;
}

static int HasLawyerHeir(GameState* game)
{
    int found = 0;
    int index = 0;

    for (index = 0; MAX_HEIRS > index; index++)
    {
        if (0 != game->heirs[index].exists && HEIR_LAWYER == game->heirs[index].type)
        {
            found = 1;
        }
    }

    return found;
}

static int CanRecruitHere(GameState* game)
{
    int allowed = 0;
    int index = 0;
    Business* biz = NULL;

    if (0 != HasLawyerHeir(game))
    {
        allowed = 1;
    }
    for (index = 0; MAX_BUSINESSES > index; index++)
    {
        biz = &game->businesses[index];
        if (BIZ_RESTAURANT == biz->type && biz->district == game->location && (BIZ_EXTORTED == biz->status || BIZ_OWNED == biz->status))
        {
            allowed = 1;
        }
    }

    return allowed;
}

static void BuildMissionChoices(App* app)
{
    GameState* game = NULL;

    game = app->game;
    if (0 == game->missionStage && DISTRICT_DOCKS == game->location && 0 == game->carryingCrate)
    {
        AddChoice(app, L"[JOB] Pick up Vincenzo's ammunition crate", ACT_CRATE_PICKUP);
    }
    if (0 == game->missionStage && DISTRICT_LITTLE_ITALY == game->location && 0 != game->carryingCrate)
    {
        AddChoice(app, L"[JOB] Deliver the crate to Vincenzo", ACT_CRATE_DELIVER);
    }
    if (2 == game->missionStage && DISTRICT_DOCKS == game->location)
    {
        AddChoice(app, L"[JOB] Hit the three hoodlums squeezing Vincenzo's friend", ACT_MISSION);
    }
    if (3 == game->missionStage && 0 != game->romanoKnown && DISTRICT_DOCKS == game->location)
    {
        AddChoice(app, L"[VENDETTA] Raid Romano's warehouse", ACT_MISSION);
    }
    if (5 == game->missionStage && 0 != game->angeloKnown && DISTRICT_DOWNTOWN == game->location)
    {
        AddChoice(app, L"[VENDETTA] Storm Club Paradiso and take Angelo", ACT_MISSION);
    }
    if (6 == game->missionStage && 0 != game->sonnyKnown && DISTRICT_SUBURBS == game->location)
    {
        AddChoice(app, L"[VENDETTA] Assault Sonny's fortified villa", ACT_MISSION);
    }
}

static void BuildMainMenu(App* app)
{
    GameState* game = NULL;
    int here = 0;
    int index = 0;
    int hasBiz = 0;

    game = app->game;
    here = game->location;
    AddChoiceFmt(app, ACT_LOOK, L"Look around %s", DistrictName(here));
    AddChoice(app, L"Review your objectives", ACT_SHOW_JOBS);
    AddChoice(app, L"Review your legend", ACT_LEGEND);
    BuildMissionChoices(app);
    if (DISTRICT_DOCKS == here && 0 != game->vincenzoFound && 0 == game->vincenzoRescued)
    {
        AddChoice(app, L"[FAMILY] Storm the basement off Pier 13 - bring Vincenzo home", ACT_RESCUE_VINCENZO);
    }
    if (KIDNAP_NOBODY != game->kidnapVictim)
    {
        AddChoiceFmt(app, ACT_RANSOM_PAY, L"[FAMILY] Pay the ransom for %s ($%d)", KidnapVictimName(game), game->kidnapRansom);
        AddChoiceFmt(app, ACT_RESCUE, L"[FAMILY] Send the scout after the hideout, then take %s back by force", KidnapVictimName(game));
    }
    if (0 != game->wifeConfront && DISTRICT_LITTLE_ITALY == here)
    {
        AddChoiceFmt(app, ACT_CONFRONT, L"[FAMILY] Face %s about the affair", game->wife.name);
    }
    if (game->favorDistrict == here)
    {
        AddChoice(app, L"[FAVOR] Deal with the men troubling the citizen", ACT_FAVOR);
    }
    if (0 != game->districtEvent.active && game->districtEvent.district == here)
    {
        AddChoice(app, EVENT_LABELS[here], ACT_DISTRICT_EVENT);
    }
    for (index = 0; NUM_RIVALS > index; index++)
    {
        if (game->rivals[index].district != here)
        {
            continue;
        }
        if (0 == game->rivals[index].hideoutDownDays)
        {
            if (0 == game->rivals[index].hideoutScouted)
            {
                AddChoiceFmt(app, PACK_ACTION(ACT_SCOUT_HIDEOUT, index), L"Send the scout to case the local hideout");
                AddChoice(app, L"[RAID] Hit the hideout (strength unknown)", PACK_ACTION(ACT_RAID_HIDEOUT, index));
            }
            else
            {
                AddChoiceFmt(app, PACK_ACTION(ACT_RAID_HIDEOUT, index), L"[RAID] Hit the hideout (%d hoodlums, %d soldiers)", game->rivals[index].hideoutHoodlums, game->rivals[index].hideoutSoldiers);
            }
        }
        if (0 == game->rivals[index].alive)
        {
            continue;
        }
        if (RIVAL_SITDOWN_MIN <= game->rivals[index].anger)
        {
            AddChoiceFmt(app, PACK_ACTION(ACT_SITDOWN, index), L"Arrange a sit-down with %s ($%d smooths things over)", game->rivals[index].name, SITDOWN_BASE + (game->rivals[index].anger * SITDOWN_PER_ANGER));
        }
        if (RIVAL_HIT_MIN <= game->rivals[index].anger)
        {
            AddChoiceFmt(app, PACK_ACTION(ACT_HIT_CAPO, index), L"[WAR] Hit %s and take his back room apart", game->rivals[index].name);
        }
    }
    if (0 <= game->contractBusiness && game->businesses[game->contractBusiness].district == here)
    {
        AddChoiceFmt(app, ACT_CONTRACT, L"[CONTRACT] Hit the manager of %s", game->businesses[game->contractBusiness].name);
    }
    for (index = 0; MAX_BUSINESSES > index; index++)
    {
        if (game->businesses[index].district == here && BIZ_DESTROYED != game->businesses[index].status)
        {
            hasBiz = 1;
        }
    }
    if (0 != hasBiz)
    {
        AddChoice(app, L"Rackets: lean on the local businesses", ACT_RACKETS_MENU);
    }
    AddChoice(app, L"Crew and recruiting", ACT_CREW_MENU);
    AddChoice(app, L"Family matters", ACT_FAMILY_MENU);
    if (DISTRICT_DOCKS == here)
    {
        AddChoice(app, L"Visit the weapons dealer", ACT_DEALER_MENU);
    }
    if (DISTRICT_DOWNTOWN == here)
    {
        AddChoiceFmt(app, ACT_NEWSMAN, L"Buy the paper from the newspaper man ($%d)", NEWSMAN_FEE);
    }
    if (DISTRICT_LITTLE_ITALY == here && 0 != game->phone.active)
    {
        AddChoice(app, L"Answer the ringing telephone", ACT_ANSWER_PHONE);
    }
    if (0 < game->chiefDeadline || 0 != game->copsHostile)
    {
        AddChoiceFmt(app, ACT_PAY_CHIEF, L"Pay off the Chief of Police ($%d)", CHIEF_DEMAND);
    }
    if (0 < game->policePresence[here] && 0 != SquadHasType(game, UNIT_BOMBER))
    {
        AddChoiceFmt(app, ACT_BOMB_STATION, L"Have your bomber level the police station ($%d)", BOMB_COST);
    }
    if (CAR_NONE == game->carTier)
    {
        AddChoice(app, L"Steal a parked car", ACT_CAR_STEAL);
    }
    AddChoice(app, L"Travel to another district", ACT_TRAVEL_MENU);
    AddChoice(app, L"Save or load the game", ACT_SAVE_MENU);
    AddChoice(app, L"End the day (return to the safe house)", ACT_END_DAY);
}

static void BuildTravelMenu(App* app)
{
    int index = 0;

    for (index = 0; NUM_DISTRICTS > index; index++)
    {
        if (index != app->game->location)
        {
            AddChoiceFmt(app, PACK_ACTION(ACT_TRAVEL, index), L"Go to %s%s", DistrictName(index), (DISTRICT_SUBURBS == index) ? L" (no police out there)" : L"");
        }
    }
    AddChoice(app, L"Stay put", ACT_BACK);
}

static void BuildRacketsMenu(App* app)
{
    GameState* game = NULL;
    int index = 0;
    Business* biz = NULL;

    game = app->game;
    for (index = 0; MAX_BUSINESSES > index; index++)
    {
        biz = &game->businesses[index];
        if (biz->district == game->location && BIZ_DESTROYED != biz->status)
        {
            AddChoiceFmt(app, PACK_ACTION(ACT_BIZ_MENU, index), L"%s ($%d/day, %s)", biz->name, biz->baseIncome, BusinessStatusWord(biz->status));
        }
    }
    AddChoice(app, L"Never mind", ACT_BACK);
}

static void BuildBusinessMenu(App* app)
{
    GameState* game = NULL;
    Business* biz = NULL;
    int index = 0;
    int heirIndex = 0;

    game = app->game;
    biz = &game->businesses[game->ctxBusiness];
    if (BIZ_INDEPENDENT == biz->status)
    {
        AddChoice(app, L"Extort: rough up the owner for a weekly cut", ACT_BIZ_EXTORT);
        AddChoice(app, L"Take over: seize the whole business", ACT_BIZ_TAKEOVER);
    }
    if (BIZ_EXTORTED == biz->status)
    {
        AddChoice(app, L"Seize it outright and run it yourself", ACT_BIZ_TAKEOVER);
    }
    if (BIZ_OWNED != biz->status && 0 != SquadHasType(game, UNIT_BOMBER))
    {
        AddChoice(app, L"Have your bomber level the place (terror has its uses)", ACT_BIZ_BOMB);
    }
    if (BIZ_OWNED == biz->status || BIZ_EXTORTED == biz->status)
    {
        AddChoice(app, L"Post a guard from your squad here", ACT_BIZ_GUARD_MENU);
        for (index = 0; MAX_CREW > index; index++)
        {
            if (0 == game->crew[index].alive || ASSIGN_BUSINESS != game->crew[index].assignment || game->ctxBusiness != game->crew[index].assignedBusiness)
            {
                continue;
            }
            AddChoiceFmt(app, PACK_ACTION(ACT_UNIT_ASSIGN, index), L"Recall %s from guard duty", game->crew[index].name);
        }
    }
    if (BIZ_OWNED == biz->status)
    {
        if (0 == biz->managerAlive)
        {
            AddChoiceFmt(app, ACT_BIZ_HIRE_MANAGER, L"Hire a new manager ($%d)", MANAGER_HIRE_COST);
        }
        for (heirIndex = 0; MAX_HEIRS > heirIndex; heirIndex++)
        {
            if (0 == game->heirs[heirIndex].exists || HEIR_LAWYER != game->heirs[heirIndex].type || PLACE_HOME != game->heirs[heirIndex].placement || 0 != biz->hasLawyer)
            {
                continue;
            }
            AddChoiceFmt(app, PACK_ACTION(ACT_BIZ_PLACE_LAWYER, heirIndex), L"Install %s the lawyer here (+50%% revenue)", game->heirs[heirIndex].name);
        }
    }
    AddChoice(app, L"Never mind", ACT_BACK);
}

static void BuildGuardPickMenu(App* app)
{
    GameState* game = NULL;
    int index = 0;
    Unit* unit = NULL;

    game = app->game;
    for (index = 0; MAX_CREW > index; index++)
    {
        unit = &game->crew[index];
        if (UNIT_NONE != unit->type && 0 != unit->alive && ASSIGN_SQUAD == unit->assignment)
        {
            AddChoiceFmt(app, PACK_ACTION(ACT_BIZ_GUARD, index), L"Post %s the %s", unit->name, GetUnitSpec(unit->type)->title);
        }
    }
    AddChoice(app, L"Never mind", ACT_BACK);
}

static const WCHAR* AssignmentWord(const Unit* unit)
{
    const WCHAR* word = L"in your squad";

    if (ASSIGN_SAFEHOUSE == unit->assignment)
    {
        word = L"guarding the safe house";
    }
    if (ASSIGN_BUSINESS == unit->assignment)
    {
        word = L"on guard duty";
    }
    if (ASSIGN_CORNER == unit->assignment)
    {
        word = L"working a corner";
    }

    return word;
}

static void BuildCrewMenu(App* app)
{
    GameState* game = NULL;
    int index = 0;
    Unit* unit = NULL;

    game = app->game;
    for (index = 0; MAX_CREW > index; index++)
    {
        unit = &game->crew[index];
        if (UNIT_NONE != unit->type && 0 != unit->alive)
        {
            AddChoiceFmt(app, PACK_ACTION(ACT_UNIT_MENU, index), L"%s the %s%s - Lv%d, %d/%d hp, %s", unit->name, GetUnitSpec(unit->type)->title, (0 != unit->armed) ? L" [Tommy]" : L"", unit->level, unit->health, unit->maxHealth, AssignmentWord(unit));
        }
    }
    if (0 != CanRecruitHere(game))
    {
        AddChoice(app, L"Recruit new muscle", ACT_RECRUIT_MENU);
    }
    if (0 == CanRecruitHere(game))
    {
        AddChoice(app, L"(You recruit at a restaurant you control, or through a lawyer)", ACT_NONE);
    }
    if (0 < game->medpacks)
    {
        AddChoice(app, L"Use a medpack on the most wounded", ACT_UNIT_MEDPACK);
    }
    AddChoice(app, L"Back", ACT_BACK);
}

static void BuildUnitMenu(App* app)
{
    GameState* game = NULL;
    Unit* unit = NULL;

    game = app->game;
    unit = &game->crew[game->ctxUnit];
    if (ASSIGN_SQUAD != unit->assignment)
    {
        AddChoiceFmt(app, PACK_ACTION(ACT_UNIT_ASSIGN, game->ctxUnit), L"Bring %s along in your squad", unit->name);
    }
    if (ASSIGN_SAFEHOUSE != unit->assignment)
    {
        AddChoiceFmt(app, PACK_ACTION(ACT_UNIT_ASSIGN, game->ctxUnit) + PACK_ACTION(0, MAX_CREW), L"Post %s at the safe house windows", unit->name);
    }
    if (UNIT_STREET_GIRL == unit->type && ASSIGN_CORNER != unit->assignment)
    {
        AddChoiceFmt(app, PACK_ACTION(ACT_UNIT_ASSIGN, game->ctxUnit) + PACK_ACTION(0, 2 * MAX_CREW), L"Send %s to work a corner (steady cash, steady heat)", unit->name);
    }
    AddChoiceFmt(app, PACK_ACTION(ACT_UNIT_DISMISS, game->ctxUnit), L"Cut %s loose", unit->name);
    AddChoice(app, L"Back", ACT_BACK);
}

static void BuildRecruitMenu(App* app)
{
    int type = 0;
    const UnitSpec* spec = NULL;

    for (type = UNIT_BOUNCER; UNIT_BOMBER >= type; type++)
    {
        spec = GetUnitSpec(type);
        AddChoiceFmt(app, PACK_ACTION(ACT_RECRUIT, type), L"Hire a %s ($%d - %d hp, %d acc)", spec->title, spec->cost, spec->health, spec->accuracy);
    }
    AddChoice(app, L"Back", ACT_BACK);
}

static const WCHAR* HeirTypeWord(int type)
{
    const WCHAR* word = L"enforcer";

    if (HEIR_LAWYER == type)
    {
        word = L"lawyer";
    }
    if (HEIR_SEDUCTRESS == type)
    {
        word = L"seductress";
    }

    return word;
}

static const WCHAR* PlacementWord(int placement)
{
    const WCHAR* word = L"at home";

    if (PLACE_FIELD == placement)
    {
        word = L"in the field";
    }
    if (PLACE_BUSINESS == placement)
    {
        word = L"running a business";
    }
    if (PLACE_DESK == placement)
    {
        word = L"on the phone desk";
    }
    if (PLACE_COURTHOUSE == placement)
    {
        word = L"at the courthouse";
    }

    return word;
}

static void BuildFamilyMenu(App* app)
{
    GameState* game = NULL;
    int index = 0;
    int anyCandidate = 0;

    game = app->game;
    for (index = 0; MAX_CANDIDATES > index; index++)
    {
        if (0 == game->candidates[index].active)
        {
            continue;
        }
        anyCandidate = 1;
        AddChoiceFmt(app, PACK_ACTION(ACT_COURT_PAY, index), L"Propose to %s (asking $%d - athletic %d, clever %d, charming %d)", game->candidates[index].name, game->candidates[index].dowry - game->candidates[index].wooed, game->candidates[index].athletic, game->candidates[index].clever, game->candidates[index].charm);
        if (DISTRICT_DOWNTOWN == game->location && game->courtedDay != game->day)
        {
            AddChoiceFmt(app, PACK_ACTION(ACT_COURT_WOO, index), L"Court %s - wine and dancing ($150 lowers her asking price)", game->candidates[index].name);
        }
    }
    if (WIFE_NONE == game->wife.status && 0 == anyCandidate)
    {
        AddChoiceFmt(app, ACT_COURT, L"Visit the Downtown social club to meet prospective wives (need $%d and a name)", COURT_MIN_CASH);
    }
    if (WIFE_ENGAGED == game->wife.status && 0 == game->churchFound)
    {
        AddChoice(app, L"Send a scout to find a church out in the Suburbs", ACT_SCOUT_CHURCH);
    }
    if (WIFE_ENGAGED == game->wife.status && 0 != game->churchFound)
    {
        AddChoice(app, L"Hold the wedding - escort the bride to the church with your squad", ACT_WEDDING);
    }
    if (WIFE_MARRIED == game->wife.status && DISTRICT_LITTLE_ITALY == game->location && 0 == game->wife.gestationDays)
    {
        AddChoiceFmt(app, ACT_ROMANCE, L"Spend the evening with %s", game->wife.name);
    }
    for (index = 0; MAX_HEIRS > index; index++)
    {
        if (0 != game->heirs[index].exists)
        {
            AddChoiceFmt(app, PACK_ACTION(ACT_HEIR_MENU, index), L"%s the %s (quality %d, %s)", game->heirs[index].name, HeirTypeWord(game->heirs[index].type), game->heirs[index].quality, PlacementWord(game->heirs[index].placement));
        }
    }
    if (WIFE_MARRIED == game->wife.status && 0 == game->mistress && DISTRICT_DOWNTOWN == game->location)
    {
        AddChoiceFmt(app, ACT_MISTRESS_MEET, L"Take a mistress at the social club ($%d - she hears everything, and so might your wife)", MISTRESS_MEET_COST);
    }
    if (0 != game->mistress)
    {
        AddChoice(app, L"End the affair with a parting gift", ACT_MISTRESS_END);
    }
    if (WIFE_MARRIED == game->wife.status && 0 < game->wifeSuspicion)
    {
        AddChoiceFmt(app, ACT_MISTRESS_GIFT, L"Buy %s jewelry to soothe her suspicions ($%d)", game->wife.name, MISTRESS_GIFT_COST);
    }
    if (WIFE_MARRIED == game->wife.status && 0 <= game->inLawBusiness && 0 == game->inLawTribute)
    {
        AddChoiceFmt(app, ACT_INLAW_TRIBUTE, L"Squeeze the in-laws at %s for tribute (your wife will hear of it)", game->businesses[game->inLawBusiness].name);
    }
    if (WIFE_MARRIED == game->wife.status && 0 <= game->inLawBusiness && 0 != game->inLawTribute)
    {
        AddChoiceFmt(app, ACT_INLAW_PROTECT, L"Stop squeezing the in-laws at %s (protect them for free)", game->businesses[game->inLawBusiness].name);
    }
    AddChoice(app, L"Back", ACT_BACK);
}

static void BuildConfrontMenu(App* app)
{
    GameState* game = NULL;

    game = app->game;
    AddChoiceFmt(app, ACT_CONFRONT_SWEAR, L"Swear it is over - end the affair, $%d of penance jewelry", SWEAR_GIFT_COST);
    AddChoice(app, L"Deny everything and lie to her face", ACT_CONFRONT_DENY);
    AddChoiceFmt(app, ACT_CONFRONT_DISMISS, L"Tell %s to mind her place", game->wife.name);
}

static void BuildHeirMenu(App* app)
{
    GameState* game = NULL;
    Heir* heir = NULL;

    game = app->game;
    heir = &game->heirs[game->ctxUnit];
    if (HEIR_ENFORCER == heir->type && PLACE_FIELD != heir->placement)
    {
        AddChoiceFmt(app, PACK_ACTION(ACT_HEIR_PLACE, PLACE_FIELD), L"Send %s into the field with twin Tommy guns", heir->name);
    }
    if (HEIR_LAWYER == heir->type && PLACE_COURTHOUSE != heir->placement)
    {
        AddChoiceFmt(app, PACK_ACTION(ACT_HEIR_PLACE, PLACE_COURTHOUSE), L"Send %s to the courthouse (cools heat daily)", heir->name);
    }
    if (HEIR_LAWYER == heir->type && 0 != game->copsHostile)
    {
        AddChoiceFmt(app, ACT_LAWYER_BRIBE, L"Have %s buy off the judges to call off the police ($%d)", heir->name, BRIBE_COST);
    }
    if (HEIR_SEDUCTRESS == heir->type && PLACE_DESK != heir->placement)
    {
        AddChoiceFmt(app, PACK_ACTION(ACT_HEIR_PLACE, PLACE_DESK), L"Put %s on the phone desk (never miss a call)", heir->name);
    }
    if (HEIR_SEDUCTRESS == heir->type && PLACE_FIELD != heir->placement)
    {
        AddChoiceFmt(app, PACK_ACTION(ACT_HEIR_PLACE, PLACE_FIELD), L"Send %s to charm cops and enemies to your side", heir->name);
    }
    if (PLACE_HOME == heir->placement && TUTOR_QUALITY_CAP > heir->quality && game->tutoredDay != game->day)
    {
        AddChoiceFmt(app, PACK_ACTION(ACT_HEIR_TUTOR, game->ctxUnit), L"Invest in %s's schooling ($%d - raises quality)", heir->name, TUTOR_COST);
    }
    if (PLACE_HOME != heir->placement)
    {
        AddChoiceFmt(app, PACK_ACTION(ACT_HEIR_PLACE, PLACE_HOME), L"Bring %s home", heir->name);
    }
    AddChoice(app, L"Back", ACT_BACK);
}

static void BuildDealerMenu(App* app)
{
    GameState* game = NULL;

    game = app->game;
    if (0 == game->weaponTier)
    {
        AddChoiceFmt(app, PACK_ACTION(ACT_DEALER_WEAPON, 1), L"Buy twin pistols ($%d)", WEAPON_COST_TIER1);
    }
    if (1 == game->weaponTier)
    {
        AddChoiceFmt(app, PACK_ACTION(ACT_DEALER_WEAPON, 2), L"Buy a Tommy gun ($%d)", WEAPON_COST_TIER2);
    }
    if (2 == game->weaponTier)
    {
        AddChoiceFmt(app, PACK_ACTION(ACT_DEALER_WEAPON, 3), L"Buy the custom Tommy gun ($%d)", WEAPON_COST_TIER3);
    }
    AddChoiceFmt(app, ACT_DEALER_MEDPACK, L"Buy a medpack ($%d)", MEDPACK_COST);
    AddChoiceFmt(app, ACT_BUY_AMMO, L"Buy a crate of ammunition ($%d for %d)", AMMO_BOX_COST, AMMO_BOX_ROUNDS);
    AddChoiceFmt(app, ACT_ARM_CREW, L"Arm a crew member with a Tommy gun ($%d, +1 attack)", ARM_COST);
    if (CAR_JALOPY > game->carTier)
    {
        AddChoiceFmt(app, PACK_ACTION(ACT_CAR_BUY, CAR_JALOPY), L"Buy a rusty jalopy ($%d)", CAR_COST_JALOPY);
    }
    if (CAR_SEDAN > game->carTier)
    {
        AddChoiceFmt(app, PACK_ACTION(ACT_CAR_BUY, CAR_SEDAN), L"Buy a black sedan ($%d - built for drive-bys)", CAR_COST_SEDAN);
    }
    if (CAR_ARMORED > game->carTier)
    {
        AddChoiceFmt(app, PACK_ACTION(ACT_CAR_BUY, CAR_ARMORED), L"Buy an armored sedan ($%d)", CAR_COST_ARMORED);
    }
    AddChoice(app, L"Leave the shop", ACT_BACK);
}

static void BuildPhoneMenu(App* app)
{
    GameState* game = NULL;

    game = app->game;
    if (PHONE_UNIT == game->phone.kind)
    {
        AddChoiceFmt(app, ACT_PHONE_ACCEPT, L"Buy the %s for $%d", GetUnitSpec(game->phone.unitType)->title, game->phone.price);
    }
    if (PHONE_HIT == game->phone.kind)
    {
        AddChoiceFmt(app, ACT_PHONE_ACCEPT, L"Take the contract: $%d for the manager of %s", game->phone.reward, game->businesses[game->phone.targetBusiness].name);
    }
    AddChoice(app, L"Hang up", ACT_PHONE_DECLINE);
}

static void BuildApproachMenu(App* app)
{
    AddChoiceFmt(app, PACK_ACTION(ACT_APPROACH, 0), L"Go in guns blazing (%s)", app->game->pending.label);
    AddChoice(app, L"Move carefully and use cover", PACK_ACTION(ACT_APPROACH, 1));
    if (0 != SquadHasType(app->game, UNIT_SCOUT))
    {
        AddChoice(app, L"Send the scout ahead to slip in quietly", PACK_ACTION(ACT_APPROACH, 2));
    }
    if (CAR_SEDAN <= app->game->carTier)
    {
        AddChoice(app, L"Roll past in the sedan first - drive-by", PACK_ACTION(ACT_APPROACH, 4));
    }
    AddChoice(app, L"Call the whole thing off", PACK_ACTION(ACT_APPROACH, 3));
}

static void DescribeSlot(int slot, WCHAR* buffer, int bufferChars)
{
    GameState* peek = NULL;

    peek = (GameState*)AllocZeroed(sizeof(GameState));
    CopyText(buffer, L"empty", bufferChars);
    if (NULL == peek)
    {
        goto Cleanup;
    }
    if (0 != PeekSave(slot, peek))
    {
        wnsprintfW(buffer, bufferChars, L"Day %d - %s, $%d", peek->day, (RANK_ERRAND_BOY == peek->rank) ? L"Errand Boy" : ((RANK_CAPO == peek->rank) ? L"Capo Soto" : L"Don"), peek->cash);
    }

Cleanup:
    FreeMemory(peek);

    return;
}

static void BuildSaveMenu(App* app)
{
    int slot = 0;
    WCHAR* description = NULL;

    description = (WCHAR*)AllocZeroed(SLOT_DESC_CHARS * sizeof(WCHAR));
    if (NULL == description)
    {
        goto Cleanup;
    }
    for (slot = 0; SAVE_SLOTS > slot; slot++)
    {
        DescribeSlot(slot, description, SLOT_DESC_CHARS);
        AddChoiceFmt(app, PACK_ACTION(ACT_SAVE, slot), L"Save to slot %d (%s)", slot + 1, description);
    }
    for (slot = 0; SAVE_SLOTS > slot; slot++)
    {
        DescribeSlot(slot, description, SLOT_DESC_CHARS);
        AddChoiceFmt(app, PACK_ACTION(ACT_LOAD, slot), L"Load from slot %d (%s)", slot + 1, description);
    }
    DescribeSlot(AUTOSAVE_SLOT, description, SLOT_DESC_CHARS);
    AddChoiceFmt(app, PACK_ACTION(ACT_LOAD, AUTOSAVE_SLOT), L"Load the dawn autosave (%s)", description);
    AddChoiceFmt(app, ACT_SOUND_TOGGLE, L"Sound: %s", (0 != app->game->soundOn) ? L"ON" : L"OFF");
    AddChoice(app, L"Start a fresh life in Paradise City", ACT_NEW_GAME);
    AddChoice(app, L"Back", ACT_BACK);

Cleanup:
    FreeMemory(description);

    return;
}

static void BuildDeadMenu(App* app)
{
    int slot = 0;
    WCHAR* description = NULL;

    description = (WCHAR*)AllocZeroed(SLOT_DESC_CHARS * sizeof(WCHAR));
    if (NULL == description)
    {
        goto Cleanup;
    }
    for (slot = 0; SAVE_SLOTS > slot; slot++)
    {
        DescribeSlot(slot, description, SLOT_DESC_CHARS);
        AddChoiceFmt(app, PACK_ACTION(ACT_LOAD, slot), L"Load from slot %d (%s)", slot + 1, description);
    }
    DescribeSlot(AUTOSAVE_SLOT, description, SLOT_DESC_CHARS);
    AddChoiceFmt(app, PACK_ACTION(ACT_LOAD, AUTOSAVE_SLOT), L"Load the dawn autosave (%s)", description);
    AddChoice(app, L"Start a new life", ACT_NEW_GAME);

Cleanup:
    FreeMemory(description);

    return;
}

void BuildChoices(App* app)
{
    switch (app->game->menuId)
    {
    case MENU_MAIN:
        BuildMainMenu(app);
        break;
    case MENU_TRAVEL:
        BuildTravelMenu(app);
        break;
    case MENU_RACKETS:
        BuildRacketsMenu(app);
        break;
    case MENU_BIZ:
        BuildBusinessMenu(app);
        break;
    case MENU_BIZ_GUARD_PICK:
        BuildGuardPickMenu(app);
        break;
    case MENU_CREW:
        BuildCrewMenu(app);
        break;
    case MENU_UNIT:
        BuildUnitMenu(app);
        break;
    case MENU_RECRUIT:
        BuildRecruitMenu(app);
        break;
    case MENU_FAMILY:
        BuildFamilyMenu(app);
        break;
    case MENU_HEIR:
        BuildHeirMenu(app);
        break;
    case MENU_DEALER:
        BuildDealerMenu(app);
        break;
    case MENU_PHONE:
        BuildPhoneMenu(app);
        break;
    case MENU_APPROACH:
        BuildApproachMenu(app);
        break;
    case MENU_SAVE:
        BuildSaveMenu(app);
        break;
    case MENU_DEAD:
        BuildDeadMenu(app);
        break;
    case MENU_CONFRONT:
        BuildConfrontMenu(app);
        break;
    default:
        AddChoice(app, L"Back", ACT_BACK);
        break;
    }
}
