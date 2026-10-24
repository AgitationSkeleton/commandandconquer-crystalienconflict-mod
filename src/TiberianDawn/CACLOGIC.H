/*
**	CACLOGIC.H -- CrystAlien Conflict behaviour hooks.
**
**	Declarations for the rules Tiberian Dawn has no equivalent for. The rule
**	each one implements is documented at its definition in CACLOGIC.CPP,
**	alongside the CrystAlien source it came from.
*/

#ifndef CACLOGIC_H
#define CACLOGIC_H

typedef enum CACVoiceEvent {
	CAC_VOICE_COMPLY = 0,		// "<type>_comply"   -- order acknowledged
	CAC_VOICE_ACTION1,			// "<type>_action_1" -- firing / working
	CAC_VOICE_ACTION2,
	CAC_VOICE_ACTION3,
	CAC_VOICE_PAIN1,				// "<type>_pain_1"   -- taking damage
	CAC_VOICE_PAIN2,
	CAC_VOICE_DIE,					// "<type>_die"

	CAC_VOICE_COUNT
} CACVoiceEvent;

typedef enum CACEngineerAction {
	CAC_ENGINEER_NOTHING = 0,
	CAC_ENGINEER_CAPTURE,		// doInfiltrate() -- enemy target
	CAC_ENGINEER_REPAIR			// doRepair()     -- damaged friendly target
} CACEngineerAction;

HousesType CAC_Side(HousesType house);

VocType CAC_Voice(RTTIType rtti, int type, HousesType owner, CACVoiceEvent event);
bool CAC_Speak(TechnoClass *obj, CACVoiceEvent event);
void CAC_Death_Effect(TechnoClass *obj);
void CAC_Death_Shake(TechnoClass const *obj, TechnoClass const *source);

int CAC_Type_Index(TechnoClass const *obj);
int CAC_Type_Cap(RTTIType rtti, int type);
void CAC_Log_Cap(char const *asset, HouseClass const *house, RTTIType rtti, int type);
int CAC_Owned_Count(HouseClass const *house, RTTIType rtti, int type);
bool CAC_At_Type_Cap(HouseClass const *house, RTTIType rtti, int type);
void CAC_Cancel_Unbuildable(HouseClass * house);

StructType CAC_Deploys_Into(UnitType type, HousesType owner);
UnitType CAC_Undeploys_Into(StructType type);
bool CAC_Is_Stock_Structure(StructType type);
bool CAC_Is_Stock_Refinery(StructType type);
InfantryType CAC_Crew_Type(TechnoTypeClass const *type);
/*
**	Construction.construct advances `speed` units per frame towards
**	`constructionTime`, which equals the item's cost, at 23 frames per second.
*/
#define CAC_BUILD_SPEED	8
#define CAC_BUILD_FPS		23

int CAC_Time_To_Build(TechnoTypeClass const *type, int cost);

/*
**	Idle turret sweep: one of the sixteen drawn facings every fourth source
**	frame, reversing every pi*100 of them -- converted to 15 ticks per second
**	against the source game's 23, and to the 256-unit facing circle.
*/
#define CAC_SPIN_STEP			(256 / 16)
#define CAC_SPIN_TICKS			3
#define CAC_SPIN_SWEEP_TICKS	205

void CAC_Idle_Turret_Spin(BuildingClass *building);
bool CAC_Has_Host(RTTIType rtti, int type);
StructType CAC_Host_Of(RTTIType rtti, int type);
bool CAC_Hosts_Unit(StructType building, RTTIType rtti, int type);
StructType CAC_AI_Role(HouseClass const *house, StructType stock);
StructType CAC_AI_Extra(HouseClass const *house);
bool CAC_Is_Miner(RTTIType rtti, int type);

/*
**	How long the computer's units are allowed to stand on one cell with an
**	errand outstanding.  CrystAlien's UnitNav counts `still` and drops the path
**	at a hundred of its frames; a hundred against fifteen ticks to its
**	twenty-three is about sixty-five.  Three of those and the order itself is
**	what is wrong, so it is taken away.  See CAC_Idle_Watch.
*/
#define CAC_STILL_REPATH 65
#define CAC_STILL_GIVEUP 195

