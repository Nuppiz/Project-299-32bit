#include "SRC/FILEAXS/Loadgfx.h"
#include "SRC/GENERAL/Common.h"
#include "SRC/GENERAL/Vectors.h"
#include "SRC/GENERAL/Def_math.h"
#include "SRC/GAMEPLAY/Action.h"
#include "SRC/GAMEPLAY/Def_game.h"
#include "SRC/GAMEPLAY/Str_wpn.h"
#include "SRC/SYS/Keyb.h"
#include "SRC/SYS/Video.h"
#include "SRC/SYS/Def_vid.h"
#include "SRC/SYS/Str_inp.h"
#include "SRC/SYS/Str_sys.h"

#include "Text.h"
#include "Def_draw.h"
#include "Str_draw.h"

/* Graphics drawing functions */

extern System_t System;
extern GameData_t Game;

extern Texture_array ObjectTextures;
extern Texture_array TileTextures;
extern PlanarTexture_array PlanarTileTextures;
extern Anim_array Animations;
extern Tile_t TileSet[];
extern Item_t* Items;
extern Input_t g_Input;
extern Weapon_t Weapons[];

// test subjects, del later
extern Sprite_t Rocket;
extern Sprite_t Explosion;
extern RLETexture_t RLETest;
extern RLETexture_t LargeRLETest;

Vec2 camera_offset;

Particle_t Particles[MAX_PARTICLES] = {0};
int particle_read = 0;
int particle_write = 0;

Corpse_t Corpses[MAX_CORPSES] = {0};
int corpse_read = 0;
int corpse_write = 0;

TempSprite_t TempSprites[MAX_TEMPSPRITES] = {0};
int tempsprite_read = 0;
int tempsprite_write = 0;

void drawDebug();

int boundaryCheck(int x, int y)
{
    if (x < SCREEN_WIDTH && x >= 0 && y < System.screen_height && y >= 0)
        return TRUE;
    else
        return FALSE;
}

int boundaryCheckRadius(int x, int y, int radius)
{
    if (x < SCREEN_WIDTH - radius && x >= 0 + radius && y < System.screen_height - radius && y >= 0 + radius)
        return FULLY_IN;
    else if (x - radius > SCREEN_WIDTH || x + radius < 0 ||
            y - radius > System.screen_height || y + radius < 0)
        return FULLY_OUT;
    else
        return PARTIAL;
}

int boundaryCheck_X(int x)
{
    if (x < SCREEN_WIDTH && x >= 0)
        return TRUE;
    else
        return FALSE;
}

int boundaryCheck_Y(int y)
{
    if (y < System.screen_height && y >= 0)
        return TRUE;
    else
        return FALSE;
}

void setPixelModeX(int x, int y, uint8_t color)
{
    int plane = x % 4; // use modulo to determine which of the four VGA memory planes to use
    ModeX_buf[plane][(y * 80) + (x >> 2)] = color;
}

void drawRLEGfx(int x, int y, RLETexture_t* texture)
{
    int pix_x = x;
    int pix_y = y;
    int index_y;
    int index_x;
    uint16_t file_index = 0; // current index in the data array that is being looked at
    uint16_t pixel_count = 0; // amount of pixels to set in the same colour
    uint8_t palette_index = 0; // palette index of the colour being set
    uint8_t pixel_count_jump = 2; // amount of bytes to jump in the data array for the next pixel count
    uint8_t palette_index_jump = 1; // amount of bytes to jump in the data array for the next pixel count
    uint16_t row_overflow = 0; // RLE compression doesn't have the original bitmap rows, so this is the amount of pixels moving to the next row

    // with larger files the pixel count spans two bytes
    if ((texture->flags & TEXTURE_FLAGS_LARGE_RLE))
    {
        pixel_count_jump = 3;
        palette_index_jump = 2;
    }

    for (index_y = 0; index_y < texture->height; index_y++)
    {
        for (index_x = 0; index_x < texture->width; index_x += pixel_count)
        {
            // fetch pixel count and palette index from the data
            if ((texture->flags & TEXTURE_FLAGS_LARGE_RLE))
            {
                pixel_count = (texture->pixels[file_index + 1] << 8) | texture->pixels[file_index]; // combine the to 8-bit values into one 16-bit
            }
            else
            {
                pixel_count = texture->pixels[file_index];
            }
            palette_index = texture->pixels[file_index + palette_index_jump];
            // draw pixels on screen if they're not set to transparent
            if (palette_index != TRANSPARENT_COLOR)
            {
                // if all pixels fit on the current row, draw them right away
                if (index_x + pixel_count <= texture->width)
                    _fmemset(&screen_buf[pix_y * SCREEN_WIDTH + pix_x], palette_index, pixel_count);
                // else calculate how many pixels won't fit on the current row, and only draw the ones that fit
                else
                {
                    row_overflow = index_x + pixel_count - texture->width;
                    _fmemset(&screen_buf[pix_y * SCREEN_WIDTH + pix_x], palette_index, pixel_count - row_overflow);
                }
                // move starting X coordinate by the amount we drew pixels, and jump to next pixel count
                pix_x += pixel_count;
                file_index += pixel_count_jump;
            }
            // with transparent pixels we skip drawing and just calculate possible row overflows
            else
            {
                if (index_x + pixel_count > texture->width)
                    row_overflow = index_x + pixel_count - texture->width;
                pix_x += pixel_count;
                file_index += pixel_count_jump;
            }

            // if there are pixels left over from the previous row(s), process them until we reach 0 leftovers
            // this should only happen if the files were converted without maintaining row data
            while (row_overflow > 0)
            {
                // reset X coordinate and index and advance to the next row in memory
                index_x = 0;
                pix_x = x;
                index_y++, pix_y++;

                pixel_count = row_overflow;
                // if all leftover pixels fit on the current row, draw them on the screen (if not transparent) and kill the while loop
                if (index_x + pixel_count <= texture->width)
                {
                    if (palette_index != TRANSPARENT_COLOR)
                        _fmemset(&screen_buf[pix_y * SCREEN_WIDTH + pix_x], palette_index, pixel_count);
                    row_overflow = 0;
                    index_x, pix_x += pixel_count;
                }
                // else continue decrementing the leftover pixels and advancing rows until we reach 0
                else
                {
                    row_overflow -= texture->width;
                    if (palette_index != TRANSPARENT_COLOR)
                        _fmemset(&screen_buf[pix_y * SCREEN_WIDTH + pix_x], palette_index, row_overflow);
                }
            }

        }
        pix_x = x;
        pix_y++;
    }
}

