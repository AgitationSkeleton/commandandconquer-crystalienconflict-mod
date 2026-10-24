/*
**	CACTYPES.CPP -- CrystAlien Conflict type tables.
**
**	GENERATED FILE -- regenerate with: python tools/gen_dll_types.py
**
**	Both factions share each slot; CAC_Faction_Suffix() in
**	DLLInterface.cpp picks the owning side's artwork at draw time.
*/

#include "function.h"
#include "CACSTATS.H"
#include "CACSLOTS.INC"
#include "CACTYPES.H"

#define	MCW	MAP_CELL_W

#include "CACFOOT.INC"

static short const CACList1[]  = {0, REFRESH_EOL};
static short const CACList11[] = {0, 1, REFRESH_EOL};
static short const CACList2[]  = {0, 1, MCW, MCW+1, REFRESH_EOL};
static short const CACList32[] = {0, 1, 2, MCW, MCW+1, MCW+2, REFRESH_EOL};

#define	XYCELL(x,y)	((y)*MAP_CELL_W+(x))

static short const CACExit11[] = {
	XYCELL(0,1), XYCELL(-1,1), XYCELL(1,1), XYCELL(-1,0), XYCELL(1,0), XYCELL(-1,-1), XYCELL(0,-1), XYCELL(1,-1),
	REFRESH_EOL
};
static short const CACExit21[] = {
	XYCELL(0,1), XYCELL(1,1), XYCELL(-1,1), XYCELL(2,1), XYCELL(-1,0), XYCELL(2,0), XYCELL(-1,-1), XYCELL(0,-1), XYCELL(1,-1), XYCELL(2,-1),
	REFRESH_EOL
};
static short const CACExit22[] = {
	XYCELL(0,2), XYCELL(1,2), XYCELL(-1,2), XYCELL(2,2), XYCELL(-1,1), XYCELL(2,1), XYCELL(-1,0), XYCELL(2,0), XYCELL(-1,-1), XYCELL(0,-1), XYCELL(1,-1), XYCELL(2,-1),
	REFRESH_EOL
};
static short const CACExit42[] = {
	XYCELL(0,2), XYCELL(1,2), XYCELL(2,2), XYCELL(3,2), XYCELL(-1,2), XYCELL(4,2), XYCELL(-1,1), XYCELL(4,1), XYCELL(-1,0), XYCELL(4,0), XYCELL(-1,-1), XYCELL(0,-1), XYCELL(1,-1), XYCELL(2,-1), XYCELL(3,-1), XYCELL(4,-1),
	REFRESH_EOL
};

BuildingTypeClass const CACClassBAGOOD(
	CAC_SLOT_BAGOOD,
	TXT_CONST_YARD,						// NAME:		Short name of the structure.
	"BAGOOD",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(24,36),			// Exit point: centre of the footprint, low.
	1,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BAGOOD,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		true,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		false,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BAGOOD,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_BUILDINGTYPE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BAGOOD_STRENGTH,			// STRNTH:	Full strength of building.
	3,							// SIGHTRANGE:	CrystAlien spread=3 tiles.
	CAC_BAGOOD_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BAGOOD_POWER,				// POWER:		Power points generated.
	CAC_BAGOOD_DRAIN,				// DRAIN:		Power points required.
	CAC_BAGOOD_SIZE,				// SIZE:		Building size.
	(short const *)CACExit22,		// Preferred exit cell list.
	(short const *)CACOccBAGOOD,		// OCCUPYLIST.
	(short const *)CACFootBAGOOD		// OVERLAPLIST.
);

BuildingTypeClass const CACClassBAEVIL(
	CAC_SLOT_BAEVIL,
	TXT_CONST_YARD,						// NAME:		Short name of the structure.
	"BAEVIL",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(24,36),			// Exit point: centre of the footprint, low.
	1,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BAEVIL,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		true,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		false,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BAEVIL,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_BUILDINGTYPE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BAEVIL_STRENGTH,			// STRNTH:	Full strength of building.
	3,							// SIGHTRANGE:	CrystAlien spread=3 tiles.
	CAC_BAEVIL_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BAEVIL_POWER,				// POWER:		Power points generated.
	CAC_BAEVIL_DRAIN,				// DRAIN:		Power points required.
	CAC_BAEVIL_SIZE,				// SIZE:		Building size.
	(short const *)CACExit22,		// Preferred exit cell list.
	(short const *)CACOccBAEVIL,		// OCCUPYLIST.
	(short const *)CACFootBAEVIL		// OVERLAPLIST.
);

BuildingTypeClass const CACClassBBGOOD(
	CAC_SLOT_BBGOOD,
	TXT_POWER,						// NAME:		Short name of the structure.
	"BBGOOD",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(12,12),			// Exit point: centre of the footprint, low.
	2,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BBGOOD,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		false,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		false,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BBGOOD,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_NONE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BBGOOD_STRENGTH,			// STRNTH:	Full strength of building.
	2,							// SIGHTRANGE:	CrystAlien spread=2 tiles.
	CAC_BBGOOD_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BBGOOD_POWER,				// POWER:		Power points generated.
	CAC_BBGOOD_DRAIN,				// DRAIN:		Power points required.
	CAC_BBGOOD_SIZE,				// SIZE:		Building size.
	(short const *)CACExit11,		// Preferred exit cell list.
	(short const *)CACOccBBGOOD,		// OCCUPYLIST.
	(short const *)CACFootBBGOOD		// OVERLAPLIST.
);

BuildingTypeClass const CACClassBBEVIL(
	CAC_SLOT_BBEVIL,
	TXT_POWER,						// NAME:		Short name of the structure.
	"BBEVIL",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(12,12),			// Exit point: centre of the footprint, low.
	2,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BBEVIL,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		false,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		false,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BBEVIL,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_NONE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BBEVIL_STRENGTH,			// STRNTH:	Full strength of building.
	2,							// SIGHTRANGE:	CrystAlien spread=2 tiles.
	CAC_BBEVIL_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BBEVIL_POWER,				// POWER:		Power points generated.
	CAC_BBEVIL_DRAIN,				// DRAIN:		Power points required.
	CAC_BBEVIL_SIZE,				// SIZE:		Building size.
	(short const *)CACExit11,		// Preferred exit cell list.
	(short const *)CACOccBBEVIL,		// OCCUPYLIST.
	(short const *)CACFootBBEVIL		// OVERLAPLIST.
);

BuildingTypeClass const CACClassBCGOOD(
	CAC_SLOT_BCGOOD,
	TXT_BARRACKS,						// NAME:		Short name of the structure.
	"BCGOOD",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(24,12),			// Exit point: centre of the footprint, low.
	3,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BCGOOD,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		true,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		false,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BCGOOD,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_INFANTRYTYPE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BCGOOD_STRENGTH,			// STRNTH:	Full strength of building.
	2,							// SIGHTRANGE:	CrystAlien spread=2 tiles.
	CAC_BCGOOD_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BCGOOD_POWER,				// POWER:		Power points generated.
	CAC_BCGOOD_DRAIN,				// DRAIN:		Power points required.
	CAC_BCGOOD_SIZE,				// SIZE:		Building size.
	(short const *)CACExit21,		// Preferred exit cell list.
	(short const *)CACOccBCGOOD,		// OCCUPYLIST.
	(short const *)CACFootBCGOOD		// OVERLAPLIST.
);

BuildingTypeClass const CACClassBCEVIL(
	CAC_SLOT_BCEVIL,
	TXT_BARRACKS,						// NAME:		Short name of the structure.
	"BCEVIL",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(24,12),			// Exit point: centre of the footprint, low.
	3,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BCEVIL,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		true,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		false,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BCEVIL,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_INFANTRYTYPE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BCEVIL_STRENGTH,			// STRNTH:	Full strength of building.
	2,							// SIGHTRANGE:	CrystAlien spread=2 tiles.
	CAC_BCEVIL_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BCEVIL_POWER,				// POWER:		Power points generated.
	CAC_BCEVIL_DRAIN,				// DRAIN:		Power points required.
	CAC_BCEVIL_SIZE,				// SIZE:		Building size.
	(short const *)CACExit21,		// Preferred exit cell list.
	(short const *)CACOccBCEVIL,		// OCCUPYLIST.
	(short const *)CACFootBCEVIL		// OVERLAPLIST.
);

BuildingTypeClass const CACClassBDGOOD(
	CAC_SLOT_BDGOOD,
	TXT_TURRET,						// NAME:		Short name of the structure.
	"BDGOOD",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(12,12),			// Exit point: centre of the footprint, low.
	4,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BDGOOD,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		false,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		true,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BDGOOD,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_NONE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BDGOOD_STRENGTH,			// STRNTH:	Full strength of building.
	4,							// SIGHTRANGE:	CrystAlien spread=4 tiles.
	CAC_BDGOOD_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	(WeaponType)(WEAPON_CAC_FIRST + 6),WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BDGOOD_POWER,				// POWER:		Power points generated.
	CAC_BDGOOD_DRAIN,				// DRAIN:		Power points required.
	CAC_BDGOOD_SIZE,				// SIZE:		Building size.
	(short const *)CACExit11,		// Preferred exit cell list.
	(short const *)CACOccBDGOOD,		// OCCUPYLIST.
	(short const *)CACFootBDGOOD		// OVERLAPLIST.
);

BuildingTypeClass const CACClassBDEVIL(
	CAC_SLOT_BDEVIL,
	TXT_TURRET,						// NAME:		Short name of the structure.
	"BDEVIL",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(12,12),			// Exit point: centre of the footprint, low.
	4,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BDEVIL,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		false,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		true,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BDEVIL,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_NONE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BDEVIL_STRENGTH,			// STRNTH:	Full strength of building.
	4,							// SIGHTRANGE:	CrystAlien spread=4 tiles.
	CAC_BDEVIL_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	(WeaponType)(WEAPON_CAC_FIRST + 6),WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BDEVIL_POWER,				// POWER:		Power points generated.
	CAC_BDEVIL_DRAIN,				// DRAIN:		Power points required.
	CAC_BDEVIL_SIZE,				// SIZE:		Building size.
	(short const *)CACExit11,		// Preferred exit cell list.
	(short const *)CACOccBDEVIL,		// OCCUPYLIST.
	(short const *)CACFootBDEVIL		// OVERLAPLIST.
);