unsigned CAC_Build_Level(TechnoTypeClass const *type);
bool CAC_Built_Counts_As(long built, long wanted);
void CAC_Logging_Recheck(void);
int CAC_Mix_Type(RTTIType rtti, int type, HousesType owner, bool locked);
bool CAC_INI_Has_Trigger(char const * line);
bool CAC_INI_Base_Has_Cell(char * buffer, CELL cell);
bool CAC_HQ_Unsellable(BuildingClass const *b);
bool CAC_Covered_By_Other(CELL cell, BuildingClass const *me);
void CAC_Idle_Watch(HouseClass *house);

bool CAC_Has_Radar(HouseClass const *house);
bool CAC_Has_Satellite(HouseClass const *house);
/*
**	CAC_ABI_PROBE -- catch a call that does not give esi back.
**
**	Two crashes now, at two different sites, both the same shape: a function keeps
**	`this` in esi across a call, and esi comes back as 0 or 1.  esi is callee-saved,
**	so something below is returning without restoring it, and the two faults were
**	nowhere near each other -- GScreenClass::Render and TabClass::Draw_It -- which
**	means the culprit is shared and somewhere under Draw_It.
**
**	Guessing has had one go at this already and was wrong.  This asks the machine
**	instead: read esi, make the call, read it again, and say so if it moved.  Only
**	the report is conditional -- the call happens either way, so a build with these
**	in behaves exactly like one without.
**
**	32-bit MSVC only, which is what this project builds.  The reads are inline asm
**	rather than intrinsics because there is no intrinsic for "the current esi", and
**	MSVC will not cache anything in a register across an __asm block that reads one,
**	which is precisely the property needed here.
*/
#define CAC_ABI_PROBE(label, call)                                            	do {                                                                        		unsigned long _cac_before = 0, _cac_after = 0;                           		__asm { mov _cac_before, esi }                                           		call;                                                                     		__asm { mov _cac_after, esi }                                            		if (_cac_before != _cac_after) {                                         			CAC_Log_Abi((label), _cac_before, _cac_after);                        		}                                                                         	} while (0)

void CAC_Log_Abi(char const * what, unsigned long before, unsigned long after);

/*
**	CAC_ABI_SHIELD -- make one call unable to damage its caller.
**
**	Containment, not a cure, and worth being plain about which.
**
**	The probe proved PowerClass::Draw_It returns with esi holding 1 instead of the
**	`this` it was given.  What it has not yet found is the instruction responsible,
**	and two crash sites were being fed by it:
**
**	    SidebarClass::Draw_It calls PowerClass::Draw_It *before* it pushes esi, so
**	    it has no saved copy to put back, and the damaged register goes straight up
**	    to TabClass::Draw_It and GScreenClass::Render -- which is exactly where the
**	    faults landed, both of them writing through a `this` of 0 or 1.
**
**	So the three callee-saved registers are saved and restored around that one call.
**	Whatever it does to them stops there.  The probe stays in place, still reporting,
**	because the underlying violation is still somewhere below and will still be
**	wrong for anything else that calls into it.
**
**	Safe with an ebp frame, which these functions have: locals are addressed through
**	ebp, so esp moving between the two blocks does not disturb them.
*/
#define CAC_ABI_SHIELD(label, call)                                           	do {                                                                        		unsigned long _cac_before = 0, _cac_after = 0;                           		__asm { mov _cac_before, esi }                                           		__asm { push esi }                                                       		__asm { push edi }                                                       		__asm { push ebx }                                                       		call;                                                                     		__asm { pop ebx }                                                        		__asm { pop edi }                                                        		__asm { pop esi }                                                        		__asm { mov _cac_after, esi }                                            		if (_cac_before != _cac_after) {                                         			CAC_Log_Abi((label), _cac_before, _cac_after);                        		}                                                                         	} while (0)


