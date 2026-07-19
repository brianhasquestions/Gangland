// gangland.h - the single shared header for Gangland (Text Edition).
//
// The project deliberately uses one header: the coding standard forbids
// mutable globals, so every module operates on the two structs defined
// here (GameState for the world, App for the window), and the save
// system writes GameState to disk as raw bytes - its full layout must
// therefore be visible to every translation unit anyway.
//
// Module map (each .c file includes only this header):
//   entry.c   - the no-CRT entry point; calls RunApp and exits
//   util.c    - heap helpers, RNG, bounded string copy, memset/memcpy
//   main.c    - window, controls, painting (map, family strip), RunApp
//   world.c   - game-state lifecycle: new game, day cycle, save/load
//   menu.c    - builds the choice list for whichever menu is active
//   actions.c - executes the choice the player picked
//   combat.c  - the battle engine, raids, and fight outcomes
//   sound.c   - procedural WAV synthesis and playback
//
// Build rules: pure C, Win32 API only, no C runtime (/NODEFAULTLIB),
// /W4 clean, optimized for size. Buffers are heap-allocated, functions
// take at most three parameters, and cleanup paths use goto.

#ifndef GANGLAND_H
#define GANGLAND_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// ---------------------------------------------------------------------------
// Capacities and shared limits.
// GameState embeds fixed-capacity arrays (no pointers) so a save file is a
// single WriteFile of the struct. Changing any of these breaks save
// compatibility - bump SAVE_VERSION in world.c when you do.
// ---------------------------------------------------------------------------

#define NUM_DISTRICTS 5
#define MAX_CREW 16
#define MAX_BUSINESSES 14
#define MAX_HEIRS 3
#define MAX_CANDIDATES 3
#define MAX_CHOICES 24
#define NAME_CHARS 32

// Menu choices carry a packed action: code + (argument * ACTION_ARG_BASE).
// HandleAction unpacks with % and /. Codes must stay below the base.
#define ACTION_ARG_BASE 1000
#define PACK_ACTION(code, arg) ((code) + ((arg) * ACTION_ARG_BASE))

// Three manual slots plus a fourth written silently every dawn.
#define SAVE_SLOTS 3
#define FORMAT_BUFFER_CHARS 1024

// ---------------------------------------------------------------------------
// Districts. Index doubles as the map-cell index and the district-event kind.
// Police presence per district lives in GameState (the Suburbs start at zero;
// bombing a station can zero the others).
// ---------------------------------------------------------------------------

#define DISTRICT_LITTLE_ITALY 0
#define DISTRICT_DOWNTOWN 1
#define DISTRICT_DOCKS 2
#define DISTRICT_MARKET 3
#define DISTRICT_SUBURBS 4

// ---------------------------------------------------------------------------
// Crew unit types. Stats come from the UNIT_SPECS table in world.c.
// UNIT_ENFORCER exists only as a spec for heir-derived fighters; it is never
// recruited into the crew array.
// ---------------------------------------------------------------------------

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

// Where a crew member stands: with the boss, on the safe-house doors, posted
// at a business (assignedBusiness says which), or working a street corner
// (street girls only - daily income with daily consequences).
#define ASSIGN_SQUAD 0
#define ASSIGN_SAFEHOUSE 1
#define ASSIGN_BUSINESS 2
#define ASSIGN_CORNER 3

#define AUTOSAVE_SLOT 3

// ---------------------------------------------------------------------------
// Businesses: what they sell and who controls them. Extorted businesses pay
// a daily vig; owned ones pay full income minus upkeep but need a living
// manager and ideally a guard. Destroyed ones are gone for good.
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// The wife's arc: courted, married, killed, or walked out (a wife who LEAVES
// sells your routes to rivals on the way).
// ---------------------------------------------------------------------------

#define WIFE_NONE 0
#define WIFE_ENGAGED 1
#define WIFE_MARRIED 2
#define WIFE_DEAD 3
#define WIFE_LEFT 4

// Vehicles reduce travel ambushes; a sedan or better unlocks the drive-by
// combat approach.
#define CAR_NONE 0
#define CAR_JALOPY 1
#define CAR_SEDAN 2
#define CAR_ARMORED 3