void drawTexture(int x, int y, Texture_t* texture)
{
    int pix_x = x;
    int pix_y = y;
    int index_x;
    int index_y;
    int i = 0;

    if ((texture->flags & TEXTURE_FLAGS_TRANSPARENCY))
    {
        for (index_y = 0; index_y < texture->height; index_y++)
        {
            for (index_x = 0; index_x < texture->width; index_x++)
            {
                if (texture->pixels[i] != TRANSPARENT_COLOR)
                {
                    if (pix_x < SCREEN_WIDTH && pix_y < System.screen_height)
                    {
                        SET_PIXEL(pix_x, pix_y, texture->pixels[i]);
                        i++;
                        pix_x++;
                    }
                    else
                    {
                        i++;
                        pix_x++;
                    }
                }
                else
                {
                    i++;
                    pix_x++;
                }
            }
            pix_x = x;
            pix_y++;
        }
    }
    else
    {
        for (index_y = 0; index_y < texture->height; index_y++)
        {
            _fmemcpy(&screen_buf[pix_y * SCREEN_WIDTH + pix_x],
                   &texture->pixels[(index_y + index_y) * texture->width],
                   texture->width);
        }
    }
}

void drawTextureClipped(int x, int y, Texture_t* texture)
{
    int start_x;
    int start_y;
    int end_x;
    int end_y;
    int index_y;
    int clip_x;
    int clip_y;
    int x_offset = 0;
    int y_offset = 0;

    int sprite_w = texture->width;
    int sprite_h = texture->height;

    start_x = x;
    start_y = y;

    end_x = start_x + sprite_w - 1;
    end_y = start_y + sprite_h - 1;

    if (start_x >= SCREEN_WIDTH || start_y >= System.screen_height || start_x <= -sprite_w || start_y <= -sprite_h)
        return;
    
    if (start_x < 0)
    {   
        x_offset = abs(start_x);
        start_x = 0;
    }
    
    else if (end_x >= SCREEN_WIDTH)
        end_x = SCREEN_WIDTH - 1;

    if (start_y < 0)
    {
        y_offset = abs(start_y);
        start_y = 0;
    }
    
    else if (end_y >= System.screen_height)
        end_y = System.screen_height - 1;

    clip_x = end_x - start_x + 1;
    clip_y = end_y - start_y + 1;


    for (index_y = 0; index_y < clip_y; index_y++)
    {
        _fmemcpy(&screen_buf[(start_y + index_y) * SCREEN_WIDTH + start_x], &texture->pixels[(index_y + y_offset) * texture->width + x_offset], clip_x);
    }
}

void drawPlanarTextureClipped(int x, int y, PlanarTexture_t* texture)
{
    int start_x;
    int start_y;
    int end_x;
    int end_y;
    int plane;
    int index_y;
    int clip_x;
    int clip_y;
    int x_offset = 0;
    int y_offset = 0;

    int sprite_w = texture->width;
    int sprite_h = texture->height;

    start_x = x;
    start_y = y;

    end_x = start_x + sprite_w - 1;
    end_y = start_y + sprite_h - 1;

    if (start_x >= SCREEN_WIDTH || start_y >= System.screen_height || start_x <= -sprite_w || start_y <= -sprite_h)
        return;
    
    if (start_x < 0)
    {   
        x_offset = abs(start_x);
        start_x = 0;
    }
    
    else if (end_x >= SCREEN_WIDTH)
        end_x = SCREEN_WIDTH - 1;

    if (start_y < 0)
    {
        y_offset = abs(start_y);
        start_y = 0;
    }
    
    else if (end_y >= System.screen_height)
        end_y = System.screen_height - 1;

    // all horizontal values must be divided by four due to the planar structure, and bit-shifting by two to the right is quicker

    clip_x = ((end_x - start_x) >> 2) + 1;
    clip_y = end_y - start_y + 1;

    start_x = start_x >> 2;
    sprite_w = sprite_w >> 2;
    x_offset = x_offset >> 2;

    for (plane = 0; plane < 4; plane++)
    {
        for (index_y = 0; index_y < clip_y; index_y++)
        {
            _fmemcpy(&ModeX_buf[plane][(start_y + index_y) * 80 + start_x], &texture->pixels[plane][(index_y + y_offset) * sprite_w + x_offset], clip_x);
        }
    }
}

void drawTexturePartial(int x, int y, Texture_t* texture)
{
    int start_x;
    int start_y;
    int end_x;
    int end_y;
    int index_x;
    int index_y;
    int clip_x;
    int clip_y;
    int x_offset = 0;
    int y_offset = 0;

    int sprite_w = texture->width;
    int sprite_h = texture->height;

    x -= texture->offset_x;
    y -= texture->offset_y;

    start_x = x;
    start_y = y;

    end_x = start_x + sprite_w - 1;
    end_y = start_y + sprite_h - 1;

    if (start_x >= SCREEN_WIDTH || start_y >= System.screen_height || start_x <= -sprite_w || start_y <= -sprite_h)
        return;
    
    if (start_x < 0)
    {   
        x_offset = abs(start_x);
        start_x = 0;
    }
    
    else if (end_x >= SCREEN_WIDTH)
        end_x = SCREEN_WIDTH - 1;

    if (start_y < 0)
    {
        y_offset = abs(start_y);
        start_y = 0;
    }
    
    else if (end_y >= System.screen_height)
        end_y = System.screen_height - 1;

    clip_x = end_x - start_x + 1;
    clip_y = end_y - start_y + 1;

    if ((texture->flags & TEXTURE_FLAGS_TRANSPARENCY))
    {
        for (index_y = 0; index_y < clip_y; index_y++)
        {
            for (index_x = 0; index_x < clip_x; index_x++)
            {
                if (texture->pixels[(index_y + y_offset) * texture->width + (x_offset + index_x)] != TRANSPARENT_COLOR)
                    SET_PIXEL(start_x + index_x, start_y + index_y, texture->pixels[(index_y + y_offset) * texture->width + (x_offset + index_x)]);
            }
        }
    }
    else
    {
        for (index_y = 0; index_y < clip_y; index_y++)
        {
            _fmemcpy(&screen_buf[(start_y + index_y) * SCREEN_WIDTH + start_x], &texture->pixels[(index_y + y_offset) * texture->width + x_offset], clip_x);
        }
    }
}