/*@THow far from its headquarters a computer will put a Driller, in cells. */
/*
**	How far from its headquarters a computer will put a Driller, in cells.
**
**	CrystAlien puts no distance rule on this at all -- Placer.think exempts
**	anything drawing no power from adjacency, and the Driller draws none, so a
**	player may site one anywhere on the map.  This is an AI-only restraint and
**	exists for one reason: without it the computer's first Driller of the round
**	goes to whichever crystal is nearest in absolute terms, which can be across
**	the map on somebody else's doorstep, in the opening minute.
**
**	So it starts close and opens up, and an uplink lifts it outright -- see
**	CAC_Driller_Range.
*/
#define CAC_DRILLER_RANGE		24	/* cells, at the start of a round */
#define CAC_OUTPOST_REACH		4	/* cells searched around an outlying building */
#define CAC_FLAG_GUARD_REACH	5	/* cells searched around the flag for a gun */
#define CAC_DEATH_SHAKE		2	/* explode()'s camera.shake, in Tiberian Dawn's units */
#define CAC_SURVIVOR_REACH	3	/* cells a crew will walk to get clear of the rubble */
#define CAC_FLAG_SITE_REACH	6	/* cells searched beside an HQ for somewhere to plant a flag */
#define CAC_SCORCH_MEMORY	(TICKS_PER_MINUTE * 4)	/* how long a scorch waits for a baseplate to lift */
#define CAC_PIZZA_STIPEND	400	/* credits a minute in Pizza Mode, so nobody is ever mined out */
#define CAC_UJ_SOURCE_COST	5000	/* UnitStats' own price for UJ, which is also its build clock */
#define CAC_PIZZA_REVEAL	12	/* cells of shroud lifted around a delivered pizza -- five was
									   not enough to find it by, and the buyer paying fifty thousand
									   credits should not then have to search the map for it */
