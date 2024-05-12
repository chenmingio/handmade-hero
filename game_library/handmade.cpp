#include "handmade.h"
#include <mach/mach_init.h>
#include <math.h>

struct game_offscreen_buffer {
    uint8 *Memory;
    uint32 Height;
    uint32 Width;
    uint32 BytesPerPixel;
    uint32 Pitch;
};

struct game_sound_output_buffer {
    int SamplesPerSecond;
    int SampleCount;
    int16 *Samples;
};

struct game_button_state {
    int HalfTransitionCount;
    bool32 EndedDown;
};

struct game_controller_input {
    bool32 IsAnalog;
    real32 StartX;
    real32 StartY;

    real32 MinX;
    real32 MinY;

    real32 MaxX;
    real32 MaxY;

    real32 EndX;
    real32 EndY;

    union {
        game_button_state Buttons[10];
        struct {
            game_button_state Up;
            game_button_state Down;
            game_button_state Left;
            game_button_state Right;

            game_button_state A;
            game_button_state B;
            game_button_state X;
            game_button_state Y;

            game_button_state LeftShoulder;
            game_button_state RightShoulder;
        };
    };
};

struct game_input {
    game_controller_input Controllers[2];
};

internal
void RenderWeirdGradient(game_offscreen_buffer *Buffer, int OffsetX, int OffsetY) {
    uint8 *Row = (uint8 *) Buffer->Memory;

    for (int Y = 0; Y < Buffer->Height; ++Y) {

        uint8 *Pixel = (uint8 *) Row;

        for (int X = 0; X < Buffer->Width; ++X) {

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

internal
void GameUpdateSound(game_sound_output_buffer *SoundBuffer, int ToneHz) {
    int16 ToneVolume = 3000;
    int WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz;

    int16 *SampleOut = SoundBuffer->Samples;
    for (
            int SampleIndex = 0;
            SampleIndex < SoundBuffer->
                    SampleCount;
            ++SampleIndex) {
        real32 SineValue = sinf(2.0f * 3.14f * ((real32) SampleIndex / (real32) WavePeriod));
        int16 SampleValue = (int16) (SineValue * ToneVolume);
        *SampleOut++ =
                SampleValue;
        *SampleOut++ =
                SampleValue;
    }
}

internal
void GameUpdateAndRender(game_input *input, game_offscreen_buffer *Buffer, game_sound_output_buffer *SoundBuffer) {
    local_persist int OffsetX = 0;
    local_persist int OffsetY = 0;
    local_persist int ToneHz = 440;

    game_controller_input *Input1 = &input->Controllers[0];
    game_controller_input *Input2 = &input->Controllers[1];

    if (Input1->A.EndedDown) {
        OffsetX += 1;
    }
    if (Input1->B.EndedDown) {
        OffsetX -= 1;
    }
    if (Input1->X.EndedDown) {
        OffsetY += 1;
    }
    if (Input1->Y.EndedDown) {
        OffsetY -= 1;
    }

    RenderWeirdGradient(Buffer, OffsetX, OffsetY);
    GameUpdateSound(SoundBuffer, ToneHz);
}