float rotateShearX(Vec2 source, double angle)
{
    float new_loc;

    new_loc = source.x - tan(angle / 2) * source.y;

    new_loc = (int)new_loc;

    return new_loc;
}

float rotateShearY(Vec2 source, double angle)
{
    float new_loc;

    new_loc = source.y + sin(angle) * source.x;

    new_loc = (int)new_loc;

    return new_loc;
}

void drawTextureRotated(int x, int y, double angle, Texture_t* source, uint8_t bgcolor)
{
    Texture_t rotated;
    Vec2 sheared;
    float width_scale = 1.0;
    float height_scale = 1.0;
    int w;
    int h;
    int w_half;
    int h_half;
    int i = 0;
    float rot_i;
    int rotated_size;
    uint8_t mirror_flip = FALSE;

    if (angle > RAD_270)
        angle -= RAD_360;

    else if (angle < -RAD_270)
        angle += RAD_360;

    if (angle >= RAD_90)
    {
        angle -= RAD_180;
        mirror_flip = TRUE;
    }

    else if (angle <= -RAD_90)
    {
        angle += RAD_180;
        mirror_flip = TRUE;
    }

    if (source->width != source->height)
    {
        if (source->width < source->height)
            width_scale = source->height / source->width;

        else if (source->height < source->width)
            height_scale = source->width / source->height;

        rotated.width = abs(source->height * sin(angle)) + abs(source->width * cos(angle)) + (7 * height_scale);
        rotated.height = abs(source->width * sin(angle)) + abs(source->height * cos(angle)) + (7 * width_scale);
    }

    else
        rotated.width = abs(source->height * sin(angle)) + abs(source->width * cos(angle)) + 5;
        rotated.height = abs(source->width * sin(angle)) + abs(source->height * cos(angle)) + 5;

    if ((source->flags & TEXTURE_FLAGS_TRANSPARENCY))
        rotated.flags |= (TEXTURE_FLAGS_TRANSPARENCY);

    rotated.offset_x = (rotated.width - source->width) / 2;
    rotated.offset_y = (rotated.height - source->height) / 2;

    w_half = source->width / 2;
    h_half = source->height / 2;

    rotated_size = rotated.width * rotated.height;
    rotated.pixels = malloc(rotated_size);
    _fmemset(rotated.pixels, bgcolor, rotated_size);

    if (mirror_flip == TRUE)
    {
        for (h = -h_half; h < h_half; h++)
        {
            for (w = -w_half; w < w_half; w++)
            {
                sheared.x = w;
                sheared.y = h;
                sheared.x = rotateShearX(sheared, angle);
                sheared.y = rotateShearY(sheared, angle);
                sheared.x = rotateShearX(sheared, angle);
                rot_i = ((rotated.height - ((int)sheared.y + rotated.offset_y + h_half))) * rotated.width + (rotated.width - (sheared.x + rotated.offset_x + w_half));
                rotated.pixels[(int)rot_i] = source->pixels[i];
                i++;
            }
        }
    }
    else
    {
        for (h = -h_half; h < h_half; h++)
        {
            for (w = -w_half; w < w_half; w++)
            {
                sheared.x = w;
                sheared.y = h;
                sheared.x = rotateShearX(sheared, angle);
                sheared.y = rotateShearY(sheared, angle);
                sheared.x = rotateShearX(sheared, angle);
                rot_i = ((int)sheared.y + rotated.offset_y + h_half) * rotated.width + (sheared.x + rotated.offset_x + w_half);
                rotated.pixels[(int)rot_i] = source->pixels[i];
                i++;
            }
        }
    }
    drawTexturePartial(x, y, &rotated);

    free(rotated.pixels);
}

int calculateRotatedTextureSize(double angle, Texture_t* source)
{
    //precalculates the size of a rotated texture for memory allocation

    int width;
    int height;
    int rotated_size;

    if (angle > RAD_270)
        angle -= RAD_360;

    else if (angle < -RAD_270)
        angle += RAD_360;

    if (angle >= RAD_90)
    {
        angle -= RAD_180;
    }

    else if (angle <= -RAD_90)
    {
        angle += RAD_180;
    }

    width = abs(source->height * sin(angle)) + abs(source->width * cos(angle)) + 5;
    height = abs(source->width * sin(angle)) + abs(source->height * cos(angle)) + 5;

    rotated_size = width * height;

    return rotated_size;
}

RotatedTexture_t saveRotatedTexture(double angle, Texture_t* source, uint8_t bgcolor)
{
    RotatedTexture_t rotated;
    Vec2 sheared;
    float width_scale = 1.0;
    float height_scale = 1.0;
    int w;
    int h;
    int w_half;
    int h_half;
    int i = 0;
    float rot_i;
    int rotated_size;
    uint8_t mirror_flip = FALSE;

    if (angle > RAD_270)
        angle -= RAD_360;

    else if (angle < -RAD_270)
        angle += RAD_360;

    if (angle >= RAD_90)
    {
        angle -= RAD_180;
        mirror_flip = TRUE;
    }

    else if (angle <= -RAD_90)
    {
        angle += RAD_180;
        mirror_flip = TRUE;
    }

    if (source->width != source->height)
    {
        if (source->width < source->height)
            width_scale = source->height / source->width;

        else if (source->height < source->width)
            height_scale = source->width / source->height;

        rotated.width = abs(source->height * sin(angle)) + abs(source->width * cos(angle)) + (7 * height_scale);
        rotated.height = abs(source->width * sin(angle)) + abs(source->height * cos(angle)) + (7 * width_scale);
    }

    else
        rotated.width = abs(source->height * sin(angle)) + abs(source->width * cos(angle)) + 5;
        rotated.height = abs(source->width * sin(angle)) + abs(source->height * cos(angle)) + 5;

    if ((source->flags & TEXTURE_FLAGS_TRANSPARENCY))
        rotated.flags |= (TEXTURE_FLAGS_TRANSPARENCY);

    rotated.offset_x = (rotated.width - source->width) / 2;
    rotated.offset_y = (rotated.height - source->height) / 2;

    w_half = source->width / 2;
    h_half = source->height / 2;

    rotated_size = rotated.width * rotated.height;
    rotated.pixels = malloc(rotated_size);
    _fmemset(rotated.pixels, bgcolor, rotated_size);

    if (mirror_flip == TRUE)
    {
        for (h = -h_half; h < h_half; h++)
        {
            for (w = -w_half; w < w_half; w++)
            {
                sheared.x = w;
                sheared.y = h;
                sheared.x = rotateShearX(sheared, angle);
                sheared.y = rotateShearY(sheared, angle);
                sheared.x = rotateShearX(sheared, angle);
                rot_i = ((rotated.height - ((int)sheared.y + rotated.offset_y + h_half))) * rotated.width + (rotated.width - (sheared.x + rotated.offset_x + w_half));
                rotated.pixels[(int)rot_i] = source->pixels[i];
                i++;
            }
        }
    }
    else
    {
        for (h = -h_half; h < h_half; h++)
        {
            for (w = -w_half; w < w_half; w++)
            {
                sheared.x = w;
                sheared.y = h;
                sheared.x = rotateShearX(sheared, angle);
                sheared.y = rotateShearY(sheared, angle);
                sheared.x = rotateShearX(sheared, angle);
                rot_i = ((int)sheared.y + rotated.offset_y + h_half) * rotated.width + (sheared.x + rotated.offset_x + w_half);
                rotated.pixels[(int)rot_i] = source->pixels[i];
                i++;
            }
        }
    }

    return rotated;
}