BuildingTypeClass const CACClassBEGOOD(
	CAC_SLOT_BEGOOD,
	TXT_WEAPON_FACTORY,						// NAME:		Short name of the structure.
	"BEGOOD",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(24,36),			// Exit point: centre of the footprint, low.
	4,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BEGOOD,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		true,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		false,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BEGOOD,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_UNITTYPE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BEGOOD_STRENGTH,			// STRNTH:	Full strength of building.
	2,							// SIGHTRANGE:	CrystAlien spread=2 tiles.
	CAC_BEGOOD_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BEGOOD_POWER,				// POWER:		Power points generated.
	CAC_BEGOOD_DRAIN,				// DRAIN:		Power points required.
	CAC_BEGOOD_SIZE,				// SIZE:		Building size.
	(short const *)CACExit22,		// Preferred exit cell list.
	(short const *)CACOccBEGOOD,		// OCCUPYLIST.
	(short const *)CACFootBEGOOD		// OVERLAPLIST.
);

BuildingTypeClass const CACClassBEEVIL(
	CAC_SLOT_BEEVIL,
	TXT_WEAPON_FACTORY,						// NAME:		Short name of the structure.
	"BEEVIL",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(24,36),			// Exit point: centre of the footprint, low.
	4,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BEEVIL,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		true,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		false,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BEEVIL,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_UNITTYPE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BEEVIL_STRENGTH,			// STRNTH:	Full strength of building.
	2,							// SIGHTRANGE:	CrystAlien spread=2 tiles.
	CAC_BEEVIL_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BEEVIL_POWER,				// POWER:		Power points generated.
	CAC_BEEVIL_DRAIN,				// DRAIN:		Power points required.
	CAC_BEEVIL_SIZE,				// SIZE:		Building size.
	(short const *)CACExit22,		// Preferred exit cell list.
	(short const *)CACOccBEEVIL,		// OCCUPYLIST.
	(short const *)CACFootBEEVIL		// OVERLAPLIST.
);

BuildingTypeClass const CACClassBFGOOD(
	CAC_SLOT_BFGOOD,
	TXT_COMMAND,						// NAME:		Short name of the structure.
	"BFGOOD",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(24,36),			// Exit point: centre of the footprint, low.
	4,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BFGOOD,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		true,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		false,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BFGOOD,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_AIRCRAFTTYPE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BFGOOD_STRENGTH,			// STRNTH:	Full strength of building.
	2,							// SIGHTRANGE:	CrystAlien spread=2 tiles.
	CAC_BFGOOD_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BFGOOD_POWER,				// POWER:		Power points generated.
	CAC_BFGOOD_DRAIN,				// DRAIN:		Power points required.
	CAC_BFGOOD_SIZE,				// SIZE:		Building size.
	(short const *)CACExit22,		// Preferred exit cell list.
	(short const *)CACOccBFGOOD,		// OCCUPYLIST.
	(short const *)CACFootBFGOOD		// OVERLAPLIST.
);

BuildingTypeClass const CACClassBFEVIL(
	CAC_SLOT_BFEVIL,
	TXT_COMMAND,						// NAME:		Short name of the structure.
	"BFEVIL",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(24,36),			// Exit point: centre of the footprint, low.
	4,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BFEVIL,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		true,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		false,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BFEVIL,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_AIRCRAFTTYPE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BFEVIL_STRENGTH,			// STRNTH:	Full strength of building.
	2,							// SIGHTRANGE:	CrystAlien spread=2 tiles.
	CAC_BFEVIL_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BFEVIL_POWER,				// POWER:		Power points generated.
	CAC_BFEVIL_DRAIN,				// DRAIN:		Power points required.
	CAC_BFEVIL_SIZE,				// SIZE:		Building size.
	(short const *)CACExit22,		// Preferred exit cell list.
	(short const *)CACOccBFEVIL,		// OCCUPYLIST.
	(short const *)CACFootBFEVIL		// OVERLAPLIST.
);

BuildingTypeClass const CACClassBGGOOD(
	CAC_SLOT_BGGOOD,
	TXT_TEMPLE,						// NAME:		Short name of the structure.
	"BGGOOD",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(12,12),			// Exit point: centre of the footprint, low.
	5,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BGGOOD,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		false,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		false,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BGGOOD,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_NONE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BGGOOD_STRENGTH,			// STRNTH:	Full strength of building.
	2,							// SIGHTRANGE:	CrystAlien spread=2 tiles.
	CAC_BGGOOD_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BGGOOD_POWER,				// POWER:		Power points generated.
	CAC_BGGOOD_DRAIN,				// DRAIN:		Power points required.
	CAC_BGGOOD_SIZE,				// SIZE:		Building size.
	(short const *)CACExit11,		// Preferred exit cell list.
	(short const *)CACOccBGGOOD,		// OCCUPYLIST.
	(short const *)CACFootBGGOOD		// OVERLAPLIST.
);

BuildingTypeClass const CACClassBGEVIL(
	CAC_SLOT_BGEVIL,
	TXT_TEMPLE,						// NAME:		Short name of the structure.
	"BGEVIL",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(12,12),			// Exit point: centre of the footprint, low.
	5,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BGEVIL,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		false,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		false,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BGEVIL,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_NONE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BGEVIL_STRENGTH,			// STRNTH:	Full strength of building.
	2,							// SIGHTRANGE:	CrystAlien spread=2 tiles.
	CAC_BGEVIL_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BGEVIL_POWER,				// POWER:		Power points generated.
	CAC_BGEVIL_DRAIN,				// DRAIN:		Power points required.
	CAC_BGEVIL_SIZE,				// SIZE:		Building size.
	(short const *)CACExit11,		// Preferred exit cell list.
	(short const *)CACOccBGEVIL,		// OCCUPYLIST.
	(short const *)CACFootBGEVIL		// OVERLAPLIST.
);

BuildingTypeClass const CACClassBHGOOD(
	CAC_SLOT_BHGOOD,
	TXT_EYE,						// NAME:		Short name of the structure.
	"BHGOOD",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(12,12),			// Exit point: centre of the footprint, low.
	6,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BHGOOD,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		false,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		false,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BHGOOD,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_NONE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BHGOOD_STRENGTH,			// STRNTH:	Full strength of building.
	2,							// SIGHTRANGE:	CrystAlien spread=2 tiles.
	CAC_BHGOOD_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BHGOOD_POWER,				// POWER:		Power points generated.
	CAC_BHGOOD_DRAIN,				// DRAIN:		Power points required.
	CAC_BHGOOD_SIZE,				// SIZE:		Building size.
	(short const *)CACExit11,		// Preferred exit cell list.
	(short const *)CACOccBHGOOD,		// OCCUPYLIST.
	(short const *)CACFootBHGOOD		// OVERLAPLIST.
);

BuildingTypeClass const CACClassBHEVIL(
	CAC_SLOT_BHEVIL,
	TXT_EYE,						// NAME:		Short name of the structure.
	"BHEVIL",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(12,12),			// Exit point: centre of the footprint, low.
	6,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BHEVIL,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		false,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		false,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BHEVIL,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_NONE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BHEVIL_STRENGTH,			// STRNTH:	Full strength of building.
	2,							// SIGHTRANGE:	CrystAlien spread=2 tiles.
	CAC_BHEVIL_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BHEVIL_POWER,				// POWER:		Power points generated.
	CAC_BHEVIL_DRAIN,				// DRAIN:		Power points required.
	CAC_BHEVIL_SIZE,				// SIZE:		Building size.
	(short const *)CACExit11,		// Preferred exit cell list.
	(short const *)CACOccBHEVIL,		// OCCUPYLIST.
	(short const *)CACFootBHEVIL		// OVERLAPLIST.
);

BuildingTypeClass const CACClassBKGOOD(
	CAC_SLOT_BKGOOD,
	TXT_MISSION,						// NAME:		Short name of the structure.
	"BKGOOD",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(24,36),			// Exit point: centre of the footprint, low.
	2,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BKGOOD,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		true,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		false,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BKGOOD,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_BUILDINGTYPE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BKGOOD_STRENGTH,			// STRNTH:	Full strength of building.
	3,							// SIGHTRANGE:	CrystAlien spread=3 tiles.
	CAC_BKGOOD_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	(WeaponType)(WEAPON_CAC_FIRST + 12),WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BKGOOD_POWER,				// POWER:		Power points generated.
	CAC_BKGOOD_DRAIN,				// DRAIN:		Power points required.
	CAC_BKGOOD_SIZE,				// SIZE:		Building size.
	(short const *)CACExit22,		// Preferred exit cell list.
	(short const *)CACOccBKGOOD,		// OCCUPYLIST.
	(short const *)CACFootBKGOOD		// OVERLAPLIST.
);

BuildingTypeClass const CACClassBKEVIL(
	CAC_SLOT_BKEVIL,
	TXT_MISSION,						// NAME:		Short name of the structure.
	"BKEVIL",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(24,36),			// Exit point: centre of the footprint, low.
	2,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BKEVIL,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		true,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		false,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BKEVIL,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_BUILDINGTYPE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BKEVIL_STRENGTH,			// STRNTH:	Full strength of building.
	3,							// SIGHTRANGE:	CrystAlien spread=3 tiles.
	CAC_BKEVIL_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	(WeaponType)(WEAPON_CAC_FIRST + 12),WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BKEVIL_POWER,				// POWER:		Power points generated.
	CAC_BKEVIL_DRAIN,				// DRAIN:		Power points required.
	CAC_BKEVIL_SIZE,				// SIZE:		Building size.
	(short const *)CACExit22,		// Preferred exit cell list.
	(short const *)CACOccBKEVIL,		// OCCUPYLIST.
	(short const *)CACFootBKEVIL		// OVERLAPLIST.
);

BuildingTypeClass const CACClassBLGOOD(
	CAC_SLOT_BLGOOD,
	TXT_REFINERY,						// NAME:		Short name of the structure.
	"BLGOOD",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(24,36),			// Exit point: centre of the footprint, low.
	3,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BLGOOD,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		false,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		false,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BLGOOD,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_NONE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BLGOOD_STRENGTH,			// STRNTH:	Full strength of building.
	2,							// SIGHTRANGE:	CrystAlien spread=2 tiles.
	CAC_BLGOOD_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BLGOOD_POWER,				// POWER:		Power points generated.
	CAC_BLGOOD_DRAIN,				// DRAIN:		Power points required.
	CAC_BLGOOD_SIZE,				// SIZE:		Building size.
	(short const *)CACExit22,		// Preferred exit cell list.
	(short const *)CACOccBLGOOD,		// OCCUPYLIST.
	(short const *)CACFootBLGOOD		// OVERLAPLIST.
);

BuildingTypeClass const CACClassBLEVIL(
	CAC_SLOT_BLEVIL,
	TXT_REFINERY,						// NAME:		Short name of the structure.
	"BLEVIL",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(24,36),			// Exit point: centre of the footprint, low.
	3,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BLEVIL,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		false,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		false,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BLEVIL,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_NONE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BLEVIL_STRENGTH,			// STRNTH:	Full strength of building.
	2,							// SIGHTRANGE:	CrystAlien spread=2 tiles.
	CAC_BLEVIL_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BLEVIL_POWER,				// POWER:		Power points generated.
	CAC_BLEVIL_DRAIN,				// DRAIN:		Power points required.
	CAC_BLEVIL_SIZE,				// SIZE:		Building size.
	(short const *)CACExit22,		// Preferred exit cell list.
	(short const *)CACOccBLEVIL,		// OCCUPYLIST.
	(short const *)CACFootBLEVIL		// OVERLAPLIST.
);

