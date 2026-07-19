#include "sound.h"
#include "util.h"
#include <mmsystem.h>

#define SAMPLE_RATE 11025
#define WAV_HEADER_BYTES 44
#define SILENCE 128
#define NOISE_SEED 0x4D4F4221u
#define NOISE_MULTIPLIER 1664525u
#define NOISE_INCREMENT 1013904223u

typedef struct ToneSpec
{
    int samples;
    int frequency;
    int volume;
    int noise;
} ToneSpec;

static void WriteWavHeader(unsigned char* buffer, int dataBytes)
{
    unsigned int riffSize = 0;
    unsigned int byteRate = SAMPLE_RATE;

    riffSize = (unsigned int)dataBytes + 36;
    buffer[0] = 'R'; buffer[1] = 'I'; buffer[2] = 'F'; buffer[3] = 'F';
    buffer[4] = (unsigned char)(riffSize); buffer[5] = (unsigned char)(riffSize >> 8); buffer[6] = (unsigned char)(riffSize >> 16); buffer[7] = (unsigned char)(riffSize >> 24);
    buffer[8] = 'W'; buffer[9] = 'A'; buffer[10] = 'V'; buffer[11] = 'E';
    buffer[12] = 'f'; buffer[13] = 'm'; buffer[14] = 't'; buffer[15] = ' ';
    buffer[16] = 16; buffer[17] = 0; buffer[18] = 0; buffer[19] = 0;
    buffer[20] = 1; buffer[21] = 0;
    buffer[22] = 1; buffer[23] = 0;
    buffer[24] = (unsigned char)(SAMPLE_RATE & 0xFF); buffer[25] = (unsigned char)(SAMPLE_RATE >> 8); buffer[26] = 0; buffer[27] = 0;
    buffer[28] = (unsigned char)(byteRate & 0xFF); buffer[29] = (unsigned char)(byteRate >> 8); buffer[30] = 0; buffer[31] = 0;
    buffer[32] = 1; buffer[33] = 0;
    buffer[34] = 8; buffer[35] = 0;
    buffer[36] = 'd'; buffer[37] = 'a'; buffer[38] = 't'; buffer[39] = 'a';
    buffer[40] = (unsigned char)(dataBytes); buffer[41] = (unsigned char)(dataBytes >> 8); buffer[42] = (unsigned char)(dataBytes >> 16); buffer[43] = (unsigned char)(dataBytes >> 24);
}

static int TriangleWave(int position, int period)
{
    int wave = 0;

    wave = ((position * 4 * SILENCE) / period) - (2 * SILENCE);
    if (SILENCE < wave)
    {
        wave = (2 * SILENCE) - wave;
    }
    if ((0 - SILENCE) > wave)
    {
        wave = (0 - (2 * SILENCE)) - wave;
    }

    return wave;
}

static int SynthSegment(unsigned char* dest, const ToneSpec* spec)
{
    int index = 0;
    int period = 0;
    int wave = 0;
    int volume = 0;
    unsigned int noiseState = NOISE_SEED;

    period = (0 < spec->frequency) ? (SAMPLE_RATE / spec->frequency) : 0;
    for (index = 0; spec->samples > index; index++)
    {
        volume = (spec->volume * (spec->samples - index)) / spec->samples;
        wave = 0;
        if (0 != spec->noise)
        {
            noiseState = (noiseState * NOISE_MULTIPLIER) + NOISE_INCREMENT;
            wave = (int)((noiseState >> 16) & 0xFF) - SILENCE;
        }
        else if (0 < period)
        {
            wave = TriangleWave(index % period, period);
        }
        dest[index] = (unsigned char)(SILENCE + ((wave * volume) / SILENCE));
    }

    return spec->samples;
}

static int SynthSilence(unsigned char* dest, int samples)
{
    int index = 0;

    for (index = 0; samples > index; index++)
    {
        dest[index] = SILENCE;
    }

    return samples;
}

static void BuildOne(App* app, int kind, const ToneSpec* parts)
{
    int totalSamples = 0;
    int index = 0;
    unsigned char* buffer = NULL;
    unsigned char* cursor = NULL;

    for (index = 0; 0 != parts[index].samples; index++)
    {
        totalSamples = totalSamples + parts[index].samples;
    }
    buffer = (unsigned char*)AllocZeroed((SIZE_T)totalSamples + WAV_HEADER_BYTES);
    if (NULL == buffer)
    {
        goto Cleanup;
    }
    WriteWavHeader(buffer, totalSamples);
    cursor = buffer + WAV_HEADER_BYTES;
    for (index = 0; 0 != parts[index].samples; index++)
    {
        if (0 == parts[index].volume)
        {
            cursor = cursor + SynthSilence(cursor, parts[index].samples);
        }
        else
        {
            cursor = cursor + SynthSegment(cursor, &parts[index]);
        }
    }
    app->sounds[kind] = buffer;
    app->soundBytes[kind] = totalSamples + WAV_HEADER_BYTES;

Cleanup:

    return;
}

void SoundBuild(App* app)
{
    static const ToneSpec GUNFIGHT_PARTS[] = { { 500, 0, 115, 1 }, { 400, 0, 0, 0 }, { 500, 0, 100, 1 }, { 400, 0, 0, 0 }, { 500, 0, 85, 1 }, { 400, 0, 0, 0 }, { 700, 0, 70, 1 }, { 0, 0, 0, 0 } };
    static const ToneSpec CASH_PARTS[] = { { 700, 1320, 70, 0 }, { 150, 0, 0, 0 }, { 900, 1760, 70, 0 }, { 0, 0, 0, 0 } };
    static const ToneSpec BELLS_PARTS[] = { { 2200, 880, 75, 0 }, { 5200, 660, 85, 0 }, { 0, 0, 0, 0 } };
    static const ToneSpec DEATH_PARTS[] = { { 1600, 220, 85, 0 }, { 1600, 165, 85, 0 }, { 2800, 110, 90, 0 }, { 0, 0, 0, 0 } };
    static const ToneSpec FANFARE_PARTS[] = { { 1300, 523, 80, 0 }, { 1300, 659, 80, 0 }, { 2600, 784, 90, 0 }, { 0, 0, 0, 0 } };

    BuildOne(app, SOUND_GUNFIGHT, GUNFIGHT_PARTS);
    BuildOne(app, SOUND_CASH, CASH_PARTS);
    BuildOne(app, SOUND_BELLS, BELLS_PARTS);
    BuildOne(app, SOUND_DEATH, DEATH_PARTS);
    BuildOne(app, SOUND_FANFARE, FANFARE_PARTS);
}

void SoundPlay(App* app, int kind)
{
    if (NULL == app || 0 > kind || SOUND_COUNT <= kind)
    {
        goto Cleanup;
    }
    if (NULL == app->sounds[kind] || 0 == app->game->soundOn)
    {
        goto Cleanup;
    }
    PlaySoundW((LPCWSTR)app->sounds[kind], NULL, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);

Cleanup:

    return;
}
