#include "SRC/GENERAL/Common.h"
#include "SRC/GENERAL/Def_wat.h"

#include "Def_vid.h"
#include "Str_vid.h"

/* Video mode and palette settings */

uint8_t* screen;            /* this points to video memory. */
uint8_t far* screen_buf;    // Double screen buffer
Palette_t NewPalette;

int checkForVGA()
{
    // uses the VGA/MCGA specific VIDEO_INT function 1A to determine the video adapter
    // this function is absent in older video systems, so it should return an error anyway
    // finally, the regs.h.bl check also filters out MCGA (the game resolution is screwed up in MCGA)

    union REGS regs;

    regs.x.eax = 0x1A00;
	int386(VIDEO_INT, &regs, &regs);
    
    if (regs.h.al == 0x1A && regs.h.bl > 6 && regs.h.bl < 9)
        return TRUE;
    else
        return FALSE;
}

void setVideoMode(uint8_t mode)
{
    union REGS regs;

    regs.h.ah = SET_MODE;
    regs.h.al = mode;
    int386(VIDEO_INT, &regs, &regs);

    screen     = (uint8_t*)VGA;
    screen_buf = malloc(SCREEN_SIZE);
}

// loads the palette from a 256-colour bitmap file
void loadPalette(char* filename, Palette_t* pal)
{
    FILE *fp;
    int i;
    
    // Open the file
    fp = fopen(filename, "rb");
    
    // skip forward and read the number of colours
    fseek(fp, 0x0036, SEEK_SET);
    
    // load palette
    for (i = 0; i < 256; i++)
    {
        // uint8_t r,g,b;
        // VGA palette values only go up to 6 bits, (2 ^ 6 == 64 different shades)
        // while bitmap palettes go up to 8 bits, (2 ^ 8 == 256 different shades)
        // right shift (>>) the bits by two places so an 8-bit value becomes a 6-bit
        // this divides the shade value (0-255) by four, giving a value between 0-63
  
        pal->colors[i].b = fgetc(fp);
        pal->colors[i].g = fgetc(fp);
        pal->colors[i].r = fgetc(fp);

        fgetc(fp); // discarded  value
    }

    fclose(fp);
}

void setPalette_VGA(Palette_t* pal)
{
    unsigned i;
    outportb(PALETTE_WRITE, 0);

    for (i = 0; i < 256; i++)
    {
       outportb(PALETTE_DATA, (pal->colors[i].r>>2));
       outportb(PALETTE_DATA, (pal->colors[i].g>>2));
       outportb(PALETTE_DATA, (pal->colors[i].b>>2));
    }
}

void render()
{
    // copy off-screen buffer to VGA memory
    memcpy(screen, screen_buf, SCREEN_SIZE);

    // clear off-screen buffer so the screen updates properly
    _fmemset(screen_buf, 0, SCREEN_SIZE);
}

void renderWithoutClear()
{     
    // copy off-screen buffer to VGA memory, don't clear buffer
    memcpy(screen, screen_buf, SCREEN_SIZE);
}