// GameState.kidnapVictim: NOBODY, an heir index (0..MAX_HEIRS-1), or the wife.
#define KIDNAP_NOBODY -1
#define KIDNAP_WIFE 3

// Procedural sound effects, synthesized once at startup into App.sounds.
#define SOUND_GUNFIGHT 0
#define SOUND_CASH 1
#define SOUND_BELLS 2
#define SOUND_DEATH 3
#define SOUND_FANFARE 4
#define SOUND_COUNT 5

// ---------------------------------------------------------------------------
// Rival capos. Three named rivals hold the non-safe districts; anger rises
// when you squeeze their turf and decays daily. TENSE rivals lean on your
// rackets; WAR rivals raid your safe house by name and redden the map.
// ---------------------------------------------------------------------------

#define NUM_RIVALS 3
#define RIVAL_ANGER_TENSE 30
#define RIVAL_ANGER_WAR 70
#define RIVAL_SITDOWN_MIN 20
#define RIVAL_HIT_MIN 30

// Map pings: one drawn marker per district, stamped by TagMap when something
// happens there and expired after PING_LIFETIME_DAYS.
#define PING_NONE 0
#define PING_FIGHT 1
#define PING_MONEY 2
#define PING_ALERT 3
#define PING_EVENT 4
#define PING_FAMILY 5
#define PING_LIFETIME_DAYS 3

// ---------------------------------------------------------------------------
// Heirs: the three underboss callings and where an heir can be placed.
// Enforcers fight (PLACE_FIELD joins your squad), lawyers run businesses or
// cool heat at the courthouse, seductresses charm from the phone desk or the
// field. Only blood can lead the family - an heir is your succession.
// ---------------------------------------------------------------------------

#define HEIR_ENFORCER 0
#define HEIR_LAWYER 1
#define HEIR_SEDUCTRESS 2

#define PLACE_HOME 0
#define PLACE_FIELD 1
#define PLACE_BUSINESS 2
#define PLACE_DESK 3
#define PLACE_COURTHOUSE 4

// ---------------------------------------------------------------------------
// Offensive fight kinds, stored in PendingCombat.kind so FinishCombat knows
// what a victory means (arg carries the target: a business index, rival
// index, or mission stage).
// ---------------------------------------------------------------------------

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

// Defensive raids resolved without an approach menu. Losing one costs cash,
// not the boss's life - he escapes at 1 hp. Only fights he starts can kill.
#define RAID_COPS 0
#define RAID_RIVALS 1
#define RAID_BOUNTY 2

// What the safe-house telephone is offering.
#define PHONE_NONE 0
#define PHONE_UNIT 1
#define PHONE_HIT 2
#define PHONE_TIP 3

// Rank drives the stipend, leadership cap, and the stars on the family strip.
#define RANK_ERRAND_BOY 0
#define RANK_CAPO 1
#define RANK_DON 2

// ---------------------------------------------------------------------------
// Menu screens. GameState.menuId selects which builder BuildChoices runs;
// ctxBusiness / ctxUnit carry the selected item into submenus.
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// Action codes, dispatched by HandleAction. Most take a packed argument
// (see PACK_ACTION): a district, unit index, business index, heir index,
// rival index, save slot, weapon tier, or approach number.
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// Data types. Everything inside GameState is plain data (no pointers, no
// handles) because saves are a raw dump of the struct.
// ---------------------------------------------------------------------------

// Static description of a crew unit type (table in world.c).
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

// One hired man or woman. A slot is empty when type is UNIT_NONE; death sets
// alive to 0 and frees the slot. armed means a crew Tommy gun (+1 attack).
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

// One of the city's fourteen businesses. hasLawyer marks a lawyer heir
// installed for the revenue boost; a dead manager halves an owned take.
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

// The wife. Her traits (1-5 each) were fixed at courtship and bias which
// heirs she bears; gestationDays counts down to a birth.
typedef struct Wife
{
    int status;
    int athletic;
    int clever;
    int charm;
    int gestationDays;
    WCHAR name[NAME_CHARS];
} Wife;

