/*
 Define data and helper functions related to entities and player here
*/

typedef struct point {
    UINT8 x;
    UINT8 y;
} point;

typedef struct pointLarge {
    UINT16 x;
    UINT16 y;
} pointLarge;

typedef struct animationState {
    UINT8 length; //amount of frames in animation
    UINT8 speed; //amount of ticks before moving to next frame
    UBYTE properties; //0x01 = loop? 0x2 = reached end of animation
    UBYTE address; //first frame in animation
    //UBYTE (*conditions[])();
} animationState;

enum direction {up=0x1, down=0x2, right=0x4, left=0x8};
enum directionPrevious {upPrevious=0x10, downPrevious=0x20, rightPrevious=0x40, leftPrevious=0x80};
enum entityFlags {facing=0x1, jumping=0x2, falling=0x4};

// This struct should be based off of everything a player needs but can probably be reused
// It's getting hacky at this point because of poor design decisions
typedef struct entity {
    UBYTE direction; // Byte representing 8 boolean values. 0-3 are current , 4-7 are previous
    pointLarge position;
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
    2U, //length 1 Indexed
    32U, //speed
    0x00,
    0x00
};

animationState PlayerRun = {
    3U, //length 1 Indexed
    10U, //speed
    0x00,
    0xC //first frame
};

animationState PlayerJump = {
    3U, //length zero indexed
    5U, //speed
    0x01, //play once
    0x14 //first frame
};

animationState PlayerFall = {
    2U, //length 1 indexed
    3U, //speed
    0x00, //loop
    0x24 //first frame
};

//global define functions
void updatePosition(entity *e);
void updateDirection(entity *e);
void setAnimation(entity *e, animationState *a);
void updateAnimation(entity *e);
void drawEntity(entity *e);
INT8 checkTileCollisionX(pointLarge *current, INT8 move);
INT8 checkTileCollisionY(pointLarge *current, INT8 move);

void updatePosition(entity *e) {
    //declare local var
    INT8 verticalMovement = 0;
    INT8 horizontalMovement = 0;

    // Check flags and update animations
    if(!(e->flags & jumping) && !(e->flags & falling)) setAnimation(e, (e->direction & left) || (e->direction & right) ? &PlayerRun : &PlayerStand);

    updateAnimation(e);
 
    // Check directions and update position
    if(e->direction & left) {
        horizontalMovement = (joypad() & J_B) ? -2 : -1; 
        e->flags &= ~facing;
    }
    if(e->direction & right) {
        horizontalMovement = (joypad() & J_B) ? 2 : 1; 
        e->flags |= facing;
    }
    
    //check jump 
   
    if((joypad() & J_A) && !(e->flags & jumping) && !(e->flags & falling)) {
        e->flags |= jumping;
        setAnimation(e, &PlayerJump);
    }

    if(e->flags & jumping && !(e->flags & falling) ) {
        INT8 inc; //define local 
        verticalMovement = -3;
        inc = checkTileCollisionY(&e->position, verticalMovement);
        if(!inc || !(joypad() & J_A) || e->currentFrame + 1 >= PlayerJump.length) {
            e->flags &= ~jumping;
            e->flags |= falling;
        }
        else e->position.y -= -inc;
    } 
    else {
        INT8 inc;
        verticalMovement = 2;
        inc = checkTileCollisionY(&e->position, verticalMovement);
        if(inc != 0) {
            e->flags |= falling;
            setAnimation(e, &PlayerFall);
            e->position.y += inc;
        }
        else  e->flags &= ~falling;
    }

    // Tile Collision X direction 
    if(horizontalMovement > 0) e->position.x += checkTileCollisionX(&e->position, horizontalMovement);

    if(horizontalMovement < 0) e->position.x -= -checkTileCollisionX(&e->position, horizontalMovement);
}

extern pointLarge bkgPosition;

void drawEntity(entity *e) {
    if(e->flags & facing) {
        set_sprite_prop(e->spriteAddress, S_FLIPX);
        set_sprite_prop(e->spriteAddress + 1U, S_FLIPX);
        move_sprite(e->spriteAddress, e->position.x + 8U - bkgPosition.x, e->position.y);
        move_sprite(e->spriteAddress + 1U, e->position.x - bkgPosition.x, e->position.y);
    }
    else {
        set_sprite_prop(e->spriteAddress, !S_FLIPX);
        set_sprite_prop(e->spriteAddress + 1U, !S_FLIPX);
        move_sprite(e->spriteAddress, e->position.x - bkgPosition.x, e->position.y);
        move_sprite(e->spriteAddress + 1U, ((e->position.x) + 8U) - bkgPosition.x, e->position.y);
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

    if(e->currentFrame >= e->currentAnimation->length && !(e->currentAnimation->properties & 0x01)) e->currentFrame = 0U;

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

extern unsigned char * currentCollisionMap;
extern UINT8 levelWidth;

BOOLEAN checkTile(UINT16 x, UINT16 y) {
    return (currentCollisionMap[ ( (UINT16) 50U * (y/16U) + (x/16U)) ]); // change this to check with list of solid tiles
}

// This function should return the amount to move the player
// If there is a collision then it's the amount to that collision
// Else it's just the movement
INT8 ret;
INT8 checkTileCollisionX(pointLarge *current, INT8 move) {
    UINT16 locX = current->x;
    UINT16 locY = current->y - 1U;

    if(move > 0) locX += move + 7U; 
    else locX -= -move + 8U;

    ret = (checkTile(locX, locY))
           ? 0
           : move;
    /* This displays a sprite if X colliding
    if( ret == 0 ){
         set_sprite_tile(4U,0U);
         move_sprite(4U, 10, 10);
    }
    else
        move_sprite(4U, 0, 0);
    */
    return ret;
}

INT8 checkTileCollisionY(pointLarge *current, INT8 move) {
    UINT16 locX = current->x;
    UINT16 locY = current->y - 1U;

    if(move > 0) locY += move; 
    else locY -= -move + 7U;

    ret = (checkTile(locX - 7, locY) || checkTile(locX + 7U, locY))
           ? 0//(current->y + move) % 8
           : move;
    
    /* This displays a sprite if Y colliding
    if( ret == 0 ){
         set_sprite_tile(5U,0U);
         move_sprite(5U, 40, 10);
    }
    else 
        move_sprite(5U, 0, 0);
    */
    return ret;
}