BuildingTypeClass const CACClassBJGOOD(
	CAC_SLOT_BJGOOD,
	TXT_HOSPITAL,						// NAME:		Short name of the structure.
	"BJGOOD",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(12,12),			// Exit point: centre of the footprint, low.
	1,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BJGOOD,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		false,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		false,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BJGOOD,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_NONE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BJGOOD_STRENGTH,			// STRNTH:	Full strength of building.
	2,							// SIGHTRANGE:	CrystAlien spread=2 tiles.
	CAC_BJGOOD_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BJGOOD_POWER,				// POWER:		Power points generated.
	CAC_BJGOOD_DRAIN,				// DRAIN:		Power points required.
	CAC_BJGOOD_SIZE,				// SIZE:		Building size.
	(short const *)CACExit11,		// Preferred exit cell list.
	(short const *)CACOccBJGOOD,		// OCCUPYLIST.
	(short const *)CACFootBJGOOD		// OVERLAPLIST.
);

BuildingTypeClass const CACClassBJEVIL(
	CAC_SLOT_BJEVIL,
	TXT_HOSPITAL,						// NAME:		Short name of the structure.
	"BJEVIL",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(12,12),			// Exit point: centre of the footprint, low.
	1,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BJEVIL,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		false,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		false,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BJEVIL,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_NONE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BJEVIL_STRENGTH,			// STRNTH:	Full strength of building.
	2,							// SIGHTRANGE:	CrystAlien spread=2 tiles.
	CAC_BJEVIL_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BJEVIL_POWER,				// POWER:		Power points generated.
	CAC_BJEVIL_DRAIN,				// DRAIN:		Power points required.
	CAC_BJEVIL_SIZE,				// SIZE:		Building size.
	(short const *)CACExit11,		// Preferred exit cell list.
	(short const *)CACOccBJEVIL,		// OCCUPYLIST.
	(short const *)CACFootBJEVIL		// OVERLAPLIST.
);

BuildingTypeClass const CACClassBXGOOD(
	CAC_SLOT_BXGOOD,
	TXT_TANKER,						// NAME:		Short name of the structure.
	"BXGOOD",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(24,36),			// Exit point: centre of the footprint, low.
	1,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BXGOOD,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		false,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		false,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BXGOOD,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_NONE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BXGOOD_STRENGTH,			// STRNTH:	Full strength of building.
	3,							// SIGHTRANGE:	CrystAlien spread=3 tiles.
	CAC_BXGOOD_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BXGOOD_POWER,				// POWER:		Power points generated.
	CAC_BXGOOD_DRAIN,				// DRAIN:		Power points required.
	CAC_BXGOOD_SIZE,				// SIZE:		Building size.
	(short const *)CACExit22,		// Preferred exit cell list.
	(short const *)CACOccBXGOOD,		// OCCUPYLIST.
	(short const *)CACFootBXGOOD		// OVERLAPLIST.
);

BuildingTypeClass const CACClassBXEVIL(
	CAC_SLOT_BXEVIL,
	TXT_TANKER,						// NAME:		Short name of the structure.
	"BXEVIL",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(24,36),			// Exit point: centre of the footprint, low.
	1,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BXEVIL,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		false,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		false,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BXEVIL,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_NONE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BXEVIL_STRENGTH,			// STRNTH:	Full strength of building.
	3,							// SIGHTRANGE:	CrystAlien spread=3 tiles.
	CAC_BXEVIL_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BXEVIL_POWER,				// POWER:		Power points generated.
	CAC_BXEVIL_DRAIN,				// DRAIN:		Power points required.
	CAC_BXEVIL_SIZE,				// SIZE:		Building size.
	(short const *)CACExit22,		// Preferred exit cell list.
	(short const *)CACOccBXEVIL,		// OCCUPYLIST.
	(short const *)CACFootBXEVIL		// OVERLAPLIST.
);

BuildingTypeClass const CACClassBIGOOD(
	CAC_SLOT_BIGOOD,
	TXT_BIO_LAB,						// NAME:		Short name of the structure.
	"BIGOOD",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(48,36),			// Exit point: centre of the footprint, low.
	1,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BIGOOD,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		false,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		false,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BIGOOD,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_NONE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BIGOOD_STRENGTH,			// STRNTH:	Full strength of building.
	2,							// SIGHTRANGE:	CrystAlien spread=2 tiles.
	CAC_BIGOOD_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BIGOOD_POWER,				// POWER:		Power points generated.
	CAC_BIGOOD_DRAIN,				// DRAIN:		Power points required.
	CAC_BIGOOD_SIZE,				// SIZE:		Building size.
	(short const *)CACExit42,		// Preferred exit cell list.
	(short const *)CACOccBIGOOD,		// OCCUPYLIST.
	(short const *)CACFootBIGOOD		// OVERLAPLIST.
);

BuildingTypeClass const CACClassBIEVIL(
	CAC_SLOT_BIEVIL,
	TXT_BIO_LAB,						// NAME:		Short name of the structure.
	"BIEVIL",						// NAME:		Asset name; faction suffix added at draw time.
	XYP_COORD(48,36),			// Exit point: centre of the footprint, low.
	1,							// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BIEVIL,				// Building prerequisite.
		false,		// Has ability to detect adjacent cloaked objects?
		false,		// Animation rate is regulated for constant speed?
		true,		// Requires a bib dirt patch?
		false,		// Always use the given name for the building?
		false,		// Is this a wall type structure?
		false,		// Is it a factory type building?
		true,		// Can this building be captured?
		false,		// Does it catch fire?
		true,		// CrystAlien overlays smoke rather than swapping to damaged art.
		false,		// Is it invisible to radar?
		true,		// Can the player select this?
		true,		// Is this a legal target for attack or move?
		false,		// Is this an insignificant building?
		false,		// Is it immune to normal combat damage?
		false,		// Theater specific graphic image?
		false,		// Does it have a rotating turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired?
		CAC_BUILDABLE_BIEVIL,		// Can it be manufactured by the player?
		true,		// Does it contain a crew?
		false,		// Does building care less if placed on concrete?
	RTTI_NONE,				// The object type produced at this factory.
	DIR_N,						// Starting idle frame to match construction.
	CAC_BIEVIL_STRENGTH,			// STRNTH:	Full strength of building.
	2,							// SIGHTRANGE:	CrystAlien spread=2 tiles.
	CAC_BIEVIL_COST,				// COST:		Cost to purchase.
	1,							// SCENARIO:	Starting availability scenario.
	0,80,						// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,				// ARMOR:		Armor type
	0,							// CANENTER:	Units that can enter building.
	0,							// CAPACITY:	Spice storage capacity.
	CAC_BIEVIL_POWER,				// POWER:		Power points generated.
	CAC_BIEVIL_DRAIN,				// DRAIN:		Power points required.
	CAC_BIEVIL_SIZE,				// SIZE:		Building size.
	(short const *)CACExit42,		// Preferred exit cell list.
	(short const *)CACOccBIEVIL,		// OCCUPYLIST.
	(short const *)CACFootBIEVIL		// OVERLAPLIST.
);

/* ---- vehicles ---------------------------------------------- */

UnitTypeClass const CACUnitUDGOOD(
	CAC_SLOT_UDGOOD,
	TXT_HARVESTER,				// NAME:		Text name of this unit type.
	"UDGOOD",				// NAME:		Asset name; faction suffix added at draw time.
	ANIM_CAC_BOOM,		// EXPLOSION:	Type of explosion when destroyed.
	2,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_UDGOOD,		// Building prerequisite.
		false,		// Can this be a goodie surprise from a crate?
		false,		// Is a leader type?
		false,		// Only has eight facings?
		true,		// Always use the given name for the vehicle?
		false,		// Is this a typical transport vehicle?
		false,		// Can it be crushed by a heavy vehicle?
		false,		// Can this unit squash infantry?
		true,		// Does this unit harvest Tiberium?
		false,		// Is invisible to radar?
		true,		// Is selectable by player?
		true,		// Can it be a target for attack or move?
		false,		// Is it insignificant (won't be announced)?
		false,		// Is it immune to normal combat damage?
		false,		// Is it equipped with a combat turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired in a repair facility?
		CAC_BUILDABLE_UDGOOD,		// Can the player construct or order this unit?
		false,		// Is there a crew inside?  Unit.destroy plays the death line, explodes, frees the block and kills whatever it was carrying.  It puts nobody on the ground: CrystAlien has no bailing-out crew, and a survivor here is a stock C&C infantryman who screams like one.
		false,		// Does it have a rotating radar dish?
		false,		// Is there an associated firing animation?
		false,		// Must the turret be locked down while moving?
		false,		// Does it lay tracks while moving?
		false,		// Is this a gigundo-rotund-enormous unit?
		false,		// Is the unit's art "chunky" cardinal facing only?
		false,		// Is the unit capable of cloaking?
		false,		// Does the unit have a constant animation?
	-1,					// AMMO:		Number of shots it has (default).
	CAC_UDGOOD_STRENGTH,		// STRENGTH:	Strength (in damage points).
	3,					// SIGHTRANGE:	CrystAlien spread=3 tiles.
	CAC_UDGOOD_COST,			// COST:		Cost to build (Credits).
	1,					// SCENARIO:	Starting availability scenario.
	50,50,				// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,			// ARMOR:		Armor type
	SPEED_TRACK,			// MOVE:		Locomotion type.
	(MPHType)20,			// SPEED:		CrystAlien speed 6.
	6,					// ROT:		Half-turn within one tile move.
	0,					// Turret center offset along body centerline.
	MISSION_HARVEST		// ORDERS:	Default order to give new unit.
);

