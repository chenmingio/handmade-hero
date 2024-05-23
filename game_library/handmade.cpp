#include "handmade_types.h"
#include "handmade.h"
#include "handmade_intrinsics.h"
#include "math.h"
#include "assert.h"

internal
void GameUpdateSound(game_state *GameState, game_sound_output_buffer *SoundBuffer, uint32 ToneHz) {
    int16 ToneVolume = 3000;
    uint32 WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz;

    int16 *SampleOut = SoundBuffer->Samples;
//    for (uint32 SampleIndex = 0;
//         SampleIndex < SoundBuffer->SampleCount;
//         ++SampleIndex) {
//        // TODO(casey): Draw this out for people
//        int16 SampleValue = 0;
//        *SampleOut++ = SampleValue;
//        *SampleOut++ = SampleValue;
//    }
}

inline uint32
TruncateReal32ToUInt32(real32 Real32) {
    uint32 Result = (uint32) Real32;
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

    int32 MinX = RoundReal32ToInt32(RealMinX);
    int32 MinY = RoundReal32ToInt32(RealMinY);
    int32 MaxX = RoundReal32ToInt32(RealMaxX);
    int32 MaxY = RoundReal32ToInt32(RealMaxY);

    if (MinX < 0) {
        MinX = 0;
    }

    if (MinY < 0) {
        MinY = 0;
    }

    if (MaxX > Buffer->Width) {
        MaxX = Buffer->Width;
    }

    if (MaxY > Buffer->Height) {
        MaxY = Buffer->Height;
    }


    uint8 *Row = ((uint8 *) Buffer->Memory +
                  MinX * Buffer->BytesPerPixel +
                  MinY * Buffer->Pitch);
    for (int32 Y = MinY; Y < MaxY; ++Y) {
        uint32 *Pixel = (uint32 *) Row;
        for (int32 X = MinX; X < MaxX; ++X) {
            *Pixel++ = Color;
        }

        Row += Buffer->Pitch;
    }
}

inline
uint32 GetTileValueUnchecked(world *World, tile_map *TileMap, int32 TileX, int32 TileY) {
    assert(TileMap);
    assert((TileX >= 0) && (TileX < World->CountX) &&
           (TileY >= 0) && (TileY < World->CountY));

    uint32 TileMapValue = TileMap->Tiles[TileY * World->CountX + TileX];
    return (TileMapValue);
}

inline
tile_map *GetTileMap(world *World, int32 TileMapX, int32 TileMapY) {
    tile_map *Result = &(World->TileMaps[TileMapY * World->TileMapCountX + TileMapX]);
    return Result;
}

inline
bool32 isTileMapPointEmpty(world *World, tile_map *TileMap, int32 TestTileX, int32 TestTileY) {
    bool32 Empty = false;

    if (TileMap) {
        if ((TestTileX >= 0) && (TestTileX < World->CountX) &&
            (TestTileY >= 0) && (TestTileY < World->CountY)) {
            uint32 TileMapValue = GetTileValueUnchecked(World, TileMap, TestTileX, TestTileY);
            Empty = (TileMapValue == 0);
        }
    }

    return (Empty);
}

internal
canonical_position GetCanonicalPosition(world *World, raw_position Pos) {
    canonical_position Result;

    Result.TileMapX = Pos.TileMapX;
    Result.TileMapY = Pos.TileMapY;

    real32 X = Pos.X - World->UpperLeftX;
    real32 Y = Pos.Y - World->UpperLeftY;
    Result.TileX = FloorReal32ToInt32(X /World->TileSideInPixels);
    Result.TileY = FloorReal32ToInt32(Y /World->TileSideInPixels);

    Result.TileRelX = X - Result.TileX *World->TileSideInPixels;
    Result.TileRelY = Y - Result.TileY *World->TileSideInPixels;

    assert(Result.TileRelX >= 0);
    assert(Result.TileRelY >= 0);
    assert(Result.TileRelX <World->TileSideInPixels);
    assert(Result.TileRelY <World->TileSideInPixels);

    if (Result.TileX < 0) {
        Result.TileX = World->CountX + Result.TileX;
        --Result.TileMapX;
    }

    if (Result.TileY < 0) {
        Result.TileY = World->CountY + Result.TileY;
        --Result.TileMapY;
    }

    if (Result.TileX >= World->CountX) {
        Result.TileX = Result.TileX - World->CountX;
        ++Result.TileMapX;
    }

    if (Result.TileY >= World->CountY) {
        Result.TileY = Result.TileY - World->CountY;
        ++Result.TileMapY;
    }

    return (Result);
}

internal
bool32 IsWorldPointEmpty(world *World, raw_position TestPos) {
    canonical_position cp = GetCanonicalPosition(World, TestPos);
    tile_map *TileMap = GetTileMap(World, cp.TileMapX, cp.TileMapY);
    return isTileMapPointEmpty(World, TileMap, cp.TileX, cp.TileY);
}


