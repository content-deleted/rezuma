#include <gb/gb.h>
#include "SlimeTest.c"
#include "entity.c"

//global def
void init();
void checkInput();
void updateSwitches();
void initSound();

UINT8 i,j = 0;

entity Player = {
    0x00,
    {0x20,0x20},
    0x00,
    0U,
    &PlayerStand
};

void main() {

	init();

	while(1) {
		updateSwitches();
		checkInput();
        //if(Player.direction & down) Player.position.y++;
        updatePosition(&Player);

        wait_vbl_done(); // Wait until VBLANK to avoid corrupting visual memory
    }
}

void init() {
	DISPLAY_ON;		// Turn on the display
	//initSounds();
    // set_bkg_data(0U, pants_tile_count, pants_tile_data); //load tiles into memory

    // Use the 'backgroundMap' array to write background tiles starting at 32,32 (tile positions)
	//  and write for 2 tiles in width and 1 tiles in height
	//set_bkg_tiles(0, 0,pants_tile_map_width,pants_tile_map_height, pants1);

    //test load sprite into memory
    SPRITES_8x16;
    set_sprite_data(0U, 4U*7U, PlayerSpriteSheet);

    //test set first sprite in memory
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