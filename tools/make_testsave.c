#define _CRT_SECURE_NO_WARNINGS
#include "../src/gangland.h"
#include <stdio.h>
#include <string.h>

#define TOOL_SAVE_MAGIC 0x474E4147
#define TOOL_SAVE_VERSION 6

static void SetName(WCHAR* dest, const WCHAR* source)
{
    lstrcpynW(dest, source, NAME_CHARS);
}

int main(void)
{
    GameState game;
    unsigned int header[2] = { TOOL_SAVE_MAGIC, TOOL_SAVE_VERSION };
    FILE* file = NULL;

    memset(&game, 0, sizeof(game));
    game.day = 41;
    game.missionStage = 7;
    game.cash = 18540;
    game.heat = 22;
    game.rank = RANK_DON;
    game.generation = 1;
    game.gunplayLevel = 4;
    game.businessLevel = 6;
    game.playerHealth = 128;
    game.playerMaxHealth = 140;
    game.weaponTier = 3;
    game.medpacks = 2;
    game.ammo = 44;
    game.carTier = CAR_SEDAN;
    game.soundOn = 1;
    game.location = DISTRICT_LITTLE_ITALY;
    game.vincenzoTaken = 1;
    game.brothersDead[0] = 1;
    game.brothersDead[1] = 1;
    game.brothersDead[2] = 1;
    game.chiefNextDay = 45;
    game.favorDistrict = -1;
    game.contractBusiness = -1;
    game.inLawBusiness = 9;
    game.kidnapVictim = KIDNAP_NOBODY;
    game.rngSeed = 424242u;
    game.policePresence[DISTRICT_LITTLE_ITALY] = 1;
    game.policePresence[DISTRICT_DOWNTOWN] = 3;
    game.policePresence[DISTRICT_DOCKS] = 2;
    game.policePresence[DISTRICT_MARKET] = 2;
    SetName(game.playerName, L"Mario Mangano");
    game.wife.status = WIFE_MARRIED;
    game.wife.athletic = 4;
    game.wife.clever = 3;
    game.wife.charm = 5;
    SetName(game.wife.name, L"Lucia Ferrante");
    game.mistress = 1;
    game.mistressDays = 6;
    game.wifeSuspicion = 45;
    game.heirs[0].exists = 1;
    game.heirs[0].type = HEIR_ENFORCER;
    game.heirs[0].quality = 78;
    game.heirs[0].placement = PLACE_FIELD;
    game.heirs[0].placedBusiness = -1;
    SetName(game.heirs[0].name, L"Vito");
    game.heirs[1].exists = 1;
    game.heirs[1].type = HEIR_SEDUCTRESS;
    game.heirs[1].quality = 84;
    game.heirs[1].placement = PLACE_HOME;
    game.heirs[1].placedBusiness = -1;
    SetName(game.heirs[1].name, L"Isabella2");
    game.crew[0].type = UNIT_HENCHMAN;
    game.crew[0].alive = 1;
    game.crew[0].level = 3;
    game.crew[0].health = 100;
    game.crew[0].maxHealth = 110;
    game.crew[0].accuracy = 71;
    game.crew[0].assignedBusiness = -1;
    SetName(game.crew[0].name, L"Paulie");
    game.crew[1].type = UNIT_SCOUT;
    game.crew[1].alive = 1;
    game.crew[1].level = 2;
    game.crew[1].health = 60;
    game.crew[1].maxHealth = 70;
    game.crew[1].accuracy = 33;
    game.crew[1].assignedBusiness = -1;
    SetName(game.crew[1].name, L"Nico");
    game.crew[2].type = UNIT_STREET_GIRL;
    game.crew[2].alive = 1;
    game.crew[2].level = 1;
    game.crew[2].health = 50;
    game.crew[2].maxHealth = 50;
    game.crew[2].accuracy = 40;
    game.crew[2].assignedBusiness = -1;
    SetName(game.crew[2].name, L"Rosa");
    game.vincenzoTaken = 1;
    game.vincenzoFound = 1;
    game.rivals[0].alive = 1;
    game.rivals[0].district = DISTRICT_DOCKS;
    game.rivals[0].anger = 78;
    game.rivals[0].strength = 55;
    SetName(game.rivals[0].name, L"Capo Ferro");
    game.rivals[1].alive = 1;
    game.rivals[1].district = DISTRICT_DOWNTOWN;
    game.rivals[1].anger = 35;
    game.rivals[1].strength = 50;
    SetName(game.rivals[1].name, L"Capo Rizzi");
    game.rivals[2].alive = 1;
    game.rivals[2].district = DISTRICT_MARKET;
    game.rivals[2].anger = 5;
    game.rivals[2].strength = 45;
    SetName(game.rivals[2].name, L"Capo Greco");
    game.rivals[0].hideoutHoodlums = 2;
    game.rivals[0].hideoutSoldiers = 1;
    game.rivals[1].hideoutHoodlums = 1;
    game.rivals[1].hideoutSoldiers = 2;
    game.rivals[2].hideoutHoodlums = 3;
    game.rivals[2].hideoutSoldiers = 0;
    game.districtEvent.active = 1;
    game.districtEvent.district = DISTRICT_SUBURBS;
    game.districtEvent.daysLeft = 2;
    game.churchFound = 1;
    game.mapPing[DISTRICT_DOCKS] = PING_FIGHT;
    game.mapPingDay[DISTRICT_DOCKS] = 40;
    game.mapPing[DISTRICT_MARKET] = PING_MONEY;
    game.mapPingDay[DISTRICT_MARKET] = 41;
    game.mapPing[DISTRICT_SUBURBS] = PING_EVENT;
    game.mapPingDay[DISTRICT_SUBURBS] = 41;
    game.mapPing[DISTRICT_LITTLE_ITALY] = PING_FAMILY;
    game.mapPingDay[DISTRICT_LITTLE_ITALY] = 40;

    file = fopen("gangland_slot2.sav", "wb");
    if (NULL == file)
    {
        return 1;
    }
    fwrite(header, sizeof(header), 1, file);
    fwrite(&game, sizeof(game), 1, file);
    fclose(file);
    printf("wrote gangland_slot2.sav (%u bytes state)\n", (unsigned int)sizeof(game));

    game.wife.status = WIFE_NONE;
    game.wife.name[0] = L'\0';
    game.mistress = 0;
    game.rank = RANK_CAPO;
    game.cash = 6000;
    game.location = DISTRICT_DOWNTOWN;
    game.candidates[0].active = 1;
    game.candidates[0].athletic = 2;
    game.candidates[0].clever = 3;
    game.candidates[0].charm = 4;
    game.candidates[0].dowry = 9400;
    SetName(game.candidates[0].name, L"Bianca Greco");
    game.candidates[1].active = 1;
    game.candidates[1].athletic = 5;
    game.candidates[1].clever = 2;
    game.candidates[1].charm = 2;
    game.candidates[1].dowry = 11200;
    SetName(game.candidates[1].name, L"Rosa DiNapoli");
    file = fopen("gangland_slot3.sav", "wb");
    if (NULL == file)
    {
        return 1;
    }
    fwrite(header, sizeof(header), 1, file);
    fwrite(&game, sizeof(game), 1, file);
    fclose(file);
    printf("wrote gangland_slot3.sav (courtship scenario)\n");

    return 0;
}
