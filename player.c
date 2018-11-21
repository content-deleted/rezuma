/*
 Define data and helper functions related to the player entity here
*/

// Player Entity Declaration
entity Player = {
    0x00,
    {0x20,0x70},
    0x00,
    0U,
    &PlayerStand,
    0U,
    0U,
};

// Player Animations

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


void updatePlayerPosition(entity *e);

// This is the main movement code for the player
// It is very simple and bad and lazy

void updatePlayerPosition(entity *e) {
    //declare local var
    INT8 verticalMovement = 0;
    INT8 horizontalMovement = 0;

    // Check flags and update animations
    if(!(e->flags & jumping) && !(e->flags & falling)) setAnimation(e, (e->direction & left) || (e->direction & right) ? &PlayerRun : &PlayerStand);
 
    // Check directions and update position
    if(e->direction & left) {
        horizontalMovement = -1; // ENABLE FOR RUN // (joypad() & J_B) ? -2 : -1; 
        e->flags &= ~facing;
    }
    if(e->direction & right) {
        horizontalMovement = 1; // ENABLE FOR RUN // (joypad() & J_B) ? 2 : 1; 
        e->flags |= facing;
    }
    
    //check jump 
   
    if((joypad() & J_A) && !(e->flags & jumping) && !(e->flags & falling)) {
        e->flags |= jumping;
        setAnimation(e, &PlayerJump);
    }

    if(e->flags & jumping && !(e->flags & falling) ) {
        INT8 inc; //define local 
        verticalMovement = -2;
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