void drawPrerotatedTexture(int x, int y, RotatedTexture_t* texture)
{
    int start_x;
    int start_y;
    int end_x;
    int end_y;
    int index_x;
    int index_y;
    int clip_x;
    int clip_y;
    int x_offset = 0;
    int y_offset = 0;

    int sprite_w = texture->width;
    int sprite_h = texture->height;

    x -= texture->offset_x;
    y -= texture->offset_y;

    start_x = x;
    start_y = y;

    end_x = start_x + sprite_w - 1;
    end_y = start_y + sprite_h - 1;

    if (start_x >= SCREEN_WIDTH || start_y >= System.screen_height || start_x <= -sprite_w || start_y <= -sprite_h)
        return;
    
    if (start_x < 0)
    {   
        x_offset = abs(start_x);
        start_x = 0;
    }
    
    else if (end_x >= SCREEN_WIDTH)
        end_x = SCREEN_WIDTH - 1;

    if (start_y < 0)
    {
        y_offset = abs(start_y);
        start_y = 0;
    }
    
    else if (end_y >= System.screen_height)
        end_y = System.screen_height - 1;

    clip_x = end_x - start_x + 1;
    clip_y = end_y - start_y + 1;

    if ((texture->flags & TEXTURE_FLAGS_TRANSPARENCY))
    {
        for (index_y = 0; index_y < clip_y; index_y++)
        {
            for (index_x = 0; index_x < clip_x; index_x++)
            {
                if (texture->pixels[(index_y + y_offset) * texture->width + (x_offset + index_x)] != TRANSPARENT_COLOR)
                    SET_PIXEL(start_x + index_x, start_y + index_y, texture->pixels[(index_y + y_offset) * texture->width + (x_offset + index_x)]);
            }
        }
    }
    else
    {
        for (index_y = 0; index_y < clip_y; index_y++)
        {
            _fmemcpy(&screen_buf[(start_y + index_y) * SCREEN_WIDTH + start_x], &texture->pixels[(index_y + y_offset) * texture->width + x_offset], clip_x);
        }
    }
}

void drawTextureFromCache(int x, int y, double angle, AnimFrame_t* source)
{
    // this is ugly af and needs to be redone - 14th Dec 2023
    // now it's redone and supports an arbitrary number of rotations - 18th March 2024

    uint8_t angle_index = 0;
    double test_angle = (ROTATION_ANGLE_STEP / 2.00); // angle that is tested against the angle of the object to determine correct rotation index

    if (angle >= RAD_360 - (ROTATION_ANGLE_STEP / 2.00) || angle < (ROTATION_ANGLE_STEP / 2.00))
        angle_index = 0;
    else
    {
        while (test_angle < RAD_360 - (ROTATION_ANGLE_STEP / 2.00))
        {
            if (test_angle < angle)
            {
                test_angle += ROTATION_ANGLE_STEP;
                angle_index++;
            }
            else
                break;
        }
    }

    #if DEBUG == 1
    if (System.debug_mode == TRUE)
    {
        char rotate_debug[5];
        sprintf(rotate_debug, "I:%d", angle_index);
        drawTextClipped(x + 12, y - 10, rotate_debug, COLOUR_WHITE);
    }
    #endif

    drawPrerotatedTexture(x, y, &source->rotations[angle_index]);
}

void drawOctants(int center_x, int offset_x, int center_y, int offset_y, uint8_t color)
{
    SET_PIXEL(center_x + offset_x, center_y + offset_y, color); // lower right octant
    SET_PIXEL(center_x - offset_x, center_y + offset_y, color); // lower left octant
    SET_PIXEL(center_x + offset_x, center_y - offset_y, color); // upper right octant
    SET_PIXEL(center_x - offset_x, center_y - offset_y, color); // upper left octant
    SET_PIXEL(center_x + offset_y, center_y + offset_x, color); // bottom right octant
    SET_PIXEL(center_x - offset_y, center_y + offset_x, color); // bottom left octant
    SET_PIXEL(center_x + offset_y, center_y - offset_x, color); // top right octant
    SET_PIXEL(center_x - offset_y, center_y - offset_x, color); // top left octant   
}

void drawCircle(Vec2* position, int radius, uint8_t color)
{
    int offset_x;
    int offset_y;
    
    int center_x = position->x - (int)camera_offset.x;
    int center_y = position->y - (int)camera_offset.y;

    offset_y = 0;

    if (boundaryCheckRadius(center_x, center_y, radius == FULLY_IN))
    {
        do
        {
            offset_x = sqrt((float)radius*radius - (float)offset_y*offset_y);
            drawOctants(center_x, offset_x, center_y, offset_y, color);
            offset_y++;
        }
        while (offset_y < offset_x);
    }
}

void drawSquareColor(int x, int y, uint8_t colour)
{
    int index_y = 0;

    for (index_y = 0; index_y < SQUARE_SIZE; index_y++)
    {
        //_fmemsetting the entire row is faster than drawing it pixel by pixel
        _fmemset(screen_buf + (y * SCREEN_WIDTH + x), colour, SQUARE_SIZE);
        y++;
    }
}