UnitTypeClass const CACUnitUDEVIL(
	CAC_SLOT_UDEVIL,
	TXT_HARVESTER,				// NAME:		Text name of this unit type.
	"UDEVIL",				// NAME:		Asset name; faction suffix added at draw time.
	ANIM_CAC_BOOM,		// EXPLOSION:	Type of explosion when destroyed.
	2,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_UDEVIL,		// Building prerequisite.
		false,		// Can this be a goodie surprise from a crate?
		false,		// Is a leader type?
		false,		// Only has eight facings?
		true,		// Always use the given name for the vehicle?
		false,		// Is this a typical transport vehicle?
		false,		// Can it be crushed by a heavy vehicle?
		false,		// Can this unit squash infantry?
		true,		// Does this unit harvest Tiberium?
		false,		// Is invisible to radar?
		true,		// Is selectable by player?
		true,		// Can it be a target for attack or move?
		false,		// Is it insignificant (won't be announced)?
		false,		// Is it immune to normal combat damage?
		false,		// Is it equipped with a combat turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired in a repair facility?
		CAC_BUILDABLE_UDEVIL,		// Can the player construct or order this unit?
		false,		// Is there a crew inside?  Unit.destroy plays the death line, explodes, frees the block and kills whatever it was carrying.  It puts nobody on the ground: CrystAlien has no bailing-out crew, and a survivor here is a stock C&C infantryman who screams like one.
		false,		// Does it have a rotating radar dish?
		false,		// Is there an associated firing animation?
		false,		// Must the turret be locked down while moving?
		false,		// Does it lay tracks while moving?
		false,		// Is this a gigundo-rotund-enormous unit?
		false,		// Is the unit's art "chunky" cardinal facing only?
		false,		// Is the unit capable of cloaking?
		false,		// Does the unit have a constant animation?
	-1,					// AMMO:		Number of shots it has (default).
	CAC_UDEVIL_STRENGTH,		// STRENGTH:	Strength (in damage points).
	3,					// SIGHTRANGE:	CrystAlien spread=3 tiles.
	CAC_UDEVIL_COST,			// COST:		Cost to build (Credits).
	1,					// SCENARIO:	Starting availability scenario.
	50,50,				// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,			// ARMOR:		Armor type
	SPEED_TRACK,			// MOVE:		Locomotion type.
	(MPHType)20,			// SPEED:		CrystAlien speed 6.
	6,					// ROT:		Half-turn within one tile move.
	0,					// Turret center offset along body centerline.
	MISSION_HARVEST		// ORDERS:	Default order to give new unit.
);

UnitTypeClass const CACUnitUEGOOD(
	CAC_SLOT_UEGOOD,
	TXT_DUNE_BUGGY,				// NAME:		Text name of this unit type.
	"UEGOOD",				// NAME:		Asset name; faction suffix added at draw time.
	ANIM_CAC_BOOM,		// EXPLOSION:	Type of explosion when destroyed.
	5,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_UEGOOD,		// Building prerequisite.
		false,		// Can this be a goodie surprise from a crate?
		false,		// Is a leader type?
		false,		// Only has eight facings?
		true,		// Always use the given name for the vehicle?
		false,		// Is this a typical transport vehicle?
		false,		// Can it be crushed by a heavy vehicle?
		false,		// Can this unit squash infantry?
		false,		// Does this unit harvest Tiberium?
		false,		// Is invisible to radar?
		true,		// Is selectable by player?
		true,		// Can it be a target for attack or move?
		false,		// Is it insignificant (won't be announced)?
		false,		// Is it immune to normal combat damage?
		false,		// Is it equipped with a combat turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired in a repair facility?
		CAC_BUILDABLE_UEGOOD,		// Can the player construct or order this unit?
		false,		// Is there a crew inside?  Unit.destroy plays the death line, explodes, frees the block and kills whatever it was carrying.  It puts nobody on the ground: CrystAlien has no bailing-out crew, and a survivor here is a stock C&C infantryman who screams like one.
		false,		// Does it have a rotating radar dish?
		false,		// Is there an associated firing animation?
		false,		// Must the turret be locked down while moving?
		false,		// Does it lay tracks while moving?
		false,		// Is this a gigundo-rotund-enormous unit?
		false,		// Is the unit's art "chunky" cardinal facing only?
		false,		// Is the unit capable of cloaking?
		false,		// Does the unit have a constant animation?
	-1,					// AMMO:		Number of shots it has (default).
	CAC_UEGOOD_STRENGTH,		// STRENGTH:	Strength (in damage points).
	2,					// SIGHTRANGE:	CrystAlien spread=2 tiles.
	CAC_UEGOOD_COST,			// COST:		Cost to build (Credits).
	1,					// SCENARIO:	Starting availability scenario.
	50,50,				// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	(WeaponType)(WEAPON_CAC_FIRST + 1),WEAPON_NONE,
	ARMOR_STEEL,			// ARMOR:		Armor type
	SPEED_WHEEL,			// MOVE:		Locomotion type.
	(MPHType)41,			// SPEED:		CrystAlien speed 10.
	13,					// ROT:		Half-turn within one tile move.
	0,					// Turret center offset along body centerline.
	MISSION_GUARD		// ORDERS:	Default order to give new unit.
);

UnitTypeClass const CACUnitUEEVIL(
	CAC_SLOT_UEEVIL,
	TXT_DUNE_BUGGY,				// NAME:		Text name of this unit type.
	"UEEVIL",				// NAME:		Asset name; faction suffix added at draw time.
	ANIM_CAC_BOOM,		// EXPLOSION:	Type of explosion when destroyed.
	5,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_UEEVIL,		// Building prerequisite.
		false,		// Can this be a goodie surprise from a crate?
		false,		// Is a leader type?
		false,		// Only has eight facings?
		true,		// Always use the given name for the vehicle?
		false,		// Is this a typical transport vehicle?
		false,		// Can it be crushed by a heavy vehicle?
		false,		// Can this unit squash infantry?
		false,		// Does this unit harvest Tiberium?
		false,		// Is invisible to radar?
		true,		// Is selectable by player?
		true,		// Can it be a target for attack or move?
		false,		// Is it insignificant (won't be announced)?
		false,		// Is it immune to normal combat damage?
		false,		// Is it equipped with a combat turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired in a repair facility?
		CAC_BUILDABLE_UEEVIL,		// Can the player construct or order this unit?
		false,		// Is there a crew inside?  Unit.destroy plays the death line, explodes, frees the block and kills whatever it was carrying.  It puts nobody on the ground: CrystAlien has no bailing-out crew, and a survivor here is a stock C&C infantryman who screams like one.
		false,		// Does it have a rotating radar dish?
		false,		// Is there an associated firing animation?
		false,		// Must the turret be locked down while moving?
		false,		// Does it lay tracks while moving?
		false,		// Is this a gigundo-rotund-enormous unit?
		false,		// Is the unit's art "chunky" cardinal facing only?
		false,		// Is the unit capable of cloaking?
		false,		// Does the unit have a constant animation?
	-1,					// AMMO:		Number of shots it has (default).
	CAC_UEEVIL_STRENGTH,		// STRENGTH:	Strength (in damage points).
	2,					// SIGHTRANGE:	CrystAlien spread=2 tiles.
	CAC_UEEVIL_COST,			// COST:		Cost to build (Credits).
	1,					// SCENARIO:	Starting availability scenario.
	50,50,				// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	(WeaponType)(WEAPON_CAC_FIRST + 1),WEAPON_NONE,
	ARMOR_STEEL,			// ARMOR:		Armor type
	SPEED_WHEEL,			// MOVE:		Locomotion type.
	(MPHType)41,			// SPEED:		CrystAlien speed 10.
	13,					// ROT:		Half-turn within one tile move.
	0,					// Turret center offset along body centerline.
	MISSION_GUARD		// ORDERS:	Default order to give new unit.
);

UnitTypeClass const CACUnitUFGOOD(
	CAC_SLOT_UFGOOD,
	TXT_MTANK,				// NAME:		Text name of this unit type.
	"UFGOOD",				// NAME:		Asset name; faction suffix added at draw time.
	ANIM_CAC_BOOM,		// EXPLOSION:	Type of explosion when destroyed.
	5,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_UFGOOD,		// Building prerequisite.
		false,		// Can this be a goodie surprise from a crate?
		false,		// Is a leader type?
		false,		// Only has eight facings?
		true,		// Always use the given name for the vehicle?
		false,		// Is this a typical transport vehicle?
		false,		// Can it be crushed by a heavy vehicle?
		false,		// Can this unit squash infantry?
		false,		// Does this unit harvest Tiberium?
		false,		// Is invisible to radar?
		true,		// Is selectable by player?
		true,		// Can it be a target for attack or move?
		false,		// Is it insignificant (won't be announced)?
		false,		// Is it immune to normal combat damage?
		false,		// Is it equipped with a combat turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired in a repair facility?
		CAC_BUILDABLE_UFGOOD,		// Can the player construct or order this unit?
		false,		// Is there a crew inside?  Unit.destroy plays the death line, explodes, frees the block and kills whatever it was carrying.  It puts nobody on the ground: CrystAlien has no bailing-out crew, and a survivor here is a stock C&C infantryman who screams like one.
		false,		// Does it have a rotating radar dish?
		false,		// Is there an associated firing animation?
		false,		// Must the turret be locked down while moving?
		false,		// Does it lay tracks while moving?
		false,		// Is this a gigundo-rotund-enormous unit?
		false,		// Is the unit's art "chunky" cardinal facing only?
		false,		// Is the unit capable of cloaking?
		false,		// Does the unit have a constant animation?
	-1,					// AMMO:		Number of shots it has (default).
	CAC_UFGOOD_STRENGTH,		// STRENGTH:	Strength (in damage points).
	3,					// SIGHTRANGE:	CrystAlien spread=3 tiles.
	CAC_UFGOOD_COST,			// COST:		Cost to build (Credits).
	1,					// SCENARIO:	Starting availability scenario.
	50,50,				// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	(WeaponType)(WEAPON_CAC_FIRST + 7),WEAPON_NONE,
	ARMOR_STEEL,			// ARMOR:		Armor type
	SPEED_TRACK,			// MOVE:		Locomotion type.
	(MPHType)16,			// SPEED:		CrystAlien speed 5.
	5,					// ROT:		Half-turn within one tile move.
	0,					// Turret center offset along body centerline.
	MISSION_GUARD		// ORDERS:	Default order to give new unit.
);

