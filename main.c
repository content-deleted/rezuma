#include <gb/gb.h>
#include <gb/cgb.h>
#include "PlayerSpriteSheet.c"
#include "entity.c"
#include "testTileData.c"
#include "testMapLarge.c"

//global def
void init();
void checkInput();
void updateSwitches();
void initSound();
void updateWindow();

UINT16 i,j = 0;

entity Player = {
    0x00,
    {0x20,0x81},
    0x00,
    0U,
    &PlayerStand,
    0U,
    0U,
};

void main() {

	init();

	while(1) {
		updateSwitches();
		checkInput();
        
        updatePosition(&Player);
        // Once we know new player position we can scroll background and handle load new bkg tiles
        updateWindow();
        // Finally draw player taking into account new bkg position
        drawEntity(&Player);

        wait_vbl_done(); // Wait until VBLANK to avoid corrupting visual memory
    }
}

void init() {
    // Turn on the display
	DISPLAY_ON;
	//initSounds();

    // Load all background data into memory
    set_bkg_data(0U, 12U, testTileData);
    // We only set the starting screen background (these should be const)
    set_bkg_tiles(0, 0, 0x64,0x20, testMapLarge);

    // Set the sprite data to 8x16 and load memory 
    SPRITES_8x16;
    set_sprite_data(0U, 4U*7U, PlayerSpriteSheet);

    // Set the player sprite tiles
    set_sprite_tile(0U,0U);
    set_sprite_tile(1U,2U);
}

void initSound() {
    NR52_REG = 0x8F;	// Turn on the sound
	NR51_REG = 0x11;	// Enable the sound channels
	NR50_REG = 0x77;	// Increase the volume to its max
}

void updateSwitches() {
	HIDE_WIN;
	SHOW_SPRITES;
	SHOW_BKG;	
}

void checkInput() {
    updateDirection(&Player);

    if(joypad() & J_UP) Player.direction |= up;
    if(joypad() & J_DOWN) Player.direction |= down;
    if(joypad() & J_LEFT) Player.direction |= left;
    if(joypad() & J_RIGHT) Player.direction |= right;
}

pointLarge bkgPosition = {0U, 0U};
const levelHeight = 40U; // These consts should be handled in a level load once added 
const levelWidth = 100U; 
const SCREENTILEWIDTH = SCREENWIDTH/8;
const SCREENTILEHEIGHT = SCREENHEIGHT/8;
const VRAMWIDTH = 0x20;

UINT16 BKPREVIOUSX = 0U;
UINT8 a;
UINT16 b;
//UINT8 tempTileColumn[SCREENTILEHEIGHT] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

void updateWindow() {
    if( Player.position.x > SCREENWIDTH/2U && (Player.position.x + SCREENWIDTH/2U) < levelWidth * 8 ) 
    {    
        unsigned char tempTileColumn[18];

        BKPREVIOUSX = bkgPosition.x;
        bkgPosition.x = Player.position.x - SCREENWIDTH/2U;
        
        if( BKPREVIOUSX/8U != bkgPosition.x/8U ) 
        {
            a =  (bkgPosition.x / 8U + (( bkgPosition.x < BKPREVIOUSX ) ? 0U : SCREENTILEWIDTH)) % VRAMWIDTH;    
            b =  ( ( bkgPosition.x < BKPREVIOUSX ) ? bkgPosition.x : bkgPosition.x + SCREENWIDTH ) / 8U ;

            // Load tiles into vram
            for(i = 0U; i < 18; i++) 
            {
                tempTileColumn[i] = testMapLarge[100 * i + b];
            }
            set_bkg_tiles(a, 0U, 
            1U, 18, 
            &tempTileColumn);
        }

        move_bkg(bkgPosition.x, bkgPosition.y);
    }
    /* if( bkgPosition.x < BKPREVIOUSX )
    //if( (Player.position.x + SCREENWIDTH/2) / 8 < levelWidth ) 
    {
        set_sprite_tile(3U,2U); 
        move_sprite(3U, 0x50, 0x50);
    }
    else
    {
        set_sprite_tile(3U,2U); 
        move_sprite(3U, 0x00, 0x00);
    } */

    // Debug
    /* if(joypad() & J_SELECT) bkgPosition.x--;
    if(joypad() & J_START) bkgPosition.x++; */
} 