/*
**	CACWEAPONS.INC -- CrystAlien Conflict weapon table.
**
**	GENERATED FILE -- regenerate with: python tools/gen_dll_types.py
**
**	CrystAlien's combat model is uniform: every shot does 25 damage, there
**	are no armour classes and no damage multipliers.  Only range and rate
**	of fire vary, and only across 13 distinct combinations, so that many of
**	TD's existing weapon slots are retuned instead of extending the enum.
**
**	Rate of fire is scaled 15/23 because CrystAlien's logic runs at 23 fps
**	and TD's nominally ticks at 15.
*/

	{BULLET_LASER,	25,	20,	0x0215,	VOC_NONE,	ANIM_NONE},	// slot 0: laser r=200px rof=20  [UA_evil,UA_good]
	{BULLET_LASER,	25,	10,	0x029B,	VOC_NONE,	ANIM_NONE},	// slot 1: laser r=250px rof=10  [UE_evil,UE_good]
	{BULLET_LASER,	25,	3,	0x02DD,	VOC_NONE,	ANIM_NONE},	// slot 2: laser r=275px rof=3  [UM_evil]
	{BULLET_LASER,	25,	3,	0x03A5,	VOC_NONE,	ANIM_NONE},	// slot 3: laser r=350px rof=3  [UQ_evil]
	{BULLET_SSM,	25,	3,	0x02DD,	VOC_NONE,	ANIM_NONE},	// slot 4: rocket r=275px rof=3  [UM_evil]
	{BULLET_SSM,	25,	2,	0x0320,	VOC_NONE,	ANIM_NONE},	// slot 5: rocket r=300px rof=2  [UG_evil,UG_good]
	{BULLET_SSM,	25,	4,	0x0320,	VOC_NONE,	ANIM_NONE},	// slot 6: rocket r=300px rof=4  [BD_evil,BD_good]
	{BULLET_SSM,	25,	5,	0x0320,	VOC_NONE,	ANIM_NONE},	// slot 7: rocket r=300px rof=5  [UF_evil,UF_good]
	{BULLET_SSM,	25,	7,	0x0320,	VOC_NONE,	ANIM_NONE},	// slot 8: rocket r=300px rof=7  [UR_good]
	{BULLET_SSM,	25,	10,	0x0320,	VOC_NONE,	ANIM_NONE},	// slot 9: rocket r=300px rof=10  [UP_evil,UP_good]
	{BULLET_SSM,	25,	20,	0x0320,	VOC_NONE,	ANIM_NONE},	// slot 10: rocket r=300px rof=20  [UC_evil,UC_good]
	{BULLET_SSM,	25,	3,	0x03A5,	VOC_NONE,	ANIM_NONE},	// slot 11: rocket r=350px rof=3  [UQ_evil]
	{BULLET_SSM,	25,	10,	0x0535,	VOC_NONE,	ANIM_NONE},	// slot 12: rocket r=500px rof=10  [BK_evil,BK_good]
	{BULLET_NONE,	0,	1,	0x0000,	VOC_NONE,	ANIM_NONE},	// WEAPON_CAC_13 -- unused
	{BULLET_NONE,	0,	1,	0x0000,	VOC_NONE,	ANIM_NONE},	// WEAPON_CAC_14 -- unused
	{BULLET_NONE,	0,	1,	0x0000,	VOC_NONE,	ANIM_NONE},	// WEAPON_CAC_15 -- unused
