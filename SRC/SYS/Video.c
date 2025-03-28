#include "SRC/GENERAL/Common.h"
#include "SRC/GENERAL/Def_wat.h"
#include "SRC/SYS/Str_sys.h"

#include "Def_vid.h"
#include "Str_vid.h"
#include "FASTVGA.H"

/* Video mode and palette settings */

uint8_t* game_screen;            /* this points to video memory. */
uint8_t far* screen_buf;        // Double screen buffer for mode 13h
uint8_t far* ModeX_buf[4];      // Double screen buffer for mode X
Palette_t NewPalette;

uint8_t far* ModeX_buf2;
int current_video_mode = MODE_13H; // Default mode

extern System_t System;

int checkForVGA()
{
    // uses the VGA/MCGA specific VIDEO_INT BIOS function 1A to determine the video adapter
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

    game_screen = (uint8_t*)VGA;
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

void setMode13h()
{
    int plane;
    // Only free Mode X buffers if we're coming from Mode X
    if (current_video_mode == MODE_X) {
        for (plane = 0; plane < 4; plane++) {
            if (ModeX_buf[plane] != NULL) {
                free(&ModeX_buf[plane]);
                ModeX_buf[plane] = NULL;
            }
        }
    }
    
    // Set mode 13h
    setVideoMode(MODE_13H);
    
    // Only allocate buffer if not already allocated
    if (screen_buf == NULL) {
        screen_buf = malloc(SCREEN_SIZE_13H);
    }
    _fmemset(screen_buf, 0, SCREEN_SIZE_13H);
    
    System.screen_height = SCREEN_HEIGHT_13H;
    current_video_mode = MODE_13H;

    loadPalette("Pal.bmp", &NewPalette);
    setPalette_VGA(&NewPalette);
}

void setModeX()
{
    int plane;

    // Only free Mode 13h buffer if we're coming from Mode 13h
    if (current_video_mode == MODE_13H && screen_buf != NULL) {
        free(&screen_buf);
    }
    
    outpw(SC_INDEX, 0x0604); // chain 4 off
    outpw(CRTC_INDEX, 0xE317); // word mode off (byte mode)
    outpw(CRTC_INDEX, 0x0014); // long mode off (byte mode)

    outp(MISC_OUTPUT, 0xE3); // vertical sync polarity
    outpw(CRTC_INDEX, 0x2C11); // write protect off
    outpw(CRTC_INDEX, 0x0D06); // vertical total
    outpw(CRTC_INDEX, 0x3E07); // overflow register
    outpw(CRTC_INDEX, 0xEA10); // vertical retrace start
    outpw(CRTC_INDEX, 0xAC11); // vertical retrace end, protect
    outpw(CRTC_INDEX, 0xDF12); // vertical display enable end
    outpw(CRTC_INDEX, 0xE715); // start vertical blanking
    outpw(CRTC_INDEX, 0x0616); // end vertical blanking

    outpw(SC_INDEX, ALL_PLANES);
    memset(game_screen, 0, SCREEN_SIZE_X);

    // Only allocate Mode X buffers if not already allocated
    if (current_video_mode != MODE_X) {
        for (plane = 0; plane < 4; plane++) {
            if (ModeX_buf[plane] == NULL) {
                ModeX_buf[plane] = malloc(PLANE_SIZE_X);
            }
            _fmemset(ModeX_buf[plane], 0, PLANE_SIZE_X);
        }
    }

    System.screen_height = SCREEN_HEIGHT_X;
    current_video_mode = MODE_X;
}

void render13h()
{
    // copy off-screen buffer to VGA memory
    _fmemcpy(game_screen, screen_buf, SCREEN_SIZE_13H);

    // clear off-screen buffer so the screen updates properly
    _fmemset(screen_buf, 0, SCREEN_SIZE_13H);
}

void renderWithoutClear()
{     
    // copy off-screen buffer to VGA memory, don't clear buffer
    _fmemcpy(game_screen, screen_buf, SCREEN_SIZE_13H);
}

void renderModeX()
{
    int plane;

    for (plane = 0; plane < 4; plane++)
    {
        outpw(SC_INDEX, (0x0100 << plane) | 0x02); // select plane
        fast_vga_memcpy(game_screen, ModeX_buf[plane], PLANE_SIZE_X);
        _fmemset(ModeX_buf[plane], 0, PLANE_SIZE_X);
    }
    //memcpy(ModeX_buf2, ModeX_buf, SCREEN_SIZE_X);
    //_fmemset(ModeX_buf, 0, SCREEN_SIZE_X);

    //outpw(SC_INDEX, (0x0100 << 0) | 0x02); // select plane
    //memset(game_screen, 160, PLANE_SIZE_X);
}

void dumpDisplayBuffer()
{
    // debug: save current content of Mode X display buffer to file
    FILE* dump_file;

    dump_file = fopen("BUFFER.DAT", "w");

    fwrite(ModeX_buf2, SCREEN_SIZE_X, 1, dump_file);

    fclose(dump_file);
}