#define CAC_PIZZA_CAMP	4	/* armed vehicles posted on a rival's pizza */
#define CAC_RAID_PATIENCE	10	/* seconds a raider may fail to get closer before giving up */
#define CAC_RAID_PROGRESS	0x0080	/* leptons of closing that count as progress */
#define CAC_POWER_MARGIN		2	/* BuildingStats.powerMargin, in cells */
#define CAC_DRILLER_WIDEN_MINUTES	3	/* how often the allowance grows */
#define CAC_DRILLER_WIDEN_STEP	8	/* cells added each time */
int CAC_Driller_Range(HouseClass const * house);
bool CAC_Needs_Crystal_Site(StructType type);
bool CAC_Crystal_Cell_Ok(CELL cell);
bool CAC_Crystal_Spread_Ok(CELL cell);
bool CAC_Ground_Ok(StructType type, CELL cell);
bool CAC_Crystal_Site_Ok(StructType type, CELL origin);
COORDINATE CAC_Find_Crystal_Site(HouseClass const * house, StructType type);
CELL CAC_Outpost_Site(HouseClass const *house, BuildingClass *building);
bool CAC_Anchors_Build(StructType type);
bool CAC_Power_Adjacency_Ok(StructType type, HousesType house, short const *list, CELL cell);
bool CAC_Is_Heap_Slot(void const * ptr);
bool CAC_Is_Live_Object(void const * ptr);
void CAC_Log_Stale(int cell, char const * where, void const * ptr);
void CAC_Log_Gun_Idle(BuildingClass const * b);
bool CAC_Has_Uplink(HouseClass const *house, bool evil);
bool CAC_Uplink_Takes_Ion(HouseClass const * house);
bool CAC_Uplink_Takes_Nuke(HouseClass const * house);
bool CAC_Superweapon_Is_Ours(HouseClass const * house);
int CAC_Queue_Of_Host(StructType building);
VocType CAC_Stock_Sound(TechnoClass const *object, VocType voc);
SmudgeType CAC_Plate_For(int size, bool evil, HousesType owner);
TechnoTypeClass const *CAC_Host_Suggestion(HouseClass *house, StructType building, RTTIType tobuild);
bool CAC_Is_CrystAlien_House(HouseClass const *house);
VocType CAC_Announce(VoxType voice);
void CAC_Interface_Sound(VocType voc, COORDINATE coord = 0);
void CAC_Order_Sound(TechnoClass const *object, MissionType mission);
bool CAC_Can_Attack(TechnoClass const * object);
bool CAC_Scenery_Yields(ObjectClass const *found);
bool CAC_Building_Unenterable(BuildingClass const *b);
BuildingClass * CAC_Building_Over(CELL cell);
void CAC_Options_Changed(HouseClass * house);
void CAC_Select_Sound(ObjectClass const *object);
void CAC_Power_Klaxon(HouseClass *house);
void CAC_Building_Hit(BuildingClass *building);
void CAC_Weapon_Noise(TechnoClass *object);
bool CAC_Single_Target(TechnoClass const *shooter);
BulletType CAC_Bullet_For(TechnoClass const *object, BulletType fires);
bool CAC_Snap_Fire(TechnoClass const *object);
bool CAC_Has_Factory(HouseClass const *house);
bool CAC_Place_Code(int code, bool evil, CELL where, HousesType owner);
bool CAC_Laser_Colours(TechnoClass const *object, int &edge, int &core);
bool CAC_Owns_Its_Sounds(TechnoClass const *object);
StructType CAC_Exit_Style(StructType building);
CELL CAC_Exit_Cell(BuildingClass const * bld, TechnoClass const * obj);
int CAC_Factory_Index(RTTIType rtti, int id);
bool CAC_Builds_Any(StructType building, RTTIType rtti);
bool CAC_Is_HQ(StructType type);
bool CAC_Unit_From_HQ(UnitType type);
int CAC_Survivor_Count(StructType type);
bool CAC_Spawn_Survivor(StructType type, int index, HousesType owner, CELL cell);
int CAC_Spawn_Survivors(BuildingClass *building);
void CAC_Building_Revenue(BuildingClass *building);

/*
**	CrystAlien starts every base with fifty points of power in hand; see
**	CAC_Power_State for what that allowance does.
*/
#define CAC_POWER_ALLOWANCE 50

typedef enum {
	CAC_POWER_FULL,
	CAC_POWER_LOW,
	CAC_POWER_OFF
} CACPowerState;

/*
**	pain() swaps to the damaged still and lights the smoke at `healthPerc <= 25`.
**	Health_Ratio counts 0x100 to the full, so a quarter is 0x40.
*/
#define CAC_DAMAGE_RATIO 0x0040

bool CAC_Shows_Damage(BuildingClass const *building);
void CAC_Damage_Smoke(BuildingClass *building);
void CAC_Unit_Smoke(TechnoClass *obj);
void CAC_Satellite_Shroud(HouseClass *house);
bool CAC_Crate_Reachable(CELL cell);
bool CAC_Crate_Ground_Ok(CELL cell);
void CAC_Credit_Harvest(HouseClass *house, int amount);
int CAC_Drawn_Rows_Above(BuildingClass const *b);
bool CAC_Overhang_Yields(CELL cell);
void CAC_Sighted_For_Real(CELL cell, HouseClass const *house);
void CAC_Reset_Uplink_Shroud(void);
void CAC_Reset_Scouting(void);
void CAC_Reset_Scorches(void);
void CAC_Defer_Scorch(CELL cell, SmudgeType type, unsigned char data);
bool CAC_Pending_Scorch(CELL cell, SmudgeType &type, unsigned char &data);
bool CAC_Has_Scouted(HouseClass const *house, CELL cell);
void CAC_AI_Scout(HouseClass *house);
void CAC_Power_Sums(HouseClass const *house, int &charge, int &drain);
int CAC_Power_Charge(HouseClass const *house);
int CAC_Power_Drain(HouseClass const *house);
int CAC_Power_Net(HouseClass const *house);
CACPowerState CAC_Power_State(HouseClass const *house);
/*
**	"this.camera.shake += 200" in source frames at 23fps.  Shake_The_Screen
**	doubles whatever it is handed, so half the tick-scaled figure gets there.
*/
#define CAC_SUPERWEAPON_SHAKE	((200 * 15 / 23) / 2)

