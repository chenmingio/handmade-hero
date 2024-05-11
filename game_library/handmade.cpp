#include "handmade.h"

struct game_offscreen_buffer {
    uint8 *Memory;
    uint32 Height;
    uint32 Width;
    uint32 BytesPerPixel;
    uint32 Pitch;
};

internal
void GameUpdateAndRender(game_offscreen_buffer *Buffer, int offsetX, int offsetY) {
    uint8 *Row = (uint8 *) Buffer->Memory;

    for (int Y = 0; Y < Buffer->Height; ++Y) {
        uint32 *Pixel = (uint32 *) Row;

        for (int X = 0; X < Buffer->Width; ++X) {

            *Pixel = 0;
            ++Pixel;

            *Pixel = (uint8) Y + (uint8) offsetY;
            ++Pixel;

            *Pixel = X + offsetX;
            ++Pixel;

            *Pixel = 255;
            ++Pixel;
        }
        Row += Buffer->Pitch;
    }
}
