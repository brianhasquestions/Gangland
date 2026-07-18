#ifndef GANGLAND_H
#define GANGLAND_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define NUM_DISTRICTS 5
#define MAX_CREW 16
#define MAX_BUSINESSES 14
#define MAX_HEIRS 3
#define MAX_CANDIDATES 3
#define MAX_CHOICES 24
#define NAME_CHARS 32
#define ACTION_ARG_BASE 1000
#define PACK_ACTION(code, arg) ((code) + ((arg) * ACTION_ARG_BASE))
#define SAVE_SLOTS 3
#define FORMAT_BUFFER_CHARS 1024

#define DISTRICT_LITTLE_ITALY 0
#define DISTRICT_DOWNTOWN 1
#define DISTRICT_DOCKS 2
#define DISTRICT_MARKET 3
#define DISTRICT_SUBURBS 4

#define UNIT_NONE 0
#define UNIT_BOUNCER 1
#define UNIT_GUNMAN 2
#define UNIT_HENCHMAN 3
#define UNIT_STREET_GIRL 4
#define UNIT_SCOUT 5
#define UNIT_SNIPER 6
#define UNIT_BOMBER 7
#define UNIT_ENFORCER 8
#define NUM_UNIT_TYPES 9

#define ASSIGN_SQUAD 0
#define ASSIGN_SAFEHOUSE 1
#define ASSIGN_BUSINESS 2
#define ASSIGN_CORNER 3

#define AUTOSAVE_SLOT 3

#define BIZ_AMMO 0
#define BIZ_RESTAURANT 1
#define BIZ_DISTILLERY 2
#define BIZ_CLOTHES 3
#define BIZ_JEWELRY 4
#define BIZ_PAWN 5
#define BIZ_PHOTO 6

#define BIZ_INDEPENDENT 0
#define BIZ_EXTORTED 1
#define BIZ_OWNED 2
#define BIZ_DESTROYED 3

#define WIFE_NONE 0
#define WIFE_ENGAGED 1
#define WIFE_MARRIED 2
#define WIFE_DEAD 3
#define WIFE_LEFT 4

#define CAR_NONE 0
#define CAR_JALOPY 1
#define CAR_SEDAN 2
#define CAR_ARMORED 3

#define KIDNAP_NOBODY -1
#define KIDNAP_WIFE 3

#define SOUND_GUNFIGHT 0
#define SOUND_CASH 1
#define SOUND_BELLS 2
#define SOUND_DEATH 3
#define SOUND_FANFARE 4
#define SOUND_COUNT 5

#define NUM_RIVALS 3
#define RIVAL_ANGER_TENSE 30
#define RIVAL_ANGER_WAR 70
#define RIVAL_SITDOWN_MIN 20
#define RIVAL_HIT_MIN 30

#define PING_NONE 0
#define PING_FIGHT 1
#define PING_MONEY 2
#define PING_ALERT 3
#define PING_EVENT 4
#define PING_FAMILY 5
#define PING_LIFETIME_DAYS 3

#define HEIR_ENFORCER 0
#define HEIR_LAWYER 1
#define HEIR_SEDUCTRESS 2

#define PLACE_HOME 0
#define PLACE_FIELD 1
#define PLACE_BUSINESS 2
#define PLACE_DESK 3
#define PLACE_COURTHOUSE 4

#define FIGHT_NONE 0
#define FIGHT_MISSION 1
#define FIGHT_EXTORT 2
#define FIGHT_TAKEOVER 3
#define FIGHT_FAVOR 4
#define FIGHT_CONTRACT 5
#define FIGHT_CRATE_AMBUSH 6
#define FIGHT_RESCUE 7
#define FIGHT_CAPO 8
#define FIGHT_HIDEOUT 9
#define FIGHT_VINCENZO 10

#define RAID_COPS 0
#define RAID_RIVALS 1
#define RAID_BOUNTY 2

#define PHONE_NONE 0
#define PHONE_UNIT 1
#define PHONE_HIT 2
#define PHONE_TIP 3

#define RANK_ERRAND_BOY 0
#define RANK_CAPO 1
#define RANK_DON 2

#define MENU_MAIN 0
#define MENU_TRAVEL 1
#define MENU_RACKETS 2
#define MENU_BIZ 3
#define MENU_BIZ_GUARD_PICK 4
#define MENU_CREW 5
#define MENU_UNIT 6
#define MENU_RECRUIT 7
#define MENU_FAMILY 8
#define MENU_COURT 9
#define MENU_HEIR 10
#define MENU_DEALER 11
#define MENU_PHONE 12
#define MENU_APPROACH 13
#define MENU_SAVE 14
#define MENU_DEAD 15
#define MENU_CONFRONT 16

