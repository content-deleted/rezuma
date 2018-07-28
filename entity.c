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
    UBYTE properties; //0x01 = loop? 0x2 = reached end of animation
    UBYTE address; //first frame in animation
    //UBYTE (*conditions[])();
} animationState;

enum direction {up=0x1, down=0x2, right=0x4, left=0x8};
enum directionPrevious {upPrevious=0x10, downPrevious=0x20, rightPrevious=0x40, leftPrevious=0x80};
enum entityFlags {facing=0x1, airborn=0x2};
// This struct should be based off of everything a player needs but can probably be reused
typedef struct entity {
    UBYTE direction; // Byte representing 8 boolean values. 0-3 are current , 4-7 are previous
    point position;
    // Bit 1 is 0 if facing left else facing right.
    // Bit 2 is 0 if on ground 1 if in air
    UBYTE flags; 

    UBYTE spriteAddress; // Stores the starting position of the current spite
    animationState *currentAnimation;
    // There's probably an easy way to reduce this size
    UINT8 currentFrame; // Stores the number frame (0 indexed)
    UINT8 ticks; // Stores the ticks past since switching to this frame of the animation
} entity;

//Player Animations

animationState PlayerStand = {
    2U, //length 
    32U, //speed
    0x00,
    0x00
};

animationState PlayerRun = {
    3U, //length 
    10U, //speed
    0x00,
    0xC //first frame
};

animationState PlayerJump = {
    1U, //length zero indexed
    3U, //speed
    0x01, //play once
    0x14 //first frame
};

//global define functions
void updatePosition(entity *e);
void updateDirection(entity *e);
void setAnimation(entity *e, animationState *a);
void updateAnimation(entity *e);
INT8 checkTileCollision(UINT8 current, INT8 move);

void updatePosition(entity *e) {
    //check flags and update animations
    if(!(e->flags & airborn)) setAnimation(e, (e->direction & left) || (e->direction & right) ? &PlayerRun : &PlayerStand);

    //update animation 
    updateAnimation(e);
 
    //check directions and update position
    if(e->direction & left) {
        e->position.x--; 
        e->flags &= ~facing;
    }
    if(e->direction & right) {
        e->position.x++; 
        e->flags |= facing;
    }
    
    //check jump 
    //this is temp code and will need to be refactored when collisions are added
    if((joypad() & J_A) && !(e->flags & airborn)) {
        e->flags |= airborn;
        setAnimation(e, &PlayerJump);
    }

    if(e->flags & airborn){
        INT8 inc = checkTileCollision(e->position.y, (e->currentFrame < e->currentAnimation->length) ? -2 : 1);
        if(!inc) e->flags &= ~airborn;
        e->position.y += inc;
    }
    //


    if(e->flags & 1) {
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
    if(e->currentAnimation != a) {
        e->currentAnimation = a;
        e->currentFrame = 0U;
        e->ticks = 0U;
    }
}

void updateAnimation(entity *e) {
    if(e->ticks > e->currentAnimation->speed && e->currentFrame < e->currentAnimation->length ) {
        e->ticks = 0U; 
        e->currentFrame++;
    }

    if(e->currentFrame >= e->currentAnimation->length && !(e->currentAnimation->properties & 0x01)){
        e->currentFrame = 0U;
    }

    // Here the sprite address on entity represents which sprite slot we use in sprite memory
    // The current animation address is where in VRAM the first tile of the first frame is stored
    // Each sprite address is one apart since we use two adjacent positions for each half of the spite
    // We add the current frame x4 to the animation address because each 'sprite' is 4 8x8 tiles
    // The offset of two in the current frame on the second tile is due to the next half being stored 2 tiles away in VRAM (this is very important)
    set_sprite_tile(e->spriteAddress + 0U, e->currentAnimation->address + e->currentFrame * 4U);
    set_sprite_tile(e->spriteAddress + 1U, e->currentAnimation->address + 2U + e->currentFrame * 4U);

    //prevent overflow? (may not be important)
    if(e->ticks < 0xFF) e->ticks++;
}

//needs to be updated when tile collisions are a thing
// This function should return the amount to move the player
// If there is a collision then it's the amount to that collision
// Else it's just the movement
INT8 checkTileCollision(UINT8 current, INT8 move) {
    return (current+move > 0x90 || current+move <= 0x00)
    ? 0 // When we move to tile collision itll look more like: (move % 8) * move
    : move;
}