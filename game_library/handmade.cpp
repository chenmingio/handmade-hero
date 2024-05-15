#include "handmade_types.h"
#include "handmade.h"

internal
void GameUpdateSound(game_sound_output_buffer *SoundBuffer, uint32 ToneHz) {
    local_persist real32 tSine = 0.0f;
    uint32 WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz;

    real32 ToneVolume = 3000;

    int16 *SampleOut = SoundBuffer->Samples;

    for (uint32 SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex) {
        real32 SineValue = sinf(tSine);
        int16 SampleValue = (int16) (SineValue * ToneVolume);
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;
        tSine += 2.0f * (real32) M_PI / (real32) WavePeriod;
    }
}

internal
void RenderWeirdGradient(game_offscreen_buffer *Buffer, int OffsetX, int OffsetY) {
    uint8 *Row = (uint8 *) Buffer->Memory;

    for (uint32 Y = 0; Y < Buffer->Height; ++Y) {

        uint8 *Pixel = (uint8 *) Row;

        for (uint32 X = 0; X < Buffer->Width; ++X) {

            /*  Pixel in memory: RR GG BB AA */
            //Red
            *Pixel = 0;
            ++Pixel;

            //Green
            *Pixel = (uint8) Y + (uint8) OffsetX;
            ++Pixel;

            //Blue
            *Pixel = (uint8) X + (uint8) OffsetY;
            ++Pixel;

            //Alpha
            *Pixel = 255;
            ++Pixel;
        }

        Row += Buffer->Pitch;
    }
}

extern "C" {
void GameGetSoundSamples(game_memory *Memory, game_sound_output_buffer *SoundBuffer) {
    game_state *GameState = (game_state *) Memory->PermanentStorage;
    GameUpdateSound(SoundBuffer, GameState->ToneHz);
}

void GameUpdateAndRender(game_memory *Memory, game_input *input, game_offscreen_buffer *Buffer) {

    game_state *GameState = (game_state *) Memory->PermanentStorage;
    if (!Memory->IsInitialized) {

#if HANDMADE_INTERNAL
        debug_read_file_result FileRead = Memory->DEBUGPlatformReadEntireFile(const_cast<char*>("test_background.bmp"));
        if (FileRead.Contents) {
            Memory->DEBUGPlatformWriteEntireFile(const_cast<char*>("test_background_copy.bmp"), FileRead.ContentsSize, FileRead.Contents);
            Memory->DEBUGPlatformFreeFileMemory(FileRead.Contents);
        }
#endif

        GameState->OffsetX = 0;
        GameState->OffsetY = 0;
        GameState->ToneHz = 440;
        Memory->IsInitialized = true;
    }

    game_controller_input *Input0 = &input->Controllers[0];
    game_controller_input *Input1 = &input->Controllers[0];

    if (Input1->IsAnalog) {
        GameState->ToneHz = (uint32) (Input1->EndX) * 256 + 256;
    }

    if (Input1->A.EndedDown) {
        GameState->OffsetX += 1;
    }
    if (Input1->B.EndedDown) {
        GameState->OffsetX -= 1;
    }
    if (Input1->X.EndedDown) {
        GameState->OffsetY += 1;
    }
    if (Input1->Y.EndedDown) {
        GameState->OffsetY -= 1;
    }

    RenderWeirdGradient(Buffer, GameState->OffsetX, GameState->OffsetY);
}
}