UnitTypeClass const CACUnitUFEVIL(
	CAC_SLOT_UFEVIL,
	TXT_MTANK,				// NAME:		Text name of this unit type.
	"UFEVIL",				// NAME:		Asset name; faction suffix added at draw time.
	ANIM_CAC_BOOM,		// EXPLOSION:	Type of explosion when destroyed.
	5,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_UFEVIL,		// Building prerequisite.
		false,		// Can this be a goodie surprise from a crate?
		false,		// Is a leader type?
		false,		// Only has eight facings?
		true,		// Always use the given name for the vehicle?
		false,		// Is this a typical transport vehicle?
		false,		// Can it be crushed by a heavy vehicle?
		false,		// Can this unit squash infantry?
		false,		// Does this unit harvest Tiberium?
		false,		// Is invisible to radar?
		true,		// Is selectable by player?
		true,		// Can it be a target for attack or move?
		false,		// Is it insignificant (won't be announced)?
		false,		// Is it immune to normal combat damage?
		false,		// Is it equipped with a combat turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired in a repair facility?
		CAC_BUILDABLE_UFEVIL,		// Can the player construct or order this unit?
		false,		// Is there a crew inside?  Unit.destroy plays the death line, explodes, frees the block and kills whatever it was carrying.  It puts nobody on the ground: CrystAlien has no bailing-out crew, and a survivor here is a stock C&C infantryman who screams like one.
		false,		// Does it have a rotating radar dish?
		false,		// Is there an associated firing animation?
		false,		// Must the turret be locked down while moving?
		false,		// Does it lay tracks while moving?
		false,		// Is this a gigundo-rotund-enormous unit?
		false,		// Is the unit's art "chunky" cardinal facing only?
		false,		// Is the unit capable of cloaking?
		false,		// Does the unit have a constant animation?
	-1,					// AMMO:		Number of shots it has (default).
	CAC_UFEVIL_STRENGTH,		// STRENGTH:	Strength (in damage points).
	3,					// SIGHTRANGE:	CrystAlien spread=3 tiles.
	CAC_UFEVIL_COST,			// COST:		Cost to build (Credits).
	1,					// SCENARIO:	Starting availability scenario.
	50,50,				// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	(WeaponType)(WEAPON_CAC_FIRST + 7),WEAPON_NONE,
	ARMOR_STEEL,			// ARMOR:		Armor type
	SPEED_TRACK,			// MOVE:		Locomotion type.
	(MPHType)16,			// SPEED:		CrystAlien speed 5.
	5,					// ROT:		Half-turn within one tile move.
	0,					// Turret center offset along body centerline.
	MISSION_GUARD		// ORDERS:	Default order to give new unit.
);

UnitTypeClass const CACUnitURGOOD(
	CAC_SLOT_URGOOD,
	TXT_APC,				// NAME:		Text name of this unit type.
	"URGOOD",				// NAME:		Asset name; faction suffix added at draw time.
	ANIM_CAC_BOOM,		// EXPLOSION:	Type of explosion when destroyed.
	5,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_URGOOD,		// Building prerequisite.
		false,		// Can this be a goodie surprise from a crate?
		false,		// Is a leader type?
		false,		// Only has eight facings?
		true,		// Always use the given name for the vehicle?
		true,		// Is this a typical transport vehicle?
		false,		// Can it be crushed by a heavy vehicle?
		false,		// Can this unit squash infantry?
		false,		// Does this unit harvest Tiberium?
		false,		// Is invisible to radar?
		true,		// Is selectable by player?
		true,		// Can it be a target for attack or move?
		false,		// Is it insignificant (won't be announced)?
		false,		// Is it immune to normal combat damage?
		false,		// Is it equipped with a combat turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired in a repair facility?
		CAC_BUILDABLE_URGOOD,		// Can the player construct or order this unit?
		false,		// Is there a crew inside?  Unit.destroy plays the death line, explodes, frees the block and kills whatever it was carrying.  It puts nobody on the ground: CrystAlien has no bailing-out crew, and a survivor here is a stock C&C infantryman who screams like one.
		false,		// Does it have a rotating radar dish?
		false,		// Is there an associated firing animation?
		false,		// Must the turret be locked down while moving?
		false,		// Does it lay tracks while moving?
		false,		// Is this a gigundo-rotund-enormous unit?
		false,		// Is the unit's art "chunky" cardinal facing only?
		false,		// Is the unit capable of cloaking?
		false,		// Does the unit have a constant animation?
	-1,					// AMMO:		Number of shots it has (default).
	CAC_URGOOD_STRENGTH,		// STRENGTH:	Strength (in damage points).
	3,					// SIGHTRANGE:	CrystAlien spread=3 tiles.
	CAC_URGOOD_COST,			// COST:		Cost to build (Credits).
	1,					// SCENARIO:	Starting availability scenario.
	50,50,				// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	(WeaponType)(WEAPON_CAC_FIRST + 8),WEAPON_NONE,
	ARMOR_STEEL,			// ARMOR:		Armor type
	SPEED_TRACK,			// MOVE:		Locomotion type.
	(MPHType)20,			// SPEED:		CrystAlien speed 6.
	6,					// ROT:		Half-turn within one tile move.
	0,					// Turret center offset along body centerline.
	MISSION_GUARD		// ORDERS:	Default order to give new unit.
);

UnitTypeClass const CACUnitBSGOOD(
	CAC_SLOT_BSGOOD,
	TXT_MISSION,				// NAME:		Text name of this unit type.
	"BSGOOD",				// NAME:		Asset name; faction suffix added at draw time.
	ANIM_CAC_PUFF,		// EXPLOSION:	Type of explosion when destroyed.
	1,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BSGOOD,		// Building prerequisite.
		false,		// Can this be a goodie surprise from a crate?
		false,		// Is a leader type?
		false,		// Only has eight facings?
		true,		// Always use the given name for the vehicle?
		false,		// Is this a typical transport vehicle?
		false,		// Can it be crushed by a heavy vehicle?
		false,		// Can this unit squash infantry?
		false,		// Does this unit harvest Tiberium?
		false,		// Is invisible to radar?
		true,		// Is selectable by player?
		true,		// Can it be a target for attack or move?
		false,		// Is it insignificant (won't be announced)?
		false,		// Is it immune to normal combat damage?
		false,		// Is it equipped with a combat turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired in a repair facility?
		CAC_BUILDABLE_BSGOOD,		// Can the player construct or order this unit?
		false,		// Is there a crew inside?  Unit.destroy plays the death line, explodes, frees the block and kills whatever it was carrying.  It puts nobody on the ground: CrystAlien has no bailing-out crew, and a survivor here is a stock C&C infantryman who screams like one.
		false,		// Does it have a rotating radar dish?
		false,		// Is there an associated firing animation?
		false,		// Must the turret be locked down while moving?
		false,		// Does it lay tracks while moving?
		false,		// Is this a gigundo-rotund-enormous unit?
		false,		// Is the unit's art "chunky" cardinal facing only?
		false,		// Is the unit capable of cloaking?
		false,		// Does the unit have a constant animation?
	-1,					// AMMO:		Number of shots it has (default).
	CAC_BSGOOD_STRENGTH,		// STRENGTH:	Strength (in damage points).
	3,					// SIGHTRANGE:	CrystAlien spread=3 tiles.
	CAC_BSGOOD_COST,			// COST:		Cost to build (Credits).
	1,					// SCENARIO:	Starting availability scenario.
	50,50,				// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,			// ARMOR:		Armor type
	SPEED_WINGED,			// MOVE:		Locomotion type.
	(MPHType)8,			// SPEED:		CrystAlien speed 1.
	3,					// ROT:		Half-turn within one tile move.
	0,					// Turret center offset along body centerline.
	MISSION_GUARD		// ORDERS:	Default order to give new unit.
);

UnitTypeClass const CACUnitBSEVIL(
	CAC_SLOT_BSEVIL,
	TXT_MISSION,				// NAME:		Text name of this unit type.
	"BSEVIL",				// NAME:		Asset name; faction suffix added at draw time.
	ANIM_CAC_PUFF,		// EXPLOSION:	Type of explosion when destroyed.
	1,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_BSEVIL,		// Building prerequisite.
		false,		// Can this be a goodie surprise from a crate?
		false,		// Is a leader type?
		false,		// Only has eight facings?
		true,		// Always use the given name for the vehicle?
		false,		// Is this a typical transport vehicle?
		false,		// Can it be crushed by a heavy vehicle?
		false,		// Can this unit squash infantry?
		false,		// Does this unit harvest Tiberium?
		false,		// Is invisible to radar?
		true,		// Is selectable by player?
		true,		// Can it be a target for attack or move?
		false,		// Is it insignificant (won't be announced)?
		false,		// Is it immune to normal combat damage?
		false,		// Is it equipped with a combat turret?
		false,		// Fires multiple shots in quick succession?
		true,		// Can it be repaired in a repair facility?
		CAC_BUILDABLE_BSEVIL,		// Can the player construct or order this unit?
		false,		// Is there a crew inside?  Unit.destroy plays the death line, explodes, frees the block and kills whatever it was carrying.  It puts nobody on the ground: CrystAlien has no bailing-out crew, and a survivor here is a stock C&C infantryman who screams like one.
		false,		// Does it have a rotating radar dish?
		false,		// Is there an associated firing animation?
		false,		// Must the turret be locked down while moving?
		false,		// Does it lay tracks while moving?
		false,		// Is this a gigundo-rotund-enormous unit?
		false,		// Is the unit's art "chunky" cardinal facing only?
		false,		// Is the unit capable of cloaking?
		false,		// Does the unit have a constant animation?
	-1,					// AMMO:		Number of shots it has (default).
	CAC_BSEVIL_STRENGTH,		// STRENGTH:	Strength (in damage points).
	3,					// SIGHTRANGE:	CrystAlien spread=3 tiles.
	CAC_BSEVIL_COST,			// COST:		Cost to build (Credits).
	1,					// SCENARIO:	Starting availability scenario.
	50,50,				// RISK/RWRD:	Risk/reward rating values.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// OWNABLE:	Ownable by house (bit field).
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_STEEL,			// ARMOR:		Armor type
	SPEED_WINGED,			// MOVE:		Locomotion type.
	(MPHType)8,			// SPEED:		CrystAlien speed 1.
	3,					// ROT:		Half-turn within one tile move.
	0,					// Turret center offset along body centerline.
	MISSION_GUARD		// ORDERS:	Default order to give new unit.
);

/* ---- infantry ---------------------------------------------- */

/*
**	CrystAlien draws infantry from three pose banks (straight/left/right)
**	and walks them through L,S,R,S -- see walkFrames in DoAction_22.as.
**	There are no fire, prone or death sequences, so every other action
**	resolves to the standing pose.  Layout is facing*4 + step.
*/
static int CACInfantryDos[DO_COUNT][3] = {
	1,	1,	4,		// DO_STAND_READY
	1,	1,	4,		// DO_STAND_GUARD
	1,	1,	4,		// DO_PRONE
	0,	4,	4,		// DO_WALK
	1,	1,	4,		// DO_FIRE_WEAPON
	1,	1,	4,		// DO_LIE_DOWN
	0,	4,	4,		// DO_CRAWL
	1,	1,	4,		// DO_GET_UP
	1,	1,	4,		// DO_FIRE_PRONE
	1,	1,	4,		// DO_IDLE1
	1,	1,	4,		// DO_IDLE2
	1,	1,	4,		// DO_ON_GUARD
	1,	1,	4,		// DO_FIGHT_READY
	1,	1,	4,		// DO_PUNCH
	1,	1,	4,		// DO_KICK
	1,	1,	4,		// DO_PUNCH_HIT1
	1,	1,	4,		// DO_PUNCH_HIT2
	1,	1,	4,		// DO_PUNCH_DEATH
	1,	1,	4,		// DO_KICK_HIT1
	1,	1,	4,		// DO_KICK_HIT2
	1,	1,	4,		// DO_KICK_DEATH
	1,	1,	4,		// DO_READY_WEAPON
	1,	1,	4,		// DO_GUN_DEATH
	1,	1,	4,		// DO_EXPLOSION_DEATH
	1,	1,	4,		// DO_EXPLOSION2_DEATH
	1,	1,	4,		// DO_GRENADE_DEATH
	1,	1,	4,		// DO_FIRE_DEATH
	1,	1,	4,		// DO_GESTURE1
	1,	1,	4,		// DO_SALUTE1
	1,	1,	4,		// DO_GESTURE2
	1,	1,	4,		// DO_SALUTE2
	1,	1,	4,		// DO_PULL_GUN
	1,	1,	4,		// DO_PLEAD
	1,	1,	4,		// DO_PLEAD_DEATH
};