int CAC_Crystal_Floor(TechnoClass const *miner);
char const *CAC_Special_Asset(HouseClass const *house, SpecialWeaponType id, char const *stock);
bool CAC_In_Infantry_Tab(RTTIType rtti, int id);
void CAC_AI_Collect(HouseClass *house);
void CAC_AI_Aggression(HouseClass * house);
void CAC_AI_Sortie(HouseClass * house);
void CAC_AI_Raid(HouseClass * house);
void CAC_AI_Patrol(HouseClass * house);
void CAC_AI_Repair(HouseClass * house);
void CAC_AI_Flag(HouseClass * house);
void CAC_AI_Flag_Guard(HouseClass * house);
CELL CAC_Flag_Site(BuildingClass const *hq);
bool CAC_On_Crate_Errand(FootClass const * foot);
CELL CAC_Flag_Cell(HouseClass const * house);
bool CAC_Guarding_Flag(FootClass const * foot, HouseClass const * house);
CELL CAC_Flag_Defence_Site(HouseClass const *house, BuildingClass *building);
void CAC_AI_Census(HouseClass * house);
bool CAC_AI_Under_Attack(HouseClass const * house);
void CAC_AI_Airlift(HouseClass *house);
void CAC_AI_Infiltrate(HouseClass *house);
UrgencyType CAC_AI_Power_Urgency(HouseClass const *house);
bool CAC_AI_Can_Power(HouseClass const *house, BuildingTypeClass const *type);
int CAC_AI_Infantry_Value(HouseClass const *house, int index);
AircraftType CAC_AI_Aircraft(HouseClass const *house);
bool CAC_Power_Off(HouseClass const *house);
bool CAC_Power_Low(HouseClass const *house);

/*
**	Level.cashUp multiplies everything it is handed by five before it reaches
**	the player's account, so every income figure in the source is a fifth of
**	what it actually pays.  See CAC_Bail_Credits.
*/
/*
**	One building's bit in a house's BScan.  The mask is sixty-four wide and the
**	structure enum is longer than that -- the civilian slots run past the end --
**	so a raw shift there is undefined, and on x86 it wraps and sets some other
**	building's bit, quietly granting a prerequisite nobody ever built.
*/
inline unsigned long long CAC_Scan_Bit(int type)
{
	return((type >= 0 && type < 64) ? (1ULL << type) : 0ULL);
}

/*
**	UnitNav pauses a miner on the pad for twenty of the source's frames before
**	sending it back out; that is thirteen ticks here.
*/
#define CAC_UNLOAD_DELAY 13

#define CAC_CASHUP 5

/*
**	CrystAlien's infantry walk-cycle rate, in ticks per step (see InfantryClass::Do_Action).
*/
#define CAC_WALK_RATE 3

/*
**	UnitNav.feed lifts up to this much from a crystal tile per mouthful.
*/
#define CAC_MINE_MOUTHFUL 10

void CAC_Mining_Sparks(UnitClass *unit, bool working);
int CAC_Load_Steps(TechnoClass const *object);
int CAC_Bail_Credits(TechnoClass const *object);
bool CAC_Is_Miner(UnitType type);
bool CAC_Is_Refinery(StructType type);
bool CAC_Is_Air_Home(StructType type);

CACEngineerAction CAC_Engineer_Effect(TechnoClass *engineer, ObjectClass *target);
void CAC_Full_Repair(TechnoClass *target);

