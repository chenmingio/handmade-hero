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
    uint8 *Row = (uint8 *)Buffer->Memory;

    for (int Y = 0; Y < Buffer->Height; ++Y) {

        uint8 *Pixel = (uint8 *)Row;

        for(int X = 0; X < Buffer->Width; ++X) {

            /*  Pixel in memory: RR GG BB AA */

            //Red
            *Pixel = 0;
            ++Pixel;

            //Green
            *Pixel = (uint8)Y+(uint8)offsetY;
            ++Pixel;

            //Blue
            *Pixel = (uint8)X+(uint8)offsetX;
            ++Pixel;

            //Alpha
            *Pixel = 255;
            ++Pixel;
        }

        Row += Buffer->Pitch;
    }
}