void drawRectangle(int x, int y, int w, int h, uint8_t color)
{
    int index_x = 0;
    int index_y = 0;

    for (index_y = 0; index_y < h;index_y++)
    {
        for (index_x = 0; index_x < w;index_x++)
        {
            SET_PIXEL(x, y, color);
            x++;
        }
        index_x = 0;
        x -= w;
        y++;
    }
    index_y = 0;
}

void drawRectangleVGA(int x, int y, int w, int h, uint8_t color)
{
    int index_x = 0;
    int index_y = 0;

    for (index_y = 0; index_y < h;index_y++)
    {
        for (index_x = 0; index_x < w;index_x++)
        {
            SET_PIXEL_VGA(x, y, color);
            x++;
        }
        index_x = 0;
        x -= w;
        y++;
    }
    index_y = 0;
}

void drawMap()
{
    int xi = (int)camera_offset.x / SQUARE_SIZE; // first column in the array to be drawn
    //int yi = (int)camera_offset.y / SQUARE_SIZE; // first row in the array to be drawn
    int i = xi; // square drawing "index" from array
    int start_index = i; // first index of the array that is drawn on screen
    int x_pixel; // x-coordinate of the currently drawn pixel
    int y_pixel; // y-coordinate of the currently drawn pixel
    int num_rows; // number of "rows" traversed in the array
    int num_cols; // number of "columns" traversed in the array
    int max_cols = Game.Map.width - xi - 1; // max columns to draw
    int max_rows = Game.Map.height; // max rows to draw
    int a; // array index for Interactives array

    // run loops until maximum number of squares is drawn and the edges of the screen have been reached
    for (y_pixel = 0 - camera_offset.y, num_rows = 0; y_pixel < System.screen_height && num_rows < max_rows; y_pixel += SQUARE_SIZE)
    {
        if (camera_offset.x > 0)
        {
            for (x_pixel = 0 - (camera_offset.x - xi * SQUARE_SIZE), num_cols = 0; x_pixel < SCREEN_WIDTH && num_cols <= max_cols; x_pixel += SQUARE_SIZE, num_cols++)
            {
                if (System.screen_height == SCREEN_HEIGHT_13H)
                    drawTextureClipped(x_pixel, y_pixel, &TileTextures.textures[Game.Map.tilemap[i].texture_id]);
                else
                    drawPlanarTextureClipped(x_pixel, y_pixel, &PlanarTileTextures.textures[Game.Map.tilemap[i].texture_id]);
                /*for (a = 0; a < Game.item_count; a++)
                {
                    if (Items[a].state == 1)
                    {                    
                        if (i == Items[a].index && Items[a].type == ITEM_KEY_RED)
                            drawTexturePartial(x_pixel, y_pixel, &ObjectTextures.textures[TEX_KEY]);
                    }
                }*/
                i++;
            }
        }
        else
        {
            for (x_pixel = 0 - (camera_offset.x - start_index * SQUARE_SIZE); x_pixel < SCREEN_WIDTH; x_pixel += SQUARE_SIZE)
            {
                // eliminate unnecessary drawing on the left of the screen
                if (x_pixel >= abs(xi) * SQUARE_SIZE)
                {
                    if (System.screen_height == SCREEN_HEIGHT_13H)
                        drawTextureClipped(x_pixel, y_pixel, &TileTextures.textures[Game.Map.tilemap[i].texture_id]);
                    else
                        drawPlanarTextureClipped(x_pixel, y_pixel, &PlanarTileTextures.textures[Game.Map.tilemap[i].texture_id]);
                    /*for (a = 0; a < Game.item_count; a++)
                    {
                        if (Items[a].state == 1)
                        {                    
                            if (i == Items[a].index && Items[a].type == ITEM_KEY_RED)
                                drawTexturePartial(x_pixel, y_pixel, &ObjectTextures.textures[TEX_KEY]);
                        }
                    }*/
                }
                i++;
            }
        }
        num_rows++;
        i = start_index + (Game.Map.width * num_rows); // jump in the texture array to the next "row"
    }
}

void testColors()
{
    int i, x;
    int y = 100;

    for (i = 0, x = 31; i < 256; i++, x++)
        SET_PIXEL(x, y, i);
}

void calcCameraOffset()
{
    Vec2 pos;
    float angle;

    int cam_min_x = SCREEN_WIDTH/2;
    int cam_max_x = Game.Map.width*SQUARE_SIZE - SCREEN_WIDTH/2;
    int cam_min_y = System.screen_height/2;
    int cam_max_y = Game.Map.height*SQUARE_SIZE - System.screen_height/2;

    angle = atan2(PLAYER_ACTOR.direction.y, PLAYER_ACTOR.direction.x);
    pos.x = PLAYER_ACTOR.position.x + cos(angle) * LOOK_DISTANCE;
    pos.y = PLAYER_ACTOR.position.y + sin(angle) * LOOK_DISTANCE;

    if (pos.x < cam_min_x)
        pos.x = cam_min_x;
    else if (pos.x > cam_max_x)
        pos.x = cam_max_x;

    if (pos.y < cam_min_y)
        pos.y = cam_min_y;
    else if (pos.y > cam_max_y)
        pos.y = cam_max_y;

    camera_offset.x = pos.x - (SCREEN_WIDTH / 2);// - SQUARE_SIZE / 2;
    camera_offset.y = pos.y - (System.screen_height / 2);// - SQUARE_SIZE / 2;
}

void drawDot(Actor_t* actor)
{
    int offset_y;
    int offset_x;
    int pos_x;
    int pos_y;
    float dot_radians;
    
    // calculate angle
    dot_radians = atan2(actor->direction.y, actor->direction.x);
    
    // directional dot's offsets from the center of the actor
    offset_y = sin(dot_radians) * DOT_DISTANCE - (int)camera_offset.y;
    offset_x = cos(dot_radians) * DOT_DISTANCE - (int)camera_offset.x;
    pos_x = actor->position.x + offset_x;
    pos_y = actor->position.y + offset_y;

    if (boundaryCheck_X(pos_x) == TRUE && boundaryCheck_Y(pos_y) == TRUE)
    {
        SET_PIXEL(pos_x, pos_y, COLOUR_WHITE);
    }
}

void increaseParticleRead()
{
    particle_read++;

    if (particle_read == MAX_PARTICLES)
        particle_read = 0;
}