#define ACT_NONE 0
#define ACT_BACK 1
#define ACT_LOOK 2
#define ACT_TRAVEL_MENU 3
#define ACT_TRAVEL 4
#define ACT_SHOW_JOBS 5
#define ACT_RACKETS_MENU 6
#define ACT_BIZ_MENU 7
#define ACT_BIZ_EXTORT 8
#define ACT_BIZ_TAKEOVER 9
#define ACT_BIZ_GUARD_MENU 10
#define ACT_BIZ_GUARD 11
#define ACT_BIZ_HIRE_MANAGER 12
#define ACT_BIZ_PLACE_LAWYER 13
#define ACT_CREW_MENU 14
#define ACT_UNIT_MENU 15
#define ACT_UNIT_ASSIGN 16
#define ACT_UNIT_DISMISS 17
#define ACT_UNIT_MEDPACK 18
#define ACT_RECRUIT_MENU 19
#define ACT_RECRUIT 20
#define ACT_FAMILY_MENU 21
#define ACT_COURT 22
#define ACT_COURT_PAY 23
#define ACT_SCOUT_CHURCH 24
#define ACT_WEDDING 25
#define ACT_ROMANCE 26
#define ACT_HEIR_MENU 27
#define ACT_HEIR_PLACE 28
#define ACT_DEALER_MENU 29
#define ACT_DEALER_WEAPON 30
#define ACT_DEALER_MEDPACK 31
#define ACT_SAFE_HEAL 32
#define ACT_ANSWER_PHONE 33
#define ACT_PHONE_ACCEPT 34
#define ACT_PHONE_DECLINE 35
#define ACT_PAY_CHIEF 36
#define ACT_NEWSMAN 37
#define ACT_MISSION 38
#define ACT_FAVOR 39
#define ACT_CONTRACT 40
#define ACT_BOMB_STATION 41
#define ACT_APPROACH 42
#define ACT_END_DAY 43
#define ACT_SAVE_MENU 44
#define ACT_SAVE 45
#define ACT_LOAD 46
#define ACT_NEW_GAME 47
#define ACT_CRATE_PICKUP 48
#define ACT_CRATE_DELIVER 49
#define ACT_LAWYER_BRIBE 50
#define ACT_MISTRESS_MEET 51
#define ACT_MISTRESS_END 52
#define ACT_MISTRESS_GIFT 53
#define ACT_CONFRONT 54
#define ACT_CONFRONT_SWEAR 55
#define ACT_CONFRONT_DENY 56
#define ACT_CONFRONT_DISMISS 57
#define ACT_INLAW_TRIBUTE 58
#define ACT_INLAW_PROTECT 59
#define ACT_RANSOM_PAY 60
#define ACT_RESCUE 61
#define ACT_CAR_BUY 62
#define ACT_CAR_STEAL 63
#define ACT_BUY_AMMO 64
#define ACT_BIZ_BOMB 65
#define ACT_SOUND_TOGGLE 66
#define ACT_COURT_WOO 67
#define ACT_SITDOWN 68
#define ACT_HIT_CAPO 69
#define ACT_DISTRICT_EVENT 70
#define ACT_LEGEND 71
#define ACT_ARM_CREW 72
#define ACT_SCOUT_HIDEOUT 73
#define ACT_RAID_HIDEOUT 74
#define ACT_HEIR_TUTOR 75
#define ACT_RESCUE_VINCENZO 76

typedef struct UnitSpec
{
    const WCHAR* title;
    int cost;
    int health;
    int accuracy;
    int damageLow;
    int damageHigh;
    int attacks;
} UnitSpec;

typedef struct Unit
{
    int type;
    int alive;
    int level;
    int xp;
    int health;
    int maxHealth;
    int accuracy;
    int assignment;
    int assignedBusiness;
    int armed;
    WCHAR name[NAME_CHARS];
} Unit;

typedef struct Business
{
    int type;
    int district;
    int status;
    int baseIncome;
    int managerAlive;
    int hasLawyer;
    WCHAR name[NAME_CHARS];
} Business;

typedef struct Wife
{
    int status;
    int athletic;
    int clever;
    int charm;
    int gestationDays;
    WCHAR name[NAME_CHARS];
} Wife;

typedef struct Candidate
{
    int active;
    int athletic;
    int clever;
    int charm;
    int dowry;
    int wooed;
    WCHAR name[NAME_CHARS];
} Candidate;

typedef struct Rival
{
    int alive;
    int anger;
    int strength;
    int district;
    int hideoutHoodlums;
    int hideoutSoldiers;
    int hideoutScouted;
    int hideoutDownDays;
    WCHAR name[NAME_CHARS];
} Rival;

typedef struct DistrictEvent
{
    int active;
    int district;
    int daysLeft;
} DistrictEvent;

typedef struct Heir
{
    int exists;
    int type;
    int quality;
    int placement;
    int placedBusiness;
    WCHAR name[NAME_CHARS];
} Heir;

typedef struct PendingCombat
{
    int active;
    int kind;
    int arg;
    int hoodlums;
    int soldiers;
    int veterans;
    int bosses;
    int cops;
    WCHAR label[NAME_CHARS * 2];
} PendingCombat;

typedef struct PhoneOffer
{
    int active;
    int kind;
    int unitType;
    int price;
    int reward;
    int targetBusiness;
} PhoneOffer;