InfantryTypeClass const CACInfUAGOOD(
	CAC_SLOT_UAGOOD,			// Infantry type number.
	TXT_E1,				// Translate name number for infantry type.
	"UAGOOD",				// Asset name; faction suffix added at draw time.
	4,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_UAGOOD,		// Building prerequisite.
	false,				// Is this a female type?
	false,				// Is a leader type?
	false,				// Has crawling animation frames?
	false,				// Is this a civilian?
	true,				// Always use the given name for the infantry?
	false,				// Is this a "fraidycat" run-away type infantry?
	false,				// Can this infantry type capture a building?
	false,				// Theater specific graphic image?
	-1,					// Number of shots it has (default).
	&CACInfantryDos[0][0],	// Ptr to the 'DO' table above.
	0,					// Frame of projectile launch.
	0,					// Frame of projectile launch while prone.
	CAC_UAGOOD_STRENGTH,		// Strength of infantry (in damage points).
	3,					// Sight range (CrystAlien spread).
	CAC_UAGOOD_COST,			// Cost of infantry (in credits).
	1,					// Scenario when they first appear.
	50,50,				// Risk/Reward of this infantry unit.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// Who can own this infantry unit.
	(WeaponType)(WEAPON_CAC_FIRST + 0),WEAPON_NONE,
	(MPHType)9			// Maximum speed of infantry (CrystAlien speed 2).
);

InfantryTypeClass const CACInfUAEVIL(
	CAC_SLOT_UAEVIL,			// Infantry type number.
	TXT_E1,				// Translate name number for infantry type.
	"UAEVIL",				// Asset name; faction suffix added at draw time.
	4,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_UAEVIL,		// Building prerequisite.
	false,				// Is this a female type?
	false,				// Is a leader type?
	false,				// Has crawling animation frames?
	false,				// Is this a civilian?
	true,				// Always use the given name for the infantry?
	false,				// Is this a "fraidycat" run-away type infantry?
	false,				// Can this infantry type capture a building?
	false,				// Theater specific graphic image?
	-1,					// Number of shots it has (default).
	&CACInfantryDos[0][0],	// Ptr to the 'DO' table above.
	0,					// Frame of projectile launch.
	0,					// Frame of projectile launch while prone.
	CAC_UAEVIL_STRENGTH,		// Strength of infantry (in damage points).
	3,					// Sight range (CrystAlien spread).
	CAC_UAEVIL_COST,			// Cost of infantry (in credits).
	1,					// Scenario when they first appear.
	50,50,				// Risk/Reward of this infantry unit.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// Who can own this infantry unit.
	(WeaponType)(WEAPON_CAC_FIRST + 0),WEAPON_NONE,
	(MPHType)9			// Maximum speed of infantry (CrystAlien speed 2).
);

InfantryTypeClass const CACInfUBGOOD(
	CAC_SLOT_UBGOOD,			// Infantry type number.
	TXT_E7,				// Translate name number for infantry type.
	"UBGOOD",				// Asset name; faction suffix added at draw time.
	4,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_UBGOOD,		// Building prerequisite.
	false,				// Is this a female type?
	false,				// Is a leader type?
	false,				// Has crawling animation frames?
	false,				// Is this a civilian?
	true,				// Always use the given name for the infantry?
	false,				// Is this a "fraidycat" run-away type infantry?
	true,				// Can this infantry type capture a building?
	false,				// Theater specific graphic image?
	-1,					// Number of shots it has (default).
	&CACInfantryDos[0][0],	// Ptr to the 'DO' table above.
	0,					// Frame of projectile launch.
	0,					// Frame of projectile launch while prone.
	CAC_UBGOOD_STRENGTH,		// Strength of infantry (in damage points).
	1,					// Sight range (CrystAlien spread).
	CAC_UBGOOD_COST,			// Cost of infantry (in credits).
	1,					// Scenario when they first appear.
	50,50,				// Risk/Reward of this infantry unit.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// Who can own this infantry unit.
	WEAPON_NONE,WEAPON_NONE,
	(MPHType)8			// Maximum speed of infantry (CrystAlien speed 1).
);

InfantryTypeClass const CACInfUBEVIL(
	CAC_SLOT_UBEVIL,			// Infantry type number.
	TXT_E7,				// Translate name number for infantry type.
	"UBEVIL",				// Asset name; faction suffix added at draw time.
	4,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_UBEVIL,		// Building prerequisite.
	false,				// Is this a female type?
	false,				// Is a leader type?
	false,				// Has crawling animation frames?
	false,				// Is this a civilian?
	true,				// Always use the given name for the infantry?
	false,				// Is this a "fraidycat" run-away type infantry?
	true,				// Can this infantry type capture a building?
	false,				// Theater specific graphic image?
	-1,					// Number of shots it has (default).
	&CACInfantryDos[0][0],	// Ptr to the 'DO' table above.
	0,					// Frame of projectile launch.
	0,					// Frame of projectile launch while prone.
	CAC_UBEVIL_STRENGTH,		// Strength of infantry (in damage points).
	1,					// Sight range (CrystAlien spread).
	CAC_UBEVIL_COST,			// Cost of infantry (in credits).
	1,					// Scenario when they first appear.
	50,50,				// Risk/Reward of this infantry unit.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// Who can own this infantry unit.
	WEAPON_NONE,WEAPON_NONE,
	(MPHType)8			// Maximum speed of infantry (CrystAlien speed 1).
);

InfantryTypeClass const CACInfUMGOOD(
	CAC_SLOT_UMGOOD,			// Infantry type number.
	TXT_C1,				// Translate name number for infantry type.
	"UMGOOD",				// Asset name; faction suffix added at draw time.
	1,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_UMGOOD,		// Building prerequisite.
	false,				// Is this a female type?
	false,				// Is a leader type?
	false,				// Has crawling animation frames?
	false,				// Is this a civilian?
	true,				// Always use the given name for the infantry?
	false,				// Is this a "fraidycat" run-away type infantry?
	false,				// Can this infantry type capture a building?
	false,				// Theater specific graphic image?
	-1,					// Number of shots it has (default).
	&CACInfantryDos[0][0],	// Ptr to the 'DO' table above.
	0,					// Frame of projectile launch.
	0,					// Frame of projectile launch while prone.
	CAC_UMGOOD_STRENGTH,		// Strength of infantry (in damage points).
	3,					// Sight range (CrystAlien spread).
	CAC_UMGOOD_COST,			// Cost of infantry (in credits).
	1,					// Scenario when they first appear.
	50,50,				// Risk/Reward of this infantry unit.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// Who can own this infantry unit.
	(WeaponType)(WEAPON_CAC_FIRST + 2),(WeaponType)(WEAPON_CAC_FIRST + 4),
	(MPHType)20			// Maximum speed of infantry (CrystAlien speed 6).
);

InfantryTypeClass const CACInfUMEVIL(
	CAC_SLOT_UMEVIL,			// Infantry type number.
	TXT_C1,				// Translate name number for infantry type.
	"UMEVIL",				// Asset name; faction suffix added at draw time.
	1,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_UMEVIL,		// Building prerequisite.
	false,				// Is this a female type?
	false,				// Is a leader type?
	false,				// Has crawling animation frames?
	false,				// Is this a civilian?
	true,				// Always use the given name for the infantry?
	false,				// Is this a "fraidycat" run-away type infantry?
	false,				// Can this infantry type capture a building?
	false,				// Theater specific graphic image?
	-1,					// Number of shots it has (default).
	&CACInfantryDos[0][0],	// Ptr to the 'DO' table above.
	0,					// Frame of projectile launch.
	0,					// Frame of projectile launch while prone.
	CAC_UMEVIL_STRENGTH,		// Strength of infantry (in damage points).
	3,					// Sight range (CrystAlien spread).
	CAC_UMEVIL_COST,			// Cost of infantry (in credits).
	1,					// Scenario when they first appear.
	50,50,				// Risk/Reward of this infantry unit.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// Who can own this infantry unit.
	(WeaponType)(WEAPON_CAC_FIRST + 2),(WeaponType)(WEAPON_CAC_FIRST + 4),
	(MPHType)20			// Maximum speed of infantry (CrystAlien speed 6).
);

InfantryTypeClass const CACInfUNGOOD(
	CAC_SLOT_UNGOOD,			// Infantry type number.
	TXT_C2,				// Translate name number for infantry type.
	"UNGOOD",				// Asset name; faction suffix added at draw time.
	1,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_UNGOOD,		// Building prerequisite.
	false,				// Is this a female type?
	false,				// Is a leader type?
	false,				// Has crawling animation frames?
	true,				// Is this a civilian?
	true,				// Always use the given name for the infantry?
	false,				// Is this a "fraidycat" run-away type infantry?
	false,				// Can this infantry type capture a building?
	false,				// Theater specific graphic image?
	-1,					// Number of shots it has (default).
	&CACInfantryDos[0][0],	// Ptr to the 'DO' table above.
	0,					// Frame of projectile launch.
	0,					// Frame of projectile launch while prone.
	CAC_UNGOOD_STRENGTH,		// Strength of infantry (in damage points).
	1,					// Sight range (CrystAlien spread).
	CAC_UNGOOD_COST,			// Cost of infantry (in credits).
	1,					// Scenario when they first appear.
	50,50,				// Risk/Reward of this infantry unit.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// Who can own this infantry unit.
	WEAPON_NONE,WEAPON_NONE,
	(MPHType)11			// Maximum speed of infantry (CrystAlien speed 3).
);