// A prospective bride at the social club. wooed is the cumulative dowry
// discount earned by courting her over evenings (floored in actions.c).
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

// A rival capo and the hoodlum hideout in his district. The hideout persists
// even after the capo dies; hideoutDownDays counts until new faces move in,
// and hideoutScouted means the player knows the guard count.
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

// The one-at-a-time district opportunity (feast, gala, freighter, vendor
// war, moonshiner). district doubles as the event kind.
typedef struct DistrictEvent
{
    int active;
    int district;
    int daysLeft;
} DistrictEvent;

// A child of the marriage. quality (roughly 30-95) scales an enforcer's
// combat stats and is raised by tutoring; placedBusiness applies when a
// lawyer sits at PLACE_BUSINESS.
typedef struct Heir
{
    int exists;
    int type;
    int quality;
    int placement;
    int placedBusiness;
    WCHAR name[NAME_CHARS];
} Heir;

// The offensive fight being lined up while the approach menu is open.
// Enemy counts by tier; cops join at queue time if the district is hot.
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

// Whatever the safe-house telephone is currently offering.
typedef struct PhoneOffer
{
    int active;
    int kind;
    int unitType;
    int price;
    int reward;
    int targetBusiness;
} PhoneOffer;

// The entire persistent world. Saved to disk verbatim (header + raw struct),
// which is why it holds no pointers and only fixed-size arrays. Any layout
// change must bump SAVE_VERSION in world.c.
typedef struct GameState
{
    int day;
    int missionStage;              // campaign progress: 0 crate .. 7 vendetta done
    int cash;
    int heat;                      // 0-100 police attention
    int rank;
    int gunplayXp;
    int gunplayLevel;
    int businessXp;
    int businessLevel;
    int playerHealth;
    int playerMaxHealth;
    int weaponTier;                // 0 pistol .. 3 custom Tommy
    int medpacks;
    int location;
    int carryingCrate;             // first mission's ammunition crate
    int churchFound;
    int copsHostile;               // set when the Chief goes unpaid
    int chiefDeadline;             // days left to pay, 0 = no demand open
    int chiefNextDay;
    int bountyDays;                // nights of hired guns remaining
    int vincenzoAlive;
    int vincenzoTaken;             // the mid-game kidnapping fired
    int romanoKnown;               // brother locations revealed by tips
    int angeloKnown;
    int sonnyKnown;
    int brothersDead[3];
    int policePresence[NUM_DISTRICTS];
    int menuId;                    // which BuildChoices screen is active
    int ctxBusiness;               // selection context for submenus
    int ctxUnit;                   // doubles as the selected heir index
    int favorDistrict;             // -1 = no citizen favor pending
    int favorEnemies;
    int contractBusiness;          // -1 = no phone contract accepted
    int contractReward;
    int gameOver;
    int gameWon;
    int generation;                // increments on succession
    int mistress;
    int mistressDays;
    int wifeSuspicion;             // 0-100; at the limit she confronts you
    int wifeConfront;
    int inLawBusiness;             // her family's shop, -1 before marriage
    int inLawTribute;              // 1 = squeezing the in-laws
    int kidnapVictim;              // KIDNAP_NOBODY, heir index, or KIDNAP_WIFE
    int kidnapRansom;
    int kidnapDaysLeft;
    int kidnapDistrict;
    int ammo;                      // one shell per fighter per fight
    int carTier;
    int soundOn;
    int courtedDay;                // last day an evening was spent courting
    int mapCollapsed;
    int tutoredDay;                // last day an heir was tutored
    int vincenzoFound;             // Pier 13 basement tip received
    int vincenzoRescued;
    int statKills;                 // lifetime tallies for the Legend screen
    int statMenLost;
    int statFightsWon;
    int statFightsLost;
    int statBizSeized;
    int statCaposKilled;
    int statRansomsPaid;
    int intimidation[NUM_DISTRICTS];   // days shopkeepers pay without a fight
    int mapPing[NUM_DISTRICTS];
    int mapPingDay[NUM_DISTRICTS];
    WCHAR playerName[NAME_CHARS];  // changes when an heir succeeds the boss
    DistrictEvent districtEvent;
    Rival rivals[NUM_RIVALS];
    unsigned int rngSeed;          // linear congruential generator state
    PendingCombat pending;
    PhoneOffer phone;
    Candidate candidates[MAX_CANDIDATES];
    Wife wife;
    Heir heirs[MAX_HEIRS];
    Unit crew[MAX_CREW];
    Business businesses[MAX_BUSINESSES];
} GameState;

