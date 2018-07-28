/*
 Define data and helper functions related to entities and player here
*/

typedef struct point {
    UINT8 x;
    UINT8 y;
} point;

// Possibly unused
typedef struct animationState {
    UINT8 length; //amount of frames in animation
    UINT8 speed; //amount of ticks before moving to next frame
    UBYTE properties; //0x1 = loop? 
    UBYTE address; //first frame in animation
    //UBYTE (*conditions[])();
} animationState;

enum direction {up=0x1, down=0x2, right=0x4, left=0x8};
enum directionPrevious {upPrevious=0x10, downPrevious=0x20, rightPrevious=0x40, leftPrevious=0x80};

// This struct should be based off of everything a player needs but can probably be reused
typedef struct entity {
    UBYTE direction; // Byte representing 8 boolean values. 0-3 are current , 4-7 are previous
    point position;
    UBYTE facing; // 0 if facing left else facing right. probably update for other flags later

    UBYTE spriteAddress; // Stores the starting position of the current spite
    animationState * currentAnimation;
    // There's probably an easy way to reduce this size
    UINT8 currentFrame; // Stores the number frame (0 indexed)
    UINT8 ticks; // Stores the ticks past since switching to this frame of the animation
} entity;

//Player Animations

animationState PlayerStand = {
    1U, //length zero indexed
    32U, //speed
    0x00,
    0x00
};

animationState PlayerRun = {
    1U, //length zero indexed
    10U, //speed
    0x00,
    0x0C //first frame
};

//global define functions
void updatePosition(entity *e);
void updateDirection(entity *e);
void setAnimation(entity *e, animationState *a);
void updateAnimation(entity *e);

void updatePosition(entity *e) {
    //check flags and update animations
    if(e->direction & 0x0F) setAnimation(e, &PlayerRun);
    else                    setAnimation(e, &PlayerStand);

    //update animation 
    updateAnimation(e);

    //check directions and update position
    if(e->direction & left) {e->position.x--; e->facing = 0U;}
    if(e->direction & right) {e->position.x++; e->facing = 1U;}

    if(e->facing) {
        set_sprite_prop(e->spriteAddress, S_FLIPX);
        set_sprite_prop(e->spriteAddress + 1U, S_FLIPX);
        move_sprite(e->spriteAddress, e->position.x + 8U, e->position.y);
        move_sprite(e->spriteAddress + 1U, e->position.x, e->position.y);
    }
    else {
        set_sprite_prop(e->spriteAddress, !S_FLIPX);
        set_sprite_prop(e->spriteAddress + 1U, !S_FLIPX);
        move_sprite(e->spriteAddress, e->position.x, e->position.y);
        move_sprite(e->spriteAddress + 1U, ((e->position.x) + 8U), e->position.y);
    }
}

void updateDirection(entity *e) {
    e->direction <<= 4;
}

void setAnimation(entity *e, animationState *a){
    if(e->currentAnimation != a) e->currentAnimation = a;
}



void updateAnimation(entity *e) {
    if(e->ticks > e->currentAnimation->speed) {
        e->ticks = 0U; 
        e->currentFrame++;
    }
    if(e->currentFrame > e->currentAnimation->length) e->currentFrame = 0U;

    // Here the sprite address on entity represents which sprite slot we use in sprite memory
    // The current animation address is where in VRAM the first tile of the first frame is stored
    // Each sprite address is one apart since we use two adjacent positions for each half of the spite
    // We add the current frame x4 to the animation address because each 'sprite' is 4 8x8 tiles
    // The offset of two in the current frame on the second tile is due to the next half being stored 2 tiles away in VRAM (this is very important)
    set_sprite_tile(e->spriteAddress + 0U, e->currentAnimation->address + e->currentFrame * 4U);
    set_sprite_tile(e->spriteAddress + 1U, e->currentAnimation->address + 2U + e->currentFrame * 4U);

    e->ticks++;
}