int CAC_Type_Index_Of_Type(TechnoTypeClass const *type);
int CAC_Max_Passengers(RTTIType rtti, int type);
void CAC_Superweapon_Damage(COORDINATE ground_zero, TechnoClass *source);
bool CAC_Is_Transformer(AircraftType type);
bool CAC_Is_Lander(AircraftType type);
StructType CAC_Homes_To(RTTIType rtti, int type);
bool CAC_Is_Boomerang(AircraftType type);
void CAC_Rearm_At_Home(AircraftClass * craft);
bool CAC_Picks_Own_Target(TechnoClass const *object);
BuildingClass *CAC_Home_Building(TechnoClass const *object);
void CAC_Check_Home(TechnoClass *object);
BuildingClass * CAC_House_Target(HouseClass const * house);
bool CAC_Holds_Fire_Landing(AircraftClass const * craft);
void CAC_Strafe(AircraftClass * craft);
void CAC_Opportunity_Fire(TechnoClass *object);
void CAC_Return_Home(AircraftClass *craft);
bool CAC_On_Map(CELL cell);
void CAC_Contain_Flier(AircraftClass *craft);
bool CAC_Aircraft_Hovers(AircraftType type);
bool CAC_Berth_Is_Clear(TechnoClass const * craft);
/*
**	How near a berth counts as on it.  Tiberian Dawn parks an aircraft close to
**	a coordinate rather than on the tile, so an exact test never comes true.
*/
#define CAC_BERTH_SLACK 2

/*
**	And how near is actually *on* it.  CAC_BERTH_SLACK above is the radius within
**	which a craft counts as home for the purpose of not being re-ordered there
**	every second; this is the much smaller distance within which it is standing
**	on the plate and may set down.  Process_Fly_To stops a craft inside 0x0010 of
**	what it was sent to, so half a cell is loose enough to reach on one approach
**	and tight enough that the craft is on the artwork.
*/
#define CAC_BERTH_EXACT 0x0080

bool CAC_At_Berth(AircraftClass const * craft);
COORDINATE CAC_Berth_Coord(AircraftClass const * craft);
bool CAC_On_Berth(AircraftClass const * craft);
void CAC_Log_Lander(AircraftClass const * craft, char const * what,
						  int still, bool doorclosed, int status);
void CAC_Log_Berth_Refusal(AircraftClass const * craft, bool clear, bool berthed);
void CAC_Log_Berth_Restless(AircraftClass const * craft, bool hasnav, int speed, int mission);
bool CAC_Sends_Home(TechnoClass const * object, ObjectClass const * target);
bool CAC_Never_Lands(AircraftClass const * craft);
bool CAC_Ignores_LZ(AircraftClass const * craft);
char const * CAC_Ground_Name(char const * stock);
bool CAC_Is_Capturer(ObjectClass const *obj);
bool CAC_Band_Excluded(ObjectClass const *obj);
bool CAC_Engineers_Answer(ObjectClass *target);
ObjectClass * CAC_Selected_Capturer(void);
ObjectClass * CAC_Selected_Boarder(void);
void CAC_Rehouse_Prebuilt(void);
void CAC_Log_Kill(TechnoClass const * victim, TechnoClass const * killer);
void CAC_Roll_Call(void);
void CAC_Watch_Factories(void);
bool CAC_Is_Carriable(ObjectClass const * obj);
bool CAC_Is_Own_Carrier(TechnoClass const *object);
bool CAC_Fires_Both(TechnoClass const *object);
ObjectClass * CAC_Group_Capturer(DynamicVectorClass<ObjectClass *> &objects,
                                 ObjectClass * subject);
ObjectClass * CAC_Group_Capturer(DynamicVectorClass<ObjectClass *> &objects,
                                 CELL cell);

/*
**	CrystAlien's logic runs at 23 frames per second against C&C's 15 ticks.
*/
#define CAC_FPS_NUM	23
#define CAC_FPS_DEN	15

void CAC_Hover_Offset(ObjectClass const *object, int &x, int &y);
int CAC_Lander_Step(AircraftClass const * craft);
int CAC_Source_Speed(TechnoClass const * object);
int CAC_Transformer_Bank(int altitude, int flight_level);