void increaseParticleWrite()
{
    particle_write++;

    if (particle_write == MAX_PARTICLES)
        particle_write = 0;

    if (particle_write == particle_read)
        increaseParticleRead();
}

void decrementParticleWrite()
{
    particle_write--;

    if (particle_write < 0)
        particle_write = MAX_PARTICLES - 1;
}

void spawnParticle(Vec2 pos, Vec2 vel, uint8_t color, int8_t life)
{
    Particle_t* particle = &Particles[particle_write];

    particle->pos.x = pos.x;
    particle->pos.y = pos.y;
    particle->vel.x = vel.x;
    particle->vel.y = vel.y;
    particle->color = color;
    particle->life = life;

    increaseParticleWrite();
}

void sprayParticles(Vec2 pos, Vec2 dir, float speed, float randomness, uint8_t num_particles, uint8_t start_color, uint8_t color_variance, int8_t life)
{
    Vec2 random_vec, vel;
    float random_scale;
    uint8_t color;
    int8_t random_life;
    int i;

    for (i = 0; i < num_particles; i++)
    {
        random_vec = getDirVec2(degToRad(rand() % 360));
        random_scale = (float)(rand() % 100) / 100.0;
        random_vec.x *= speed * random_scale * randomness;
        random_vec.y *= speed * random_scale * randomness;

        vel.x = dir.x * speed + random_vec.x;
        vel.y = dir.y * speed + random_vec.y;
        color = start_color + rand() % color_variance;
        random_life = (life / 2) + life * random_scale;
        spawnParticle(pos, vel, color, random_life);
    }
}

void particleFx(Vec2 pos, Vec2 dir, uint8_t fx_type)
{
    uint8_t start_color, color_variance, num_particles;
    int8_t life;
    float speed, randomness;

    switch (fx_type)
    {
    case FX_BLOOD: start_color = 176, color_variance = 15, num_particles = 20, 
    randomness = 0.8, speed = 1, life = 20; break;
    case FX_SPARKS: start_color = 160, color_variance = 7, num_particles = 15, 
    randomness = 0.5, speed = -5, life = 10; break;
    case FX_DIRT: start_color = 64, color_variance = 15, num_particles = 8, 
    randomness = 0.3, speed = 3, life = 10; break;
    case FX_WATERGUN: start_color = 196, color_variance = 5, num_particles = 15, 
    randomness = 0.1, speed = 10, life = 10; break;
    
    default: break;
    }
    if (boundaryCheck(((int)(pos.x - camera_offset.x)), ((int)(pos.y - camera_offset.y))) == TRUE)
        sprayParticles(pos, dir, speed, randomness, num_particles, start_color, color_variance, life);
}

void deleteParticle(int index)
{
    int last_particle = (particle_write == 0) ? MAX_PARTICLES - 1 : particle_write - 1;

    if (last_particle != particle_read)
        Particles[index] = Particles[last_particle];

    decrementParticleWrite();
}

void drawParticle(Vec2* pos, Vec2 vel, uint8_t color)
{
    pos->x += vel.x * System.ticks_per_frame;
    pos->y += vel.y * System.ticks_per_frame;
    if (boundaryCheck(((int)(pos->x - camera_offset.x)), ((int)(pos->y - camera_offset.y))) == TRUE)
        SET_PIXEL(((int)(pos->x - camera_offset.x)), ((int)(pos->y - camera_offset.y)), color);
}

void particleArrayManager()
{
    int i = particle_read;

    while (i != particle_write)
    {
        Particle_t* particle = &Particles[i];
        particle->life -= System.ticks_per_frame;
        if (particle->life <= 0)
        {
            deleteParticle(i);
            continue;
        }    
        drawParticle(&particle->pos, particle->vel, particle->color);
        i++;
        if (i == MAX_PARTICLES)
        {
            i = 0;
        }
    }
}

void increaseCorpseRead()
{
    corpse_read++;

    if (corpse_read == MAX_CORPSES)
        corpse_read = 0;
}

void increaseCorpseWrite()
{
    corpse_write++;

    if (corpse_write == MAX_CORPSES)
        corpse_write = 0;

    if (corpse_write == corpse_read)
        increaseCorpseRead();
}

void decrementCorpseWrite()
{
    corpse_write--;

    if (corpse_write < 0)
        corpse_write = MAX_CORPSES - 1;
}

void spawnCorpse(Vec2 pos, double angle, int8_t life)
{
    Corpse_t* corpse = &Corpses[corpse_write];

    corpse->pos.x = pos.x;
    corpse->pos.y = pos.y;
    corpse->life = life;
    corpse->sprite = saveRotatedTexture(angle, &ObjectTextures.textures[TEX_CORPSE], TRANSPARENT_COLOR);

    increaseCorpseWrite();
}

void deleteCorpse(int index)
{
    int last_corpse = (corpse_write == 0) ? MAX_CORPSES - 1 : corpse_write - 1;

    if (last_corpse != corpse_read)
        Corpses[index] = Corpses[last_corpse];

    decrementCorpseWrite();
}

void corpseArrayManager()
{
    int i = corpse_read;

    while (i != corpse_write)
    {
        if (Corpses[i].life != -1)
        {
            Corpses[i].life -= System.ticks_per_frame;
            if (Corpses[i].life <= 0)
            {
                deleteCorpse(i);
                continue;
            }
        }
        if (boundaryCheckRadius((int)Corpses[i].pos.x - camera_offset.x, (int)Corpses[i].pos.y - camera_offset.y, CORPSE_DIAMETER) != FULLY_OUT)
            drawPrerotatedTexture(Corpses[i].pos.x - camera_offset.x, Corpses[i].pos.y - camera_offset.y, &Corpses[i].sprite);
        i++;
        if (i == MAX_CORPSES)
        {
            i = 0;
        }
    }
}

void emptyCorpseArray()
{
    _fmemset(Corpses, 0, sizeof(Corpse_t) * MAX_CORPSES);
    corpse_read = 0;
    corpse_write = 0;
}

void increaseTempSpriteRead()
{
    tempsprite_read++;

    if (tempsprite_read == MAX_TEMPSPRITES)
        tempsprite_read = 0;
}

void increaseTempSpriteWrite()
{
    tempsprite_write++;

    if (tempsprite_write == MAX_TEMPSPRITES)
        tempsprite_write = 0;

    if (tempsprite_write == tempsprite_read)
        increaseTempSpriteRead();
}