typedef struct GameState
{
    int day;
    int missionStage;
    int cash;
    int heat;
    int rank;
    int gunplayXp;
    int gunplayLevel;
    int businessXp;
    int businessLevel;
    int playerHealth;
    int playerMaxHealth;
    int weaponTier;
    int medpacks;
    int location;
    int carryingCrate;
    int churchFound;
    int copsHostile;
    int chiefDeadline;
    int chiefNextDay;
    int bountyDays;
    int vincenzoAlive;
    int vincenzoTaken;
    int romanoKnown;
    int angeloKnown;
    int sonnyKnown;
    int brothersDead[3];
    int policePresence[NUM_DISTRICTS];
    int menuId;
    int ctxBusiness;
    int ctxUnit;
    int favorDistrict;
    int favorEnemies;
    int contractBusiness;
    int contractReward;
    int gameOver;
    int gameWon;
    int generation;
    int mistress;
    int mistressDays;
    int wifeSuspicion;
    int wifeConfront;
    int inLawBusiness;
    int inLawTribute;
    int kidnapVictim;
    int kidnapRansom;
    int kidnapDaysLeft;
    int kidnapDistrict;
    int ammo;
    int carTier;
    int soundOn;
    int courtedDay;
    int mapCollapsed;
    int tutoredDay;
    int vincenzoFound;
    int vincenzoRescued;
    int statKills;
    int statMenLost;
    int statFightsWon;
    int statFightsLost;
    int statBizSeized;
    int statCaposKilled;
    int statRansomsPaid;
    int intimidation[NUM_DISTRICTS];
    int mapPing[NUM_DISTRICTS];
    int mapPingDay[NUM_DISTRICTS];
    WCHAR playerName[NAME_CHARS];
    DistrictEvent districtEvent;
    Rival rivals[NUM_RIVALS];
    unsigned int rngSeed;
    PendingCombat pending;
    PhoneOffer phone;
    Candidate candidates[MAX_CANDIDATES];
    Wife wife;
    Heir heirs[MAX_HEIRS];
    Unit crew[MAX_CREW];
    Business businesses[MAX_BUSINESSES];
} GameState;

typedef struct App
{
    HWND windowMain;
    HWND windowLog;
    HWND windowStatus;
    HWND windowChoices;
    HWND windowButton;
    HWND windowMapButton;
    HFONT font;
    HFONT fontBanner;
    HFONT fontCaption;
    HFONT fontSmall;
    HBRUSH brushBackground;
    HBRUSH brushPane;
    HBRUSH brushSelect;
    HBRUSH brushGold;
    HBRUSH brushBlood;
    HBRUSH brushText;
    HBRUSH brushPolice;
    HPEN penGold;
    HPEN penGoldDim;
    HPEN penStripe;
    HPEN penBlood;
    HPEN penPolice;
    RECT rectLog;
    RECT rectStatus;
    RECT rectChoices;
    RECT rectFamily;
    RECT rectMap;
    int choiceCount;
    int choiceActions[MAX_CHOICES];
    void* sounds[SOUND_COUNT];
    int soundBytes[SOUND_COUNT];
    WCHAR* formatBuffer;
    GameState* game;
} App;

void* AllocZeroed(SIZE_T bytes);
void FreeMemory(void* memory);
int RandomRange(GameState* game, int span);
int RandomBetween(GameState* game, int low, int high);
void CopyText(WCHAR* dest, const WCHAR* source, int destChars);

void UiLog(App* app, const WCHAR* text);
void UiLogFmt(App* app, const WCHAR* format, ...);
void RefreshUi(App* app);
void AddChoice(App* app, const WCHAR* label, int packedAction);

void NewGame(App* app);
void EndDay(App* app);
int PeekSave(int slot, GameState* peek);
void RollHideout(GameState* game, int rivalIndex);
void AddHeat(App* app, int amount);
void GainBusinessXp(App* app, int amount);
void GainGunplayXp(App* app, int amount);
void BuildStatusText(App* app);
void SaveGame(App* app, int slot, int silent);
void LoadGame(App* app, int slot);
const WCHAR* DistrictName(int district);
const UnitSpec* GetUnitSpec(int type);
int CountCrew(GameState* game);
int LeadershipCap(GameState* game);
int ControlsBusinessType(GameState* game, int type);
void RecruitUnit(App* app, int type, int silent);
const WCHAR* KidnapVictimName(GameState* game);
void KidnapVictimDies(App* app);
void TagMap(App* app, int district, int kind);
Rival* RivalInDistrict(GameState* game, int district);
void RaiseRivalAnger(App* app, int district, int amount);
int RunApp(void);

void BuildChoices(App* app);
void HandleAction(App* app, int packedAction);
void QueueCombat(App* app, const WCHAR* label);
void ResolveCombat(App* app, int approach);
void ResolveRaid(App* app, int kind);

void SoundBuild(App* app);
void SoundPlay(App* app, int kind);

#endif
