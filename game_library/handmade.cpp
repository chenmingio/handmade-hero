#define Kilobytes(Value) ((Value) * 1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)

#if HANDMADE_INTERNAL
struct debug_read_file_result {
    void *Contents;
    uint32 ContentsSize;
};

debug_read_file_result DEBUGPlatformReadEntireFile(char *Filename);
void DEBUGPlatformFreeFileMemory(void *Memory);

#endif

struct game_offscreen_buffer {
    uint8 *Memory;
    uint32 Height;
    uint32 Width;
    uint32 BytesPerPixel;
    uint32 Pitch;
};

struct game_sound_output_buffer {
    int16 *Samples;
    uint32 SampleCount;
    uint32 SamplesPerSecond;
};

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
        tSine += 2.0f + (real32) M_PI / (real32) WavePeriod;
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


struct game_memory {
    uint64 PermanentStorageSize;
    void *PermanentStorage;

    uint64 TransientStorageSize;
    void *TransientStorage;

    bool32 IsInitialized;
};

struct game_state {
    int OffsetX;
    int OffsetY;
    uint32 ToneHz;
};

internal
void GameUpdateAndRender(game_memory *Memory, game_input *input, game_offscreen_buffer *Buffer,
                         game_sound_output_buffer *SoundBuffer) {

    game_state *GameState = (game_state *) Memory->PermanentStorage;
    if (!Memory->IsInitialized) {
        debug_read_file_result FileReadResult = DEBUGPlatformReadEntireFile(const_cast<char*>("test_background.bmp"));

        GameState->OffsetX = 0;
        GameState->OffsetY = 0;
        GameState->ToneHz = 440;
        Memory->IsInitialized = true;
    }

    game_controller_input *Input0 = &input->Controllers[0];
    game_controller_input *Input1 = &input->Controllers[0];

    if (Input1->IsAnalog) {
        GameState->ToneHz = (uint32)(Input1->EndX) * 256 + 256;
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
    GameUpdateSound(SoundBuffer, GameState->ToneHz);
}



