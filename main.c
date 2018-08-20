#include <gb/gb.h>
//#include <gb/cgb.h>
#include "PlayerSpriteSheet.c"
#include "entity.c"
#include "testTileData.c"
#include "testMapLargeVert.c"

//asm funcs
//void setTile(UINT8 x, UINT8 y, unsigned char *);

//global def
void init();
void checkInput();
void updateSwitches();
void initSound();
void updateWindow();

// Map info
unsigned char * currentMap = &testMapLarge;
//unsigned char currentMapColumns[36U * 100U];
unsigned char collisionMap [16*50];
unsigned char * currentCollisionMap = &collisionMap;

UINT8 levelHeight = 32U; // These consts should be handled in a level load once added 
UINT8 levelWidth = 100U; 
const SCREENTILEWIDTH = SCREENWIDTH/8;
const SCREENTILEHEIGHT = SCREENHEIGHT/8;
const VRAMWIDTH = 0x20;

UINT16 i,j = 0;

entity Player = {
    0x00,
    {0x20,0x70},
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
    // Since we're using a map in column form we need to rotate our tile loading
    for(i = 0U; i <= SCREENTILEWIDTH; i++) set_bkg_tiles(i, 0U, 1U, 18U, currentMap + i * levelHeight); 

    // Set the sprite data to 8x16 and load memory 
    SPRITES_8x16;
    set_sprite_data(0U, 4U*11U, PlayerSpriteSheet);

    // Set the player sprite tiles
    set_sprite_tile(0U,0U);
    set_sprite_tile(1U,2U);

    /* THIS IS FOR LOADING FROM A ROW MAP
    // Load the collision map
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 50; j++) {
            collisionMap[50 * i + j] = currentMap[2*i*100 + j * 2] == 0x0C ? 0x00 : 0x01;
            //collisionMap[50 * i + j] = (i>8) ? 0x01 : 0x00;
        }
    }
    */
   
    // Load the collision map (Columns)
    for (i = 0; i < 16U; i++) { // y pos
        for (j = 0; j < 50U; j++) { // x pos
            collisionMap[50U * i + j] = currentMap[2U * j * 32U + i * 2U] == 0x0C ? 0x00 : 0x01;
            //collisionMap[50 * i + j] = (i>8) ? 0x01 : 0x00;
        }
    }
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

UINT16 BKPREVIOUSX = 0U;
UINT8 screenPosX;
UINT16 tileMapPosX;

// The 18 here is the height of the screen (using a const causes a compiler error)
unsigned char tempTileColumn[18];

void updateWindow() {
    if( Player.position.x > SCREENWIDTH/2U && (Player.position.x + SCREENWIDTH/2U) < ((UINT16) levelWidth) * 8 ) 
    {    
        BKPREVIOUSX = bkgPosition.x;
        bkgPosition.x = Player.position.x - SCREENWIDTH/2U;
        
        if( BKPREVIOUSX/8U != bkgPosition.x/8U ) 
        {
            screenPosX =  (bkgPosition.x / 8U + (( bkgPosition.x < BKPREVIOUSX ) ? 0U : SCREENTILEWIDTH)) % VRAMWIDTH;    
            tileMapPosX =  ( ( bkgPosition.x < BKPREVIOUSX ) ? bkgPosition.x : bkgPosition.x + SCREENWIDTH ) / 8U ;

            // Since the map is already in columns we dont need to do extra math
            // The 18 here is once again the screen size 
            set_bkg_tiles(screenPosX, 0U, 1U, 18U, currentMap + tileMapPosX * levelHeight);
        }

        move_bkg(bkgPosition.x, bkgPosition.y);
    }
} 