InfantryTypeClass const CACInfUNEVIL(
	CAC_SLOT_UNEVIL,			// Infantry type number.
	TXT_C2,				// Translate name number for infantry type.
	"UNEVIL",				// Asset name; faction suffix added at draw time.
	1,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_UNEVIL,		// Building prerequisite.
	false,				// Is this a female type?
	false,				// Is a leader type?
	false,				// Has crawling animation frames?
	true,				// Is this a civilian?
	true,				// Always use the given name for the infantry?
	false,				// Is this a "fraidycat" run-away type infantry?
	false,				// Can this infantry type capture a building?
	false,				// Theater specific graphic image?
	-1,					// Number of shots it has (default).
	&CACInfantryDos[0][0],	// Ptr to the 'DO' table above.
	0,					// Frame of projectile launch.
	0,					// Frame of projectile launch while prone.
	CAC_UNEVIL_STRENGTH,		// Strength of infantry (in damage points).
	1,					// Sight range (CrystAlien spread).
	CAC_UNEVIL_COST,			// Cost of infantry (in credits).
	1,					// Scenario when they first appear.
	50,50,				// Risk/Reward of this infantry unit.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// Who can own this infantry unit.
	WEAPON_NONE,WEAPON_NONE,
	(MPHType)11			// Maximum speed of infantry (CrystAlien speed 3).
);

InfantryTypeClass const CACInfUQEVIL(
	CAC_SLOT_UQEVIL,			// Infantry type number.
	TXT_C3,				// Translate name number for infantry type.
	"UQEVIL",				// Asset name; faction suffix added at draw time.
	3,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_UQEVIL,		// Building prerequisite.
	false,				// Is this a female type?
	false,				// Is a leader type?
	false,				// Has crawling animation frames?
	false,				// Is this a civilian?
	true,				// Always use the given name for the infantry?
	false,				// Is this a "fraidycat" run-away type infantry?
	false,				// Can this infantry type capture a building?
	false,				// Theater specific graphic image?
	-1,					// Number of shots it has (default).
	&CACInfantryDos[0][0],	// Ptr to the 'DO' table above.
	0,					// Frame of projectile launch.
	0,					// Frame of projectile launch while prone.
	CAC_UQEVIL_STRENGTH,		// Strength of infantry (in damage points).
	4,					// Sight range (CrystAlien spread).
	CAC_UQEVIL_COST,			// Cost of infantry (in credits).
	1,					// Scenario when they first appear.
	50,50,				// Risk/Reward of this infantry unit.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// Who can own this infantry unit.
	(WeaponType)(WEAPON_CAC_FIRST + 3),(WeaponType)(WEAPON_CAC_FIRST + 11),
	(MPHType)16			// Maximum speed of infantry (CrystAlien speed 5).
);

InfantryTypeClass const CACInfUJGOOD(
	CAC_SLOT_UJGOOD,			// Infantry type number.
	TXT_C4,				// Translate name number for infantry type.
	"UJGOOD",				// Asset name; faction suffix added at draw time.
	1,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_UJGOOD,		// Building prerequisite.
	false,				// Is this a female type?
	false,				// Is a leader type?
	false,				// Has crawling animation frames?
	false,				// Is this a civilian?
	true,				// Always use the given name for the infantry?
	false,				// Is this a "fraidycat" run-away type infantry?
	false,				// Can this infantry type capture a building?
	false,				// Theater specific graphic image?
	-1,					// Number of shots it has (default).
	&CACInfantryDos[0][0],	// Ptr to the 'DO' table above.
	0,					// Frame of projectile launch.
	0,					// Frame of projectile launch while prone.
	CAC_UJGOOD_STRENGTH,		// Strength of infantry (in damage points).
	1,					// Sight range (CrystAlien spread).
	CAC_UJGOOD_COST,			// Cost of infantry (in credits).
	1,					// Scenario when they first appear.
	50,50,				// Risk/Reward of this infantry unit.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// Who can own this infantry unit.
	WEAPON_NONE,WEAPON_NONE,
	(MPHType)8			// Maximum speed of infantry (CrystAlien speed 0).
);

InfantryTypeClass const CACInfUJEVIL(
	CAC_SLOT_UJEVIL,			// Infantry type number.
	TXT_C4,				// Translate name number for infantry type.
	"UJEVIL",				// Asset name; faction suffix added at draw time.
	1,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_UJEVIL,		// Building prerequisite.
	false,				// Is this a female type?
	false,				// Is a leader type?
	false,				// Has crawling animation frames?
	false,				// Is this a civilian?
	true,				// Always use the given name for the infantry?
	false,				// Is this a "fraidycat" run-away type infantry?
	false,				// Can this infantry type capture a building?
	false,				// Theater specific graphic image?
	-1,					// Number of shots it has (default).
	&CACInfantryDos[0][0],	// Ptr to the 'DO' table above.
	0,					// Frame of projectile launch.
	0,					// Frame of projectile launch while prone.
	CAC_UJEVIL_STRENGTH,		// Strength of infantry (in damage points).
	1,					// Sight range (CrystAlien spread).
	CAC_UJEVIL_COST,			// Cost of infantry (in credits).
	1,					// Scenario when they first appear.
	50,50,				// Risk/Reward of this infantry unit.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// Who can own this infantry unit.
	WEAPON_NONE,WEAPON_NONE,
	(MPHType)8			// Maximum speed of infantry (CrystAlien speed 0).
);

/* ---- aircraft ---------------------------------------------- */

AircraftTypeClass const CACAirUCGOOD(
	CAC_SLOT_UCGOOD,		// What kind of aircraft is this.
	TXT_HELI,				// Translated text number for aircraft.
	"UCGOOD",				// Asset name; faction suffix added at draw time.
	6,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_UCGOOD,		// Building prerequisite.
		false,			// Is a leader type?
		false,			// Does it fire a pair of shots in quick succession?
		false,			// Is this a typical transport vehicle?
		false,			// Fixed wing aircraft?
		false,			// Equipped with a rotor?
		false,			// Custom rotor sets for each facing?
		false,			// Can this aircraft land on clear terrain?
		false,			// Can the aircraft be crushed by a tracked vehicle?
		false,			// Is it invisible on radar?
		true,			// Can the player select it so as to give it orders?
		true,			// Can it be assigned as a target for attack.
		false,			// Is it insignificant (won't be announced)?
		false,			// Is it immune to normal combat damage?
		false,			// Theater specific graphic image?
		true,			// Can it be repaired in a repair facility?
		CAC_BUILDABLE_UCGOOD,			// Can the player construct or order this unit?
		false,			// Is there a crew inside?  Unit.destroy plays the death line, explodes, frees the block and kills whatever it was carrying.  It puts nobody on the ground: CrystAlien has no bailing-out crew, and a survivor here is a stock C&C infantryman who screams like one.
	-1,					// Number of shots it has (default).
	CAC_UCGOOD_STRENGTH,		// The strength of this unit.
	3,					// Reveal radius (CrystAlien spread).
	CAC_UCGOOD_COST,			// Credit cost to construct.
	1,					// The scenario this becomes available.
	50,50,				// Risk, reward when calculating AI.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// Who can own this aircraft type.
	(WeaponType)(WEAPON_CAC_FIRST + 10),WEAPON_NONE,
	ARMOR_ALUMINUM,			// Armor type of this aircraft.
	(MPHType)16,			// Maximum speed (CrystAlien speed 5).
	5,					// Rate of turn.
	MISSION_GUARD		// Default mission for aircraft.
);

AircraftTypeClass const CACAirUCEVIL(
	CAC_SLOT_UCEVIL,		// What kind of aircraft is this.
	TXT_HELI,				// Translated text number for aircraft.
	"UCEVIL",				// Asset name; faction suffix added at draw time.
	6,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_UCEVIL,		// Building prerequisite.
		false,			// Is a leader type?
		false,			// Does it fire a pair of shots in quick succession?
		false,			// Is this a typical transport vehicle?
		false,			// Fixed wing aircraft?
		false,			// Equipped with a rotor?
		false,			// Custom rotor sets for each facing?
		false,			// Can this aircraft land on clear terrain?
		false,			// Can the aircraft be crushed by a tracked vehicle?
		false,			// Is it invisible on radar?
		true,			// Can the player select it so as to give it orders?
		true,			// Can it be assigned as a target for attack.
		false,			// Is it insignificant (won't be announced)?
		false,			// Is it immune to normal combat damage?
		false,			// Theater specific graphic image?
		true,			// Can it be repaired in a repair facility?
		CAC_BUILDABLE_UCEVIL,			// Can the player construct or order this unit?
		false,			// Is there a crew inside?  Unit.destroy plays the death line, explodes, frees the block and kills whatever it was carrying.  It puts nobody on the ground: CrystAlien has no bailing-out crew, and a survivor here is a stock C&C infantryman who screams like one.
	-1,					// Number of shots it has (default).
	CAC_UCEVIL_STRENGTH,		// The strength of this unit.
	3,					// Reveal radius (CrystAlien spread).
	CAC_UCEVIL_COST,			// Credit cost to construct.
	1,					// The scenario this becomes available.
	50,50,				// Risk, reward when calculating AI.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// Who can own this aircraft type.
	(WeaponType)(WEAPON_CAC_FIRST + 10),WEAPON_NONE,
	ARMOR_ALUMINUM,			// Armor type of this aircraft.
	(MPHType)16,			// Maximum speed (CrystAlien speed 5).
	5,					// Rate of turn.
	MISSION_GUARD		// Default mission for aircraft.
);

AircraftTypeClass const CACAirUGGOOD(
	CAC_SLOT_UGGOOD,		// What kind of aircraft is this.
	TXT_ORCA,				// Translated text number for aircraft.
	"UGGOOD",				// Asset name; faction suffix added at draw time.
	5,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_UGGOOD,		// Building prerequisite.
		false,			// Is a leader type?
		false,			// Does it fire a pair of shots in quick succession?
		false,			// Is this a typical transport vehicle?
		false,			// Fixed wing aircraft?
		false,			// Equipped with a rotor?
		false,			// Custom rotor sets for each facing?
		false,			// Can this aircraft land on clear terrain?
		false,			// Can the aircraft be crushed by a tracked vehicle?
		false,			// Is it invisible on radar?
		true,			// Can the player select it so as to give it orders?
		true,			// Can it be assigned as a target for attack.
		false,			// Is it insignificant (won't be announced)?
		false,			// Is it immune to normal combat damage?
		false,			// Theater specific graphic image?
		true,			// Can it be repaired in a repair facility?
		CAC_BUILDABLE_UGGOOD,			// Can the player construct or order this unit?
		false,			// Is there a crew inside?  Unit.destroy plays the death line, explodes, frees the block and kills whatever it was carrying.  It puts nobody on the ground: CrystAlien has no bailing-out crew, and a survivor here is a stock C&C infantryman who screams like one.
	10,					// Number of shots it has (default).
	CAC_UGGOOD_STRENGTH,		// The strength of this unit.
	4,					// Reveal radius (CrystAlien spread).
	CAC_UGGOOD_COST,			// Credit cost to construct.
	1,					// The scenario this becomes available.
	50,50,				// Risk, reward when calculating AI.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// Who can own this aircraft type.
	(WeaponType)(WEAPON_CAC_FIRST + 5),WEAPON_NONE,
	ARMOR_ALUMINUM,			// Armor type of this aircraft.
	(MPHType)41,			// Maximum speed (CrystAlien speed 10).
	13,					// Rate of turn.
	MISSION_GUARD		// Default mission for aircraft.
);

