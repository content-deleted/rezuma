    .globl _setTile
    _setTile:     ; setTile(UINT8 x, UINT8 y, unsigned char * tileMap)
                  ; There is no register to save:
                  ;  BC is not used
                  ;  DE is the return register
                  ;  HL needs never to be saved
	LDHL SP,#2
	LD	D,(HL) ; D = x
	INC	HL
	LD	E,(HL) ; E = y
    INC HL
    LD	B,(HL)		; BC = tiles
	LDHL SP,#4

;while 0xff41 & 02 != 0 (cannot write)
	LD	DE,#0xff41
1$:
	LD	A,(DE)
	AND	A, #0x02
	JR	NZ,1$

;Write tile
	LD	A,(HL)
	LD	(BC),A

;Check again stat is 0 or 1
	LD	A,(DE)
	AND	A, #0x02
	JR	NZ,1$
                  ; There is no register to restore
    RET           ; Return result in DE