void decrementTempSpriteWrite()
{
    tempsprite_write--;

    if (tempsprite_write < 0)
        tempsprite_write = MAX_TEMPSPRITES - 1;
}

int spawnTempSprite(uint8_t draw_type, uint8_t move_type, Vec2 pos, Vec2 vel, double angle, Sprite_t* sprite)
{
    TempSprite_t* tempspr = &TempSprites[tempsprite_write];
    int tempsprite_id = tempsprite_write;

    tempspr->draw_type = draw_type;
    tempspr->move_type = move_type;
    tempspr->status = ACTIVE;
    tempspr->pos.x = pos.x;
    tempspr->pos.y = pos.y;
    tempspr->vel.x = vel.x;
    tempspr->vel.y = vel.y;
    tempspr->angle = angle;
    tempspr->sprite = sprite;

    increaseTempSpriteWrite();
    return tempsprite_id;
}

void updateTempSprite(int index, Vec2 pos, Vec2 vel, double angle)
{
    TempSprite_t* tempspr = &TempSprites[index];

    tempspr->pos.x = pos.x;
    tempspr->pos.y = pos.y;
    tempspr->vel.x = vel.x;
    tempspr->vel.y = vel.y;
    tempspr->angle = angle;
}

void deleteTempSprite(int index)
{
    TempSprites[index].status = INACTIVE;
}

void tempSpriteArrayManager()
{
    int i = tempsprite_read;

    while (i != tempsprite_write)
    {
        TempSprite_t* tempsprite = &TempSprites[i];
        if (tempsprite->status != INACTIVE)
        {
            Texture_t* texture = &ObjectTextures.textures[Animations.anims[tempsprite->sprite->anim_id].frames[tempsprite->frame].frame_id];

            if (tempsprite->move_type == STATIC_SPRITE)
            {
                if (tempsprite->angle != 0.0)
                    drawTextureRotated(tempsprite->pos.x - camera_offset.x - texture->width/2, tempsprite->pos.y - camera_offset.y - texture->height/2,
                    tempsprite->angle, texture, TRANSPARENT_COLOR);
                else
                    drawTexture(tempsprite->pos.x - camera_offset.x - texture->width/2, tempsprite->pos.y - camera_offset.y- texture->height/2, texture);
            }

            else if (tempsprite->move_type == MOVING_SPRITE)
            {
                tempsprite->pos.x += tempsprite->vel.x * System.ticks_per_frame;
                tempsprite->pos.y += tempsprite->vel.y * System.ticks_per_frame;
                drawTextureFromCache(tempsprite->pos.x - camera_offset.x - texture->width/2, tempsprite->pos.y - camera_offset.y - texture->height/2,
                tempsprite->angle, &Animations.anims[tempsprite->sprite->anim_id].frames[tempsprite->frame]);
            }

            tempsprite->frame++;
            if (tempsprite->frame == Animations.anims[tempsprite->sprite->anim_id].num_frames)
            {
                if (tempsprite->draw_type == RUN_ONCE)
                {
                    tempsprite->frame = 0;
                    deleteTempSprite(i);
                    continue;
                }
                else
                {
                    tempsprite->frame = 0;
                }
            }
        }
        i++;
        if (i == MAX_TEMPSPRITES)
        {
            i = 0;
        }
    }
}

void emptyTempSpriteArray()
{
    _fmemset(TempSprites, 0, sizeof(TempSprite_t) * MAX_TEMPSPRITES);
    tempsprite_read = 0;
    tempsprite_write = 0;
}

// test function, delete/flesh out later
void drawAnim()
{
    static int frame_counter = 0;
    static int animation_counter = 0;
    static ticks_t last_frame_drawn;

    Texture_t* texture = &ObjectTextures.textures[Animations.anims[animation_counter].frames[frame_counter].frame_id];

    drawTexture(110, 100, texture);

    if (last_frame_drawn + 10 < System.ticks)
    {
        last_frame_drawn = System.ticks;
        frame_counter++;
        if (frame_counter >= Animations.anims[animation_counter].num_frames)
        {
            frame_counter = 0;
            animation_counter++;
            if (animation_counter >= Animations.anim_count)
                animation_counter = 1;
        }
    }
}

void testDrawAnimFromName(int x, int y, char* name)
{
    static int frame_counter = 0;
    static ticks_t last_frame_drawn;
    int i;

    if ((i = findAnim(name)) != 0)
    {
        Texture_t* texture = &ObjectTextures.textures[Animations.anims[i].frames[frame_counter].frame_id];

        drawTexture(x, y, texture);

        if (last_frame_drawn + 5 < System.ticks)
        {
            last_frame_drawn = System.ticks;
            frame_counter++;
            if (frame_counter >= Animations.anims[i].num_frames)
            {
                frame_counter = 0;
            }
        }
    }
    else
        drawTextureClipped(x, y, &ObjectTextures.textures[Animations.anims[TEX_ERROR].frames[TEX_ERROR].frame_id]);
}

void drawAnimFromSprite(int x, int y, double angle, Sprite_t* sprite)
{
    Texture_t* texture;

    if (sprite->last_frame + sprite->frame_interval < System.ticks)
    {
        sprite->last_frame = System.ticks;
        sprite->frame++;
        if (sprite->frame >= Animations.anims[sprite->anim_id].num_frames)
        {
            sprite->frame = 0;
        }
    }

    texture = &ObjectTextures.textures[Animations.anims[sprite->anim_id].frames[sprite->frame].frame_id];

    if (angle != 0.0)
        drawTextureFromCache(x, y, angle, &Animations.anims[sprite->anim_id].frames[sprite->frame]);
    else
        drawTexturePartial(x, y, texture);
}

void drawSprite(int x, int y, double angle, Sprite_t* sprite)
{
    if (sprite->flags == SPRITE_IS_STATIC)
    {
        Texture_t* texture = &ObjectTextures.textures[Animations.anims[sprite->anim_id].frames[0].frame_id];

        if (angle != 0.0)
            drawTextureFromCache(x, y, angle, &Animations.anims[sprite->anim_id].frames[0]);
        else
            drawTexture(x, y, texture);
    }
    else
        drawAnimFromSprite(x, y, angle, sprite);
}

void setActorAnim(Actor_t* actor, int new_anim_type)
{
    actor->sprite.anim_id = actor->animset->anim_ids[new_anim_type];
    actor->sprite.frame = 0;
}

