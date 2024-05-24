#include "handmade_types.h"
#include "handmade.h"
#include "handmade_intrinsics.h"
#include "assert.h"
#include "stdio.h"

internal
void GameUpdateSound(game_state *GameState, game_sound_output_buffer *SoundBuffer, uint32 ToneHz) {
    int16 ToneVolume = 3000;
    uint32 WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz;

    int16 *SampleOut = SoundBuffer->Samples;
    for (uint32 SampleIndex = 0;
         SampleIndex < SoundBuffer->SampleCount;
         ++SampleIndex) {
        // TODO(casey): Draw this out for people
        int16 SampleValue = 0;
        // *SampleOut++ = SampleValue;
        // *SampleOut++ = SampleValue;
    }
}

internal void
DrawRectangle(game_offscreen_buffer *Buffer,
              real32 RealMinX, real32 RealMinY, real32 RealMaxX, real32 RealMaxY,
              real32 R, real32 G, real32 B) {


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

    // 0x123456RR
    uint32 Color = ((RoundReal32ToUInt32(R * 255.0f) << 16) |
                    (RoundReal32ToUInt32(G * 255.0f) << 8) |
                    (RoundReal32ToUInt32(B * 255.0f) << 0));

    Color = Color << 8 | 0x9F;

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
tile_map *GetTileMap(world *World, int32 TileMapX, int32 TileMapY) {
    tile_map *Result = &(World->TileMaps[TileMapY * World->TileMapCountX + TileMapX]);
    return Result;
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
void RecanonicalizeCoord(world *World, int32 TileCount, int32 *TileMap, int32 *Tile, real32 *TileRel) {
    // TODO(casey): Need to do something that doesn't use the divide/multiply method
    // for recanonicalizing because this can end up rounding back on to the tile
    // you just came from.

    // TODO(casey): Add bounds checking to prevent wrapping

    int32 Offset = FloorReal32ToInt32(*TileRel / World->TileSideInMeters);
    *Tile += Offset;
    *TileRel -= Offset * World->TileSideInMeters;

    assert(*TileRel >= 0);
    // TODO(casey): Fix floating point math so this can be <
    assert(*TileRel <= World->TileSideInMeters);

    if (*Tile < 0) {
        *Tile = TileCount + *Tile;
        --*TileMap;
    }

    if (*Tile >= TileCount) {
        *Tile = *Tile - TileCount;
        ++*TileMap;
    }
}

internal
canonical_position RecanonicalizePosition(world *World, canonical_position Pos) {
    canonical_position Result = Pos;

    RecanonicalizeCoord(World, World->CountX, &Result.TileMapX, &Result.TileX, &Pos.TileRelX);
    RecanonicalizeCoord(World, World->CountY, &Result.TileMapY, &Result.TileY, &Pos.TileRelY);

    return (Result);
}

internal
bool32 IsWorldPointEmpty(world *World, canonical_position cp) {
    tile_map *TileMap = GetTileMap(World, cp.TileMapX, cp.TileMapY);
    return isTileMapPointEmpty(World, TileMap, cp.TileX, cp.TileY);
}

internal
void printPlayerPosition(canonical_position PlayerP) {
    printf("PlayerP: TileMap(%d, %d), Tile(%d, %d), TileRel(%.2f, %.2f)\n",
           PlayerP.TileMapX, PlayerP.TileMapY,
           PlayerP.TileX, PlayerP.TileY,
           (double) PlayerP.TileRelX, (double) PlayerP.TileRelY);
}


extern "C" {
void GameGetSoundSamples(game_memory *Memory, game_sound_output_buffer *SoundBuffer) {
    game_state *GameState = (game_state *) Memory->PermanentStorage;
    GameUpdateSound(GameState, SoundBuffer, 440);
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
    World.MetersToPixels = (real32) World.TileSideInPixels / World.TileSideInMeters;
    World.UpperLeftX = -(real32) World.TileSideInPixels / 2;
    World.UpperLeftY = 0;

    real32 PlayerWidth = 1.4f;
    real32 PlayerHeight = 0.75f * PlayerWidth;

    World.TileMaps = (tile_map *) TileMaps;

    game_state *GameState = (game_state *) Memory->PermanentStorage;

    // run once
    if (!Memory->IsInitialized) {

        GameState->PlayerP.TileMapX = 0;
        GameState->PlayerP.TileMapY = 0;
        GameState->PlayerP.TileX = 3;
        GameState->PlayerP.TileY = 3;
        GameState->PlayerP.TileRelX = 5.0f;
        GameState->PlayerP.TileRelY = 5.0f;

        Memory->IsInitialized = true;
    }

    printPlayerPosition(GameState->PlayerP);

    tile_map *TileMap = GetTileMap(&World, GameState->PlayerP.TileMapX, GameState->PlayerP.TileMapY);
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


    dPlayerX *= 10.0f;
    dPlayerY *= 10.0f;

    // get new player position
    canonical_position NewPlayerP = GameState->PlayerP;
    NewPlayerP.TileRelX += input->dtForFrame * dPlayerX;
    NewPlayerP.TileRelY += input->dtForFrame * dPlayerY;
    NewPlayerP = RecanonicalizePosition(&World, NewPlayerP);

    // collision detection
    canonical_position PlayerLeft = NewPlayerP;
    PlayerLeft.TileRelX -= 0.5f * PlayerWidth;
    PlayerLeft = RecanonicalizePosition(&World, PlayerLeft);

    canonical_position PlayerRight = NewPlayerP;
    PlayerRight.TileRelX += 0.5f * PlayerWidth;
    PlayerRight = RecanonicalizePosition(&World, PlayerRight);

    if (
            IsWorldPointEmpty(&World, NewPlayerP) &&
            IsWorldPointEmpty(&World, PlayerLeft) &&
            IsWorldPointEmpty(&World, PlayerRight)
            ) {
        GameState->PlayerP = NewPlayerP;
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

            real32 MinX = World.UpperLeftX + (real32) X * World.TileSideInPixels;
            real32 MinY = World.UpperLeftY + (real32) Y * World.TileSideInPixels;
            real32 MaxX = MinX + World.TileSideInPixels;
            real32 MaxY = MinY + World.TileSideInPixels;

            DrawRectangle(Buffer, MinX, MinY, MaxX, MaxY, Gray, Gray, Gray);

        }
    }

    // draw player
    real32 PlayerR = 1.0f;
    real32 PlayerG = 0.0f;
    real32 PlayerB = 0.0f;

    real32 PlayerLeftX = World.UpperLeftX + GameState->PlayerP.TileRelX * World.MetersToPixels +
                         GameState->PlayerP.TileRelX * World.MetersToPixels - PlayerWidth * 0.5f * World.MetersToPixels;
    real32 PlayerTopY = World.UpperLeftY + GameState->PlayerP.TileRelY * World.MetersToPixels +
                        GameState->PlayerP.TileRelY * World.MetersToPixels - PlayerHeight * World.MetersToPixels;

    DrawRectangle(Buffer, PlayerLeftX, PlayerTopY,
                  PlayerLeftX + PlayerWidth * World.MetersToPixels,
                  PlayerTopY + PlayerHeight * World.MetersToPixels, PlayerR,
                  PlayerG, PlayerB);

}
}