extern "C" {
void GameGetSoundSamples(game_memory *Memory, game_sound_output_buffer *SoundBuffer) {
    game_state *GameState = (game_state *) Memory->PermanentStorage;
    GameUpdateSound(GameState, SoundBuffer, GameState->ToneHz);
}

void GameUpdateAndRender(game_memory *Memory, game_input *input, game_offscreen_buffer *Buffer) {

#if HANDMADE_INTERNAL
    debug_read_file_result FileRead = Memory->DEBUGPlatformReadEntireFile(const_cast<char*>("test_background.bmp"));
    if (FileRead.Contents) {
        Memory->DEBUGPlatformWriteEntireFile(const_cast<char*>("test_background_copy.bmp"), FileRead.ContentsSize, FileRead.Contents);
        Memory->DEBUGPlatformFreeFileMemory(FileRead.Contents);
    }
#endif

#define TILE_MAP_COUNT_X 17
#define TILE_MAP_COUNT_Y 9
    uint32 Tiles00[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
            {
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                    {1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
                    {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
                    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                    {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                    {1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
                    {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
                    {1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
                    {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1},
            };

    uint32 Tiles01[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
            {
                    {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1},
                    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            };

    uint32 Tiles10[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
            {
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                    {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1},
            };

    uint32 Tiles11[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
            {
                    {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1},
                    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            };

    tile_map TileMaps[2][2];

    TileMaps[0][0].Tiles = (uint32 *) Tiles00;
    TileMaps[0][1].Tiles = (uint32 *) Tiles10;
    TileMaps[1][0].Tiles = (uint32 *) Tiles01;
    TileMaps[1][1].Tiles = (uint32 *) Tiles11;

    world World;
    World.TileMapCountX = 2;
    World.TileMapCountY = 2;
    World.CountX = TILE_MAP_COUNT_X;
    World.CountY = TILE_MAP_COUNT_Y;

    World.TileSideInMeters = 1.4f;
    World.TileSideInPixels = 60;
    World.UpperLeftX = -(real32) World.TileSideInPixels / 2;
    World.UpperLeftY = 0;

    World.TileMaps = (tile_map *) TileMaps;

    real32 PlayerWidth = (real32) 0.75f * World.TileSideInPixels;
    real32 PlayerHeight = (real32) World.TileSideInPixels;

    game_state *GameState = (game_state *) Memory->PermanentStorage;

    // run once
    if (!Memory->IsInitialized) {

        GameState->PlayerX = 150;
        GameState->PlayerY = 150;
        GameState->PlayerTileMapX = 0;
        GameState->PlayerTileMapY = 0;

        Memory->IsInitialized = true;
    }

    tile_map *TileMap = GetTileMap(&World, GameState->PlayerTileMapX, GameState->PlayerTileMapY);
    assert(TileMap);

    game_controller_input *Input0 = &input->Controllers[0];
    game_controller_input *Input1 = &input->Controllers[0];



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


    dPlayerX *= 100.0f;
    dPlayerY *= 100.0f;

    real32 NewPlayerX = GameState->PlayerX + input->dtForFrame * dPlayerX;
    real32 NewPlayerY = GameState->PlayerY + input->dtForFrame * dPlayerY;

    raw_position PlayerPos = {GameState->PlayerTileMapX, GameState->PlayerTileMapY, NewPlayerX, NewPlayerY};
    raw_position PlayerLeft = PlayerPos;
    PlayerLeft.X -= PlayerWidth * 0.5f;
    raw_position PlayerRight = PlayerPos;
    PlayerRight.X += PlayerWidth * 0.5f;

    if (
            IsWorldPointEmpty(&World, PlayerPos) &&
            IsWorldPointEmpty(&World, PlayerLeft) &&
            IsWorldPointEmpty(&World, PlayerRight)
            ) {
        canonical_position CanPos = GetCanonicalPosition(&World, PlayerPos);

        GameState->PlayerTileMapX = CanPos.TileMapX;
        GameState->PlayerTileMapY = CanPos.TileMapY;
        GameState->PlayerX = World.UpperLeftX + World.TileSideInPixels * CanPos.TileX + CanPos.TileRelX;
        GameState->PlayerY = World.UpperLeftY + World.TileSideInPixels * CanPos.TileY + CanPos.TileRelY;
    }

    // draw background
    DrawRectangle(Buffer, 0.0f, 0.0f, (real32) Buffer->Width, (real32) Buffer->Height,
                  1.0f, 0.0f, 0.1f);

    // draw tile map
    // draw map
    for (int32 Y = 0; Y < 9; ++Y) {
        for (int32 X = 0; X < 17; ++X) {

            real32 Gray = 0.5f;
            uint32 TileValue = GetTileValueUnchecked(&World, TileMap, X, Y);

            if (TileValue == 0) {
                Gray = 1.0f;
            }

            real32 MinX = World.UpperLeftX + X * World.TileSideInPixels;
            real32 MinY = World.UpperLeftY + Y * World.TileSideInPixels;
            real32 MaxX = MinX + World.TileSideInPixels;
            real32 MaxY = MinY + World.TileSideInPixels;

            DrawRectangle(Buffer, MinX, MinY, MaxX, MaxY, Gray, Gray, Gray);

        }
    }

    real32 PlayerR = 1.0f;
    real32 PlayerG = 0.0f;
    real32 PlayerB = 0.0f;

    real32 PlayerLeftX = GameState->PlayerX - PlayerWidth * 0.5f;
    real32 PlayerTopY = GameState->PlayerY - PlayerHeight;

    DrawRectangle(Buffer, PlayerLeftX, PlayerTopY, PlayerLeftX + PlayerWidth, PlayerTopY + PlayerHeight, PlayerR,
                  PlayerG, PlayerB);

}
}