AircraftTypeClass const CACAirUGEVIL(
	CAC_SLOT_UGEVIL,		// What kind of aircraft is this.
	TXT_ORCA,				// Translated text number for aircraft.
	"UGEVIL",				// Asset name; faction suffix added at draw time.
	5,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_UGEVIL,		// Building prerequisite.
		false,			// Is a leader type?
		false,			// Does it fire a pair of shots in quick succession?
		false,			// Is this a typical transport vehicle?
		false,			// Fixed wing aircraft?
		false,			// Equipped with a rotor?
		false,			// Custom rotor sets for each facing?
		false,			// Can this aircraft land on clear terrain?
		false,			// Can the aircraft be crushed by a tracked vehicle?
		false,			// Is it invisible on radar?
		true,			// Can the player select it so as to give it orders?
		true,			// Can it be assigned as a target for attack.
		false,			// Is it insignificant (won't be announced)?
		false,			// Is it immune to normal combat damage?
		false,			// Theater specific graphic image?
		true,			// Can it be repaired in a repair facility?
		CAC_BUILDABLE_UGEVIL,			// Can the player construct or order this unit?
		false,			// Is there a crew inside?  Unit.destroy plays the death line, explodes, frees the block and kills whatever it was carrying.  It puts nobody on the ground: CrystAlien has no bailing-out crew, and a survivor here is a stock C&C infantryman who screams like one.
	10,					// Number of shots it has (default).
	CAC_UGEVIL_STRENGTH,		// The strength of this unit.
	4,					// Reveal radius (CrystAlien spread).
	CAC_UGEVIL_COST,			// Credit cost to construct.
	1,					// The scenario this becomes available.
	50,50,				// Risk, reward when calculating AI.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// Who can own this aircraft type.
	(WeaponType)(WEAPON_CAC_FIRST + 5),WEAPON_NONE,
	ARMOR_ALUMINUM,			// Armor type of this aircraft.
	(MPHType)41,			// Maximum speed (CrystAlien speed 10).
	13,					// Rate of turn.
	MISSION_GUARD		// Default mission for aircraft.
);

AircraftTypeClass const CACAirUHGOOD(
	CAC_SLOT_UHGOOD,		// What kind of aircraft is this.
	TXT_TRANS,				// Translated text number for aircraft.
	"UHGOOD",				// Asset name; faction suffix added at draw time.
	6,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_UHGOOD,		// Building prerequisite.
		false,			// Is a leader type?
		false,			// Does it fire a pair of shots in quick succession?
		true,			// Is this a typical transport vehicle?
		false,			// Fixed wing aircraft?
		false,			// Equipped with a rotor?
		false,			// Custom rotor sets for each facing?
		true,			// Can this aircraft land on clear terrain?
		false,			// Can the aircraft be crushed by a tracked vehicle?
		false,			// Is it invisible on radar?
		true,			// Can the player select it so as to give it orders?
		true,			// Can it be assigned as a target for attack.
		false,			// Is it insignificant (won't be announced)?
		false,			// Is it immune to normal combat damage?
		false,			// Theater specific graphic image?
		true,			// Can it be repaired in a repair facility?
		CAC_BUILDABLE_UHGOOD,			// Can the player construct or order this unit?
		false,			// Is there a crew inside?  Unit.destroy plays the death line, explodes, frees the block and kills whatever it was carrying.  It puts nobody on the ground: CrystAlien has no bailing-out crew, and a survivor here is a stock C&C infantryman who screams like one.
	-1,					// Number of shots it has (default).
	CAC_UHGOOD_STRENGTH,		// The strength of this unit.
	4,					// Reveal radius (CrystAlien spread).
	CAC_UHGOOD_COST,			// Credit cost to construct.
	1,					// The scenario this becomes available.
	50,50,				// Risk, reward when calculating AI.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// Who can own this aircraft type.
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_ALUMINUM,			// Armor type of this aircraft.
	(MPHType)20,			// Maximum speed (CrystAlien speed 6).
	6,					// Rate of turn.
	MISSION_GUARD		// Default mission for aircraft.
);

AircraftTypeClass const CACAirUHEVIL(
	CAC_SLOT_UHEVIL,		// What kind of aircraft is this.
	TXT_TRANS,				// Translated text number for aircraft.
	"UHEVIL",				// Asset name; faction suffix added at draw time.
	6,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_UHEVIL,		// Building prerequisite.
		false,			// Is a leader type?
		false,			// Does it fire a pair of shots in quick succession?
		true,			// Is this a typical transport vehicle?
		false,			// Fixed wing aircraft?
		false,			// Equipped with a rotor?
		false,			// Custom rotor sets for each facing?
		true,			// Can this aircraft land on clear terrain?
		false,			// Can the aircraft be crushed by a tracked vehicle?
		false,			// Is it invisible on radar?
		true,			// Can the player select it so as to give it orders?
		true,			// Can it be assigned as a target for attack.
		false,			// Is it insignificant (won't be announced)?
		false,			// Is it immune to normal combat damage?
		false,			// Theater specific graphic image?
		true,			// Can it be repaired in a repair facility?
		CAC_BUILDABLE_UHEVIL,			// Can the player construct or order this unit?
		false,			// Is there a crew inside?  Unit.destroy plays the death line, explodes, frees the block and kills whatever it was carrying.  It puts nobody on the ground: CrystAlien has no bailing-out crew, and a survivor here is a stock C&C infantryman who screams like one.
	-1,					// Number of shots it has (default).
	CAC_UHEVIL_STRENGTH,		// The strength of this unit.
	4,					// Reveal radius (CrystAlien spread).
	CAC_UHEVIL_COST,			// Credit cost to construct.
	1,					// The scenario this becomes available.
	50,50,				// Risk, reward when calculating AI.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// Who can own this aircraft type.
	WEAPON_NONE,WEAPON_NONE,
	ARMOR_ALUMINUM,			// Armor type of this aircraft.
	(MPHType)20,			// Maximum speed (CrystAlien speed 6).
	6,					// Rate of turn.
	MISSION_GUARD		// Default mission for aircraft.
);

AircraftTypeClass const CACAirUPGOOD(
	CAC_SLOT_UPGOOD,		// What kind of aircraft is this.
	TXT_C17,				// Translated text number for aircraft.
	"UPGOOD",				// Asset name; faction suffix added at draw time.
	3,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_UPGOOD,		// Building prerequisite.
		false,			// Is a leader type?
		false,			// Does it fire a pair of shots in quick succession?
		false,			// Is this a typical transport vehicle?
		false,			// Fixed wing aircraft?
		false,			// Equipped with a rotor?
		false,			// Custom rotor sets for each facing?
		true,			// Can this aircraft land on clear terrain?
		false,			// Can the aircraft be crushed by a tracked vehicle?
		false,			// Is it invisible on radar?
		true,			// Can the player select it so as to give it orders?
		true,			// Can it be assigned as a target for attack.
		false,			// Is it insignificant (won't be announced)?
		false,			// Is it immune to normal combat damage?
		false,			// Theater specific graphic image?
		true,			// Can it be repaired in a repair facility?
		CAC_BUILDABLE_UPGOOD,			// Can the player construct or order this unit?
		false,			// Is there a crew inside?  Unit.destroy plays the death line, explodes, frees the block and kills whatever it was carrying.  It puts nobody on the ground: CrystAlien has no bailing-out crew, and a survivor here is a stock C&C infantryman who screams like one.
	-1,					// Number of shots it has (default).
	CAC_UPGOOD_STRENGTH,		// The strength of this unit.
	4,					// Reveal radius (CrystAlien spread).
	CAC_UPGOOD_COST,			// Credit cost to construct.
	1,					// The scenario this becomes available.
	50,50,				// Risk, reward when calculating AI.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// Who can own this aircraft type.
	(WeaponType)(WEAPON_CAC_FIRST + 9),WEAPON_NONE,
	ARMOR_ALUMINUM,			// Armor type of this aircraft.
	(MPHType)41,			// Maximum speed (CrystAlien speed 10).
	13,					// Rate of turn.
	MISSION_GUARD		// Default mission for aircraft.
);

AircraftTypeClass const CACAirUPEVIL(
	CAC_SLOT_UPEVIL,		// What kind of aircraft is this.
	TXT_C17,				// Translated text number for aircraft.
	"UPEVIL",				// Asset name; faction suffix added at draw time.
	3,					// Build level (depth in CrystAlien's tree).
	CAC_PREREQ_UPEVIL,		// Building prerequisite.
		false,			// Is a leader type?
		false,			// Does it fire a pair of shots in quick succession?
		false,			// Is this a typical transport vehicle?
		false,			// Fixed wing aircraft?
		false,			// Equipped with a rotor?
		false,			// Custom rotor sets for each facing?
		true,			// Can this aircraft land on clear terrain?
		false,			// Can the aircraft be crushed by a tracked vehicle?
		false,			// Is it invisible on radar?
		true,			// Can the player select it so as to give it orders?
		true,			// Can it be assigned as a target for attack.
		false,			// Is it insignificant (won't be announced)?
		false,			// Is it immune to normal combat damage?
		false,			// Theater specific graphic image?
		true,			// Can it be repaired in a repair facility?
		CAC_BUILDABLE_UPEVIL,			// Can the player construct or order this unit?
		false,			// Is there a crew inside?  Unit.destroy plays the death line, explodes, frees the block and kills whatever it was carrying.  It puts nobody on the ground: CrystAlien has no bailing-out crew, and a survivor here is a stock C&C infantryman who screams like one.
	-1,					// Number of shots it has (default).
	CAC_UPEVIL_STRENGTH,		// The strength of this unit.
	4,					// Reveal radius (CrystAlien spread).
	CAC_UPEVIL_COST,			// Credit cost to construct.
	1,					// The scenario this becomes available.
	50,50,				// Risk, reward when calculating AI.
	HOUSEF_GOOD|HOUSEF_BAD|HOUSEF_MULTI1|HOUSEF_MULTI2|HOUSEF_MULTI3|HOUSEF_MULTI4|HOUSEF_MULTI5|HOUSEF_MULTI6,	// Who can own this aircraft type.
	(WeaponType)(WEAPON_CAC_FIRST + 9),WEAPON_NONE,
	ARMOR_ALUMINUM,			// Armor type of this aircraft.
	(MPHType)41,			// Maximum speed (CrystAlien speed 10).
	13,					// Rate of turn.
	MISSION_GUARD		// Default mission for aircraft.
);