void drawActors()
{
    int i; // actor array "index"
    int start_x;
    int start_y;
    Texture_t* texture;
    Actor_t* actor;
    char str[8] = {0};

    for (i = 0; i < Game.actor_count; i++)
    {
        actor = &Game.Actors[i];
        texture = &ObjectTextures.textures[actor->texture_id];

        start_x = actor->position.x - camera_offset.x - texture->width / 2;
        start_y = actor->position.y - camera_offset.y - texture->height / 2;
        
        if ((i != 0) && boundaryCheckRadius(start_x, start_y, actor->radius * 2) != FULLY_OUT && texture != NULL)
            drawTextureRotated(start_x, start_y, actor->angle, texture, TRANSPARENT_COLOR);
        else if (boundaryCheckRadius(start_x, start_y, actor->radius) != FULLY_OUT)
            drawSprite(start_x, start_y, PLAYER_ACTOR.angle, &PLAYER_ACTOR.sprite);

        drawDot(actor);

        #if DEBUG == 1
        if (System.debug_mode == TRUE)
        {
            str[0] = '\0';
            sprintf(str, "%u", actor->id);
            drawTextClipped(start_x, start_y - 10, str, COLOUR_YELLOW);
        }
        #endif
    }
}

void drawHealth()
{
    char plr_health[10];

    sprintf(plr_health, "HP: %d", PLAYER_ACTOR.health);
    drawText(250, 190, plr_health, COLOUR_WHITE);
}

void drawStats()
{
    char cur_weapon[30];

    sprintf(cur_weapon, "%s", Weapons[PLAYER_ACTOR.primary_weapon_id].name);
    drawText(2, 190, cur_weapon, COLOUR_WHITE);
}

void menuDraw()
{
    _fmemset(screen_buf, 0, SCREEN_SIZE_13H);
    drawMenuText();
}

void titleDraw()
{
    char time [50];
    sprintf(time, "Ticks: %d", System.ticks);
    drawText(93, 96, "PRESS SPACE", COLOUR_BLUE);
    drawText(1, 1, time, COLOUR_BLUE);
}

void testRocketAngle()
{
    static double angle = 0.0;
    drawSprite(10, 80, angle, &Rocket);
    angle += 0.1;
    if (angle >= RAD_360)
        angle = 0;
}

void testSetPlayerAnim()
{
    if (PLAYER_ACTOR.magnitude != 0.0)
    {
        PLAYER_ACTOR.sprite.flags = SPRITE_IS_ANIM;
        if (!KEY_IS_PRESSED(KEY_SPACEBAR) && PLAYER_ACTOR.sprite.anim_id != PLAYER_ACTOR.animset->anim_ids[ANIM_WALK])
            setActorAnim(&PLAYER_ACTOR, ANIM_WALK);
        else if (KEY_IS_PRESSED(KEY_SPACEBAR))
            if (PLAYER_ACTOR.primary_weapon_id == WEAPON_FIST && PLAYER_ACTOR.sprite.anim_id != PLAYER_ACTOR.animset->anim_ids[ANIM_PUNCH_WALK])
                setActorAnim(&PLAYER_ACTOR, ANIM_PUNCH_WALK);
            else if (PLAYER_ACTOR.primary_weapon_id != WEAPON_FIST && PLAYER_ACTOR.sprite.anim_id != PLAYER_ACTOR.animset->anim_ids[ANIM_SHOOT])
                setActorAnim(&PLAYER_ACTOR, ANIM_SHOOT);
    }
    else if (PLAYER_ACTOR.magnitude == 0.0)
    {
        if (!KEY_IS_PRESSED(KEY_SPACEBAR))
        {
            setActorAnim(&PLAYER_ACTOR, ANIM_IDLE);
            PLAYER_ACTOR.sprite.flags = SPRITE_IS_STATIC;
        }
        else
            if (PLAYER_ACTOR.primary_weapon_id == WEAPON_FIST && PLAYER_ACTOR.sprite.anim_id != PLAYER_ACTOR.animset->anim_ids[ANIM_PUNCH_STILL])
                setActorAnim(&PLAYER_ACTOR, ANIM_PUNCH_STILL);
            else if (PLAYER_ACTOR.primary_weapon_id != WEAPON_FIST && PLAYER_ACTOR.sprite.anim_id != PLAYER_ACTOR.animset->anim_ids[ANIM_SHOOT])
                setActorAnim(&PLAYER_ACTOR, ANIM_SHOOT);
    }
}

void animTestBlock()
{
    drawAnim();
    testDrawAnimFromName(60, 80, "ANIMS/ROCKET.ANI");
    testDrawAnimFromName(60, 100, "ANIMS/EXPLO.ANI");
    testDrawAnimFromName(60, 140, "ANIMS/LOL.ANI");
    testRocketAngle();
    drawSprite(10, 120, 0.0, &Explosion);
}

void rocketPrecacheTest()
{
    int i;
    int x = 10;
    int y = 170;
    double angle = 0.0;
    int frame = 0;

    for (i = 0; i < NUM_ROTATIONS; i++)
    {
        drawTextureFromCache(x, y, angle, &Animations.anims[1].frames[frame]);
        angle += RAD_30;
        x += 25;
    }
}

void testFont()
{
    drawText(1, 1, "[\\]^_'abcdefg\nhijklmnopqrst\nuvwxyz", COLOUR_WHITE);
}

void gameDraw()
{
    calcCameraOffset();
    drawMap();
    corpseArrayManager();
    testSetPlayerAnim();
    //drawActors();
    //drawHealth();
    //drawStats();
    //testFont();
    //animTestBlock();
    //rocketPrecacheTest();
    particleArrayManager();
    tempSpriteArrayManager();
    //drawRLEGfx(0, 0, &LargeRLETest);
    //drawRLEGfx(50, 50, &RLETest);
    #if DEBUG == 1
    if (System.debug_mode == TRUE)
    {
        //sprintf(debug[DEBUG_DRAW], "ANIM: %d", PLAYER_ACTOR.sprite.anim_id);
        //sprintf(debug[DEBUG_DRAW], "TS: %f", TempSprites[tempsprite_write - 1].angle);
        drawDebug();
    }
    #endif
}

void pauseDraw()
{
    drawText(132, 96, "PAUSED", COLOUR_WHITE);
}

void ingameMenuDraw()
{
    drawMenuText();
}