// Everything runtime-only: window handles, GDI resources, the synthesized
// sounds, and the choice list backing the listbox. Never saved. The pens,
// brushes, and fonts live for the process and are created in CreateControls.
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
    RECT rectLog;                  // pane rectangles, set by LayoutControls
    RECT rectStatus;
    RECT rectChoices;
    RECT rectFamily;
    RECT rectMap;                  // zero-height while the map is collapsed
    int choiceCount;
    int choiceActions[MAX_CHOICES];    // packed action per listbox row
    void* sounds[SOUND_COUNT];     // in-memory WAVs for PlaySound(SND_MEMORY)
    int soundBytes[SOUND_COUNT];
    WCHAR* formatBuffer;           // shared scratch for UiLogFmt/AddChoiceFmt
    GameState* game;
} App;

// ---------------------------------------------------------------------------
// util.c - allocation, randomness, bounded copies.
// ---------------------------------------------------------------------------

void* AllocZeroed(SIZE_T bytes);
void FreeMemory(void* memory);
int RandomRange(GameState* game, int span);            // 0 .. span-1
int RandomBetween(GameState* game, int low, int high); // inclusive
void CopyText(WCHAR* dest, const WCHAR* source, int destChars);

// ---------------------------------------------------------------------------
// main.c - the window and everything drawn in it.
// ---------------------------------------------------------------------------

void UiLog(App* app, const WCHAR* text);           // append a line to the log
void UiLogFmt(App* app, const WCHAR* format, ...); // printf-style append
void RefreshUi(App* app);                          // rebuild status + choices
void AddChoice(App* app, const WCHAR* label, int packedAction);

// ---------------------------------------------------------------------------
// world.c - game-state lifecycle, the day cycle, and persistence.
// ---------------------------------------------------------------------------

void NewGame(App* app);
void EndDay(App* app);                             // income, wages, night events
int PeekSave(int slot, GameState* peek);           // read a save without loading
void RollHideout(GameState* game, int rivalIndex); // (re)populate a hideout
void AddHeat(App* app, int amount);                // scaled by local police
void GainBusinessXp(App* app, int amount);
void GainGunplayXp(App* app, int amount);
void BuildStatusText(App* app);
void SaveGame(App* app, int slot, int silent);
void LoadGame(App* app, int slot);
const WCHAR* DistrictName(int district);
const UnitSpec* GetUnitSpec(int type);
int CountCrew(GameState* game);
int LeadershipCap(GameState* game);                // how many you can lead
int ControlsBusinessType(GameState* game, int type);
void RecruitUnit(App* app, int type, int silent);
const WCHAR* KidnapVictimName(GameState* game);
void KidnapVictimDies(App* app);
void TagMap(App* app, int district, int kind);     // stamp a map ping
Rival* RivalInDistrict(GameState* game, int district);
void RaiseRivalAnger(App* app, int district, int amount);
int RunApp(void);                                  // defined in main.c; called by entry.c

// ---------------------------------------------------------------------------
// menu.c / actions.c - the choice list and what picking one does.
// ---------------------------------------------------------------------------

void BuildChoices(App* app);
void HandleAction(App* app, int packedAction);

// ---------------------------------------------------------------------------
// combat.c - offensive fights (via the approach menu) and defensive raids.
// ---------------------------------------------------------------------------

void QueueCombat(App* app, const WCHAR* label);    // pending -> approach menu
void ResolveCombat(App* app, int approach);
void ResolveRaid(App* app, int kind);

// ---------------------------------------------------------------------------
// sound.c - integer-math WAV synthesis, built once at startup.
// ---------------------------------------------------------------------------

void SoundBuild(App* app);
void SoundPlay(App* app, int kind);

#endif
