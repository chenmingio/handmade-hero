#include "handmade_types.h"
#include "handmade.h"

internal
void GameUpdateSound(game_sound_output_buffer *SoundBuffer, uint32 ToneHz) {
    local_persist real32 tSine = 0.0f;
    uint32 WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz;

    real32 ToneVolume = 3000;

    int16 *SampleOut = SoundBuffer->Samples;

    for (uint32 SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex) {
#if 0
        real32 SineValue = sinf(tSine);
        int16 SampleValue = (int16) (SineValue * ToneVolume);
#else
        int16 SampleValue = 0;
#endif
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

internal uint32
RoundReal32ToUInt32(real32 Real32) {
    uint32 Result = (uint32) (Real32 + 0.5f);
    // TODO(casey): Intrinsic????
    return (Result);
}

internal void
DrawRectangle(game_offscreen_buffer *Buffer,
              real32 RealMinX, real32 RealMinY, real32 RealMaxX, real32 RealMaxY,
              real32 R, real32 G, real32 B) {

    // 0x123456RR
    uint32 Color = ((RoundReal32ToUInt32(R * 255.0f) << 16) |
                    (RoundReal32ToUInt32(G * 255.0f) << 8) |
                    (RoundReal32ToUInt32(B * 255.0f) << 0));

    Color = Color << 8 | 0x9F;

    uint32 MinX = RoundReal32ToUInt32(RealMinX);
    uint32 MinY = RoundReal32ToUInt32(RealMinY);
    uint32 MaxX = RoundReal32ToUInt32(RealMaxX);
    uint32 MaxY = RoundReal32ToUInt32(RealMaxY);

//    if (MinX < 0) {
//        MinX = 0;
//    }
//
//    if (MinY < 0) {
//        MinY = 0;
//    }

    if (MaxX > Buffer->Width) {
        MaxX = Buffer->Width;
    }

    if (MaxY > Buffer->Height) {
        MaxY = Buffer->Height;
    }


    uint8 *Row = ((uint8 *) Buffer->Memory +
                  MinX * Buffer->BytesPerPixel +
                  MinY * Buffer->Pitch);
    for (uint32 Y = MinY; Y < MaxY; ++Y) {
        uint32 *Pixel = (uint32 *) Row;
        for (uint32 X = MinX; X < MaxX; ++X) {
            *Pixel++ = Color;
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

    if (Input1->Right.EndedDown) {
        GameState->OffsetX += 1;
    }
    if (Input1->Left.EndedDown) {
        GameState->OffsetX -= 1;
    }
    if (Input1->Down.EndedDown) {
        GameState->OffsetY += 1;
    }
    if (Input1->Up.EndedDown) {
        GameState->OffsetY -= 1;
    }

    DrawRectangle(Buffer, 0.0f, 0.0f, (real32) Buffer->Width, (real32) Buffer->Height, 1.0f, 0.0f, 0.1f);
    DrawRectangle(Buffer, 10.0f, 10.0f, 40.0f, 40.0f, 1.0f, 1.0f, 0.0f);
}
}


