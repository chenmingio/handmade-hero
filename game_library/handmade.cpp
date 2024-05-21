#include "handmade_types.h"
#include "handmade.h"

internal
void GameUpdateSound(game_sound_output_buffer *SoundBuffer, uint32 ToneHz) {
//    local_persist real32 tSine = 0.0f;
//    uint32 WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz;
//
//    real32 ToneVolume = 3000;
//
//    int16 *SampleOut = SoundBuffer->Samples;
//
//    for (uint32 SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex) {
//#if 0
//        real32 SineValue = sinf(tSine);
//        int16 SampleValue = (int16) (SineValue * ToneVolume);
//#else
//        int16 SampleValue = 0;
//#endif
//        *SampleOut++ = SampleValue;
//        *SampleOut++ = SampleValue;
//        tSine += 2.0f * (real32) M_PI / (real32) WavePeriod;
//    }
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

inline uint32
RoundReal32ToUInt32(real32 Real32) {
    uint32 Result = (uint32) (Real32 + 0.5f);
    // TODO(casey): Intrinsic????
    return (Result);
}

inline int32
TruncateReal32ToInt32(real32 Real32) {
    int32 Result = (int32) Real32;
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

internal
uint32 GetTileValueUnchecked(tile_map *TileMap, uint32 TileX, uint32 TileY) {
    return TileMap->Tiles[TileY * TileMap->CountX + TileX];
}

internal
bool32 isEmpty(real32 posX, real32 posY, tile_map *TileMap) {
    int32 TileX = TruncateReal32ToInt32((posX - TileMap->UpperLeftX) / TileMap->Width);
    int32 TileY = TruncateReal32ToInt32((posY - TileMap->UpperLeftY) / TileMap->Height);
    if (TileX < 0 || (uint32)TileX >= TileMap->CountX || TileY < 0 || (uint32)TileY >= TileMap->CountY) {
        return false;
    }
    uint32 TileValue = GetTileValueUnchecked(TileMap, (uint32)TileX, (uint32)TileY);
    return TileValue == 0;
}


extern "C" {
void GameGetSoundSamples(game_memory *Memory, game_sound_output_buffer *SoundBuffer) {
    game_state *GameState = (game_state *) Memory->PermanentStorage;
    GameUpdateSound(SoundBuffer, GameState->ToneHz);
}

void GameUpdateAndRender(game_memory *Memory, game_input *input, game_offscreen_buffer *Buffer) {

    game_state *GameState = (game_state *) Memory->PermanentStorage;
    // run once
    if (!Memory->IsInitialized) {

#if HANDMADE_INTERNAL
        debug_read_file_result FileRead = Memory->DEBUGPlatformReadEntireFile(const_cast<char*>("test_background.bmp"));
        if (FileRead.Contents) {
            Memory->DEBUGPlatformWriteEntireFile(const_cast<char*>("test_background_copy.bmp"), FileRead.ContentsSize, FileRead.Contents);
            Memory->DEBUGPlatformFreeFileMemory(FileRead.Contents);
        }
#endif
        Memory->IsInitialized = true;
        GameState->ToneHz = 414;
        GameState->PlayerX = 400;
        GameState->PlayerY = 300;
    }

    game_controller_input *Input0 = &input->Controllers[0];
    game_controller_input *Input1 = &input->Controllers[0];

    // update game state according to input
    if (Input1->IsAnalog) {
        GameState->ToneHz = (uint32) (Input1->EndX) * 256 + 256;
    }

    // draw background
    DrawRectangle(Buffer, 0.0f, 0.0f, (real32) Buffer->Width, (real32) Buffer->Height, 1.0f, 0.0f, 0.1f);

    // draw tile map
#define TILE_MAP_COUNT_X 17
#define TILE_MAP_COUNT_Y 9
    uint32 Tiles00[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
            {
                    {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
                    {1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
                    {1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1},
                    {1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1},
                    {1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 0},
                    {1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0, 1},
                    {1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0, 1},
                    {1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
                    {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
            };

    uint32 Tiles01[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
            {
                    {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
                    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
                    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
                    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
                    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0},
                    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
                    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
                    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
                    {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
            };

    uint32 Tiles10[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
            {
                    {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
                    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
                    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
                    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
                    {0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
                    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
                    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
                    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
                    {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
            };

    uint32 Tiles11[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
            {
                    {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
                    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
                    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
                    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
                    {0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
                    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
                    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
                    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
                    {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
            };

    tile_map TileMaps[2][2];
    TileMaps[0][0].CountX = TILE_MAP_COUNT_X;
    TileMaps[0][0].CountY = TILE_MAP_COUNT_Y;

    TileMaps[0][0].Width = 60;
    TileMaps[0][0].Height = 60;
    TileMaps[0][0].UpperLeftX = -30;
    TileMaps[0][0].UpperLeftY = 0;

    TileMaps[0][0].Tiles = (uint32 *) Tiles00;

    TileMaps[0][1] = TileMaps[0][0];
    TileMaps[0][1].Tiles = (uint32 *) Tiles01;

    TileMaps[1][0] = TileMaps[0][0];
    TileMaps[1][0].Tiles = (uint32 *) Tiles10;

    TileMaps[1][1] = TileMaps[0][0];
    TileMaps[1][1].Tiles = (uint32 *) Tiles11;

    tile_map TileMap = TileMaps[0][0];

    for (uint32 Y = 0; Y < 9; ++Y) {
        for (uint32 X = 0; X < 17; ++X) {

            real32 Gray = 0.5f;
            uint32 TileValue = GetTileValueUnchecked(&TileMap, X, Y);

            if (TileValue == 0) {
                Gray = 1.0f;
            }

            real32 MinX = TileMap.UpperLeftX + X * TileMap.Width;
            real32 MinY = TileMap.UpperLeftY + Y * TileMap.Height;
            real32 MaxX = MinX + TileMap.Width;
            real32 MaxY = MinY + TileMap.Height;

            DrawRectangle(Buffer, MinX, MinY, MaxX, MaxY, Gray, Gray, Gray);

        }
    }

    // draw player
    real32 dPlayerX = 0;
    real32 dPlayerY = 0;


    if (Input1->Right.EndedDown) {
        dPlayerX = 1.0f;
    }
    if (Input1->Left.EndedDown) {
        dPlayerX = -1.0f;
    }
    if (Input1->Down.EndedDown) {
        dPlayerY = 1.0f;
    }
    if (Input1->Up.EndedDown) {
        dPlayerY = -1.0f;
    }

    real32 PlayerR = 1.0f;
    real32 PlayerG = 0.0f;
    real32 PlayerB = 0.0f;
    real32 PlayerWidth = TileMap.Width * 0.75f;
    real32 PlayerHeight = TileMap.Height;

    dPlayerX *= input->dtForFrame * 120.0f;
    dPlayerY *= input->dtForFrame * 120.0f;

    real32 nextPlayerX = GameState->PlayerX + dPlayerX;
    real32 nextPlayerY = GameState->PlayerY + dPlayerY;

    real32 nextPlayerLeftX = nextPlayerX - PlayerWidth * 0.5f;
    real32 nextPlayerRightX = nextPlayerX + PlayerWidth * 0.5f;

    if (
            isEmpty(nextPlayerX, nextPlayerY, &TileMap) &&
            isEmpty(nextPlayerLeftX, nextPlayerY, &TileMap) &&
            isEmpty(nextPlayerRightX, nextPlayerY, &TileMap)
            ) {
        GameState->PlayerX += dPlayerX;
        GameState->PlayerY += dPlayerY;
    }


    real32 PlayerLeftX = GameState->PlayerX - PlayerWidth * 0.5f;
    real32 PlayerTopY = GameState->PlayerY - PlayerHeight;

    DrawRectangle(Buffer, PlayerLeftX, PlayerTopY, PlayerLeftX + PlayerWidth, PlayerTopY + PlayerHeight, PlayerR,
                  PlayerG, PlayerB);
}
}


