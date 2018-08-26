
/*
 Define data and helper functions related to the enemies here
*/

animationState BadBadFollow = {
    5U, //length one indexed
    2U, //speed
    0x00, //loop
    0x2C //first frame
};

// Test Enemy Entity Declaration
entity Enemy = {
    0x00,
    {0x50,0x50},
    0x00,
    2U, // Sprite starting address in sprite memory
    &BadBadFollow,
    0U,
    0U,
};

void updateFollowerPosition(entity *e);

UBYTE enemyInternalTimer;

void updateFollowerPosition(entity *e) {
    if(enemyInternalTimer % 2 == 0){ 
        if(Player.position.x > e->position.x) e->position.x++; else e->position.x--;
        if(Player.position.y > e->position.y) e->position.y++; else e->position.y--;
    }

    // Put this update routine somewhere else
    enemyInternalTimer++;
}