int CAC_Threat_Rating(RTTIType rtti, int type);
int CAC_Target_Score(TechnoClass const *object);

/*
**	Powerup pickup, per Unit::collect: "weaponRange += 25" in CrystAlien world
**	pixels (96 to a tile, 256 leptons to a cell) and "maxWeaponCharge - 1", never
**	below 2.
*/
#define CAC_POWERUP_RANGE		67
#define CAC_POWERUP_MIN_ROF	2
#define CAC_POWERUP_MAX			8

bool CAC_Special_Ops_Allowed(void);
bool CAC_Christmas_Allowed(void);
void CAC_Miner_Stuck(UnitClass *miner);
bool CAC_Pizza_Mode(void);
CELL CAC_Pizza_Cell(HouseClass const *whose, bool theirs);
bool CAC_Pizza_Protected(BuildingClass const *b);
void CAC_Pizza_Stipend(HouseClass *house);
bool CAC_Pizza_Deliver(HouseClass *house);
bool CAC_Pizza_Taken(CellClass *cell, ObjectClass *collector);
bool CAC_Pizza_Errand(FootClass const *obj);
void CAC_AI_Pizza(HouseClass *house);
bool CAC_SpecOps_Needs_Tech(void);
bool CAC_Visceroids_Allowed(void);
void CAC_Purge_Visceroids(void);
bool CAC_Prereq_Met(HouseClass const *house, RTTIType rtti, int type);
bool CAC_Owns(HouseClass const *house, int code);
/*
**	CrystAlien's own names for things, so that the parts of the engine that care
**	about a role rather than a slot can say which role they mean.  See
**	CAC_Prereq_Met, which reads the build tree in these terms.
*/
#define CAC_CODE_VALUES
#include "CACCODES.INC"
#undef CAC_CODE_VALUES

int CAC_Code_Of(RTTIType rtti, int type);
bool CAC_Is_Code(RTTIType rtti, int type, int letters);
int CAC_Slot_For(RTTIType rtti, int letters, bool evil);
bool CAC_Has_Santa(HouseClass const *house, RTTIType rtti, int type);
void CAC_Unlock_Santa(HouseClass *house);
/*
**	Which of CrystAlien's three pickups this is.  See CAC_Crate_Effect.
*/
typedef enum {
	CAC_PICKUP_BOX,			/* the wooden crystal box -- a free unit */
	CAC_PICKUP_CAPSULE,	/* the white-and-green Alien capsule -- money */
	CAC_PICKUP_GIFT		/* the Christmas present -- Santa's Sleigh */
} CACPickup;

bool CAC_Crate_Effect(FootClass *object, CELL cell, CACPickup kind);

/* Shape bases for a transformer's landing and ground art banks. */
#define CAC_BANK_LANDING	128
#define CAC_BANK_GROUND		160
#define CAC_TRANSFORMER_IDLE_TICKS	20
void CAC_Remap_Interface_Sounds(void);
bool CAC_Is_CrystAlien_Type(char const *ininame);

/*
**	Diagnostics.  Silent unless CACDEBUG.TAG is on the search path, so leaving the
**	calls in costs a pointer test per line and nothing else.
*/
bool CAC_Logging(void);
void CAC_Log(char const * fmt, ...);
bool CAC_Is_CrystAlien_Effect(char const *ininame);

/*
**	Diagnostics.  Silent unless CACDEBUG.TAG is on the search path, so leaving the
**	calls in costs a pointer test per line and nothing else.
*/
bool CAC_Logging(void);
void CAC_Log(char const * area, char const * fmt, ...);
void CAC_Log_Volatile(char const * area, char const * fmt, ...);
void CAC_Log_Warn(char const * area, char const * fmt, ...);
void CAC_Install_Crash_Filter(void);
void CAC_Log_Asset(char const * area, char const * name);
void CAC_Disable_Stock_Types(void);

#endif
