/*
**	CACLOGIC.CPP -- CrystAlien Conflict behaviour that Tiberian Dawn does not have.
**
**	Everything here is driven by what the original game's ActionScript actually
**	does; the reference for each rule is quoted next to it.  See
**	docs/01-crystalien-internals.md for the recovered source these came from.
*/

#include "function.h"
#include <math.h>
#include <time.h>
#include <stdarg.h>
#include "CACSTATS.H"
#include "CACSLOTS.INC"
#include "CACLOGIC.H"
#include "CACRADAR.INC"


/***********************************************************************************************
 * CAC_Side -- Which CrystAlien faction a house plays as.                                      *
 *                                                                                             *
 *    Multiplayer houses are neither GOOD nor BAD themselves; they play *as* one of them,      *
 *    which ActLike records.  Everything faction-flavoured keys off this.                      *
 *=============================================================================================*/
HousesType CAC_Side(HousesType house)
{
	for (int i = 0; i < Houses.Count(); i++) {
		HouseClass *hptr = Houses.Ptr(i);
		if (hptr != NULL && hptr->Class->House == house) {
			return (hptr->ActLike == HOUSE_BAD) ? HOUSE_BAD : HOUSE_GOOD;
		}
	}
	return (house == HOUSE_BAD) ? HOUSE_BAD : HOUSE_GOOD;
}


/*
**	Per-unit voice table.  CrystAlien plays "<type>_comply" when a unit takes an
**	order, "<type>_action_N" when it fires or works, "<type>_pain_N" when hurt
**	and "<type>_die" on death -- and every unit has its own recordings on each
**	side.
*/
struct CACVoiceEntry {
	RTTIType RTTI;
	int Type;
	HousesType Side;
	VocType Voice[CAC_VOICE_COUNT];
};

#include "CACVOICES.INC"


/***********************************************************************************************
 * CAC_Voice -- The CrystAlien voice line for one object and event.                            *
 *                                                                                             *
 * OUTPUT:  VOC_NONE when the object is not a CrystAlien type, so callers can fall back to     *
 *          the stock behaviour.                                                               *
 *=============================================================================================*/
VocType CAC_Voice(RTTIType rtti, int type, HousesType owner, CACVoiceEvent event)
{
	if (event < 0 || event >= CAC_VOICE_COUNT) return(VOC_NONE);

	/*
	**	The recordings are named for the unit -- "UE_evil_die" -- so the side that
	**	decides which of them plays is the unit's own.  This used to ask the owner
	**	instead, which agreed with the type only while the two factions shared a
	**	set of slots.  Since the split they can disagree: an Alien Speeder handed
	**	over by a pod, or a captured one, is owned by an Astro house and matched
	**	no row at all, so it died in silence and left the stock explosion as the
	**	only thing to hear.
	**
	**	The owner is still tried first, for the one case where it says something
	**	the type cannot: a slot that both factions share.
	*/
	HousesType side = CAC_Side(owner);
	int fallback = -1;
	for (int i = 0; i < (int)(sizeof(CACVoices) / sizeof(CACVoices[0])); i++) {
		CACVoiceEntry const &e = CACVoices[i];
		if (e.RTTI != rtti || e.Type != type) continue;
		if (e.Side == side) return(e.Voice[event]);
		if (fallback < 0) fallback = i;
	}
	if (fallback >= 0) return(CACVoices[fallback].Voice[event]);
	return(VOC_NONE);
}


/***********************************************************************************************
 * CAC_Speak -- Play an object's own CrystAlien voice line.                                    *
 *                                                                                             *
 * OUTPUT:  true if a CrystAlien line was played; false leaves the caller to do whatever it     *
 *          would normally have done.                                                          *
 *=============================================================================================*/
int CAC_Type_Index(TechnoClass const *obj)
{
	if (obj == NULL) return(-1);
	switch (obj->What_Am_I()) {
		case RTTI_INFANTRY:	return((int)*(InfantryClass *)obj);
		case RTTI_UNIT:		return((int)*(UnitClass *)obj);
		case RTTI_AIRCRAFT:	return((int)*(AircraftClass *)obj);
		case RTTI_BUILDING:	return((int)*(BuildingClass *)obj);
		default:					return(-1);
	}
}


bool CAC_Speak(TechnoClass *obj, CACVoiceEvent event)
{
	if (obj == NULL) return(false);

	int type = CAC_Type_Index(obj);
	if (type < 0) return(false);

	/*
	**	makeNoise plays nothing for something you cannot see:
	**
	**		this.makeNoise = function(id)
	**		{
	**		   if(!this.parent.arena.onDisplay(this)) return undefined;
	**		   this.parent.parent.sfx.play(id);
	**		}
	**
	**	It is a flat test against the visible rectangle rather than a distance
	**	falloff, so a battle across the map is silent rather than faint -- which
	**	is what a six-player match needs it to be.
	*/
	/*
	**	Off-screen is silent, except when answering an order.
	**
	**	makeNoise is what the source guards -- `if (!arena.onDisplay(this)) return;`
	**	-- and it is used for the things a unit does: taking a mouthful, being hit,
	**	dying.  The comply line does not go through it.  Unit.handle plays it
	**	straight, `sfx.play(this.type + "_comply")`, because it is the answer to
	**	something the player just did and they are owed it wherever the unit is.
	**
	**	Without that distinction a Fighter parked at its station across the map --
	**	which is where a Fighter usually is when it is given an order -- answered
	**	nothing at all.
	*/
	if (event != CAC_VOICE_COMPLY &&
		!Map.In_View(Coord_Cell(obj->Center_Coord()))) return(true);

	VocType voc = CAC_Voice(obj->What_Am_I(), type, obj->Owner(), event);
	if (voc == VOC_NONE) return(false);

	/*
	**	Acknowledgements are voice responses and honour the voice setting; the
	**	rest are positional effects at the object.
	*/
	if (event == CAC_VOICE_COMPLY) {
		if (!AllowVoice) return(true);
		Sound_Effect(voc, 0, -(obj->As_Target() & 0xFF) - 1);
	} else {
		Sound_Effect(voc, obj->Center_Coord());
	}
	return(true);
}


/***********************************************************************************************
 * CAC_Type_Cap -- The per-type build cap for a CrystAlien object.                             *
 *                                                                                             *
 *    CrystAlien hard-caps how many of each type a player may own -- `this.max` in UnitStats    *
 *    and BuildingStats, enforced by `checkMax(total)` which sets `disabled` and greys the      *
 *    sidebar entry out.  Tiberian Dawn has no such concept, so the caps come from the          *
 *    generated CACSTATS.H and are applied in the production path.                              *
 *                                                                                             *
 * OUTPUT:  0 when the type is uncapped.                                                       *
 *=============================================================================================*/
struct CACCapEntry {
	RTTIType	RTTI;
	int			Type;
	int			Max;
};

static CACCapEntry const CACCaps[] = {
#include "CACCAPS.INC"
};

/***********************************************************************************************
 * CAC_Build_Level -- The level a build is held to in this kind of game.                        *
 *                                                                                             *
 *    A CrystAlien type carries its depth in CrystAlien's own tree as `Level`, which is what    *
 *    the skirmish lobby's Tech Level slider should cut against.  A campaign mission is a        *
 *    different scale -- Read_Scenario_Ini sets BuildLevel to the mission number, and the stock *
 *    missions are paced to stock types -- so there a build answers with the level of the stock *
 *    type it stands in for.  Nod's third mission is won by capturing a prison and is level      *
 *    three because that is where the stock game hands out an engineer; the Saboteur's depth is *
 *    four, so it could never be built there.  See campaign_levels in gen_dll_types.py.         *
 *=============================================================================================*/
struct CACCampaignEntry {
	RTTIType	RTTI;
	int			Type;
	int			Level;
};

static CACCampaignEntry const CACCampaign[] = {
#include "CACCAMPAIGN.INC"
};

unsigned CAC_Build_Level(TechnoTypeClass const *type)
{
	if (type == NULL) return(0);
	if (GameToPlay != GAME_NORMAL || !CAC_Is_CrystAlien_Type(type->IniName)) {
		return(type->Level);
	}

	RTTIType rtti = type->What_Am_I();
	int index = CAC_Type_Index_Of_Type(type);
	for (int i = 0; i < (int)(sizeof(CACCampaign) / sizeof(CACCampaign[0])); i++) {
		if (CACCampaign[i].RTTI == rtti && CACCampaign[i].Type == index) {
			return((unsigned)CACCampaign[i].Level);
		}
	}
	return(type->Level);
}


/***********************************************************************************************
 * CAC_Built_Counts_As -- Does building this satisfy a trigger that asks for that?            *
 *                                                                                             *
 *    The scenarios' "Built It" event names a stock StructType -- Nod's fifth mission allows   *
 *    its win only once a SAM Site (10) goes up -- and TriggerClass::Spring compares it with   *
 *    the type just built.  A CrystAlien structure has a slot of its own, so the Alien turret  *
 *    built for it allowed nothing, and nothing else in the mission could win it.  The         *
 *    missions are converted with make_missions.SWAP, and CACSTANDS.INC is the same table:    *
 *    the stock structures each CrystAlien one stands in for.                                  *
 *=============================================================================================*/
struct CACStandsEntry {
	StructType			Type;
	unsigned long long	Stock;
};

static CACStandsEntry const CACStands[] = {
#include "CACSTANDS.INC"
};

bool CAC_Built_Counts_As(long built, long wanted)
{
	if (built == wanted) return(true);
	if (wanted < 0 || wanted >= 64) return(false);
	for (int i = 0; i < (int)(sizeof(CACStands) / sizeof(CACStands[0])); i++) {
		if ((long)CACStands[i].Type == built) {
			return((CACStands[i].Stock & (1ULL << wanted)) != 0);
		}
	}
	return(false);
}


int CAC_Type_Cap(RTTIType rtti, int type)
{
	RTTIType want = rtti;
	if (rtti == RTTI_UNIT)      want = RTTI_UNITTYPE;
	if (rtti == RTTI_INFANTRY)  want = RTTI_INFANTRYTYPE;
	if (rtti == RTTI_AIRCRAFT)  want = RTTI_AIRCRAFTTYPE;
	if (rtti == RTTI_BUILDING)  want = RTTI_BUILDINGTYPE;

	for (int i = 0; i < (int)(sizeof(CACCaps) / sizeof(CACCaps[0])); i++) {
		if (CACCaps[i].RTTI == want && CACCaps[i].Type == type) return(CACCaps[i].Max);
	}
	return(0);
}


/***********************************************************************************************
 * CAC_Owned_Count -- How many of a type a house already owns.                                 *
 *                                                                                             *
 *    Counts objects in play plus anything the house currently has in production, so the cap    *
 *    cannot be beaten by queueing.                                                             *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Aboard_Count -- How many of a type this house has sitting inside its own transports.     *
 *                                                                                             *
 *    A passenger is IsInLimbo, and it still exists as far as the source is concerned:          *
 *    doEmbark moves it to posX/posY -1000 and pushes it into the carrier's `contents`, but it  *
 *    is never removed from `parent.units` and so never leaves checkMax's total.  So a squad    *
 *    in a Dropship still counts against the cap, and the count below cannot simply skip every  *
 *    object in limbo.                                                                          *
 *=============================================================================================*/
static int CAC_Aboard_Count(HouseClass const *house, RTTIType want, int type)
{
	HousesType owner = house->Class->House;
	int count = 0;

	for (int pass = 0; pass < 2; pass++) {
		int total = (pass == 0) ? Units.Count() : Aircraft.Count();
		for (int i = 0; i < total; i++) {
			TechnoClass const *carrier = (pass == 0)
				? (TechnoClass const *)Units.Ptr(i)
				: (TechnoClass const *)Aircraft.Ptr(i);
			if (carrier == NULL || !carrier->IsActive) continue;
			if (carrier->Owner() != owner) continue;

			for (FootClass const *f = ((CargoClass const *)carrier)->Attached_Object();
				  f != NULL; f = (FootClass const *)f->Next) {
				if (!f->IsActive || f->What_Am_I() != want) continue;
				TechnoTypeClass const *t = f->Techno_Type_Class();
				if (t == NULL) continue;
				if (CAC_Type_Index_Of_Type(t) == type) count++;
			}
		}
	}
	return(count);
}


/***********************************************************************************************
 * CAC_Owned_Count -- How many of a type this house actually has.                               *
 *                                                                                             *
 *    "I have no Astro infantry and I cannot build any, as if it were capped" -- and it was.     *
 *                                                                                             *
 *    This counted every object in the heap that was IsActive and owned, and IsActive is not     *
 *    the same question as being in the game.  An object that was created and never placed is    *
 *    active, owned, in limbo, and standing at a cell number outside the map; it cannot be seen, *
 *    selected, commanded or killed, and it filled a slot under the cap for the rest of the      *
 *    round.  Five of those and the cameo greys with nothing on the field to show for it.        *
 *                                                                                             *
 *    See CAC_Spawn_Survivor for where they came from.  Not counting them is the other half:     *
 *    a phantom that does turn up from somewhere else must not be able to do this again.         *
 *                                                                                             *
 *    Passengers are the exception and are counted separately -- see CAC_Aboard_Count.  A squad  *
 *    in a Dropship is in limbo too, and it is genuinely yours.                                  *
 *=============================================================================================*/
int CAC_Owned_Count(HouseClass const *house, RTTIType rtti, int type)
{
	if (house == NULL || house->Class == NULL) return(0);
	HousesType owner = house->Class->House;
	int count = 0;

	switch (rtti) {
		case RTTI_INFANTRYTYPE:
		case RTTI_INFANTRY:
			for (int i = 0; i < Infantry.Count(); i++) {
				InfantryClass *o = Infantry.Ptr(i);
				if (o != NULL && o->IsActive && !o->IsInLimbo &&
					o->Owner() == owner && *o == (InfantryType)type) count++;
			}
			break;

		case RTTI_UNITTYPE:
		case RTTI_UNIT:
			for (int i = 0; i < Units.Count(); i++) {
				UnitClass *o = Units.Ptr(i);
				if (o != NULL && o->IsActive && !o->IsInLimbo &&
					o->Owner() == owner && *o == (UnitType)type) count++;
			}
			break;

		case RTTI_AIRCRAFTTYPE:
		case RTTI_AIRCRAFT:
			for (int i = 0; i < Aircraft.Count(); i++) {
				AircraftClass *o = Aircraft.Ptr(i);
				if (o != NULL && o->IsActive && !o->IsInLimbo &&
					o->Owner() == owner && *o == (AircraftType)type) count++;
			}
			break;

		case RTTI_BUILDINGTYPE:
		case RTTI_BUILDING:
			for (int i = 0; i < Buildings.Count(); i++) {
				BuildingClass *o = Buildings.Ptr(i);
				if (o != NULL && o->IsActive && o->Owner() == owner && *o == (StructType)type) count++;
			}
			break;

		default:
			break;
	}
	/*
	**	And whoever is riding in something, who is in limbo but is still yours.
	*/
	if (rtti == RTTI_INFANTRY || rtti == RTTI_INFANTRYTYPE) {
		count += CAC_Aboard_Count(house, RTTI_INFANTRY, type);
	} else if (rtti == RTTI_UNIT || rtti == RTTI_UNITTYPE) {
		count += CAC_Aboard_Count(house, RTTI_UNIT, type);
	} else if (rtti == RTTI_AIRCRAFT || rtti == RTTI_AIRCRAFTTYPE) {
		count += CAC_Aboard_Count(house, RTTI_AIRCRAFT, type);
	}

	return(count);
}


/***********************************************************************************************
 * CAC_At_Type_Cap -- Whether a house may build another of this type.                          *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Cancel_Unbuildable -- Give back what is being built for a building that is gone.         *
 *                                                                                             *
 *    Construction.think, before it advances anything:                                          *
 *                                                                                             *
 *        if (!this.constructingBuilding.active) this.cancel(this.constructingBuilding, true);  *
 *        if (!this.constructingUnit.active)     this.cancel(this.constructingUnit, true);      *
 *                                                                                             *
 *    `active` is the shortcut's prerequisite state, so the moment the building that unlocked   *
 *    an item stops standing, the item stops being built -- and the `true` is the refund flag,  *
 *    which hands back what has been spent rather than only what an unfinished item is worth.   *
 *                                                                                             *
 *    Without it the queue keeps a thing nothing can finish.  Tiberian Dawn has no equivalent   *
 *    because its prerequisites do not usually vanish mid-build, and its factories are the      *
 *    prerequisite -- lose the Weapons Factory and the vehicle in it goes with the building.    *
 *    Here the two are separate: an Ops Ship is built at the headquarters that unlocked it, so  *
 *    losing an Eagle Command Base leaves a factory holding something it will never be allowed  *
 *    to release, and no way to clear it and nothing given back.                                *
 *=============================================================================================*/
void CAC_Cancel_Unbuildable(HouseClass * house)
{
	if (house == NULL) return;

	/*
	**	Backwards, because this deletes what it is walking.
	**	
	**	Abandon_Production removes the factory, and TFixedIHeapClass compacts on
	**	delete -- ActivePointers.Delete(pointer) -- so cancelling the one at index i
	**	moves what was at i+1 down into i and the loop then steps over it.  Whatever
	**	gets skipped keeps a queue nobody can finish until a later pass happens to
	**	catch it at a different index.
	**	
	**	Counting down means a deletion only ever disturbs indices this walk has
	**	already been past.  The superweapon needs the heavier fix because a blast can
	**	kill things other than the one it is holding; here only the current entry
	**	goes, so this is enough.
	*/
	for (int i = Factories.Count() - 1; i >= 0; i--) {
		FactoryClass * f = Factories.Ptr(i);
		if (f == NULL || f->Get_House() != house) continue;

		TechnoClass const * o = f->Get_Object();
		if (o == NULL || o->Techno_Type_Class() == NULL) continue;

		RTTIType rtti = o->What_Am_I();
		int index = CAC_Type_Index_Of_Type(o->Techno_Type_Class());

		/*
		**	CrystAlien's own, and only CrystAlien's.  The engine's items are tied to
		**	the factory holding them and do not outlive it.
		*/
		if (CAC_Code_Of(rtti, index) == CAC_CODE_NONE) continue;
		if (CAC_Prereq_Met(house, rtti, index)) continue;

		/*
		**	Abandon refunds what was spent, which is what cancel(item, true) does.
		**	One a frame: this walks the list the call is about to change.
		*/
		house->Abandon_Production(rtti, index);
		house->IsRecalcNeeded = true;
		return;
	}
}


/***********************************************************************************************
 * CAC_Log_Cap -- Say which numbers greyed a cameo, not merely that something did.               *
 *                                                                                             *
 *    "Only one of any foot soldier can be built" is a claim about three numbers -- how many    *
 *    are held, how many are on the way, and what the cap is -- and the line this replaces      *
 *    printed none of them.  It printed `busy=1`, which says only that the flag it had just     *
 *    set was set, and it damped itself per asset name, so the first occurrence was the only    *
 *    one ever recorded whatever the numbers did afterwards.                                    *
 *                                                                                             *
 *    The caps themselves are worth knowing while reading it: five for either foot soldier,     *
 *    eight for the Reindeer (nine here, a deliberate override for a full team), four for the   *
 *    Miner, and one for exactly two types -- Santa, and the Alien Commander, which is the      *
 *    other deliberate override.  A cap of one on anything else is a fault.                     *
 *=============================================================================================*/
void CAC_Log_Cap(char const *asset, HouseClass const *house, RTTIType rtti, int type)
{
	if (!CAC_Logging() || asset == NULL || house == NULL) return;

	int cap = CAC_Type_Cap(rtti, type);
	if (cap <= 0) return;

	int held = CAC_Owned_Count(house, rtti, type);

	RTTIType want = rtti;
	if (want == RTTI_UNIT)      want = RTTI_UNITTYPE;
	if (want == RTTI_INFANTRY)  want = RTTI_INFANTRYTYPE;
	if (want == RTTI_AIRCRAFT)  want = RTTI_AIRCRAFTTYPE;
	if (want == RTTI_BUILDING)  want = RTTI_BUILDINGTYPE;

	int queued = 0;
	for (int i = 0; i < Factories.Count(); i++) {
		FactoryClass *f = Factories.Ptr(i);
		if (f == NULL || f->Get_House() != house) continue;
		TechnoClass const *o = f->Get_Object();
		if (o == NULL || o->Techno_Type_Class() == NULL) continue;

		RTTIType kind = o->What_Am_I();
		if (kind == RTTI_UNIT)      kind = RTTI_UNITTYPE;
		if (kind == RTTI_INFANTRY)  kind = RTTI_INFANTRYTYPE;
		if (kind == RTTI_AIRCRAFT)  kind = RTTI_AIRCRAFTTYPE;
		if (kind == RTTI_BUILDING)  kind = RTTI_BUILDINGTYPE;
		if (kind != want) continue;
		if (CAC_Type_Index_Of_Type(o->Techno_Type_Class()) != type) continue;
		queued++;
	}

	/*
	**	Damped on the numbers rather than on the name, so a cap that moves is
	**	reported each time it moves and a cap that sits still is reported once.
	*/
	static char _seen[64][16];
	static int  _held[64];
	static int  _queued[64];
	static int  _n = 0;

	int i;
	for (i = 0; i < _n; i++) if (strcmp(_seen[i], asset) == 0) break;
	if (i < _n && _held[i] == held && _queued[i] == queued) return;
	if (i == _n) {
		if (_n >= 64) return;
		strncpy(_seen[_n], asset, 15);
		_seen[_n][15] = 0;
		i = _n++;
	}
	_held[i] = held;
	_queued[i] = queued;

	CAC_Log("cap", "%s greyed: %d held + %d building >= cap %d",
			  asset, held, queued, cap);
}


bool CAC_At_Type_Cap(HouseClass const *house, RTTIType rtti, int type)
{
	int cap = CAC_Type_Cap(rtti, type);
	if (cap <= 0) return(false);


	int held = CAC_Owned_Count(house, rtti, type);

	/*
	**	And whatever is on the way counts too.
	**
	**	checkMax greys the cameo at `total >= max`, and CrystAlien cannot be building
	**	a fifth of anything while it holds four -- there is one unit queue, so the
	**	fourth has to have arrived before a fifth can be started.  Here there is a
	**	queue per host, so a Miner can be in the works while three stand outside; the
	**	count said three, the cameo stayed lit, and the click that followed was
	**	refused by Begin_Production with nothing to show for it.
	*/
	RTTIType want = rtti;
	if (want == RTTI_UNIT)      want = RTTI_UNITTYPE;
	if (want == RTTI_INFANTRY)  want = RTTI_INFANTRYTYPE;
	if (want == RTTI_AIRCRAFT)  want = RTTI_AIRCRAFTTYPE;
	if (want == RTTI_BUILDING)  want = RTTI_BUILDINGTYPE;

	for (int i = 0; i < Factories.Count(); i++) {
		FactoryClass * f = Factories.Ptr(i);
		if (f == NULL || f->Get_House() != house) continue;

		TechnoClass const * o = f->Get_Object();
		if (o == NULL || o->Techno_Type_Class() == NULL) continue;

		RTTIType kind = o->What_Am_I();
		if (kind == RTTI_UNIT)      kind = RTTI_UNITTYPE;
		if (kind == RTTI_INFANTRY)  kind = RTTI_INFANTRYTYPE;
		if (kind == RTTI_AIRCRAFT)  kind = RTTI_AIRCRAFTTYPE;
		if (kind == RTTI_BUILDING)  kind = RTTI_BUILDINGTYPE;
		if (kind != want) continue;

		if (CAC_Type_Index_Of_Type(o->Techno_Type_Class()) != type) continue;
		held++;
	}

	return(held >= cap);
}


/***********************************************************************************************
 * CAC_Is_Miner -- Is this slot the Crystal Miner / Crystal Extractor?                          *
 *                                                                                             *
 *    Tiberian Dawn asks `*this == UNIT_HARVESTER` in a dozen places -- when to head home on    *
 *    damage, what to do after unloading, whether to resume harvesting, which remap table to    *
 *    use.  In the conversion UD *is* UNIT_HARVESTER and all of that lands correctly, but the   *
 *    supplemental build gives CrystAlien its own slots, and there the miner would answer no    *
 *    to every one of those questions and simply stop working.                                  *
 *                                                                                             *
 *    Asking here instead means both builds behave the same, and the conversion's answer is     *
 *    unchanged.                                                                                *
 *=============================================================================================*/
bool CAC_Is_Miner(UnitType type)
{
	return(type == UNIT_HARVESTER || CAC_Is_Code(RTTI_UNITTYPE, (int)type, CAC_CODE_UD));
}


/***********************************************************************************************
 * CAC_Is_Refinery -- Whether harvesters may dock at this structure.                           *
 *                                                                                             *
 *    In CrystAlien the miner's home is `findBuilding("BA_" + team)` -- the Command Base, not a *
 *    separate refinery -- and it delivers with `cashUp(cargo)` on arrival.  The Astro-only     *
 *    MT-201 Driller (BL) is a refinery in its own right (`this.refinery = true`), so both      *
 *    count.                                                                                   *
 *=============================================================================================*/
bool CAC_Is_Refinery(StructType type)
{
	/*
	**	The Ops Ship and the Alien Hive take a miner too, on purpose.
	**
	**	CrystAlien does not do this.  UnitStats resolves a miner's home once, at
	**	construction, to `findBuilding("BA_" + this.team)` -- the Command Base and
	**	never anything else -- so losing it in the source game strands every miner
	**	a player has with nowhere to deliver.
	**
	**	Both are headquarters here, doPrerequisites already treats either as the
	**	root of the build tree, and a second place to unload is worth having.  So
	**	the Ops Ship accepts cargo, can be chosen deliberately, and is what miners
	**	fall back to when the Command Base is gone.  A deliberate departure, asked
	**	for directly; see UnitClass::Find_Best_Refinery for the order of preference
	**	and FootClass for the part that makes an explicit order stick.
	*/
	return(CAC_Is_Code(RTTI_BUILDINGTYPE, (int)type, CAC_CODE_BA) ||
	       CAC_Is_Code(RTTI_BUILDINGTYPE, (int)type, CAC_CODE_BK) ||
	       type == STRUCT_REFINERY);
}


/***********************************************************************************************
 * CAC_Is_Air_Home -- Whether aircraft dock and rearm at this structure.                       *
 *                                                                                             *
 *    Fighters and dropships home to `findBuilding("BF_" + team)` -- the Radar/Sonar Station -- *
 *    and UnitNav tops their payload back up on arrival (`weaponPayload = min(++payload, max)`).*
 *    The Ops Ship / Alien Hive (BK) launches the transforming fighter, so it counts too.       *
 *=============================================================================================*/
bool CAC_Is_Air_Home(StructType type)
{
	return(CAC_Is_Code(RTTI_BUILDINGTYPE, (int)type, CAC_CODE_BF) ||
	       CAC_Is_Code(RTTI_BUILDINGTYPE, (int)type, CAC_CODE_BK));
}


/***********************************************************************************************
 * CAC_Engineer_Effect -- What an engineer does on reaching its target.                        *
 *                                                                                             *
 *    UnitNav::checkNonDestructiveTarget dispatches on ownership:                               *
 *                                                                                             *
 *        if (stats.repair) {                                                                  *
 *            if (target.team != parent.team)                    -> doInfiltrate()              *
 *            if (target.repairable && health < maxHealth)       -> doRepair()                  *
 *        }                                                                                    *
 *                                                                                             *
 *    doInfiltrate() flips the target's team -- a capture.  doRepair() restores it to full      *
 *    health outright, not gradually.  Both then call destroy(true): the engineer is consumed   *
 *    either way.  Note this is symmetric -- both the Astro Engineer and the Alien Saboteur     *
 *    can capture *and* repair; they are the same unit with different art.                     *
 *=============================================================================================*/
CACEngineerAction CAC_Engineer_Effect(TechnoClass *engineer, ObjectClass *target)
{
	if (engineer == NULL || target == NULL) return(CAC_ENGINEER_NOTHING);

	/*
	**	doRepair ends with makeNoise("INT_repair") and doInfiltrate with
	**	makeNoise("INT_infiltrate"), both alongside the engineer's own action
	**	line.  The samples shipped with the rest and nothing had played them.
	*/

	TechnoClass *victim = (TechnoClass *)target;
	if (victim->Owner() != engineer->Owner()) {
		// makeNoise, so only when the player can see it happen
		if (Map.In_View(Coord_Cell(target->Center_Coord()))) {
			CAC_Interface_Sound(VOC_CAC_INT_INFILT, target->Center_Coord());
		}
		return(CAC_ENGINEER_CAPTURE);
	}
	if (victim->Health_Ratio() < 0x100) {
		/*
		**	Volatile: reaching a *vehicle* is unsolved.  CrystAlien has the target give
		**	up its tile so the engineer can stand on it; nothing here does, so the
		**	engineer halts beside it and its arrival is never noticed.  If this line
		**	never appears, it never got close enough to be asked.
		*/
		CAC_Log_Volatile("engineer", "repairs %s (%d hp)",
								 victim->Techno_Type_Class()
									? victim->Techno_Type_Class()->IniName : "?", victim->Strength);

		// makeNoise, as above
		if (Map.In_View(Coord_Cell(target->Center_Coord()))) {
			CAC_Interface_Sound(VOC_CAC_INT_REPAIR, target->Center_Coord());
		}
		return(CAC_ENGINEER_REPAIR);
	}
	return(CAC_ENGINEER_NOTHING);
}


/***********************************************************************************************
 * CAC_Full_Repair -- Restore a target to full health, the CrystAlien way.                     *
 *                                                                                             *
 *    doRepair() sets `target.health = target.stats.maxHealth` in one step and clears the       *
 *    damage smoke, rather than repairing over time.                                            *
 *=============================================================================================*/
void CAC_Full_Repair(TechnoClass *target)
{
	if (target == NULL) return;
	target->Strength = target->Class_Of().MaxStrength;
	if (target->What_Am_I() == RTTI_BUILDING) {
		((BuildingClass *)target)->Mark(MARK_CHANGE);
	}
}


/***********************************************************************************************
 * CAC_Is_CrystAlien_Type -- Does this IniName belong to the conversion?                       *
 *                                                                                             *
 *    CrystAlien types carry the source game's own two-letter codes ("BA".."BL", "UA".."UR").  *
 *    Stock civilian structures ("V01".."V37", "ARCO", ...) and terrain are longer, so the      *
 *    length-two test cleanly separates the two rosters.  This is the same rule                 *
 *    CAC_Faction_Suffix uses when picking per-faction artwork.                                 *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Logging / CAC_Log -- Write down what actually happened.                                  *
 *                                                                                             *
 *    Several things in this mod have been diagnosed by reading the source and reasoning about  *
 *    it, and a few of those diagnoses were wrong -- the flash that was a placeholder, the      *
 *    music that reached the jukebox and played nothing, the crate collector that was never     *
 *    called.  Each cost a round trip to find out.  What was missing every time was a record of *
 *    what the game did rather than what it should have done.                                    *
 *                                                                                             *
 *    So: a file, written beside the save games, listing the things that are hard to see from   *
 *    outside -- the asset names handed to the client, what a dying object asks to be drawn,     *
 *    which cameo an entry resolves through, and what the computer decided to do.                *
 *                                                                                             *
 *    Off unless CACDEBUG.TAG is findable, which is the same trick the LEGO Battles mod uses:    *
 *    a file's presence is a switch a player can throw without a rebuild.  Asked once.           *
 *=============================================================================================*/
static int CACLoggingOn = -1;

bool CAC_Logging(void)
{
	if (CACLoggingOn < 0) CACLoggingOn = CCFileClass("CACDEBUG.TAG").Is_Available() ? 1 : 0;
	return(CACLoggingOn != 0);
}

/*
**	Asked once -- but only once the search path can find the tag.  A line
**	logged before DLLExportClass::Set_Content_Directory had set it asked the
**	question with nowhere to look, got "no", and switched the log off for the
**	whole session; so the answer is forgotten each time the path is set.
*/
void CAC_Logging_Recheck(void)
{
	CACLoggingOn = -1;
}


/***********************************************************************************************
 * CAC_Log_Write -- One line: when, which frame, how sure, where from, and what happened.       *
 *                                                                                             *
 *    Three levels, and the middle one is the point of this.                                    *
 *                                                                                             *
 *        EVENT     -- something happened, and the code that did it is understood.              *
 *        VOLATILE  -- something happened in a part of the mod I have got wrong before, or      *
 *                     have not fully explained.  Not a warning: nothing is necessarily amiss.  *
 *                     It marks the lines worth reading first when a report comes in, and it    *
 *                     is an honest admission rather than a severity.                            *
 *        WARN      -- something is definitely not as intended.                                  *
 *                                                                                             *
 *    Wall clock as well as frame, because a report arrives as "this happened just now" and     *
 *    frame numbers do not survive that translation.                                             *
 *=============================================================================================*/
static void CAC_Log_Write(char const * kind, char const * area, char const * fmt, va_list ap)
{
	static FILE * _fp = NULL;
	static bool _tried = false;

	if (!_tried) {
		_tried = true;
		char path[512];
		char const * home = getenv("USERPROFILE");
		sprintf(path, "%s%s", home ? home : ".",
				"\\Documents\\CnCRemastered\\CAC_log.txt");
		/*
		**	Keep the last run before starting a new one.
		**	
		**	Opening "w" truncates, so every report of the form "this happened last
		**	game" arrives after the evidence for it has already been thrown away --
		**	and a crash is usually followed by launching straight back in, which is
		**	the worst possible moment to lose the previous log.  One generation back
		**	is enough and costs nothing.
		*/
		{
			char prev[512];
			sprintf(prev, "%s%s", home ? home : ".",
					"\\Documents\\CnCRemastered\\CAC_log_previous.txt");
			remove(prev);
			rename(path, prev);
		}

		_fp = fopen(path, "w");
		if (_fp == NULL) _fp = fopen("CAC_log.txt", "w");
		if (_fp != NULL) {
			/*
			**	Which build wrote this, and when the round began.
			**	
			**	The file has no date in it and is overwritten each run, so a log from an
			**	earlier playtest is indistinguishable from the current one at a glance --
			**	and a stale log is worse than none, because everything in it is plausible.
			**	That has happened here more than once.
			**	
			**	__DATE__ and __TIME__ are the moment this DLL was compiled, so the header
			**	can be checked against the build that is actually installed.  The wall
			**	clock gives the day, which the per-line clock cannot -- it carries only
			**	the time.
			*/
			char started[64];
			{
				time_t began = time(NULL);
				struct tm * bt = localtime(&began);
				if (bt != NULL) {
					sprintf(started, "%04d-%02d-%02d %02d:%02d:%02d",
							 bt->tm_year + 1900, bt->tm_mon + 1, bt->tm_mday,
							 bt->tm_hour, bt->tm_min, bt->tm_sec);
				} else {
					strcpy(started, "unknown");
				}
			}

			fprintf(_fp, "CrystAlien Conflict -- diagnostic log%s", "\n");
			fprintf(_fp, "DLL built    %s %s%s", __DATE__, __TIME__, "\n");
			fprintf(_fp, "run started  %s\n", started);
			fprintf(_fp, "%s", "\n");
			fprintf(_fp, "clock     frame  level     area        what%s", "\n");
			fprintf(_fp, "%s", "-------------------------------------------------------------\n");
		}
	}
	if (_fp == NULL) return;

	time_t now = time(NULL);
	struct tm * lt = localtime(&now);
	fprintf(_fp, "%02d:%02d:%02d  %6d  %-8s  %-10s  ",
			lt ? lt->tm_hour : 0, lt ? lt->tm_min : 0, lt ? lt->tm_sec : 0,
			(int)Frame, kind, area);
	vfprintf(_fp, fmt, ap);
	fputc('\n', _fp);

	/*
	**	Flushed every line: a log still sitting in a buffer when the game falls over
	**	is a log of everything except the interesting part.
	*/
	fflush(_fp);
}


void CAC_Log(char const * area, char const * fmt, ...)
{
	if (!CAC_Logging()) return;
	va_list ap;
	va_start(ap, fmt);
	CAC_Log_Write("EVENT", area, fmt, ap);
	va_end(ap);
}


void CAC_Log_Volatile(char const * area, char const * fmt, ...)
{
	if (!CAC_Logging()) return;
	va_list ap;
	va_start(ap, fmt);
	CAC_Log_Write("VOLATILE", area, fmt, ap);
	va_end(ap);
}


/***********************************************************************************************
 * CAC_Crash_Filter -- Write down where it fell over, since the dump does not survive.          *
 *                                                                                             *
 *    Three crashes on this project have gone undiagnosed for the same reason: the game's own   *
 *    reporter zips a dump, uploads it to a Petroglyph host that is not reachable from here,    *
 *    and deletes the zip.  By the time anyone looks there is nothing on disk but the report    *
 *    text, which carries no address and no stack.                                              *
 *                                                                                             *
 *    The diagnostic log does survive -- it is flushed every line for exactly this reason -- so  *
 *    this puts the faulting address into it.  Addresses are written as module plus offset,     *
 *    because the module is rebased on every run and a raw pointer is worthless afterwards;     *
 *    an offset into TiberianDawn.dll can be resolved against src/bin/Win32/TiberianDawn.pdb.   *
 *                                                                                             *
 *    RtlCaptureStackBackTrace rather than DbgHelp: it needs no extra library, cannot itself    *
 *    fail badly, and a list of return addresses is all that is wanted.  Frames from other      *
 *    modules are kept -- a crash inside the client called from here is worth seeing.           *
 *                                                                                             *
 *    The previous filter is chained rather than replaced, so the game still makes its own      *
 *    report and nothing a player relies on changes.                                            *
 *=============================================================================================*/
static LPTOP_LEVEL_EXCEPTION_FILTER _cac_prev_filter = NULL;

static void CAC_Describe_Address(void * addr, char * out, int size)
{
	MEMORY_BASIC_INFORMATION mbi;
	out[0] = 0;

	if (addr != NULL && VirtualQuery(addr, &mbi, sizeof(mbi)) == sizeof(mbi)) {
		char path[MAX_PATH];
		HMODULE mod = (HMODULE)mbi.AllocationBase;
		if (mod != NULL && GetModuleFileNameA(mod, path, MAX_PATH) != 0) {
			char const * leaf = strrchr(path, 92);	/* the path separator, by code: it does not survive every editor */
			leaf = (leaf != NULL) ? leaf + 1 : path;
			sprintf(out, "%s+0x%X", leaf,
					  (unsigned)((char *)addr - (char *)mod));
			return;
		}
	}
	sprintf(out, "0x%p", addr);
	size;
}


static LONG WINAPI CAC_Crash_Filter(EXCEPTION_POINTERS * info)
{
	if (info != NULL && info->ExceptionRecord != NULL) {
		char where[MAX_PATH + 32];
		CAC_Describe_Address(info->ExceptionRecord->ExceptionAddress, where, sizeof(where));

		CAC_Log("crash", "exception 0x%08X at %s, frame %d",
				  (unsigned)info->ExceptionRecord->ExceptionCode, where, (int)Frame);

		/*
		**	An access violation says which address it could not reach, and whether
		**	it was reading or writing it.  A small value is a null dereference and a
		**	very large one is usually a freed object -- which narrows it a long way
		**	before any symbol is looked up.
		*/
		if (info->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION &&
			 info->ExceptionRecord->NumberParameters >= 2) {
			CAC_Log("crash", "  %s address 0x%p",
					  (info->ExceptionRecord->ExceptionInformation[0] == 8) ? "executing"
					: (info->ExceptionRecord->ExceptionInformation[0] != 0) ? "writing" : "reading",
					  (void *)info->ExceptionRecord->ExceptionInformation[1]);
		}

		void * frames[24];
		USHORT got = RtlCaptureStackBackTrace(0, 24, frames, NULL);
		for (USHORT f = 0; f < got; f++) {
			char line[MAX_PATH + 32];
			CAC_Describe_Address(frames[f], line, sizeof(line));
			CAC_Log("crash", "  [%2d] %s", (int)f, line);
		}
	}

	if (_cac_prev_filter != NULL) return(_cac_prev_filter(info));
	return(EXCEPTION_CONTINUE_SEARCH);
}


/***********************************************************************************************
 * CAC_Vectored_Crash -- Catch it before whoever is swallowing it does.                        *
 *                                                                                             *
 *    The top-level filter did not fire on the second crash, which is itself the answer: the    *
 *    game installs its own reporter -- it writes a dump, so it must -- and whatever it uses    *
 *    runs first and never passes the exception on.  A top-level filter only ever sees what     *
 *    nobody else handled.                                                                      *
 *                                                                                             *
 *    A vectored handler registered at the front of the chain runs before all of that, on the   *
 *    first chance, so it cannot be pre-empted.  The price is that it also sees exceptions      *
 *    that are perfectly normal and handled a moment later, so:                                 *
 *                                                                                             *
 *      * C++ throws (0xE06D7363) and breakpoints are ignored outright.                        *
 *      * Only the hardware faults that actually kill a process are recorded.                  *
 *      * At most a handful are written, because a first-chance handler that logs in a loop is  *
 *        a way to fill a disk.                                                                 *
 *                                                                                             *
 *    It always returns EXCEPTION_CONTINUE_SEARCH: this observes and never handles, so the      *
 *    game's own reporter still runs and behaviour is unchanged.  A line here is a "first       *
 *    chance" report -- if the game recovers, it is noise, and the log carrying on afterwards   *
 *    says so.                                                                                  *
 *=============================================================================================*/
static LONG CALLBACK CAC_Vectored_Crash(EXCEPTION_POINTERS * info)
{
	static int _written = 0;

	if (info == NULL || info->ExceptionRecord == NULL) return(EXCEPTION_CONTINUE_SEARCH);

	DWORD code = info->ExceptionRecord->ExceptionCode;

	switch (code) {
		case EXCEPTION_ACCESS_VIOLATION:
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		case EXCEPTION_DATATYPE_MISALIGNMENT:
		case EXCEPTION_ILLEGAL_INSTRUCTION:
		case EXCEPTION_IN_PAGE_ERROR:
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
		case EXCEPTION_PRIV_INSTRUCTION:
		case EXCEPTION_STACK_OVERFLOW:
			break;
		default:
			return(EXCEPTION_CONTINUE_SEARCH);
	}

	if (_written >= 4) return(EXCEPTION_CONTINUE_SEARCH);
	_written++;

	{
		char where[MAX_PATH + 32];
		CAC_Describe_Address(info->ExceptionRecord->ExceptionAddress, where, sizeof(where));

		CAC_Log("crash", "first chance: exception 0x%08X at %s, frame %d",
				  (unsigned)code, where, (int)Frame);

		if (code == EXCEPTION_ACCESS_VIOLATION &&
			 info->ExceptionRecord->NumberParameters >= 2) {
			CAC_Log("crash", "  %s address 0x%p",
					  (info->ExceptionRecord->ExceptionInformation[0] == 8) ? "executing"
					: (info->ExceptionRecord->ExceptionInformation[0] != 0) ? "writing" : "reading",
					  (void *)info->ExceptionRecord->ExceptionInformation[1]);
		}

		/*
		**	Walk the *faulting* stack, not this handler's.
		**	
		**	RtlCaptureStackBackTrace was the obvious thing and it was useless: it walks
		**	the stack this handler is standing on, so frame zero resolved to
		**	CAC_Vectored_Crash itself and the rest was ntdll's dispatcher.  The frames
		**	that matter are on the stack as it was when the fault happened, and the
		**	frame-pointer chain is no help because the crash *is* a jump to a bad
		**	address -- there is no valid frame at the top of it.
		**	
		**	So the stack is scanned: anything on it pointing into executable memory is
		**	very likely a return address, and the first is whoever made the bad call.
		**	Some of it will be stale leftovers rather than live frames -- that is the
		**	price of scanning -- but a list containing the answer beats one that cannot.
		*/
		if (info->ContextRecord != NULL) {
			CONTEXT * ctx = info->ContextRecord;

			char at[MAX_PATH + 32];
			CAC_Describe_Address((void *)(ULONG_PTR)ctx->Eip, at, sizeof(at));
			CAC_Log("crash", "  eip %s  esp 0x%p  ebp 0x%p",
					 at, (void *)(ULONG_PTR)ctx->Esp, (void *)(ULONG_PTR)ctx->Ebp);

			/*
			**	ecx is `this` in a __thiscall, which is the register that settles whether
			**	the object was garbage or the fault was inside a perfectly good one.  The
			**	rest are cheap and occasionally decisive.
			*/
			CAC_Log("crash", "  ecx(this) 0x%p  eax 0x%p  edx 0x%p  esi 0x%p  edi 0x%p",
					 (void *)(ULONG_PTR)ctx->Ecx, (void *)(ULONG_PTR)ctx->Eax,
					 (void *)(ULONG_PTR)ctx->Edx, (void *)(ULONG_PTR)ctx->Esi,
					 (void *)(ULONG_PTR)ctx->Edi);

			if (CAC_Is_Heap_Slot((void *)(ULONG_PTR)ctx->Ecx)) {
				CAC_Log("crash", "  ecx is a heap slot, %s",
						  CAC_Is_Live_Object((void *)(ULONG_PTR)ctx->Ecx) ? "and alive" : "but dead");
			}

			ULONG_PTR * sp = (ULONG_PTR *)(ULONG_PTR)ctx->Esp;
			int shown = 0;

			for (int w = 0; w < 512 && shown < 12; w++) {
				ULONG_PTR * slot = sp + w;

				/*
				**	Every read is checked first.  A crash inside the crash handler loses
				**	the report entirely, which is the one outcome worth avoiding here.
				*/
				MEMORY_BASIC_INFORMATION smem;
				if (VirtualQuery(slot, &smem, sizeof(smem)) != sizeof(smem)) break;
				if (smem.State != MEM_COMMIT) break;

				ULONG_PTR value = *slot;
				if (value < 0x10000) continue;

				MEMORY_BASIC_INFORMATION vmem;
				if (VirtualQuery((void *)value, &vmem, sizeof(vmem)) != sizeof(vmem)) continue;
				if (vmem.State != MEM_COMMIT || vmem.AllocationBase == NULL) continue;

				/*	Executable pages only -- a data pointer is not a return address. */
				DWORD prot = vmem.Protect & 0xFF;
				if (prot != PAGE_EXECUTE && prot != PAGE_EXECUTE_READ &&
					 prot != PAGE_EXECUTE_READWRITE && prot != PAGE_EXECUTE_WRITECOPY) continue;

				char line[MAX_PATH + 32];
				CAC_Describe_Address((void *)value, line, sizeof(line));
				CAC_Log("crash", "  stack+0x%03X  %s", (unsigned)(w * sizeof(ULONG_PTR)), line);
				shown++;
			}
		}
	}

	return(EXCEPTION_CONTINUE_SEARCH);
}


void CAC_Install_Crash_Filter(void)
{
	/*
	**	Re-armable, and called again from time to time.
	**	
	**	Whoever calls SetUnhandledExceptionFilter last wins, and the game installs
	**	its own reporter -- if it does so after this one, this one never runs and the
	**	whole exercise is silent in exactly the situation it was written for.  So the
	**	current filter is read back and this is re-installed whenever something else
	**	has taken it.
	**	
	**	Reading it takes two calls because there is no getter: setting NULL returns
	**	whoever held it, and it is put straight back.  The comparison matters more
	**	than it looks -- without it this would eventually record itself as the
	**	previous filter and recurse forever on the one crash it exists to report.
	*/
	/*
	**	The vectored handler is the one that actually gets to see it; the top-level
	**	filter below is kept because it costs nothing and is the tidier of the two
	**	when nothing else is competing.
	*/
	static bool _vectored = false;
	if (!_vectored) {
		_vectored = true;
		AddVectoredExceptionHandler(1, CAC_Vectored_Crash);	/* 1 = front of the chain */
	}

	LPTOP_LEVEL_EXCEPTION_FILTER current = SetUnhandledExceptionFilter(NULL);
	SetUnhandledExceptionFilter(current);

	if (current == CAC_Crash_Filter) return;

	_cac_prev_filter = current;
	SetUnhandledExceptionFilter(CAC_Crash_Filter);
}


/***********************************************************************************************
 * CAC_Is_CrystAlien_Effect -- Is this one of the conversion's animations or decals?             *
 *                                                                                             *
 *    Not the same question as CAC_Is_CrystAlien_Type, and asking that one about an animation   *
 *    is how the death marks were quietly lost.                                                 *
 *                                                                                             *
 *    A unit or building is named for its slot and its side -- "BAGOOD", "UDEVIL" -- so that    *
 *    test is six characters, starting B or U, ending GOOD or EVIL.  An effect is named for     *
 *    what it is: CACBOOM, CACPUFF, CACSMOKE, CACSCRCH.  Seven or eight characters, starting    *
 *    C, ending in none of those.  It answers false to every one of them.                       *
 *                                                                                              *
 *    Eight, and never nine.  ObjectTypeClass carries `char IniName[9]`, so a ninth character    *
 *    is truncated on the way in -- and every art lookup in the game is that string.  CACSCORCH  *
 *    became CACSCORC, CACSPARKS became CACSPARK, CACSUPERSC became CACSUPER and collided with   *
 *    the superweapon animation.  Nothing complains: SmudgeTypeClass::Init retrieves NULL and     *
 *    Cell_Class_Draw_It skips a smudge whose Get_Image_Data is NULL without a word.  That is    *
 *    the whole of why the scorch and the mining sparks were never once seen.                    *
 *                                                                                             *
 *    So AnimClass::Middle asked "is this ours?" of CACPUFF, was told no, and stamped Tiberian   *
 *    Dawn's own SC1..SC6 and CR1 instead -- which is exactly the thing the CACScorch smudge     *
 *    was added to stop.  It resolves, so nothing draws a missing-texture square and nothing     *
 *    looks broken; the marks are simply the wrong game's.  A log of one round asks for SC1      *
 *    through SC6 and CR1 through CR5 and never once for CACSCORCH.                              *
 *                                                                                               *
 *    The unit test was tightened to the six-character form when each faction got its own        *
 *    slots, and this call went with it without anybody noticing.                                *
 *=============================================================================================*/
bool CAC_Is_CrystAlien_Effect(char const *ininame)
{
	if (ininame == NULL) return(false);
	return(strncmp(ininame, "CAC", 3) == 0);
}


/***********************************************************************************************
 * CAC_Log_Asset -- Note every distinct name the client is asked to find a texture for.         *
 *                                                                                             *
 *    The client draws a white square with a Petroglyph logo where a texture it cannot resolve  *
 *    would have gone.  Nothing in the DLL knows this happened -- it hands over a name and      *
 *    hears nothing back -- so the only way to find the culprit is to write down every name     *
 *    handed over and compare the list against what the mod actually ships.                     *
 *                                                                                             *
 *    Deduplicated, because these are called per object per frame and an undeduplicated list    *
 *    would be a gigabyte of the same forty names.                                              *
 *                                                                                             *
 *    Volatile by nature: this is the unresolved decal-on-death report, and the whole reason    *
 *    the log exists.                                                                          *
 *=============================================================================================*/
void CAC_Log_Asset(char const * area, char const * name)
{
	static char _seen[512][32];
	static int _count = 0;

	if (!CAC_Logging() || name == NULL || *name == 0) return;

	int i;
	for (i = 0; i < _count; i++) {
		if (strcmp(_seen[i], name) == 0) return;
	}
	if (_count >= 512) return;

	strncpy(_seen[_count], name, sizeof(_seen[0]) - 1);
	_seen[_count][sizeof(_seen[0]) - 1] = 0;
	_count++;

	CAC_Log_Volatile("asset", "%s asks for \"%s\"", area, name);
}


void CAC_Log_Warn(char const * area, char const * fmt, ...)
{
	if (!CAC_Logging()) return;
	va_list ap;
	va_start(ap, fmt);
	CAC_Log_Write("WARN", area, fmt, ap);
	va_end(ap);
}

bool CAC_Is_CrystAlien_Type(char const *ininame)
{
	if (ininame == NULL) return(false);

	/*
	**	A CrystAlien name is now the asset name outright -- two letters and a
	**	side, "BAGOOD" or "UDEVIL" -- because each faction has its own types.
	*/
	size_t len = strlen(ininame);
	if (len != 6) return(false);
	if (ininame[0] != 'B' && ininame[0] != 'U') return(false);
	return(strcmp(ininame + 2, "GOOD") == 0 || strcmp(ininame + 2, "EVIL") == 0);
}


/***********************************************************************************************
 * CAC_Disable_Stock_Types -- Take the surviving GDI/Nod roster out of the sidebar.            *
 *                                                                                             *
 *    The conversion re-points the type slots it needs, but Tiberian Dawn has more slots than   *
 *    CrystAlien has objects, so the leftovers still held stock units and structures -- and     *
 *    they would appear in the build list beside the CrystAlien ones.  Clearing Ownable and     *
 *    IsBuildable leaves them in the tables (scenarios and the AI may still reference them)     *
 *    while making them unbuildable, which is what a total conversion wants.                    *
 *=============================================================================================*/
void CAC_Disable_Stock_Types(void)
{
#ifdef CAC_SUPPLEMENTAL
	/*
	**	The supplemental build is the opposite proposition: CrystAlien sits in
	**	its own slots alongside GDI and Nod rather than instead of them, so the
	**	stock roster stays exactly as it is.
	*/
	return;
#else
	for (int i = 0; i < STRUCT_COUNT; i++) {
		BuildingTypeClass &t = (BuildingTypeClass &)BuildingTypeClass::As_Reference((StructType)i);
		if (!CAC_Is_CrystAlien_Type(t.IniName)) {
			t.Ownable = 0;
			t.IsBuildable = 0;
		}
	}
	for (int i = 0; i < UNIT_COUNT; i++) {
		UnitTypeClass &t = (UnitTypeClass &)UnitTypeClass::As_Reference((UnitType)i);
		if (!CAC_Is_CrystAlien_Type(t.IniName)) {
			t.Ownable = 0;
			t.IsBuildable = 0;
		}
	}
	for (int i = 0; i < INFANTRY_COUNT; i++) {
		InfantryTypeClass &t = (InfantryTypeClass &)InfantryTypeClass::As_Reference((InfantryType)i);
		if (!CAC_Is_CrystAlien_Type(t.IniName)) {
			t.Ownable = 0;
			t.IsBuildable = 0;
		}
	}
	for (int i = 0; i < AIRCRAFT_COUNT; i++) {
		AircraftTypeClass &t = (AircraftTypeClass &)AircraftTypeClass::As_Reference((AircraftType)i);
		if (!CAC_Is_CrystAlien_Type(t.IniName)) {
			t.Ownable = 0;
			t.IsBuildable = 0;
		}
	}
#endif
}


/***********************************************************************************************
 * CAC_Max_Passengers -- Carrier capacity for a CrystAlien transport.                          *
 *                                                                                             *
 *    CrystAlien sets capacity per type rather than using one figure for every transport:       *
 *    the Recon Dropship / Hyper Carrier (UH) takes 6, and the Astro-only Crystal Reaper (UR)   *
 *    takes 1.  doEmbark() refuses a load once `contents.length >= stats.capacity`.  Tiberian   *
 *    Dawn hard-codes 5 for anything flagged IsTransporter.                                    *
 *                                                                                             *
 * OUTPUT:  -1 when the type is not a CrystAlien carrier, so the caller keeps its own answer.  *
 *=============================================================================================*/
int CAC_Max_Passengers(RTTIType rtti, int type)
{
	if (rtti == RTTI_AIRCRAFTTYPE || rtti == RTTI_AIRCRAFT) {
		if (CAC_Is_Code(RTTI_AIRCRAFTTYPE, type, CAC_CODE_UH)) return(CAC_UHGOOD_CAPACITY);
	}
	if (rtti == RTTI_UNITTYPE || rtti == RTTI_UNIT) {
		if (CAC_Is_Code(RTTI_UNITTYPE, type, CAC_CODE_UR)) return(CAC_URGOOD_CAPACITY);
	}
	return(-1);
}


/***********************************************************************************************
 * CAC_Type_Index_Of_Type -- Type index for a *TypeClass rather than an instance.              *
 *=============================================================================================*/
int CAC_Type_Index_Of_Type(TechnoTypeClass const *type)
{
	if (type == NULL) return(-1);
	switch (type->What_Am_I()) {
		case RTTI_INFANTRYTYPE:	return((int)((InfantryTypeClass const *)type)->Type);
		case RTTI_UNITTYPE:		return((int)((UnitTypeClass const *)type)->Type);
		case RTTI_AIRCRAFTTYPE:	return((int)((AircraftTypeClass const *)type)->Type);
		case RTTI_BUILDINGTYPE:	return((int)((BuildingTypeClass const *)type)->Type);
		default:						return(-1);
	}
}


/***********************************************************************************************
 * CAC_Superweapon_Damage -- The EM Pulse Cannon / 4T2 Ion Disrupter blast.                    *
 *                                                                                             *
 *    Level::launchSuperweapon walks every building and every unit on the map and applies       *
 *                                                                                             *
 *        dist = sqrt(dx*dx + dy*dy)                     -- world pixels                        *
 *        if (dist < 400) pain({dmg: round((1 - dist/400) * 1000)})                             *
 *                                                                                             *
 *    Three things follow from that, and none of them match Tiberian Dawn's ion cannon:         *
 *                                                                                             *
 *      * The falloff is linear to zero at the rim, not the engine's spread curve.              *
 *      * Peak damage is 1000, against the ion cannon's 600.                                    *
 *      * There is no ownership test at all -- the blast hits the firing player's own forces    *
 *        exactly as hard as the enemy's.                                                       *
 *                                                                                             *
 *    CrystAlien measures in world pixels at 96 to the tile; C&C measures in leptons at 256 to  *
 *    the cell, so the 400px radius is 400 * 256/96 leptons.                                    *
 *=============================================================================================*/
#define CAC_SUPERWEAPON_RADIUS  ((400 * CELL_LEPTON_W) / 96)   /* 400 world pixels */
#define CAC_SUPERWEAPON_DAMAGE  1000

static void CAC_Blast(TechnoClass *victim, COORDINATE ground_zero, TechnoClass *source)
{
	if (victim == NULL || !victim->IsActive || victim->IsInLimbo) return;

	int dist = (int)Distance(ground_zero, victim->Center_Coord());
	if (dist >= CAC_SUPERWEAPON_RADIUS) return;

	int damage = (CAC_SUPERWEAPON_DAMAGE * (CAC_SUPERWEAPON_RADIUS - dist))
					 / CAC_SUPERWEAPON_RADIUS;
	if (damage <= 0) return;

	/*
	**	Distance 0 so the warhead's own falloff cannot apply on top of ours, and
	**	the uniform CrystAlien warhead so no armour class scales it either.
	*/
	CAC_Log("superweapon", "  hits %s for %d (had %d)",
			victim->Techno_Type_Class() ? victim->Techno_Type_Class()->IniName : "?",
			damage, victim->Strength);

	victim->Take_Damage(damage, 0, WARHEAD_LASER, source);
}

void CAC_Superweapon_Damage(COORDINATE ground_zero, TechnoClass *source)
{
	/*	Level.launchSuperweapon answers with INT_superweapon. */
	CAC_Log("superweapon", "fires at %d,%d",
			Cell_X(Coord_Cell(ground_zero)), Cell_Y(Coord_Cell(ground_zero)));

	/*	...direct, so it is heard wherever it lands. */
	CAC_Interface_Sound(VOC_CAC_INT_SUPERW);

	/*
	**	"this.camera.shake += 200" -- and the HUD flashes white with it.  The
	**	client owns the camera and takes a shake through ScreenShakeTime, so that
	**	much carries across; there is no channel for the flash.  Everyone feels it,
	**	because launchSuperweapon shakes the one camera the game has rather than
	**	the firing player's.
	*/
	/*
	**	And the white over the top of it.
	**
	**	launchSuperweapon opens with hud.flashthru() and there is no HUD here to put
	**	one on, so this is the nearest that can be drawn: a very large round sprite
	**	centred on the shot, which fills the view because the camera is on the target
	**	the player just clicked.  See ANIM_CAC_FLASH.
	*/
	new AnimClass(ANIM_CAC_FLASH, ground_zero);

	Shake_The_Screen(CAC_SUPERWEAPON_SHAKE);

	int i;
	/*
	**	Gather first, then hit.  Never both at once.
	**	
	**	These four loops used to call CAC_Blast straight from the walk, and CAC_Blast
	**	calls Take_Damage, which destroys.  TFixedIHeapClass compacts on delete --
	**	ActivePointers.Delete(pointer) -- so killing the object at index i moves what
	**	was at i+1 down into i, the loop then steps to i+1, and that object is never
	**	hit at all.  A superweapon dropped on a packed base therefore spared roughly
	**	every second thing it should have killed, which reads as the weapon being
	**	weak rather than as a bug.
	**	
	**	And it is the same shape as the crash this project spent a week on: walking a
	**	container that is being modified underneath.  Bounded by the re-read of
	**	Count(), so it did not run off the end -- but a cascade, where one death
	**	takes something else with it, is not bounded by anything.
	**	
	**	Targets rather than pointers, so anything that dies to an earlier blast is
	**	simply illegal by the time its turn comes.
	*/
	enum { MAX_BLAST = 256 };
	TARGET caught[MAX_BLAST];
	int hits = 0;

	#define CAC_GATHER(heap)																for (i = 0; i < (heap).Count() && hits < MAX_BLAST; i++) {							TechnoClass * o = (heap).Ptr(i);													if (o == NULL || !o->IsActive || o->IsInLimbo) continue;							if ((int)Distance(ground_zero, o->Center_Coord()) >= CAC_SUPERWEAPON_RADIUS) continue;				caught[hits++] = o->As_Target();												}

	CAC_GATHER(Buildings)
	CAC_GATHER(Units)
	CAC_GATHER(Infantry)
	CAC_GATHER(Aircraft)

	#undef CAC_GATHER

	if (hits >= MAX_BLAST) {
		CAC_Log("superweapon", "  more than %d in the blast -- the rest are spared", MAX_BLAST);
	}

	for (i = 0; i < hits; i++) {
		CAC_Blast(As_Techno(caught[i]), ground_zero, source);
	}
}


/***********************************************************************************************
 * CAC_Is_Transformer -- Does this aircraft land and keep fighting on the ground?               *
 *                                                                                             *
 *    The Switch Fighter (UP_good) and Alien Infiltrator (UP_evil) carry `transformer = true`   *
 *    in UnitStats, along with the `landing` and `ground` art banks and a 48-frame shadow that  *
 *    no other unit has.                                                                       *
 *=============================================================================================*/
bool CAC_Is_Transformer(AircraftType type)
{
	return(CAC_Is_Code(RTTI_AIRCRAFTTYPE, (int)type, CAC_CODE_UP));
}


/***********************************************************************************************
 * CAC_Lander_Step -- How fast a transformer changes altitude, in pixels per tick.              *
 *                                                                                             *
 *    UnitNav moves `landerPerc` by the unit's speed rating each tick and derives altitude from *
 *    it (`altitude = maxAltitude * (1 - landerPerc/100)`), so a speed-10 unit lands in ten     *
 *    ticks.  Tiberian Dawn steps Altitude one pixel at a time over FLIGHT_LEVEL, which would   *
 *    take 24.  Scaling the step keeps the source game's timing.                                *
 *=============================================================================================*/
int CAC_Lander_Step(AircraftClass const * craft)
{
	/*
	**	The craft's own speed, which is what the source uses.
	**
	**	    if (this.still > 20 && ...) this.landerPerc += this.parent.stats.speed;
	**
	**	landerPerc runs nought to a hundred, so a unit takes 100/speed frames to
	**	set down and the same to get up.  This had CAC_UPGOOD_SPEED written into
	**	it -- one craft's figure used for all of them -- which is right for the
	**	Dropship it was taken from and wrong for everything else.  The Hyper
	**	Carrier moves at six, so it should take seventeen frames to seat and was
	**	taking twelve: half again as fast as the source, on the heaviest thing
	**	either side flies.
	*/
	int speed = CAC_Source_Speed(craft);
	int step = (speed * 24) / 100;    /* FLIGHT_LEVEL is 24 */
	return(step < 1 ? 1 : step);
}


/***********************************************************************************************
 * CAC_Transformer_Bank -- Which art bank a transformer is currently drawn from.                *
 *                                                                                             *
 *    UnitNav sets the bank from how far through the landing it is:                             *
 *                                                                                             *
 *        if (landerPerc > 0)   bank = "landing";                                               *
 *        if (landerPerc == 100) bank = "ground";                                               *
 *                                                                                             *
 *    Altitude is the same quantity inverted, so full height is the flying bank, ground level is *
 *    the ground bank, and anything between is the landing bank.                                *
 *                                                                                             *
 * OUTPUT:  0 flying, 1 landing, 2 grounded.                                                    *
 *=============================================================================================*/
int CAC_Transformer_Bank(int altitude, int flight_level)
{
	if (altitude <= 0) return(2);
	if (altitude >= flight_level) return(0);
	return(1);
}

/***********************************************************************************************
 * CAC_Threat_Rating -- CrystAlien's own auto-target priority for a type.                      *
 *                                                                                             *
 *    Level::attack (frame_201/DoAction_22) picks a target by walking every enemy object and   *
 *    keeping the one with the highest `threat` that lies inside weapon range:                 *
 *                                                                                             *
 *        threshold = 1;                                                                       *
 *        for each enemy building  if (b.stats.threat >= threshold && dist < range) { ... }     *
 *        for each enemy unit      if (u.stats.threat >= threshold && dist < range) { ... }     *
 *                                                                                             *
 *    So threat is an absolute ranking, not Tiberian Dawn's cost-and-kills score.  A defence    *
 *    turret (8) outranks a Command Base (2) no matter what either one cost to build.          *
 *                                                                                             *
 * OUTPUT:  0 for anything that is not a CrystAlien type, so callers keep the stock scoring.   *
 *=============================================================================================*/
struct CACThreatEntry {
	RTTIType	RTTI;
	int			Type;
	int			Threat;
};

static CACThreatEntry const CACThreats[] = {
#include "CACTHREAT.INC"
};

int CAC_Threat_Rating(RTTIType rtti, int type)
{
	RTTIType want = rtti;
	if (rtti == RTTI_UNIT)      want = RTTI_UNITTYPE;
	if (rtti == RTTI_INFANTRY)  want = RTTI_INFANTRYTYPE;
	if (rtti == RTTI_AIRCRAFT)  want = RTTI_AIRCRAFTTYPE;
	if (rtti == RTTI_BUILDING)  want = RTTI_BUILDINGTYPE;

	for (int i = 0; i < (int)(sizeof(CACThreats) / sizeof(CACThreats[0])); i++) {
		if (CACThreats[i].RTTI == want && CACThreats[i].Type == type) {
			return(CACThreats[i].Threat);
		}
	}
	return(0);
}


/***********************************************************************************************
 * CAC_Target_Score -- Rank a candidate target the way CrystAlien ranks it.                    *
 *                                                                                             *
 *    Level::attack applies three rules when a unit picks its own target.  Two of them         *
 *    Tiberian Dawn already enforces, and are noted here so nobody adds them twice:            *
 *                                                                                             *
 *      "if(!(this.friend && !_loc2_.stats.weapon))" -- a player-owned unit never volunteers   *
 *      to shoot an unarmed building; the player has to order that shot.  The AI has no such   *
 *      restraint, hence the test on `friend`.  TECHNO.CPP already does exactly this:          *
 *          if (House->IsHuman && otype == RTTI_BUILDING && tclass->Primary == WEAPON_NONE)    *
 *                                                                                             *
 *      "this.stats.weapon == 'laser' && _loc2_.stats.flying" -- lasers cannot reach a flying   *
 *      target.  BULLET_LASER carries IsAntiAircraft false and ClassMissile carries it true,    *
 *      so Is_Weapon_Equipped/In_Range already rule those matchups out.                        *
 *                                                                                             *
 *    What is left is the ranking itself, and that is the part Tiberian Dawn gets wrong for     *
 *    these units: it scores by build cost plus kill count, which floats a Refinery above a     *
 *    Defence Turret.  CrystAlien ranks by a flat authored `threat`, so the turret (8) wins     *
 *    over the Command Base (2) regardless of what either cost.                                *
 *                                                                                             *
 *    (The fourth rule, "if(!_loc2_.stats.pickup)", has nothing to bite on: the crystal and     *
 *    powerup pickups are not object types in this port.)                                      *
 *                                                                                             *
 * OUTPUT:  The threat rating, or 0 for anything that is not a CrystAlien type -- callers      *
 *          then keep the stock scoring, which is what the fallback stock units want.          *
 *=============================================================================================*/
int CAC_Target_Score(TechnoClass const *object)
{
	if (object == NULL) return(0);

	TechnoTypeClass const *ttype = object->Techno_Type_Class();
	if (ttype == NULL || !CAC_Is_CrystAlien_Type(ttype->IniName)) return(0);

	return(CAC_Threat_Rating(object->What_Am_I(), CAC_Type_Index_Of_Type(ttype)));
}

struct CACCodeEntry {
	RTTIType	RTTI;
	int			Type;
	int			Code;
};

static CACCodeEntry const CACCodes[] = {
#define CAC_CODE_TABLE
#include "CACCODES.INC"
#undef CAC_CODE_TABLE
};

/***********************************************************************************************
 * CAC_Code_Of -- The CrystAlien code and side behind an engine slot.                          *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Is_Code -- Is this slot the given CrystAlien building, on either side?                   *
 *                                                                                             *
 *    Most of the engine's questions are about a *role* -- is this a headquarters, is this a    *
 *    station a fighter can dock at -- and do not care which faction built it.  Asking by code  *
 *    keeps those places from having to name both slots, and from being missed when a third    *
 *    thing is added.                                                                           *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Slot_For -- The slot a CrystAlien code occupies, on the side asked for.                  *
 *=============================================================================================*/
int CAC_Slot_For(RTTIType rtti, int letters, bool evil)
{
	int want = letters | (evil ? CAC_CODE_EVIL : 0);
	for (int i = 0; i < (int)(sizeof(CACCodes) / sizeof(CACCodes[0])); i++) {
		if (CACCodes[i].RTTI == rtti && CACCodes[i].Code == want) return(CACCodes[i].Type);
	}
	/*	A role only one faction has answers with the one that exists. */
	for (int i = 0; i < (int)(sizeof(CACCodes) / sizeof(CACCodes[0])); i++) {
		if (CACCodes[i].RTTI == rtti && (CACCodes[i].Code & ~CAC_CODE_EVIL) == letters) {
			return(CACCodes[i].Type);
		}
	}
	return(-1);
}


bool CAC_Is_Code(RTTIType rtti, int type, int letters)
{
	return((CAC_Code_Of(rtti, type) & ~CAC_CODE_EVIL) == letters);
}


int CAC_Code_Of(RTTIType rtti, int type)
{
	RTTIType want = rtti;
	if (rtti == RTTI_UNIT)      want = RTTI_UNITTYPE;
	if (rtti == RTTI_INFANTRY)  want = RTTI_INFANTRYTYPE;
	if (rtti == RTTI_AIRCRAFT)  want = RTTI_AIRCRAFTTYPE;
	if (rtti == RTTI_BUILDING)  want = RTTI_BUILDINGTYPE;

	for (int i = 0; i < (int)(sizeof(CACCodes) / sizeof(CACCodes[0])); i++) {
		if (CACCodes[i].RTTI == want && CACCodes[i].Type == type) return(CACCodes[i].Code);
	}
	return(CAC_CODE_NONE);
}


/***********************************************************************************************
 * CAC_Owns -- Does this house hold a building of exactly this code and side?                  *
 *                                                                                             *
 *    findBuilding(type, team) in the source: an exact type, owned by us, and standing -- one   *
 *    still going up has not opened anything yet.                                               *
 *=============================================================================================*/
bool CAC_Owns(HouseClass const *house, int code)
{
	if (house == NULL) return(false);

	for (int i = 0; i < Buildings.Count(); i++) {
		BuildingClass const *b = Buildings.Ptr(i);
		if (b == NULL || !b->IsActive || b->IsInLimbo) continue;
		if (b->House != house || b->Class == NULL) continue;
		/*
		**	A building still going up counts for both.  findBuilding asks only that
		**	it be `active`, and Building() sets that in its constructor, so a Solar
		**	Panel carries its load from the moment it is placed and a Defence Station
		**	draws one from then too.  Waiting for the build-up meant a base browned
		**	out for as long as its next plant took to finish -- and a brownout is
		**	exactly when turrets stop, which is what "the turret needs a plant next
		**	to it" turns out to have been: it needed the plant *finished*.
		*/
		if (CAC_Code_Of(RTTI_BUILDINGTYPE, (int)b->Class->Type) == code) return(true);
	}
	return(false);
}


/***********************************************************************************************
 * CAC_Prereq_Met -- CrystAlien's build tree, per side.                                        *
 *                                                                                             *
 *    Construction.doPrerequisites, near enough verbatim.  It turns every entry off, then       *
 *    walks the whole tree once for each side -- your own and your opponent's -- and each test  *
 *    inside asks whether you own a building of *that side's own type*:                         *
 *                                                                                             *
 *        var sides = new Array(this.parent.parent.team, this.parent.parent.oppo);              *
 *        while (i < sides.length) {                                                            *
 *            var s = sides[i];                                                                 *
 *            if (findBuilding("BA_" + s, team).friend || findBuilding("BK_" + s, team).friend) {*
 *                if (findBuilding("BA_" + s, team).friend) shortcuts["UD_" + s].active = true;  *
 *                if (findBuilding("BK_" + s, team).friend) {                                   *
 *                    shortcuts["UP_" + s].active = true;                                       *
 *                    shortcuts["BL_" + s].active = true;                                       *
 *                }                                                                             *
 *                shortcuts["BB_" + s].active = true;                                           *
 *                if (findBuilding("BB_" + s, team).friend) shortcuts["BC_" + s].active = true;  *
 *                ...                                                                           *
 *                                                                                             *
 *    Which is why capturing pays: an Astro player holding an Alien Mothership satisfies the     *
 *    Alien pass of that loop and gets Alien power plants, then Alien breeding pits, and on      *
 *    down -- a second tree beside their own, in the captured faction's own buildings.           *
 *                                                                                             *
 *    Two things a Tiberian Dawn prerequisite mask cannot say, and this can.  The root is an     *
 *    `or` -- a Command Base *or* an Ops Ship opens the tree -- and a mask is an `and`.  And     *
 *    the test is per side, whereas a mask is one number attached to a type.  So CrystAlien's    *
 *    types carry no mask at all and this stands in for them.                                    *
 *                                                                                             *
 * INPUT:   house -- who is asking.                                                            *
 *          rtti  -- what kind of thing.                                                        *
 *          type  -- which slot.                                                                *
 *                                                                                             *
 * OUTPUT:  false when the tree does not reach it.  Anything that is not CrystAlien's is left    *
 *          to the engine's own prerequisite mask and answers true here.                         *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Special_Ops_Allowed -- The skirmish menu's redeploy switch, put to a use it can have.     *
 *                                                                                             *
 *    Nothing in the conversion deploys or undeploys -- see CAC_Deploys_Into -- so "Redeployable *
 *    MCV" is a setting with nothing on the other end of it.  The client sends its value over as *
 *    CNCMultiplayerOptionsStruct::IsMCVDeploy and DLLInterface copies it into Special, so the   *
 *    switch is already readable; it was simply answering a question this game does not ask.     *
 *                                                                                             *
 *    So it asks a different one: whether the branch off the Ops Ship and the Alien Hive may be  *
 *    built at all -- the Hive and the Ops Ship themselves, the MT-201 Driller, the Alien        *
 *    Commander, and the Switch Fighter with its Alien Infiltrator.  Those five are the ones     *
 *    CrystAlien either never offers in a sidebar or offers only from that one host, so they are *
 *    the ones a player might reasonably want to play without.                                   *
 *                                                                                             *
 *    ModText.csv renames the label and its tooltip to match, which is the only part of this the *
 *    client sees.                                                                              *
 *                                                                                             *
 *    The campaign is not the skirmish menu and always allows them, and the supplemental is left *
 *    alone entirely: there the MCV really does redeploy, and the setting means what it says.    *
 *=============================================================================================*/
bool CAC_Special_Ops_Allowed(void)
{
#ifdef CAC_SUPPLEMENTAL
	return(true);
#else
	/*	A campaign scenario decides its own roster; this is a lobby switch. */
	if (GameToPlay == GAME_NORMAL) return(true);

	return(Special.IsMCVDeploy != 0);
#endif
}


/***********************************************************************************************
 * CAC_Christmas_Allowed -- The other lobby switch with nothing on the end of it.               *
 *                                                                                             *
 *    "Modern Balancing" reaches the DLL as Special.ModernBalance, and every branch that reads  *
 *    it names a Tiberian Dawn type:                                                           *
 *                                                                                             *
 *        AIRCRAFT_CARGO   -- the supply plane's spawn delay, in AircraftClass::Draw_It and     *
 *                            ::AI, and the edge it enters from in Do_Reinforcements            *
 *        STRUCT_WEAP      -- the GDI Weapons Factory at 520 strength instead of 400            *
 *        UNIT_APC         -- a Repair Bay added to its prerequisites                           *
 *                                                                                             *
 *    There is no fourth.  A conversion house can own none of those three and the roster        *
 *    contains none of them, so the setting was inert -- a switch in the lobby that changed     *
 *    nothing whatever about the game being played.                                             *
 *                                                                                             *
 *    So it asks a different question: whether the present may appear among the crates.  Level  *
 *    29 is CrystAlien's Christmas level and the only one that places any; here the present is  *
 *    the whole of the way in to Santa's Sleigh, and the Sleigh in turn to Santa and the         *
 *    reindeer.  Refusing to place it closes the branch at its root, which is exactly what a    *
 *    player who wants a Christmas-free game is asking for, and it needs no second gate on the  *
 *    build tree: CAC_Has_Santa already refuses all three until a present has been opened.      *
 *                                                                                             *
 *    ModText.csv renames the label and the Tiberian Dawn tooltip to match.  The Red Alert      *
 *    tooltip is left alone -- it is the same string in the client and not ours to rewrite.     *
 *                                                                                             *
 *    The campaign is not the skirmish menu and always allows them, or level 29 would place     *
 *    seven presents that cannot be opened.  The supplemental is left alone entirely: GDI and   *
 *    Nod are on the field there, so all three of the branches above are live and the setting   *
 *    means what it says.                                                                       *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_SpecOps_Needs_Tech -- The third lobby switch, and the third one that reached nothing.    *
 *                                                                                             *
 *    "Spawn Visceroids" arrives as Special.IsVisceroids and is read in exactly one place --    *
 *    HouseClass::AI, where a Visceroid is created from the Jurassic/creature branch once the   *
 *    build level is high enough.  That is not inert the way the other two were: a log of one   *
 *    round has `Neutral's VICE dies at 7,36`, so a Tiberian Dawn mutant really was walking     *
 *    about a Mars map.  CrystAlien has no such creature, so turning it off is the faithful     *
 *    state and refunctioning the switch costs the conversion nothing it should have had.       *
 *                                                                                             *
 *    What it asks instead is where the Special Ops branch begins.  Off, the tree is the        *
 *    source's: `case CAC_CODE_BK: return(ba);` -- the MX-81 Ops Ship and the Alien Hive        *
 *    hang straight off the headquarters, so a player can be building Switch Fighters within    *
 *    a few minutes of the first Power Plant.  On, they hang off the Technology Centre and the  *
 *    Experiment Lab instead, which themselves need a Vehicle Factory and a Radar Station, so   *
 *    the whole branch -- Switch Fighter, Alien Commander, Driller, Crystal Reaper -- arrives    *
 *    a tier later.                                                                            *
 *                                                                                             *
 *    A deliberate deviation, and it is the point of the setting: off preserves the source's    *
 *    tree exactly, on buys an early game.                                                      *
 *                                                                                             *
 *    Nothing else needs teaching.  Can_Build asks CAC_Prereq_Met for the computer as well as   *
 *    the player -- see the note there -- so the base plan simply stops allocating a Hive it    *
 *    cannot build, and goes on wanting the Technology Centre it already wanted.                *
 *                                                                                             *
 *    Campaign and supplemental are exempt, as with the other two: a scenario decides its own    *
 *    tree, and in the supplemental GDI and Nod are on the field and Visceroids are theirs.      *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Visceroids_Allowed -- Never in the conversion, whatever the flag now means.               *
 *                                                                                             *
 *    Asked of the build rather than of a house, and the first attempt at this got that wrong.  *
 *    The spawn is guarded by                                                                   *
 *                                                                                             *
 *        if (GameToPlay != GAME_NORMAL && Class->House == HOUSE_JP)                            *
 *                                                                                             *
 *    so the house doing the spawning is always the Disaster Containment Team and never a       *
 *    CrystAlien one.  Testing CAC_Is_CrystAlien_House(this) there was therefore always true    *
 *    and changed nothing at all -- the creature went on appearing exactly as before.           *
 *                                                                                             *
 *    There is no non-CrystAlien play in the conversion, so the answer is simply no.  The       *
 *    supplemental keeps it: GDI and Nod are on the field there, Spawn Visceroids still means    *
 *    what it says, and the creature is theirs.                                                  *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Purge_Visceroids -- Take off the map anything that should never have been on it.         *
 *                                                                                             *
 *    Gating HouseClass::AI's spawn was right and was not enough: they are still turning up,     *
 *    and the asset log has `object asks for "VICE"` on frame nought -- before that routine has  *
 *    rolled its dice even once.  So something else is putting them there, and reading has not   *
 *    found it: the crate payout returns into CAC_Crate_Effect before the stock switch, the      *
 *    Jurassic branch is dead because AreThingiesEnabled is false, and no map this mod ships     *
 *    names one.                                                                                *
 *                                                                                             *
 *    Two answers rather than a third guess.  This one takes them off the board whatever put     *
 *    them there -- CrystAlien has no such creature and the conversion has no honest use for     *
 *    one -- and the line it writes names the frame and the cell, so the next round says where   *
 *    they are coming from instead of leaving it to be argued about again.                       *
 *                                                                                             *
 *    Delete_This rather than a flag: a unit left in limbo is a phantom, and this project has    *
 *    already paid for that lesson once, in infantry caps full of things that were never on the  *
 *    map.                                                                                       *
 *=============================================================================================*/
void CAC_Purge_Visceroids(void)
{
	if (CAC_Visceroids_Allowed()) return;
	if ((Frame % TICKS_PER_SECOND) != 0) return;

	for (int i = Units.Count() - 1; i >= 0; i--) {
		UnitClass *u = Units.Ptr(i);
		if (u == NULL || !u->IsActive || u->Class == NULL) continue;
		if (u->Class->Type != UNIT_VICE) continue;

		CAC_Log("visceroid", "removing one at %d,%d -- house %s, frame %ld",
				  Cell_X(Coord_Cell(u->Center_Coord())),
				  Cell_Y(Coord_Cell(u->Center_Coord())),
				  (u->House != NULL && u->House->Class != NULL)
					  ? u->House->Class->IniName : "?",
				  (long)Frame);

		u->Delete_This();
	}
}


bool CAC_Visceroids_Allowed(void)
{
#ifdef CAC_SUPPLEMENTAL
	return(true);
#else
	return(false);
#endif
}


bool CAC_SpecOps_Needs_Tech(void)
{
#ifdef CAC_SUPPLEMENTAL
	return(false);
#else
	/*	A campaign scenario decides its own tree; this is a lobby switch. */
	if (GameToPlay == GAME_NORMAL) return(false);

	return(Special.IsVisceroids != 0);
#endif
}


/***********************************************************************************************
 * CAC_Miner_Stuck -- Say when a Miner has somewhere to be and is not getting there.            *
 *                                                                                             *
 *    "They end up cluttering and getting stuck on each other and the bases" is two reports.    *
 *    The first is a standoff and is fixed where it happens -- see UnitClass::Scatter, which     *
 *    refused to let a Miner move aside because it always has a destination.  The second is not  *
 *    diagnosed: a Miner that cannot path round a base looks identical from outside to one       *
 *    waiting on a blocker, and identical again to one whose crystal field has been mined out.   *
 *                                                                                             *
 *    So this measures it rather than guessing.  A Miner holding a destination it has not got    *
 *    closer to in ten seconds is stuck at something, and the line says where it is, what it is  *
 *    aimed at, and what is standing on the cell in front of it.                                 *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Idle_Watch -- Nothing of the computer's stands still for long.                          *
 *                                                                                             *
 *    CrystAlien has this rule and it is three lines.  UnitNav.handle, for a unit that is not   *
 *    getting anywhere:                                                                        *
 *                                                                                             *
 *        this.still++;                                                                        *
 *        if(this.still > 100) { delete this.path; }                                           *
 *                                                                                             *
 *    A hundred of its frames against fifteen of Tiberian Dawn's ticks to the second is about   *
 *    sixty-five.  Throwing the path away is not a nudge: the next handle() has nowhere to walk *
 *    and works the route out again from where the unit is standing now, which is what gets it  *
 *    round whatever appeared in front of it while it was walking.                              *
 *                                                                                             *
 *    Tiberian Dawn has no equivalent.  A unit whose order cannot be carried out keeps the      *
 *    order -- the Saboteur beside a building it cannot enter, the raider whose target died on  *
 *    the way, the vehicle boxed in by its own side -- and waits with it until something else   *
 *    happens to it, which for a computer is usually being shot.  "Engineers/saboteurs keep     *
 *    getting stranded" and "no bot AI unit should ever stay stationary for too long" are the   *
 *    same report.                                                                              *
 *                                                                                             *
 *    So, in two stages.  At sixty-five ticks the destination is re-issued, which is the        *
 *    source's `delete this.path` said in this engine's terms -- Assign_Destination recomputes  *
 *    from where the thing is now.  If it is still in the same cell a hundred and ninety-five   *
 *    ticks later, the order itself is the problem and it goes: the object is turned loose to be *
 *    given something else by the routines that hand out work.                                  *
 *                                                                                             *
 *    Only the computer's, and never anything with a job of its own that looks like standing    *
 *    still -- harvesting, unloading, being built, or sitting in a transport.                   *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_HQ_Unsellable -- A headquarters is not for sale, and neither is the last one.            *
 *                                                                                             *
 *    Two rules, both asked for directly:                                                      *
 *                                                                                             *
 *      - the Eagle Command Base and the Alien Mothership cannot be sold at all, whoever owns    *
 *        them.  They are where a side begins -- Construction.checkPrereq never offers either,   *
 *        so one sold is one gone for good -- and a captured one is no different;               *
 *      - the MX-81 Ops Ship and the Alien Hive can be, unless it is the only headquarters the   *
 *        house has left.  doPrerequisites opens the whole structure tree with                  *
 *                                                                                             *
 *            if (findBuilding("BA_" + s).friend || findBuilding("BK_" + s).friend)             *
 *                                                                                             *
 *        so selling the last of them sells the tree with it, and nothing puts either back.    *
 *                                                                                             *
 *    Either side's headquarters counts toward "the only one left": a captured Mothership keeps *
 *    an Astro player building exactly as it would an alien one.                                *
 *=============================================================================================*/
bool CAC_HQ_Unsellable(BuildingClass const *b)
{
	if (b == NULL || b->Class == NULL || b->House == NULL) return(false);
	if (CAC_Is_Stock_Structure(b->Class->Type)) return(false);

	int code = CAC_Code_Of(RTTI_BUILDINGTYPE, (int)b->Class->Type) & ~CAC_CODE_EVIL;
	if (code == CAC_CODE_BA) return(true);
	if (code != CAC_CODE_BK) return(false);

	for (int i = 0; i < Buildings.Count(); i++) {
		BuildingClass const *o = Buildings.Ptr(i);
		if (o == NULL || o == b || !o->IsActive || o->IsInLimbo || o->Class == NULL) continue;
		if (o->House != b->House) continue;
		int oc = CAC_Code_Of(RTTI_BUILDINGTYPE, (int)o->Class->Type) & ~CAC_CODE_EVIL;
		if (oc == CAC_CODE_BA || oc == CAC_CODE_BK) return(false);
	}
	return(true);
}


/***********************************************************************************************
 * CAC_Covered_By_Other -- Is this cell part of some other structure's footprint?               *
 *                                                                                             *
 *    The footprint the client is sent for a CrystAlien structure is its drawn one, and a tall  *
 *    structure is given the rows its artwork rises into as well -- see DLLExportClass, where    *
 *    that list is built.  A Training Camp is two cells across and one deep and fifty pixels     *
 *    tall, so it is given the row behind it.  Put an Eagle Command in that row and both of them *
 *    claim the same squares.                                                                  *
 *                                                                                             *
 *    The client resolves a shared square to one of the two and it was not the Camp: "when I    *
 *    captured the training camp and tried to sell it, it wouldn't let me, but when I clicked on *
 *    it it let me sell Eagle Command behind it".  So an added row stops where another           *
 *    structure's own footprint begins.                                                          *
 *=============================================================================================*/
bool CAC_Covered_By_Other(CELL cell, BuildingClass const *me)
{
	if ((unsigned)cell >= MAP_CELL_TOTAL) return(false);

	for (int i = 0; i < Buildings.Count(); i++) {
		BuildingClass const *o = Buildings.Ptr(i);
		if (o == NULL || o == me || !o->IsActive || o->IsInLimbo || o->Class == NULL) continue;

		short const *list = o->Class->Occupy_List();
		if (!CAC_Is_Stock_Structure(o->Class->Type) && o->Class->Overlap_List() != NULL) {
			list = o->Class->Overlap_List();
		}
		if (list == NULL) continue;

		CELL root = Coord_Cell(o->Coord);
		for (short const *c = list; *c != REFRESH_EOL; c++) {
			if ((CELL)(root + *c) == cell) return(true);
		}
	}
	return(false);
}


void CAC_Idle_Watch(HouseClass *house)
{
	if (house == NULL || house->IsHuman || !house->IsActive) return;
	if (house->Class == NULL || house->Class->House < HOUSE_MULTI1) return;

	static unsigned short _cell[3][UNIT_MAX * 4];
	static unsigned short _held[3][UNIT_MAX * 4];

	for (int pass = 0; pass < 3; pass++) {
		int total = (pass == 0) ? Units.Count()
					 : (pass == 1) ? Infantry.Count() : Aircraft.Count();

		for (int i = 0; i < total; i++) {
			FootClass *o = (pass == 0) ? (FootClass *)Units.Ptr(i)
							  : (pass == 1) ? (FootClass *)Infantry.Ptr(i)
													: (FootClass *)Aircraft.Ptr(i);
			if (o == NULL || !o->IsActive || o->IsInLimbo) continue;
			if (o->House != house) continue;
			if (i >= UNIT_MAX * 4) continue;

			/*
			**	Miners retain the behavior they had before this general idle watch was
			**	added.  Their harvesting and return missions own their state changes;
			**	the watch must not re-path, clear, or replace those errands.
			*/
			if (pass == 0 && CAC_Is_Miner(*(UnitClass *)o)) {
				_held[pass][i] = 0;
				continue;
			}

			/*
			**	Standing still on purpose is not being stuck.  Anything mid-unload or
			**	mid-build looks like a unit that has stopped, and is working.
			*/
			if (o->Mission == MISSION_UNLOAD ||
				 o->Mission == MISSION_CONSTRUCTION || o->Mission == MISSION_DECONSTRUCTION ||
				 o->Mission == MISSION_SLEEP) {
				_held[pass][i] = 0;
				continue;
			}

			/*
			**	And neither is guarding with nothing to do, which is what most of an
			**	army is doing most of the time.  Only something that has been given
			**	somewhere to be, or something to shoot, is being watched here.
			*/
			bool errand = Target_Legal(o->NavCom) || Target_Legal(o->TarCom);
			if (!errand) { _held[pass][i] = 0; continue; }

			unsigned short here = (unsigned short)Coord_Cell(o->Center_Coord());
			if (_cell[pass][i] != here) {
				_cell[pass][i] = here;
				_held[pass][i] = 0;
				continue;
			}

			if (_held[pass][i] < 0xFFF0) _held[pass][i]++;

			if (_held[pass][i] == CAC_STILL_REPATH) {
				TARGET want = o->NavCom;
				if (Target_Legal(want)) {
					o->Assign_Destination(TARGET_NONE);
					o->Assign_Destination(want);
				}
				continue;
			}

			/*
			**	One errand is allowed to take as long as it takes.
			**
			**	A passenger on MISSION_ENTER is queued for a carrier that may still be
			**	coming down, and standing there is the correct behaviour rather than a
			**	symptom.  Miners are excluded above and never reach this.
			**
			**	It still gets the re-path.  Only the order is left alone.
			*/
			bool mustkeep = (o->Mission == MISSION_ENTER);

			if (!mustkeep && _held[pass][i] >= CAC_STILL_GIVEUP) {
				_held[pass][i] = 0;

				CAC_Log_Volatile("stranded", "%s of %s has not moved from %d,%d on "
										 "mission %d -- turned loose",
										 (o->Techno_Type_Class() != NULL)
											? o->Techno_Type_Class()->IniName : "?",
										 house->Class->IniName,
										 Cell_X(here), Cell_Y(here), (int)o->Mission);

				o->Assign_Destination(TARGET_NONE);
				o->Assign_Target(TARGET_NONE);
				o->Enter_Idle_Mode();
			}
		}
	}
}


void CAC_Miner_Stuck(UnitClass *miner)
{
	if (!CAC_Logging()) return;
	if (miner == NULL || !miner->IsActive || miner->IsInLimbo) return;
	if (!CAC_Is_Miner(*miner)) return;

	static unsigned int _dist[UNIT_MAX];
	static unsigned short _held[UNIT_MAX];
	static bool _init = false;
	if (!_init) {
		_init = true;
		for (int i = 0; i < UNIT_MAX; i++) { _dist[i] = 0; _held[i] = 0; }
	}

	int id = Units.ID(miner);
	if (id < 0 || id >= UNIT_MAX) return;

	if (!Target_Legal(miner->NavCom)) {
		_held[id] = 0;
		return;
	}

	unsigned int away = (unsigned int)::Distance(miner->Center_Coord(),
															  ::As_Coord(miner->NavCom));

	if (away + 0x0040 < _dist[id] || _held[id] == 0) {
		_dist[id] = away;
		_held[id] = 1;
		return;
	}

	if (++_held[id] != (unsigned short)(TICKS_PER_SECOND * 10)) return;

	CELL ahead = Coord_Cell(Coord_Move(miner->Center_Coord(),
												  miner->PrimaryFacing.Current(), 0x0100));
	ObjectClass *front = ((unsigned)ahead < MAP_CELL_TOTAL)
								? Map[ahead].Cell_Occupier() : NULL;

	CAC_Log_Volatile("miner", "%s stuck at %d,%d for 10s -- wants %d,%d, ahead is %s",
						  miner->Class->IniName,
						  Cell_X(Coord_Cell(miner->Center_Coord())),
						  Cell_Y(Coord_Cell(miner->Center_Coord())),
						  Cell_X(::As_Cell(miner->NavCom)),
						  Cell_Y(::As_Cell(miner->NavCom)),
						  (front == NULL) ? "clear"
							: (front->What_Am_I() == RTTI_BUILDING) ? "a building"
							: (front->What_Am_I() == RTTI_UNIT) ? "a vehicle" : "something");
	_held[id] = 1;
}


/***********************************************************************************************
 * CAC_Pizza_Mode -- Which lobby mode is the pizza one, and which ones could never be.          *
 *                                                                                             *
 *    The Mode box in the skirmish screen offers five entries.  ClientG.exe builds them in one  *
 *    run and hands each an index:  0 Destroy Structures, 1 Destroy All, 2 Bases Off, 3 Capture *
 *    The Flag, 4 Mobile Headquarters -- the last added only when the Tiberian Dawn/Red Alert   *
 *    global reads Tiberian Dawn, so it is ours and not borrowed from the other game.  Five     *
 *    entries, and CNCMultiplayerOptionsStruct carries three: MPlayerBases, DestroyStructures,  *
 *    CaptureTheFlag.  There is no spare combination, so a mode of our own costs a real one.    *
 *                                                                                             *
 *    Mobile Headquarters was the tempting one and it is not available.  Tiberian Dawn has no   *
 *    such mode; it has UNIT_MHQ, a vehicle, and INI.CPP builds one in exactly one live place   *
 *    -- bases off, with IsCaptureTheFlag set, as something mobile to hang the flag on -- while *
 *    UNIT.CPP kills the side when it dies, which is the lobby's tooltip word for word.  The    *
 *    fifth entry is capture the flag with bases off, and taking it would forbid the base the   *
 *    pizza is built from and break the flag rules ported further down this file at once.      *
 *                                                                                             *
 *    Bases Off was tried and handed back.  It reads cleanly enough -- MPlayerBases clear with  *
 *    CaptureTheFlag clear is a combination nothing else sends -- but the entry does not come   *
 *    alone: choosing it greys out Credits, Tech Level, Tiberium Regrows and Allow Special Ops  *
 *    in the skirmish screen, because a units-only game has no use for them, and the client     *
 *    sends whatever the dead controls happened to show.  Credits arrive at 500 and Special Ops *
 *    arrives locked on.  The first two can be overruled from here and the third cannot, and it *
 *    is a switch this mode was asked to honour.                                               *
 *                                                                                             *
 *    So Destroy Structures, which leaves every control in the panel live and needs nothing     *
 *    overruled.  It costs the short game.  Destroy All -- the mode most games are played on -- *
 *    sits beside it untouched, which is the half of that pair worth keeping.                  *
 *                                                                                             *
 *    What the mode is: nobody can be killed, and the game ends when somebody spends fifty      *
 *    thousand credits on a Pizza Delivery and walks a unit onto the pizza it puts on the map.  *
 *                                                                                             *
 *    The campaign decides its own victory condition and the supplemental has GDI and Nod on    *
 *    the field with no pizza to their name, so both answer no before the switch is read.      *
 *=============================================================================================*/
bool CAC_Pizza_Mode(void)
{
#ifdef CAC_SUPPLEMENTAL
	return(false);
#else
	if (GameToPlay == GAME_NORMAL) return(false);

	return(Special.CAC_IsPizza != 0);
#endif
}


/***********************************************************************************************
 * CAC_Pizza_Protected -- Is this the headquarters Pizza Mode refuses to let anyone take?       *
 *                                                                                             *
 *    The Eagle Command Base and the Alien Mothership only.  The MX-81 Ops Ship and the Alien   *
 *    Hive answer `hq` to the build tree as well -- CAC_Prereq_Met's `bool hq = ba || bk;` --    *
 *    and they are deliberately *not* protected: a player who has taken the Special Ops branch  *
 *    has something worth attacking, and leaving them fair game is what stops the mode becoming *
 *    two untouchable bases staring at each other.                                              *
 *                                                                                             *
 *    Asked of a building rather than a slot so the caller need not know which faction it is    *
 *    looking at.                                                                              *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Pizza_Stipend -- Nobody is ever mined out in Pizza Mode.                                 *
 *                                                                                             *
 *    The mode's danger is a stalemate rather than a defeat.  A house cannot lose its           *
 *    headquarters, so a player who has been flattened and has spent everything still holds a   *
 *    building and still has a game to play -- but with no refinery, no miner and no credits    *
 *    there is nothing they can do with it, and the round sits there until somebody else can    *
 *    afford fifty thousand.                                                                    *
 *                                                                                             *
 *    So every house draws a wage.  Four hundred a minute is deliberately small -- a Crystal    *
 *    Miner in a couple of minutes, nowhere near a pizza on its own -- so it rebuilds an         *
 *    economy without replacing one.                                                            *
 *                                                                                             *
 *    Everyone, not only the computer: HouseClass::AI runs for human houses too, and a human     *
 *    who is mined out is exactly the case this exists for.                                     *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Pizza_Site -- Somewhere far from home to put the thing everyone is racing for.           *
 *                                                                                             *
 *    Far is the whole design.  A pizza that appeared in its buyer's base would be fifty        *
 *    thousand credits for an instant win with nothing in between; one on the far side of the   *
 *    map is fifty thousand credits for a journey somebody else may be waiting along.           *
 *                                                                                             *
 *    So: the ground a crate would be allowed on -- CAC_Crate_Ground_Ok and CAC_Crate_Reachable *
 *    between them refuse rock, crystal, water and anything walled off -- and of those, the     *
 *    cell furthest from the buyer's own buildings.  Furthest rather than random, because a     *
 *    random draw on a small map can still land next door.                                      *
 *=============================================================================================*/
static CELL CAC_Pizza_Site(HouseClass const *house)
{
	/*
	**	Somewhere random a crate would have been allowed, which is the same
	**	hundred tries MapClass::Place_Random_Crate makes.
	**
	**	This used to walk the whole cell array and pick the cell furthest from the
	**	buyer's own buildings, on the reasoning that a pizza at home is fifty
	**	thousand credits for nothing.  True, and still the wrong rule: furthest is
	**	deterministic, so every pizza on a given map landed on the same square and
	**	both sides would learn where to wait.  Random is what makes it a race
	**	rather than a rendezvous, and a pizza that happens to land near its buyer
	**	is the same luck a crate has always been able to hand somebody.
	*/
	(void)house;

	for (int tries = 0; tries < 100; tries++) {
		int x = Random_Pick(0, Map.MapCellWidth - 1);
		int y = Random_Pick(0, Map.MapCellHeight - 1);
		CELL cell = XY_Cell(Map.MapCellX + x, Map.MapCellY + y);

		CellClass const *ptr = &Map[cell];
		if (!ptr->Is_Generally_Clear()) continue;
		if (ptr->Overlay != OVERLAY_NONE) continue;
		if (!CAC_Crate_Ground_Ok(cell)) continue;
		if (!CAC_Crate_Reachable(cell)) continue;

		/*
		**	And somewhere it can actually be seen.
		**
		**	Is_Generally_Clear asks what *occupies* a cell, which is not the same
		**	question as what is drawn over one.  A CrystAlien structure stands a
		**	cell or two above the ground it occupies -- see CAC_Drawn_Rows_Above --
		**	and a tree overlaps its neighbours, so a pizza can pass every test
		**	above and still end up behind a Training Camp or under a canopy with
		**	nothing to show the player it is there.
		**
		**	Fifty thousand credits should not buy a hunt.  The same two tests the
		**	cursor uses are asked here: nothing drawn over the cell, and nothing
		**	standing on or overlapping it.
		*/
		if (CAC_Building_Over(cell) != NULL) continue;

		bool hidden = false;
		for (int over = 0; over < (int)(sizeof(ptr->Overlapper) / sizeof(ptr->Overlapper[0])); over++) {
			ObjectClass const *lap = ptr->Overlapper[over];
			if (lap != NULL && lap->IsActive) hidden = true;
		}
		if (hidden) continue;
		if (ptr->Cell_Occupier() != NULL) continue;

		return(cell);
	}
	return(0);
}


/***********************************************************************************************
 * CAC_Pizza_Deliver -- Turn a bought delivery into the pizza on the map.                       *
 *                                                                                             *
 *    The Pizza Delivery never reaches the field.  What a player buys is the delivery; what     *
 *    arrives is a pizza somewhere else entirely, and the unit that carried the order is gone   *
 *    by the time anyone could look at it.                                                      *
 *                                                                                             *
 *    Three things happen at once, and each is in the specification for a reason:               *
 *                                                                                             *
 *      the cell is marked with the buyer's house, so the overlay draws in their colour and     *
 *      so the collection test below knows whose it is;                                         *
 *                                                                                             *
 *      the ground around it is revealed to the buyer, because a prize they cannot find is not  *
 *      a prize -- and shroud is the one thing fifty thousand credits should not have to buy    *
 *      twice;                                                                                  *
 *                                                                                             *
 *      everybody else hears about it.  CrystAlien's own INT_powerwarning is the alarm it      *
 *      plays when a base browns out and its guns stop firing -- something badly wrong that     *
 *      you still have a moment to answer -- and a rival buying the win is exactly that.        *
 *=============================================================================================*/
bool CAC_Pizza_Deliver(HouseClass *house)
{
	if (!CAC_Pizza_Mode()) return(false);
	if (house == NULL || house->Class == NULL) return(false);

	CELL site = CAC_Pizza_Site(house);
	if (site == 0) {
		CAC_Log("pizza", "%s bought a delivery and there is nowhere to put it",
				  house->Class->IniName);
		return(false);
	}

	/*
	**	Which of the six boxes, which is the owner's own colour.
	**
	**	The client colours objects by their owner and leaves terrain decals alone
	**	-- CACPLATES.INC says exactly that, and an overlay is a decal as much as a
	**	baseplate is -- so the tint is baked, six times, and the cell remembers
	**	which one.  OverlayData reaches the client as ShapeIndex, so nothing has to
	**	be recoloured at draw time: the blue player's pizza is the blue box.
	**
	**	PlayerColorType runs REMAP_GOLD, LTBLUE, RED, GREEN, ORANGE, BLUE and the
	**	art is baked in that order, so the colour is its own index.
	*/
	int colour = (int)house->RemapColor;
	if (colour < 0 || colour > 5) colour = 0;

	CellClass *cell = &Map[site];
	cell->Overlay = OVERLAY_CAC_PIZZA;
	cell->OverlayData = (unsigned char)colour;
	cell->Owner = house->Class->House;
	cell->Redraw_Objects();

	/*
	**	Shown to the buyer.  Map_Cell refuses a computer house outright -- see the
	**	note in CAC_AI_Scout -- so this does nothing for one, which is right: a
	**	computer knows where it is because CAC_Pizza_Fetch is told.
	*/
	if (house->IsHuman) {
		for (int dy = -CAC_PIZZA_REVEAL; dy <= CAC_PIZZA_REVEAL; dy++) {
			for (int dx = -CAC_PIZZA_REVEAL; dx <= CAC_PIZZA_REVEAL; dx++) {
				if ((dx * dx) + (dy * dy) > (CAC_PIZZA_REVEAL * CAC_PIZZA_REVEAL)) continue;
				int x = Cell_X(site) + dx;
				int y = Cell_Y(site) + dy;
				if (x < 0 || y < 0 || x >= MAP_CELL_W || y >= MAP_CELL_H) continue;
				Map.Map_Cell(XY_Cell(x, y), house, true);
			}
		}
	}

	/*
	**	And the alarm, for everyone it is bad news for.
	*/
	if (PlayerPtr != NULL && PlayerPtr != house && !PlayerPtr->Is_Ally(house)) {
		CAC_Interface_Sound(VOC_CAC_INT_PWRWARN);
	}

	CAC_Log("pizza", "%s's pizza is delivered to %d,%d in colour %d",
			  house->Class->IniName, Cell_X(site), Cell_Y(site), colour);
	return(true);
}


/***********************************************************************************************
 * CAC_Pizza_Taken -- Somebody walked onto a pizza.  Was it theirs, and what happens?           *
 *                                                                                             *
 *    Answers false for a pizza that is not this collector's, and leaves it exactly where it    *
 *    was: an enemy walking over one neither wins nor destroys it, and does not even consume    *
 *    the pickup.  That is the rule that makes the mode a race rather than a game of denial --  *
 *    the only way to stop a pizza being collected is to stand between it and its owner.        *
 *                                                                                             *
 *    Its own owner wins outright.  Everything that is not theirs or an ally's is removed,      *
 *    which is what CrystAlien's arena.win() amounts to with more than two sides on the field.  *
 *=============================================================================================*/
bool CAC_Pizza_Taken(CellClass *cell, ObjectClass *collector)
{
	if (cell == NULL || collector == NULL) return(false);
	if (cell->Overlay != OVERLAY_CAC_PIZZA) return(false);

	HouseClass *owner = HouseClass::As_Pointer((HousesType)cell->Owner);
	HouseClass *taker = collector->Is_Techno()
							  ? ((TechnoClass *)collector)->House : NULL;

	if (owner == NULL || taker == NULL) return(false);

	/*
	**	Not yours, so nothing at all -- not a win, not a payout, and not eaten.
	*/
	if (taker != owner && !taker->Is_Ally(owner)) {
		/*
		**	Once a second at most.  Goodie_Check runs for every unit entering the
		**	cell, so a pizza in a busy place wrote this several times a frame and
		**	buried everything else in the log.
		*/
		static long _said = -99999;
		if ((long)Frame - _said > TICKS_PER_SECOND) {
			_said = (long)Frame;
			CAC_Log_Volatile("pizza", "%s walked over %s's pizza and left it there",
								  taker->Class->IniName, owner->Class->IniName);
		}
		return(false);
	}

	cell->Overlay = OVERLAY_NONE;
	cell->OverlayData = 0;
	cell->Redraw_Objects();

	TechnoTypeClass const *what = ((TechnoClass *)collector)->Techno_Type_Class();
	CAC_Log("pizza", "%s collected their pizza at %d,%d with %s (%s) -- that is the game",
			  taker->Class->IniName, Cell_X(cell->Cell_Number()),
			  Cell_Y(cell->Cell_Number()),
			  what ? what->IniName : "?",
			  taker->IsHuman ? "player" : "computer");

	/*
	**	Everyone else dies, which is the whole of arena.win() here.
	**
	**	Flag_To_Die, not Flag_To_Lose, and no Flag_To_Win at all.  IsToWin and
	**	IsToLose are only ever acted on inside `if (GameToPlay == GAME_NORMAL)` --
	**	one line in HouseClass::AI -- so in a skirmish they set a bit nobody reads.
	**	Which is exactly what happened: the pizza was collected, the pickup went,
	**	and the game carried on.
	**
	**	Flag_To_Die is the route a skirmish already uses -- Check_Pertinent_Structures
	**	ends a short game with it -- and the last house standing wins on its own
	**	without anybody being told to.
	*/
	for (int h = 0; h < HOUSE_COUNT; h++) {
		HouseClass *other = HouseClass::As_Pointer((HousesType)h);
		if (other == NULL || !other->IsActive || other == taker) continue;
		if (taker->Is_Ally(other)) continue;
		other->Flag_To_Die();
	}

	return(true);
}


/***********************************************************************************************
 * CAC_Pizza_Cell -- Where a pizza is, and whose.                                               *
 *                                                                                             *
 *    One pass of the cell array, which is cheap enough at four thousand cells and is called    *
 *    once a second per house.  Answers the first pizza belonging to `whose`, or the first      *
 *    belonging to anyone but `whose` when `theirs` is true.                                    *
 *=============================================================================================*/
CELL CAC_Pizza_Cell(HouseClass const *whose, bool theirs)
{
	if (whose == NULL || whose->Class == NULL) return(0);

	for (CELL cell = 0; cell < MAP_CELL_TOTAL; cell++) {
		if (Map[cell].Overlay != OVERLAY_CAC_PIZZA) continue;

		HouseClass *owner = HouseClass::As_Pointer((HousesType)Map[cell].Owner);
		if (owner == NULL) continue;

		bool mine = (owner == whose || whose->Is_Ally(owner));
		if (mine != theirs) return(cell);
	}
	return(0);
}


/***********************************************************************************************
 * CAC_Pizza_Errand -- Is this unit already on its way to a pizza?                              *
 *                                                                                             *
 *    So that the routines below do not re-order the runner every second, and so that the       *
 *    patrol and the aggression do not take it back off the job.                                *
 *=============================================================================================*/
bool CAC_Pizza_Errand(FootClass const *obj)
{
	if (!CAC_Pizza_Mode()) return(false);
	if (obj == NULL || !Target_Legal(obj->NavCom)) return(false);

	CELL want = ::As_Cell(obj->NavCom);
	if ((unsigned)want >= MAP_CELL_TOTAL) return(false);

	return(Map[want].Overlay == OVERLAY_CAC_PIZZA);
}


#ifdef CAC_SUPPLEMENTAL
/***********************************************************************************************
 * CAC_Owns_Yard_For -- Does this house hold a Construction Yard of that faction?            *
 *                                                                                             *
 *    A yard's faction is the ActLike of the house that built it, which capture does not      *
 *    change: GDI's yards are the Astros' way in, Nod's the Aliens', and a captured one opens  *
 *    the other faction's.                                                                     *
 *=============================================================================================*/
static bool CAC_Owns_Yard_For(HouseClass const *house, bool evil)
{
	if (house == NULL || (house->BScan & STRUCTF_CONST) == 0) return(false);
	for (int i = 0; i < Buildings.Count(); i++) {
		BuildingClass const *b = Buildings.Ptr(i);
		if (b == NULL || !b->IsActive || b->IsInLimbo || b->House != house) continue;
		if (b->Class == NULL || b->Class->Type != STRUCT_CONST) continue;
		if ((b->ActLike == HOUSE_BAD) == evil) return(true);
	}
	return(false);
}
#endif


/***********************************************************************************************
 * CAC_AI_Pizza -- The computer playing the mode it is actually in.                             *
 *                                                                                             *
 *    Three jobs, in the order they matter.                                                    *
 *                                                                                             *
 *    Its own pizza is fetched by whatever can get there soonest.  Anything that can walk onto  *
 *    a crate can take it, so the pool is every ground unit and footsoldier the house owns, and *
 *    the pick is by speed -- a Trike beats a trooper to the far side of a map by minutes, and  *
 *    minutes are what the other side has to react in.                                          *
 *                                                                                             *
 *    Somebody else's pizza is camped.  It cannot be destroyed -- that is deliberate, see the    *
 *    overlay -- and it cannot be collected by anyone but its owner, so the only defence is a   *
 *    body between the two, and the only useful body is one that can fight.  MISSION_GUARD_AREA *
 *    anchored on the pizza is exactly the stock behaviour for "sit here and shoot what comes".  *
 *                                                                                             *
 *    And it does not bother doing either outside the mode, because CAC_Pizza_Mode is false and *
 *    this returns on the first line.                                                           *
 *=============================================================================================*/
void CAC_AI_Pizza(HouseClass *house)
{
	if (!CAC_Pizza_Mode()) return;
	if (house == NULL || house->IsHuman || !house->IsActive) return;
	if (house->Class == NULL) return;

	/*
	**	Somebody actually playing.  Neutral, Special and the two campaign sides
	**	are houses like any other and were being asked to camp a pizza and to
	**	report that they had posted nobody on it, which is noise about houses that
	**	own nothing and want nothing.
	*/
	if (house->Class->House < HOUSE_MULTI1) return;

	if (((int)Frame + Houses.ID(house) * 13) % TICKS_PER_SECOND != 0) return;

	CELL mine = CAC_Pizza_Cell(house, false);
	CELL theirs = CAC_Pizza_Cell(house, true);

	if (mine == 0 && theirs == 0) return;

	/*
	**	Ours first.  Fastest thing that can stand on a crate, and only one of
	**	them -- a second runner is a unit not defending the first.
	*/
	if (mine != 0) {
		TechnoClass *runner = NULL;
		int bestspeed = -1;
		bool already = false;

		for (int pass = 0; pass < 2 && !already; pass++) {
			int total = (pass == 0) ? Units.Count() : Infantry.Count();
			for (int i = 0; i < total; i++) {
				TechnoClass *o = (pass == 0) ? (TechnoClass *)Units.Ptr(i)
													  : (TechnoClass *)Infantry.Ptr(i);
				if (o == NULL || !o->IsActive || o->IsInLimbo) continue;
				if (o->House != house || o->Strength == 0) continue;

				if (CAC_Pizza_Errand((FootClass *)o)) { already = true; break; }

				TechnoTypeClass const *type = o->Techno_Type_Class();
				if (type == NULL) continue;

				/*	A miner has a job; taking it off crystal starves the house. */
				if (pass == 0 && CAC_Is_Miner(*(UnitClass *)o)) continue;

				int speed = (int)type->MaxSpeed;
				if (speed > bestspeed) {
					bestspeed = speed;
					runner = o;
				}
			}
		}

		if (!already && runner != NULL) {
			runner->Assign_Target(TARGET_NONE);
			runner->Assign_Destination(::As_Target(mine));
			runner->Assign_Mission(MISSION_MOVE);

			CAC_Log("pizza", "%s sends %s for its pizza at %d,%d",
					  house->Class->IniName,
					  runner->Techno_Type_Class()->IniName,
					  Cell_X(mine), Cell_Y(mine));
		}
	}

	/*
	**	And somebody else's gets sat on.  Armed things only, and never the ones
	**	already doing something that matters more.
	*/
	if (theirs != 0) {
		int posted = 0;

		/*
		**	Vehicles and footsoldiers both.
		**
		**	This walked Units alone, and a game that ended with a pizza sitting out
		**	for ninety seconds posted nobody on it -- not because the routine was
		**	broken but because the houses that would have camped had been ground
		**	down to one or two things each, and what a CrystAlien computer has left
		**	at that point is infantry: a hundred and fifty Astro Troopers to a
		**	hundred Claw Tanks across that round.  "However possible" was the
		**	instruction, and a trooper standing on a pizza is a trooper the owner
		**	has to shoot.
		*/
		for (int pass = 0; pass < 2 && posted < CAC_PIZZA_CAMP; pass++) {
			int total = (pass == 0) ? Units.Count() : Infantry.Count();

			for (int i = 0; i < total && posted < CAC_PIZZA_CAMP; i++) {
				FootClass *u = (pass == 0) ? (FootClass *)Units.Ptr(i)
													: (FootClass *)Infantry.Ptr(i);
				if (u == NULL || !u->IsActive || u->IsInLimbo) continue;
				if (u->House != house || u->Strength == 0) continue;

				TechnoTypeClass const *type = u->Techno_Type_Class();
				if (type == NULL || type->Primary == WEAPON_NONE) continue;
				if (pass == 0 && CAC_Is_Miner(*(UnitClass *)u)) continue;
				if (CAC_Pizza_Errand(u)) continue;

				/*	Already posted there. */
				if (u->Mission == MISSION_GUARD_AREA && Target_Legal(u->ArchiveTarget) &&
					 ::As_Cell(u->ArchiveTarget) == theirs) {
					posted++;
					continue;
				}

				if (u->Mission == MISSION_CAPTURE || u->Mission == MISSION_ENTER) continue;

				u->Assign_Target(TARGET_NONE);
				u->Assign_Destination(::As_Target(theirs));
				u->Assign_Mission(MISSION_GUARD_AREA);
				u->ArchiveTarget = ::As_Target(theirs);
				posted++;
			}
		}

		/*
		**	Said either way.  A whole game went by with a pizza sitting on the map
		**	for ninety seconds and not one computer house posting anything on it,
		**	and "posted > 0" logging meant that looked identical to the routine
		**	never running.  Which of the two it is matters.
		*/
		CAC_Log_Volatile("pizza", "%s has %d camped on %s's pizza at %d,%d",
							  house->Class->IniName, posted,
							  HouseClass::As_Pointer((HousesType)Map[theirs].Owner)
								  ? HouseClass::As_Pointer((HousesType)Map[theirs].Owner)->Class->IniName
								  : "?",
							  Cell_X(theirs), Cell_Y(theirs));
	}
}


void CAC_Pizza_Stipend(HouseClass *house)
{
	if (!CAC_Pizza_Mode()) return;
	if (house == NULL || house->Class == NULL || !house->IsActive) return;
	if (house->IsDefeated) return;

	/*	Staggered, so six houses do not all draw on the same frame. */
	int id = (int)house->Class->House;
	if (((int)Frame + id * 7) % TICKS_PER_MINUTE != 0) return;

	house->Refund_Money(CAC_PIZZA_STIPEND);

	CAC_Log_Volatile("pizza", "%s draws its %d a minute (now %ld)",
						  house->Class->IniName, (int)CAC_PIZZA_STIPEND,
						  house->Available_Money());
}


bool CAC_Pizza_Protected(BuildingClass const *b)
{
	if (!CAC_Pizza_Mode()) return(false);
	if (b == NULL || b->Class == NULL) return(false);
	if (!CAC_Is_CrystAlien_Type(b->Class->IniName)) return(false);

	int code = CAC_Code_Of(RTTI_BUILDINGTYPE, (int)*b) & ~CAC_CODE_EVIL;

	return(code == CAC_CODE_BA);
}


bool CAC_Christmas_Allowed(void)
{
#ifdef CAC_SUPPLEMENTAL
	return(true);
#else
	/*	A campaign scenario decides its own pickups; this is a lobby switch. */
	if (GameToPlay == GAME_NORMAL) return(true);

	return(Special.ModernBalance != 0);
#endif
}


bool CAC_Prereq_Met(HouseClass const *house, RTTIType rtti, int type)
{
	if (house == NULL) return(false);

	int code = CAC_Code_Of(rtti, type);
	if (code == CAC_CODE_NONE) return(true);

	bool evil = ((code & CAC_CODE_EVIL) != 0);

	/*
	**	And the lobby's own switch, before any of the tree is consulted.
	**	See CAC_Special_Ops_Allowed -- five types hang off this one setting.
	*/
	if (!CAC_Special_Ops_Allowed()) {
		switch (code & ~CAC_CODE_EVIL) {
			case CAC_CODE_BK:		/* the Ops Ship and the Alien Hive */
			case CAC_CODE_BL:		/* the MT-201 Driller */
			case CAC_CODE_UQ:		/* the Alien Commander */
			case CAC_CODE_UP:		/* the Switch Fighter and the Alien Infiltrator */
			case CAC_CODE_UR:		/* the Crystal Reaper */
				return(false);

			default:
				break;
		}
	}

	/*
	**	"findBuilding(type, team).friend" -- one of that exact type, owned by us.
	*/
	#define OWN(letters) CAC_Owns(house, CAC_CODE_##letters | (evil ? CAC_CODE_EVIL : 0))

	/*
	**	Either headquarters will do, for everything the tree hangs off one.
	**
	**	doPrerequisites opens the whole structure branch with
	**
	**	    if (findBuilding("BA_" + s).friend || findBuilding("BK_" + s).friend)
	**
	**	so a player who loses the Eagle Command Base but still holds an Ops Ship --
	**	or loses the Alien Mothership but still holds a Hive -- goes on building.
	**	That is what makes the second one worth putting up.
	**
	**	Three things are not shared, and they are the three the source names
	**	inside that block rather than outside it: the Crystal Miner comes off the
	**	Command Base alone, and the Switch Fighter and the Driller off the Ops Ship
	**	alone.
	*/
	bool ba = OWN(BA);
	bool bk = OWN(BK);

	bool hq = ba || bk;

	switch (code & ~CAC_CODE_EVIL) {

		/*
		**	The headquarters.
		**
		**	In the total conversion it is never in a sidebar: it arrives with the
		**	player, and an MCV unfolds into one.
		**
		**	The supplemental build is the other way round.  GDI and Nod keep their
		**	own Construction Yard and their own MCV, so the only way to reach
		**	CrystAlien is to be sold a headquarters by a yard.
		**
		**	A yard sells its own faction's: a GDI yard the Eagle Command Base, a Nod
		**	yard the Alien Mothership -- the pairing make_missions and CAC_Side use
		**	everywhere else.  The other faction's comes with a captured yard, which
		**	keeps the ActLike of whoever built it.  This sold both to anyone from
		**	August, and on trying it the report was "both teams have immediate
		**	access to the Mothership and Eagle Command tech trees".
		*/
#ifdef CAC_SUPPLEMENTAL
		case CAC_CODE_BA:	return(CAC_Owns_Yard_For(house, evil));
#else
		case CAC_CODE_BA:	return(false);
#endif
		/*
		**	The Ops Ship and the Alien Hive, off the headquarters -- or off the
		**	Technology Centre when the lobby asked for that.  See
		**	CAC_SpecOps_Needs_Tech.  BG needs the headquarters itself, so requiring it
		**	is strictly a tier further up rather than a different branch.
		*/
		case CAC_CODE_BK:
			if (CAC_SpecOps_Needs_Tech()) return(hq && OWN(BG));
			return(ba);

		case CAC_CODE_BB:	return(hq);
		case CAC_CODE_BC:	return(hq && OWN(BB));

		case CAC_CODE_BD:
		case CAC_CODE_BE:
		case CAC_CODE_BF:	return(hq && OWN(BC));

		case CAC_CODE_BG:	return(hq && OWN(BE) && OWN(BF));
		case CAC_CODE_BH:	return(hq && OWN(BE) && OWN(BF) && OWN(BG));

		/*
		**	The MT-201 Driller is the Astros', and that is the source's rule rather
		**	than this conversion's -- which an earlier note here got wrong.
		**
		**	doPrerequisites hands it to whichever side owns a BK:
		**
		**	    if (findBuilding("BK_" + s).friend) {
		**	        shortcuts["UP_" + s].active = true;
		**	        shortcuts["BL_" + s].active = true;
		**	    }
		**
		**	and then, once both sides have been walked, takes it back off one of them:
		**
		**	    this.shortcuts.BL_evil.active = false;
		**
		**	unconditionally, as the last thing the routine does.  The Aliens have no
		**	passive income at all -- only what the Crystal Extractor carries home --
		**	and they have the Commander, which the Astros do not.  They earn the rest
		**	by raiding: see CAC_AI_Infiltrate, which sends Saboteurs at exactly these.
		*/
		/*
		**	And the Driller stays behind the Ops Ship on either setting.
		**
		**	It was briefly moved to the headquarters when the tier switch is on,
		**	because gating BK takes the Astros' passive income up a tier with it and
		**	a round's log has every computer house pinned between nought and six
		**	hundred credits with no Driller at any point.  That reading of the cost
		**	was right and the remedy was not mine to choose: the Driller hangs off
		**	the Ops Ship in the source and it hangs off it here, switch or no switch.
		**
		**	So the tier setting really does delay the Astro economy as well as the
		**	Special Ops branch.  That is what it costs, and it is deliberate.
		*/
		case CAC_CODE_BL:	return(bk && !evil);

		/*
		**	Either headquarters.  See CAC_Hosts_Unit -- the Eagle Command cannot be
		**	rebuilt, so pinning the Miner to it makes losing one final, and for the
		**	Aliens, who have no Driller, final means no income at all.
		*/
		case CAC_CODE_UD:	return(hq);

		/*
		**	The Commander, from the Alien Hive.
		**
		**	checkPrereq never lists it -- in the source it arrives in a capsule as a
		**	trap and is never built.  A skirmish has no campaign to hand it out, so
		**	the conversion sells it from the Hive, one to a player.  That is this
		**	conversion's decision, not CrystAlien's.
		*/
		case CAC_CODE_UQ:	return(bk);
		case CAC_CODE_UP:	return(bk);

		case CAC_CODE_UF:	return(hq && OWN(BE) && OWN(BF));
		case CAC_CODE_UK:	return(hq && OWN(BE) && OWN(BF) && OWN(BG) && OWN(BH));

		case CAC_CODE_UA:
		case CAC_CODE_UB:	return(OWN(BC));
		case CAC_CODE_UC:	return(OWN(BC) && OWN(BG));

		case CAC_CODE_UE:	return(OWN(BE));
		case CAC_CODE_UR:	return(OWN(BE) && OWN(BL));

		case CAC_CODE_UG:	return(OWN(BF));
		case CAC_CODE_UH:	return(OWN(BF) && OWN(BE) && OWN(BG));

		/*
		**	The Christmas grotto and what lives in it, which no sidebar in the
		**	source offers at all -- see CAC_Has_Santa for the way in.
		*/
		case CAC_CODE_BJ:
		case CAC_CODE_UM:
		case CAC_CODE_UN:	return(true);

		/*
		**	The Pizza Delivery, which exists in one game mode and nowhere else.
		**
		**	Answering false outside Pizza Mode is what keeps the whole feature from
		**	bleeding: Can_Build asks this for the computer as readily as for the
		**	player, so in any other mode the type is not merely hidden from a
		**	sidebar -- nothing can order it, the base plan cannot want it, and the
		**	cap and host tables are never consulted.
		**
		**	No prerequisite inside the mode, by design.  It is hosted at the
		**	headquarters (see CACHOSTS.INC) so it is on offer from the first
		**	minute with nothing to build first, which is what makes it a race
		**	rather than a tech path.
		*/
		case CAC_CODE_UJ:	return(CAC_Pizza_Mode());

		/*
		**	Level decoration, and the Claw Tank, which reach the field by being
		**	placed or handed out by a pod rather than built.
		*/
		default:			return(false);
	}

	#undef OWN
}


/***********************************************************************************************
 * CAC_Has_Santa -- Has this house opened a gift yet?                                          *
 *                                                                                             *
 *    Level 29 is CrystAlien's Christmas level: it hands the player Santa's Sleigh, one Santa   *
 *    and eight reindeer, and asks them to collect seven capsules.  Nothing else in the game    *
 *    places any of the three, and no sidebar ever offers them, so outside that one level the   *
 *    content is unreachable -- which is why this conversion adds a way in.                     *
 *                                                                                             *
 *    That way is the gift: one extra outcome on the Alien capsule's table, and the only        *
 *    payout here that CrystAlien does not have.  Opening one drops a reindeer where the        *
 *    capsule stood and puts the Sleigh in the finder's sidebar; the Sleigh then hosts Santa    *
 *    and further reindeer, matching the arrangement level 29 sets up.                          *
 *                                                                                             *
 * INPUT:   house -- Who is asking.                                                            *
 *          rtti  -- What kind of slot.                                                        *
 *          type  -- Which slot.                                                               *
 *                                                                                             *
 * OUTPUT:  false only for a Christmas slot belonging to a house that has not found a gift.    *
 *=============================================================================================*/
bool CAC_Has_Santa(HouseClass const *house, RTTIType rtti, int type)
{
	bool christmas = CAC_Is_Code(rtti, type, CAC_CODE_BJ)
	              || CAC_Is_Code(rtti, type, CAC_CODE_UM)
	              || CAC_Is_Code(rtti, type, CAC_CODE_UN);

	if (!christmas) return(true);

	if (house == NULL || house->Class == NULL) return(false);

	/*
	**	The half that matches the house, which is how everything else is filtered.
	**
	**	Nothing anywhere keeps a faction roster -- CACROSTER.INC generated empty
	**	and is read by no one -- because the tree does the job on its own: a slot
	**	carries its own side, CAC_Prereq_Met derives `evil` from that, and OWN()
	**	then asks whether the house holds the same-side prerequisite.  An Astro
	**	house never satisfies an Alien slot because it does not own an Alien
	**	headquarters.
	**
	**	The sleigh is the one build with no prerequisite at all -- `case
	**	CAC_CODE_BJ: return(true);` -- so nothing filtered it, and both halves of
	**	each pair were offered to whoever opened a present: an Astro sleigh and an
	**	Alien one, side by side in the same sidebar.
	**
	**	Requiring the Alien half fixed the duplicate and took the sleigh away from
	**	Astro players altogether, which is not what was wanted: the artwork is
	**	Alien-aligned, the build is not.  Either side may have Santa if it finds
	**	the present -- so ask the question the tree would have asked, and offer
	**	each house the half that belongs to it.
	*/
	bool evilslot  = ((CAC_Code_Of(rtti, type) & CAC_CODE_EVIL) != 0);
	bool evilhouse = (CAC_Side(house->Class->House) == HOUSE_BAD);

	if (evilslot != evilhouse) return(false);

	/*
	**	The sleigh is the crew's factory, so owning one is what grants them.
	**
	**	UnitStats gives Santa and the reindeer `home: BJ` -- they are built at the
	**	sleigh the way a Trike is built at a Vehicle Factory, and CACHOSTS.INC says
	**	the same.  Gating all three on the present instead meant capturing an
	**	enemy's sleigh handed over a building that could not make the only two
	**	things it exists to make, because the flag it was really asking about
	**	belongs to whoever opened the present rather than to whoever holds the
	**	sleigh now.
	**
	**	So the present opens the sleigh, and the sleigh opens its crew.
	*/
	if (CAC_Is_Code(rtti, type, CAC_CODE_UM) || CAC_Is_Code(rtti, type, CAC_CODE_UN)) {
		return(CAC_Owns(house, CAC_CODE_BJ | (evilhouse ? CAC_CODE_EVIL : 0)));
	}

	/*
	**	Said once per house, ever.
	**
	**	One static for all of them, again -- and Can_Build runs the whole roster on
	**	every sidebar recalculation, for every house, so six players wrote eight
	**	thousand nine hundred lines of this in five minutes and buried everything
	**	else.  Third time for this mistake: a static that is not indexed by
	**	whatever varies is not a limit at all.
	*/
	if (CAC_Logging() && house->CACSanta == 0) {
		static bool _said[HOUSE_COUNT];
		static bool _init = false;
		if (!_init) { _init = true; for (int h = 0; h < HOUSE_COUNT; h++) _said[h] = false; }

		int hi = (int)house->Class->House;
		if ((unsigned)hi < HOUSE_COUNT && !_said[hi]) {
			_said[hi] = true;
			CAC_Log_Volatile("santa", "%s cannot build the sleigh -- no present opened yet",
								 house->Class->IniName);
		}
	}

	return(house->CACSanta != 0);
}


/***********************************************************************************************
 * CAC_Unlock_Santa -- Open the Christmas branch for a house.                                  *
 *                                                                                             *
 *    Called from the gift payout.  Sidebar entries are recomputed from Can_Build whenever a    *
 *    building finishes or changes hands, so the flag alone is not enough to make the Sleigh    *
 *    appear -- the sidebar has to be told to look again, which is what Recalc does.           *
 *=============================================================================================*/
void CAC_Unlock_Santa(HouseClass *house)
{
	if (house == NULL || house->CACSanta) return;
	house->CACSanta = 1;
	house->IsRecalcNeeded = true;
}


/***********************************************************************************************
 * CAC_Crate_Effect -- Collect a CrystAlien pickup.                                            *
 *                                                                                             *
 *    CrystAlien's pickups are Unit records with "pickup" set: no weapon, no obstruction, and  *
 *    checkForHit returns false for them, so they cannot be shot -- they just sit on a tile     *
 *    until something drives over it.  UnitNav::checkCollect then calls Unit::collect, which    *
 *    hands out whatever the level author stored in the pickup's `freeInside`:                 *
 *                                                                                             *
 *        if (pickup == "special")           parent.win()                                      *
 *        else if (freeInside == "powerup")  charge -= 1 (min 2); range += 25;                 *
 *                                           maxHealth += 50; health = maxHealth               *
 *        else if (!freeInside || number)    cash += (freeInside || 2000)                      *
 *        else                               spawn Unit(freeInside) at the nearest free tile,   *
 *                                           for the finder -- or the enemy if the name ends    *
 *                                           "_oppo"                                           *
 *                                                                                             *
 *    There are two of them and they are not interchangeable.  Across the campaign's 109        *
 *    placements the crystal box holds a free unit half the time and cash the rest, and never   *
 *    a powerup; the Alien capsule is almost the opposite -- two thirds powerup, and otherwise  *
 *    ten thousand credits or a Claw Tank handed to the *enemy*.  So the box is the wooden      *
 *    crate and the capsule the steel one, each with its own table.                            *
 *                                                                                             *
 *    Two payouts do not carry across.  The instant-win pickup is a mission objective an        *
 *    author places, never a random crate.  And "maxHealth += 50" has no per-object home in     *
 *    Tiberian Dawn, where max strength is read from the shared type -- so the powerup gives    *
 *    the range, the reload and the full heal, and leaves the ceiling alone.                    *
 *                                                                                             *
 * INPUT:   object -- Whoever drove onto the crate.                                            *
 *          cell   -- Where it is.                                                             *
 *          steel  -- True for the Alien capsule, false for the crystal box.                   *
 *                                                                                             *
 * OUTPUT:  true if something now occupies that cell and the finder must not enter it.         *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Free_Cell -- Somewhere to put what a crate was holding.                                 *
 *                                                                                             *
 *    Unit.collect places what it finds "at the nearest free tile", and the crate's own tile   *
 *    is never free -- whoever opened it is standing on it.  Placing there simply failed, so a  *
 *    crate holding a unit looked like a crate holding nothing.                                 *
 *=============================================================================================*/
static CELL CAC_Free_Cell(CELL cell)
{
	if (Map[cell].Cell_Occupier() == NULL) return(cell);
	CELL nearby = Map.Nearby_Location(cell);
	return((nearby != 0) ? nearby : cell);
}


bool CAC_Crate_Effect(FootClass *object, CELL cell, CACPickup kind)
{
	enum {
		CASH_500, CASH_1000, CASH_1200, CASH_1500, CASH_2000, CASH_8000, CASH_10000,
		POWERUP, FREE_UNIT, ENEMY_UNIT, GIFT
	};

	/*
	**	What a crystal box holds.
	**
	**	The campaign's boxes are mostly money -- twenty-one at two thousand, and
	**	the rest spread over amounts its authors reached for -- with fifty-two
	**	free units among them.  A skirmish is not a campaign: money from the
	**	ground undoes the economy the miners are there to run, so a box hands
	**	over a unit and nothing else.
	**
	**	That is this conversion's decision, not CrystAlien's.
	*/
	static unsigned char const _box[] = {
		FREE_UNIT
	};

	/*
	**	And what an Alien capsule holds: money, which is the one thing the boxes
	**	no longer do.  Ten thousand is the figure the source puts in them.  A
	**	capsule is one crate in nine, so it stays a windfall rather than an income.
	*/
	static unsigned char const _capsule[] = {
		CASH_10000
	};

	/*
	**	And the present, which is the whole of the way in to Santa's Sleigh.  It
	**	looks like a present and it is one; it used to be one chance in twelve
	**	inside a wooden box, while its artwork sat on the capsule paying out cash.
	*/
	static unsigned char const _gift[] = {
		GIFT
	};

	/*
	**	What the free-unit boxes hold, one entry per placement across the campaign
	**	-- fifty-two of them, counted out of the level data.
	**
	**	The side comes with the type, and that is the point of this table.
	**	Unit.collect reads `freeInside` as a full type name -- "UE_evil", not "UE"
	**	-- and hands it to the finder:
	**
	**	    var team = this.parent.parent.team;
	**	    if (this.freeInside.substr(-5) == "_oppo") { ...; team = oppo; }
	**	    new Unit(this.parent, this.freeInside, x, y, null, team);
	**
	**	so the type names one faction's machine and the team names who ends up
	**	driving it.  An Astro player who opens a box can walk away with an Alien
	**	Speeder that fights for them, and the campaign leans on it: twenty-seven of
	**	the fifty-two are Astro and twenty-five Alien, and the single commonest
	**	entry is UE_evil at nine.
	**
	**	This used to convert whatever came up to the finder's own side, which threw
	**	away half of what makes a box worth opening.
	*/
	static struct { RTTIType RTTI; int Code; bool Evil; } const _freeunit[] = {
		{RTTI_UNITTYPE,	CAC_CODE_UE,	true},	/* UE_evil */
		{RTTI_UNITTYPE,	CAC_CODE_UE,	true},	/* UE_evil */
		{RTTI_UNITTYPE,	CAC_CODE_UE,	true},	/* UE_evil */
		{RTTI_UNITTYPE,	CAC_CODE_UE,	true},	/* UE_evil */
		{RTTI_UNITTYPE,	CAC_CODE_UE,	true},	/* UE_evil */
		{RTTI_UNITTYPE,	CAC_CODE_UE,	true},	/* UE_evil */
		{RTTI_UNITTYPE,	CAC_CODE_UE,	true},	/* UE_evil */
		{RTTI_UNITTYPE,	CAC_CODE_UE,	true},	/* UE_evil */
		{RTTI_UNITTYPE,	CAC_CODE_UE,	true},	/* UE_evil */
		{RTTI_UNITTYPE,	CAC_CODE_UF,	false},	/* UF_good */
		{RTTI_UNITTYPE,	CAC_CODE_UF,	false},	/* UF_good */
		{RTTI_UNITTYPE,	CAC_CODE_UF,	false},	/* UF_good */
		{RTTI_UNITTYPE,	CAC_CODE_UF,	false},	/* UF_good */
		{RTTI_UNITTYPE,	CAC_CODE_UF,	false},	/* UF_good */
		{RTTI_UNITTYPE,	CAC_CODE_UF,	false},	/* UF_good */
		{RTTI_UNITTYPE,	CAC_CODE_UF,	false},	/* UF_good */
		{RTTI_UNITTYPE,	CAC_CODE_UF,	false},	/* UF_good */
		{RTTI_UNITTYPE,	CAC_CODE_UF,	false},	/* UF_good */
		{RTTI_INFANTRYTYPE,	CAC_CODE_UA,	true},	/* UA_evil */
		{RTTI_INFANTRYTYPE,	CAC_CODE_UA,	true},	/* UA_evil */
		{RTTI_INFANTRYTYPE,	CAC_CODE_UA,	true},	/* UA_evil */
		{RTTI_INFANTRYTYPE,	CAC_CODE_UA,	true},	/* UA_evil */
		{RTTI_INFANTRYTYPE,	CAC_CODE_UA,	true},	/* UA_evil */
		{RTTI_INFANTRYTYPE,	CAC_CODE_UA,	true},	/* UA_evil */
		{RTTI_INFANTRYTYPE,	CAC_CODE_UA,	true},	/* UA_evil */
		{RTTI_AIRCRAFTTYPE,	CAC_CODE_UC,	false},	/* UC_good */
		{RTTI_AIRCRAFTTYPE,	CAC_CODE_UC,	false},	/* UC_good */
		{RTTI_AIRCRAFTTYPE,	CAC_CODE_UC,	false},	/* UC_good */
		{RTTI_AIRCRAFTTYPE,	CAC_CODE_UC,	false},	/* UC_good */
		{RTTI_AIRCRAFTTYPE,	CAC_CODE_UC,	false},	/* UC_good */
		{RTTI_AIRCRAFTTYPE,	CAC_CODE_UC,	false},	/* UC_good */
		{RTTI_AIRCRAFTTYPE,	CAC_CODE_UC,	false},	/* UC_good */
		{RTTI_INFANTRYTYPE,	CAC_CODE_UB,	true},	/* UB_evil */
		{RTTI_INFANTRYTYPE,	CAC_CODE_UB,	true},	/* UB_evil */
		{RTTI_INFANTRYTYPE,	CAC_CODE_UB,	true},	/* UB_evil */
		{RTTI_UNITTYPE,	CAC_CODE_UE,	false},	/* UE_good */
		{RTTI_UNITTYPE,	CAC_CODE_UE,	false},	/* UE_good */
		{RTTI_UNITTYPE,	CAC_CODE_UE,	false},	/* UE_good */
		{RTTI_UNITTYPE,	CAC_CODE_UF,	true},	/* UF_evil */
		{RTTI_UNITTYPE,	CAC_CODE_UF,	true},	/* UF_evil */
		{RTTI_UNITTYPE,	CAC_CODE_UF,	true},	/* UF_evil */
		{RTTI_INFANTRYTYPE,	CAC_CODE_UA,	false},	/* UA_good */
		{RTTI_INFANTRYTYPE,	CAC_CODE_UA,	false},	/* UA_good */
		{RTTI_INFANTRYTYPE,	CAC_CODE_UB,	false},	/* UB_good */
		{RTTI_INFANTRYTYPE,	CAC_CODE_UB,	false},	/* UB_good */
		{RTTI_AIRCRAFTTYPE,	CAC_CODE_UC,	true},	/* UC_evil */
		{RTTI_AIRCRAFTTYPE,	CAC_CODE_UC,	true},	/* UC_evil */
		{RTTI_AIRCRAFTTYPE,	CAC_CODE_UP,	false},	/* UP_good */
		{RTTI_AIRCRAFTTYPE,	CAC_CODE_UP,	false},	/* UP_good */
		{RTTI_UNITTYPE,	CAC_CODE_UD,	true},	/* UD_evil */
		{RTTI_UNITTYPE,	CAC_CODE_UD,	false},	/* UD_good */
		{RTTI_UNITTYPE,	CAC_CODE_UR,	false},	/* UR_good */
	};

	static int const _cash[] = {500, 1000, 1200, 1500, 2000, 8000, 10000};

	if (object == NULL || object->House == NULL) return(false);

	COORDINATE coord = ::Cell_Coord(cell);
	HousesType owner = object->Owner();

	/*
	**	"this.makeNoise('INT_collect')" -- one cue, whatever the pickup held, and
	**	makeNoise means only when it is on screen.  The two ungated INT_collect
	**	calls in the source are the splash screen answering a cheat code, not
	**	this.  The draw below stays outside the test: it decides what the pickup
	**	holds, which is not a matter of who is looking.
	*/
	if (Map.In_View(Coord_Cell(coord))) {
		CAC_Interface_Sound(VOC_CAC_INT_COLLECT, coord);
	}

	unsigned char const *table = _box;
	int count = (int)(sizeof(_box) / sizeof(_box[0]));
	if (kind == CAC_PICKUP_CAPSULE) {
		table = _capsule;
		count = (int)(sizeof(_capsule) / sizeof(_capsule[0]));
	} else if (kind == CAC_PICKUP_GIFT) {
		table = _gift;
		count = (int)(sizeof(_gift) / sizeof(_gift[0]));
	}
	int what = table[Sim_Random_Pick((unsigned)0, (unsigned)(count - 1))];

	switch (what) {
		case POWERUP:
			new AnimClass(ANIM_CRATE_INVUN, coord);
			if (object->CACBoost < CAC_POWERUP_MAX) object->CACBoost++;
			object->Strength = object->Class_Of().MaxStrength;
			CAC_Speak(object, CAC_VOICE_COMPLY);
			break;

		case GIFT: {
			/*
			**	Santa's Sleigh in the sidebar from here on, and nothing standing on
			**	the ground: the cast is built at the Sleigh rather than handed out.
			**
			**	A player who already has it gets what an Alien capsule would have
			**	paid, so a second gift is never a wasted pickup.
			*/
			HouseClass *finder = HouseClass::As_Pointer(owner);

			CAC_Log_Volatile("santa", "%s (%s) opened a present at %d,%d -- had santa=%d",
									 finder ? finder->Class->IniName : "?",
									 (finder && finder->IsHuman) ? "player" : "computer",
									 Cell_X(cell), Cell_Y(cell),
									 finder ? (int)finder->CACSanta : -1);

			if (finder != NULL && finder->CACSanta != 0) {
				finder->Refund_Money(10000);
				new AnimClass(ANIM_CRATE_DOLLAR, coord);
				return(true);
			}

			CAC_Unlock_Santa(finder);
			new AnimClass(ANIM_CRATE_INVUN, coord);
			return(true);
		}

		case FREE_UNIT:
		case ENEMY_UNIT: {
			/*
			**	The two "_oppo" entries in the campaign spawn for the opponent rather
			**	than the finder, and both are "UQ_evil_oppo" -- an Alien Commander that
			**	turns up fighting for the other side.
			**
			**	Which is infantry, and this read it out of the vehicle table: the
			**	Commander moved rosters and this line did not follow.  Unreachable
			**	since the box table stopped offering it, which is the only reason it
			**	has not gone wrong yet.  CAC_Place_Code works the kind out for itself.
			*/
			if (what == ENEMY_UNIT) {
				if (object->House->Enemy == HOUSE_NONE) return(false);
				return(CAC_Place_Code(CAC_CODE_UQ, true,
											CAC_Free_Cell(cell), object->House->Enemy));
			}

			/*
			**	"spawn Unit(freeInside) at the nearest free tile" -- and the crate's
			**	own tile is not free: whoever opened it is standing on it.  Placing
			**	there simply failed, which is why a crate holding a unit appeared to
			**	hold nothing at all.
			*/
			CELL where = CAC_Free_Cell(cell);

			/*
			**	And if it cannot be delivered, the crate pays instead.
			**
			**	CAC_Place_Code answers false when the type is at its `max` or the object
			**	list is full, and that answer was the whole of what happened: the crate
			**	vanished and nothing came out of it.  A pickup that gives nothing reads
			**	as broken however good the reason is.
			**
			**	So the roster is tried in turn from a random start, and if every one of
			**	them is capped the finder gets what a capsule would have paid.
			*/
			int const kinds = (int)(sizeof(_freeunit) / sizeof(_freeunit[0]));
			int pick = (int)Sim_Random_Pick((unsigned)0, (unsigned)(kinds - 1));

			for (int tries = 0; tries < kinds; tries++) {
				int k = (pick + tries) % kinds;
				if (CAC_Place_Code(_freeunit[k].Code, _freeunit[k].Evil, where, owner)) {
					return(true);
				}
			}

			CAC_Log("crate", "nothing could be delivered at %d,%d -- paying instead",
						Cell_X(cell), Cell_Y(cell));

			HouseClass * paid = HouseClass::As_Pointer(owner);
			if (paid != NULL) paid->Refund_Money(2000);
			new AnimClass(ANIM_CRATE_DOLLAR, coord);
			return(true);
		}

		default: {
			new AnimClass(ANIM_CRATE_DOLLAR, coord);
			HouseClass *hptr = HouseClass::As_Pointer(owner);
			if (hptr != NULL) hptr->Refund_Money(_cash[what]);
			break;
		}
	}

	return(false);
}


/***********************************************************************************************
 * CAC_Power_State -- How the base is doing for power, CrystAlien's way.                       *
 *                                                                                             *
 *    Tiberian Dawn expresses power as a ratio of output to drain.  CrystAlien does not: it     *
 *    adds, starting from an allowance of fifty, and Level.think recomputes the whole sum       *
 *    every frame:                                                                              *
 *                                                                                             *
 *        powerCharge = 50; powerDrain = 0;                                                     *
 *        for each building:                                                                    *
 *            if (stats.power > 0) powerCharge += stats.power * (healthPerc / 100)              *
 *            if (stats.power < 0) powerDrain += -stats.power                                   *
 *        net = powerCharge - powerDrain                                                        *
 *        powerLow = net < 50                                                                   *
 *        powerOff = net < 0                                                                    *
 *                                                                                             *
 *    Two things follow that a ratio cannot express.  The allowance cancels out of powerLow,    *
 *    so a base is "low" the moment its drain passes its output -- but it only goes "off" once  *
 *    the drain passes the output by more than fifty, which is a grace band a ratio has no      *
 *    room for.  And only the generators scale with health: a half-wrecked Power Plant supplies *
 *    half, while a half-wrecked Radar Station still draws its full fifty.                      *
 *                                                                                             *
 * INPUT:   house -- Whose base to total up.                                                   *
 *                                                                                             *
 * OUTPUT:  CAC_POWER_FULL, CAC_POWER_LOW or CAC_POWER_OFF.                                    *
 *=============================================================================================*/
void CAC_Power_Sums(HouseClass const *house, int &charge, int &drain)
{
	charge = CAC_POWER_ALLOWANCE;
	drain = 0;
	if (house == NULL) return;

	for (int i = 0; i < Buildings.Count(); i++) {
		BuildingClass const *b = Buildings.Ptr(i);
		if (b == NULL || b->IsInLimbo || !b->IsActive) continue;
		if (b->House != house) continue;
		if (b->Class == NULL) continue;

		/*
		**	A building still going up contributes nothing either way; CrystAlien
		**	only adds a structure to the sum once it exists.
		*/
		/*
		**	A building still going up counts for both.  findBuilding asks only that
		**	it be `active`, and Building() sets that in its constructor, so a Solar
		**	Panel carries its load from the moment it is placed and a Defence Station
		**	draws one from then too.  Waiting for the build-up meant a base browned
		**	out for as long as its next plant took to finish -- and a brownout is
		**	exactly when turrets stop, which is what "the turret needs a plant next
		**	to it" turns out to have been: it needed the plant *finished*.
		*/

		if (b->Class->Power > 0) {
			/*
			**	`stats.power * (healthPerc / 100)`, and healthPerc is a ceiling of
			**	the percentage rather than a truncation.
			*/
			int perc = 100;
			if (b->Class->MaxStrength > 0) {
				perc = ((int)b->Strength * 100 + b->Class->MaxStrength - 1) / b->Class->MaxStrength;
			}
			charge += (b->Class->Power * perc) / 100;
		}
		drain += b->Class->Drain;
	}

}


/***********************************************************************************************
 * CAC_Power_Charge / CAC_Power_Drain -- The two halves, for the bar that shows them.           *
 *                                                                                             *
 *    The allowance is part of the charge, because that is where Level.think puts it:            *
 *    `this.powerCharge = 50` before anything is counted.                                        *
 *=============================================================================================*/
int CAC_Power_Charge(HouseClass const *house)
{
	int charge, drain;
	CAC_Power_Sums(house, charge, drain);
	return(charge);
}

int CAC_Power_Drain(HouseClass const *house)
{
	int charge, drain;
	CAC_Power_Sums(house, charge, drain);
	return(drain);
}


/***********************************************************************************************
 * CAC_Power_Net -- Charge less drain; below zero the base is dark.                             *
 *=============================================================================================*/
int CAC_Power_Net(HouseClass const *house)
{
	int charge, drain;
	CAC_Power_Sums(house, charge, drain);
	return(charge - drain);
}


/***********************************************************************************************
 * CAC_Power_State -- Which of the three lights the base is showing.                           *
 *=============================================================================================*/
CACPowerState CAC_Power_State(HouseClass const *house)
{
	/*
	**	Volatile: the allowance -- 50 for the player, none for the computer -- and
	**	the health-scaled generation against unscaled drain are read off CrystAlien's
	**	Level, and an error here shows up indirectly: as build tempo, or as turrets
	**	that look unpowered.  Logged when the state changes, not every call.
	*/
	if (CAC_Logging() && house != NULL) {
		static int _was[HOUSE_COUNT];
		static bool _init = false;
		if (!_init) { _init = true; for (int h = 0; h < HOUSE_COUNT; h++) _was[h] = -99; }
		int hi = (int)house->Class->House;
		if ((unsigned)hi < HOUSE_COUNT) {
			int charge, drain;
			CAC_Power_Sums(house, charge, drain);
			int net = charge - drain;
			int state = (net < 0) ? 2 : ((net < 50) ? 1 : 0);
			if (_was[hi] != state) {
				_was[hi] = state;
				int plants = 0;
				int sinks = 0;
				for (int bi = 0; bi < Buildings.Count(); bi++) {
					BuildingClass const * bp = Buildings.Ptr(bi);
					if (bp == NULL || !bp->IsActive || bp->IsInLimbo) continue;
					if (bp->House != house || bp->Class == NULL) continue;
					if (bp->Class->Power > 0) plants++;
					if (bp->Class->Drain > 0) sinks++;
				}
				CAC_Log_Volatile("power",
					"%s: charge %d, drain %d, net %d -- %s"
					"  (%d plants, %d draws; the house itself says %d/%d)",
					house->Class->IniName, charge, drain, net,
					(state == 2) ? "off" : ((state == 1) ? "low" : "fine"),
					plants, sinks, (int)house->Power, (int)house->Drain);
			}
		}
	}

	int net = CAC_Power_Net(house);
	if (net < 0) return(CAC_POWER_OFF);
	if (net < CAC_POWER_ALLOWANCE) return(CAC_POWER_LOW);
	return(CAC_POWER_FULL);
}


/***********************************************************************************************
 * CAC_Power_Off -- Is this house's base dark enough that its guns stop?                       *
 *                                                                                             *
 *    Level.engageEnemy refuses to fire at all for a building whose owner is powerOff -- units  *
 *    are unaffected -- and Building.draw stops the idle turret sweep on the same flag.         *
 *=============================================================================================*/
bool CAC_Power_Off(HouseClass const *house)
{
	return(CAC_Power_State(house) == CAC_POWER_OFF);
}


/***********************************************************************************************
 * CAC_Power_Low -- Is this house's base short of power?                                       *
 *                                                                                             *
 *    Construction.think halves its build speed on this flag, and the radar and the shroud go   *
 *    with it.                                                                                  *
 *=============================================================================================*/
bool CAC_Power_Low(HouseClass const *house)
{
	return(CAC_Power_State(house) != CAC_POWER_FULL);
}




/***********************************************************************************************
 * CAC_Crystal_Floor -- How much this miner has to leave in the ground.                        *
 *                                                                                             *
 *    UnitNav.feed:                                                                             *
 *                                                                                             *
 *        take = floor(min(10, baits[x][y]));                                                   *
 *        baits[x][y] = limit(baits[x][y] - take, 1, 100);                                      *
 *                                                                                             *
 *    limit() clamps to one, not to nothing, so a CrystAlien field is never mined out.  That    *
 *    also happens to be what makes "Tiberium: regrow" mean anything: growth works on cells      *
 *    that still hold crystal and spread needs a heavy one to spread from, so a field stripped   *
 *    bare has nothing left for either to act on.                                                *
 *                                                                                             *
 *    Everywhere that asks "is there crystal here" has to ask it the same way, or a miner        *
 *    stands on a cell it has finished with while Goto_Tiberium tells it that it is already      *
 *    where it needs to be.  That is a deadlock, and it is what the first attempt at this did.   *
 *                                                                                             *
 * OUTPUT:  the OverlayData a cell must exceed before this object can take from it.             *
 *=============================================================================================*/
int CAC_Crystal_Floor(TechnoClass const *miner)
{
	if (miner == NULL || miner->What_Am_I() != RTTI_UNIT) return(0);
	return(CAC_Is_Miner(*(UnitClass const *)miner) ? 1 : 0);
}


/***********************************************************************************************
 * CAC_Special_Asset -- What the client should call this special weapon.                       *
 *                                                                                             *
 *    A sidebar entry reaches the client as an asset name, and the client uses it for both the  *
 *    cameo it draws and the text it looks up -- BuildIcon_TD_<name> and the ObjectTypeClass    *
 *    row of the same name in CACBuildables.xml.  Every ordinary entry passes tech->IniName,    *
 *    so it all lines up.                                                                       *
 *                                                                                             *
 *    A special weapon has no TechnoTypeClass to take a name from, so DLLInterface writes one   *
 *    in by hand, and the one it writes for the ion cannon is "UK".  That is the CrystAlien      *
 *    code without the side on it, and nothing is called that: the artwork is UKGOOD and        *
 *    UKEVIL, the text rows are TEXT_UNIT_UKGOOD and TEXT_UNIT_UKEVIL, and the buildables XML   *
 *    has TD_UKGOOD and TD_UKEVIL.  So the entry drew with no picture and no name at all.        *
 *                                                                                             *
 *    The two are different weapons besides -- the Astros' EM Pulse Cannon and the Aliens' 4T2  *
 *    Ion Disrupter -- so which one it is depends on who is looking at the sidebar.             *
 *                                                                                             *
 * OUTPUT:  the asset name, which is `stock` for anything not CrystAlien's.  Answering rather
 *          than declining means the call sites need no local, and they sit inside switch
 *          cases that have no braces to declare one in.                            *
 *=============================================================================================*/
char const *CAC_Special_Asset(HouseClass const *house, SpecialWeaponType id, char const *stock)
{
	if (id != SPC_ION_CANNON || !CAC_Is_CrystAlien_House(house)) return(stock);

	/*
	**	A name the client already knows, because a superweapon's picture is not
	**	found the way a buildable's is.
	**
	**	A buildable entry is looked up by its asset name in the buildables XML, which
	**	is why UAGOOD works there.  A special is not in that XML at all -- the engine
	**	ships "SW_Ion", "SW_Nuke" and "SW_AirStrike", and none of those is an object
	**	anywhere in CONFIG.MEG.  They are labels the client resolves against a table
	**	of its own, and a label that is not in it resolves to nothing.
	**
	**	"UKEVIL" was not in it, so the entry drew with no picture -- which is the only
	**	thing that was wrong: the superweapon itself worked throughout.
	**
	**	There are two labels and CrystAlien has two uplinks, so each side takes one
	**	and keeps its own artwork.  This is the picture only.  SuperWeaponType stays
	**	SW_ION_CANNON for both, so the targeting, the charge and the blast are
	**	unchanged and the Aliens are not firing a nuke.
	*/
	/*
	**	The ion slot is the Astros' now -- the Aliens have the other one, and it is
	**	already labelled SW_Nuke by the engine -- so this answers for one slot rather
	**	than for whoever is asking.
	*/
	/*
	**	Which uplink is standing, not which slot it landed in.
	**
	**	The two labels are the only names the client will resolve a special's picture
	**	from, so each side's uplink is given one and the artwork behind it is that
	**	side's own cameo.  What this used to answer was the *slot* -- and a lone
	**	uplink always takes the ion cannon's slot whichever side built it, so an
	**	Alien commander with an Orbital Uplink was handed the Astro picture.
	**
	**	The Astro uplink shows the Astro cameo and anything else shows the Alien one.
	**	Hold both and there are two entries -- this one for the Astro uplink, and the
	**	nuke slot, which the engine already labels SW_Nuke, for the Alien -- with a
	**	picture each.
	*/
	return(CAC_Has_Uplink(house, false) ? "SW_Ion" : "SW_Nuke");
}


/***********************************************************************************************
 * CAC_In_Infantry_Tab -- Should this be shown beside the foot soldiers?                       *
 *                                                                                             *
 *    CrystAlien has no tabs.  Construction builds one flat list -- BA, UA, BB, UB and so on,   *
 *    buildings and units interleaved in code order -- so there is nothing in the source to be  *
 *    faithful to here; the grouping is the client's own and the choice is ours.                *
 *                                                                                             *
 *    The rule that gets it right without inventing anything is: a thing belongs beside what it *
 *    is built with.  The JetPack Explorer and the Viper Attack have                            *
 *                                                                                             *
 *        home = findBuilding("BC_" + team)                                                     *
 *                                                                                             *
 *    -- the Training Camp and the Breeding Pit, the same building that trains the Infantry and *
 *    the Engineer.  They came up under the vehicles only because they fly and the conversion   *
 *    put them in aircraft slots for it.                                                        *
 *                                                                                             *
 *    Which is why this moves the tab and not the type.  Both carry `altitude: 50` and          *
 *    `flying: true`; on the infantry roster they would walk, and be stopped by everything a    *
 *    walker is stopped by.                                                                     *
 *=============================================================================================*/
static void CAC_Host_Output(StructType host, int &foot, int &air);

bool CAC_In_Infantry_Tab(RTTIType rtti, int id)
{
	if (rtti != RTTI_AIRCRAFT && rtti != RTTI_AIRCRAFTTYPE) return(false);

	StructType host = CAC_Host_Of(RTTI_AIRCRAFTTYPE, id);
	if (host == STRUCT_NONE) return(false);

	/*
	**	What the host mostly makes, rather than whether it makes any at all.
	**
	**	"The host builds infantry" was too weak a test.  The Training Camp and the
	**	Breeding Pit build two foot soldiers and one flier, which is the case this
	**	was written for -- but the Ops Ship and the Alien Hive build one of each,
	**	the Alien Commander and the Switch Fighter, and one commander was enough to
	**	drag a fighter in beside the infantry.
	**
	**	So it is settled by weight: a flier sits with the foot soldiers when its
	**	host is a place that mostly makes foot soldiers.
	*/
	int foot = 0;
	int air = 0;
	CAC_Host_Output(host, foot, air);
	return(foot > air);
}


/***********************************************************************************************
 * CAC_Is_Decoy -- Somebody worth sending somewhere dangerous.                                 *
 *                                                                                             *
 *    A CrystAlien foot soldier with no weapon and no ability to capture, which comes to        *
 *    exactly one type: the reindeer.  Seventy-five health, speed three, eight of them, and     *
 *    `weapon: false` in UnitStats -- there is nothing else they can be asked to do.            *
 *                                                                                             *
 *    What they are good for is being shot at.  engageEnemy gives a defence one target at a     *
 *    time and picks the nearest, so a reindeer walking in ahead of a Saboteur is a turret not  *
 *    shooting the Saboteur.  That is this conversion's use for them; Level 29 places them and  *
 *    gives them nothing to do either.                                                          *
 *=============================================================================================*/
static bool CAC_Is_Decoy(InfantryClass const *man)
{
	if (man == NULL || !man->IsActive || man->IsInLimbo || man->Strength == 0) return(false);
	if (man->Class == NULL) return(false);
	if (!CAC_Is_CrystAlien_Type(man->Class->IniName)) return(false);
	return(man->Class->Primary == WEAPON_NONE && !man->Class->IsCapture);
}


/***********************************************************************************************
 * CAC_Free_Decoy -- One that is standing about doing nothing.                                 *
 *=============================================================================================*/
static InfantryClass *CAC_Free_Decoy(HouseClass const *house, InfantryClass const *skip)
{
	for (int i = 0; i < Infantry.Count(); i++) {
		InfantryClass *man = Infantry.Ptr(i);
		if (man == NULL || man == skip || man->House != house) continue;
		if (!CAC_Is_Decoy(man)) continue;
		if (Target_Legal(man->NavCom)) continue;
		if (man->Mission != MISSION_GUARD && man->Mission != MISSION_GUARD_AREA) continue;
		return(man);
	}
	return(NULL);
}


/***********************************************************************************************
 * CAC_AI_Escort -- Walk a few decoys in ahead of whoever is doing the real work.              *
 *=============================================================================================*/
static void CAC_AI_Escort(HouseClass *house, BuildingClass *target, int wanted)
{
	if (house == NULL || target == NULL) return;

	CELL where = Coord_Cell(target->Center_Coord());
	while (wanted-- > 0) {
		InfantryClass *man = CAC_Free_Decoy(house, NULL);
		if (man == NULL) return;
		man->Assign_Target(TARGET_NONE);
		man->Assign_Destination(::As_Target(where));
		man->Assign_Mission(MISSION_MOVE);
	}
}


/***********************************************************************************************
 * CAC_AI_Collect -- Send somebody to pick up what is lying about.                             *
 *                                                                                             *
 *    Unit.collect is on every unit in CrystAlien, and its levels scatter boxes where they      *
 *    expect both sides to be moving.  Tiberian Dawn's computer has no notion of a crate at     *
 *    all: it walks over one by accident or not at all, so on a skirmish map the human          *
 *    collected every box, every capsule and every present unopposed.                            *
 *                                                                                             *
 *    One errand at a time, and only ever a unit with nothing else to do -- not a Miner, not a  *
 *    raider, not anything already going somewhere.  Nearest pickup to whoever is free.          *
 *                                                                                             *
 *    Called on the attack cadence, so a base looks around every half minute to two minutes     *
 *    rather than every frame.                                                                   *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_AI_Aggression -- Send anything idle out looking, and everything out when it is over.     *
 *                                                                                             *
 *    CrystAlien has no attack wave.  Every enemy unit that is not a miner runs a UnitHAL every *
 *    frame, and a HAL in its ordinary state -- "seek" -- picks a random free tile anywhere on  *
 *    the map and goes there:                                                                   *
 *                                                                                             *
 *        do { x = random(arena.cols); y = random(arena.rows); }                                 *
 *        while (arena.tiles[x][y]);                                                             *
 *                                                                                             *
 *    So its army is permanently roaming and permanently running into things.  There is nothing *
 *    to build up to and nothing to wait for.                                                    *
 *                                                                                             *
 *    Tiberian Dawn is the opposite: it waits out an Attack timer of AttackInterval times half  *
 *    to two minutes -- between a minute and a half and six minutes of standing still -- and    *
 *    then throws one wave.  Between waves a computer with a full bank and a full base does      *
 *    nothing at all, which is what "keeps to itself and never crosses the map" is.              *
 *                                                                                             *
 *    MISSION_HUNT is the engine's own equivalent of seek: it finds an enemy anywhere and goes  *
 *    for it.  So anything armed and idle is put on it, continuously, and the wave timer is     *
 *    left to do whatever it does on top.                                                       *
 *                                                                                             *
 *    And the last stand.  Losing the headquarters is the end of the game in CrystAlien --      *
 *    Building.destroy calls lose() outright -- so there is nothing left to protect and no       *
 *    reason to hold anything back.  Here it is survivable, so what it becomes is everything     *
 *    the house has, thrown at whoever is left.                                                  *
 *=============================================================================================*/
static bool CAC_Has_HQ(HouseClass const * house)
{
	return(CAC_Owns(house, CAC_CODE_BA) || CAC_Owns(house, CAC_CODE_BA | CAC_CODE_EVIL) ||
		   CAC_Owns(house, CAC_CODE_BK) || CAC_Owns(house, CAC_CODE_BK | CAC_CODE_EVIL));
}

bool CAC_House_Has_HQ(HouseClass const * house)
{
	return(CAC_Has_HQ(house));
}


/*
**	Which of a house's objects CrystAlien's own routines may command.
**
**	In the conversion, all of them.  In the supplemental a GDI or Nod house
**	becomes a "CrystAlien house" at its first CrystAlien building, and these
**	routines then sent its stock army to hunt, raided and patrolled with it, and
**	cancelled its stock repairs -- overriding Tiberian Dawn's own attack and base
**	logic wholesale.  There they keep to CrystAlien's units and buildings, and the
**	stock ones are left to the stock AI.
*/
/*
**	Whether CrystAlien's whole-house routines -- crates, the flag, its fighters'
**	sorties -- run for this house.  In the conversion, for a CrystAlien house.  In
**	the supplemental, for every skirmish computer: those routines are for the
**	units on the field, and a GDI or Nod computer with no CrystAlien building yet
**	still has crates to fetch and CrystAlien starting units to use.
*/
static bool CAC_AI_Runs_For(HouseClass const * house)
{
#ifdef CAC_SUPPLEMENTAL
	return(house != NULL && house->Class != NULL && !house->IsHuman &&
			 house->Class->House >= HOUSE_MULTI1);
#else
	return(CAC_Is_CrystAlien_House(house));
#endif
}


static bool CAC_AI_Owns(TechnoClass const * object)
{
#ifdef CAC_SUPPLEMENTAL
	return(object != NULL && object->Techno_Type_Class() != NULL &&
			 CAC_Is_CrystAlien_Type(object->Techno_Type_Class()->IniName));
#else
	return(object != NULL);
#endif
}


void CAC_AI_Aggression(HouseClass * house)
{
	if (house == NULL || house->IsHuman) return;
	if (!CAC_Is_CrystAlien_House(house)) return;

	/*
	**	Nothing left to defend.  Everything it has goes out, and keeps going out --
	**	Do_All_To_Hunt is the engine's own, and is what it does at STATE_ENDGAME.
	*/
	bool lastditch = (!CAC_Has_HQ(house) && house->CurBuildings > 0);
#ifdef CAC_SUPPLEMENTAL
	/*
	**	Not while there is a stock base to fight from.  A GDI or Nod house that
	**	lost its Eagle Command, or captured one CrystAlien building, still has
	**	its own yard and factories, and this sent everything it owned -- the
	**	harvesters and the MCV included -- on the suicide hunt every two seconds.
	*/
	if (house->ActiveBScan & (STRUCTF_CONST|STRUCTF_WEAP|STRUCTF_BARRACKS|STRUCTF_HAND|STRUCTF_AIRSTRIP|STRUCTF_HELIPAD)) {
		lastditch = false;
	}
#endif
	if (lastditch) {
		/*
		**	One static for all of them again -- seventh time for this.  Six houses lose
		**	their headquarters at different moments and each takes the flag from the
		**	last, so it announced six hundred and sixty-five times in one round.
		*/
		static bool _said[HOUSE_COUNT];
		static bool _saidinit = false;
		if (!_saidinit) { _saidinit = true; for (int h = 0; h < HOUSE_COUNT; h++) _said[h] = false; }

		int hi = (int)house->Class->House;
		if ((unsigned)hi < HOUSE_COUNT && !_said[hi]) {
			_said[hi] = true;
			CAC_Log("ai.lastditch", "%s has lost its headquarters -- everything goes out",
						house->Class->IniName);
		}
		/*
		**	And the fliers, which Do_All_To_Hunt does not reach.
		**
		**	It walks Units and Infantry, and its third loop -- the one that would
		**	have covered anything else -- is `#if (0)`.  Since this branch returns
		**	immediately afterwards, the aircraft pass further down never runs
		**	either, so a house that had lost its headquarters sent its ground force
		**	out on the suicide hunt and left every Switch Fighter and JetPack
		**	Explorer hovering over the rubble.
		**
		**	Boomerangs are left out: their station is gone by definition here, and
		**	CAC_Check_Home takes them with it.
		*/
		for (int a = 0; a < Aircraft.Count(); a++) {
			AircraftClass * craft = Aircraft.Ptr(a);
			if (craft == NULL || !craft->IsActive || craft->IsInLimbo) continue;
			if (craft->House != house || craft->Strength == 0) continue;
			if (!craft->Is_Weapon_Equipped()) continue;
			if (CAC_Is_Boomerang(*craft)) continue;

			craft->Assign_Mission(MISSION_HUNT);
		}

		house->Do_All_To_Hunt();
		return;
	}

	/*
	**	GUARD_AREA means "out on patrol" now -- see CAC_AI_Patrol -- so it is only
	**	taken back when there is something to answer.  Left unconditional, this
	**	routine reclaimed every patrolling unit within two seconds of it being sent
	**	and the patrol never existed.
	*/
	bool reclaim = CAC_AI_Under_Attack(house);

	/*
	**	How many this pass actually set going.
	**
	**	"The computer stops commanding its units until provoked" is a claim about
	**	this routine doing nothing, and there was no way to tell that from a
	**	routine doing something to nought units.  Provoked is the telling word:
	**	FootClass::Take_Damage retaliates out of MISSION_GUARD and hands a
	**	computer unit MISSION_HUNT itself, so a house whose units are all sitting
	**	in GUARD looks asleep until something shoots one of them -- and this is
	**	the routine whose whole job is to stop that happening.
	*/
	int woke = 0;
	int guarding = 0;

	int i;
	for (i = 0; i < Units.Count(); i++) {
		UnitClass * u = Units.Ptr(i);
		if (u == NULL || !u->IsActive || u->IsInLimbo || u->House != house) continue;
		if (!CAC_AI_Owns(u)) continue;
		if (u->Strength == 0 || CAC_Is_Miner(*u) || u->Flagged != HOUSE_NONE) continue;
		if (!u->Is_Weapon_Equipped()) continue;
		if (Target_Legal(u->NavCom) || Target_Legal(u->TarCom)) continue;
		if (u->Mission != MISSION_GUARD &&
			!(reclaim && u->Mission == MISSION_GUARD_AREA)) continue;

		u->Assign_Mission(MISSION_HUNT);
		woke++;
	}

	/*
	**	The fliers that are not tethered, which nothing was moving at all.
	**
	**	This walked Units and Infantry and stopped there, so the JetPack Explorer,
	**	the Viper Attack and both Switch Fighters were built and then left hovering
	**	where they were made for the rest of the game.  They are ordinary armed
	**	units as far as this is concerned; only a boomerang is special, and it is
	**	special because CAC_AI_Sortie owns it.
	*/
	for (i = 0; i < Aircraft.Count(); i++) {
		AircraftClass * a = Aircraft.Ptr(i);
		if (a == NULL || !a->IsActive || a->IsInLimbo || a->House != house) continue;
		if (!CAC_AI_Owns(a)) continue;
		if (a->Strength == 0 || a->Class == NULL) continue;
		if (!a->Is_Weapon_Equipped()) continue;		/* the Dropship is CAC_AI_Airlift's */
		if (CAC_Is_Boomerang(*a)) continue;			/* and a Fighter is CAC_AI_Sortie's */

		/*
		**	A Switch Fighter is commanded rarely, on purpose.
		**	
		**	UnitHAL.handle throttles exactly one type:
		**	
		**	    if (this.parent.stats.transformer && random(20) != 0) return undefined;
		**	
		**	so the computer gives it an order about one call in twenty and leaves it
		**	alone the rest of the time.  Nothing here reproduced that, so it was being
		**	handled at the full aggression cadence -- twenty times as often as the
		**	source, which is what turns a craft that is supposed to settle somewhere
		**	and hold it into one that is perpetually being sent somewhere new.
		*/
		/*
		**	UnitHAL throttles a transformer's re-decisions to one frame in twenty:
		**
		**	    if (this.parent.stats.transformer && random(20) != 0) return;
		**
		**	It has to land to sit and take off to travel, so re-picking every frame
		**	would leave it doing nothing else.
		**
		**	On the simulation stream, not the presentation one.  This decides what
		**	the computer *does*, so every client has to reach the same answer, and
		**	Random_Pick is the draw-side generator.
		*/
		if (CAC_Is_Transformer(*a) && Sim_Random_Pick(0, 19) != 0) continue;
		if (Target_Legal(a->NavCom) || Target_Legal(a->TarCom)) continue;
		if (a->Mission != MISSION_GUARD &&
			!(reclaim && a->Mission == MISSION_GUARD_AREA)) continue;

		a->Assign_Mission(MISSION_HUNT);
		woke++;
	}

	for (i = 0; i < Infantry.Count(); i++) {
		InfantryClass * man = Infantry.Ptr(i);
		if (man == NULL || !man->IsActive || man->IsInLimbo || man->House != house) continue;
		if (!CAC_AI_Owns(man)) continue;
		if (man->Strength == 0 || !man->Is_Weapon_Equipped()) continue;
		if (Target_Legal(man->NavCom) || Target_Legal(man->TarCom)) continue;
		if (man->Mission != MISSION_GUARD &&
			!(reclaim && man->Mission == MISSION_GUARD_AREA)) continue;

		/*
		**	And not the engineers, who have somewhere better to be.  See
		**	CAC_AI_Infiltrate, which is picking raids out of the same pool.
		*/
		if (man->Class->IsCapture || CAC_Is_Decoy(man)) continue;

		man->Assign_Mission(MISSION_HUNT);
		woke++;
	}
	/*
	**	Said every half minute, and only when there was something to say.
	*/
	if (CAC_Logging() && (Frame % (TICKS_PER_MINUTE / 2)) == 0) {

		/*
		**	Counted afterwards rather than inside the loops, so it means what it
		**	says: armed things of this house that are still standing in guard now
		**	that the pass has been over them.  A number that stays high is this
		**	routine declining to move them.
		*/
		for (i = 0; i < Units.Count(); i++) {
			UnitClass const * u = Units.Ptr(i);
			if (u != NULL && u->IsActive && !u->IsInLimbo && u->House == house &&
				 u->Strength > 0 && u->Is_Weapon_Equipped() &&
				 u->Mission == MISSION_GUARD) guarding++;
		}
		for (i = 0; i < Aircraft.Count(); i++) {
			AircraftClass const * a = Aircraft.Ptr(i);
			if (a != NULL && a->IsActive && !a->IsInLimbo && a->House == house &&
				 a->Strength > 0 && a->Is_Weapon_Equipped() &&
				 a->Mission == MISSION_GUARD) guarding++;
		}
		for (i = 0; i < Infantry.Count(); i++) {
			InfantryClass const * man = Infantry.Ptr(i);
			if (man != NULL && man->IsActive && !man->IsInLimbo && man->House == house &&
				 man->Strength > 0 && man->Is_Weapon_Equipped() &&
				 man->Mission == MISSION_GUARD) guarding++;
		}
	}

	if (CAC_Logging() && (Frame % (TICKS_PER_MINUTE / 2)) == 0 && (woke || guarding)) {
		CAC_Log_Volatile("ai.idle", "%s: %d set going, %d still standing in guard",
							  (house->Class != NULL) ? house->Class->IniName : "?",
							  woke, guarding);
	}

}


/***********************************************************************************************
 * CAC_AI_Flag -- Go and get the flag, and bring it home.                                       *
 *                                                                                             *
 *    Capture the flag has exactly one piece of computer behaviour in Tiberian Dawn, and it is  *
 *    defensive:  HouseClass::AI scatters anything sitting on the house's own flag cell.  There *
 *    is nothing anywhere that sends a computer unit at an enemy flag, so a bot played capture  *
 *    the flag as an ordinary skirmish and took one only by walking over it by accident.        *
 *                                                                                             *
 *    Who may carry one is not a matter of preference.  `Flagged` is a member of UnitClass and  *
 *    of nothing else, and the pickup is in UnitClass::Per_Cell_Process:                        *
 *                                                                                             *
 *        if (center && Flagged == HOUSE_NONE) {                                                *
 *            if (Map[cell].IsFlagged && !House->Is_Ally(Map[cell].Owner)) { ... }               *
 *                                                                                             *
 *    InfantryClass has no such field and no such code, and neither has AircraftClass.  So the  *
 *    Alien Commander, Santa and the reindeer cannot take a flag however fast they are, and     *
 *    neither can a Switch Fighter -- on this roster the runners are the Speeder, the Crusher   *
 *    and the Crystal Reaper, and the Ops Ship.  Miners are left out of it; the economy is      *
 *    theirs.                                                                                   *
 *                                                                                             *
 *    Fastest first, which on CrystAlien's own numbers picks the Speeder every time -- speed 10 *
 *    against the Reaper's 6 and the Crusher's 5 -- and falls through to whatever is left when  *
 *    there is no Speeder to send.                                                              *
 *                                                                                             *
 *    Three things, in order of importance:  a runner already holding a flag drives it home,    *
 *    because a flag picked up and not delivered scores nothing; anything carrying *our* flag   *
 *    is hunted, because that is the one unit that can end the round; and otherwise the fastest *
 *    free runner is sent at the nearest enemy flag still on the ground.                        *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Flag_Cell -- Where this house's flag is standing, if it is standing anywhere.            *
 *                                                                                             *
 *    FlagLocation is a target and it is either a cell or the unit carrying it.  When somebody  *
 *    is carrying it there is no ground to guard, and hunting the carrier is the answer         *
 *    instead.  FlagHome is where it started and where it goes back to; it is not necessarily   *
 *    where it is, because a carrier that dies drops it on the spot:                            *
 *                                                                                             *
 *        HouseClass::As_Pointer(Flagged)->Flag_Attach(Coord_Cell(Coord));                      *
 *                                                                                             *
 *    with set_home false, so FlagHome stays put and FlagLocation moves.  Guarding FlagHome     *
 *    after a flag has been dropped elsewhere is guarding an empty square.                      *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Flag_Site -- Somewhere beside the headquarters that a flag can actually be stood on.     *
 *                                                                                             *
 *    The flag used to be hung on the MCV -- Flag_Attach(UnitClass *) -- and an MCV drives off, *
 *    deploys, and leaves the flag on the open ground it was standing on.  With the headquarters*
 *    placed outright instead there is no vehicle to hang it on, so it went on by cell:         *
 *                                                                                             *
 *        hptr->Flag_Attach(Coord_Cell(hq->Center_Coord()), true);                              *
 *                                                                                             *
 *    -- the middle of the building.  Flag_Attach asks Flag_Place, Flag_Place asks              *
 *    Is_Generally_Clear, and that routine opens with                                           *
 *                                                                                             *
 *        if (ScenarioInit) return(true);                                                       *
 *                                                                                             *
 *    which is exactly when this runs.  So every test that would have refused the cell was      *
 *    skipped and the flag was planted underneath the Headquarters, on ground the building       *
 *    occupies.                                                                                 *
 *                                                                                             *
 *    A cell with a building occupier is impassable, so nothing could be ordered onto it -- the  *
 *    cursor answers ACTION_NOMOVE, the pathfinder routes around it, and that flag could not be  *
 *    captured by anybody for the whole round.  Which is precisely the report.                   *
 *                                                                                             *
 *    So pick the site here, asking the questions Is_Generally_Clear would have asked if it had  *
 *    been listening: real ground, nothing built on it, nothing drawn over it.  Nearest first,   *
 *    so the flag still sits at the base the way it is meant to.                                 *
 *=============================================================================================*/
CELL CAC_Flag_Site(BuildingClass const *hq)
{
	if (hq == NULL || hq->Class == NULL) return(0);

	CELL origin = Coord_Cell(hq->Center_Coord());

	for (int reach = 1; reach <= CAC_FLAG_SITE_REACH; reach++) {
		for (int dy = -reach; dy <= reach; dy++) {
			for (int dx = -reach; dx <= reach; dx++) {

				/*	Only the ring, so the nearest clear cell wins. */
				if (ABS(dx) != reach && ABS(dy) != reach) continue;

				CELL cell = (CELL)(origin + dx + dy * MAP_CELL_W);
				if ((unsigned)cell >= MAP_CELL_TOTAL) continue;
				if (!Map.In_Radar(cell)) continue;

				CellClass const & c = Map[cell];

				/*	Ground a vehicle can stand on -- a flag nobody can reach is no flag. */
				if (Ground[c.Land_Type()].Cost[SPEED_TRACK] == 0) continue;
				if (c.Overlay != OVERLAY_NONE) continue;
				if (c.IsFlagged) continue;
				if (c.Cell_Occupier() != NULL) continue;

				bool built = false;
				for (int over = 0; over < (int)(sizeof(c.Overlapper) / sizeof(c.Overlapper[0])); over++) {
					ObjectClass const *lap = c.Overlapper[over];
					if (lap != NULL && lap->IsActive && lap->What_Am_I() == RTTI_BUILDING) built = true;
				}
				if (built) continue;

				return(cell);
			}
		}
	}
	return(0);
}


/***********************************************************************************************
 * CAC_On_Crate_Errand -- Is this one already walking to a crate?                                *
 *                                                                                             *
 *    The flag routines recruit out of MISSION_GUARD, HUNT and GUARD_AREA on purpose, and both  *
 *    of them looked at the mission and not at whether the unit had somewhere to be.  So a unit *
 *    dispatched for a crate had its destination overwritten by the flag guard two seconds      *
 *    later, CAC_AI_Collect saw its errand count fall back to nought, sent somebody else, and   *
 *    the guard took that one too.  One crate drew sixty-eight dispatch orders in two minutes   *
 *    before anybody reached it.                                                                *
 *                                                                                             *
 *    The order of precedence this file has settled on is errand, then raid, then patrol -- a   *
 *    crate is a thing that will be gone shortly and a flag post will still be there            *
 *    afterwards.  So the flag routines keep their claim on a patroller and a hunter, and let   *
 *    an errand finish.                                                                         *
 *=============================================================================================*/
bool CAC_On_Crate_Errand(FootClass const * foot)
{
	if (foot == NULL || !Target_Legal(foot->NavCom)) return(false);

	CELL dest = As_Cell(foot->NavCom);
	if ((unsigned)dest >= MAP_CELL_TOTAL) return(false);
	if (Map[dest].Overlay == OVERLAY_NONE) return(false);
	return(OverlayTypeClass::As_Reference(Map[dest].Overlay).IsCrate != 0);
}


CELL CAC_Flag_Cell(HouseClass const * house)
{
	if (house == NULL) return(0);
	if (!Target_Legal(house->FlagLocation)) return(house->FlagHome);
	if (As_Unit(house->FlagLocation) != NULL) return(0);	/* somebody has it */
	return(As_Cell(house->FlagLocation));
}


/***********************************************************************************************
 * CAC_Guarding_Flag -- Is this one of ours standing over the flag already?                     *
 *                                                                                             *
 *    Written as a question about where a unit is anchored rather than as a flag on the unit,   *
 *    so that CAC_AI_Patrol can ask it and leave the guard alone -- the patrol recruits out of  *
 *    MISSION_GUARD_AREA on purpose, which is exactly what a flag guard is sitting in.          *
 *=============================================================================================*/
bool CAC_Guarding_Flag(FootClass const * foot, HouseClass const * house)
{
	if (foot == NULL || house == NULL) return(false);
	if (foot->Mission != MISSION_GUARD_AREA) return(false);
	if (!Target_Legal(foot->ArchiveTarget)) return(false);

	CELL flag = CAC_Flag_Cell(house);
	if (flag == 0) return(false);
	return(As_Cell(foot->ArchiveTarget) == flag);
}


/***********************************************************************************************
 * CAC_Flag_Defence_Site -- Put the guns where the flag is, not where the average building is.  *
 *                                                                                             *
 *    Find_Build_Location rates the four sides of the base by how well defended each already    *
 *    is and drops the next gun on the weakest.  That is the right question in a fight over a   *
 *    base and the wrong one in a fight over one square: in capture the flag the only ground    *
 *    that has to hold is the flag's, and a Defence Station on the far side of the base is a    *
 *    Defence Station that will never fire at the unit that ends the round.                     *
 *                                                                                             *
 *    So an armed structure is offered the flag's neighbourhood first, nearest first, and the   *
 *    zone machinery is left to answer for everything else.  The proximity rule still has to    *
 *    pass, so this grants no reach a player has not got.                                       *
 *=============================================================================================*/
CELL CAC_Flag_Defence_Site(HouseClass const *house, BuildingClass *building)
{
	if (house == NULL || house->Class == NULL) return(0);
	if (building == NULL || building->Class == NULL) return(0);
	if (!CAC_Is_CrystAlien_House(house)) return(0);
	if (!Special.IsCaptureTheFlag || GameToPlay == GAME_NORMAL) return(0);

	/*	Only the guns.  A refinery near the flag defends nothing. */
	if (!building->Anti_Air() && !building->Anti_Armor() && !building->Anti_Infantry()) {
		return(0);
	}

	CELL flag = CAC_Flag_Cell(house);
	if (flag == 0) flag = house->FlagHome;
	if (flag == 0 || !Map.In_Radar(flag)) return(0);

	short const *list = building->Occupy_List(true);
	if (list == NULL) return(0);

	CELL origin = flag;
	CELL best = 0;
	int bestrange = 0;

	for (int dy = -CAC_FLAG_GUARD_REACH; dy <= CAC_FLAG_GUARD_REACH; dy++) {
		for (int dx = -CAC_FLAG_GUARD_REACH; dx <= CAC_FLAG_GUARD_REACH; dx++) {

			CELL cell = (CELL)(origin + dx + dy * MAP_CELL_W);
			if ((unsigned)cell >= (unsigned)MAP_CELL_TOTAL) continue;
			if (!Map.In_Radar(cell)) continue;
			if (!building->Class->Legal_Placement(cell)) continue;
			if (!Map.Passes_Proximity_Check(building->Class,
													  house->Class->House, list, cell)) continue;

			int range = ABS(dx) + ABS(dy);
			if (best == 0 || range < bestrange) {
				best = cell;
				bestrange = range;
			}
		}
	}

	if (best != 0) {
		CAC_Log("flag", "%s puts %s beside its flag at %d,%d",
				  house->Class->IniName, building->Class->IniName,
				  Cell_X(best), Cell_Y(best));
	}
	return(best);
}


void CAC_AI_Flag(HouseClass * house)
{
	if (house == NULL || house->IsHuman || !house->IsActive) return;
	if (house->Class == NULL) return;
	if (!Special.IsCaptureTheFlag || GameToPlay == GAME_NORMAL) return;
	if (!CAC_AI_Runs_For(house)) return;

	if ((Frame % (TICKS_PER_SECOND / 2)) != 0) return;

	/*
	**	Anything of ours already holding a flag goes home and keeps going.  Re-asserted
	**	every pass rather than ordered once, because CAC_AI_Aggression reclaims an idle
	**	unit within two seconds and a runner that arrives, is reclaimed and wanders off
	**	again is a flag that never scores.
	*/
	int i;
	for (i = 0; i < Units.Count(); i++) {
		UnitClass * u = Units.Ptr(i);
		if (u == NULL || !u->IsActive || u->IsInLimbo || u->House != house) continue;
		if (u->Strength == 0 || u->Flagged == HOUSE_NONE) continue;

		CELL home = house->FlagHome;
		if (home == 0 || Coord_Cell(u->Coord) == home) continue;

		if (!Target_Legal(u->NavCom) || As_Cell(u->NavCom) != home) {
			u->Assign_Target(TARGET_NONE);
			u->Assign_Destination(::As_Target(home));
			u->Assign_Mission(MISSION_MOVE);
		}
	}

	/*
	**	Whoever is carrying ours is worth everybody's attention.
	*/
	if (Target_Legal(house->FlagLocation)) {
		UnitClass * thief = As_Unit(house->FlagLocation);
		if (thief != NULL && thief->IsActive && thief->House != house) {
			for (i = 0; i < Units.Count(); i++) {
				UnitClass * u = Units.Ptr(i);
				if (u == NULL || !u->IsActive || u->IsInLimbo || u->House != house) continue;
				if (u->Strength == 0 || CAC_Is_Miner(*u) || u->Flagged != HOUSE_NONE) continue;
				if (!u->Is_Weapon_Equipped()) continue;
				if (u->TarCom == thief->As_Target()) continue;

				u->Assign_Target(thief->As_Target());
				u->Assign_Mission(MISSION_ATTACK);
			}
		}
	}

	/*
	**	And the nearest enemy flag still lying on the ground.
	*/
	CELL want = 0;
	int best = 0;
	for (int h = 0; h < Houses.Count(); h++) {
		HouseClass * other = Houses.Ptr(h);
		if (other == NULL || !other->IsActive || other == house) continue;
		if (house->Is_Ally(other)) continue;
		if (!Target_Legal(other->FlagLocation)) continue;
		if (As_Unit(other->FlagLocation) != NULL) continue;	/* somebody has it */

		CELL cell = As_Cell(other->FlagLocation);
		if (!Map.In_Radar(cell)) continue;

		int d = ::Distance(Cell_Coord(cell), house->Center);
		if (want == 0 || d < best) { want = cell; best = d; }
	}
	if (want == 0) return;

	/*
	**	Somebody already on the way, so nobody else is needed.
	*/
	UnitClass * runner = NULL;
	int fastest = -1;
	for (i = 0; i < Units.Count(); i++) {
		UnitClass * u = Units.Ptr(i);
		if (u == NULL || !u->IsActive || u->IsInLimbo || u->House != house) continue;
		if (u->Strength == 0 || u->Class == NULL) continue;
		if (u->Flagged != HOUSE_NONE) continue;
		if (CAC_Is_Miner(*u)) continue;

		if (Target_Legal(u->NavCom) && As_Cell(u->NavCom) == want) return;
		if (CAC_On_Crate_Errand(u)) continue;

		if ((int)u->Class->MaxSpeed > fastest) {
			fastest = (int)u->Class->MaxSpeed;
			runner = u;
		}
	}
	if (runner == NULL) return;

	runner->Assign_Target(TARGET_NONE);
	runner->Assign_Destination(::As_Target(want));
	runner->Assign_Mission(MISSION_MOVE);

	CAC_Log_Volatile("flag", "%s sends %s at the flag on %d,%d",
						 house->Class->IniName,
						 runner->Class->IniName, Cell_X(want), Cell_Y(want));
}


/***********************************************************************************************
 * CAC_Flag_Guard_Rank -- Who is posted over the flag first.                                    *
 *                                                                                             *
 *    Nought is the Alien Commander and Santa, one is everything else on foot, two is a         *
 *    vehicle.  Not a judgement about quality:  infantry cannot carry a flag at all, so posting *
 *    them costs the raid nothing, while a Speeder held back to guard is the one unit that      *
 *    could have been scoring.                                                                  *
 *=============================================================================================*/
static int CAC_Flag_Guard_Rank(TechnoClass const * obj)
{
	if (obj == NULL) return(2);

	if (obj->What_Am_I() != RTTI_INFANTRY) return(2);

	int code = CAC_Code_Of(RTTI_INFANTRYTYPE, (int)*(InfantryClass const *)obj);
	int bare = code & ~CAC_CODE_EVIL;
	if (bare == CAC_CODE_UQ || bare == CAC_CODE_UM) return(0);
	return(1);
}


/***********************************************************************************************
 * CAC_AI_Flag_Guard -- Stand over our own flag, wherever it has ended up.                      *
 *                                                                                             *
 *    A house cannot recover its own flag by touching it.  UnitClass::Per_Cell_Process opens    *
 *    the pickup with                                                                          *
 *                                                                                             *
 *        if (Map[cell].IsFlagged && !House->Is_Ally(Map[cell].Owner))                          *
 *                                                                                             *
 *    so the one flag you may never pick up is your own.  A dropped one stays where the carrier *
 *    died -- Flag_Attach(Coord_Cell(Coord)) with set_home false, so FlagLocation moves and     *
 *    FlagHome does not -- until somebody hostile walks over it.  There are exactly two things  *
 *    that can be done about that: stand on the ground around it, and kill whoever comes for    *
 *    it.  Which makes guarding it the whole of the defensive game rather than a nicety.        *
 *                                                                                             *
 *    Where, and not merely how many: the detail follows CAC_Flag_Cell rather than FlagHome, so *
 *    a flag dropped halfway across the map is guarded where it is lying.                       *
 *                                                                                             *
 *    Anchored with MISSION_GUARD_AREA on the flag cell, which engages what comes near without  *
 *    chasing it over the horizon.  CAC_Guarding_Flag is how CAC_AI_Patrol knows to leave them  *
 *    where they are, since it recruits out of GUARD_AREA on purpose.                           *
 *=============================================================================================*/
void CAC_AI_Flag_Guard(HouseClass * house)
{
	if (house == NULL || house->IsHuman || !house->IsActive) return;
	if (house->Class == NULL) return;
	if (!Special.IsCaptureTheFlag || GameToPlay == GAME_NORMAL) return;
	if (!CAC_AI_Runs_For(house)) return;

	if ((Frame % (TICKS_PER_SECOND * 2)) != 0) return;

	CELL flag = CAC_Flag_Cell(house);
	if (flag == 0 || !Map.In_Radar(flag)) return;

	/*
	**	The fastest vehicle stays free.  It is the only kind that can score, and a
	**	Speeder standing on our own flag wins nothing.
	*/
	UnitClass * spare = NULL;
	int fastest = -1;
	int i;
	for (i = 0; i < Units.Count(); i++) {
		UnitClass * u = Units.Ptr(i);
		if (u == NULL || !u->IsActive || u->IsInLimbo || u->House != house) continue;
		if (u->Strength == 0 || u->Class == NULL || CAC_Is_Miner(*u)) continue;
		if ((int)u->Class->MaxSpeed > fastest) { fastest = (int)u->Class->MaxSpeed; spare = u; }
	}

	/*
	**	How many to hold there: a third of what is armed, never fewer than two, so
	**	a small force still posts a picket and a large one does not empty the map.
	*/
	int armed = 0;
	for (i = 0; i < Units.Count(); i++) {
		UnitClass const * u = Units.Ptr(i);
		if (u == NULL || !u->IsActive || u->IsInLimbo || u->House != house) continue;
		if (u->Strength == 0 || CAC_Is_Miner(*u) || !u->Is_Weapon_Equipped()) continue;
		armed++;
	}
	for (i = 0; i < Infantry.Count(); i++) {
		InfantryClass const * man = Infantry.Ptr(i);
		if (man == NULL || !man->IsActive || man->IsInLimbo || man->House != house) continue;
		if (man->Strength == 0 || !man->Is_Weapon_Equipped()) continue;
		if (man->Class == NULL || man->Class->IsCapture || CAC_Is_Decoy(man)) continue;
		armed++;
	}
	int want = MAX(2, armed / 3);

	/*
	**	Three passes, best first, so the Commander and Santa are posted before
	**	anything else is looked at and a vehicle is taken only when nothing on
	**	foot is left.
	*/
	int posted = 0;
	for (int pass = 0; pass < 3 && posted < want; pass++) {

		for (i = 0; i < Infantry.Count() && posted < want; i++) {
			InfantryClass * man = Infantry.Ptr(i);
			if (man == NULL || !man->IsActive || man->IsInLimbo || man->House != house) continue;
			if (man->Strength == 0 || !man->Is_Weapon_Equipped()) continue;
			if (man->Class == NULL || man->Class->IsCapture || CAC_Is_Decoy(man)) continue;
			if (CAC_Flag_Guard_Rank(man) != pass) continue;

			if (CAC_Guarding_Flag(man, house)) { posted++; continue; }
			if (Target_Legal(man->TarCom)) continue;		/* in a fight already */
			if (CAC_On_Crate_Errand(man)) continue;		/* and an errand outranks a post */

			man->Assign_Target(TARGET_NONE);
			man->Assign_Destination(::As_Target(flag));
			man->ArchiveTarget = ::As_Target(flag);
			man->Assign_Mission(MISSION_GUARD_AREA);
			posted++;
		}

		if (pass != 2) continue;

		for (i = 0; i < Units.Count() && posted < want; i++) {
			UnitClass * u = Units.Ptr(i);
			if (u == NULL || !u->IsActive || u->IsInLimbo || u->House != house) continue;
			if (u->Strength == 0 || CAC_Is_Miner(*u) || !u->Is_Weapon_Equipped()) continue;
			if (u->Flagged != HOUSE_NONE || u == spare) continue;

			if (CAC_Guarding_Flag(u, house)) { posted++; continue; }
			if (Target_Legal(u->TarCom)) continue;
			if (CAC_On_Crate_Errand(u)) continue;

			u->Assign_Target(TARGET_NONE);
			u->Assign_Destination(::As_Target(flag));
			u->ArchiveTarget = ::As_Target(flag);
			u->Assign_Mission(MISSION_GUARD_AREA);
			posted++;
		}
	}

	CAC_Log_Volatile("flag", "%s holds its flag at %d,%d with %d of %d wanted",
						 house->Class->IniName, Cell_X(flag), Cell_Y(flag), posted, want);
}


/***********************************************************************************************
 * CAC_AI_Repair -- The computer's buildings mend themselves, which is how CrystAlien does it.  *
 *                                                                                             *
 *    "The computer never uses its engineers to repair" is true and cannot be otherwise: in the *
 *    source an engineer reaching a target dispatches on ownership --                           *
 *                                                                                             *
 *        if (target.team != parent.team)                  -> doInfiltrate()                    *
 *        if (target.repairable && health < maxHealth)     -> doRepair()                        *
 *                                                                                             *
 *    -- and UnitHAL hands a computer engineer one target and one only:                          *
 *                                                                                             *
 *        if (this.parent.stats.repair) { this.parent.target = this.parent.parent.friendlyTarget; }  *
 *                                                                                             *
 *    friendlyTarget is picked from the buildings the *player* owns (`_loc6_.friend`), so a     *
 *    computer engineer is always walking at something it does not own and always ends in       *
 *    doInfiltrate.  doRepair is reachable only from the player's repair cursor.                 *
 *                                                                                             *
 *    The computer's repair is somewhere else entirely, in Building.think, and it is not an      *
 *    engineer at all:                                                                          *
 *                                                                                             *
 *        if (!(this.count % 10)) {                                                              *
 *            if (this.health < this.stats.maxHealth) {                                          *
 *                if (this.repairing || !this.friend && (findBuilding("BA_" + oppo, oppo)        *
 *                                                    || findBuilding("BK_" + oppo, oppo))) {    *
 *                    this.repairing = true;                                                     *
 *                    ... spend, health += 20 ...                                                *
 *                                                                                              *
 *    Every damaged computer building mends itself, continuously, for cash, for as long as the  *
 *    house still holds a Headquarters or an Alien Hive -- the same two buildings the last-ditch *
 *    branch above asks after.  Nothing here did that, so a computer base carried its damage     *
 *    for the whole game and every scratch was permanent.                                        *
 *                                                                                              *
 *    Done by turning the engine's own repair on rather than by adding a second one beside it,   *
 *    so the wrench shows, the money is spent through House::Spend_Money, and it stops of its    *
 *    own accord at full health or an empty bank.  Its rate is Tiberian Dawn's -- REPAIR_STEP    *
 *    every fifteen frames against twenty every ten -- and that is the player's rate too;        *
 *    matching the source's would be a change to what repairing costs everybody, which is not    *
 *    what this is.                                                                              *
 *                                                                                              *
 *    Not through BuildingClass::Repair, which calls Clicked_As_Target(PlayerPtr) on the way in  *
 *    and would flash a computer building at whoever is watching.                                *
 *=============================================================================================*/
void CAC_AI_Repair(HouseClass * house)
{
	if (house == NULL || house->IsHuman || !house->IsActive) return;
	if (house->Class == NULL) return;
	if (!CAC_Is_CrystAlien_House(house)) return;

	/*
	**	Building.think asks every ten frames at the source's twenty-three; the
	**	engine's own repair tick is every fifteen and does the spending, so this
	**	only has to be often enough to catch a building becoming damaged.
	*/
	if ((Frame % (TICKS_PER_SECOND / 2)) != 0) return;

	bool allowed = CAC_Has_HQ(house);

	for (int i = 0; i < Buildings.Count(); i++) {
		BuildingClass * b = Buildings.Ptr(i);
		if (b == NULL || !b->IsActive || b->IsInLimbo) continue;
		if (b->House != house || b->Class == NULL) continue;
		if (!CAC_AI_Owns(b)) continue;

		bool hurt = (b->Strength < b->Class->MaxStrength);

		/*
		**	`this.repairing` latches on and is cleared by reaching full health or
		**	running out of money.  The engine's flag behaves the same way, so this
		**	only ever sets it -- and takes it off again if the headquarters goes,
		**	which is the one case the source's condition stops being true.
		*/
		if (allowed && hurt) {
			b->IsRepairing = true;
		} else if (!allowed && b->IsRepairing) {
			b->IsRepairing = false;
		}
	}
}


/***********************************************************************************************
 * CAC_AI_Sortie -- Let the computer's Fighters out, on CrystAlien's terms.                     *
 *                                                                                             *
 *    Nothing was sending them anywhere.  CAC_AI_Aggression walks Units and Infantry and never  *
 *    touches Aircraft, and CAC_Picks_Own_Target stops a boomerang looking for its own target   *
 *    -- correctly, that is what keeps it near its station -- so a computer's Fighters were     *
 *    built, shuttled home, and did nothing else for the whole game.                            *
 *                                                                                             *
 *    UnitHAL is where the source game hands them one, and it is deliberately grudging:         *
 *                                                                                             *
 *        if (this.parent.stats.boomerang) {                                                    *
 *            if (this.parent.weaponPayload != this.parent.stats.maxWeaponPayload) return;      *
 *            if (this.parent.parent.count % 400 < 350) return;                                 *
 *            this.parent.target = this.parent.parent.friendlyTarget;                           *
 *            return;                                                                           *
 *        }                                                                                     *
 *                                                                                             *
 *    A full magazine, and only in a fifty-frame window out of every four hundred -- about two  *
 *    seconds in every seventeen at the source game's frame rate.  That is what makes a Radar   *
 *    Station feel like a station rather than an airfield: the Fighters are mostly sitting on    *
 *    it, and they go out in occasional sorties rather than streaming.                          *
 *                                                                                             *
 *    The window is converted to Tiberian Dawn's fifteen ticks a second rather than copied, so   *
 *    the rhythm is the same length in seconds.                                                  *
 *                                                                                             *
 *    What they are pointed at is not the source's answer.  CrystAlien hands them                *
 *    `friendlyTarget` -- whatever the *player* is currently attacking -- which is a sensible    *
 *    thing to do in a two-sided skirmish against one human and a nonsense in a six-player      *
 *    game.  They take the nearest enemy to their own station instead, which keeps them          *
 *    defending the place they are tethered to.                                                  *
 *=============================================================================================*/
void CAC_AI_Sortie(HouseClass * house)
{
	if (house == NULL || house->IsHuman) return;
	if (!CAC_AI_Runs_For(house)) return;

	/*
	**	Two seconds in every seventeen, which is UnitHAL's fifty frames in four
	**	hundred put into this game's clock.
	*/
	enum {
		CAC_SORTIE_CYCLE = (TICKS_PER_SECOND * 400) / 23,
		CAC_SORTIE_OPEN  = (TICKS_PER_SECOND * 350) / 23
	};
	if (((long)Frame % CAC_SORTIE_CYCLE) < CAC_SORTIE_OPEN) return;

	for (int i = 0; i < Aircraft.Count(); i++) {
		AircraftClass * a = Aircraft.Ptr(i);
		if (a == NULL || !a->IsActive || a->IsInLimbo || a->House != house) continue;
		if (a->Strength == 0 || a->Class == NULL) continue;
		if (!CAC_Is_Boomerang(*a)) continue;

		/*
		**	A full magazine, and nothing already in hand.
		*/
		if (a->Class->MaxAmmo > 0 && a->Ammo < a->Class->MaxAmmo) continue;
		/*
		**	Already out on a run.
		**	
		**	The source's guard is `if (this.parent.target) return;` and the port had it
		**	as a TarCom test -- which stopped working the moment the sortie was changed
		**	to fly a craft *through* a target rather than at one, because that assigns
		**	no TarCom deliberately.
		**	
		**	What was left was the ammo test, and a boomerang that has not fired has full
		**	ammo.  This window is open for thirty-two consecutive frames out of every two
		**	hundred and sixty, and the routine runs on every one of them -- so a craft
		**	would have been handed a fresh destination thirty-two times in a row: never
		**	still, therefore never landing, quietly undoing the re-seating fix made in
		**	the same commit as the change that broke this.
		**	
		**	A destination *is* the run.  Having one means it is already flying it, and
		**	CAC_Return_Home hands it another when the path ends.
		*/
		if (Target_Legal(a->NavCom) || Target_Legal(a->TarCom)) continue;

		BuildingClass * home = CAC_Home_Building(a);
		if (home == NULL) continue;

		/*
		**	The nearest enemy to the station it is tethered to, so a sortie defends
		**	the place it flies from.
		*/
		/*
		**	The house's target, not one of this Fighter's choosing.
		**	
		**	The HAL hands a boomerang `this.parent.parent.friendlyTarget` -- one
		**	building, drawn at random from the enemy's, shared by every Fighter the
		**	house releases and re-rolled on its own timer.  See CAC_House_Target.
		**	
		**	This used to walk the display layer for the nearest enemy to the station,
		**	which is a different behaviour and a worse one on both counts: each craft
		**	picked separately, so they scattered instead of arriving together, and
		**	nearest-first meant a Fighter spent the game killing whatever had wandered
		**	closest to home rather than being sent somewhere.  It was also one more
		**	walk of a list that can hold objects that are gone.
		*/
		BuildingClass * best = CAC_House_Target(house);
		if (best == NULL) continue;

		CAC_Log("berth", "%s sorties at %s, %d leptons from its station",
					a->Class->IniName,
					(best->Class != NULL) ? best->Class->IniName : "?",
					(int)::Distance(home->Center_Coord(), best->Center_Coord()));

		/*
		**	Flown *through*, not *at*.
		**	
		**	MISSION_ATTACK is Tiberian Dawn's attack run: fly to the target, stop over
		**	it, and shoot until one of you is finished.  CrystAlien's boomerang has no
		**	such mission -- the HAL hands it a voyage, engageEnemy fires at whatever
		**	comes inside range along the way, and UnitNav.handle turns it round the
		**	moment the path runs out:
		**	
		**	    if (this.parent.stats.boomerang) this.returnHome();
		**	
		**	So the order is a move to where the target is, and the shooting is
		**	opportunistic on the way -- see CAC_Strafe.  That is the difference between
		**	hovering over a Defence Station trading shots with it and crossing the base
		**	hitting whatever is beneath, which is what a boomerang is for.
		**	
		**	No target is assigned deliberately.  A TarCom would pull it back into the
		**	engine's attack behaviour and undo the whole thing.
		*/
		/*
		**	A target to shoot and a path that goes there -- both, and neither alone.
		**	
		**	The HAL hands a boomerang the team's shared target and nothing else:
		**	
		**	    this.parent.target = this.parent.parent.friendlyTarget;
		**	
		**	and engageEnemy will not fire without one.  So the target is what makes it
		**	shoot, and it must be set.
		**	
		**	The mission is a move rather than an attack because those are the engine's
		**	two halves and only one of them is wanted.  MISSION_ATTACK would fly the
		**	craft to the target and hold station over it; MISSION_MOVE flies it there
		**	and finishes, and a finished path is what CAC_Return_Home is watching for.
		**	Arriving is what turns it round -- that is where the hit and run comes
		**	from, not from the shooting.
		*/
		a->Assign_Target(best->As_Target());
		a->Assign_Destination(::As_Target(Coord_Cell(best->Center_Coord())));
		a->Assign_Mission(MISSION_MOVE);
	}
}


/***********************************************************************************************
 * CAC_AI_Under_Attack -- Has this house had a building hit lately?                             *
 *                                                                                             *
 *    Reads the record BuildingClass::Take_Damage now keeps.  A minute, which is the same       *
 *    window HouseClass::AI uses to enter and leave STATE_ATTACKED.                             *
 *=============================================================================================*/
bool CAC_AI_Under_Attack(HouseClass const * house)
{
	if (house == NULL || house->LATime <= 0) return(false);
	return((long)house->LATime + TICKS_PER_MINUTE > (long)Frame);
}


/***********************************************************************************************
 * CAC_Revenue_Target -- What the enemy is earning with.                                       *
 *                                                                                             *
 *    The Aliens have no Driller and cannot get one built.  CrystAlien does not hand them a     *
 *    substitute either -- what it hands them is a reason, and the levels that play the two      *
 *    sides against one another are won by acting on it: take the Astros' income away, by        *
 *    capture where a Saboteur can reach and by destruction where one cannot.                    *
 *                                                                                             *
 *    So this is the Alien computer's standing objective, and it is deliberately narrower than  *
 *    CAC_Best_Assault.  Three things earn on the Astro side and nothing else does:              *
 *                                                                                             *
 *      1. A Driller.  It earns without moving and without a unit, so killing one is a          *
 *         permanent cut rather than an interruption.                                            *
 *      2. A loaded Crystal Miner.  Revenue on legs, and the cargo dies with it -- a Miner shot  *
 *         on the way home costs its owner the whole trip, not a share of it.                    *
 *      3. The Command Base and the Ops Ship, which are where a Miner unloads.  Ranked last of   *
 *         the three because they are also the best defended and CAC_Best_Assault will get to    *
 *         them anyway.                                                                          *
 *                                                                                             *
 *    Nearest first inside a tier, so a raid does not walk past the Driller it came for.         *
 *                                                                                             *
 * OUTPUT:  something to shoot, or NULL if the enemy has no visible income.                     *
 *=============================================================================================*/
static TechnoClass * CAC_Revenue_Target(HouseClass const * house)
{
	if (house == NULL || house->Class == NULL) return(NULL);

	COORDINATE home = house->Center;
	TechnoClass * best = NULL;
	int bestworth = 0;
	int bestrange = 0;

	for (int i = 0; i < Buildings.Count(); i++) {
		BuildingClass * b = Buildings.Ptr(i);
		if (b == NULL || !b->IsActive || b->IsInLimbo || b->Strength == 0) continue;
		if (b->House == NULL || house->Is_Ally(b->House)) continue;
		if (b->Class == NULL || !b->Class->IsLegalTarget) continue;
		if (CAC_Pizza_Protected(b)) continue;	/* cannot be hurt -- see CAC_Pizza_Protected */

		int code = CAC_Code_Of(RTTI_BUILDINGTYPE, (int)b->Class->Type) & ~CAC_CODE_EVIL;
		int worth = 0;
		if (code == CAC_CODE_BL) worth = 3;
		if (code == CAC_CODE_BA || code == CAC_CODE_BK) worth = 1;
		if (worth == 0) continue;

		int range = (int)::Distance(home, b->Center_Coord());
		if (best == NULL || worth > bestworth || (worth == bestworth && range < bestrange)) {
			best = b; bestworth = worth; bestrange = range;
		}
	}

	for (int i = 0; i < Units.Count(); i++) {
		UnitClass * u = Units.Ptr(i);
		if (u == NULL || !u->IsActive || u->IsInLimbo || u->Strength == 0) continue;
		if (u->House == NULL || house->Is_Ally(u->House)) continue;
		if (!CAC_Is_Miner(*u)) continue;

		/*
		**	A Miner standing empty on a field is worth less than one carrying a
		**	load home, but not nothing -- it is going to be carrying one shortly.
		*/
		int worth = (CAC_Load_Steps(u) > 0) ? 2 : 1;

		int range = (int)::Distance(home, u->Center_Coord());
		if (best == NULL || worth > bestworth || (worth == bestworth && range < bestrange)) {
			best = u; bestworth = worth; bestrange = range;
		}
	}

	return(best);
}


/***********************************************************************************************
 * CAC_AI_Patrol -- Keep some of the force out on the map instead of all of it at home.        *
 *                                                                                             *
 *    CAC_AI_Aggression puts every idle armed thing on MISSION_HUNT, which is the engine's own  *
 *    "go and find somebody".  It is the right answer for a force that has decided to attack    *
 *    and the wrong one for a map, because a hunter walks at whatever it can see and everything *
 *    ends up in the same place: either massed on the enemy or standing in the base.  Nothing   *
 *    ever sat between the two, so crates went uncontested, expansions were never noticed, and  *
 *    a raid crossing open ground met nothing until it arrived.                                 *
 *                                                                                             *
 *    A patrol is the missing state.  Units are sent to guard a point away from home and left   *
 *    there -- MISSION_GUARD_AREA already does the hard part, engaging what comes near without  *
 *    chasing it over the horizon -- and the points rotate, so over a few minutes the force     *
 *    sweeps the map rather than picketing one spot.                                            *
 *                                                                                             *
 *    The points are the playable map in thirds, centres of the nine cells of a noughts-and-    *
 *    crosses grid, which needs nothing known about the map and covers it evenly.  Which point  *
 *    a given unit takes is its index plus the round number, so they move on together and a     *
 *    unit does not sit in one corner for the whole game.                                       *
 *                                                                                             *
 *    Two things it deliberately does not do.  It never sends the last of the garrison out --   *
 *    half the force stays home, and below four armed units nothing patrols at all, because a   *
 *    base that patrols itself empty is a base that gets walked into.  And it stands down while *
 *    the house is under attack: CAC_AI_Aggression and CAC_AI_Raid want everything then, and    *
 *    they take patrolling units back because GUARD_AREA is only respected while things are     *
 *    quiet.                                                                                    *
 *=============================================================================================*/
static CELL CAC_Patrol_Point(int slot, SpeedType speed)
{
	int w = Map.MapCellWidth / 3;
	int h = Map.MapCellHeight / 3;

	/*
	**	Anywhere inside the ninth, not its exact centre.
	**
	**	UnitHAL picks a uniformly random tile and rejects the blocked ones:
	**
	**	    do { x = random(cols); y = random(rows); } while (arena.tiles[x][y]);
	**
	**	Nine fixed points is the grid keeping that even, but taken literally it
	**	also means every patrolling unit of a house walks at one of nine cells,
	**	queues behind whoever got there first, and stops dead on arrival because
	**	the destination it has just been handed is the cell it is standing in.
	**	The jitter is the source's randomness put back inside the grid's evenness.
	**
	**	Sim_Random_Pick and not Random_Pick: this decides where the computer's
	**	army goes, so every client has to draw the same number.
	*/
	for (int tries = 0; tries < 8; tries++) {
		int gx = Map.MapCellX + (Map.MapCellWidth  * (slot % 3)) / 3;
		int gy = Map.MapCellY + (Map.MapCellHeight * (slot / 3)) / 3;
		if (w > 1) gx += Sim_Random_Pick(0, w - 1);
		if (h > 1) gy += Sim_Random_Pick(0, h - 1);

		CELL cell = XY_Cell(gx, gy);
		if (!Map.In_Radar(cell)) continue;
		if (Ground[Map[cell].Land_Type()].Cost[speed] == 0) continue;
		return(cell);
	}
	return(0);
}


static bool CAC_Patrol_Send(FootClass * foot, SpeedType speed, int slot)
{
	/*
	**	Guarding, or hunting with nothing found.
	**
	**	Taking only MISSION_GUARD looked right and meant this routine could never
	**	find anybody: CAC_AI_Aggression runs every two seconds and puts every idle
	**	unit on MISSION_HUNT, so by the time a patrol is due -- twenty seconds --
	**	there is nothing left in the pool it was drawing from.  The whole session
	**	logged not one patrol.
	**
	**	A hunter that has not found anything is exactly the unit that should be out
	**	on the map, so that is the pool.  One with a target is left on it.
	**
	**	Including one already patrolling, which is what makes it a patrol: the
	**	points rotate by round so the force sweeps rather than pickets, and that
	**	only works if a unit that has reached its point can be handed the next one.
	**	Only ones that have arrived -- the NavCom test leaves anything still on its
	**	way alone, so a unit is not turned round halfway there.
	*/
	if (foot->Mission != MISSION_GUARD && foot->Mission != MISSION_HUNT &&
		foot->Mission != MISSION_GUARD_AREA) return(false);
	if (Target_Legal(foot->NavCom) || Target_Legal(foot->TarCom)) return(false);

	/*
	**	Except the flag guard, which is standing in GUARD_AREA on purpose and is
	**	the one detail that must not be swept up by a patrol.  See
	**	CAC_AI_Flag_Guard.
	*/
	if (CAC_Guarding_Flag(foot, foot->House)) return(false);

	CELL cell = CAC_Patrol_Point(slot, speed);
	if (cell == 0) return(false);

	foot->Assign_Target(TARGET_NONE);
	foot->Assign_Destination(::As_Target(cell));

	/*
	**	And the point is the anchor, which it was not.
	**
	**	Mission_Guard_Area guards ArchiveTarget, not NavCom, and opens by filling
	**	an illegal one in from where the unit is standing:
	**
	**	    if (!Target_Legal(ArchiveTarget)) ArchiveTarget = As_Target(Coord_Cell(Coord));
	**
	**	Nothing here ever set it, so every patrol anchored itself to the base it
	**	was leaving.  The unit walked to its point, arrived, found itself further
	**	from the anchor than its weapon reaches, and was sent straight home again.
	**	"Sent to guard a point away from home and left there" described a shuttle.
	**
	**	After Assign_Destination, which does not touch it but reads better this
	**	way round.
	*/
	foot->ArchiveTarget = ::As_Target(cell);
	foot->Assign_Mission(MISSION_GUARD_AREA);
	return(true);
}


void CAC_AI_Patrol(HouseClass * house)
{
	if (house == NULL || house->IsHuman || !house->IsActive) return;
	if (house->Class == NULL) return;
	if (!CAC_Is_CrystAlien_House(house)) return;

	/*
	**	Not while it is being attacked.  Everything is wanted at home or on the
	**	raid, and CAC_AI_Aggression reclaims these the moment that is true.
	*/
	if (CAC_AI_Under_Attack(house)) return;

	int id = Houses.ID(house);
	int interval = TICKS_PER_SECOND * 20;
	if (((int)Frame + id * 13) % interval != 0) return;

	/*
	**	How many can be spared.  Count first, then send half, so the answer does
	**	not depend on the order the list happens to be in.
	**
	**	Infantry as well as vehicles, which is the whole of why nobody wandered.
	**	CrystAlien has one Unit class and gives every non-miner computer unit the
	**	same UnitHAL, so a trooper roams exactly as a buggy does.  Split across two
	**	heaps here, this routine walked Units and stopped -- and on this roster the
	**	armed vehicles are the Rover, the Crusher and the Crystal Reaper against
	**	five infantry types, so the count it was taking was not merely incomplete
	**	but usually under four, and the floor below then turned the whole routine
	**	off.  The computer attacked and infiltrated, because those routines do read
	**	Infantry, and never patrolled.
	*/
	int armed = 0;
	int i;
	for (i = 0; i < Units.Count(); i++) {
		UnitClass const * u = Units.Ptr(i);
		if (u == NULL || !u->IsActive || u->IsInLimbo || u->House != house) continue;
		if (!CAC_AI_Owns(u)) continue;
		if (u->Strength == 0 || CAC_Is_Miner(*u) || u->Flagged != HOUSE_NONE ||
			!u->Is_Weapon_Equipped()) continue;
		armed++;
	}
	for (i = 0; i < Infantry.Count(); i++) {
		InfantryClass const * man = Infantry.Ptr(i);
		if (man == NULL || !man->IsActive || man->IsInLimbo || man->House != house) continue;
		if (!CAC_AI_Owns(man)) continue;
		if (man->Strength == 0 || !man->Is_Weapon_Equipped()) continue;

		/*
		**	Not the engineers or the decoys, who have somewhere better to be.  See
		**	CAC_AI_Infiltrate, which is picking raids out of the same pool, and
		**	CAC_AI_Aggression, which leaves them alone for the same reason.
		*/
		if (man->Class->IsCapture || CAC_Is_Decoy(man)) continue;
		armed++;
	}
	if (armed < 4) return;

	int want = armed / 2;
	int sent = 0;

	/*
	**	The grid.  Map extents rather than the whole 64x64, so the patrol stays
	**	inside the part of the map that can actually be walked on.
	*/
	int round = (int)(Frame / interval);

	for (i = 0; i < Units.Count() && sent < want; i++) {
		UnitClass * u = Units.Ptr(i);
		if (u == NULL || !u->IsActive || u->IsInLimbo || u->House != house) continue;
		if (!CAC_AI_Owns(u)) continue;
		if (u->Strength == 0 || CAC_Is_Miner(*u) || u->Flagged != HOUSE_NONE ||
			!u->Is_Weapon_Equipped()) continue;
		if (u->Class == NULL) continue;

		if (CAC_Patrol_Send(u, u->Class->Speed, (sent + round) % 9)) sent++;
	}

	for (i = 0; i < Infantry.Count() && sent < want; i++) {
		InfantryClass * man = Infantry.Ptr(i);
		if (man == NULL || !man->IsActive || man->IsInLimbo || man->House != house) continue;
		if (!CAC_AI_Owns(man)) continue;
		if (man->Strength == 0 || !man->Is_Weapon_Equipped()) continue;
		if (man->Class == NULL || man->Class->IsCapture || CAC_Is_Decoy(man)) continue;

		if (CAC_Patrol_Send(man, SPEED_FOOT, (sent + round) % 9)) sent++;
	}

	CAC_Log_Volatile("patrol", "%s: %d of %d armed sent out (wanted %d)",
						 house->Class->IniName, sent, armed, want);
}


/***********************************************************************************************
 * CAC_AI_Census -- Write down what each computer base actually has.                           *
 *                                                                                             *
 *    "The Alien computers build sparse bases and the Astro ones sprawl" is the kind of report  *
 *    that is obviously true when you are looking at the map and impossible to act on           *
 *    afterwards, because nothing anywhere writes down how big a base got or what paid for it.  *
 *    Every other question on this project was settled by logging the thing and reading it      *
 *    back, and this one had no instrument at all.                                              *
 *                                                                                             *
 *    So: one line per computer house every half minute, carrying the three numbers that decide *
 *    how big a base is allowed to get.  CurBuildings is the one that matters most -- every      *
 *    ratio in AI_Building is taken against it, so it feeds itself -- and the Driller count and  *
 *    the cash are what drive it.                                                               *
 *                                                                                             *
 *    Volatile, because it is a running measurement rather than an event, and staggered by      *
 *    house so six players do not all write on the same frame.                                  *
 *=============================================================================================*/
void CAC_AI_Census(HouseClass * house)
{
	if (house == NULL || house->IsHuman || !house->IsActive) return;
	if (house->Class == NULL) return;
	if (!CAC_Logging()) return;

	int id = Houses.ID(house);
	if (((int)Frame + id * 37) % (TICKS_PER_SECOND * 30) != 0) return;

	/*
	**	And nothing about a house that has nothing.  Special and Neutral are
	**	computer houses that never build, so two thirds of this channel was two
	**	rows of zeroes repeated every half minute, in front of the one house the
	**	census was written to answer questions about.
	*/
	if (house->CurBuildings == 0 && house->Available_Money() == 0) return;

	CAC_Install_Crash_Filter();		/* in case something else has taken it -- see there */

	/*
	**	The Driller, on whichever side this house is -- the only building in the
	**	game that earns without a unit carrying anything.
	*/
	int drills = 0;
	int drill = CAC_Slot_For(RTTI_BUILDINGTYPE, CAC_CODE_BL,
									 CAC_Side(house->Class->House) == HOUSE_BAD);
	if (drill >= 0) {
		/*	Counted off the map rather than out of BQuantity, which is private. */
		for (int i = 0; i < Buildings.Count(); i++) {
			BuildingClass const * b = Buildings.Ptr(i);
			if (b == NULL || !b->IsActive || b->IsInLimbo || b->House != house) continue;
			if (b->Class != NULL && (int)b->Class->Type == drill) drills++;
		}
	}

	CACPowerState power = CAC_Power_State(house);

	CAC_Log_Volatile("base", "%s: %u building(s), %u miner(s), %d driller(s), %ld credits, power %s",
						 house->Class->IniName,
						 house->CurBuildings,
						 house->Miner_Count(),
						 drills,
						 house->Available_Money(),
						 (power == CAC_POWER_OFF) ? "off" :
						 (power == CAC_POWER_LOW) ? "low" : "full");

	/*
	**	And what is keeping a house alive once its base is gone.
	**
	**	Reported as a game that cannot be won, with every side showing no
	**	buildings and the round refusing to end -- so something is still out
	**	there, and hunting for it across sixty cells of map is not a job for a
	**	player.  Destroy All ends a house when it holds neither a building nor a
	**	unit, so if the buildings are gone and it is still standing, the answer
	**	is a unit, and this says which and where.
	**
	**	Only when the base is gone, so it costs nothing during a normal game, and
	**	damped to once a minute per house because the census runs often.
	*/
	if (house->CurBuildings == 0) {
		static long _told[HOUSE_COUNT];
		static bool _init = false;
		if (!_init) {
			for (int i = 0; i < HOUSE_COUNT; i++) _told[i] = -99999;
			_init = true;
		}

		int which = (int)house->Class->House;
		if (which >= 0 && which < HOUSE_COUNT &&
			(long)Frame - _told[which] > (long)(TICKS_PER_MINUTE)) {

			int said = 0;
			for (int pass = 0; pass < 3 && said < 8; pass++) {
				int count = (pass == 0) ? Units.Count()
						  : (pass == 1) ? Infantry.Count() : Aircraft.Count();
				for (int i = 0; i < count && said < 8; i++) {
					TechnoClass const * o =
						(pass == 0) ? (TechnoClass const *)Units.Ptr(i)
					  : (pass == 1) ? (TechnoClass const *)Infantry.Ptr(i)
									: (TechnoClass const *)Aircraft.Ptr(i);
					if (o == NULL || !o->IsActive || o->House != house) continue;
					if (o->Strength == 0) continue;

					CELL at = Coord_Cell(o->Center_Coord());
					CAC_Log("survivor", "%s still holds %s at %d,%d%s",
								house->Class->IniName,
								o->Techno_Type_Class()
									? o->Techno_Type_Class()->IniName : "?",
								Cell_X(at), Cell_Y(at),
								o->IsInLimbo ? " (in limbo)" : "");
					said++;
				}
			}
			if (said > 0) _told[which] = (long)Frame;
		}
	}
}


/***********************************************************************************************
 * CAC_Best_Assault -- The building a raid is worth mounting against.                          *
 *                                                                                             *
 *    Not CAC_Best_Raid, which is the infiltrator's picker and answers only buildings that     *
 *    can be *captured*.  A raid shoots, so it wants a different set and a different order.    *
 *                                                                                             *
 *    A raid wants to cost the enemy something that takes time to replace, so this ranks by     *
 *    what a building *does* rather than by what it cost or by how dangerous it is.  Ranking    *
 *    by danger is CAC_Target_Score's job and is the right answer for a gun choosing between    *
 *    things in range; it is the wrong answer here, because it sends every raid this house      *
 *    ever mounts into the teeth of the same Defence Turret.                                    *
 *                                                                                             *
 *    So: a factory outranks a headquarters outranks anything else, and distance from the       *
 *    raiders' own ground breaks ties.  Defences are not excluded -- a base of nothing but      *
 *    turrets should still be attacked -- they are just last.                                   *
 *                                                                                             *
 * OUTPUT:  The building to aim at, or NULL if the house can see nothing worth raiding.        *
 *=============================================================================================*/
static BuildingClass * CAC_Best_Assault(HouseClass const * house)
{
	if (house == NULL) return(NULL);

	COORDINATE home = house->Center;
	BuildingClass * best = NULL;
	int bestscore = 0;

	for (int i = 0; i < Buildings.Count(); i++) {
		BuildingClass * b = Buildings.Ptr(i);
		if (b == NULL || !b->IsActive || b->IsInLimbo || b->Strength == 0) continue;
		if (b->House == NULL || house->Is_Ally(b->House)) continue;
		if (b->Class == NULL || !b->Class->IsLegalTarget) continue;
		if (CAC_Pizza_Protected(b)) continue;	/* cannot be hurt -- see CAC_Pizza_Protected */

		/*
		**	Worth, in the order a raid should care about it.  The dock test catches
		**	the headquarters and the secondary base on both sides at once, which is
		**	the whole reason those carry codes rather than slot numbers.
		*/
		int worth = 100;
		if (CAC_Is_Code(RTTI_BUILDINGTYPE, b->Class->Type, CAC_CODE_BA) ||
			CAC_Is_Code(RTTI_BUILDINGTYPE, b->Class->Type, CAC_CODE_BK)) worth = 300;
		if (b->Class->ToBuild != RTTI_NONE) worth = 400;

		/*
		**	Distance in cells, floored at one so that something sitting on top of the
		**	raiders cannot divide by zero and so that the near field does not swamp
		**	the ranking entirely.
		*/
		int range = (int)::Distance(home, b->Center_Coord()) / CELL_LEPTON_W;
		if (range < 1) range = 1;

		int score = (worth * 32) / range;
		if (best == NULL || score > bestscore) { best = b; bestscore = score; }
	}

	return(best);
}


/***********************************************************************************************
 * CAC_AI_Raid -- Send a group at one target together, rather than singly.                      *
 *                                                                                             *
 *    MISSION_HUNT is what CAC_AI_Aggression gives an idle unit, and it is a solitary order:    *
 *    each unit finds its own nearest enemy and walks into it alone.  Against a defended base   *
 *    that is a queue of casualties, which is what "they amass all this money and never do       *
 *    anything" looks like from the other side of the map -- they were doing something, one at   *
 *    a time, and dying for it.                                                                 *
 *                                                                                             *
 *    So: gather what is idle and point all of it at the same building at the same moment.      *
 *    CrystAlien has no equivalent -- its computer roams and bumps into things -- and this is    *
 *    another deliberate departure, of a piece with the crate collecting and the base building.  *
 *                                                                                             *
 *    Threat changes both numbers.  A house that has had a building hit inside the last minute   *
 *    commits more and commits it sooner, and aims at whatever is nearest its own base rather    *
 *    than at the far side of the map, which reads as answering an attack instead of ignoring    *
 *    it.                                                                                       *
 *=============================================================================================*/
void CAC_AI_Raid(HouseClass * house)
{
	if (house == NULL || house->IsHuman) return;
	if (!CAC_Is_CrystAlien_House(house)) return;

	bool threatened = CAC_AI_Under_Attack(house);

	/*
	**	Half a minute between raids, or twelve seconds while being attacked.
	*/
	int every = threatened ? (TICKS_PER_SECOND * 12) : (TICKS_PER_SECOND * 30);
	if (((long)Frame + (long)house->Class->House) % every) return;

	/*
	**	Something worth walking into.  When threatened, whatever is nearest home;
	**	otherwise the best thing to take off an enemy.
	*/
	TARGET aim = TARGET_NONE;
	COORDINATE where = 0;

	if (threatened) {
		BuildingClass * anchor = NULL;
		for (int b = 0; b < Buildings.Count(); b++) {
			BuildingClass * mine = Buildings.Ptr(b);
			if (mine != NULL && mine->IsActive && !mine->IsInLimbo && mine->House == house) {
				anchor = mine;
				break;
			}
		}
		if (anchor != NULL) {
			/*
			**	Not called `near`: that is still a macro from the old segment keywords
			**	and windows.h defines it away to nothing, which turns the declaration
			**	into a syntax error a long way from anything that looks wrong.
			*/
			TechnoClass * closest = NULL;
			int bestrange = 0;
			for (int k = 0; k < Logic.Count(); k++) {
				ObjectClass * obj = Logic[k];
				/*	See the other Logic walk -- same kind of list, same hazard. */
				if (!CAC_Is_Live_Object(obj) || !obj->Is_Techno()) continue;
				TechnoClass * foe = (TechnoClass *)obj;
				if (foe->IsInLimbo || foe->Strength == 0 || foe->House == NULL) continue;
				if (house->Is_Ally(foe) || !foe->Class_Of().IsLegalTarget) continue;

				/*	And only where this house has been -- see CAC_Has_Scouted. */
				if (!CAC_Has_Scouted(house, Coord_Cell(foe->Center_Coord()))) continue;

				int range = (int)::Distance(anchor->Center_Coord(), foe->Center_Coord());
				if (closest == NULL || range < bestrange) { closest = foe; bestrange = range; }
			}
			if (closest != NULL) { aim = closest->As_Target(); where = closest->Center_Coord(); }
		}
	}

	/*
	**	An Alien raid goes for the Astros' income before it goes for anything else.
	**	
	**	They have no Driller of their own and there is no Alien one to give them, so
	**	their whole economic position is what they carry home plus what they take off
	**	somebody else.  A computer that raids whatever is nearest never makes that
	**	trade; one that goes for Drillers and loaded Miners does, and it is the
	**	strategy the source's own side-against-side levels are built around.
	**	
	**	Not while it is being attacked -- the branch above has already aimed at
	**	whatever is nearest its own ground, and answering an attack comes first.
	*/
	/*
	**	And in Pizza Mode it is everybody's raid, not just the Aliens'.
	**
	**	The gate above is the source's reason: the Aliens have no Driller and
	**	cannot get one, so taking the Astros' income is their whole economic
	**	position.  An Astro house has its own Driller and no such need -- in an
	**	ordinary game.
	**
	**	Pizza Mode removes the "ordinary".  Nobody can be killed, so an army buys
	**	nothing on its own; the only thing worth taking from a rival is the fifty
	**	thousand credits they are saving, and the only way to take it is to shoot
	**	what earns it.  A computer that raids the nearest building instead spends
	**	the game knocking down Power Plants that go straight back up, which is what
	**	was reported: a bot that built and raided and never went after the miners.
	*/
	bool revenue = (CAC_Side(house->Class->House) == HOUSE_BAD) || CAC_Pizza_Mode();

	if (!Target_Legal(aim) && revenue) {
		TechnoClass * earner = CAC_Revenue_Target(house);
		if (earner != NULL) {
			aim = earner->As_Target();
			where = earner->Center_Coord();

			CAC_Log_Volatile("ai.raid", "%s goes for %s's income (%s)",
								  house->Class->IniName,
								  (earner->House && earner->House->Class)
									  ? earner->House->Class->IniName : "?",
								  earner->Techno_Type_Class()
									  ? earner->Techno_Type_Class()->IniName : "?");
		}
	}

	if (!Target_Legal(aim)) {
		BuildingClass * prize = CAC_Best_Assault(house);
		if (prize == NULL) {
			CAC_Log_Volatile("ai.raid", "%s has nothing to raid", house->Class->IniName);
			return;
		}
		aim = prize->As_Target();
		where = prize->Center_Coord();
	}
	if (!Target_Legal(aim)) return;

	/*
	**	And who goes.  A raid is worth mounting only if enough of it turns up at
	**	once; four is a nuisance, eight is a problem.
	*/
	int want = threatened ? 8 : 5;
	int sent = 0;

	for (int i = 0; i < Units.Count() && sent < want; i++) {
		UnitClass * u = Units.Ptr(i);
		if (u == NULL || !u->IsActive || u->IsInLimbo || u->House != house) continue;
		if (!CAC_AI_Owns(u)) continue;
		if (u->Strength == 0 || CAC_Is_Miner(*u) || u->Flagged != HOUSE_NONE ||
			!u->Is_Weapon_Equipped()) continue;

		/*
		**	Anything already on a deliberate errand is left on it -- a crate run, a
		**	capture, a delivery.  Hunting is not one of those, and is exactly what a
		**	raid should be gathering up.
		*/
		if (u->Mission == MISSION_ENTER || u->Mission == MISSION_CAPTURE ||
			u->Mission == MISSION_HARVEST || u->Mission == MISSION_UNLOAD) continue;
		if (Target_Legal(u->NavCom) && u->Mission == MISSION_MOVE) continue;

		u->Assign_Target(aim);
		u->Assign_Mission(MISSION_ATTACK);
		sent++;
	}

	for (int i = 0; i < Aircraft.Count() && sent < want; i++) {
		AircraftClass * a = Aircraft.Ptr(i);
		if (a == NULL || !a->IsActive || a->IsInLimbo || a->House != house) continue;
		if (!CAC_AI_Owns(a)) continue;
		if (a->Strength == 0 || a->Class == NULL || !a->Is_Weapon_Equipped()) continue;
		if (CAC_Is_Boomerang(*a)) continue;			/* tethered; CAC_AI_Sortie decides */
		if (Target_Legal(a->TarCom)) continue;

		a->Assign_Target(aim);
		a->Assign_Mission(MISSION_ATTACK);
		sent++;
	}

	/*
	**	Reported either way.  The first session with this in it logged not one raid
	**	and there was no way to tell whether that meant "never reached", "nothing to
	**	raid" or "nobody free to send" -- three quite different faults that all look
	**	like silence.
	*/
	if (sent > 0) {
		CAC_Log("ai.raid", "%s sends %d at %d,%d%s", house->Class->IniName, sent,
				Cell_X(Coord_Cell(where)), Cell_Y(Coord_Cell(where)),
				threatened ? "  (answering an attack)" : "");
	} else {
		/*
		**	A standing condition rather than an event, so said occasionally.  On the
		**	raid cadence it was seventy-six lines saying the same thing.
		*/
		static long _quiet = -99999;
		if ((long)Frame - _quiet > (long)(TICKS_PER_MINUTE * 2)) {
			_quiet = (long)Frame;
			CAC_Log_Volatile("ai.raid", "%s wanted %d for a raid and found nobody free",
								 house->Class->IniName, want);
		}
	}
}


void CAC_AI_Collect(HouseClass *house)
{
	if (house == NULL || house->IsHuman) return;
	if (house->Class == NULL || !CAC_AI_Runs_For(house)) return;

	/*
	**	Once a second, staggered, and not once a frame.
	**
	**	This ran from HouseClass::AI with no cadence at all, and one crate collected
	**	sixty-eight dispatch orders in two minutes before anybody reached it -- the
	**	errand counter above is meant to stop that and demonstrably was not, so the
	**	orders went out again every few frames to whoever was nearest.  Whether the
	**	counter is wrong or something downstream is clearing the NavCom it counts is
	**	a separate question, and the log line below now names the unit so the next
	**	round can answer it.  Either way, deciding this sixty times a second was
	**	never right.
	**
	**	Neutral is skipped outright: it owns nothing, wants a crate every pass, and
	**	half this channel was it saying so.
	*/
	if (((int)Frame + Houses.ID(house) * 7) % TICKS_PER_SECOND != 0) return;

	/*
	**	How many errands are already running, and how many this house may run.
	**
	**	This used to stop at the first one found: a single crate at a time for the
	**	whole house, however big the house was.  A log of a twenty-nine minute game
	**	had six dispatches in it, two of which arrived -- and while one unit walked
	**	the length of the map for a crate, every other crate on the board was
	**	something this house could not be sent for.
	**
	**	A small force still sends one.  A large one can afford to contest more than
	**	one, which is what a player does, and the cap keeps it from turning into a
	**	stampede.  Note this is a departure from the source game -- CrystAlien's
	**	computer does not collect at all -- and a deliberate one.
	*/
	int errands = 0;
	int force = 0;
	int i;
	for (i = 0; i < Units.Count(); i++) {
		UnitClass const *u = Units.Ptr(i);
		if (u == NULL || !u->IsActive || u->IsInLimbo || u->House != house) continue;
		force++;
		if (!Target_Legal(u->NavCom)) continue;
		CELL dest = As_Cell(u->NavCom);
		if ((unsigned)dest >= MAP_CELL_TOTAL) continue;
		if (Map[dest].Overlay != OVERLAY_NONE &&
			OverlayTypeClass::As_Reference(Map[dest].Overlay).IsCrate) errands++;
	}
	for (i = 0; i < Infantry.Count(); i++) {
		InfantryClass const *man = Infantry.Ptr(i);
		if (man == NULL || !man->IsActive || man->IsInLimbo || man->House != house) continue;
		force++;
		if (!Target_Legal(man->NavCom)) continue;
		CELL dest = As_Cell(man->NavCom);
		if ((unsigned)dest >= MAP_CELL_TOTAL) continue;
		if (Map[dest].Overlay != OVERLAY_NONE &&
			OverlayTypeClass::As_Reference(Map[dest].Overlay).IsCrate) errands++;
	}

	int allowed = 1 + (force / 12);
	if (allowed > 3) allowed = 3;
	if (errands >= allowed) return;

	/*
	**	A reindeer for preference.  They are the one thing on the roster with
	**	nothing else to be doing, and a crate wants legs rather than guns --
	**	sending a Trike costs the base a gun for as long as the walk takes.
	*/
	FootClass *idle = CAC_Free_Decoy(house, NULL);

	/*
	**	Failing that, anybody free.  A Miner is earning and a raider is on its way
	**	somewhere that matters more; anything else standing about will do.
	*/
	if (idle == NULL) {
		for (i = 0; i < Units.Count(); i++) {
			UnitClass *u = Units.Ptr(i);
			if (u == NULL || !u->IsActive || u->IsInLimbo || u->House != house) continue;
			if (u->Strength == 0 || CAC_Is_Miner(*u) || u->Flagged != HOUSE_NONE) continue;

			/*
			**	Hunting is not an errand, so a hunter may be taken off it.
			**
			**	CAC_AI_Aggression puts every idle armed unit on MISSION_HUNT every two
			**	seconds, and a hunter that has found something has a TarCom -- so
			**	refusing anything with a target meant aggression starved this routine
			**	completely.  A log of a six-player game has crates drawn on the map and
			**	not one dispatch in five minutes: the two things I added to make the
			**	computer livelier were competing for the same units, and the louder one
			**	won every time.
			**
			**	CrystAlien has the same ordering and settles it the same way: the HAL's
			**	ordinary state is "seek", a random walk, and anything with a reason to
			**	interrupt it does.  A crate is a reason.  Something already on its way
			**	somewhere deliberate is not to be taken.
			*/
			/*
			**	A patrol is not a deliberate errand, and yields to one.
			**	
			**	Third time this routine has been starved by something else I added, and
			**	the same shape every time: CAC_AI_Patrol takes half the armed force every
			**	twenty seconds and gives each unit a destination on the far side of the
			**	map, so half the army carries a NavCom permanently and the test below
			**	refused all of it.  Crates sat beside bases untouched again.
			**	
			**	The priority is errand, then raid, then patrol -- a crate is a thing that
			**	will be gone shortly and a patrol point will still be there afterwards.
			**	So a patrolling unit is fair game and anything else with somewhere to be
			**	is not.
			*/
			/*
			**	An errand outranks a fight.  Fourth time of writing this down.
			**	
			**	Aggression took the guards, so this started taking hunters.  The patrol
			**	took half the force and gave each a destination, so this learnt to take
			**	them back.  Now the raid takes five to eight at a time and puts them on
			**	MISSION_ATTACK with a target, and once more there was nobody left: a log
			**	of one session has a hundred and eight raids and not one dispatch from
			**	here, with crates sitting on the map throughout.
			**	
			**	Patching the newest thief each time is what made this recur, so the rule
			**	is stated once and positively: what disqualifies a unit is being on a
			**	deliberate errand of its own -- carrying, capturing, boarding, delivering,
			**	or already walking to a chosen place.  Fighting is not one of those.  A
			**	raider is pointed at something that will still be there in a minute; a
			**	crate will not.
			**	
			**	The cost is bounded by the caller: `allowed` is 1 + force/12, capped at
			**	three, so at worst three units leave a raid to fetch something.
			*/
			if (Target_Legal(u->NavCom) &&
				u->Mission != MISSION_GUARD_AREA &&
				u->Mission != MISSION_ATTACK &&
				u->Mission != MISSION_HUNT) continue;
			/*
			**	Not just the ones standing guard.
			**
			**	A computer's units spend most of a round hunting rather than guarding,
			**	so asking for guards meant asking for a state its army is rarely in --
			**	and a crate could sit beside its base all game with nobody eligible to
			**	go and get it.  What actually matters is already tested above: nothing
			**	in its NavCom and nothing in its TarCom, which is to say it is between
			**	jobs whatever the mission is nominally called.
			*/
			if (u->Mission == MISSION_MOVE || u->Mission == MISSION_ENTER ||
				u->Mission == MISSION_CAPTURE || u->Mission == MISSION_HARVEST ||
				u->Mission == MISSION_UNLOAD) continue;

			/*	And say when the pool was empty, so the next report is not guesswork. */
			idle = u;
			break;
		}
	}
	if (idle == NULL) {
		if (CAC_Logging()) {
			static long _when = -9999;
			if ((long)Frame - _when > (long)(TICKS_PER_SECOND * 20)) {
				_when = (long)Frame;
			CAC_Log_Volatile("ai.collect", "%s wants a crate and has nobody free (force %d)",
								 house->Class ? house->Class->IniName : "?", force);
			}
		}
		return;
	}

	/*
	**	And the nearest thing worth walking to.
	*/
	CELL best = 0;
	int bestrange = 0;
	for (CELL cell = 0; cell < MAP_CELL_TOTAL; cell++) {
		CellClass const *ptr = &Map[cell];
		if (ptr->Overlay == OVERLAY_NONE) continue;
		if (!OverlayTypeClass::As_Reference(ptr->Overlay).IsCrate) continue;

		int range = (int)::Distance(idle->Center_Coord(), ::Cell_Coord(cell));
		if (best == 0 || range < bestrange) {
			best = cell;
			bestrange = range;
		}
	}
	if (best == 0) return;

	/*
	**	Volatile: reported three times as not happening, for three different reasons
	**	-- never called, then called too seldom, then finding nobody eligible.  If a
	**	crate still sits untouched, this line says whether anyone was ever sent.
	*/
	CAC_Log_Volatile("ai.collect", "%s #%d sent for the crate at %d,%d (errand %d of %d, force %d)",
							 idle->Techno_Type_Class() ? idle->Techno_Type_Class()->IniName : "?",
							 (idle->What_Am_I() == RTTI_UNIT)
								? Units.ID((UnitClass *)idle) : Infantry.ID((InfantryClass *)idle),
							 Cell_X(best), Cell_Y(best), errands + 1, allowed, force);

	idle->Assign_Target(TARGET_NONE);
	idle->Assign_Destination(::As_Target(best));
	idle->Assign_Mission(MISSION_MOVE);
}




static int CAC_Raid_Worth(HouseClass const *house, BuildingClass const *b)
{
	if (b == NULL || b->Class == NULL) return(0);
	if (!b->Class->IsCaptureable) return(0);

	/*
	**	Nothing without a threat rating is worth crossing a map for.  Level.think
	**	picks the computer's target with
	**
	**	    while (tries < buildings.length * 2 && (!b.friend || !b.stats.threat))
	**
	**	-- a building with `threat` of zero is never chosen at all.  Every real
	**	structure has one; the Soccer Pitch is the only thing in the game that does
	**	not, and it is level decoration.
	*/
	if (CAC_Threat_Rating(RTTI_BUILDINGTYPE, (int)b->Class->Type) <= 0) return(0);

	int code = CAC_Code_Of(RTTI_BUILDINGTYPE, (int)b->Class->Type) & ~CAC_CODE_EVIL;
	/*
	**	For an Alien commander a Driller outranks even a headquarters.
	**	
	**	The Aliens cannot build one and never will -- CrystAlien has no Alien
	**	Driller, so this is not something the conversion withholds from them, it is
	**	something that does not exist.  What the source gives them instead is a
	**	reason to go and take somebody else's, and taking one is doubly worth it:
	**	it is the only building in the game that earns on its own, so capturing it
	**	moves the income across rather than merely denying it.
	**	
	**	So the Alien order is the Driller and then the headquarters, and the Astros'
	**	is the other way about -- they have Drillers of their own and have nothing
	**	particular to gain from a second-hand one.
	*/
	bool evil = (house != NULL && house->Class != NULL &&
					 CAC_Side(house->Class->House) == HOUSE_BAD);

	/*
	**	And in Pizza Mode the money *is* the game.
	**
	**	Nobody can be killed, so the only way to keep a rival from fifty thousand
	**	credits is to take what earns them.  The MT-201 Driller is that building
	**	and the only one: BuildingStats flags BL `refinery: True` and nothing
	**	else, and the conversion puts it on STRUCT_REFINERY for exactly that
	**	reason -- it is where crystal lands and it earns on its own besides.
	**
	**	The headquarters is worth nothing here rather than the three it is worth
	**	everywhere else, because in this mode it cannot be captured at all.  See
	**	CAC_Pizza_Protected.
	*/
	if (CAC_Pizza_Mode()) {
		if (code == CAC_CODE_BA) return(0);
		if (code == CAC_CODE_BL) return(8);
		if (code == CAC_CODE_BK) return(2);
		return(1);
	}

	if (code == CAC_CODE_BL) return(evil ? 4 : 2);	/* the only building that earns */
	if (code == CAC_CODE_BA || code == CAC_CODE_BK) return(3);	/* the game itself */
	return(1);
}


/***********************************************************************************************
 * CAC_Best_Raid -- The enemy building most worth walking into.                                *
 *                                                                                             *
 *    Headquarters first, because doInfiltrate on one ends the game; then a Driller, because it *
 *    is the only building that earns on its own and the Aliens can no longer build one.        *
 *    Distance decides between equals, so nobody walks past a target it could have taken.       *
 *=============================================================================================*/
/***********************************************************************************************
 * The computer's memory of where it has been.                                                  *
 *                                                                                             *
 *    Tiberian Dawn's computer has no shroud at all.  DisplayClass::Map_Cell opens with         *
 *                                                                                             *
 *        if (!house->IsHuman) { if (!ShareAllyVisibility || !and_for_allies) return false; }   *
 *                                                                                             *
 *    -- nothing is ever mapped for a computer house, because nothing needs to be: every        *
 *    routine that picks a target walks the object heaps directly and sees the whole board.     *
 *    CrystAlien is no better; `arena.shroud.reveal` is called only `if (this.friend)`.          *
 *                                                                                             *
 *    That is invisible in a campaign, where the opponent is scripted and the player knows the  *
 *    computer is cheating because the level said so.  In a skirmish against a player who must  *
 *    scout, and must pay for a Satellite Uplink to stop scouting, it is the difference between *
 *    a hard opponent and an unfair one: raid parties leave for a base nobody has been near.    *
 *                                                                                             *
 *    So the computer is given the one thing it lacked -- a record of where it has actually     *
 *    had something.  One bit per house per cell, set from the sight radius of anything it      *
 *    owns, never cleared: once seen, remembered, which is what a player's own explored map     *
 *    does.  A raid may then only be aimed at ground the house has stood on or looked at.        *
 *                                                                                             *
 *    Deliberately narrow.  It gates the *raid* target and nothing else: MISSION_HUNT still      *
 *    walks a unit at whatever is nearest, because a hunter blundering into an enemy is how a   *
 *    computer discovers anything in the first place, and CAC_AI_Patrol sweeps the map on a     *
 *    grid, which is what turns that into deliberate scouting rather than luck.                 *
 *=============================================================================================*/
static unsigned char CACScouted[MAP_CELL_TOTAL];

void CAC_Reset_Scouting(void)
{
	for (int i = 0; i < MAP_CELL_TOTAL; i++) CACScouted[i] = 0;
}


bool CAC_Has_Scouted(HouseClass const *house, CELL cell)
{
	if (house == NULL || house->Class == NULL) return(true);
	if ((unsigned)cell >= MAP_CELL_TOTAL) return(false);

	/*	A human's own shroud is the client's business and is already honest. */
	if (house->IsHuman) return(true);

	unsigned int bit = 1u << (int)house->Class->House;
	return((CACScouted[cell] & bit) != 0);
}


/***********************************************************************************************
 * CAC_AI_Scout -- Write down the ground this house has had something on.                       *
 *                                                                                             *
 *    Once a second and staggered, because it walks every object the house owns and paints a    *
 *    square around each.  Sight range is the type's own, so a Radar Station sees further than  *
 *    a trooper and an aircraft further than either -- the same numbers the shroud would have    *
 *    used had the computer ever had one.                                                        *
 *=============================================================================================*/
void CAC_AI_Scout(HouseClass *house)
{
	if (house == NULL || house->IsHuman || !house->IsActive) return;
	if (house->Class == NULL) return;

	if (((int)Frame + Houses.ID(house) * 5) % TICKS_PER_SECOND != 0) return;

	unsigned int bit = 1u << (int)house->Class->House;

	/*
	**	How much of the map it has actually been to.  "The computer knows where
	**	everything is" is a claim about coverage, and a number settles it -- a house
	**	that has seen a tenth of the board and is raiding across it is doing
	**	something this record was meant to stop.
	*/
	/*
	**	Said every half minute of this house's own passes, not on a frame number.
	**
	**	This asked for `Frame % (TICKS_PER_MINUTE/2) == 0` from inside a routine that
	**	only runs when `(Frame + id*5) % TICKS_PER_SECOND == 0`, so a house spoke only
	**	if its stagger happened to land on a multiple of both -- which for most of
	**	them is never.  Three houses out of eight reported, once, on frame nought.
	*/
	static int _pass[HOUSE_COUNT];
	static bool _passinit = false;
	if (!_passinit) {
		_passinit = true;
		for (int h = 0; h < HOUSE_COUNT; h++) _pass[h] = 0;
	}
	int hid = (int)house->Class->House;
	bool say = false;
	if ((unsigned)hid < HOUSE_COUNT) {
		if (++_pass[hid] >= 30) { _pass[hid] = 0; say = true; }
	}

	if (CAC_Logging() && say) {
		int seen = 0;
		int total_cells = 0;
		for (int y = Map.MapCellY; y < Map.MapCellY + Map.MapCellHeight; y++) {
			for (int x = Map.MapCellX; x < Map.MapCellX + Map.MapCellWidth; x++) {
				total_cells++;
				if (CACScouted[XY_Cell(x, y)] & bit) seen++;
			}
		}
		if (total_cells > 0) {
			CAC_Log_Volatile("scout", "%s has been to %d%% of the map (%d of %d cells)",
								  house->Class->IniName, (seen * 100) / total_cells,
								  seen, total_cells);
		}
	}

	for (int pass = 0; pass < 4; pass++) {
		int total = (pass == 0) ? Buildings.Count()
					 : (pass == 1) ? Units.Count()
					 : (pass == 2) ? Infantry.Count() : Aircraft.Count();

		for (int i = 0; i < total; i++) {
			TechnoClass const *o =
				  (pass == 0) ? (TechnoClass const *)Buildings.Ptr(i)
				: (pass == 1) ? (TechnoClass const *)Units.Ptr(i)
				: (pass == 2) ? (TechnoClass const *)Infantry.Ptr(i)
							  : (TechnoClass const *)Aircraft.Ptr(i);

			if (o == NULL || !o->IsActive || o->IsInLimbo) continue;
			if (o->House != house || o->Strength == 0) continue;

			TechnoTypeClass const *type = o->Techno_Type_Class();
			if (type == NULL) continue;

			int sight = type->SightRange;
			if (sight < 1) sight = 1;

			CELL here = Coord_Cell(o->Center_Coord());
			int cx = Cell_X(here);
			int cy = Cell_Y(here);

			for (int dy = -sight; dy <= sight; dy++) {
				for (int dx = -sight; dx <= sight; dx++) {
					if ((dx * dx) + (dy * dy) > (sight * sight)) continue;
					int x = cx + dx;
					int y = cy + dy;
					if (x < 0 || y < 0 || x >= MAP_CELL_W || y >= MAP_CELL_H) continue;
					CACScouted[XY_Cell(x, y)] |= bit;
				}
			}
		}
	}
}


static BuildingClass *CAC_Best_Raid(HouseClass const *house, COORDINATE from,
											TARGET avoid)
{
	if (house == NULL) return(NULL);

	BuildingClass *best = NULL;
	int bestworth = 0;
	int bestrange = 0;
	for (int j = 0; j < Buildings.Count(); j++) {
		BuildingClass *b = Buildings.Ptr(j);
		if (b == NULL || !b->IsActive || b->IsInLimbo || b->Strength == 0) continue;
		if (b->House == house || house->Is_Ally(b->House)) continue;
		if (!b->Can_Capture()) continue;
		if (CAC_Pizza_Protected(b)) continue;	/* cannot be taken -- see CAC_Pizza_Protected */

		/*
		**	And only somewhere this house has actually been.  See CAC_Has_Scouted --
		**	a raid party leaving for a base nobody has been near is the whole of
		**	"the computer is omniscient", and it is, because Tiberian Dawn never
		**	maps a cell for a computer house and every picker walks the heaps.
		*/
		if (!CAC_Has_Scouted(house, Coord_Cell(b->Center_Coord()))) continue;

		/*
		**	And not the one this raider has just given up on -- see CAC_Raid_Stalled.
		**	Without this the picker hands back the same unreachable building on the
		**	very next pass and the walk starts again from the beginning.
		*/
		if (avoid != TARGET_NONE && b->As_Target() == avoid) continue;

		int worth = CAC_Raid_Worth(house, b);
		if (worth == 0) continue;

		int range = (int)::Distance(from, b->Center_Coord());
		if (best == NULL || worth > bestworth ||
			(worth == bestworth && range < bestrange)) {
			best = b;
			bestworth = worth;
			bestrange = range;
		}
	}
	return(best);
}


/***********************************************************************************************
 * CAC_AI_Airlift -- Fly the raiders in rather than walking them.                               *
 *                                                                                             *
 *    A Saboteur walks at speed one.  Crossing a map to a headquarters on foot, past everything *
 *    defending it, is a walk it does not finish -- and the Recon Dropship and the Hyper        *
 *    Carrier are sitting on the pad with `carrier: true` and `capacity: 6`, which the          *
 *    conversion already reads into Max_Passengers.  Tiberian Dawn's computer only ever loads a *
 *    transport through a TeamType, and a skirmish has none, so they were being built and never *
 *    used for anything.                                                                        *
 *                                                                                             *
 *    Two steps, each driven by what can be seen rather than by remembered state, so it recovers *
 *    on its own if a step is interrupted:                                                      *
 *                                                                                             *
 *      * A loaded transport standing about is sent to a clear cell beside the best target and  *
 *        told to unload.  MISSION_UNLOAD on a rotor craft is already the whole sequence --     *
 *        find the zone, fly, land, put them down.                                              *
 *      * An empty one standing about, with a raider who has somewhere to be, gets the raider   *
 *        walked aboard.                                                                        *
 *                                                                                             *
 *    Once they are put down they have no orders, and the next pass of CAC_AI_Infiltrate finds  *
 *    them standing next to the thing they came for.                                            *
 *                                                                                             *
 *    One airlift at a time, on the attack cadence.                                             *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Raider_Rank -- Who goes in the transport, and in what order.                             *
 *                                                                                             *
 *    Nought is the engineer, because a raid without one is a bus trip.  One is the Alien       *
 *    Commander and Santa, the two carriable types worth a turret apiece.  Two is everything    *
 *    else that can be carried and can shoot -- troopers and reindeer.  Minus one is anything   *
 *    a carrier will not take, which is read off `carriable` in UnitStats rather than guessed.  *
 *                                                                                             *
 *    A transport used to be loaded with engineers and nothing else, so a Saboteur was flown    *
 *    into an enemy base alone and met whatever was standing there.  CAC_AI_Escort already      *
 *    walks a couple of reindeer in behind a raid on foot; this is the same idea for the ones   *
 *    that arrive by air, except that here the escort travels with the engineer rather than     *
 *    trying to keep up with it.                                                                *
 *=============================================================================================*/
static int CAC_Raider_Rank(InfantryClass const *man)
{
	if (man == NULL || man->Class == NULL) return(-1);
	if (!CAC_Is_Carriable(man)) return(-1);

	if (man->Class->IsCapture) return(0);

	int code = CAC_Code_Of(RTTI_INFANTRYTYPE, (int)*man) & ~CAC_CODE_EVIL;
	if (code == CAC_CODE_UQ || code == CAC_CODE_UM) return(1);

	if (!man->Is_Weapon_Equipped()) return(-1);
	return(2);
}


/***********************************************************************************************
 * CAC_Raid_Ready -- Is this transport loaded enough to be worth flying?                        *
 *                                                                                             *
 *    It used to leave the moment anybody was aboard, which with a one-at-a-time loader meant   *
 *    it left with one.  A Hyper Carrier holds six.                                             *
 *                                                                                             *
 *    Full, or an engineer aboard and the hold no longer filling -- there is nothing left to    *
 *    put in it, so waiting for a seventh trooper that does not exist would ground the raid     *
 *    permanently.  Either way it does not take off empty-handed of the one type that makes     *
 *    the trip worth making.                                                                    *
 *=============================================================================================*/
static bool CAC_Raid_Ready(AircraftClass const *ride, bool morecoming)
{
	if (ride == NULL || ride->Class == NULL) return(false);
	if (!ride->Is_Something_Attached()) return(false);

	if (ride->How_Many() >= ride->Class->Max_Passengers()) return(true);
	return(!morecoming);
}


void CAC_AI_Airlift(HouseClass *house)
{
	if (house == NULL || house->IsHuman) return;

	/*
	**	Same cadence and for the same reason -- see CAC_AI_Infiltrate.  This loads
	**	one passenger per pass by design, so on the attack timer a Hyper Carrier
	**	took six rolls of Random_Pick(TICKS_PER_MINUTE/2, TICKS_PER_MINUTE*2) to
	**	fill: somewhere between three minutes and a quarter of an hour, for a raid
	**	that was supposed to be the fast way in.
	*/
	if (((int)Frame + Houses.ID(house) * 11) % TICKS_PER_SECOND != 0) return;

	int i;

	/*
	**	Who is still walking towards a transport.  Counted first, because whether a
	**	half-full one waits or goes depends on whether anything else is coming.
	*/
	int boarding = 0;
	for (i = 0; i < Infantry.Count(); i++) {
		InfantryClass const *man = Infantry.Ptr(i);
		if (man == NULL || !man->IsActive || man->IsInLimbo || man->House != house) continue;
		if (man->Mission == MISSION_ENTER) boarding++;
	}

	/*
	**	A transport with a worthwhile load, and nowhere it has been told to go.
	*/
	for (i = 0; i < Aircraft.Count(); i++) {
		AircraftClass *a = Aircraft.Ptr(i);
		if (a == NULL || !a->IsActive || a->IsInLimbo || a->House != house) continue;
		if (a->Strength == 0 || a->Class == NULL || !a->Class->IsTransporter) continue;

		/*
		**	These are questions about *this* transport, so they move on to the next
		**	one rather than ending the routine.  Written as `return`, a house whose
		**	first transport happened to be mid-unload never gave orders to its second,
		**	and which one was first is just the order the heap is in.
		*/
		if (a->Mission == MISSION_UNLOAD || Target_Legal(a->NavCom)) continue;
		if (!CAC_Raid_Ready(a, boarding > 0)) continue;

		BuildingClass *best = CAC_Best_Raid(house, a->Center_Coord(), TARGET_NONE);
		if (best == NULL) return;		/* nothing worth raiding anywhere -- that is a house-wide answer */

		/*
		**	Somewhere clear to put down beside it; the landing zone has to be free
		**	or Mission_Unload goes looking for one of its own.
		*/
		CELL lz = Map.Nearby_Location(Coord_Cell(best->Center_Coord()));
		if (lz == 0) continue;

		a->Assign_Target(TARGET_NONE);
		a->Assign_Destination(::As_Target(lz));
		a->Assign_Mission(MISSION_UNLOAD);

		CAC_Log("airlift", "%s: %s flying %d raider(s) to %s at %d,%d",
				house->Class->IniName, a->Class->IniName, a->How_Many(),
				best->Class->IniName, Cell_X(lz), Cell_Y(lz));
		return;					/* one airlift at a time, as documented above */
	}

	/*
	**	Otherwise an empty or half-full one, and somebody to put in it.
	*/
	AircraftClass *ride = NULL;
	for (i = 0; i < Aircraft.Count(); i++) {
		AircraftClass *a = Aircraft.Ptr(i);
		if (a == NULL || !a->IsActive || a->IsInLimbo || a->House != house) continue;
		if (a->Strength == 0 || a->Class == NULL || !a->Class->IsTransporter) continue;
		if (Target_Legal(a->NavCom)) continue;
		if (a->Mission != MISSION_GUARD && a->Mission != MISSION_GUARD_AREA) continue;

		/*	Room for one more, counting whoever is already on the way to it. */
		if (a->How_Many() + boarding >= a->Class->Max_Passengers()) continue;
		ride = a;
		break;
	}
	if (ride == NULL) return;
	if (CAC_Best_Raid(house, ride->Center_Coord(), TARGET_NONE) == NULL) return;

	/*
	**	Best first, so the engineer is aboard before the escort is considered and
	**	a half-loaded transport is never the wrong half.
	*/
	for (int rank = 0; rank < 3; rank++) {
		for (i = 0; i < Infantry.Count(); i++) {
			InfantryClass *man = Infantry.Ptr(i);
			if (man == NULL || !man->IsActive || man->IsInLimbo || man->House != house) continue;
			if (man->Strength == 0 || man->Class == NULL) continue;
			if (man->Mission == MISSION_ENTER || man->Mission == MISSION_CAPTURE) continue;
			if (Target_Legal(man->NavCom)) continue;
			if (CAC_Guarding_Flag(man, house)) continue;	/* the flag detail is not a raiding pool */
			if (CAC_Raider_Rank(man) != rank) continue;

			/*
			**	An escort is only worth sending with an engineer.  A transport full
			**	of reindeer arriving at a Defence Station is a delivery of reindeer.
			*/
			if (rank > 0 && !ride->Is_Something_Attached()) continue;

			/*	The errand first, so the destination is taken as boarding (CAC_Try_Embark). */
			man->Assign_Target(TARGET_NONE);
			man->Assign_Mission(MISSION_ENTER);
			man->ArchiveTarget = ride->As_Target();
			man->Assign_Destination(ride->As_Target());

			CAC_Log("airlift", "%s: %s boarding %s (%d aboard of %d)",
					house->Class->IniName, man->Class->IniName, ride->Class->IniName,
					ride->How_Many(), ride->Class->Max_Passengers());
			return;
		}
	}
}


/***********************************************************************************************
 * CAC_AI_Infiltrate -- Send the computer's Engineers and Saboteurs at something worth taking.  *
 *                                                                                             *
 *    HouseClass::AI_Attack sends everything armed on MISSION_HUNT and makes one exception for  *
 *    an unarmed man: `*i == INFANTRY_RENOVATOR`.  That is Red Alert's engineer, and no          *
 *    CrystAlien type is it, so the Engineer and the Saboteur were given no orders at all and   *
 *    stood in the base for the whole game.                                                     *
 *                                                                                             *
 *    What they should be doing is in doInfiltrate:                                             *
 *                                                                                             *
 *        if(this.target.friend && this.target.type == "BA_" + team) { this.parent.lose(); }     *
 *        this.target.team = this.team;                                                         *
 *        ...                                                                                   *
 *        this.parent.construction.doPrerequisites(this);                                       *
 *                                                                                             *
 *    -- taking a headquarters ends the game for whoever lost it, and taking anything else      *
 *    hands over that building's half of the build tree.  So a raid is not a nuisance in this    *
 *    game, it is how a side wins, and the levels that play side against side lean on it.        *
 *                                                                                             *
 *    The order of preference follows from that:                                                *
 *                                                                                             *
 *      1. A headquarters -- Command Base, Ops Ship, Mothership, Hive.  Taking one is the game.  *
 *      2. A Driller.  It earns on its own, it is the only building that does, and since the     *
 *         Aliens can no longer build one this is how they get theirs.                           *
 *      3. Anything else, nearest first.                                                         *
 *                                                                                             *
 *    Distance decides between equals, so a raider does not walk across the map past a target    *
 *    it could have taken.                                                                       *
 *=============================================================================================*/

/***********************************************************************************************
 * CAC_Unscouted_Point -- Somewhere this house has not been, worth walking to.                  *
 *                                                                                             *
 *    The nine points CAC_AI_Patrol sweeps, filtered to the ones this house has never had       *
 *    anything near and can put a foot on, nearest first.                                       *
 *=============================================================================================*/
static CELL CAC_Unscouted_Point(HouseClass const *house, COORDINATE from)
{
	CELL best = 0;
	int bestrange = 0;

	for (int slot = 0; slot < 9; slot++) {
		int gx = Map.MapCellX + (Map.MapCellWidth  * (1 + 2 * (slot % 3))) / 6;
		int gy = Map.MapCellY + (Map.MapCellHeight * (1 + 2 * (slot / 3))) / 6;

		CELL cell = XY_Cell(gx, gy);
		if (!Map.In_Radar(cell)) continue;
		if (CAC_Has_Scouted(house, cell)) continue;
		if (Ground[Map[cell].Land_Type()].Cost[SPEED_FOOT] == 0) continue;

		int range = (int)::Distance(from, ::Cell_Coord(cell));

		/*
		**	Far enough away to be a journey.
		**
		**	This took the nearest unscouted cell without asking how near, and a unit
		**	standing on one is nought away from it -- so it was ordered to walk to
		**	the square it was already on, arrived instantly, was asked again the next
		**	second and ordered there again.  The log of the match that found this has
		**	"Multi5 sends UBGOOD to look at 11,52" against a roll call reading "UBGOOD
		**	at 11,52 Guard", four thousand times: the engineers standing around were
		**	not idle, they were busy arriving where they were.
		**
		**	A cell the seeker is on teaches it nothing anyway.  Two cells is the least
		**	that counts as going to look at something.
		*/
		if (range < 0x0200) continue;

		if (best == 0 || range < bestrange) { best = cell; bestrange = range; }
	}
	return(best);
}


/***********************************************************************************************
 * CAC_Raid_Worthwhile -- Is what this raider is walking to still worth arriving at?            *
 *                                                                                             *
 *    Target_Legal is not the question.  A building captured by this house a moment before the  *
 *    Saboteur reached it is perfectly legal and perfectly pointless: MISSION_CAPTURE on ground  *
 *    the house already owns does nothing at all, so the man walks up to his own building and    *
 *    stands there with a legal NavCom and a mission that will never complete.  An ally's is     *
 *    the same.  So is one that has stopped being capturable.                                    *
 *                                                                                             *
 *    That is one half of "engineers sit until killed".                                          *
 *=============================================================================================*/
static bool CAC_Raid_Worthwhile(HouseClass const *house, TARGET nav)
{
	if (house == NULL || !Target_Legal(nav)) return(false);

	BuildingClass *b = As_Building(nav);
	if (b == NULL || !b->IsActive || b->IsInLimbo || b->Strength == 0) return(false);
	if (b->House == house || house->Is_Ally(b->House)) return(false);
	if (!b->Can_Capture()) return(false);

	return(true);
}


/***********************************************************************************************
 * CAC_Raid_Stalled -- Has this raider stopped getting closer to what it was sent at?           *
 *                                                                                             *
 *    The other half.  A target across water, behind a wall, or on ground the pathfinder cannot  *
 *    reach leaves NavCom legal and the building alive, so every test above passes and the man   *
 *    walks into the same obstruction for the rest of the round.  Nothing in the engine notices: *
 *    a foot soldier with an unreachable destination is not an error state, it is just a foot    *
 *    soldier that never arrives.                                                                *
 *                                                                                             *
 *    So measure it.  Each pass, remember how far this man was from his target; if the distance  *
 *    has not improved over several consecutive passes he is not going to get there, and the     *
 *    target is handed back so a different one can be chosen.  Distance rather than position,    *
 *    because a raider walking a long way round a cliff is making progress and should be left    *
 *    alone, and the measure has to tell that apart from one pushing into a wall.                *
 *                                                                                             *
 *    Kept per man rather than per house: two Saboteurs sent at the same building can easily be  *
 *    one stuck and one walking.                                                                 *
 *=============================================================================================*/
static bool CAC_Raid_Stalled(InfantryClass const *man, TARGET nav)
{
	static TARGET        _nav[INFANTRY_MAX];
	static unsigned int  _dist[INFANTRY_MAX];
	static unsigned char _strikes[INFANTRY_MAX];
	static bool          _init = false;

	if (!_init) {
		_init = true;
		for (int i = 0; i < INFANTRY_MAX; i++) {
			_nav[i] = TARGET_NONE;
			_dist[i] = 0;
			_strikes[i] = 0;
		}
	}

	int id = Infantry.ID((InfantryClass *)man);
	if (id < 0 || id >= INFANTRY_MAX) return(false);

	if (!Target_Legal(nav)) {
		_nav[id] = TARGET_NONE;
		_strikes[id] = 0;
		return(false);
	}

	unsigned int range = (unsigned int)::Distance(man->Center_Coord(), ::As_Coord(nav));

	/*	A new target starts the count again. */
	if (_nav[id] != nav) {
		_nav[id] = nav;
		_dist[id] = range;
		_strikes[id] = 0;
		return(false);
	}

	if (range + CAC_RAID_PROGRESS < _dist[id]) {
		_dist[id] = range;
		_strikes[id] = 0;
		return(false);
	}

	if (_strikes[id] < 255) _strikes[id]++;
	if (_strikes[id] < CAC_RAID_PATIENCE) return(false);

	_nav[id] = TARGET_NONE;
	_strikes[id] = 0;
	return(true);
}


/***********************************************************************************************
 * CAC_Rally_Cell -- Somewhere to stand that is not the middle of a field.                      *
 *                                                                                             *
 *    The last resort for a raider with nothing to raid and nothing left to look at.  With the  *
 *    map scouted CAC_Unscouted_Point answers nothing, and an engineer is excluded from every    *
 *    other routine there is -- aggression and the patrol both skip IsCapture on purpose -- so   *
 *    it stood exactly where it was built until something shot it.                               *
 *                                                                                             *
 *    Sending it home is the useful answer: it is out of the open, it is somewhere a transport   *
 *    can pick it up from, and it is standing on the one thing worth repairing or recapturing.   *
 *=============================================================================================*/
static CELL CAC_Rally_Cell(HouseClass const *house, COORDINATE from)
{
	if (house == NULL) return(0);

	BuildingClass *best = NULL;
	int bestrange = 0;
	for (int i = 0; i < Buildings.Count(); i++) {
		BuildingClass *b = Buildings.Ptr(i);
		if (b == NULL || !b->IsActive || b->IsInLimbo || b->Strength == 0) continue;
		if (b->House != house) continue;

		int range = (int)::Distance(from, b->Center_Coord());
		if (best == NULL || range < bestrange) {
			best = b;
			bestrange = range;
		}
	}
	if (best == NULL) return(0);

	CELL site = CAC_Exit_Cell(best, NULL);
	return(site ? site : Coord_Cell(best->Center_Coord()));
}


void CAC_AI_Infiltrate(HouseClass *house)
{
	if (house == NULL || house->IsHuman) return;

	/*
	**	Once a second, staggered, and not on the attack timer.
	**
	**	This used to be called from the tail of HouseClass::AI_Attack, which sets
	**	`Attack = Rule.AttackInterval * Random_Pick(TICKS_PER_MINUTE/2,
	**	TICKS_PER_MINUTE*2)` on its way out -- so every decision about every
	**	engineer in the house was taken once, somewhere between thirty seconds and
	**	two minutes apart.
	**
	**	That is the whole of "engineers get stranded".  FootClass::Mission_Capture
	**	calls Enter_Idle_Mode the moment its NavCom stops being legal, so a raider
	**	whose building died under him is put on guard duty immediately and then
	**	stands there until this routine next happens to run -- which is a minute of
	**	a man walking nowhere, every time, and several minutes if he is unlucky
	**	with the roll.  No test here could have caught it, because the tests were
	**	not being asked.
	**
	**	A second is cheap: the loop is over this house's infantry, and only the
	**	handful that capture get as far as looking for a target.
	*/
	if (((int)Frame + Houses.ID(house) * 7) % TICKS_PER_SECOND != 0) return;

	for (int i = 0; i < Infantry.Count(); i++) {
		InfantryClass *man = Infantry.Ptr(i);
		if (man == NULL || !man->IsActive || man->IsInLimbo) continue;
		if (man->House != house || man->Strength == 0) continue;
		if (man->Class == NULL || !man->Class->IsCapture) continue;

		/*
		**	Already on its way somewhere.
		*/
		/*
		**	Already on its way in -- unless what it was walking to has gone.
		**	
		**	A raider whose building died or changed hands mid-walk keeps the mission and
		**	loses the destination, and this routine would then refuse it for the rest of
		**	the round on the strength of that mission alone.  One Saboteur standing in a
		**	field forever is cheap to prevent and impossible to notice.
		*/
		/*
		**	Not one on his way to a transport, or sitting in one -- see
		**	CAC_AI_Airlift.  Being flown there beats walking, and a man walking at
		**	a moving aircraft is not stalled just because the aircraft moved.
		*/
		if (man->Mission == MISSION_ENTER || man->IsInLimbo) continue;

		TARGET giveup = TARGET_NONE;

		/*
		**	Whatever he is already walking to -- a building to take, a corner to
		**	look at, or his way home -- he is left alone only while he is still
		**	getting closer to it.
		**
		**	Target_Legal on its own was not enough, and between them these are the
		**	report that these men stand about until they are shot.  A building
		**	captured or destroyed out from under a raider leaves a destination that
		**	is legal and pointless; one that cannot be reached leaves everything
		**	legal and the man pushing into a wall; and the scouting and rally walks
		**	below can strand him exactly the same way, which the old test did not
		**	cover at all because it only looked at capture missions.
		**
		**	Whatever the reason, the destination is cleared rather than merely
		**	stepped over -- the fallback further down refuses a man with a legal
		**	NavCom, so leaving it set is how one of these gets skipped forever.
		*/
		if (Target_Legal(man->NavCom)) {
			bool taking = (man->Mission == MISSION_CAPTURE ||
								man->Mission == MISSION_SABOTAGE);

			if (taking && !CAC_Raid_Worthwhile(house, man->NavCom)) {
				man->Assign_Destination(TARGET_NONE);
			} else if (!CAC_Raid_Stalled(man, man->NavCom)) {
				continue;
			} else {
				if (taking) {
					giveup = man->NavCom;
					BuildingClass const *lost = As_Building(giveup);
					CAC_Log_Volatile("raid", "%s: %s gives up on %s -- no closer in %d passes",
											 house->Class->IniName, man->Class->IniName,
											 (lost && lost->Class) ? lost->Class->IniName : "its target",
											 (int)CAC_RAID_PATIENCE);
				} else {
					CAC_Log_Volatile("raid", "%s: %s stopped getting anywhere -- given something else",
											 house->Class->IniName, man->Class->IniName);
				}
				man->Assign_Destination(TARGET_NONE);
			}
		}

		BuildingClass *best = CAC_Best_Raid(house, man->Center_Coord(), giveup);
		if (best == NULL) {

			/*
			**	Nothing known to raid, so go and find something.
			**
			**	This was a bare `continue`, which was harmless while the computer
			**	could see the whole board: CAC_Best_Raid always found a target, so an
			**	engineer always had somewhere to be.  Giving the computer a memory
			**	took that away -- with nothing scouted there is nothing to raid, and an
			**	engineer is excluded from every other routine there is.  Aggression
			**	skips IsCapture deliberately and wants a weapon besides; the patrol
			**	skips it for the same reason; the airlift needs the same target this
			**	does.  So they were built and then stood still for the rest of the
			**	round, which is exactly the report.
			**
			**	Walking them at unscouted ground is the useful answer rather than the
			**	tidy one: it is how the house learns where anything is, and an engineer
			**	that finds a base has found its own next job.
			*/
			if (Target_Legal(man->NavCom)) continue;

			CELL look = CAC_Unscouted_Point(house, man->Center_Coord());

			/*	And somewhere it already is, is nowhere to send it. */
			if (look == Coord_Cell(man->Center_Coord())) look = 0;

			if (look == 0) {

				/*
				**	Nothing to raid and nothing left to look at -- a scouted map with
				**	no capturable building on it.  Standing still is what these men
				**	were doing; go home instead.  See CAC_Rally_Cell.
				*/
				CELL home = CAC_Rally_Cell(house, man->Center_Coord());
				if (home == 0 || Coord_Cell(man->Center_Coord()) == home) {
					if (man->Mission != MISSION_GUARD) man->Assign_Mission(MISSION_GUARD);
					continue;
				}

				man->Assign_Target(TARGET_NONE);
				man->Assign_Destination(::As_Target(home));
				man->Assign_Mission(MISSION_MOVE);

				CAC_Log_Volatile("raid", "%s: %s has nothing to raid or scout -- sent home to %d,%d",
										 house->Class->IniName, man->Class->IniName,
										 Cell_X(home), Cell_Y(home));
				continue;
			}

			man->Assign_Target(TARGET_NONE);
			man->Assign_Destination(::As_Target(look));
			man->Assign_Mission(MISSION_MOVE);

			CAC_Log_Volatile("scout", "%s sends %s to look at %d,%d -- nothing known to raid",
								  house->Class->IniName, man->Class->IniName,
								  Cell_X(look), Cell_Y(look));
			continue;
		}

		man->Assign_Target(TARGET_NONE);
		man->Assign_Destination(best->As_Target());
		man->Assign_Mission(MISSION_CAPTURE);

		/*
		**	And a couple of reindeer in with them -- see CAC_AI_Escort.  A raider
		**	walking up to a defended building alone is a raider that does not arrive.
		*/
		CAC_AI_Escort(house, best, 2);
	}
}


/***********************************************************************************************
 * CAC_AI_Power_Urgency -- How badly does this computer base need another plant?               *
 *                                                                                             *
 *    The stock question is `Power <= Drain + Rule.PowerSurplus`, asked of HouseClass::Power    *
 *    and ::Drain.  Those are still maintained, but they are no longer what decides whether a   *
 *    turret shoots: CAC_Power_State does, and it counts differently -- it adds the fifty-point *
 *    allowance every CrystAlien base starts with, and it counts a structure from the moment it *
 *    is placed rather than from the moment it finishes.                                        *
 *                                                                                             *
 *    So the computer was optimising a number that had stopped meaning anything.  It built      *
 *    Defence Stations at seventy-five drain apiece against Power Pumps worth two hundred, and  *
 *    read a power figure that said it was fine while its guns stood dark.                      *
 *                                                                                             *
 *    Asked of the real model instead:                                                          *
 *                                                                                             *
 *      OFF   -- the guns have actually stopped.  Ranked above defence and above a second        *
 *               refinery, but below a base with no Driller at all: a base that cannot earn     *
 *               cannot buy its way out of a brownout either, so income comes first.            *
 *      LOW   -- still firing, but the margin is gone.  Ranked level with defence, and the       *
 *               plant is offered first, so a tie goes to the supply rather than the gun.        *
 *      FULL  -- there is headroom.  Keep a plant on the list at the low urgency the stock code  *
 *               used, so the base still grows its supply ahead of demand.                      *
 *                                                                                             *
 *    CrystAlien has no computer opponent to copy here -- it is a campaign, and its levels are  *
 *    scripted.  The thresholds are this conversion's, the model they read is the source's.     *
 *=============================================================================================*/
UrgencyType CAC_AI_Power_Urgency(HouseClass const *house)
{
	if (!CAC_Is_CrystAlien_House(house)) return(URGENCY_NONE);

	switch (CAC_Power_State(house)) {
		case CAC_POWER_OFF:	return(URGENCY_HIGH);
		case CAC_POWER_LOW:	return(URGENCY_MEDIUM);
		default:				return(URGENCY_LOW);
	}
}


/***********************************************************************************************
 * CAC_AI_Can_Power -- Would this structure still leave the base able to run its guns?         *
 *                                                                                             *
 *    Asked before a Defence Station goes on the build list.  A base that keeps adding turrets  *
 *    it cannot supply ends up with a row of ornaments, which is exactly what was happening --  *
 *    the defence entry never looked at power at all, and at the same urgency as the plant it   *
 *    tied with it every time.                                                                  *
 *                                                                                             *
 *    A structure that generates rather than drains is always allowed: that is the way out.     *
 *=============================================================================================*/
bool CAC_AI_Can_Power(HouseClass const *house, BuildingTypeClass const *type)
{
	if (!CAC_Is_CrystAlien_House(house) || type == NULL) return(true);
	if (type->Drain <= 0) return(true);
	return(CAC_Power_Net(house) - type->Drain >= 0);
}


/***********************************************************************************************
 * CAC_AI_Infantry_Value -- What a computer base is willing to pay attention to on foot.       *
 *                                                                                             *
 *    HouseClass::AI_Infantry weighs its choices with a switch over INFANTRY_E1 to E7 and a     *
 *    `default: Value = 0` underneath.  Every CrystAlien type falls into that default, the      *
 *    weights all come out zero, `count` never rises above nothing, and the computer built no   *
 *    infantry whatsoever -- not one Drone, not one Explorer, in any base, all game.  The       *
 *    vehicle list has no such switch, which is why tanks appeared and troops did not.          *
 *                                                                                             *
 *    The weights below are the same shape as the ones they stand in for:                       *
 *                                                                                             *
 *      * A soldier who shoots is the staple, and is worth what E1 was worth.                   *
 *      * The Engineer and the Saboteur follow the E7 rule exactly -- wanted only once the      *
 *        base has troops to spare, and only ever one at a time.                                *
 *      * Anything capped -- the Commander, and the Sleigh's cast -- is worth having once and   *
 *        is not worth crowding the queue for.                                                  *
 *                                                                                             *
 *    CrystAlien's own levels script their reinforcements rather than choosing them, so there   *
 *    is nothing in the source to copy; what is copied is Tiberian Dawn's own reasoning,        *
 *    pointed at this roster.                                                                   *
 *=============================================================================================*/
int CAC_AI_Infantry_Value(HouseClass const *house, int index)
{
	if (house == NULL) return(-1);

	InfantryTypeClass const *type = &InfantryTypeClass::As_Reference((InfantryType)index);
	if (!CAC_Is_CrystAlien_Type(type->IniName)) return(-1);	/* not ours -- leave the switch to it */

	if (CAC_At_Type_Cap(house, RTTI_INFANTRYTYPE, index)) return(0);

	/*
	**	The Pizza Delivery, which is the game rather than a unit.
	**
	**	Nought until the house can actually pay for it, so the ordinary build
	**	priority is not disturbed on the way there -- a computer saving for fifty
	**	thousand by refusing to train anything is a computer that loses the base
	**	it was saving from.  It banks by playing normally; the stipend and its
	**	miners do the rest.
	**
	**	The moment it can afford one, nothing else is worth building.  And only
	**	one at a time: CAC_At_Type_Cap already refuses a second while the first
	**	is alive, but the delivery deletes itself on arrival, so the pizza already
	**	on the map is what has to be checked.
	*/
	if (CAC_Pizza_Mode() && strncmp(type->IniName, "UJ", 2) == 0) {
		if (house->Available_Money() < type->Cost_Of()) return(0);
		if (CAC_Pizza_Cell(house, false) != 0) return(0);
		return(100);
	}

	/*
	**	The Engineer and the Saboteur are not the afterthought E7 was.
	**
	**	Tiberian Dawn wants one, and only once the base has five other men, because
	**	an engineer there repairs a building and occasionally steals one.  Here a
	**	raid is how the game is won -- doInfiltrate on a headquarters ends it -- so
	**	a pair of them is worth having from early on, and the side with no Driller
	**	of its own wants them more, because taking one is how it gets one.
	*/
	if (type->IsCapture) {
		int have = CAC_Owned_Count(house, RTTI_INFANTRYTYPE, index);
		if (have >= 2) return(0);
		bool evil = ((CAC_Code_Of(RTTI_INFANTRYTYPE, index) & CAC_CODE_EVIL) != 0);
		return(evil ? 6 : 4);
	}

	/*
	**	A capped type is a one-off, but a good one: the Commander is the Aliens'
	**	answer to the Driller and there is only ever one, so it is worth reaching
	**	for rather than waiting for the queue to get round to it.
	*/
	if (CAC_Type_Cap(RTTI_INFANTRYTYPE, index) > 0) {
		return(type->Primary != WEAPON_NONE ? 5 : 1);
	}

	/*
	**	An unarmed soldier who cannot capture is a reindeer, and a reindeer is
	**	worth having a few of -- they walk in ahead of a raid and they fetch what
	**	is lying about.  Not worth as much as somebody who shoots.
	*/
	if (type->Primary == WEAPON_NONE && !type->IsCapture) return(2);

	return(type->Primary != WEAPON_NONE ? 3 : 1);
}


/***********************************************************************************************
 * CAC_AI_Aircraft -- What a computer base should have in the air, if anything.                *
 *                                                                                             *
 *    HouseClass::AI_Aircraft names AIRCRAFT_HELICOPTER and AIRCRAFT_ORCA outright and counts   *
 *    BQuantity[STRUCT_AIRSTRIP].  A CrystAlien house owns none of the three, so the routine     *
 *    fell straight through and no computer player ever fielded an Astro Fighter, a Viper, a    *
 *    Dropship or a Hyper Carrier.                                                              *
 *                                                                                             *
 *    The rule it was applying is worth keeping: build up to one aircraft per building that can *
 *    host one, and alternate between the types rather than massing a single kind.  So that is  *
 *    what this does, over whatever the side's roster actually offers.                          *
 *                                                                                             *
 * OUTPUT:  the type to build, or AIRCRAFT_NONE.                                                *
 *=============================================================================================*/
AircraftType CAC_AI_Aircraft(HouseClass const *house)
{
	if (!CAC_Is_CrystAlien_House(house)) return(AIRCRAFT_NONE);

	/*
	**	Somewhere to build them from, and how many are already up.
	**
	**	The stock rule is one aircraft per airstrip, because a Tiberian Dawn
	**	airstrip holds one plane and the next has nowhere to sit.  CrystAlien's
	**	fighters do not dock anywhere -- the Astro Fighter and the Switch Fighter
	**	land where they stop -- so one Sonar Station meant one aircraft for the
	**	whole game, which is why the computer barely seemed to use them.
	**
	**	Four to a station instead, and each type still bounded by its own `max`
	**	from UnitStats -- five Vipers, four Strike Fighters, two Hyper Carriers.
	*/
	int pads = 0;
	for (int i = 0; i < Buildings.Count(); i++) {
		BuildingClass const *b = Buildings.Ptr(i);
		if (b == NULL || !b->IsActive || b->IsInLimbo || b->House != house) continue;
		if (b->Class != NULL && b->Class->ToBuild == RTTI_AIRCRAFTTYPE &&
			 CAC_Is_CrystAlien_Type(b->Class->IniName)) pads++;	/* its own pads; stock ones are the stock rule's */
	}
	if (pads == 0) return(AIRCRAFT_NONE);

	/*
	**	Everything this house has committed to, not everything it has in the air.
	**	
	**	These two counts have to be taken on the same population as the per-type
	**	count below, and they were not: the cap skipped anything still in limbo
	**	while CAC_Owned_Count included it.  A base with a Sonar Station and four
	**	aircraft in production therefore read as nought flying, kept queueing, and
	**	flipped between the two types it was least of as each queue changed the
	**	count -- about eight times a second, five computer opponents at once.  The
	**	log said "1 station(s), 0 up ... 2 flying", which is the same house
	**	counted two ways in one line.
	*/
	int owned = 0;
	int flying = 0;
	for (int i = 0; i < Aircraft.Count(); i++) {
		AircraftClass const *a = Aircraft.Ptr(i);
		if (a == NULL || !a->IsActive || a->House != house) continue;
		if (a->Class == NULL || !CAC_Is_CrystAlien_Type(a->Class->IniName)) continue;
		owned++;
		if (!a->IsInLimbo) flying++;
	}
	if (owned >= pads * 4) return(AIRCRAFT_NONE);

	/*
	**	And every type is still bounded by its own `max`, so a mixed force is a
	**	mixed force rather than four of whichever came first.
	*/

	/*
	**	The one there is least of, so a base ends up with a mix.
	**
	**	CrystAlien's own aircraft only, in both builds.  The supplemental once put
	**	both rosters in this list, which made it answer for stock helipads and
	**	Chinooks too and kept AI_Aircraft's own one-per-pad rule from ever being
	**	asked; now that rule picks the stock aircraft and this picks CrystAlien's.
	*/
	AircraftType best = AIRCRAFT_NONE;
	int fewest = 0;
	for (AircraftType index = AIRCRAFT_FIRST; index < AIRCRAFT_COUNT; index++) {
		AircraftTypeClass const *type = &AircraftTypeClass::As_Reference(index);
		if (!CAC_Is_CrystAlien_Type(type->IniName)) continue;
		if (!house->Can_Build(type, house->ActLike)) continue;
		if (CAC_At_Type_Cap(house, RTTI_AIRCRAFTTYPE, (int)index)) continue;

		int have = CAC_Owned_Count(house, RTTI_AIRCRAFTTYPE, (int)index);
		if (best == AIRCRAFT_NONE || have < fewest) {
			best = index;
			fewest = have;
		}
	}

	/*
	**	And once an answer is given, it is given for a few seconds.
	**	
	**	Fixing the counts stops the flip-flop between two types, but the shape of
	**	the routine still invites it: it re-derives the whole decision from the
	**	map every time it is asked, so any quantity that moves while a factory is
	**	starting up can change the answer before the last one was acted on.
	**	Holding it is cheaper than making every input stable, and a computer that
	**	changes its mind about what to build eight times a second builds nothing.
	**	
	**	Indexed by house -- fifth time this has been the mistake -- and dropped as
	**	soon as the held answer stops being buildable, so losing the building that
	**	made it possible is noticed at once rather than five seconds later.
	*/
	{
		static int  _held[HOUSE_COUNT];
		static long _heldat[HOUSE_COUNT];
		static bool _heldinit = false;
		if (!_heldinit) {
			_heldinit = true;
			for (int h = 0; h < HOUSE_COUNT; h++) { _held[h] = (int)AIRCRAFT_NONE; _heldat[h] = -9999; }
		}

		int hh = (int)house->Class->House;
		if ((unsigned)hh < HOUSE_COUNT) {
			AircraftType hold = (AircraftType)_held[hh];
			if (hold != AIRCRAFT_NONE &&
				(long)Frame - _heldat[hh] < (long)(TICKS_PER_SECOND * 5) &&
				house->Can_Build(&AircraftTypeClass::As_Reference(hold), house->ActLike) &&
				!CAC_At_Type_Cap(house, RTTI_AIRCRAFTTYPE, (int)hold)) {
				best = hold;
			} else {
				_held[hh] = (int)best;
				_heldat[hh] = (long)Frame;
			}
		}
	}

	/*
	**	Volatile: reported as the computer rarely flying anything.  Every gate on the
	**	way here is a plausible culprit and none of them announces itself -- the IQ
	**	threshold and the aircraft cap are both checked by the caller, and the cap is
	**	averaged from what the *other* houses are allowed, so it can be low for
	**	reasons that have nothing to do with this house.  Logged when the answer
	**	changes, with the numbers that produced it.
	*/
	if (CAC_Logging()) {
		static int _said[HOUSE_COUNT];
		static bool _init = false;
		if (!_init) { _init = true; for (int h = 0; h < HOUSE_COUNT; h++) _said[h] = -99; }
		/*
		**	And not more than once every few seconds per house.  The answer genuinely
		**	alternates -- one session had it swapping about eight times a second --
		**	so "only when it changes" is not a limit at all.  With five computer
		**	opponents it is five of those at once.
		*/
		static long _when[HOUSE_COUNT];
		static bool _whenset = false;
		if (!_whenset) { _whenset = true; for (int h = 0; h < HOUSE_COUNT; h++) _when[h] = -9999; }

		int hi = (int)house->Class->House;
		if ((unsigned)hi < HOUSE_COUNT && _said[hi] != (int)best &&
			(long)Frame - _when[hi] >= (long)(TICKS_PER_SECOND * 5)) {

			_when[hi] = (long)Frame;
			_said[hi] = (int)best;
			CAC_Log_Volatile("ai.air",
				"%s: %d station(s), %d flying + %d building = %d of %d, IQ %d vs %d -> %s",
				house->Class->IniName, pads, flying, owned - flying, owned, pads * 4,
				house->IQ, Rule.IQAircraft,
				best == AIRCRAFT_NONE
					? "nothing"
					: AircraftTypeClass::As_Reference(best).IniName);
		}
	}

	return(best);
}


/***********************************************************************************************
 * CAC_Shows_Damage -- Should this structure be drawn as its damaged still?                     *
 *                                                                                             *
 *    Building.pain():                                                                          *
 *                                                                                              *
 *        if (this.healthPerc <= 25) {                                                          *
 *            if (!this.stats.weapon || this.stats.isHQ) {                                      *
 *                this.MCsprite.building.gotoAndStop(1);                                        *
 *                this.MCsprite.shadow.gotoAndStop(1);                                          *
 *            }                                                                                 *
 *            ...                                                                               *
 *        }                                                                                     *
 *                                                                                              *
 *    Two departures from Tiberian Dawn.  The threshold is a quarter, not a half.  And an       *
 *    armed structure that is not the headquarters -- the Defence Station and the Assault       *
 *    Turret, whose frames are their sixteen facings -- never swaps at all, because it has      *
 *    no frame to spare for the damage.                                                         *
 *                                                                                             *
 *    Repair reverses it: once `healthPerc > 25` the clip goes back to "loopstart".              *
 *=============================================================================================*/
bool CAC_Shows_Damage(BuildingClass const *building)
{
	if (building == NULL || building->Class == NULL) return(false);

	if (!CAC_Is_CrystAlien_Type(building->Class->IniName)) {
		return(building->Health_Ratio() < 0x0080);
	}

	if (building->Health_Ratio() > CAC_DAMAGE_RATIO) return(false);

	bool armed = (building->Class->Primary != WEAPON_NONE);
	return(!armed || CAC_Is_HQ(building->Class->Type));
}


/***********************************************************************************************
 * CAC_Attached_Anim -- The animation of a given kind riding on this object.                   *
 *                                                                                             *
 *    A heap slot index is not a handle.  TFixedIHeapClass compacts on delete --                *
 *    ActivePointers.Delete(pointer) closes the gap -- so every slot after a freed one moves    *
 *    down by one and a stored index quietly comes to name a different animation.               *
 *                                                                                             *
 *    That is what CACSmoke and CACSparks were.  Checking the type and IsActive on the way out  *
 *    caught the common case, and hid the one that matters: when the slot happened to hold      *
 *    *another* building's smoke, both tests passed, and the building deleted a neighbour's     *
 *    animation while the neighbour went on holding the index.  Delete_This restores the vtable *
 *    on the way out, so the corpse answers virtual calls, and the next Center_Coord on it       *
 *    reaches Validate with a slot that is no longer its own.                                   *
 *                                                                                             *
 *    AnimClass already carries the only handle that survives compaction: Object, the thing it  *
 *    is attached to.  ANIM.CPP scans on exactly that to decide whether an object still has any *
 *    animation riding it, so this is the engine's own idiom rather than a new one.             *
 *=============================================================================================*/
static AnimClass * CAC_Attached_Anim(ObjectClass const * owner, AnimType type)
{
	if (owner == NULL) return(NULL);

	for (int index = 0; index < Anims.Count(); index++) {
		AnimClass * a = Anims.Ptr(index);
		if (a != NULL && a->IsActive && a->Object == owner && *a == type) {
			return(a);
		}
	}
	return(NULL);
}


/***********************************************************************************************
 * CAC_Damage_Smoke -- Hang smoke on a hurt structure, and take it off when mended.            *
 *                                                                                             *
 *    pain() attaches one "smoke" clip the first time a structure reaches a quarter health,     *
 *    guarded by `if (!this.MCsprite.building.smoke)` so it never stacks, and plays one of      *
 *    "INT_damage_1..3" with it.  Building.think removes it again the moment repairs bring      *
 *    the structure back above a quarter.                                                       *
 *                                                                                             *
 *    Note this half runs for every structure, the turret included -- it is only the swap to    *
 *    the damaged still that the turret sits out.                                               *
 *=============================================================================================*/
void CAC_Damage_Smoke(BuildingClass *building)
{
	if (building == NULL || building->Class == NULL) return;
	if (!CAC_Is_CrystAlien_Type(building->Class->IniName)) return;
	if (building->IsInLimbo || !building->IsActive) return;
	if (building->BState == BSTATE_CONSTRUCTION) return;

	bool hurt = (building->Health_Ratio() <= CAC_DAMAGE_RATIO);

	/*
	**	Found by what it is attached to, not by where it sits in the heap.
	**	See CAC_Attached_Anim -- the index this used to store stopped being this
	**	building's the moment any earlier animation expired.
	*/
	AnimClass *smoke = CAC_Attached_Anim(building, ANIM_CAC_SMOKE);
	building->CACSmoke = (smoke != NULL) ? 1 : 0;

	if (hurt && smoke == NULL) {
		/*
		**	`sfx.play("INT_damage_" + (random(3) + 1))`, straight, in the same
		**	branch of Building.pain that first attaches the smoke.  Ungated like
		**	the pain cue above it.
		*/
		static VocType const _hurt[] = {VOC_CAC_INT_DMG1, VOC_CAC_INT_DMG2, VOC_CAC_INT_DMG3};
		Sound_Effect(_hurt[Random_Pick(0, 2)]);
		AnimClass *a = new AnimClass(ANIM_CAC_SMOKE, building->Center_Coord());
		if (a != NULL) {
			a->Attach_To(building);
			building->CACSmoke = 1;
		}
		return;
	}

	if (!hurt && smoke != NULL) {
		smoke->Delete_This();
		building->CACSmoke = 0;
	}
}


/***********************************************************************************************
 * CAC_Unit_Smoke -- Hang smoke on a hurt vehicle or aircraft, and take it off when mended.    *
 *                                                                                             *
 *    Unit.pain does the same thing to a unit that Building.pain does to a structure --         *
 *                                                                                             *
 *        if(this.healthPerc <= 25) {                                                          *
 *           if(!this.MCsprite.unit.smoke && this.stats.mechanical) {                          *
 *              _loc2_ = this.MCsprite.unit.attachMovie("smoke","smoke",1);                    *
 *                                                                                             *
 *    -- and doRepair takes it off again with `this.target.MCsprite.unit.smoke.removeMovieClip()`.
 *    Only the structure half was ever built, so a Driller or a Fighter on its last quarter     *
 *    looked untouched.                                                                        *
 *                                                                                             *
 *    `mechanical` is `obstruct || flying`, so vehicles and craft smoke and infantry do not --  *
 *    the same test CAC_Death_Effect reads to decide a fireball from a puff.                    *
 *                                                                                             *
 *    No sound here.  Building.pain plays INT_damage_N when the smoke first catches; Unit.pain  *
 *    attaches the clip and says nothing.                                                       *
 *=============================================================================================*/
void CAC_Unit_Smoke(TechnoClass *obj)
{
	if (obj == NULL) return;
	if (obj->IsInLimbo || !obj->IsActive) return;

	RTTIType what = obj->What_Am_I();
	if (what != RTTI_UNIT && what != RTTI_AIRCRAFT) return;

	TechnoTypeClass const *type = obj->Techno_Type_Class();
	if (type == NULL || !CAC_Is_CrystAlien_Type(type->IniName)) return;

	bool hurt = (obj->Health_Ratio() <= CAC_DAMAGE_RATIO);
	AnimClass *smoke = CAC_Attached_Anim(obj, ANIM_CAC_SMOKE);

	if (hurt && smoke == NULL) {
		AnimClass *a = new AnimClass(ANIM_CAC_SMOKE, obj->Center_Coord());
		if (a != NULL) a->Attach_To(obj);
		return;
	}

	if (!hurt && smoke != NULL) {
		smoke->Delete_This();
	}
}


/***********************************************************************************************
 * CAC_Load_Steps -- How many mouthfuls make a full hold.                                      *
 *                                                                                             *
 *    UnitNav.feed takes up to ten from a tile at a time --                                     *
 *                                                                                             *
 *        var got = Math.floor(min(10, arena.baits[x][y]));                                     *
 *        arena.baits[x][y] = limit(baits[x][y] - got, 1, 100);                                 *
 *        this.parent.cargo += got;                                                             *
 *                                                                                             *
 *    -- and turns for home at `cargo >= stats.maxCargo`, which is two hundred.  So a hold is   *
 *    twenty mouthfuls, not Tiberian Dawn's twenty-eight, and a crystal tile is never mined     *
 *    out: the limit floors it at one rather than zero.                                         *
 *=============================================================================================*/
int CAC_Load_Steps(TechnoClass const *object)
{
	if (object != NULL && object->What_Am_I() == RTTI_UNIT &&
		CAC_Is_Miner(*((UnitClass const *)object))) {

		TechnoTypeClass const *type = object->Techno_Type_Class();
		if (type != NULL && CAC_Is_CrystAlien_Type(type->IniName)) {
			return(CAC_UDGOOD_CARGO / CAC_MINE_MOUTHFUL);
		}
	}
	return(UnitTypeClass::STEP_COUNT);
}


/***********************************************************************************************
 * CAC_Bail_Credits -- What one mouthful is worth on delivery.                                 *
 *                                                                                             *
 *    Arriving home runs `cashUp(this.parent.cargo, this.parent)`, and cashUp is not the        *
 *    identity everyone assumes:                                                                *
 *                                                                                             *
 *        this.cash += cash * 5;                                                                *
 *                                                                                             *
 *    Every figure the source quotes for income is therefore a fifth of the credits it really   *
 *    pays.  A full two-hundred hold is a thousand credits, not two hundred, and the Driller's  *
 *    hundred a tick is five hundred.  Getting this wrong left the whole economy at a fifth     *
 *    speed, which reads as the game being punishingly slow rather than as a bug.               *
 *=============================================================================================*/
int CAC_Bail_Credits(TechnoClass const *object)
{
	if (CAC_Load_Steps(object) != UnitTypeClass::STEP_COUNT) {
		return(CAC_MINE_MOUTHFUL * CAC_CASHUP);
	}
	return(0);
}


/***********************************************************************************************
 * CAC_Mining_Sparks -- Show a miner working, and stop when it stops.                          *
 *                                                                                             *
 *    UnitNav.feed switches `sparks._visible` on with every mouthful and off again the moment   *
 *    the hold is full and the miner turns for home; returnHome and voyage both clear it too,   *
 *    so it burns only while the miner is actually standing on crystal and taking some.         *
 *                                                                                             *
 *    The sparks are a child of the unit's own clip in the source.  Here they are an animation  *
 *    attached to it, which comes to the same thing on screen and lets the miner keep one set   *
 *    of frames.                                                                                *
 *=============================================================================================*/
void CAC_Mining_Sparks(UnitClass *unit, bool working)
{
	if (unit == NULL || unit->Class == NULL) return;
	if (!CAC_Is_CrystAlien_Type(unit->Class->IniName)) return;

	/*
	**	Same correction as the damage smoke:  by attachment, not by heap slot.
	*/
	AnimClass *sparks = CAC_Attached_Anim(unit, ANIM_CAC_SPARKS);
	unit->CACSparks = (sparks != NULL) ? 1 : 0;

	if (working && sparks == NULL && !unit->IsInLimbo && unit->IsActive) {
		AnimClass *a = new AnimClass(ANIM_CAC_SPARKS, unit->Center_Coord());
		if (a != NULL) {
			a->Attach_To(unit);
			unit->CACSparks = 1;
		}
		/*
		**	Volatile: reported as not appearing at all.  Everything between the
		**	rendered clip and the animation type resolves -- six frames in the
		**	classic archive, six textures in cacsprks.zip, six shapes registered
		**	in TD_VFX -- so if they are genuinely absent it is either this line
		**	never running or the animation dying on the way out, and nothing in
		**	the code can tell those apart after the fact.  This can.
		*/
		CAC_Log_Volatile("sparks", "%s lights up at %d,%d (%s)",
							  unit->Class->IniName,
							  Cell_X(Coord_Cell(unit->Center_Coord())),
							  Cell_Y(Coord_Cell(unit->Center_Coord())),
							  (a != NULL) ? "attached" : "the anim heap refused it");
		return;
	}

	if (!working && sparks != NULL) {
		sparks->Delete_This();
		unit->CACSparks = 0;
	}
}


/***********************************************************************************************
 * CAC_Spawn_Survivors -- Everyone a structure gives up, wherever it is losing them.            *
 *                                                                                             *
 *    Building.destroy takes one argument, `sold`, and the crew does not depend on it:          *
 *                                                                                             *
 *        for (var k in this.stats.children) {                                                  *
 *            var c = this.stats.children[k];                                                   *
 *            if (construction.shortcuts[c].total < construction.shortcuts[c].max) {             *
 *                this.parent.units.push(new Unit(this.parent, c, dockPos.x, dockPos.y,          *
 *                                               0.125 * random(8), this.team));                 *
 *            }                                                                                 *
 *        }                                                                                      *
 *                                                                                             *
 *    -- one of each, at the dock, facing anywhere, whether the structure was sold or shot out   *
 *    from under its owner.  Tiberian Dawn does something quite different when a building dies:  *
 *    it rolls for a survivor on every cell of the footprint, and whoever turns up is always the *
 *    same `Crew_Type`.  So the Experiment Lab never gave up its Viper and the Driller never     *
 *    gave up its Saboteur, and a Vehicle Factory sometimes gave up four men and sometimes none. *
 *                                                                                             *
 *    `children` is authored per structure and is not the same as what the structure builds --   *
 *    the Power Plant lists one, and it builds nothing at all.                                   *
 *=============================================================================================*/
int CAC_Spawn_Survivors(BuildingClass *building)
{
	if (building == NULL || building->Class == NULL) return(-1);

	int count = CAC_Survivor_Count(building->Class->Type);
	if (count < 0) return(-1);		/* not CrystAlien's -- leave the caller to it */

	/*
	**	The owner, reached three deep, on the one path where a building is coming
	**	apart.  Nothing here has been seen to fail, but it runs from inside
	**	destruction and there is no version of it worth crashing for.
	*/
	if (building->House == NULL || building->House->Class == NULL) return(-1);

	/*
	**	The dock is where a structure puts anything it lets out; see Exit_Object.
	*/
	CELL where = Coord_Cell(building->Docking_Coord());

	int out = 0;
	for (int i = 0; i < count; i++) {
		if (CAC_Spawn_Survivor(building->Class->Type, i,
								building->House->Class->House, where)) out++;
	}
	return(out);
}


/***********************************************************************************************
 * CAC_Building_Revenue -- Pay out a CrystAlien income building.                               *
 *                                                                                             *
 *    Building.think:                                                                          *
 *                                                                                             *
 *        if(this.stats.refinery) {                                                            *
 *           if(!(this.count % this.stats.revenueCount)) {                                     *
 *              this.parent.cashUp(this.stats.revenue, this);                                  *
 *              this.refine();                                                                 *
 *           }                                                                                 *
 *        }                                                                                    *
 *                                                                                             *
 *    `count` is a per-building frame counter, so several Drillers pay out staggered rather    *
 *    than together; the building's own ID reproduces that here.                               *
 *                                                                                             *
 *    refine() sets the crystal marker back to full under the building's own footprint, which  *
 *    is how a Driller keeps a field alive.  That half does not carry across -- Tiberian Dawn  *
 *    cannot have harvestable Tiberium beneath a structure -- so what is left is the income,   *
 *    which is what the building is for.                                                       *
 *=============================================================================================*/
void CAC_Building_Revenue(BuildingClass *building)
{
#ifdef CAC_BLGOOD_REVENUE
	if (building == NULL || building->Class == NULL) return;
	if (building->Class->Type != CAC_SLOT_BLGOOD &&
		building->Class->Type != CAC_SLOT_BLEVIL) return;
	if (building->House == NULL) return;

	/*
	**	Building.think runs the refinery branch whatever the base's power, so
	**	there is deliberately no power test here.
	*/

	/*
	**	Stagger the payouts the way CrystAlien's per-building frame counter does.
	*/
	if ((int)((Frame + Buildings.ID(building)) % CAC_BLGOOD_REVENUE_TICKS) != 0) return;

	CAC_Credit_Harvest(building->House, CAC_BLGOOD_REVENUE * CAC_CASHUP);
	if (building->House == PlayerPtr && Map.In_View(Coord_Cell(building->Center_Coord()))) {
		CAC_Interface_Sound(VOC_CAC_INT_CASH6);
	}
#endif
}

/***********************************************************************************************
 * CAC_Crew_Type -- Who walks out of a CrystAlien object when it is sold or destroyed.         *
 *                                                                                             *
 *    Tiberian Dawn hands out civilians (INFANTRY_C1..C9) for unarmed structures and neutral   *
 *    owners, and its own INFANTRY_E1/E7 otherwise.  In the conversion E1 and E7 are the       *
 *    CrystAlien Infantry and Engineer, so those came out right, but the civilian arms of the  *
 *    same routine were still handing out C&C townsfolk -- which is what a sold structure was  *
 *    mixing in.  CrystAlien has no civilians, so every survivor is its Infantry.              *
 *                                                                                             *
 * OUTPUT:  INFANTRY_NONE for anything that is not a CrystAlien type, so the stock routine     *
 *          keeps its own answer -- which is what the supplemental build's GDI and Nod want.   *
 *=============================================================================================*/
InfantryType CAC_Crew_Type(TechnoTypeClass const *type)
{
	if (type == NULL || !CAC_Is_CrystAlien_Type(type->IniName)) return(INFANTRY_NONE);

	/*
	**	The crew that walks out belongs to the structure's own faction, which
	**	after a capture is not the same as its owner's.
	*/
	bool evil = (CAC_Code_Of(type->What_Am_I(),
							 CAC_Type_Index_Of_Type(type)) & CAC_CODE_EVIL) != 0;
	return((InfantryType)CAC_Slot_For(RTTI_INFANTRYTYPE, CAC_CODE_UA, evil));
}

/***********************************************************************************************
 * CAC_Survivor_Count / CAC_Survivor_Type -- What a lost CrystAlien structure gives up.        *
 *                                                                                             *
 *    Building.destroy walks the type's `children` list and spawns each entry at the           *
 *    building's dock.  It is a short, authored list -- the Technology Centre gives up a        *
 *    JetPack Explorer, the Driller an Infantry and an Engineer, and the Command Base, the      *
 *    Defence Station and the Ops Ship give up nothing at all.                                 *
 *                                                                                             *
 *    Tiberian Dawn instead spawns `Raw_Cost()/200` of them, clamped to 1..5, all from          *
 *    Crew_Type() -- which is where the C&C townsfolk were coming from.                        *
 *=============================================================================================*/
struct CACSurvivorEntry {
	StructType		Struct;
	int				Count;
	struct { RTTIType RTTI; int Type; } Who[3];
};

static CACSurvivorEntry const CACSurvivors[] = {
#include "CACSURVIVORS.INC"
};

static CACSurvivorEntry const *CAC_Survivors_Of(StructType type)
{
	if (!CAC_Is_CrystAlien_Type(BuildingTypeClass::As_Reference(type).IniName)) return(NULL);
	for (int i = 0; i < (int)(sizeof(CACSurvivors) / sizeof(CACSurvivors[0])); i++) {
		if (CACSurvivors[i].Struct == type) return(&CACSurvivors[i]);
	}
	return(NULL);
}

int CAC_Survivor_Count(StructType type)
{
	CACSurvivorEntry const *e = CAC_Survivors_Of(type);
	return((e != NULL) ? e->Count : -1);
}


/***********************************************************************************************
 * CAC_Spawn_Survivor -- Put one of a lost structure's `children` on the map.                  *
 *                                                                                             *
 *    The list is not all infantry -- the Technology Centre gives up a JetPack Explorer, which  *
 *    is an aircraft -- so each entry carries its own class.                                   *
 *=============================================================================================*/
bool CAC_Spawn_Survivor(StructType type, int index, HousesType owner, CELL cell)
{
	CACSurvivorEntry const *e = CAC_Survivors_Of(type);
	if (e == NULL || index < 0 || index >= e->Count) return(false);

	/*
	**	"if (construction.shortcuts[c].total < construction.shortcuts[c].max)" --
	**	a crew member who would put the player over that type's cap does not walk
	**	out at all.
	*/
	if (CAC_At_Type_Cap(HouseClass::As_Pointer(owner),
						e->Who[index].RTTI, e->Who[index].Type)) return(false);

	/*
	**	Made and placed here rather than through Create_And_Place, because that
	**	routine leaks whatever it cannot put down.
	**
	**	    InfantryClass * i = new InfantryClass(Type, house);
	**	    COORDINATE coord = Map[cell].Closest_Free_Spot(Cell_Coord(cell));
	**	    if (coord) { return(i->Unlimbo(coord, DIR_E)); } else { delete i; }
	**
	**	It cleans up when there is no free spot at all and not when Unlimbo itself
	**	refuses -- and Unlimbo asks Can_Enter_Cell, which is a different and
	**	stricter question than Closest_Free_Spot.  The one path that reaches here
	**	is a structure coming apart, which is precisely when its own dock is full
	**	of the rubble it is turning into, so the refusal is the common case.
	**
	**	What is left behind is active, owned, in limbo, and standing at a cell
	**	number off the map.  It cannot be seen, selected, commanded or killed, and
	**	every routine that counts what a house owns counted it -- including the one
	**	that greys a cameo at the type cap.  Every structure lost put one or two
	**	more of them in the bank, so a player who had been fighting for a while
	**	could not build infantry at all and had none to show for the five the cap
	**	believed were standing somewhere.
	**
	**	So: own the pointer, try the dock and then outward the way a crew would
	**	actually walk, and delete it if the ground will not take it.
	*/
	TechnoClass *born = NULL;
	switch (e->Who[index].RTTI) {
		case RTTI_INFANTRYTYPE:
			born = new InfantryClass((InfantryType)e->Who[index].Type, owner);
			break;
		case RTTI_AIRCRAFTTYPE:
			born = new AircraftClass((AircraftType)e->Who[index].Type, owner);
			break;
		case RTTI_UNITTYPE:
			born = new UnitClass((UnitType)e->Who[index].Type, owner);
			break;
		default:
			return(false);
	}
	if (born == NULL) return(false);

	for (int reach = 0; reach <= CAC_SURVIVOR_REACH; reach++) {
		for (int dy = -reach; dy <= reach; dy++) {
			for (int dx = -reach; dx <= reach; dx++) {

				/*	Only the ring, so the dock itself is tried first and once. */
				if (reach > 0 && ABS(dx) != reach && ABS(dy) != reach) continue;

				CELL c = (CELL)(cell + dx + dy * MAP_CELL_W);
				if ((unsigned)c >= MAP_CELL_TOTAL) continue;
				if (!Map.In_Radar(c)) continue;

				COORDINATE spot = Map[c].Closest_Free_Spot(Cell_Coord(c));
				if (spot == 0) continue;
				/*
				**	Facing anywhere, as CrystAlien's new units do (`0.125 * random(8)`).
				**	Every survivor used to face east, and a JetPack Explorer let out
				**	of a Technology Centre set off that way.
				*/
				if (born->Unlimbo(spot, (DirType)(Random_Pick(0, 7) * 32))) return(true);
			}
		}
	}

	delete born;
	return(false);
}

/***********************************************************************************************
 * CAC_Is_HQ / CAC_Unit_From_HQ -- The headquarters builds the Miner.                          *
 *                                                                                             *
 *    CrystAlien produces the Crystal Miner at the Command Base: checkPrereq asks only for      *
 *    "BA", and the unit's `home` is that same building, which is where a finished one appears. *
 *                                                                                             *
 *    Tiberian Dawn gives a structure a single ToBuild, and the Command Base has to be          *
 *    RTTI_BUILDINGTYPE because it is also the Construction Yard.  So the Miner briefly wanted  *
 *    a Vehicle Factory here, which is not what the source game asks for and is a long way up   *
 *    the tree for the unit your whole economy runs on.                                        *
 *                                                                                             *
 *    Rather than move the goalposts, the engine now lets the headquarters host this one        *
 *    vehicle: Who_Can_Build_Me accepts it, Update_Buildables offers it, and the factory        *
 *    counters include it, so production, the sidebar and the exit all behave as though it      *
 *    were a vehicle factory -- for the Miner and nothing else.                                *
 *=============================================================================================*/
bool CAC_Is_HQ(StructType type)
{
	if (!CAC_Is_CrystAlien_Type(BuildingTypeClass::As_Reference(type).IniName)) return(false);
	return(CAC_Is_Code(RTTI_BUILDINGTYPE, (int)type, CAC_CODE_BA));
}

bool CAC_Unit_From_HQ(UnitType type)
{
	return(CAC_Is_Code(RTTI_UNITTYPE, (int)type, CAC_CODE_UD));
}

/***********************************************************************************************
 * CAC_Time_To_Build -- How long CrystAlien takes to build something.                          *
 *                                                                                             *
 *    Construction.construct advances an item by `this.speed` per frame and finishes at        *
 *    `constructionTime`, which for every type equals its cost:                                *
 *                                                                                             *
 *        this.speed = 8;                          // Construction's constructor               *
 *        item.progress += _loc3_;                 // _loc3_ = speed, halved on low power      *
 *        item.progress = min(item.progress, item.constructionTime);                           *
 *                                                                                             *
 *    At the source game's 23 frames per second that is cost / (8 * 23) seconds -- a 500        *
 *    credit Training Camp in a little under three.  Tiberian Dawn instead spends one tick per  *
 *    credit at 15 ticks per second, so the same building took thirty-three seconds: about      *
 *    twelve times too slow, which is what made the mod feel sluggish next to the original.     *
 *                                                                                             *
 *    (Low power halves the rate in both games -- Tiberian Dawn does it in FactoryClass, so     *
 *    that half needs no help here.)                                                           *
 *                                                                                             *
 * OUTPUT:  The tick count to use, or -1 for anything that is not a CrystAlien type so the     *
 *          stock timing is left alone -- which is what the supplemental build's GDI and Nod   *
 *          want.                                                                              *
 *=============================================================================================*/
int CAC_Time_To_Build(TechnoTypeClass const *type, int cost)
{
	if (type == NULL || !CAC_Is_CrystAlien_Type(type->IniName)) return(-1);

	/*
	**	The pizza is timed by CrystAlien's clock, not by its price here.
	**
	**	This derives a build time from cost, which is right for every type the
	**	conversion ports at the source's own price.  The Pizza Delivery is not one
	**	of those: UnitStats gives it `cost 5000` and `constructionTime 5000`, and
	**	Pizza Mode charges fifty thousand for it -- so reading the price would make
	**	it ten times the wait the source asks for, on top of a wait that is already
	**	the longest in the game.
	**
	**	So it is pinned to the source's own figure.  The price is the mode's
	**	business; the clock is CrystAlien's.
	*/
	if (strncmp(type->IniName, "UJ", 2) == 0) {
		return(MAX(1, (CAC_UJ_SOURCE_COST * TICKS_PER_SECOND) /
							(CAC_BUILD_SPEED * CAC_BUILD_FPS)));
	}

	return(MAX(1, (cost * TICKS_PER_SECOND) / (CAC_BUILD_SPEED * CAC_BUILD_FPS)));
}

/***********************************************************************************************
 * CAC_Idle_Turret_Spin -- Sweep a defence turret that has nothing to shoot.                   *
 *                                                                                             *
 *    Building.draw, for a rotatable armed structure:                                          *
 *                                                                                             *
 *        if(!(this.count % 4) && this.blink && <owner has power>) {                           *
 *           this.angle += sgn(Math.sin(this.count / 100)) * 0.0625;                           *
 *           ... wrap to 0..1                                                                   *
 *        }                                                                                     *
 *                                                                                             *
 *    `blink` is the countdown set when a target search comes up empty (10 + random(4) frames)  *
 *    and cleared the moment one is acquired, so this only runs while the turret is idle.  The  *
 *    step is 0.0625 of a turn -- one of the sixteen drawn facings -- every fourth frame, and   *
 *    `sgn(sin(count/100))` flips every pi*100 frames, so it sweeps back and forth rather than  *
 *    spinning one way.                                                                        *
 *                                                                                             *
 *    Converted to Tiberian Dawn's 15 ticks against the source game's 23 frames, and to its     *
 *    256-unit facing circle.                                                                  *
 *=============================================================================================*/
void CAC_Idle_Turret_Spin(BuildingClass *building)
{
	if (building == NULL || building->Class == NULL) return;
	if (!building->Class->IsTurretEquipped) return;
	if (!CAC_Is_CrystAlien_Type(building->Class->IniName)) return;

	/*
	**	And a dark one does not sweep either.  The sweep is part of engageEnemy,
	**	which returns before reaching it when the owner has no power, so a turret
	**	that cannot shoot also stops turning -- which is the whole visible signal
	**	that a base has browned out.
	*/
	if (CAC_Power_Off(building->House)) return;

	/*
	**	Idle only: a turret with a target is aiming, not sweeping -- and one being
	**	put up or taken down is doing neither.
	*/
	if (Target_Legal(building->TarCom)) {

		/*
		**	Still aiming only if there is still something to aim at.
		**
		**	engageEnemy re-arms `blink` on every sweep of the map and shoot() is the only
		**	thing that clears it, so a turret goes back to sweeping the moment it stops
		**	actually firing.  Asking whether TarCom is *legal* is not the same question:
		**	a target that has walked out of range is still a legal target, so the turret
		**	sat pointing after something it could no longer reach and never swept again.
		**
		**	So drop it and let the engine find another.  Assign_Target(TARGET_NONE) is
		**	what it does itself when a target dies.
		*/
		if (building->In_Range(building->TarCom, 0)) return;
		building->Assign_Target(TARGET_NONE);
	}
	if (building->Mission == MISSION_CONSTRUCTION ||
		building->Mission == MISSION_DECONSTRUCTION) return;

	/*
	**	"this.friend && !this.parent.powerOff" -- a browned-out base stops sweeping.
	*/
	if (CAC_Power_Off(building->House)) return;

	int id = Buildings.ID(building);
	if ((int)((Frame + id) % CAC_SPIN_TICKS) != 0) return;

	/*
	**	sgn(sin(count / 100)) reverses every pi*100 source frames.
	*/
	int half = CAC_SPIN_SWEEP_TICKS;
	bool forward = (((Frame + id * 7) / half) % 2) == 0;

	DirType face = building->PrimaryFacing.Current();
	face = (DirType)((face + (forward ? CAC_SPIN_STEP : -CAC_SPIN_STEP)) & 0x00FF);
	building->PrimaryFacing.Set(face);
	building->Mark(MARK_CHANGE);
}

/***********************************************************************************************
 * CAC_Is_Stock_Structure -- Is this slot still holding a GDI or Nod building?                 *
 *                                                                                             *
 *    The conversion puts CrystAlien structures in GDI and Nod slots, and several of the       *
 *    engine's special cases are written against those slots by name.  This tells the two      *
 *    apart, so a rule meant for the Temple of Nod does not fire on the Technology Centre       *
 *    that took its place -- while the supplemental build, where the Temple really is the       *
 *    Temple, keeps every one of them.                                                         *
 *=============================================================================================*/
bool CAC_Is_Stock_Structure(StructType type)
{
	if (type == STRUCT_NONE) return(false);
	return(!CAC_Is_CrystAlien_Type(BuildingTypeClass::As_Reference(type).IniName));
}

/***********************************************************************************************
 * CAC_Has_Host / CAC_Hosts_Unit -- Which building produces a unit.                            *
 *                                                                                             *
 *    UnitStats gives most types a `home`, and Construction spawns a finished unit at that      *
 *    building's dock.  It is not always the building Tiberian Dawn would pick from ToBuild:    *
 *    the Miner comes out of the Command Base, the JetPack Explorer out of the Training Camp,   *
 *    the Dropship out of the Vehicle Factory and the Switch Fighter out of the Ops Ship.       *
 *                                                                                             *
 *    Where a type has a home it is the *only* building that may build it.  Without that, the   *
 *    engine finds any factory of the matching kind, which is why a finished Miner sat on       *
 *    READY until a Vehicle Factory existed and then walked out of that instead -- and why the  *
 *    JetPack Explorer, an aircraft here, never finished at all until a Radar Station stood.    *
 *=============================================================================================*/
struct CACHostEntry {
	RTTIType	RTTI;
	int			Type;
	StructType	Host;
};

static CACHostEntry const CACHosts[] = {
#include "CACHOSTS.INC"
};

static CACHostEntry const *CAC_Host_Entry(RTTIType rtti, int type)
{
	for (int i = 0; i < (int)(sizeof(CACHosts) / sizeof(CACHosts[0])); i++) {
		if (CACHosts[i].Type != type) continue;
		if (CACHosts[i].RTTI != rtti) continue;
		return(&CACHosts[i]);
	}
	return(NULL);
}

bool CAC_Has_Host(RTTIType rtti, int type)
{
	return(CAC_Host_Entry(rtti, type) != NULL);
}

/***********************************************************************************************
 * CAC_Host_Of -- The building a type is produced at, if it has one.                            *
 *=============================================================================================*/
StructType CAC_Host_Of(RTTIType rtti, int type)
{
	RTTIType want = rtti;
	if (rtti == RTTI_UNIT)      want = RTTI_UNITTYPE;
	if (rtti == RTTI_INFANTRY)  want = RTTI_INFANTRYTYPE;
	if (rtti == RTTI_AIRCRAFT)  want = RTTI_AIRCRAFTTYPE;

	for (int i = 0; i < (int)(sizeof(CACHosts) / sizeof(CACHosts[0])); i++) {
		if (CACHosts[i].RTTI == want && CACHosts[i].Type == type) return(CACHosts[i].Host);
	}
	return(STRUCT_NONE);
}


bool CAC_Hosts_Unit(StructType building, RTTIType rtti, int type)
{
	CACHostEntry const *e = CAC_Host_Entry(rtti, type);
	if (e == NULL) return(false);
	if (e->Host == building) return(true);

	/*
	**	Either headquarters will build the Miner.
	**
	**	The source hosts it at the Eagle Command -- `if (findBuilding("BA_" +
	**	team).friend) shortcuts["UD_"].active = true;` -- and that is where the
	**	dead end is.  Nothing rebuilds an Eagle Command: CAC_Prereq_Met answers
	**	false for it outright, and the only thing that becomes one is a stock MCV,
	**	which is not on a CrystAlien sidebar.  So losing it takes the Miner away
	**	for the rest of the game.
	**
	**	The Astros survive that on Drillers, which the Ops Ship grants.  The
	**	Aliens do not: there is no Alien Driller, so a Hive standing over a dead
	**	Mothership has no way to earn at all, and the team is finished while still
	**	holding a working headquarters.  Which is what "softlocked entirely even
	**	though it has an ops ship" is.
	**
	**	The building tree already treats the two as one -- BB and everything under
	**	it read `ba || bk`, exactly as doPrerequisites does -- so this brings the
	**	one production the tree still pins to the Eagle Command into line with it.
	**	A deliberate departure, and a small one: the Ops Ship keeps the Switch
	**	Fighter and the Driller that only it grants, and gains the Miner only
	**	because the alternative is a team that cannot play on.
	*/
	if (rtti == RTTI_UNITTYPE &&
		CAC_Is_Code(RTTI_UNITTYPE, type, CAC_CODE_UD)) {

		bool evil = (CAC_Code_Of(RTTI_UNITTYPE, type) & CAC_CODE_EVIL) != 0;
		int other = CAC_Slot_For(RTTI_BUILDINGTYPE, CAC_CODE_BK, evil);
		if (other >= 0 && (StructType)other == building) return(true);
	}

	return(false);
}


/***********************************************************************************************
 * CAC_AI_Role -- The building this house would put up in a stock structure's place.           *
 *                                                                                             *
 *    HouseClass::AI_Building is a base plan written as a list of structure types: build a      *
 *    STRUCT_POWER if power is short, a STRUCT_REFINERY if there is no income, a               *
 *    STRUCT_BARRACKS, a STRUCT_WEAP, and so on.  Every one of those is a Tiberian Dawn         *
 *    building, and a CrystAlien house can own none of them, so Can_Build refused the lot and   *
 *    the computer never laid a single foundation.  What it fielded was whatever the map        *
 *    handed it at the start, which is why a round could be won by killing a few drones.        *
 *                                                                                             *
 *    So the plan is read as a list of *roles* instead, and this says which of the faction's    *
 *    own buildings fills each one.  The roles line up almost exactly, because CrystAlien's     *
 *    tree is the same shape: a power plant, a barracks, a vehicle factory, a radar station, a  *
 *    technology centre and a defence station.                                                  *
 *                                                                                             *
 *    Two roles have no counterpart and are answered with the headquarters, which is never      *
 *    buildable (checkPrereq never offers it) -- so the plan simply steps over them.  The        *
 *    refinery is answered the same way on purpose: CrystAlien's income is a Miner carrying to  *
 *    the headquarters, so owning one *is* having a refinery, and there is nothing to build.    *
 *                                                                                             *
 * OUTPUT:  the structure to use, unchanged for a house that is not CrystAlien's.               *
 *=============================================================================================*/
StructType CAC_AI_Role(HouseClass const *house, StructType stock)
{
	if (house == NULL || house->Class == NULL) return(stock);

#ifdef CAC_SUPPLEMENTAL
	/*
	**	GDI and Nod build their own base in the supplemental.  Answering every role
	**	with a CrystAlien structure would have the computer put up an Eagle Command
	**	Base where it wanted a Construction Yard and nothing it recognises anywhere
	**	-- and its whole roster hangs off the buildings it is not making.
	**
	**	The CrystAlien branch is reached the way a player reaches it: the yard opens
	**	it, and AI_Building asks for the branch separately.  See CAC_AI_Extra.
	*/
	return(stock);
#else

	/*
	**	Which faction's buildings this house puts up.  A captured base does not
	**	change that: the computer builds what it is.
	*/
	bool evil = (CAC_Side(house->Class->House) == HOUSE_BAD);

	int code;
	switch (stock) {
		case STRUCT_POWER:
		case STRUCT_ADVANCED_POWER:	code = CAC_CODE_BB;	break;

		case STRUCT_BARRACKS:
		case STRUCT_HAND:				code = CAC_CODE_BC;	break;

		case STRUCT_WEAP:				code = CAC_CODE_BE;	break;

		case STRUCT_RADAR:
		case STRUCT_HELIPAD:				code = CAC_CODE_BF;	break;

		/*
		**	The Ops Ship, and the Alien Hive facing it.  It is where that half of
		**	the tree begins -- the Switch Fighter comes out of it and the Driller
		**	hangs from it -- so the computer has to put one up.
		*/
		case STRUCT_AIRSTRIP:				code = CAC_CODE_BK;	break;

		case STRUCT_TEMPLE:
		case STRUCT_BIO_LAB:				code = CAC_CODE_BG;	break;

		case STRUCT_EYE:					code = CAC_CODE_BH;	break;

		/*
		**	Every kind of gun emplacement is the one Defence Station.
		*/
		case STRUCT_TURRET:
		case STRUCT_OBELISK:
		case STRUCT_SAM:
		case STRUCT_GTOWER:
		case STRUCT_ATOWER:				code = CAC_CODE_BD;	break;

		/*
		**	And the rest -- the refinery above all -- have no counterpart.  The
		**	headquarters stands in, and is never buildable.
		*/
		default:						code = CAC_CODE_BA;	break;
	}

	int slot = CAC_Slot_For(RTTI_BUILDINGTYPE, code, evil);

	/*
	**	And the captured side's, when our own will not come.
	**
	**	This has always chosen by the house's own faction, so a computer standing
	**	over a captured Mothership went on putting up Astro buildings and never
	**	touched the tree it had just taken.  Mostly that costs nothing -- both
	**	sides have a power plant, and one is as good as the other -- but it means
	**	a capture that opens a whole tree is worth no more to the computer than
	**	the rubble would have been.
	**
	**	The rest of the machinery is already even-handed about it.  Every type is
	**	Ownable by every house; CAC_Prereq_Met takes the side from the thing being
	**	built rather than from the builder, so a captured Breeding Pit satisfies
	**	OWN(BC|EVIL) for an Astro house; CACHOSTS maps the Alien infantry to it;
	**	and AI_Infantry walks every type in the game filtered on Can_Build.  So a
	**	computer that *holds* an enemy factory already builds from it, and
	**	CAC_AI_Infiltrate sends any engineer it makes -- of either side, it never
	**	checks -- at the best thing to capture.  This one function was the piece
	**	that never looked across.
	**
	**	Only when its own is not available, so nothing changes for a house with an
	**	intact tree of its own: the captured branch is a fallback, not a
	**	preference.
	*/
	if (slot >= 0 && house->IsHuman == false) {
		BuildingTypeClass const * mine =
			&BuildingTypeClass::As_Reference((StructType)slot);
		if (!house->Can_Build(mine, house->ActLike)) {
			int other = CAC_Slot_For(RTTI_BUILDINGTYPE, code, !evil);
			if (other >= 0) {
				BuildingTypeClass const * theirs =
					&BuildingTypeClass::As_Reference((StructType)other);
				if (house->Can_Build(theirs, house->ActLike)) {
					slot = other;
				}
			}
		}
	}

	if (slot < 0) slot = CAC_Slot_For(RTTI_BUILDINGTYPE, code, !evil);
	return((slot >= 0) ? (StructType)slot : stock);
#endif
}


/***********************************************************************************************
 * CAC_AI_Extra -- The CrystAlien building a supplemental computer wants next.                 *
 *                                                                                             *
 *    In the total conversion this is nothing: CAC_AI_Role already answers every role with a    *
 *    CrystAlien structure and AI_Building puts up the lot.                                     *
 *                                                                                             *
 *    In the supplemental it is the whole of how the computer reaches the new roster.  It       *
 *    builds its own GDI or Nod base as it always did, and then wants one of each CrystAlien    *
 *    production building on top -- because that is what the CrystAlien units come out of, and  *
 *    a computer that never puts one up has the roster in its sidebar and never touches it.     *
 *                                                                                             *
 *    In tree order, one at a time, so the branch grows rather than arriving.                    *
 *                                                                                             *
 * OUTPUT:  a structure to want, or STRUCT_NONE.                                                *
 *=============================================================================================*/
StructType CAC_AI_Extra(HouseClass const *house)
{
	if (house == NULL || house->Class == NULL) return(STRUCT_NONE);

	bool evil = (CAC_Side(house->Class->House) == HOUSE_BAD);

	/*
	**	The order the tree opens in: power first because everything needs it, then
	**	somewhere to train, then defence, then the two factories, then the ladder
	**	above them.
	**
	**	This used to answer nothing at all in the total conversion, on the grounds
	**	that CAC_AI_Role had already mapped every stock role onto a CrystAlien
	**	building there.  It has, and that is not the same thing: a role is a
	**	question Tiberian Dawn knows how to ask, and the conversion's tree has
	**	rungs with no stock analogue.  What the computer could reach was therefore
	**	whatever happened to look like a Refinery or a Barracks, and the rest of
	**	the tree existed for the player alone.
	**
	**	The bots get the tree the humans get.  Can_Build still decides what is
	**	legal -- it asks CAC_Prereq_Met for the computer exactly as it does for a
	**	player, so this only ever *asks*, and asks in the order the tree opens.
	**
	**	Three rungs are deliberately not here, because each carries a condition a
	**	first-unowned walk cannot express, and each has a block of its own in
	**	AI_Building: the Ops Ship and the Alien Hive want an urgency that depends
	**	on the tier switch; the MT-201 Driller is worth two and is the only
	**	building that earns; and Santa's Sleigh needs a present opened first.
	*/
	static int const _want[] = {
		CAC_CODE_BA,		/* the headquarters, before any of it is reachable */
		CAC_CODE_BB, CAC_CODE_BC, CAC_CODE_BD, CAC_CODE_BE,
		CAC_CODE_BF,
#ifdef CAC_SUPPLEMENTAL
		/*
		**	The Ops Ship or the Hive.  The conversion reaches it through the
		**	airstrip role; in the supplemental that role asks for the stock
		**	Airstrip, so without this the Driller, Switch Fighter, Infiltrator,
		**	Commander and Crystal Reaper never came.
		*/
		CAC_CODE_BK,
#endif
		CAC_CODE_BG, CAC_CODE_BH
	};

	for (int i = 0; i < (int)(sizeof(_want) / sizeof(_want[0])); i++) {
		int slot = CAC_Slot_For(RTTI_BUILDINGTYPE, _want[i], evil);
		if (slot < 0) continue;
		if ((house->BScan & CAC_Scan_Bit((StructType)slot)) != 0) continue;
#ifdef CAC_SUPPLEMENTAL
		/*	A rung it cannot build yet is passed over, not waited on: the branch
		**	stalled on one while the stock plan beside it went on regardless. */
		if (!house->Can_Build(&BuildingTypeClass::As_Reference((StructType)slot), house->ActLike)) continue;
#endif
		return((StructType)slot);
	}
	return(STRUCT_NONE);
}


/***********************************************************************************************
 * CAC_Is_Miner -- Is this the type that carries crystal home?                                 *
 *                                                                                             *
 *    The computer treats UNIT_HARVESTER specially in several places -- it wants one before it  *
 *    wants anything else, and counts them when deciding whether it has an economy at all.      *
 *    CrystAlien's carrier is the Miner, which is a type of its own.                            *
 *=============================================================================================*/
bool CAC_Is_Miner(RTTIType rtti, int type)
{
	return(CAC_Is_Code(rtti, type, CAC_CODE_UD));
}


/***********************************************************************************************
 * CAC_Has_Radar / CAC_Has_Satellite -- Does this player have the minimap, and the map?        *
 *                                                                                             *
 *    Level.think decides both once a frame, from the buildings the player owns:               *
 *                                                                                             *
 *        radar = radar || stats.isRadar;                                                      *
 *        sat   = sat   || stats.isSatellite;                                                  *
 *        this.arena.shroud.active = !(sat && !this.powerLow);                                 *
 *        this.arena.radar.active  = (radar || sat) && !this.powerLow;                         *
 *                                                                                             *
 *    So the Astro Radar Station and the Alien Sonar Station both do it -- they are the two     *
 *    sides of BF and they carry the same flag -- and so does the Satellite Uplink, which       *
 *    lifts the shroud as well.  A brownout takes all of it back.                              *
 *                                                                                             *
 *    Nothing granted it before: Tiberian Dawn reads one bit, STRUCTF_RADAR, and that is its    *
 *    own Communications Center, which no CrystAlien player can own.  These are the bits that   *
 *    actually stand for the buildings that do it.                                             *
 *                                                                                             *
 * OUTPUT:  whether the house owns one, ignoring power -- the caller decides about that,        *
 *          because Tiberian Dawn expresses low power its own way.                             *
 *=============================================================================================*/
bool CAC_Has_Radar(HouseClass const *house)
{
	if (house == NULL) return(false);
	return((house->BScan & (CAC_RADAR_SCAN | CAC_SATELLITE_SCAN)) != 0);
}


/***********************************************************************************************
 * CAC_Superweapon_Is_Ours -- Did the shot that is about to land come from CrystAlien's?       *
 *                                                                                             *
 *    In the conversion there is only one kind of uplink and the answer is always yes.          *
 *                                                                                             *
 *    In the supplemental a house can hold an Advanced Communications Center and an Orbital     *
 *    Uplink at once, and both grant SPC_ION_CANNON -- the engine keeps one superweapon, not    *
 *    one per building.  Asking whether the house is "a CrystAlien house" gave CrystAlien's     *
 *    six-tile column to GDI's ion beam as soon as a Solar Panel went up beside the Comm        *
 *    Center, which is the wrong answer in the one case it is easy to get wrong.                *
 *                                                                                             *
 *    So it is CrystAlien's only when nothing else could have fired it.  With both standing,    *
 *    the engine's own beam is the safer of the two to show: it is what GDI's building has      *
 *    always done, and the uplink is the newcomer here.                                         *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Log_Abi -- Name the call that did not put a callee-saved register back.                  *
 *                                                                                             *
 *    Said once per site, ever.  The render path runs every frame and a violation will repeat   *
 *    every frame with it; one line naming the call is the whole of what is wanted.             *
 *=============================================================================================*/
void CAC_Log_Abi(char const * what, unsigned long before, unsigned long after)
{
	if (!CAC_Logging()) return;

	static char const * _said[16];
	static int _count = 0;

	for (int i = 0; i < _count; i++) {
		if (_said[i] == what) return;
	}
	if (_count < 16) _said[_count++] = what;

	CAC_Log("abi", "%s returned with esi 0x%08X instead of 0x%08X",
			  what, (unsigned)after, (unsigned)before);
}


/***********************************************************************************************
 * CAC_Ground_Ok -- Is this cell buildable, for this building?                                  *
 *                                                                                             *
 *    Tiberian Dawn says no cell of crystal is buildable by anything:                          *
 *                                                                                             *
 *        {143, {S3,S3,S3,S4,S6,S5,S1}, false},   // LAND_TIBERIUM                             *
 *                                                                                             *
 *    and Is_Generally_Clear ends with `return(::Ground[Land_Type()].Build)`, so every crystal  *
 *    cell fails it.  That is the whole of "I cannot place the Driller on the tiberium", and it *
 *    is why the overlay looked inverted: the proximity flag said yes on crystal while the      *
 *    clear flag said no, and the client colours a cell by the second one.  Two flags           *
 *    disagreeing, not a test returning the wrong answer.                                       *
 *                                                                                             *
 *    CrystAlien has no such rule -- crystal is ordinary ground there, and the one building     *
 *    that must stand on it would be unplaceable if it had.  So the Driller is exempted, and    *
 *    only the Driller: changing Ground[LAND_TIBERIUM].Build would let every structure in the   *
 *    game sit on a field, which is a different game.                                           *
 *                                                                                             *
 *    Everything else Is_Generally_Clear refuses for still stands.  An occupier is checked      *
 *    explicitly because CAC_Crystal_Cell_Ok deliberately tolerates one -- a Miner sitting on a *
 *    field does not spoil it for the *search*, but it certainly blocks a foundation.           *
 *=============================================================================================*/
bool CAC_Ground_Ok(StructType type, CELL cell)
{
	if ((unsigned)cell >= MAP_CELL_TOTAL) return(false);

	/*
	**	Never on top of a pickup.
	**
	**	Is_Generally_Clear refuses a flag and a wall and lets everything else
	**	through, so a crate is buildable over -- stock Tiberian Dawn behaviour, and
	**	harmless there because a crate is worth a few hundred credits.  Here a
	**	present is the only way into Santa's Sleigh and there may be one on the
	**	whole map for the round, so burying one under a Radar Station can cost a
	**	branch of the tree outright.  Reported exactly that way: a gift that could
	**	not be reached, under a building that had since been destroyed.
	**
	**	Refusing the placement is the fix that needs no theory about why the cell
	**	was still unreachable afterwards -- the situation simply does not arise.
	*/
	if (Map[cell].Overlay != OVERLAY_NONE &&
		OverlayTypeClass::As_Reference(Map[cell].Overlay).IsCrate) {
		return(false);
	}

	if (Map[cell].Is_Generally_Clear()) return(true);

	if (!CAC_Needs_Crystal_Site(type)) return(false);
	if (Map[cell].Cell_Occupier() != NULL) return(false);

	/*
	**	Everything else this branch has to refuse -- a structure merely standing over
	**	the cell, and a flag lying on it -- is inside CAC_Crystal_Cell_Ok, which is
	**	also the routine the client's placement overlay asks.  One place, so the
	**	cursor cannot promise what the click then refuses.
	*/
	return(CAC_Crystal_Cell_Ok(cell));
}


/***********************************************************************************************
 * CAC_Crystal_Cell_Ok -- One tile, judged on its own.                                          *
 *                                                                                             *
 *    Placer.think marks each tile of the footprint valid or invalid separately -- it walks     *
 *    width by height and calls gotoAndStop on a marker per tile -- and the placement overlay   *
 *    this client draws works the same way: Get_Placement_State asks the question once per      *
 *    cell and the client puts the answers together.                                           *
 *                                                                                             *
 *    Answering that per-cell question with a whole-footprint verdict is what made the field    *
 *    in the report unbuildable.  Every cell was being asked "is the 2x2 starting *here* all    *
 *    crystal", so a cell only passed if it and its three neighbours to the east and south      *
 *    were crystal -- and since the client wants that of every cell under the cursor, the       *
 *    effect was to demand a solid three-by-three patch for a two-by-two building.  On a        *
 *    Tiberian Dawn field, which grows in ragged blobs, that is close to impossible.            *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Crystal_Spread_Ok -- May a field grow into this cell?                                    *
 *                                                                                             *
 *    MapClass::Logic spreads crystal into an adjacent cell that is clear, carries no overlay   *
 *    and holds no Cell_Object.  Cell_Object is the occupier, and a CrystAlien structure claims *
 *    less ground than it covers -- setBlock leaves a lane down two sides so a packed base can  *
 *    be walked through -- so every cell a building merely stands over passed that test and     *
 *    grew crystal underneath the half of the structure you can see.                            *
 *                                                                                             *
 *    Which is what "a bot placed a power plant on a crystal field" turns out to be.  Placement *
 *    refuses crystal and always did: Legal_Placement walks the covered footprint and           *
 *    CAC_Ground_Ok refuses every cell of it that is not clear ground.  The plant did not move. *
 *    The field arrived.                                                                        *
 *                                                                                             *
 *    And not onto a flag.  The home cell carries OVERLAY_FLAG_SPOT and was already refused by  *
 *    the overlay test; one lying where its carrier died sets IsFlagged and no overlay, and     *
 *    crystal growing over it is crystal nobody can stand on afterwards.                        *
 *=============================================================================================*/
bool CAC_Crystal_Spread_Ok(CELL cell)
{
	if ((unsigned)cell >= MAP_CELL_TOTAL) return(false);

	CellClass const & c = Map[cell];
	if (c.IsFlagged) return(false);

	for (int over = 0; over < (int)(sizeof(c.Overlapper) / sizeof(c.Overlapper[0])); over++) {
		ObjectClass const * lap = c.Overlapper[over];
		if (lap != NULL && lap->IsActive && lap->What_Am_I() == RTTI_BUILDING) return(false);
	}
	return(true);
}


bool CAC_Crystal_Cell_Ok(CELL cell)
{
	if ((unsigned)cell >= MAP_CELL_TOTAL) return(false);
	if (!Map.In_Radar(cell)) return(false);

	CellClass const & c = Map[cell];

	/*
	**	Crystal, and nothing built on it.  A unit standing on a field does not
	**	spoil it -- it will move -- which is why only buildings and terrain count.
	*/
	if (c.Land_Type() != LAND_TIBERIUM) return(false);
	if (c.Cell_Building() != NULL) return(false);
	if (c.Cell_Terrain() != NULL) return(false);

	/*
	**	Nor a cell some other structure is merely standing over.
	**
	**	Cell_Building answers with the occupier, and a CrystAlien structure claims
	**	less ground than it covers -- setBlock leaves a lane down two sides so a
	**	packed base can be walked through -- so a covered cell carries an overlapper
	**	and no occupier and reads as empty ground here.  CAC_Ground_Ok learned that
	**	once already, with Drillers dropped through one another; this routine is the
	**	one the client's placement overlay asks, and it had not.
	*/
	for (int over = 0; over < (int)(sizeof(c.Overlapper) / sizeof(c.Overlapper[0])); over++) {
		ObjectClass const * lap = c.Overlapper[over];
		if (lap != NULL && lap->IsActive && lap->What_Am_I() == RTTI_BUILDING) return(false);
	}

	/*
	**	And never a flag.
	**
	**	Is_Generally_Clear refuses a flagged cell, which covers every other structure
	**	in the game.  This is the branch that steps around it, because crystal is not
	**	clear ground and the Driller has to be allowed on it anyway -- and stepping
	**	around that routine means stepping around all of it.  A flag lying on a
	**	crystal field would otherwise have been buildable over, which in capture the
	**	flag is the round.
	**
	**	Both spellings: IsFlagged is set wherever a flag is lying, and the overlay is
	**	set only on the cell it calls home.
	*/
	if (c.IsFlagged) return(false);
	if (c.Overlay == OVERLAY_FLAG_SPOT) return(false);

	return(true);
}


/***********************************************************************************************
 * CAC_Needs_Crystal_Site / CAC_Crystal_Site_Ok -- The MT-201 stands on a crystal field.        *
 *                                                                                             *
 *    Placer.think decides where a structure may go, and a refinery is the one special case in  *
 *    it:                                                                                      *
 *                                                                                             *
 *        if (tooFarAway                                                                       *
 *            || this.stats.refinery && !this.arena.baits[x][y]                                 *
 *            || this.arena.useds[x][y] && (!this.stats.refinery || ...))                       *
 *                                                                                             *
 *    -- tested for every tile of the footprint, so *all* of the ground it covers has to be a   *
 *    crystal field, not merely some of it.  The Driller is the only building in the game with  *
 *    `refinery: true`, which is what ties the one building that earns to the one resource on   *
 *    the map.                                                                                  *
 *                                                                                             *
 *    And the adjacency rule does not apply to it, which is the same routine a few lines up:    *
 *                                                                                             *
 *        if (!this.stats.power) { tooFarAway = false; }                                        *
 *                                                                                              *
 *    Anything that draws no power is exempt from having to be near the base, and the Driller   *
 *    draws none.  So it is placed out on a field, away from everything, which is the whole     *
 *    shape of the Astro economy -- and the reason it is worth raiding.                         *
 *                                                                                              *
 *    The covered cells are the overlap list rather than the occupy list: the Driller blocks    *
 *    one cell and covers four, and it is the four the source tests.                            *
 *=============================================================================================*/
bool CAC_Needs_Crystal_Site(StructType type)
{
	return(CAC_Is_Code(RTTI_BUILDINGTYPE, (int)type, CAC_CODE_BL));
}


bool CAC_Crystal_Site_Ok(StructType type, CELL origin)
{
	if (!CAC_Needs_Crystal_Site(type)) return(true);
	if ((unsigned)origin >= MAP_CELL_TOTAL) return(false);

	BuildingTypeClass const & btype = BuildingTypeClass::As_Reference(type);
	short const * list = btype.Overlap_List();
	if (list == NULL) list = btype.Occupy_List();
	if (list == NULL) return(false);

	while (*list != REFRESH_EOL) {
		CELL cell = (CELL)(origin + *list);
		list++;

		/*	Each tile judged the same way the overlay judges it. */
		if (!CAC_Crystal_Cell_Ok(cell)) return(false);
	}
	return(true);
}


/***********************************************************************************************
 * CAC_Driller_Range -- How far out the computer may look, which opens up as the round goes on.  *
 *                                                                                             *
 *    Three ways to reach the whole map, and the interesting one is the third.                 *
 *                                                                                             *
 *      * Time.  A step every few minutes, so early expansion is local -- outward from what the *
 *        house holds, the way a player does it -- and a distant field becomes reachable later, *
 *        once it has been earned rather than claimed in the opening minute.                    *
 *                                                                                             *
 *      * The map running out.  Capped at the map's own diagonal, measured from the corners     *
 *        with the same Distance the search uses, so the two agree by construction rather than  *
 *        by a constant somebody has to keep in step.  Past that there is no cell left to       *
 *        reach and the restraint has lifted completely.                                        *
 *                                                                                             *
 *      * An uplink, which skips the wait outright.  The Satellite Uplink and the Orbital       *
 *        Uplink are what lift the shroud -- `shroud.active = !(sat && !powerLow)` -- so a      *
 *        house holding one can see the whole map, and a computer that can see the whole map    *
 *        has no business being told it may not know where the far crystal is.  Asked of the    *
 *        buildings the house *owns*, so an Alien commander who has taken an Eagle Command      *
 *        Base and built up to its uplink gets the same reach: they paid for it with a raid.    *
 *=============================================================================================*/
int CAC_Driller_Range(HouseClass const * house)
{
	CELL topleft = XY_Cell(Map.MapCellX, Map.MapCellY);
	CELL bottomright = XY_Cell(Map.MapCellX + Map.MapCellWidth - 1,
										Map.MapCellY + Map.MapCellHeight - 1);
	int cap = (int)::Distance(::Cell_Coord(topleft), ::Cell_Coord(bottomright)) / CELL_LEPTON_W;
	if (cap < CAC_DRILLER_RANGE) cap = CAC_DRILLER_RANGE;

	/*	Eyes on the whole map, so the whole map is fair game. */
	if (CAC_Has_Satellite(house)) return(cap);

	int range = CAC_DRILLER_RANGE
			  + (int)(Frame / (TICKS_PER_MINUTE * CAC_DRILLER_WIDEN_MINUTES)) * CAC_DRILLER_WIDEN_STEP;

	if (range > cap) range = cap;
	return(range);
}


/***********************************************************************************************
 * CAC_Anchors_Build / CAC_Power_Adjacency_Ok -- CrystAlien's adjacency rule, which is about     *
 *   power and not about buildings.                                                              *
 *                                                                                             *
 *    BuildingSite.handle is the whole of it:                                                    *
 *                                                                                             *
 *        var x = pos.x + topLeftX - powerMargin;                                                *
 *        while (x < pos.x + topLeftX + width + powerMargin) {                                   *
 *           y = pos.y + topLeftY - powerMargin;                                                 *
 *           while (y < pos.y + topLeftY + height + powerMargin) {                                *
 *              if (arena.useds[x][y].isBuilding && arena.useds[x][y].friend &&                   *
 *                  arena.useds[x][y].stats.power) { tooFarAway = false; break; }                 *
 *              ...                                                                              *
 *        if (!this.stats.power) { tooFarAway = false; }                                          *
 *                                                                                             *
 *    So the anchor has to be a friendly building whose `power` is non-zero, within two cells of  *
 *    the footprint -- and a building whose own `power` is zero needs no anchor at all.           *
 *    `power` is a number: BA 1, BB 200, BC -10, BD -75, BE -25, BF -50, BG -100, BH -100,        *
 *    BJ 100, BK 1, and BI, BX and BL zero.                                                       *
 *                                                                                             *
 *    The Driller is the point of this.  BuildingStats writes its power twice --                  *
 *                                                                                             *
 *        this.power = -100;                                                                      *
 *        this.power = 0;                                                                         *
 *                                                                                             *
 *    -- and that zero does two jobs at once: the Driller needs no anchor, so it goes on any      *
 *    crystal field anywhere, and it *is* no anchor, so nothing may be built beside it.  An       *
 *    outlying Driller is an income outpost and never the seed of a base.                         *
 *                                                                                             *
 *    Tiberian Dawn's adjacency is looser in two ways, which is what made the difference:         *
 *    DisplayClass::Passes_Proximity_Check accepts any friendly building with no power test, and   *
 *    it also accepts any cell the house merely *owns* -- which includes the baseplate under a    *
 *    CrystAlien structure, since SmudgeClass::Mark's bib branch writes cell->Owner.  Either      *
 *    alone lets a remote Driller seed a base; together they made it easy.                        *
 *                                                                                             *
 *    Power maps across exactly: `stats.power` non-zero is `Power != 0 || Drain != 0`.            *
 *=============================================================================================*/
bool CAC_Anchors_Build(StructType type)
{
	if ((unsigned)type >= (unsigned)STRUCT_COUNT) return(false);

	BuildingTypeClass const &t = BuildingTypeClass::As_Reference(type);

	/*	Tiberian Dawn's own keep Tiberian Dawn's rule. */
	if (!CAC_Is_CrystAlien_Type(t.IniName)) return(true);

	return(t.Power != 0 || t.Drain != 0);
}


bool CAC_Power_Adjacency_Ok(StructType type, HousesType house, short const *list, CELL cell)
{
	if ((unsigned)type >= (unsigned)STRUCT_COUNT) return(false);
	if ((unsigned)cell >= (unsigned)MAP_CELL_TOTAL) return(false);

	BuildingTypeClass const &t = BuildingTypeClass::As_Reference(type);

	/*	`if (!this.stats.power) { tooFarAway = false; }` */
	if (t.Power == 0 && t.Drain == 0) return(true);

	if (list == NULL) return(false);

	/*
	**	The source expands the footprint rectangle by powerMargin and walks that.
	**	Walking a box of the same margin around each cell of the occupy list covers
	**	the same ground for the rectangular footprints these buildings have, and
	**	does not need the footprint's bounds worked out from the list first.
	*/
	for (short const *ptr = list; *ptr != REFRESH_EOL; ptr++) {
		CELL foot = (CELL)(cell + *ptr);

		for (int dy = -CAC_POWER_MARGIN; dy <= CAC_POWER_MARGIN; dy++) {
			for (int dx = -CAC_POWER_MARGIN; dx <= CAC_POWER_MARGIN; dx++) {

				CELL look = (CELL)(foot + dx + dy * MAP_CELL_W);
				if ((unsigned)look >= (unsigned)MAP_CELL_TOTAL) continue;
				if (!Map.In_Radar(look)) continue;

				BuildingClass *b = Map[look].Cell_Building();
				if (b == NULL || !b->IsActive || b->IsInLimbo || b->Class == NULL) continue;
				if (b->House == NULL || b->House->Class == NULL) continue;
				if (b->House->Class->House != house) continue;

				if (CAC_Anchors_Build(b->Class->Type)) return(true);
			}
		}
	}

	return(false);
}


/***********************************************************************************************
 * CAC_Outpost_Site -- Build out from a remote Driller, the way a player does.                    *
 *                                                                                             *
 *    A Driller is exempt from adjacency -- BuildingClass::Passes_Proximity_Check answers it      *
 *    with CAC_Crystal_Site_Ok and nothing else -- so it can be dropped on any crystal field on  *
 *    the map.  And once it is standing, it is a friendly building, so ordinary structures may    *
 *    be placed touching it.  Repeat that and a base walks across the map: a field, a factory     *
 *    beside it, a turret beside that.  It is strong, and it was the player's alone.              *
 *                                                                                             *
 *    The computer could not do it, and the reason is not the Driller.  HouseClass::Which_Zone    *
 *    is the whole of it:                                                                       *
 *                                                                                             *
 *        int distance = Distance(Center, coord);                                                *
 *        if (distance <= Radius) return(ZONE_CORE);                                             *
 *        if (distance > Radius*4) return(ZONE_NONE);                                            *
 *                                                                                             *
 *    Everything past four radii of the base's centre of mass is in no zone at all, and          *
 *    Find_Cell_In_Zone will not look at a cell that is in no zone.  So a remote Driller sat      *
 *    there on its field, legal to build beside and never built beside.                          *
 *                                                                                             *
 *    This looks at the outposts specifically -- the house's own buildings that the zone          *
 *    machinery has written off -- and offers the nearest cell around one that is both legally    *
 *    placeable and adjacent to something friendly.  Both of those are the player's rules,        *
 *    unchanged: the proximity check still has to pass, so this grants no reach that a player     *
 *    does not have.  It only stops the computer refusing to look.                                *
 *                                                                                             *
 *    The AI is a deliberate exception to fidelity here, as elsewhere: CrystAlien's own computer  *
 *    does not expand at all.                                                                    *
 *=============================================================================================*/
CELL CAC_Outpost_Site(HouseClass const *house, BuildingClass *building)
{
	if (house == NULL || house->Class == NULL) return(0);
	if (building == NULL || building->Class == NULL) return(0);
	if (!CAC_Is_CrystAlien_House(house)) return(0);

	short const *list = building->Occupy_List(true);
	if (list == NULL) return(0);

	CELL best = 0;
	int bestrange = 0;

	for (int i = 0; i < Buildings.Count(); i++) {
		BuildingClass const *b = Buildings.Ptr(i);
		if (b == NULL || !b->IsActive || b->IsInLimbo) continue;
		if (b->House != house) continue;

		/*	Only the outposts.  What is inside the base is the zone machinery's job. */
		if (house->Which_Zone(b->Center_Coord()) != ZONE_NONE) continue;

		CELL origin = Coord_Cell(b->Center_Coord());

		for (int dy = -CAC_OUTPOST_REACH; dy <= CAC_OUTPOST_REACH; dy++) {
			for (int dx = -CAC_OUTPOST_REACH; dx <= CAC_OUTPOST_REACH; dx++) {

				CELL cell = (CELL)(origin + dx + dy * MAP_CELL_W);
				if ((unsigned)cell >= (unsigned)MAP_CELL_TOTAL) continue;
				if (!Map.In_Radar(cell)) continue;
				if (!building->Class->Legal_Placement(cell)) continue;

				/*	The rule a player plays by, and it still has to pass. */
				if (!Map.Passes_Proximity_Check(building->Class,
														  house->Class->House, list, cell)) continue;

				int range = ABS(dx) + ABS(dy);
				if (best == 0 || range < bestrange) {
					best = cell;
					bestrange = range;
				}
			}
		}
	}

	if (best != 0) {
		CAC_Log("base", "%s builds %s out at an outpost, %d,%d",
				  (house->Class != NULL) ? house->Class->IniName : "?",
				  building->Class->IniName, Cell_X(best), Cell_Y(best));
	}
	return(best);
}


/***********************************************************************************************
 * CAC_Find_Crystal_Site -- Somewhere for a computer to put its Driller.                        *
 *                                                                                             *
 *    A human picks the field; a computer has to be told how.  Nearest suitable field to the    *
 *    headquarters, searched outward, and bounded -- without a bound the first Driller of the   *
 *    round goes to whichever crystal happens to be lowest-numbered on the map, which can be    *
 *    on somebody else's doorstep.                                                              *
 *                                                                                             *
 *    Ranged off the headquarters rather than off the base's centre of mass, because the        *
 *    centre moves as the base grows and the Driller should be near where the base *is*.        *
 *=============================================================================================*/
COORDINATE CAC_Find_Crystal_Site(HouseClass const * house, StructType type)
{
	if (house == NULL) return(0);

	COORDINATE home = house->Center;
	if (home == 0) {
		for (int i = 0; i < Buildings.Count(); i++) {
			BuildingClass const * b = Buildings.Ptr(i);
			if (b == NULL || !b->IsActive || b->IsInLimbo) continue;
			if (b->House != house) continue;
			home = b->Center_Coord();
			break;
		}
	}
	if (home == 0) return(0);

	int allowed = CAC_Driller_Range(house);

	CELL best = 0;
	int bestrange = 0;

	for (CELL cell = 0; cell < MAP_CELL_TOTAL; cell++) {
		if (Map[cell].Land_Type() != LAND_TIBERIUM) continue;
		if (!CAC_Crystal_Site_Ok(type, cell)) continue;

		int range = (int)::Distance(home, ::Cell_Coord(cell)) / CELL_LEPTON_W;
		if (range > allowed) continue;

		if (best == 0 || range < bestrange) {
			best = cell;
			bestrange = range;
		}
	}

	char const * who = (house->Class != NULL) ? house->Class->IniName : "?";

	if (best == 0) {
		/*
		**	Worth a line: "the computer never builds Drillers" and "there is no field
		**	it is allowed to reach yet" look identical from outside, and the second
		**	one fixes itself a few minutes later.
		*/
		static int _said = -1;
		if (CAC_Logging() && _said != allowed) {
			_said = allowed;
			CAC_Log_Volatile("driller", "%s has no crystal field within %d cells yet", who, allowed);
		}
		return(0);
	}

	CAC_Log("driller", "%s sites a Driller at %d,%d -- %d cells out of %d allowed%s",
			 who, Cell_X(best), Cell_Y(best), bestrange, allowed,
			 CAC_Has_Satellite(house) ? " (uplink: the whole map)" : "");
	return(::Cell_Coord(best));
}


/***********************************************************************************************
 * CAC_Is_Live_Object -- Is this pointer really a living object, or a leftover?                 *
 *                                                                                             *
 *    Three crashes, all the same: 0xC0000005 with the instruction pointer at 0x000000AA -- a  *
 *    jump to an address that is not code.  A null `this` would fault *reading* the vtable near *
 *    zero; jumping to 0xAA means the vtable pointer was read successfully and was rubbish, so  *
 *    the call was made through a pointer to memory that is no longer the object it was.        *
 *                                                                                             *
 *    The stack named the path: DisplayClass::Refresh_Cells -> CellClass::Redraw_Objects ->     *
 *    FootClass::Mark.  Redraw_Objects walks a cell's occupier chain and its three overlapper   *
 *    slots and calls the virtual Mark on each, so one of those is stale.                       *
 *                                                                                             *
 *    It already tests IsActive, and that was not enough, which is itself the clue: a wild      *
 *    pointer lands on memory where that bit reads set as often as not.  What it cannot do is   *
 *    ask whether the pointer is an object at all.                                              *
 *                                                                                             *
 *    This can.  Every object of a given kind lives in a fixed heap, so a real one is a         *
 *    correctly aligned slot inside one of them -- ID gives the index, Raw_Ptr gives back the   *
 *    slot address, and the two agreeing proves both membership and alignment.  Only then is    *
 *    IsActive worth reading, and only then is the vtable worth calling through.                *
 *                                                                                             *
 *    A handful of integer comparisons, which matters: this is on the redraw path.              *
 *                                                                                             *
 *    Note what this deliberately does not do -- it does not fix whatever leaves the stale      *
 *    pointer behind.  It stops the crash and records where the bad pointer was found, which is *
 *    the thing three crash reports could not say.                                              *
 *=============================================================================================*/
#define IN_FIXED_HEAP(heap, ptr)                                                     	do {                                                                             		int _id = (heap).ID((ptr));                                                   		if (_id >= 0 && _id < (heap).Length() &&                                      			 (void *)(heap).Raw_Ptr(_id) == (void *)(ptr)) return(true);              	} while (0)

static bool CAC_In_Any_Heap(void const * ptr)
{
	IN_FIXED_HEAP(Units,     (UnitClass const *)ptr);
	IN_FIXED_HEAP(Infantry,  (InfantryClass const *)ptr);
	IN_FIXED_HEAP(Aircraft,  (AircraftClass const *)ptr);
	IN_FIXED_HEAP(Buildings, (BuildingClass const *)ptr);
	IN_FIXED_HEAP(Terrains,  (TerrainClass const *)ptr);
	IN_FIXED_HEAP(Anims,     (AnimClass const *)ptr);
	IN_FIXED_HEAP(Bullets,   (BulletClass const *)ptr);
	return(false);
}

#undef IN_FIXED_HEAP


bool CAC_Is_Heap_Slot(void const * ptr)
{
	return(ptr != NULL && CAC_In_Any_Heap(ptr));
}


bool CAC_Is_Live_Object(void const * ptr)
{
	if (ptr == NULL) return(false);
	if (!CAC_In_Any_Heap(ptr)) return(false);

	/*
	**	Safe to read now: the pointer is a slot in a heap we own, so the memory is
	**	ours whether or not the object in it is alive.
	*/
	return(((ObjectClass const *)ptr)->IsActive != 0);
}


/***********************************************************************************************
 * CAC_Log_Stale -- Note a cell holding a pointer to something that is not there any more.      *
 *=============================================================================================*/
void CAC_Log_Stale(int cell, char const * where, void const * ptr)
{
	if (!CAC_Logging()) return;

	static long _when = -9999;
	if ((long)Frame - _when < (long)TICKS_PER_SECOND) return;
	_when = (long)Frame;

	/*
	**	Two very different things were being reported as one.
	**	
	**	AbstractClass::Delete_This is a documented hack: `delete this` hands the slot
	**	back to a fixed heap without freeing the memory, and the vtable pointer is
	**	then written back deliberately, with a comment saying it is "for when the C&C
	**	code wants to call a virtual function on an object it just deleted".  So a
	**	cell holding a *recognisable* dead slot is close to normal here and the stock
	**	code already coped with it silently.
	**	
	**	A pointer that is not a slot in any heap is the opposite of normal, and is
	**	the one that produced three identical crashes.  Only that is worth a line.
	*/
	bool known = CAC_Is_Heap_Slot(ptr);

	CAC_Log(known ? "stale" : "wild",
			  "cell %d,%d lists %s in its %s (0x%p)",
			  Cell_X((CELL)cell), Cell_Y((CELL)cell),
			  known ? "a dead but recognisable object" : "SOMETHING THAT IS NOT AN OBJECT",
			  where, ptr);
}


/***********************************************************************************************
 * CAC_Log_Gun_Idle -- Why an armed building is not shooting.                                   *
 *                                                                                             *
 *    Reported as "the Hive is not shooting at all", and everything that can be settled by      *
 *    reading has been: both BK slots carry weapon slot 12 -- a rocket at five hundred leptons  *
 *    -- neither has a turret, so Can_Fire's facing test is skipped, Greatest_Threat is the     *
 *    stock one, and Exit_Object's CrystAlien path leaves no tether and no MISSION_UNLOAD       *
 *    behind.  On paper it should fire.                                                        *
 *                                                                                             *
 *    Which leaves the two things paper cannot answer: what mission it is actually sitting in   *
 *    -- Mission_Guard is the only place a building looks for a target, so a building parked    *
 *    in anything else never looks -- and what Can_Fire says when it does have one.             *
 *                                                                                             *
 *    Also answers the question behind the report.  CrystAlien's engageEnemy has no production  *
 *    gate at all: a building shoots while it is building something, and the only thing that    *
 *    silences one is a brownout.  So if this shows a gun going quiet exactly while its factory *
 *    is busy, that is the conversion's invention and not the source's.                        *
 *=============================================================================================*/
void CAC_Log_Gun_Idle(BuildingClass const * b)
{
	if (!CAC_Logging() || b == NULL || b->Class == NULL) return;
	if (b->Class->Primary == WEAPON_NONE) return;
	if (!CAC_Is_CrystAlien_Type(b->Class->IniName)) return;
	if (b->House == NULL || !b->House->IsHuman) return;	/* the player's, which is what was reported */

	/*
	**	One line per building per five seconds, and only when the answer changes.
	**	This runs from BuildingClass::AI, so it is every building every frame.
	*/
	/*
	**	Only when there is something wrong to say.
	**	
	**	"No target, power on, nothing to shoot" is a turret behaving perfectly, and
	**	it came to two hundred and forty-six lines of an eleven-hundred-line file --
	**	a fifth of the log spent reporting that nothing is the matter.  What was
	**	ever worth knowing is a gun that *has* a target and will not fire, which is
	**	the shape of the complaint this was written for.
	*/
	if (!Target_Legal(b->TarCom)) return;
	if (((BuildingClass *)b)->Can_Fire(b->TarCom, 0) == FIRE_OK) return;

	int code = ((int)b->Mission * 8)
			 + (Target_Legal(b->TarCom) ? 4 : 0)
			 + (b->Is_Something_Attached() ? 2 : 0)
			 + (CAC_Power_Off(b->House) ? 1 : 0);

	static int  _said[64];
	static long _when[64];
	static bool _init = false;
	if (!_init) { _init = true; for (int i = 0; i < 64; i++) { _said[i] = -1; _when[i] = -9999; } }

	int id = Buildings.ID((BuildingClass *)b);
	if (id < 0 || id >= 64) return;
	if (_said[id] == code && (long)Frame - _when[id] < (long)(TICKS_PER_SECOND * 5)) return;
	_said[id] = code;
	_when[id] = (long)Frame;

	FireErrorType err = FIRE_OK;
	if (Target_Legal(b->TarCom)) err = ((BuildingClass *)b)->Can_Fire(b->TarCom, 0);

	CAC_Log_Volatile("gun", "%s: mission %d, %s, power %s, factory %s -> %s",
		b->Class->IniName, (int)b->Mission,
		Target_Legal(b->TarCom) ? "has a target" : "no target",
		CAC_Power_Off(b->House) ? "off" : "on",
		(b->Factory != 0) ? "busy" : "idle",
		!Target_Legal(b->TarCom) ? "nothing to shoot"
			: (err == FIRE_OK)      ? "firing"
			: (err == FIRE_BUSY)    ? "refused: busy"
			: (err == FIRE_RANGE)   ? "refused: out of range"
			: (err == FIRE_FACING)  ? "refused: facing"
			: (err == FIRE_REARM)   ? "refused: rearming"
			: (err == FIRE_AMMO)    ? "refused: no ammo"
			: (err == FIRE_CANT)    ? "refused: cannot"
			: (err == FIRE_ILLEGAL) ? "refused: illegal target"
			: "refused");
}


/***********************************************************************************************
 * CAC_Has_Uplink -- Does this house hold one side's uplink in particular?                     *
 *                                                                                             *
 *    doPrerequisites runs its whole chain once per side -- `for (side in [team, oppo])` -- so  *
 *    the last link of it,                                                                     *
 *                                                                                             *
 *        if (findBuilding("BH_" + side).friend) shortcuts["UK_" + side].active = true;         *
 *                                                                                             *
 *    is reached twice.  Hold a Satellite Uplink and UK_good lights up; hold an Orbital Uplink  *
 *    and UK_evil does; hold both and both do, because they are separate entries with a `max`   *
 *    of one apiece rather than one entry shared.  Two uplinks really are two superweapons.     *
 *=============================================================================================*/
bool CAC_Has_Uplink(HouseClass const * house, bool evil)
{
	if (house == NULL) return(false);
	return((house->BScan & (evil ? STRUCTF_CAC_BHEVIL : STRUCTF_CAC_BHGOOD)) != 0);
}


/***********************************************************************************************
 * CAC_Uplink_Takes_Ion / _Nuke -- Which of the engine's two slots an uplink is put in.         *
 *                                                                                             *
 *    CrystAlien's superweapon is one thing.  launchSuperweapon(x, y) takes a position and      *
 *    nothing else -- no side, no type -- so the Astro and the Alien uplink fire the same shot  *
 *    and always have.  What differs here is only where the engine can keep it, because it      *
 *    holds one superweapon of each *kind* rather than a list.                                  *
 *                                                                                             *
 *    In the conversion both kinds are free, so the first uplink takes the ion cannon's slot    *
 *    and a second one -- a player holding both -- takes the nuke's, which is how two uplinks   *
 *    come to be two superweapons the way doPrerequisites has it.                               *
 *                                                                                             *
 *    In the supplemental the slots are spoken for, and an uplink is meant to be an addition    *
 *    to what GDI and Nod already field rather than a replacement for it.  So it takes whichever *
 *    of the two its owner is not using: a GDI base with an Advanced Communications Center      *
 *    keeps its ion cannon and the uplink goes in the nuke's slot; a Nod base with a Temple     *
 *    keeps its nuke and the uplink goes in the ion cannon's.  Either way the commander ends up *
 *    with two, which is the point.                                                             *
 *                                                                                             *
 *    Only the slot is borrowed.  Both fire CrystAlien's own blast, so nothing about holding    *
 *    the nuke's slot makes an Astro uplink launch a warhead.                                   *
 *=============================================================================================*/
bool CAC_Uplink_Takes_Ion(HouseClass const * house)
{
	if (house == NULL) return(false);
	if (!CAC_Has_Uplink(house, false) && !CAC_Has_Uplink(house, true)) return(false);

#ifndef CAC_SUPPLEMENTAL
	return(true);
#else
	return((house->BScan & STRUCTF_EYE) == 0);
#endif
}


bool CAC_Uplink_Takes_Nuke(HouseClass const * house)
{
	if (house == NULL) return(false);
	if ((house->BScan & STRUCTF_TEMPLE) != 0) return(false);

#ifndef CAC_SUPPLEMENTAL
	return(CAC_Has_Uplink(house, false) && CAC_Has_Uplink(house, true));
#else
	if (!CAC_Has_Uplink(house, false) && !CAC_Has_Uplink(house, true)) return(false);

	/*
	**	The nuke's slot is for the one that could not have the ion cannon's -- either
	**	because the owner is already using it, or because they hold both uplinks.
	*/
	return((house->BScan & STRUCTF_EYE) != 0 ||
		   (CAC_Has_Uplink(house, false) && CAC_Has_Uplink(house, true)));
#endif
}


bool CAC_Superweapon_Is_Ours(HouseClass const * house)
{
	if (house == NULL) return(false);
#ifndef CAC_SUPPLEMENTAL
	return(true);
#else
	return(CAC_Has_Satellite(house) && (house->BScan & STRUCTF_EYE) == 0);
#endif
}


bool CAC_Has_Satellite(HouseClass const *house)
{
	if (house == NULL) return(false);
	return((house->BScan & CAC_SATELLITE_SCAN) != 0);
}


/***********************************************************************************************
 * CAC_Crate_Reachable -- Can anything walk to this cell, or is it a sealed pocket?              *
 *                                                                                             *
 *    A CrystAlien mountain is a ring of wall around a floor a unit can stand on, and a ring     *
 *    with no gap in it is a room with no door.  Eclipse has ten of them -- 137 cells, five per  *
 *    cent of the walkable map -- and they only became sealed when the last of the holes in      *
 *    those walls was closed.  Before that the gaps were a bug and a way in at the same time.    *
 *                                                                                             *
 *    Place_Random_Crate asks only that a cell is clear and unoccupied, which every one of       *
 *    those 137 is.  So about one crate in twenty landed somewhere nothing could ever reach it:  *
 *    "an untouchable crate", visible, marked on the radar, and permanent.                       *
 *                                                                                             *
 *    CrystAlien never has this problem because it does not scatter pickups -- every capsule and *
 *    crystal box in the game is placed by hand in a level, on ground the designer could see.    *
 *    Scattering them is this conversion's own idea, so the reachability test is its own job.    *
 *                                                                                             *
 *    Bounded rather than exhaustive.  The question is not "which region is this" but "is this   *
 *    region big enough to be the map", and the largest pocket on Eclipse is 24 cells.  Stopping *
 *    at ninety-six answers it without walking four thousand cells per attempt.                  *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Credit_Harvest -- Pay a house for crystal, and let the scoreboard know it happened.       *
 *                                                                                             *
 *    A CrystAlien headquarters has no storage.  cashUp adds to the cash and that is the end of  *
 *    it, so this conversion pays with Refund_Money rather than Harvested -- Harvested puts the  *
 *    load in a silo first and clamps it to the house's capacity, and a capacity of nothing      *
 *    threw every delivery away.                                                                 *
 *                                                                                             *
 *    But Harvested does two things, and only the first was the problem.  The second is          *
 *    HarvestedCredits, which is the only record anything keeps of how much was ever gathered:   *
 *    ScoreClass reads it directly for the harvested line and again for the efficiency rating.   *
 *    Paying with Refund_Money alone left it at nought, so a round spent mining reported no       *
 *    resources gathered at all.                                                                 *
 *                                                                                             *
 *    So: the money uncapped, the tally kept.                                                    *
 *=============================================================================================*/
void CAC_Credit_Harvest(HouseClass *house, int amount)
{
	if (house == NULL || amount <= 0) return;

	house->Refund_Money(amount);
	house->HarvestedCredits += amount;
}


/***********************************************************************************************
 * CAC_Crate_Ground_Ok -- Is this cell ground a crate can sit on and be walked to?               *
 *                                                                                             *
 *    Is_Generally_Clear ends on `Ground[Land_Type()].Build`, which reads the cell's cached      *
 *    land type -- and that is not the same question as what the cell is drawn as.               *
 *                                                                                             *
 *    Read_Binary is why.  A cell whose icon is not in its template's icon map is quietly reset: *
 *                                                                                             *
 *        if ((temp.TIcon >= (ttype.Width * ttype.Height)) || (map[temp.TIcon] == -1)) {         *
 *           temp.TIcon = 0;                                                                    *
 *           temp.TType = TEMPLATE_NONE;                                                        *
 *        }                                                                                     *
 *                                                                                             *
 *    TEMPLATE_NONE reads as CLEAR1's land type, so the cell becomes buildable ground as far as  *
 *    every test in the engine is concerned, whatever the map meant it to be.                    *
 *                                                                                             *
 *    One round placed eight crates: two landed on SHORE18 cells and one on SLOPE12 -- water and *
 *    a cliff.  The two on water were collected, because the engine thought they were clear.  The *
 *    one in the cliff never was, and that is the report: a crate you cannot pick up, sitting on  *
 *    something solid.                                                                          *
 *                                                                                             *
 *    So ask the template rather than the cell.  A template whose own land type is anything but  *
 *    clear is drawn as something other than open ground, and a crate has no business on it --    *
 *    whether or not the engine happens to agree that the cell is passable.                      *
 *                                                                                             *
 *    This is the same trap the terrain conversion already carries a note about: a template's     *
 *    land type and its artwork are two different things, and SH31 is LAND_ROCK while drawing a   *
 *    shoreline.                                                                                *
 *=============================================================================================*/
bool CAC_Crate_Ground_Ok(CELL cell)
{
	if ((unsigned)cell >= MAP_CELL_TOTAL) return(false);

	TemplateType tt = Map[cell].TType;

	/*	Nothing laid here, or plain clear terrain: open ground either way. */
	if (tt == TEMPLATE_NONE || tt == TEMPLATE_CLEAR1) return(true);

	TemplateTypeClass const &t = TemplateTypeClass::As_Reference(tt);

	/*
	**	Both, because the icon decides which one this cell uses and the alternate
	**	exists precisely so that part of a cliff can be walked on.  A crate should
	**	not be on either part of it.
	*/
	return(t.Land == LAND_CLEAR && t.AltLand == LAND_CLEAR);
}


bool CAC_Crate_Reachable(CELL cell)
{
	if ((unsigned)cell >= MAP_CELL_TOTAL) return(false);

	enum { ENOUGH = 96 };

	static CELL queue[ENOUGH * 4];
	static bool seen[MAP_CELL_TOTAL];
	memset(seen, 0, sizeof(seen));

	int head = 0, tail = 0, count = 0;
	queue[tail++] = cell;
	seen[cell] = true;

	while (head < tail && count < ENOUGH) {
		CELL at = queue[head++];
		count++;

		static int const _step[4] = {-MAP_CELL_W, MAP_CELL_W, -1, 1};
		for (int i = 0; i < 4; i++) {
			CELL next = (CELL)(at + _step[i]);
			if ((unsigned)next >= MAP_CELL_TOTAL || seen[next]) continue;

			/*
			**	Walkable by something with tracks, which is the widest of the
			**	ground movement classes and so the least likely to call a passage
			**	closed that is merely narrow.
			*/
			if (Ground[Map[next].Land_Type()].Cost[SPEED_TRACK] == 0) continue;

			seen[next] = true;
			if (tail < (int)(sizeof(queue) / sizeof(queue[0]))) queue[tail++] = next;
		}
	}

	return(count >= ENOUGH);
}


/***********************************************************************************************
 * CAC_Satellite_Shroud -- Lift the shroud while an uplink stands, and put it back when it does not.
 *                                                                                             *
 *    Level.handle recomputes this every frame, at the end of the pass that walks the buildings:*
 *                                                                                             *
 *        this.arena.shroud.active = !(_loc7_ && !this.powerLow);                               *
 *                                                                                             *
 *    -- where `_loc7_` is "any live friendly building with isSatellite", which is BH_good and  *
 *    BH_evil and nothing else.  Shroud.handle then does                                        *
 *                                                                                             *
 *        this.MC._visible = this.parent.radar.MCshroud._visible = this.active;                 *
 *                                                                                             *
 *    so `active` only hides the overlay.  The array behind it, which Shroud.reveal fills in as *
 *    things actually go and look, is never touched.  Lose the uplink -- or let the power drop --*
 *    and the overlay comes back over everything that was never really explored, while every    *
 *    tile a unit has stood near stays open.                                                    *
 *                                                                                             *
 *    This was one-way here, on the reasoning that Tiberian Dawn can reveal a map and has no way *
 *    to put the shroud back.  It has one: CellClass::Set_Mapped takes a bool, and passing false *
 *    un-maps the cell for that house.  So the cells the uplink opened are remembered, and the   *
 *    ones still unexplored are closed again when it falls.                                     *
 *                                                                                             *
 *    Only a human house has a shroud to lift -- DisplayClass::Map_Cell returns early for anyone *
 *    else unless allies are sharing vision -- so this follows the same rule on the way back.    *
 *=============================================================================================*/
static unsigned int CACUplinkOpened[MAP_CELL_TOTAL];
static bool CACUplinkLifting = false;


/*
**	A cell the uplink opened stops being the uplink's the moment something goes
**	and looks at it for real.  Called from DisplayClass::Map_Cell.
*/
void CAC_Sighted_For_Real(CELL cell, HouseClass const * house)
{
	if (CACUplinkLifting) return;
	if (house == NULL || house->Class == NULL) return;
	if ((unsigned)cell >= (unsigned)MAP_CELL_TOTAL) return;
	CACUplinkOpened[cell] &= ~(1u << (int)house->Class->House);
}


void CAC_Reset_Uplink_Shroud(void)
{
	for (int i = 0; i < MAP_CELL_TOTAL; i++) CACUplinkOpened[i] = 0;
}


/***********************************************************************************************
 * CAC_Feather_Shroud -- Put back the soft edge that a restored shroud has no way to grow.      *
 *                                                                                             *
 *    Tiberian Dawn's shroud has two flags per cell and the edge is made of the difference      *
 *    between them.  Is_Mapped means explored and drawn; Is_Visible without Is_Mapped is the    *
 *    half-lit ring around it, and it is the only state that carries a shape:                   *
 *                                                                                             *
 *        shroud_entry.ShadowIndex = -1;                                                        *
 *        if (!shroud_entry.IsMapped) {                                                         *
 *            if (shroud_entry.IsVisible) {                                                     *
 *                shroud_entry.ShadowIndex = (char) Map.Cell_Shadow(cell, PlayerPtr);           *
 *                                                                                             *
 *    Twelve shapes -- four cardinal edges, four corners, four diagonals -- and they are what   *
 *    makes a C&C shroud edge read as a soft diagonal rather than a wall of squares.  A cell    *
 *    that is neither mapped nor visible has no shape at all and is drawn solid.                *
 *                                                                                             *
 *    Exploring grows that ring as a side effect: the second half of DisplayClass::Map_Cell     *
 *    walks the eight neighbours of every cell it maps and marks each unmapped one visible --    *
 *    or maps it outright when Cell_Shadow comes back -1, which is its way of saying no shape    *
 *    can express that combination.                                                             *
 *                                                                                             *
 *    Nothing does the equivalent when a shroud is *restored*.  CAC_Satellite_Shroud clears both *
 *    flags on every cell the uplink opened, and the Look() pass afterwards only rebuilds the    *
 *    ring around things that can actually see -- so the long border between ground the player   *
 *    had genuinely explored and ground that has just gone dark had no half-lit cells anywhere   *
 *    along it.  Mapped cell, then solid black cell, and the join is the cell grid itself.       *
 *    That is the blockiness: not a different shroud, an edge with its edging missing.           *
 *                                                                                             *
 *    So run Map_Cell's own neighbour pass over the whole map afterwards.  Same rule, same       *
 *    routine for the -1 repair, so the restored edge is built the way an explored one is.       *
 *                                                                                              *
 *    The repair credits those few cells to the house through CAC_Sighted_For_Real, so a later   *
 *    uplink cannot take them back.  That is correct rather than a leak: a cell whose edge you   *
 *    can see the shape of has been seen, and it is the same handful of cells vanilla would      *
 *    have mapped while exploring to that border in the first place.                             *
 *=============================================================================================*/
static void CAC_Feather_Shroud(HouseClass *house)
{
	if (house == NULL) return;

	for (CELL cell = 0; cell < MAP_CELL_TOTAL; cell++) {
		if (!Map[cell].Is_Mapped(house)) continue;

		int xx = Cell_X(cell);
		for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir++) {

			CELL c = Adjacent_Cell(cell, dir);
			if ((unsigned)c >= MAP_CELL_TOTAL) continue;

			/*	The wrap guard Map_Cell uses, for the same reason. */
			int xdiff = Cell_X(c) - xx;
			if (ABS(xdiff) > 1) continue;
			if (Map[c].Is_Mapped(house)) continue;

			int shadow = Map.Cell_Shadow(c, house);
			if (shadow == -1) {
				/*
				**	No shape can draw this one, so it is mapped instead -- which is
				**	what Map_Cell does, and why it recurses.  Calling it rather than
				**	setting the flag by hand keeps the repair identical, neighbours
				**	and all.
				*/
				Map.Map_Cell(c, house, false);
			} else if (shadow != -2) {
				Map[c].Set_Visible(house);
				Map[c].Redraw_Objects();
			}
		}
	}
}


void CAC_Satellite_Shroud(HouseClass *house)
{
	if (house == NULL || house->Class == NULL) return;
	if (!CAC_Is_CrystAlien_House(house)) return;

	bool want = CAC_Has_Satellite(house) && !CAC_Power_Low(house);
	if (want == (house->CACRevealed != false)) return;

	unsigned int bit = 1u << (int)house->Class->House;

	if (want) {
		CACUplinkLifting = true;
		for (CELL cell = 0; cell < MAP_CELL_TOTAL; cell++) {
			if (Map[cell].Is_Mapped(house)) continue;
			if (Map.Map_Cell(cell, house, true)) {
				CACUplinkOpened[cell] |= bit;
			}
		}
		CACUplinkLifting = false;
		CAC_Log("uplink", "shroud lifted for house %d", (int)house->Class->House);
	} else {
		int closed = 0;
		for (CELL cell = 0; cell < MAP_CELL_TOTAL; cell++) {
			if ((CACUplinkOpened[cell] & bit) == 0) continue;
			CACUplinkOpened[cell] &= ~bit;
			Map[cell].Set_Mapped(house, false);
			Map[cell].Set_Visible(house, false);

			/*
			**	Map_Cell recurses to allies when vision is shared, so closing has
			**	to reach the same houses or their half of it would stay open.
			*/
			if (ShareAllyVisibility) {
				HousesType first = (GameToPlay == GAME_NORMAL)
										 ? HOUSE_FIRST : HOUSE_MULTI1;
				for (HousesType h = first; h < HOUSE_COUNT; h++) {
					HouseClass *other = HouseClass::As_Pointer(h);
					if (other == NULL || !other->IsActive) continue;
					if (other == house || !house->Is_Ally(other)) continue;
					Map[cell].Set_Mapped(other, false);
					Map[cell].Set_Visible(other, false);
				}
			}
			closed++;
		}
		/*
		**	And then let everything look again.
		**
		**	Closing a cell clears Is_Visible as well as Is_Mapped, and nothing
		**	re-lights it until whatever can see it next calls Look() -- which for a
		**	building or a unit standing still may be a long time.  So the shroud
		**	came back over the player's own base, which is the part that makes no
		**	sense to look at.
		**
		**	One pass over the house's objects puts back exactly what it can really
		**	see, and Map_Cell credits those cells to the house on the way through
		**	(see DisplayClass::Map_Cell), so they are not the uplink's to take away
		**	a second time.
		*/
		for (int i = 0; i < Buildings.Count(); i++) {
			BuildingClass *b = Buildings.Ptr(i);
			if (b != NULL && b->IsActive && !b->IsInLimbo && b->House == house) b->Look();
		}
		for (int i = 0; i < Units.Count(); i++) {
			UnitClass *u = Units.Ptr(i);
			if (u != NULL && u->IsActive && !u->IsInLimbo && u->House == house) u->Look();
		}
		for (int i = 0; i < Infantry.Count(); i++) {
			InfantryClass *f = Infantry.Ptr(i);
			if (f != NULL && f->IsActive && !f->IsInLimbo && f->House == house) f->Look();
		}
		for (int i = 0; i < Aircraft.Count(); i++) {
			AircraftClass *a = Aircraft.Ptr(i);
			if (a != NULL && a->IsActive && !a->IsInLimbo && a->House == house) a->Look();
		}

		/*
		**	And then feather the edge, which is the one thing a restored shroud
		**	cannot grow for itself.  See CAC_Feather_Shroud.
		*/
		CAC_Feather_Shroud(house);

		Map.Flag_To_Redraw(true);
		CAC_Log("uplink", "shroud back over %d cell(s) for house %d",
				  closed, (int)house->Class->House);
	}
	house->CACRevealed = want;
}


/***********************************************************************************************
 * CAC_Queue_Of_Host -- The production queue that belongs to a building.                       *
 *                                                                                             *
 *    The counterpart to CAC_Factory_Index, which answers for the thing being built.  This      *
 *    answers for the building doing the building, which is what is wanted when that building   *
 *    is about to stop existing.                                                                *
 *                                                                                             *
 * OUTPUT:  a queue index, or -1 for a building that hosts nothing.                             *
 *=============================================================================================*/
int CAC_Queue_Of_Host(StructType building)
{
	if (!CAC_Builds_Any(building, RTTI_UNITTYPE) &&
		!CAC_Builds_Any(building, RTTI_INFANTRYTYPE) &&
		!CAC_Builds_Any(building, RTTI_AIRCRAFTTYPE)) {
		return(-1);
	}

	int code = CAC_Code_Of(RTTI_BUILDINGTYPE, (int)building);
	if (code == CAC_CODE_NONE) return(-1);
	return(((code & ~CAC_CODE_EVIL) * 2) + ((code & CAC_CODE_EVIL) ? 1 : 0));
}


/***********************************************************************************************
 * CAC_Stock_Sound -- What a CrystAlien object should make of one of the engine's noises.      *
 *                                                                                             *
 *    Tiberian Dawn has an effect for a good many moments CrystAlien passes over in silence,    *
 *    and they are unmistakably its own: the rumble as a foundation goes down, the slam as a    *
 *    structure lands, the till-roll of a refund, the big explosion when a building goes up.    *
 *    Played on a CrystAlien object they are simply the wrong game -- which is what "the        *
 *    purchase sound when a building is placed" and "the construction sound when one is         *
 *    destroyed" both were.                                                                     *
 *                                                                                             *
 *    CrystAlien answers at a different moment: INT_construction when the item is *bought*,     *
 *    not when it is placed, and INT_constructioncomplete_building when it finishes.  Both are  *
 *    already played where they belong, so these are silenced rather than moved.                *
 *                                                                                             *
 * OUTPUT:  the sound to play instead, VOC_NONE for silence, or the original if this is not     *
 *          a CrystAlien object.                                                                *
 *=============================================================================================*/
VocType CAC_Stock_Sound(TechnoClass const *object, VocType voc)
{
	if (object == NULL || !CAC_Owns_Its_Sounds(object)) return(voc);

	switch (voc) {

		/*
		**	The moments the source has nothing for.
		*/
		case VOC_CONSTRUCTION:		// the foundation rumble as a build-up starts
		case VOC_SLAM:					// a structure setting down
		case VOC_XPLOBIG4:			// a building going up
		case VOC_CRUMBLE:			// and coming down
		case VOC_CASHTURN:			// the till-roll of a refund
		case VOC_BUTTON:				// the repair-mode click
		case VOC_LASER_POWER:		// an Obelisk charging, which nothing here does
			return(VOC_NONE);

		default:
			return(voc);
	}
}


/***********************************************************************************************
 * CAC_Plate_For -- The baseplate a structure of this side, size and owner stands on.          *
 *                                                                                             *
 *    Building() draws the plate as `team + "_1"` or `team + "_2"` -- one tile or two, light    *
 *    for the Astros and dark for the Aliens -- and that is the whole choice, because           *
 *    CrystAlien never puts more than two sides on a map.  A skirmish here can hold six, and    *
 *    two Astro players then stand on identical plates, which is the complaint: a base reads    *
 *    as somebody's by its colour and the plate had none.                                      *
 *                                                                                             *
 *    The client colours an object by its owner but leaves terrain decals alone, and a plate    *
 *    is a decal, so the tint is baked -- one plate per side, size and player colour.           *
 *                                                                                             *
 * OUTPUT:  the smudge to lay, falling back to the untinted pair if the owner has no colour.    *
 *=============================================================================================*/
SmudgeType CAC_Plate_For(int size, bool evil, HousesType owner)
{
	static struct {
		int				Size;
		bool				Evil;
		PlayerColorType	Colour;
		SmudgeType		Plate;
	} const _plates[] = {
#define CAC_PLATE_PICK
#include "CACPLATES.INC"
#undef CAC_PLATE_PICK
	};

	HouseClass const *house = HouseClass::As_Pointer(owner);
	if (house != NULL && house->Class != NULL) {

		/*
		**	The house's colour, not its type's.
		**
		**	HouseClass::RemapColor is what the player was actually dealt -- Init_Data
		**	writes it from the lobby.  HouseTypeClass::RemapColor is the default that
		**	seeds it, and every CrystAlien house shares one, so reading the type gave
		**	all six players the same plate and all six the same pizza.
		*/
		PlayerColorType colour = house->RemapColor;
		for (int i = 0; i < (int)(sizeof(_plates) / sizeof(_plates[0])); i++) {
			if (_plates[i].Size == size && _plates[i].Evil == evil &&
				_plates[i].Colour == colour) {
				return(_plates[i].Plate);
			}
		}
	}

	if (size < 2) return(evil ? SMUDGE_CAC_PAD1_EVIL : SMUDGE_CAC_PAD1_GOOD);
	return(evil ? SMUDGE_CAC_PAD2_EVIL : SMUDGE_CAC_PAD2_GOOD);
}


/***********************************************************************************************
 * CAC_Host_Suggestion -- What a CrystAlien host building should start building next.          *
 *                                                                                             *
 *    The computer builds out of a structure by asking Suggest_New_Object for the one category  *
 *    that structure's ToBuild names.  A CrystAlien building can host more than one: the        *
 *    Command Base builds structures *and* the Miner, the Training Camp builds infantry and the *
 *    JetPack Explorer, the Vehicle Factory builds vehicles and the Dropship.  ToBuild has room *
 *    for one of those, so everything else it hosts was never suggested -- which is why a       *
 *    computer player fielded a single Miner and then nothing at all.                           *
 *                                                                                             *
 *    So ask about each category the building actually hosts, and take the first answer.  The   *
 *    order follows the engine's own preference: whatever ToBuild says first, then the rest.    *
 *                                                                                             *
 * OUTPUT:  the type to begin, or NULL if this building has nothing to suggest.                 *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Host_Allows -- May this building be the one that makes this type?                        *
 *                                                                                             *
 *    Two rules, and conflating them broke every computer base in the game.                     *
 *                                                                                             *
 *    A type with a `home` in UnitStats is built there and nowhere else -- CACHOSTS.INC is that  *
 *    list, and it is why a Speeder has no business coming out of a Mothership.  A type without  *
 *    one has no such restriction and uses Tiberian Dawn's own rule: any factory of the matching *
 *    kind will do.                                                                             *
 *                                                                                             *
 *    CAC_Hosts_Unit answers the first question and returns false for anything not in the table, *
 *    which is the right answer to "is this its home" and the wrong answer to "may it build it". *
 *    Asking it directly refused every type that has no home at all, so a computer house built    *
 *    its Miner -- which does have one -- and then nothing else for the rest of the game.         *
 *=============================================================================================*/
static bool CAC_Host_Allows(StructType building, RTTIType rtti, TechnoTypeClass const *type)
{
	if (type == NULL) return(false);

	int index = CAC_Type_Index_Of_Type(type);
	if (!CAC_Has_Host(rtti, index)) return(true);	/* no home, so anywhere of the right kind */

	return(CAC_Hosts_Unit(building, rtti, index));
}


TechnoTypeClass const *CAC_Host_Suggestion(HouseClass *house, StructType building, RTTIType tobuild)
{
	if (house == NULL) return(NULL);

	static RTTIType const _kinds[] = {
		RTTI_UNITTYPE, RTTI_INFANTRYTYPE, RTTI_AIRCRAFTTYPE
	};

	/*
	**	Whatever the computer has already decided it wants, if this is the building
	**	that makes it.
	**
	**	The Command Base is both the construction yard and where the Miner comes
	**	from, and asking it about its ToBuild first means asking about structures
	**	first -- and there is always another structure worth having, so the Miner
	**	was never reached and the base never earned anything.  AI_Unit and
	**	AI_Infantry have already picked something by the time this is called, so
	**	ask about that before falling back to the plan.
	*/
	if (house->BuildUnit != UNIT_NONE &&
		CAC_Hosts_Unit(building, RTTI_UNITTYPE, (int)house->BuildUnit)) {
		TechnoTypeClass const *pick = house->Suggest_New_Object(RTTI_UNITTYPE);
		if (pick != NULL) return(pick);
	}
	if (house->BuildInfantry != INFANTRY_NONE &&
		CAC_Hosts_Unit(building, RTTI_INFANTRYTYPE, (int)house->BuildInfantry)) {
		TechnoTypeClass const *pick = house->Suggest_New_Object(RTTI_INFANTRYTYPE);
		if (pick != NULL) return(pick);
	}
	if (house->BuildAircraft != AIRCRAFT_NONE &&
		CAC_Hosts_Unit(building, RTTI_AIRCRAFTTYPE, (int)house->BuildAircraft)) {
		TechnoTypeClass const *pick = house->Suggest_New_Object(RTTI_AIRCRAFTTYPE);
		if (pick != NULL) return(pick);
	}

	/*
	**	Then what the structure was going to be asked about anyway -- but only if
	**	this is the building that makes it.
	**
	**	The three branches above each ask CAC_Hosts_Unit before handing a type
	**	over.  These two did not: they asked whether the building makes units of
	**	that *kind* and then passed whatever the house happened to want next.  A
	**	CrystAlien host makes one thing -- CACHOSTS.INC is the whole rule, and the
	**	Alien Mothership's entry is the Crystal Extractor -- so "does it build
	**	vehicles" is true of it and "does it build Speeders" is not.
	**
	**	That is a Speeder coming out of a Mothership, which is what was reported.
	**	It is not a prerequisite bypass -- Suggest_New_Object only ever offers
	**	something Can_Build allowed, so the house really could build one -- but it
	**	is the host rule going out of the window, and from outside the two look
	**	identical.
	*/
	if (tobuild != RTTI_NONE) {
		TechnoTypeClass const *first = house->Suggest_New_Object(tobuild);
		if (first != NULL && CAC_Host_Allows(building, tobuild, first)) {
			return(first);
		}
	}

	for (int i = 0; i < (int)(sizeof(_kinds) / sizeof(_kinds[0])); i++) {
		if (_kinds[i] == tobuild) continue;
		if (!CAC_Builds_Any(building, _kinds[i])) continue;

		TechnoTypeClass const *pick = house->Suggest_New_Object(_kinds[i]);
		if (pick != NULL && CAC_Host_Allows(building, _kinds[i], pick)) {
			return(pick);
		}
	}
	return(NULL);
}


/***********************************************************************************************
 * CAC_Is_CrystAlien_House -- Is this player playing CrystAlien's side of the game?             *
 *                                                                                             *
 *    In the total conversion every faction is CrystAlien's, so the answer is always yes.  In   *
 *    the supplemental build GDI and Nod are still themselves, and a player counts as           *
 *    CrystAlien's once they hold one of its structures -- which is the point at which they     *
 *    have gone down that branch of the tree and should be hearing its interface.               *
 *=============================================================================================*/
bool CAC_Is_CrystAlien_House(HouseClass const *house)
{
	/*
	**	Nobody is playing the civilians.
	**
	**	In a total conversion every side is CrystAlien, so this used to answer
	**	true for anything with a pulse -- and the AI routines guard on it, so
	**	Neutral and Special were being run as opponents.  The log has both of
	**	them calling for raids and finding, reasonably enough, nobody free.
	**	Wasted work at best, and a house that is meant to hold nothing being
	**	handed a strategy at worst.
	*/
	if (house != NULL) {
		HousesType which = house->Class ? house->Class->House : HOUSE_NONE;
		if (which == HOUSE_NEUTRAL || which == HOUSE_JP) return(false);
	}

#ifndef CAC_SUPPLEMENTAL
	house;
	return(true);
#else
	if (house == NULL) return(false);
	for (int i = 0; i < (int)(sizeof(CACCodes) / sizeof(CACCodes[0])); i++) {
		if (CACCodes[i].RTTI != RTTI_BUILDINGTYPE) continue;
		if ((house->BScan & CAC_Scan_Bit(CACCodes[i].Type)) != 0) return(true);
	}
	return(false);
#endif
}


/***********************************************************************************************
 * CAC_Announce -- CrystAlien's answer to one of the engine's spoken lines.                    *
 *                                                                                             *
 *    CrystAlien has no announcer.  Where Tiberian Dawn says "Construction complete" it plays   *
 *    a short interface sound, and the whole set was extracted and shipped with everything      *
 *    else -- and then never played, because nothing in the conversion asked for it.            *
 *                                                                                             *
 *    Only the lines with a counterpart are answered here.  The rest keep the engine's voice:   *
 *    silencing "Mission accomplished" because the source has no equivalent would take          *
 *    something away rather than put CrystAlien's own in its place.                             *
 *                                                                                             *
 * OUTPUT:  the sound to play instead, or VOC_NONE to leave the line alone.                    *
 *=============================================================================================*/
VocType CAC_Announce(VoxType voice)
{
	switch (voice) {
		/*
		**	Not the power lines, and not "base under attack".
		**
		**	Level.think raises both klaxons off its own power state, on the frame it
		**	changes and only then:
		**
		**	    if (powerLow  && !wasLow) { showMessage(int_power_low); play("INT_powerwarning"); }
		**	    if (powerOff  && !wasOff) { showMessage(int_power_off); play("INT_warning"); }
		**
		**	-- so INT_warning is the sound of the lights going out, not of being shot
		**	at, and the conversion had it answering VOX_BASE_UNDER_ATTACK, which is a
		**	different event entirely.  And VOX_LOW_POWER fires off Power_Fraction(),
		**	which is the running total that drifts rather than the sum the guns obey.
		**
		**	Both are raised in CAC_Power_Klaxon now, from CAC_Power_State, so they
		**	say what they mean.  CrystAlien has no sound for a base under attack.
		*/
		case VOX_NO_CASH:				return(VOC_CAC_INT_NOFUNDS);
		case VOX_CONSTRUCTION:			return(VOC_CAC_INT_CCBLDG);
		case VOX_UNIT_READY:			return(VOC_CAC_INT_CCUNIT);
		/* the options sounds are counted, not inferred -- CAC_Options_Changed */

		case VOX_NEED_MO_MONEY:		return(VOC_CAC_INT_NOFUNDS);
		/* the options sounds are counted, not inferred -- CAC_Options_Changed */
		default:						return(VOC_NONE);
	}
}


/***********************************************************************************************
 * CAC_Order_Sound -- The cue that answers an order.                                           *
 *                                                                                             *
 *    Named INT_cursor_target and INT_cursor_move, which is misleading: neither has anything to *
 *    do with the cursor moving about.  Control.doRelease plays them when an order is actually  *
 *    given, once for the whole selection:                                                      *
 *                                                                                             *
 *        if (this.cursorState == "target") {                                                   *
 *            for (i in selected) if (selected[i].stats.weapon) selected[i].target = activeTarget;
 *            sfx.play("INT_cursor_target");                                                    *
 *        }                                                                                     *
 *        if (this.cursorState == "repair") {                                                   *
 *            ... selected[i].stats.repair -> target = activeTarget ...                          *
 *            ... or a carriable one boarding a carrier ...                                      *
 *            sfx.play("INT_cursor_target");                                                    *
 *        }                                                                                     *
 *        ... a plain move ...                                                                   *
 *        sfx.play("INT_cursor_move");                                                          *
 *                                                                                             *
 *    So one cue answers anything aimed at a *thing* -- an attack, an Engineer sent to capture   *
 *    or repair, a unit sent to board a transport -- and the other answers being sent to a       *
 *    place.  Which is why the same sound comes back whether the order was a Saboteur walking    *
 *    into a Mothership or an Astro Fighter told what to shoot: the boomerangs will not pick a   *
 *    target for themselves, so telling one to attack is a targeted order like any other.        *
 *                                                                                             *
 *    They were both written off as the client's during the sweep of the interface set, on the   *
 *    strength of the names.  Only INT_cursor_drag really is the client's -- that one is the     *
 *    selection box being dragged, which a DLL never sees.                                       *
 *                                                                                             *
 *    Tiberian Dawn calls Player_Assign_Mission once per selected object, so this arrives as     *
 *    many times as there are units; the one-per-second gate in CAC_Interface_Sound is exactly   *
 *    what makes that come out as the single cue the source plays.                               *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Can_Attack -- Has this thing anything to attack with?                                    *
 *                                                                                             *
 *    TechnoClass::What_Action offers ACTION_ATTACK on the strength of the *target* being a     *
 *    legal one and the attacker being able to reach it.  It never asks whether the attacker    *
 *    is armed, which is how a reindeer comes to be offered a target.                           *
 *                                                                                             *
 *    That is worse than a wrong cursor.  A group takes the highest-priority action any of its  *
 *    members offers and ACTION_ATTACK outranks ACTION_CAPTURE, so one unarmed reindeer in the  *
 *    selection turns a Saboteur's infiltration into an attack order that nothing can carry     *
 *    out -- the reindeer were meant to be the distraction, not the veto.                       *
 *                                                                                             *
 *    Only asked of CrystAlien's own, so the engine's unarmed types keep whatever they did.     *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Drawn_Rows_Above -- How many rows of artwork stand above the ground a structure covers.  *
 *                                                                                             *
 *    The overlap list is generated from CrystAlien's own `width` and `height`, with the height *
 *    halved because its tiles are 96 by 48 and ours are square -- so it describes the *ground*,*
 *    and a Training Camp comes out two cells across and one deep while being drawn two cells   *
 *    tall.  The rows above appear in no list the engine keeps.                                 *
 *                                                                                             *
 *    Three places need this and each had worked it out for itself: the object export, so the   *
 *    client knows what the cursor is over; CAC_Building_Over, so Close_Object can find a       *
 *    structure under a cell that holds nothing; and the per-cell action map DLLInterface builds*
 *    every frame.  The third had been left doing a plain walk of the overlap list with no rows *
 *    added, so the client would let a player point at the top half of a Breeding Pit and the   *
 *    action map, asked what was there, answered with the bare ground.  Attacking it worked --  *
 *    that goes through Close_Object -- while capturing it and repairing it did not, which is   *
 *    exactly the shape of the report.                                                          *
 *                                                                                             *
 *    One sum, one place, so they cannot drift again.                                           *
 *=============================================================================================*/
int CAC_Drawn_Rows_Above(BuildingClass const * b)
{
	if (b == NULL || b->Class == NULL) return(0);
	if (!CAC_Is_CrystAlien_Type(b->Class->IniName)) return(0);

	/*
	**	Measured from the art, not from the shape frame.
	**
	**	This asked Get_Build_Frame_Height, which is the height of the *frame* --
	**	the renderer's canvas, transparent margin and all.  Nearly every structure
	**	is drawn inside a frame a good deal taller than itself, so nearly every
	**	structure came out one whole row taller than anything drawn in it, and the
	**	Training Camp and the Power Pump came out two.
	**
	**	A row claimed here is a row of cells the map believes the building stands
	**	on: it cannot be ordered into, it answers ACTION_SELECT instead of a move,
	**	and on a capture-the-flag map an Eagle Command in front of the flag made
	**	the flag itself unreachable.  That is the whole of "the hitboxes are bigger
	**	than the sprites and I cannot click behind them".
	**
	**	So the rise is worked out from the rendered pixels at build time, where
	**	there is an alpha channel to look at, and read back from a table.  See
	**	art_rise_rows in gen_dll_types.py for the sum; shapes draw SHAPE_CENTER on
	**	a structure's footprint centre, which is what makes it answerable.
	*/
	static struct {
		StructType	Type;
		int			Rows;
	} const _rise[] = {
#include "CACRISE.INC"
	};

	for (int i = 0; i < (int)(sizeof(_rise) / sizeof(_rise[0])); i++) {
		if (_rise[i].Type == b->Class->Type) return(_rise[i].Rows);
	}
	return(0);
}


/***********************************************************************************************
 * CAC_Overhang_Yields -- Is this a cell artwork may not claim, whatever stands over it?        *
 *                                                                                             *
 *    The rows a structure is drawn across but does not stand on are artwork, and they are      *
 *    claimed so that the part of a building you can see can be clicked, targeted and captured. *
 *    A flag is the one thing that cannot afford to lose that argument: it is an overlay, so    *
 *    the cell holds no object of its own, and a Headquarters drawn across it takes every       *
 *    click aimed at it -- which in capture the flag is the game.                               *
 *                                                                                             *
 *    Footprint as well as overhang.  CellClass::Flag_Place opens with `if (!IsFlagged &&        *
 *    Is_Generally_Clear())`, which refuses a cell with a building occupying it or merely         *
 *    standing over it -- so a flag placed while the game is running is never under a structure,  *
 *    and any claim on its cell is the overlap list's rectangle reaching past the artwork.        *
 *                                                                                               *
 *    That guarantee has one hole and it is not this routine's to plug: Is_Generally_Clear opens  *
 *    `if (ScenarioInit) return(true);`, so a flag planted during scenario setup gets none of it. *
 *    See CAC_Flag_Site, which asks the questions itself at the one moment that happens.          *
 *=============================================================================================*/
bool CAC_Overhang_Yields(CELL cell)
{
	if ((unsigned)cell >= MAP_CELL_TOTAL) return(false);
	return(Map[cell].Overlay == OVERLAY_FLAG_SPOT);
}


/***********************************************************************************************
 * CAC_Building_Over -- Which structure is drawn across this cell, occupied or not.             *
 *                                                                                             *
 *    Building.setBlock claims a column and a row less than the building is drawn on, which is  *
 *    deliberate -- it leaves a lane down two sides of every structure and is why a packed      *
 *    CrystAlien base can still be walked through.  The occupy lists here follow it.            *
 *                                                                                             *
 *    But in Tiberian Dawn the occupied cells are also the only cells that answer.  Cell_Object *
 *    asks Cell_Techno, Cell_Techno walks the occupier list, and a cell the building merely     *
 *    *covers* holds nothing -- so most of a Breeding Pit could not be clicked, targeted or     *
 *    captured, and which part of it worked depended on where in the sprite you happened to     *
 *    point.  CrystAlien has no such trouble because it hit-tests the sprite rather than the    *
 *    grid.                                                                                     *
 *                                                                                             *
 *    So the ground it stands on and the ground it answers for are separated: this reads the    *
 *    overlap list, which is the drawn footprint, and is consulted only when a cell holds       *
 *    nothing of its own.  Nothing about movement or placement changes.                         *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Scenery_Yields -- Should a structure drawn over this cell answer instead of what is on it?*
 *                                                                                             *
 *    Both places that turn a point into an object -- Close_Object for the click, and the       *
 *    per-cell action map for the cursor -- ask CAC_Building_Over only when they have found     *
 *    nothing at all.  That covers bare ground, which is the case they were written for, and    *
 *    misses the one in the report: a tree standing behind a Solar Panel.                       *
 *                                                                                             *
 *    The tree *occupies* the cell the upper half of the building is drawn over, so it is found *
 *    first and answers, and a structure two cells tall loses the half of itself the player can  *
 *    actually see.  From outside that is a wrench cursor that will not appear over a damaged    *
 *    power plant, and an engineer that cannot be told to repair it.                             *
 *                                                                                             *
 *    Scenery is the right thing to give way.  A tree or a rock is not something an order can    *
 *    be aimed at -- nothing can be sent to one, entered into one or repaired on one -- so       *
 *    where it competes with a building for the same pixel the building is what was meant.       *
 *    Anything that can take an order keeps precedence, which is why this asks for RTTI_TERRAIN  *
 *    rather than for "not a building".                                                          *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Building_Unenterable -- Has this structure no ground an engineer may legally stand on?   *
 *                                                                                             *
 *    An engineer takes or repairs a building by walking into it: Can_Enter_Cell makes a special *
 *    exception so the target's own cells are passable to it, and Per_Cell_Process fires when it *
 *    arrives on one.  Every step of that assumes at least one of those cells can be reached.    *
 *                                                                                             *
 *    A structure standing on the very edge of the map has none.  Can_Enter_Cell opens with      *
 *                                                                                             *
 *        if (IsLocked && !IsALoaner && !ScenarioInit && !Map.In_Radar(cell)) return(MOVE_NO);   *
 *                                                                                             *
 *    and In_Radar is the playable rectangle, so a Solar Panel sitting on the bottom border has  *
 *    its footprint outside it.  The engineer paths as close as it can, stops one square short,  *
 *    and no amount of telling it again helps -- there is nowhere for it to go.  That is the     *
 *    report, and it is also why a miner behaves the same way at the same place.                 *
 *                                                                                             *
 *    So this asks the narrow question rather than relaxing the arrival test generally: only a   *
 *    building with no legal cell at all lets an engineer finish the job from beside it.  One    *
 *    standing anywhere else is entered the way it always was.                                   *
 *=============================================================================================*/
bool CAC_Building_Unenterable(BuildingClass const *b)
{
	if (b == NULL || b->Class == NULL) return(false);

	CELL origin = Coord_Cell(b->Coord);
	short const *list = b->Class->Occupy_List();
	if (list == NULL) return(false);

	while (*list != REFRESH_EOL) {
		CELL c = (CELL)(origin + *list);
		if ((unsigned)c < MAP_CELL_TOTAL && Map.In_Radar(c)) return(false);
		list++;
	}
	return(true);
}


bool CAC_Scenery_Yields(ObjectClass const *found)
{
	if (found == NULL) return(true);
	return(found->What_Am_I() == RTTI_TERRAIN);
}


BuildingClass * CAC_Building_Over(CELL cell)
{
	if ((unsigned)cell >= MAP_CELL_TOTAL) return(NULL);

	for (int i = 0; i < Buildings.Count(); i++) {
		BuildingClass * b = Buildings.Ptr(i);
		if (b == NULL || !b->IsActive || b->IsInLimbo || b->Class == NULL) continue;
		if (!CAC_Is_CrystAlien_Type(b->Class->IniName)) continue;

		CELL origin = Coord_Cell(b->Coord);
		short const * list = b->Class->Overlap_List();
		if (list == NULL) continue;

		/*
		**	How many rows of artwork stand above the ground it covers.
		**
		**	The overlap list is generated from CrystAlien's own `width` and
		**	`height`, with the height halved because its tiles are 96 by 48 and
		**	ours are square -- so it describes the *ground*, and a Training Camp
		**	comes out two cells across and one deep.  The building is drawn two
		**	cells tall on top of that, and the upper one appears in neither list.
		**
		**	Which is why this building has never been clickable.  Every route into
		**	it ends at a cell lookup -- the attack order, the engineer's arrival,
		**	the repair cursor -- and the half you actually see belongs to no
		**	structure at all as far as the map is concerned.
		**
		**	DLLInterface already tells the client about those rows, working them
		**	out from the drawn shape.  The same sum, so the two agree about where
		**	the building is.
		*/
		int footrows = 0;
		for (short const * scan = list; *scan != REFRESH_EOL; scan++) {
			int row = ((int)*scan + (MAP_CELL_W / 2)) / MAP_CELL_W;
			if (row + 1 > footrows) footrows = row + 1;
		}

		int extra = CAC_Drawn_Rows_Above(b);
		(void)footrows;

		while (*list != REFRESH_EOL) {
			for (int up = 0; up <= extra; up++) {
				if (CAC_Overhang_Yields(cell)) continue;
				if ((CELL)(origin + *list - (MAP_CELL_W * up)) == cell) return(b);
			}
			list++;
		}
	}
	return(NULL);
}


bool CAC_Can_Attack(TechnoClass const * object)
{
	if (object == NULL) return(true);

	TechnoTypeClass const * type = object->Techno_Type_Class();
	if (type == NULL || !CAC_Is_CrystAlien_Type(type->IniName)) return(true);

	/*
	**	An engineer counts as able to attack, because that is how it captures.
	**
	**	InfantryClass::What_Action derives the capture from the attack -- it waits for
	**	action to come back as ACTION_ATTACK and turns it into ACTION_CAPTURE if the
	**	target can be taken.  Refusing an unarmed unit the attack refuses it the
	**	capture with it, which left Engineers and Saboteurs hovering enemy buildings
	**	and offering nothing but a select cursor.
	**
	**	So it is the ones that can do neither that are stopped: no weapon, and nothing
	**	to capture with.  Which is the reindeer, and is where this started.
	*/
	if (object->What_Am_I() == RTTI_INFANTRY &&
		((InfantryTypeClass const *)type)->IsCapture) return(true);

	return(type->Primary != WEAPON_NONE || type->Secondary != WEAPON_NONE);
}


/***********************************************************************************************
 * CAC_Options_Changed -- Say whether the build list grew or shrank.                            *
 *                                                                                             *
 *    Construction.buildMenus counts what is on offer before and after it rebuilds:             *
 *                                                                                             *
 *        if (this.buildings > was_b || this.units > was_u) sfx.play("INT_optionsadd");         *
 *        if (this.buildings < was_b || this.units < was_u) sfx.play("INT_optionsremove");      *
 *                                                                                             *
 *    Two sounds, and which one plays is decided by the direction the count moved.  Hanging     *
 *    them off the engine's spoken lines instead -- VOX_NEW_CONSTRUCT for one, VOX_CANCELED for *
 *    the other -- got it wrong, because the engine says "new construction options" whenever    *
 *    the list *changes*, including when a building is lost and the list is shorter for it.  So *
 *    losing a Vehicle Factory played the sound for gaining something.                          *
 *                                                                                             *
 *    Counted rather than inferred now.  Called where the sidebar is recalculated, which is the *
 *    same moment buildMenus runs, and cheap enough there: it is a few hundred Can_Build calls  *
 *    on the frame a building changes hands rather than every frame.                            *
 *=============================================================================================*/
void CAC_Options_Changed(HouseClass * house)
{
	if (house == NULL || house != PlayerPtr || !CAC_Is_CrystAlien_House(house)) return;

	int count = 0;
	int i;
	for (i = STRUCT_FIRST; i < STRUCT_COUNT; i++) {
		if (house->Can_Build((StructType)i, house->ActLike)) count++;
	}
	for (i = UNIT_FIRST; i < UNIT_COUNT; i++) {
		if (house->Can_Build((UnitType)i, house->ActLike)) count++;
	}
	for (i = INFANTRY_FIRST; i < INFANTRY_COUNT; i++) {
		if (house->Can_Build((InfantryType)i, house->ActLike)) count++;
	}
	for (i = AIRCRAFT_FIRST; i < AIRCRAFT_COUNT; i++) {
		if (house->Can_Build((AircraftType)i, house->ActLike)) count++;
	}

	static int _was = -1;
	if (_was >= 0 && count != _was) {
		CAC_Interface_Sound(count > _was ? VOC_CAC_INT_OPTADD : VOC_CAC_INT_OPTREM);
	}
	_was = count;
}


void CAC_Order_Sound(TechnoClass const *object, MissionType mission)
{
	if (object == NULL || object->House != PlayerPtr) return;
	TechnoTypeClass const *type = object->Techno_Type_Class();
	if (type == NULL || !CAC_Is_CrystAlien_Type(type->IniName)) return;

	switch (mission) {
		case MISSION_ATTACK:
		case MISSION_CAPTURE:
		case MISSION_SABOTAGE:
		case MISSION_ENTER:
		case MISSION_REPAIR:
			CAC_Interface_Sound(VOC_CAC_INT_CURTGT);
			break;

		default:
			CAC_Interface_Sound(VOC_CAC_INT_CURMOVE);
			break;
	}
}


/***********************************************************************************************
 * CAC_Select_Sound -- The cue that answers picking something up.                              *
 *                                                                                             *
 *    "sfx.play("INT_cursor_select")" wherever Control settles on a new selection.  Same gate    *
 *    as the orders: a box drawn round six units is one sound, not six.                          *
 *=============================================================================================*/
void CAC_Select_Sound(ObjectClass const *object)
{
	if (object == NULL || !object->Is_Techno()) return;
	if (((TechnoClass const *)object)->House != PlayerPtr) return;
	TechnoTypeClass const *type = ((TechnoClass const *)object)->Techno_Type_Class();
	if (type == NULL || !CAC_Is_CrystAlien_Type(type->IniName)) return;

	CAC_Interface_Sound(VOC_CAC_INT_CURSEL);
}


/***********************************************************************************************
 * CAC_Power_Klaxon -- The two sounds a base makes when its lights go.                         *
 *                                                                                             *
 *    Level.think, once the level has been running long enough to have settled:                 *
 *                                                                                             *
 *        if (this.count > 200) {                                                               *
 *            if (this.powerLow && !wasLow) { showMessage(int_power_low); play("INT_powerwarning"); }
 *            if (this.powerOff && !wasOff) { showMessage(int_power_off); play("INT_warning"); }
 *        }                                                                                     *
 *                                                                                             *
 *    On the transition and only on the transition -- a base that sits browned out is quiet     *
 *    about it.  Two hundred source frames is about eight and a half seconds, and it is there   *
 *    so a level does not open with a klaxon while its buildings are still being placed.        *
 *                                                                                             *
 *    Recovering is silent: there is a message for it and no sound.                             *
 *                                                                                             *
 *    Kept per house in a plain table rather than on HouseClass, because it is only ever used   *
 *    to notice a change -- the worst a lost value can do is let one klaxon through after a     *
 *    load, and the alternative is another field in the save format.                            *
 *=============================================================================================*/
void CAC_Power_Klaxon(HouseClass *house)
{
	if (house == NULL || house->Class == NULL) return;
	if (!CAC_Is_CrystAlien_House(house)) return;

	/*
	**	Only the player is told; nobody is listening to the computer's base.
	*/
	if (house != PlayerPtr) return;

	/*
	**	`count > 200` -- source frames at twenty-three to the second.
	*/
	if ((long)Frame < (200L * 15L) / 23L) return;

	static CACPowerState _was[HOUSE_COUNT];
	static bool _seeded = false;
	if (!_seeded) {
		for (int i = 0; i < HOUSE_COUNT; i++) _was[i] = CAC_POWER_FULL;
		_seeded = true;
	}

	int who = (int)house->Class->House;
	if (who < 0 || who >= HOUSE_COUNT) return;

	CACPowerState now = CAC_Power_State(house);
	CACPowerState before = _was[who];
	_was[who] = now;
	if (now == before) return;

	/*
	**	Each threshold speaks for itself, so falling straight from full to dark
	**	raises both, which is what the source does -- the two tests are separate.
	*/
	if (now != CAC_POWER_FULL && before == CAC_POWER_FULL) {
		CAC_Interface_Sound(VOC_CAC_INT_PWRWARN);
	}
	if (now == CAC_POWER_OFF && before != CAC_POWER_OFF) {
		CAC_Interface_Sound(VOC_CAC_INT_WARNING);
	}
}


/***********************************************************************************************
 * CAC_Building_Hit -- The noise a structure makes when something lands on it.                 *
 *                                                                                             *
 *    Building.pain, the first thing it does:                                                   *
 *                                                                                             *
 *        if (weapon.dmg) {                                                                     *
 *            this.health -= weapon.dmg;                                                        *
 *            var pool = new Array("UD_good_pain_1", "UD_good_pain_2", "UE_good_pain_1",         *
 *                                 "UE_good_pain_1", "UH_good_pain_1", "UH_good_pain_1");        *
 *            sfx.play(pool[random(6) + 1]);                                                     *
 *        }                                                                                      *
 *                                                                                             *
 *    A struck building borrows a *unit's* pain recording, and always an Astro one whichever    *
 *    faction owns it.  Nothing else in the game plays these for a structure and there is no    *
 *    structure-specific sample; this is simply what it does.                                    *
 *                                                                                             *
 *    The `+ 1` is reproduced along with the rest.  It runs the index from one to six over a    *
 *    six-element array, so the first entry is never heard and one hit in six is silent.  That  *
 *    is audible -- it is the difference between a building that yelps every time and one that  *
 *    mostly does -- so it stays.                                                                *
 *                                                                                             *
 *    Separate from INT_damage_1..3, which is the once-only cue when a structure first drops to *
 *    a quarter and starts smoking.  See CAC_Damage_Smoke.                                       *
 *=============================================================================================*/
void CAC_Building_Hit(BuildingClass *building)
{
	if (building == NULL || building->Class == NULL) return;
	if (!CAC_Is_CrystAlien_Type(building->Class->IniName)) return;

	static VocType const _pool[6] = {
		VOC_CAC_UDG_PAIN1, VOC_CAC_UDG_PAIN2, VOC_CAC_UEG_PAIN1,
		VOC_CAC_UEG_PAIN1, VOC_CAC_UHG_PAIN1, VOC_CAC_UHG_PAIN1
	};

	int pick = Sim_Random_Pick(0, 5) + 1;	/* one to six, over six entries */
	if (pick > 5) return;					/* the sixth reading is off the end: silence */

	/*
	**	No coordinate: this one carries across the map.
	**
	**	Building.pain plays it with `this.parent.parent.sfx.play(...)` and not
	**	through makeNoise, so it is not gated on the building being on screen
	**	and there is no attenuation in that engine to soften it with.  It is how
	**	a player learns a base is being hit while looking somewhere else, and
	**	putting it at the building took that away.
	*/
	Sound_Effect(_pool[pick]);
}


/***********************************************************************************************
 * CAC_Interface_Sound -- Play one of CrystAlien's interface sounds, once.                     *
 *                                                                                             *
 *    These are short, distinctive and meant to mark a moment -- money arriving, a radar        *
 *    waking, a miner breaking ground.  Played twice in a row they stop being a moment, and     *
 *    played every frame they become a drone that seems to come from whatever building is       *
 *    nearest, which is how a Solar Panel came to be humming.                                   *
 *                                                                                             *
 *    Every route to one of these is meant to be a single event, but they run off game state    *
 *    that is polled -- a radar that reports itself "still switching on" for a whole animation, *
 *    a harvest mission that re-enters its looking state -- and a poll repeats.  So rather than *
 *    trust each caller, the same sound simply will not play twice inside a second.             *
 *                                                                                             *
 *    Different sounds do not block each other, and the unit voices do not come through here    *
 *    at all: two units answering at once is the game working.                                  *
 *=============================================================================================*/
void CAC_Interface_Sound(VocType voc, COORDINATE coord)
{
	if (voc == VOC_NONE) return;

	static struct {
		VocType	Voc;
		long		When;
	} _recent[8] = {{VOC_NONE, 0}, {VOC_NONE, 0}, {VOC_NONE, 0}, {VOC_NONE, 0},
						 {VOC_NONE, 0}, {VOC_NONE, 0}, {VOC_NONE, 0}, {VOC_NONE, 0}};
	static int _next = 0;

	for (int i = 0; i < (int)(sizeof(_recent) / sizeof(_recent[0])); i++) {
		if (_recent[i].Voc != voc) continue;
		if ((long)Frame - _recent[i].When < TICKS_PER_SECOND) return;
		_recent[i].When = (long)Frame;
		Sound_Effect(voc, coord);
		return;
	}

	_recent[_next].Voc = voc;
	_recent[_next].When = (long)Frame;
	_next = (_next + 1) % (int)(sizeof(_recent) / sizeof(_recent[0]));
	Sound_Effect(voc, coord);
}


/***********************************************************************************************
 * CAC_Weapon_Noise -- The sound a CrystAlien shot makes.                                      *
 *                                                                                             *
 *    A unit's is its own voice.  Unit.attack ends with                                        *
 *                                                                                             *
 *        this.makeNoise(this.type + "_action_" + (random(3) + 1));                            *
 *                                                                                             *
 *    -- the same three recordings it uses for mining and for repairing, because in CrystAlien  *
 *    "action" means the unit is doing its job and firing is one of those.                     *
 *                                                                                             *
 *    A building's is INT_guntower_1..3, and finding that took longer than it should have:      *
 *    no ActionScript anywhere names those samples, so a search of the sfx.play call sites      *
 *    concludes that structures fire silently.  They do not -- the sound sits on the turret     *
 *    clip's own timeline and plays when the animation does, which is a mechanism the scripts   *
 *    never mention.  The samples are unmistakable, they were extracted and shipped with        *
 *    everything else, and nothing in the conversion had ever played them.                      *
 *                                                                                             *
 *    Three variations, picked at random, exactly as the unit voices are.                       *
 *=============================================================================================*/
void CAC_Weapon_Noise(TechnoClass *object)
{
	if (object == NULL) return;
	TechnoTypeClass const *type = object->Techno_Type_Class();
	if (type == NULL || !CAC_Is_CrystAlien_Type(type->IniName)) return;

	/*
	**	A structure's shot is INT_guntower_1..3, one of three at random -- the same
	**	shape as a unit's action line, and the sound a Defence Station or an Ops
	**	Ship makes when it fires.  No ActionScript names those samples, which is
	**	why reading the sfx.play call sites says structures fire in silence; the
	**	sound sits on the turret clip's own animation instead.
	*/
	if (object->What_Am_I() == RTTI_BUILDING) {
		static VocType const _tower[] = {
			VOC_CAC_INT_GUNT1, VOC_CAC_INT_GUNT2, VOC_CAC_INT_GUNT3
		};

		/*
		**	Random_Pick, not Sim_Random_Pick, and the draw happens whether the
		**	sound plays or not.
		**
		**	Sim_Random_Pick is the synchronised stream -- Sim_IRandom -- and this
		**	sat behind Map.In_View, which is a different answer on every client
		**	because every player is looking somewhere else.  One player watching
		**	a Defence Station fire drew from the shared sequence and a player
		**	looking elsewhere did not, so the streams parted and the game
		**	desynchronised.  Which of three samples an effect uses is a local
		**	decision and has no business in the synchronised stream at all.
		*/
		VocType voc = _tower[Random_Pick(0, 2)];
		if (!Map.In_View(Coord_Cell(object->Center_Coord()))) return;
		Sound_Effect(voc, object->Center_Coord());
		return;
	}

	CAC_Speak(object, (CACVoiceEvent)(CAC_VOICE_ACTION1 + Sim_Random_Pick(0, 2)));
}


/***********************************************************************************************
 * CAC_Single_Target -- Does this shot damage only what it was aimed at?                       *
 *                                                                                             *
 *    Weapon.collision() is the whole of CrystAlien's impact:                                   *
 *                                                                                             *
 *        if(this.target.checkForHit(this.posX, this.posY))                                     *
 *        {                                                                                     *
 *           this.target.pain(this);                                                            *
 *           this.destroy();                                                                    *
 *        }                                                                                     *
 *                                                                                             *
 *    It asks one object whether it was hit, damages that object, and stops.  Nothing else on   *
 *    the map is consulted -- not what stands beside the target, not what the bolt flew over on *
 *    the way.  The laser is the same shape: `this.target.pain(this)` at the moment it is drawn *
 *    and no second look.                                                                       *
 *                                                                                             *
 *    Tiberian Dawn detonates instead.  Explosion_Damage sweeps the impact cell and all eight   *
 *    around it and damages everything standing there, with no ownership test -- so a Trike     *
 *    firing past a friendly Miner takes a bite out of the Miner, and a shot at one building in *
 *    a packed base chips its neighbours.  That does not happen in the original, and now that   *
 *    bases pack tightly it happens constantly.                                                 *
 *                                                                                             *
 *    So the question is asked of the shooter rather than of the warhead.  WARHEAD_LASER is     *
 *    shared with Tiberian Dawn's own Obelisk and SSM launcher, which still exist in the         *
 *    supplemental build and are entitled to the splash they were designed with.                 *
 *                                                                                             *
 * OUTPUT:  true if only the intended target should take damage.                                *
 *=============================================================================================*/
bool CAC_Single_Target(TechnoClass const *shooter)
{
	if (shooter == NULL) return(false);
	TechnoTypeClass const *type = shooter->Techno_Type_Class();
	if (type == NULL) return(false);
	return(CAC_Is_CrystAlien_Type(type->IniName));
}


/***********************************************************************************************
 * CAC_Bullet_For -- Which projectile a CrystAlien shot actually sends.                        *
 *                                                                                             *
 *    Weapon() picks its artwork from the shooter, not from the weapon:                        *
 *                                                                                             *
 *        this.MCsprite.gotoAndStop(this.type + "_" + this.shooter.team);                      *
 *                                                                                             *
 *    -- so a rocket is Astro blue or Alien yellow-green depending on who fired it, and the     *
 *    same weapon fires either.  That is why this is decided here rather than in the weapon     *
 *    table: the table has one row per range and rate of fire, shared by both factions, and     *
 *    splitting every one of them in two to carry a colour would have doubled it.               *
 *                                                                                             *
 *    Only the rocket has a projectile at all.  The laser is drawn straight to the target and   *
 *    never becomes an object -- see CAC_Laser_Colours.                                         *
 *                                                                                             *
 * OUTPUT:  the projectile to send, unchanged for anything that is not CrystAlien's.            *
 *=============================================================================================*/
BulletType CAC_Bullet_For(TechnoClass const *object, BulletType fires)
{
	if (fires != BULLET_SSM || object == NULL) return(fires);

	TechnoTypeClass const *type = object->Techno_Type_Class();
	if (type == NULL || !CAC_Is_CrystAlien_Type(type->IniName)) return(fires);

	bool evil = ((CAC_Code_Of(type->What_Am_I(), CAC_Type_Index_Of_Type(type))
					   & CAC_CODE_EVIL) != 0);
	return(evil ? BULLET_CAC_ROCKEVIL : BULLET_CAC_ROCKGOOD);
}


/***********************************************************************************************
 * CAC_Snap_Fire -- Does this object aim by turning, or by simply pointing?                    *
 *                                                                                             *
 *    Unit.attack works out the angle to the target, assigns it, and fires -- all in the one    *
 *    frame:                                                                                    *
 *                                                                                             *
 *        this.nav.nextAngle = this.angle = _loc2_;                                             *
 *        this.parent.fireWeapon(this, this.stats.weapon, ...);                                 *
 *                                                                                             *
 *    There is no tolerance to satisfy and no wait for a rotation to finish, and nothing stops  *
 *    a unit shooting while it drives.  Tiberian Dawn asks for all three: the facing has to be  *
 *    within about eleven degrees, a body that is still turning may not fire, and a unit with   *
 *    no turret must stop first.                                                                *
 *                                                                                             *
 *    Between two quick units at close quarters those three compound.  The bearing swings       *
 *    faster than either can turn, so each is always still turning and always just outside the  *
 *    tolerance, and neither ever fires -- which is the pair of Trikes sitting nose to nose      *
 *    doing nothing.                                                                             *
 *                                                                                             *
 * OUTPUT:  true for a CrystAlien object, which aims by pointing.                               *
 *=============================================================================================*/
bool CAC_Snap_Fire(TechnoClass const *object)
{
	if (object == NULL) return(false);
	TechnoTypeClass const *type = object->Techno_Type_Class();
	if (type == NULL) return(false);
	return(CAC_Is_CrystAlien_Type(type->IniName));
}


/***********************************************************************************************
 * CAC_Has_Factory -- Can this house still produce anything at all?                            *
 *                                                                                             *
 *    The computer asks this before deciding the game is over for it.  Check_Fire_Sale reads    *
 *                                                                                             *
 *        ActiveBScan & (STRUCTF_BARRACKS|STRUCTF_CONST|STRUCTF_AIRSTRIP|STRUCTF_WEAP|          *
 *                       STRUCTF_HELIPAD)                                                       *
 *                                                                                             *
 *    -- every one of them a Tiberian Dawn building, and a CrystAlien house owns none.  So the  *
 *    answer was always "no factories left", from the first frame it had a building at all,     *
 *    and the reply to that is Fire_Sale: sell every structure and send everything on a         *
 *    suicide hunt.  The computer was demolishing its own base the moment its construction      *
 *    vehicle unfolded, which is why no Eagle Command or Mothership was ever standing.          *
 *                                                                                             *
 *    The question CrystAlien would ask is whether any building with a `home` list is left,     *
 *    because that is what Construction builds out of -- so it is asked of the same hosts       *
 *    table that says which building makes what.                                                *
 *                                                                                             *
 * OUTPUT:  true if the house owns something that can still produce.                            *
 *=============================================================================================*/
bool CAC_Has_Factory(HouseClass const *house)
{
	if (house == NULL) return(false);

	for (int i = 0; i < (int)(sizeof(CACHosts) / sizeof(CACHosts[0])); i++) {
		if ((house->ActiveBScan & CAC_Scan_Bit((int)CACHosts[i].Host)) != 0) return(true);
	}
	return(false);
}


/***********************************************************************************************
 * CAC_Place_Code -- Put one of a CrystAlien role's units on the map, whatever kind it is.      *
 *                                                                                             *
 *    A crate names what it hands over by role -- a Reindeer, a Space Buggy -- and the engine   *
 *    wants to know which of its four classes that is before it can place one.  Naming the      *
 *    class at the call site is how the Christmas gift came to hand over nothing: the Reindeer  *
 *    was a vehicle when the crate was written and is infantry now, so the lookup found no      *
 *    slot and the gift opened onto an empty tile.                                              *
 *                                                                                             *
 *    So the role is asked for and the class follows from wherever it is found.                 *
 *                                                                                             *
 * OUTPUT:  was one placed?                                                                     *
 *=============================================================================================*/
bool CAC_Place_Code(int code, bool evil, CELL where, HousesType owner)
{
	static RTTIType const _kinds[] = {
		RTTI_UNITTYPE, RTTI_INFANTRYTYPE, RTTI_AIRCRAFTTYPE
	};

	for (int i = 0; i < (int)(sizeof(_kinds) / sizeof(_kinds[0])); i++) {
		int slot = CAC_Slot_For(_kinds[i], code, evil);
		if (slot < 0) continue;

		switch (_kinds[i]) {
			case RTTI_INFANTRYTYPE:
				return(InfantryTypeClass::As_Reference((InfantryType)slot).Create_And_Place(where, owner));
			case RTTI_AIRCRAFTTYPE:
				return(AircraftTypeClass::As_Reference((AircraftType)slot).Create_And_Place(where, owner));
			default:
				return(UnitTypeClass::As_Reference((UnitType)slot).Create_And_Place(where, owner));
		}
	}
	return(false);
}


/***********************************************************************************************
 * CAC_Laser_Colours -- The beam a CrystAlien shooter draws.                                   *
 *                                                                                             *
 *    Weapon() draws the laser itself rather than sending a projectile: a three-pixel line      *
 *    from the shooter to the target, in the shooter's team colour, under a GlowFilter, alive   *
 *    for five frames and fading as it goes.                                                    *
 *                                                                                             *
 *        var c = 13041527;                            // 0xC6FF77, the Alien yellow-green      *
 *        if (this.shooter.team == "good") c = 12313073;   // 0xBBE1F1, the Astro pale blue     *
 *        MC.lineStyle(3, c, 100);                                                              *
 *        MC.moveTo(posX, posY / 2);  MC.lineTo(targetX, targetY / 2);                          *
 *        this.life = 5;                                                                        *
 *                                                                                              *
 *    Tiberian Dawn draws its own laser the same way and for the same five frames -- three      *
 *    parallel lines, a pair of edges around a brighter core, which is what the glow amounts    *
 *    to.  Only the colour differs, and it was the Obelisk's red.                               *
 *                                                                                             *
 *    The palette has no exact match for either, so these are the nearest by hue: a laser is    *
 *    read by its colour before anything else, and the three theatre palettes agree on all      *
 *    four entries, so the beam looks the same wherever it is fired.                            *
 *                                                                                             *
 * OUTPUT:  false for anything that is not a CrystAlien shooter, leaving the engine's own.      *
 *=============================================================================================*/
bool CAC_Laser_Colours(TechnoClass const *object, int &edge, int &core)
{
	if (object == NULL) return(false);
	TechnoTypeClass const *type = object->Techno_Type_Class();
	if (type == NULL || !CAC_Is_CrystAlien_Type(type->IniName)) return(false);

	bool evil = ((CAC_Code_Of(type->What_Am_I(), CAC_Type_Index_Of_Type(type))
					   & CAC_CODE_EVIL) != 0);
	if (evil) {
		/*
		**	0xA6 was here as the core, picked on hue alone -- it is the same 82 degrees
		**	as #C6FF77 and nothing else in the palette is.  But hue was the only thing
		**	it had right: (162,227,28) is a hard lime, where the beam is a pale
		**	yellow-green, and measured against the target it sits 102 away when 0x83 is
		**	54.  The saturated one read as a different weapon entirely.
		**
		**	0x82 and 0x83 are neighbours on one ramp, so the pair reads as a lit edge
		**	around a brighter middle -- the same relationship 0x7D and 0x7F have, which
		**	is how the engine's own laser is built.
		*/
		edge = 0x82;		// (182,186,121) -- the dimmer edge
		core = 0x83;		// (203,211,150) -- nearest in the palette to #C6FF77
	} else {
		edge = 0xA2;		// (162,207,211)
		core = 0xA1;		// (219,255,255)
	}
	return(true);
}


/***********************************************************************************************
 * CAC_Owns_Its_Sounds -- Should this object be left to its own recordings?                    *
 *                                                                                             *
 *    CrystAlien plays four things for a unit -- comply, action, pain and die -- and a handful  *
 *    of interface sounds for the player.  It has nothing for crushing infantry, nothing for    *
 *    refusing an order, nothing for cloaking, and the engine's own effects in those places     *
 *    are unmistakably Tiberian Dawn's.  So they are simply not played for CrystAlien's own.    *
 *                                                                                             *
 * OUTPUT:  true if this is a CrystAlien type, and the engine should stay quiet for it.         *
 *=============================================================================================*/
bool CAC_Owns_Its_Sounds(TechnoClass const *object)
{
	if (object == NULL) return(false);
	TechnoTypeClass const *type = object->Techno_Type_Class();
	if (type == NULL) return(false);
	return(CAC_Is_CrystAlien_Type(type->IniName));
}


/***********************************************************************************************
 * CAC_Exit_Style -- How a finished object leaves a CrystAlien building.                       *
 *                                                                                             *
 *    BuildingClass::Exit_Object decides that by switching on the building's own slot: a        *
 *    STRUCT_WEAP rolls the vehicle out of its door, a STRUCT_BARRACKS walks it to a free       *
 *    adjacent cell, a STRUCT_AIRSTRIP calls in a transport.  Anything the switch does not      *
 *    name returns zero, and a zero means the object never leaves -- it sits on READY.          *
 *                                                                                             *
 *    While CrystAlien's buildings borrowed the engine's slots they were named by accident.     *
 *    They have their own slots now, so they have to be named on purpose, and this says which   *
 *    style each one uses.  They all use the same one: Building.spawn puts the new object at    *
 *    the building's dock and lets it drive away, which is what the barracks style does.        *
 *                                                                                             *
 * OUTPUT:  STRUCT_NONE for a building that hosts nothing, or is not CrystAlien's.              *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Exit_Cell -- Where a finished object is put, the way CrystAlien puts it.                 *
 *                                                                                             *
 *    arena.closestAvailable(pos) is the whole of CrystAlien's placement, and it is a fixed     *
 *    list rather than a search: the tile asked for, then its four neighbours in the order      *
 *    north, east, south, west, then the eight of the next ring, then the twenty-four of the    *
 *    one after -- thirty-seven in all -- and the first of them that is unoccupied wins.        *
 *                                                                                             *
 *    Two things about it matter more than the order.                                          *
 *                                                                                             *
 *    It cannot fail.  The last line is `return pos`, so a factory hemmed in on every side      *
 *    still hands back the tile it started from and the unit is placed regardless.  Nothing     *
 *    waits for a bib to clear and nothing is refused, which is the behaviour asked for, and    *
 *    Tiberian Dawn's own exit walk does the opposite -- it returns zero and the factory sits   *
 *    on a finished unit it will not release.                                                   *
 *                                                                                             *
 *    And what counts as occupied is the arena's tile grid, which a unit joins only if its      *
 *    stats say `obstruct`.  Ground vehicles do; aircraft and infantry do not.  So each vehicle *
 *    that rolls out takes its tile and pushes the next one further along the list, while       *
 *    aircraft never claim anything and every one of them is handed the same tile -- which is   *
 *    exactly the pile of fighters that stacks up in front of a Radar Station, and why they sit *
 *    in each other without colliding.                                                          *
 *                                                                                             *
 *    The tile asked for is the building's own dock, which BuildingStats puts at the bottom row *
 *    of the footprint and left of centre.  The building occupies that itself, so the first     *
 *    candidate that is ever free is the one directly south of it -- the front of the baseplate.*
 *                                                                                             *
 * INPUT:   bld -- the building letting the object out.                                         *
 *          obj -- what is coming out; only whether it flies is consulted.                      *
 *                                                                                             *
 * OUTPUT:  A cell to place it in.  Always a cell -- there is no failure return.                *
 *=============================================================================================*/
CELL CAC_Exit_Cell(BuildingClass const * bld, TechnoClass const * obj)
{
	static struct {
		signed char	X;
		signed char	Y;
	} const _ring[] = {
		/*
		**	The tile itself.
		*/
		{ 0,  0},

		/*
		**	And its four neighbours, north, east, south, west.
		*/
		{ 0, -1}, { 1,  0}, { 0,  1}, {-1,  0},

		/*
		**	The eight of the next ring out, in the order CrystAlien lists them.
		*/
		{ 0, -2}, { 1, -1}, { 2,  0}, { 1,  1}, { 0,  2}, {-1,  1},
		{-2,  0}, {-1, -1},

		/*
		**	And the twenty-four beyond that.
		*/
		{ 0, -3}, { 1, -3}, { 1, -2}, { 2, -2}, { 2, -1}, { 3, -1},
		{ 3,  0}, { 2,  1}, { 3,  1}, { 1,  2}, { 2,  2}, { 1,  3},
		{ 0,  3}, {-1,  3}, {-1,  2}, {-2,  2}, {-2,  1}, {-3,  1},
		{-3,  0}, {-2, -1}, {-3, -1}, {-1, -2}, {-2, -2}, {-1, -3}
	};

	if (bld == NULL || bld->Class == NULL) return(0);

	/*
	**	The dock.
	**
	**	BuildingStats works it out as the bottom row of the footprint -- topLeftY plus
	**	height minus one -- and, across the width, topLeftX plus width/2 minus one.  For
	**	anything up to three tiles wide that lands on the leftmost column and for four it
	**	is the second, so it is the front left corner of the baseplate in every case a
	**	CrystAlien building actually uses.
	*/
	CELL origin = Coord_Cell(bld->Coord);

	/*
	**	The dock sits on the row the building does not claim.
	**
	**	setBlock stops a row short on purpose:
	**
	**	    var last = topLeftY + (height - 1);
	**	    while (y < last) tiles[x][y] = this;
	**
	**	-- so rows topLeftY through topLeftY+height-2 are taken and the bottom one is
	**	not.  And the dock is exactly that bottom row, topLeftY+height-1, which means
	**	it is free by construction: closestAvailable returns its very first candidate
	**	and the unit appears on the front of the building's own plate.  That is what
	**	puts them on the near-left corner of the baseplate every time, and why a stack
	**	of fighters gathers there.
	**
	**	Where that row is here is settled by Bib_And_Offset, which lays the plate at
	**	`cell += (Height()-1) * MAP_CELL_W`.  So the plate is the last row of the
	**	footprint, and the dock is the plate: a unit appearing anywhere else is
	**	either standing on the building or standing off the front of it.
	*/
	int width = bld->Class->Width() + 1;		/* what CrystAlien draws */
	int col = Cell_X(origin) + (width / 2) - 1;
	if (col < Cell_X(origin)) col = Cell_X(origin);
	CELL dock = XY_Cell(col, Cell_Y(origin) + bld->Class->Height() - 1);

	/*
	**	Aircraft are placed on top of one another deliberately -- see above -- so the only
	**	thing in their way is a building.  Everything else has to be able to stand there.
	*/
	bool flies = (obj != NULL && obj->What_Am_I() == RTTI_AIRCRAFT);

	for (int i = 0; i < (int)(sizeof(_ring) / sizeof(_ring[0])); i++) {
		int x = Cell_X(dock) + _ring[i].X;
		int y = Cell_Y(dock) + _ring[i].Y;

		if (x < Map.MapCellX || x >= Map.MapCellX + Map.MapCellWidth) continue;
		if (y < Map.MapCellY || y >= Map.MapCellY + Map.MapCellHeight) continue;

		CELL look = XY_Cell(x, y);

		/*
		**	What `!tiles[x][y]` means, as nearly as it can be meant here.
		**
		**	CrystAlien's grid holds buildings and the units that obstruct, and the dock
		**	is a tile the producing building draws over but does not claim -- so it is
		**	free, and the first candidate wins.  Asking Can_Enter_Cell instead refuses
		**	it, because as far as the engine is concerned the building is there.
		**
		**	So the building letting the unit out is not an obstacle to it.  Anything
		**	else standing there is, which is what keeps a queue spreading along the ring
		**	rather than stacking on one tile.
		*/
		BuildingClass * there = Map[look].Cell_Building();
		if (there != NULL && there != bld) continue;

		/*
		**	And the building itself is not something standing in the way.
		**
		**	Cell_Techno answers with whatever is in the cell, and on the dock that is
		**	the producing building -- a techno like any other.  So the dock was being
		**	rejected as occupied every time, by the very building trying to use it,
		**	and the search moved on to the first candidate after it, which is north.
		**	Which is behind.  Santa above the sleigh, infantry behind the camp: the
		**	same line both times.
		*/
		TechnoClass * sitting = Map[look].Cell_Techno();
		if (sitting == (TechnoClass *)bld) sitting = NULL;
		if (!flies && sitting != NULL) continue;
		if (!flies && there == NULL && obj != NULL &&
			obj->Can_Enter_Cell(look) != MOVE_OK) continue;

		/*
		**	Only when the answer changes, and remembered per building rather than
		**	globally.
		**
		**	One static for the whole map meant two stations taking turns defeated it
		**	completely -- and with five computer opponents there are five of them
		**	alternating, so it would have written a line for every call.  The same
		**	mistake the berth log made.
		*/
		/*
		**	Per building, not per building *type* -- sixth time for this mistake.
		**	
		**	The note above is right about two stations defeating one static and then
		**	makes the same error one line down: six houses each own a Command Base and
		**	they all share the BAGOOD slot, so their docks overwrite one another and
		**	every call writes a line.  A log of one session has BAGOOD alternating
		**	between 49,4 and 17,23 for the whole round -- two houses, taking turns,
		**	exactly as described.
		**	
		**	The thing that varies is the building.  Bounded because the heap is bigger
		**	than this array and an ID past the end simply goes unreported.
		*/
		static CELL _last[128];
		static bool _init = false;
		if (!_init) { _init = true; for (int k = 0; k < 128; k++) _last[k] = 0; }

		int slot = Buildings.ID(bld);
		if ((unsigned)slot < 128 && _last[slot] != look) {
			_last[slot] = look;
			CAC_Log("exit", "%s places its output at %d,%d (candidate %d of 37)",
						bld->Class->IniName, Cell_X(look), Cell_Y(look), i);
		}
		return(look);
	}

	/*
	**	Nothing was free.  CrystAlien hands back the tile it was asked about and places the
	**	unit there anyway, so a walled-in factory keeps producing.
	*/
	return(dock);
}


StructType CAC_Exit_Style(StructType building)
{
	if (CAC_Builds_Any(building, RTTI_UNITTYPE) ||
		CAC_Builds_Any(building, RTTI_INFANTRYTYPE) ||
		CAC_Builds_Any(building, RTTI_AIRCRAFTTYPE)) {
		return(STRUCT_BARRACKS);
	}
	return(STRUCT_NONE);
}


/***********************************************************************************************
 * CAC_Factory_Index -- Which production queue this type is built in.                          *
 *                                                                                             *
 *    Tiberian Dawn gives a house one queue per *kind* of thing -- one for vehicles, one for   *
 *    infantry, and so on -- so a house can only ever build one vehicle at a time whichever    *
 *    building is making it.  CrystAlien queues per building: Construction holds an item for   *
 *    each `home`, and a Miner coming out of the Command Base has nothing to do with a Trike   *
 *    coming out of the Vehicle Factory.  With one shared queue the Miner simply could not be  *
 *    started while the factory was busy, which is exactly how it behaved.                     *
 *                                                                                             *
 *    So a type is queued against its host building instead.  Per host *type* rather than per  *
 *    building: the sidebar has one entry per type and cannot show two of them, so a second    *
 *    Vehicle Factory adds speed, as it does in Tiberian Dawn, rather than a second queue.     *
 *                                                                                             *
 * OUTPUT:  a queue index, or -1 for anything that should use the engine's own.                *
 *=============================================================================================*/
int CAC_Factory_Index(RTTIType rtti, int id)
{
	if (id < 0) return(-1);

	StructType host = CAC_Host_Of(rtti, id);
	if (host == STRUCT_NONE) return(-1);

	/*
	**	The two factions queue separately as well -- a captured Alien foundry is
	**	its own production line -- so the side rides along in the code.
	*/
	int code = CAC_Code_Of(RTTI_BUILDINGTYPE, (int)host);
	if (code == CAC_CODE_NONE) return(-1);
	return(((code & ~CAC_CODE_EVIL) * 2) + ((code & CAC_CODE_EVIL) ? 1 : 0));
}


/***********************************************************************************************
 * CAC_Builds_Any -- Does this building produce anything at all?                               *
 *                                                                                             *
 *    Used for the factory bookkeeping and the sidebar, so a building that hosts a type the     *
 *    engine would not have given it still counts as a factory for that kind.                  *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Host_Output -- How many foot soldiers and how many fliers a building makes.              *
 *                                                                                             *
 *    Used to settle which tab a flier belongs in; see CAC_In_Infantry_Tab.                     *
 *=============================================================================================*/
static void CAC_Host_Output(StructType host, int &foot, int &air)
{
	foot = 0;
	air = 0;
	for (int i = 0; i < (int)(sizeof(CACHosts) / sizeof(CACHosts[0])); i++) {
		if (CACHosts[i].Host != host) continue;
		if (CACHosts[i].RTTI == RTTI_INFANTRYTYPE) foot++;
		if (CACHosts[i].RTTI == RTTI_AIRCRAFTTYPE) air++;
	}
}


bool CAC_Builds_Any(StructType building, RTTIType rtti)
{
	for (int i = 0; i < (int)(sizeof(CACHosts) / sizeof(CACHosts[0])); i++) {
		if (CACHosts[i].Host == building && CACHosts[i].RTTI == rtti) return(true);
	}
	return(false);
}

struct CACCarryEntry {
	RTTIType	RTTI;
	int			Type;
};

static CACCarryEntry const CACCarry[] = {
#include "CACCARRY.INC"
};


/***********************************************************************************************
 * CAC_Is_Carriable -- May a carrier pick this up?                                             *
 *                                                                                             *
 *    `carriable` in UnitStats, and not a synonym for "is it infantry" -- the Astro and Alien   *
 *    Fighters (UC) ride along too, while the Commander does not.  A carrier is never itself    *
 *    carriable, which is what keeps a Dropship in a selection from trying to board itself.     *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Fires_Both -- Does this one put up a laser and a rocket on the same shot?                  *
 *                                                                                             *
 *    `weapon` in UnitStats is a string, and two types carry "both": Santa and the Alien         *
 *    Commander.  It is not a choice between the two -- fireWeapon spawns one of each:            *
 *                                                                                             *
 *        if (type == "both") {                                                                  *
 *           this.weapons.push(new Weapon(this, shooter, "laser",  x-1, y-1, tx, ty, target));    *
 *           this.weapons.push(new Weapon(this, shooter, "rocket", x-1, y-1, tx, ty, target));    *
 *        }                                                                                     *
 *                                                                                             *
 *    and each Weapon carries dmg 25, so a shot from either of those two lands fifty.             *
 *                                                                                             *
 *    Tiberian Dawn has no such idea.  A secondary weapon is a *fallback*: TarComClass::AI tries  *
 *    Can_Fire(TarCom, 0) and only reaches Can_Fire(TarCom, 1) when the first is refused, and     *
 *    every other firing path in the conversion asks for weapon 0 by name.  Both of these types   *
 *    carry their two weapons with the same range and the same rate of fire, so the primary is    *
 *    practically never refused and the second weapon practically never fired.  They were doing   *
 *    half the damage the source gives them, which is most of why the Commander felt a poor       *
 *    match for a Switch Fighter.                                                                *
 *                                                                                             *
 *    Both weapons present is exactly the "both" set, so the table needs nothing new: nothing     *
 *    else on the roster is given a secondary.                                                    *
 *=============================================================================================*/
bool CAC_Fires_Both(TechnoClass const *object)
{
	if (object == NULL) return(false);

	TechnoTypeClass const *type = object->Techno_Type_Class();
	if (type == NULL || !CAC_Is_CrystAlien_Type(type->IniName)) return(false);

	return(type->Primary != WEAPON_NONE && type->Secondary != WEAPON_NONE);
}


/***********************************************************************************************
 * CAC_Is_Own_Carrier -- Is this one of ours that carries passengers?                            *
 *                                                                                             *
 *    `carrier` in UnitStats, which the generated type carries as IsTransporter and             *
 *    CAC_Max_Passengers as the capacity.  Two things have it: the Hyper Carrier and the Alien   *
 *    Dropship (UH), and the Crystal Reaper (UR_good) with a capacity of one.                    *
 *                                                                                             *
 *    It exists because UnitClass::Receive_Message compares against UNIT_APC by hand, and a      *
 *    named local cannot be declared there -- the case labels below it would jump the            *
 *    initialisation.                                                                            *
 *=============================================================================================*/
bool CAC_Is_Own_Carrier(TechnoClass const *object)
{
	if (object == NULL) return(false);

	TechnoTypeClass const *type = object->Techno_Type_Class();
	if (type == NULL || !CAC_Is_CrystAlien_Type(type->IniName)) return(false);

	return(type->IsTransporter != 0);
}


bool CAC_Is_Carriable(ObjectClass const * obj)
{
	if (obj == NULL || !((ObjectClass *)obj)->Is_Techno()) return(false);

	TechnoTypeClass const * type = ((TechnoClass *)obj)->Techno_Type_Class();
	if (type == NULL) return(false);

	RTTIType want = obj->What_Am_I();
	if (want == RTTI_UNIT)     want = RTTI_UNITTYPE;
	if (want == RTTI_INFANTRY) want = RTTI_INFANTRYTYPE;
	if (want == RTTI_AIRCRAFT) want = RTTI_AIRCRAFTTYPE;

	int slot = CAC_Type_Index_Of_Type(type);
	if (slot < 0) return(false);

	for (int i = 0; i < (int)(sizeof(CACCarry) / sizeof(CACCarry[0])); i++) {
		if (CACCarry[i].RTTI == want && CACCarry[i].Type == slot) return(true);
	}
	return(false);
}


struct CACHomingEntry {
	RTTIType	RTTI;
	int			Type;
	StructType	Home;
};

static CACHomingEntry const CACHoming[] = {
#include "CACHOMING.INC"
};

/***********************************************************************************************
 * CAC_Homes_To -- Which building this type is tethered to, if any.                            *
 *                                                                                             *
 *    `boomerang` types never wander.  UnitNav ends every completed path with                   *
 *                                                                                             *
 *        if (this.parent.stats.boomerang) this.returnHome();                                   *
 *                                                                                             *
 *    and returnHome voyages to `stats.home.stats.dockX/dockY` -- the Radar Station for the      *
 *    Astro Fighter, the Sonar Station for the Strike Fighter.  Arriving tops the payload back  *
 *    up by one (`weaponPayload = min(++weaponPayload, max)`), which is why they can only sortie *
 *    as often as they can get home.                                                            *
 *                                                                                             *
 * OUTPUT:  STRUCT_NONE for anything that holds station instead.                                *
 *=============================================================================================*/
StructType CAC_Homes_To(RTTIType rtti, int type)
{
	RTTIType want = rtti;
	if (rtti == RTTI_UNIT)     want = RTTI_UNITTYPE;
	if (rtti == RTTI_INFANTRY) want = RTTI_INFANTRYTYPE;
	if (rtti == RTTI_AIRCRAFT) want = RTTI_AIRCRAFTTYPE;

	for (int i = 0; i < (int)(sizeof(CACHoming) / sizeof(CACHoming[0])); i++) {
		if (CACHoming[i].RTTI == want && CACHoming[i].Type == type) return(CACHoming[i].Home);
	}
	return(STRUCT_NONE);
}


/***********************************************************************************************
 * CAC_Is_Boomerang -- Does this aircraft live tethered to a home building?                     *
 *=============================================================================================*/
bool CAC_Is_Boomerang(AircraftType type)
{
	return(CAC_Homes_To(RTTI_AIRCRAFTTYPE, (int)type) != STRUCT_NONE);
}


/***********************************************************************************************
 * CAC_Home_Building -- The tethered unit's own home, or NULL if it has none left.             *
 *                                                                                             *
 *    `stats.home` is resolved once, when the unit is created, and Unit.think then watches it:  *
 *                                                                                              *
 *        if (this.stats.boomerang && !this.stats.home.active) this.destroy();                  *
 *                                                                                              *
 *    Resolved by search here rather than remembered, so that a fighter whose station is        *
 *    rebuilt adopts the new one instead of dying with the old.                                 *
 *=============================================================================================*/
BuildingClass *CAC_Home_Building(TechnoClass const *object)
{
	if (object == NULL || object->House == NULL) return(NULL);

	TechnoTypeClass const *type = object->Techno_Type_Class();
	if (type == NULL) return(NULL);

	StructType home = CAC_Homes_To(object->What_Am_I(), CAC_Type_Index_Of_Type(type));
	if (home == STRUCT_NONE) return(NULL);

	BuildingClass *best = NULL;
	int bestdist = -1;
	for (int i = 0; i < Buildings.Count(); i++) {
		BuildingClass *b = Buildings.Ptr(i);
		if (b == NULL || !b->IsActive || b->IsInLimbo) continue;
		if (b->House != object->House) continue;
		if (*b != home) continue;
		int dist = object->Distance(b);
		if (bestdist == -1 || dist < bestdist) {
			bestdist = dist;
			best = b;
		}
	}
	return(best);
}


/***********************************************************************************************
 * CAC_Check_Home -- Kill a tethered unit whose station is gone.                               *
 *                                                                                             *
 *    Unit.think, verbatim in effect: a boomerang whose home is no longer active destroys       *
 *    itself.  It is what stops a Radar Station being a free permanent airbase.                 *
 *=============================================================================================*/
void CAC_Check_Home(TechnoClass *object)
{
	if (object == NULL || object->IsInLimbo || !object->IsActive) return;

	TechnoTypeClass const *type = object->Techno_Type_Class();
	if (type == NULL) return;
	/*
	**	A miner with nowhere to take it does the same thing.
	**
	**	The source destroys two kinds of unit that have lost their building, and
	**	this only had the first.  Unit.think, every frame:
	**
	**	    if (this.stats.boomerang && !this.stats.home.active) this.destroy();
	**
	**	and UnitNav.feed, the moment a hold fills:
	**
	**	    if (!home.active || home.team != this.team) { this.parent.destroy(); return; }
	**
	**	The second is why nobody has to comb the map for a last miner after the
	**	base that built it is rubble -- it takes itself off when it next has
	**	something to deliver.  Note the second half of that test: a home that has
	**	changed hands counts as gone, so a captured Command Base does not inherit
	**	the other side's miners.
	*/
	if (CAC_Is_Miner(object->What_Am_I(), CAC_Type_Index_Of_Type(type))) {
		bool somewhere = false;
		for (int i = 0; i < Buildings.Count() && !somewhere; i++) {
			BuildingClass const * b = Buildings.Ptr(i);
			if (b == NULL || !b->IsActive || b->IsInLimbo || b->Class == NULL) continue;
			if (b->House != object->House) continue;
			if (CAC_Is_Refinery(b->Class->Type)) somewhere = true;
		}

		/*
		**	The moment there is nowhere to deliver, not the next time it fills.
		**
		**	The source waits for a full hold, because that is where the test sits:
		**
		**	    if(this.parent.cargo >= this.stats.maxCargo) {
		**	       ...
		**	       if(!home.active || home.team != this.team) { this.parent.destroy(); }
		**
		**	-- one arm of UnitNav.endMove's feeding branch, reached only when a
		**	mouthful takes the hold to maxCargo.  A boomerang is checked every frame
		**	in Unit.think, so the two are not symmetrical in CrystAlien either.
		**
		**	Asked for directly: destroying a base should take its miners with it the
		**	way destroying a Radar Station takes its fighters, rather than leaving
		**	them to finish a load first.  A half-full miner could otherwise wander
		**	for a long time, and one that can no longer reach crystal never dies at
		**	all.  A departure from the source's timing and nothing else -- what
		**	counts as home is unchanged, and it still takes losing every
		**	headquarters, since the Ops Ship and the Hive accept a miner too.
		**	See CAC_Is_Refinery for why that fallback is there; without it, losing
		**	an Eagle Command that nothing can rebuild is a softlock.
		*/
		if (!somewhere && object->What_Am_I() == RTTI_UNIT) {
			CAC_Log_Volatile("berth", "%s has nowhere left to deliver -- destroying it",
								  type->IniName);
			int damage = object->Strength * 4 + 64;
			object->Take_Damage(damage, 0, WARHEAD_HE, NULL);
		}
		return;
	}

	if (CAC_Homes_To(object->What_Am_I(), CAC_Type_Index_Of_Type(type)) == STRUCT_NONE) return;

	if (CAC_Home_Building(object) == NULL) {

		/*
		**	One a frame, and only one.
		**	
		**	Losing the last Radar Station orphans every Fighter homed to it at once,
		**	and each one destroyed here runs the full death: a CACBOOM, a fireball, a
		**	crater stamped on the ground, and the craft handed back to the heap from
		**	inside the loop that is walking it.  Eight of those in one frame is the
		**	most violent thing this mod can do to the object lists, and every crash on
		**	this project has come from that neighbourhood -- an aircraft dying, then a
		**	cell walking a list that still holds it.
		**	
		**	Spreading them costs nothing anyone can see: a Fighter that outlives its
		**	station by a fraction of a second still dies, and a chain of explosions
		**	reads better than one flat bang.  Not the source's timing -- Unit.think
		**	checks every craft every frame -- but the source is not walking a fixed
		**	heap while it does it.
		*/
		static long _killed = -1;
		if ((long)Frame == _killed) return;
		_killed = (long)Frame;

		CAC_Log_Volatile("berth", "%s has no station left -- destroying it",
								 type->IniName);
		/*
		**	Enough damage to actually kill it, which its own strength is not.
		**	
		**	AircraftClass::Take_Damage opens with
		**	
		**	    if (Altitude) { damage /= 2; }
		**	
		**	and armour modifies the warhead on top of that.  Handing it exactly the
		**	craft's strength therefore removed about half, the next frame removed half
		**	of the remainder, and so on down to one -- at which point the halving
		**	rounds the damage to nothing and the craft is immortal.
		**	
		**	A log of one six-player round carries this line three thousand nine hundred
		**	and thirty-three times.  That is not thousands of Fighters; it is a handful
		**	that could not be killed, sitting at one health where nobody would look --
		**	and the same round ends with four houses holding no buildings at all while
		**	the game runs on, because a house with a unit left is not defeated.  That
		**	is the whole of the softlock.
		*/
		int damage = object->Strength * 4 + 64;
		object->Take_Damage(damage, 0, WARHEAD_HE, NULL);
	}
}


/***********************************************************************************************
 * CAC_Picks_Own_Target -- May this object go looking for something to shoot?                  *
 *                                                                                             *
 *    Level.engageEnemy, before anything else it does:                                          *
 *                                                                                             *
 *        if (this.stats.boomerang && !this.target) return;                                     *
 *                                                                                             *
 *    A tethered fighter fires only at what it was told to fire at.  Left alone it shuttles     *
 *    between its station and wherever it was last sent, which is what keeps it near the        *
 *    Radar Station instead of drifting off to hunt.  The computer opponent gets round it the   *
 *    same way a player does, by handing it a target -- HAL.handle assigns one once the         *
 *    payload is full and its own timer comes round.                                            *
 *=============================================================================================*/
bool CAC_Picks_Own_Target(TechnoClass const *object)
{
	if (object == NULL) return(true);
	TechnoTypeClass const *type = object->Techno_Type_Class();
	if (type == NULL) return(true);
	return(CAC_Homes_To(object->What_Am_I(), CAC_Type_Index_Of_Type(type)) == STRUCT_NONE);
}


/***********************************************************************************************
 * CAC_House_Target -- The one building a house's Fighters are all pointed at.                  *
 *                                                                                             *
 *    Level.think, every three hundred and twenty frames:                                       *
 *                                                                                             *
 *        this.friendlyTarget = false;                                                          *
 *        do { _loc6_ = this.buildings[random(this.buildings.length)]; ... }                     *
 *        while (tries < this.buildings.length * 2 && (!_loc6_.friend || !_loc6_.stats.threat)); *
 *        if (_loc6_.friend && _loc6_.stats.threat) this.friendlyTarget = _loc6_;                *
 *                                                                                             *
 *    A random enemy building that has a threat rating, re-rolled on a timer, and shared: the    *
 *    HAL hands the same one to every boomerang it releases.  So a house's Fighters arrive in    *
 *    ones and twos at the same building rather than each picking its own, and the target        *
 *    wanders every twenty seconds or so whether or not the old one died.                        *
 *                                                                                             *
 *    Random rather than best on purpose.  Ranking by threat would send every Fighter at the     *
 *    Defence Station every time; the roll is what makes a squadron turn up somewhere            *
 *    inconvenient instead.                                                                      *
 *                                                                                             *
 *    Cached per house because the roll has to persist between calls -- re-rolling on each ask   *
 *    would give every Fighter a different target and lose the whole point of sharing one.       *
 *=============================================================================================*/
BuildingClass * CAC_House_Target(HouseClass const * house)
{
	if (house == NULL || house->Class == NULL) return(NULL);

	/*	320 CrystAlien frames, in this game's ticks. */
	enum { REROLL = (TICKS_PER_SECOND * 320) / 23 };

	/*
	**	Held as a TARGET rather than a heap index.
	**	
	**	Buildings is a fixed heap whose slots are reused, so index twelve now may be
	**	a different building in a minute, or somebody else's, or a free slot.  The
	**	checks below would catch most of that, and "most" is the wrong standard for
	**	something re-read on every sortie for the length of a round.  A TARGET
	**	carries the object's identity and As_Building validates it.
	*/
	static TARGET _held[HOUSE_COUNT];
	static long _when[HOUSE_COUNT];
	static bool _init = false;
	if (!_init) {
		_init = true;
		for (int h = 0; h < HOUSE_COUNT; h++) { _held[h] = TARGET_NONE; _when[h] = -99999; }
	}

	int hi = (int)house->Class->House;
	if ((unsigned)hi >= HOUSE_COUNT) return(NULL);

	/*
	**	The held one, while it is still a going concern.  A target that has been
	**	destroyed or taken is dropped at once rather than waiting for the timer.
	*/
	if ((long)Frame - _when[hi] < (long)REROLL && Target_Legal(_held[hi])) {
		BuildingClass * held = As_Building(_held[hi]);
		if (held != NULL && held->IsActive && !held->IsInLimbo && held->Strength > 0 &&
			 held->House != NULL && !house->Is_Ally(held->House)) {
			return(held);
		}
	}

	_when[hi] = (long)Frame;
	_held[hi] = TARGET_NONE;

	/*
	**	The source's own draw: pick at random, twice as many attempts as there are
	**	buildings, and give up if none of them answered.
	*/
	int count = Buildings.Count();
	if (count <= 0) return(NULL);

	for (int tries = 0; tries < count * 2; tries++) {
		int at = Sim_Random_Pick(0, count - 1);
		BuildingClass * b = Buildings.Ptr(at);
		if (b == NULL || !b->IsActive || b->IsInLimbo || b->Strength == 0) continue;
		if (b->House == NULL || house->Is_Ally(b->House)) continue;
		if (b->Class == NULL || !b->Class->IsLegalTarget) continue;
		if (CAC_Pizza_Protected(b)) continue;	/* cannot be hurt -- see CAC_Pizza_Protected */
		if (CAC_Threat_Rating(RTTI_BUILDINGTYPE, (int)b->Class->Type) <= 0) continue;

		_held[hi] = b->As_Target();
		return(b);
	}
	return(NULL);
}


/***********************************************************************************************
 * CAC_Holds_Fire_Landing -- A flier coming down stops shooting, unless it is the Switch Fighter.*
 *                                                                                             *
 *    engageEnemy, three lines before it looks for anything to shoot at:                        *
 *                                                                                             *
 *        if (this.stats.flying && this.nav.landerPerc > 25 && !this.stats.transformer)         *
 *            return undefined;                                                                 *
 *                                                                                             *
 *    landerPerc is how far down the craft has come, nought at flight level and a hundred on    *
 *    the ground, so a quarter of the way down is where a flier stops being able to fire.  The  *
 *    Switch Fighter is exempt, and it is the only aircraft in the game with `transformer`      *
 *    set -- settling on the ground is the whole point of it, and it is expected to go on        *
 *    fighting once it is there.                                                                *
 *                                                                                             *
 *    Which makes an Astro Fighter that is landing genuinely defenceless: it cannot shoot, and  *
 *    a laser can hit it as soon as it touches down.  Both halves of that are the source's.     *
 *                                                                                             *
 *    Altitude runs the other way from landerPerc -- FLIGHT_LEVEL down to nought -- so a        *
 *    quarter descended is three quarters of the way up.                                        *
 *=============================================================================================*/
bool CAC_Holds_Fire_Landing(AircraftClass const * craft)
{
	if (craft == NULL || craft->Class == NULL) return(false);
	if (!CAC_Is_CrystAlien_Type(craft->Class->IniName)) return(false);

	/*	The exemption, and the only type that has it. */
	if (CAC_Is_Transformer(*craft)) return(false);

	/*	Already down is not "coming down" -- a grounded flier is out of this rule. */
	if (craft->Altitude <= 0) return(false);

	return(craft->Altitude < (AircraftClass::FLIGHT_LEVEL * 3) / 4);
}


/***********************************************************************************************
 * CAC_Strafe -- A boomerang shoots what it passes, without stopping to do it.                  *
 *                                                                                             *
 *    engageEnemy is called from the unit's think every frame and is independent of movement.   *
 *    It picks the highest-threat enemy inside weaponRange -- a plain distance test, nothing    *
 *    about facing or arriving -- and fires:                                                    *
 *                                                                                             *
 *        _loc5_ = distance(this.posX, this.posY, _loc3_.posX, _loc3_.posY);                    *
 *        if (_loc5_ < this.stats.weaponRange) { target = _loc3_; ... }                         *
 *                                                                                             *
 *    So a craft crossing a base fires the moment something comes into range and keeps flying.  *
 *    Tiberian Dawn does the opposite: MISSION_ATTACK flies an aircraft *to* its target and     *
 *    holds station over it, which is what "they wait to fire until fully positioned above the  *
 *    target" is -- the engine's attack run, not CrystAlien's.                                  *
 *                                                                                             *
 *    This is the missing half.  It fires and returns; it never touches NavCom, TarCom or the   *
 *    mission, so nothing about where the craft is going changes.  Combined with a sortie that  *
 *    flies *through* rather than *at* (see CAC_AI_Sortie) that is the whole of a strafing run: *
 *    out, shooting whatever it passes, and home when the path runs out.                        *
 *                                                                                             *
 *    Ranked by threat rather than by distance, as the source does -- a Defence Station passed  *
 *    at the same moment as a Trike is the one worth the shot.                                  *
 *=============================================================================================*/
void CAC_Strafe(AircraftClass * craft)
{
	if (craft == NULL || craft->Class == NULL) return;
	if (!craft->IsActive || craft->IsInLimbo || craft->Strength == 0) return;
	if (!CAC_Is_Boomerang(*craft)) return;

	/*
	**	Its own target, and only ever its own.
	**
	**	engageEnemy opens with
	**
	**	    if (this.stats.boomerang && !this.target) return undefined;
	**
	**	so a boomerang with nothing assigned does not shoot, and one with a target
	**	shoots that and nothing else -- it never takes the opportunistic scan the
	**	rest of the roster runs.  An earlier version of this fired at whatever came
	**	into range, which is livelier and is not CrystAlien: it turned a Fighter
	**	into a gunship that cleared everything it flew over.
	**
	**	What this exists for is the timing.  engageEnemy runs from the unit's think
	**	every frame and is independent of movement, so the shot happens while the
	**	craft is still travelling.  Tiberian Dawn would otherwise fly it to the
	**	target and hold station before firing, which is the hovering that was
	**	reported.
	*/
	if (!Target_Legal(craft->TarCom)) return;

	/*	Coming down means holding fire -- see CAC_Holds_Fire_Landing. */
	if (CAC_Holds_Fire_Landing(craft)) return;

	/*
	**	Empty: drop the target and go home.  This is the cycle, not a detail.
	**	
	**	    if (this.weaponPayload <= 0) {
	**	        this.target = false;
	**	        this.nav.returnHome();
	**	        return undefined;
	**	    }
	**	
	**	and the HAL will not hand out another target until `weaponPayload ==
	**	maxWeaponPayload` again.  So the whole life of a Fighter is: rearmed at its
	**	station, given the house's target, out to spend the payload, home to fill
	**	it.  Without this the payload runs dry and nothing ever clears the target,
	**	so the sortie's own re-entry guard refuses that craft for the rest of the
	**	round -- it would sit at its plate with a target it can never shoot and
	**	never be released again.  Stuck, quietly, one Fighter at a time.
	**	
	**	A craft whose target died needs no handling: Target_Legal answers false for
	**	a dead object, which is the other place the source clears it
	**	(`if (this.target && !this.target.active) this.target = false`).
	*/
	if (craft->Class->MaxAmmo > 0 && craft->Ammo <= 0) {
		craft->Assign_Target(TARGET_NONE);
		CAC_Return_Home(craft);
		return;
	}

	if (craft->Can_Fire(craft->TarCom, 0) != FIRE_OK) return;

	craft->Fire_At(craft->TarCom, 0);
}

/***********************************************************************************************
 * CAC_Opportunity_Fire -- Everything else shoots what comes into range, and keeps going.        *
 *                                                                                             *
 *    engageEnemy is called from Unit.think every frame and never touches the navigation.  It    *
 *    walks every enemy building and then every enemy unit, takes the highest `threat` inside    *
 *    weaponRange on a plain distance test, and fires:                                          *
 *                                                                                             *
 *        _loc5_ = distance(this.posX, this.posY, _loc2_.posX, _loc2_.posY);                     *
 *        if (_loc5_ < this.stats.weaponRange) { target = _loc2_; _loc4_ = target.stats.threat; }*
 *        ...                                                                                   *
 *        this.shoot(target);                                                                   *
 *                                                                                             *
 *    and shoot() aims by assigning the angle in the same frame it fires --                      *
 *    `this.nav.nextAngle = this.angle = _loc2_` -- so there is no tolerance to satisfy, no      *
 *    rotation to wait for, and no reason to stop.  A CrystAlien unit crossing a battlefield     *
 *    shoots as it goes.                                                                        *
 *                                                                                             *
 *    Tiberian Dawn only fires out of MISSION_ATTACK and MISSION_GUARD, so a unit under a move   *
 *    order drives past enemies without a shot, and one told to attack walks into range, stops,  *
 *    and only then fires.  CAC_Snap_Fire already lifted the three conditions inside Can_Fire;   *
 *    this is the other half, the one that makes the shot happen at all while travelling.        *
 *                                                                                             *
 *    A target it was *given* is left to the mission, which is engageEnemy's own order of        *
 *    preference (`if (this.target) target = this.target;`).  And the target is not recorded      *
 *    here: for a player's unit `this.hal` is undefined, so the scan fires and assigns nothing,  *
 *    which is why shooting something in passing never diverts a CrystAlien unit from its        *
 *    orders.                                                                                   *
 *                                                                                             *
 *    Boomerangs are excluded -- engageEnemy opens by refusing them a scan (`if (boomerang &&    *
 *    !this.target) return`) and CAC_Strafe is their half of this.                               *
 *=============================================================================================*/
static void CAC_Try_Shot(TechnoClass *object, TechnoClass *victim, int &best, TechnoClass *&pick)
{
	if (victim == NULL || !victim->IsActive || victim->IsInLimbo) return;
	if (victim->Strength <= 0) return;
	if (victim->House == object->House) return;
	if (object->House->Is_Ally(victim->House)) return;		/* allies are not targets */

	/*
	**	And a headquarters in Pizza Mode is not a target for anybody.
	**
	**	Put here rather than only at the callers because this is the one gate every
	**	shot goes through.  Take_Damage answers RESULT_NONE for a protected
	**	headquarters, so a unit that picks one stands in front of it firing at a
	**	wall until something else comes along -- which is what was being seen, and
	**	which the stock Evaluate_Object refusal could not stop, because none of
	**	this conversion's own target pickers ask Evaluate_Object.
	*/
	if (victim->What_Am_I() == RTTI_BUILDING &&
		 CAC_Pizza_Protected((BuildingClass *)victim)) {
		return;
	}

	int score = CAC_Target_Score(victim);
	if (score < best) return;

	if (object->Distance(victim) >= object->Weapon_Range(0)) return;

	best = score;
	pick = victim;
}


/***********************************************************************************************
 * CAC_Fire_Report -- Why a flier that should be shooting is not.                                *
 *                                                                                             *
 *    engageEnemy runs from every unit's think, every frame, and asks nothing about altitude    *
 *    except one line:                                                                          *
 *                                                                                             *
 *        if (this.stats.flying && this.nav.landerPerc > 25 && !this.stats.transformer) return; *
 *                                                                                             *
 *    -- so a Switch Fighter engages at any height and on the ground, on its own, with nobody   *
 *    clicking.  The machinery for that is all here and wired into AircraftClass::AI, and it is *
 *    reported as not happening, so one of the half-dozen early returns below is taken every    *
 *    pass and they are indistinguishable from outside.  This names the one that was.           *
 *=============================================================================================*/
static void CAC_Fire_Report(TechnoClass const *object, char const *why)
{
	if (!CAC_Logging() || object == NULL || why == NULL) return;
	if (object->What_Am_I() != RTTI_AIRCRAFT) return;

	AircraftClass const *craft = (AircraftClass const *)object;
	if (!CAC_Is_Transformer(*craft)) return;		/* the one type this is asked about */
	if ((Frame % TICKS_PER_SECOND) != 0) return;

	CAC_Log_Volatile("hold", "%s at %d,%d altitude %d mission %d: %s",
						  craft->Class->IniName,
						  Cell_X(Coord_Cell(craft->Center_Coord())),
						  Cell_Y(Coord_Cell(craft->Center_Coord())),
						  craft->Altitude, (int)craft->Mission, why);
}


void CAC_Opportunity_Fire(TechnoClass *object)
{
	if (object == NULL || !object->IsActive || object->IsInLimbo) return;
	if (object->Strength <= 0 || object->House == NULL) return;

	TechnoTypeClass const *type = object->Techno_Type_Class();
	if (type == NULL || !CAC_Is_CrystAlien_Type(type->IniName)) return;
	if (type->Primary == WEAPON_NONE) return;

	/*	Theirs is CAC_Strafe's; see the note above. */
	if (object->What_Am_I() == RTTI_AIRCRAFT) {
		if (CAC_Is_Boomerang(*(AircraftClass *)object)) return;
		if (CAC_Holds_Fire_Landing((AircraftClass *)object)) {
			CAC_Fire_Report(object, "holding fire on the way down");
			return;
		}
	}

	/*
	**	Something it was told to shoot takes precedence, and the mission is already
	**	shooting it.  `if (this.target) target = this.target;` before the scan.
	*/
	if (Target_Legal(object->TarCom)) {

		/*
		**	For a ground unit the mission really does own the shot: TarComClass::AI
		**	lines the turret up and fires at TarCom every pass, whatever mission the
		**	unit is on.
		**
		**	An aircraft has no TarComClass in its ancestry.  The only place one ever
		**	fires at its TarCom is Mission_Attack -- and ours returns out of
		**	Mission_Guard on its first line, deliberately, so that a CrystAlien flier
		**	holds station instead of being parked by the two lines that seat a
		**	helicopter.  So a Switch Fighter that has a target and is guarding sat
		**	there holding it and never shot: this routine stood aside for a mission
		**	that was never going to take the shot.
		**
		**	engageEnemy does not care which of those it is.  `if (this.target) target
		**	= this.target;` and then it fires, from wherever the unit happens to be
		**	and at whatever altitude -- the one exemption in the whole routine is the
		**	landing test above, which excuses the transformer by name.
		*/
		if (object->What_Am_I() != RTTI_AIRCRAFT || object->Mission == MISSION_ATTACK) {
			CAC_Fire_Report(object, "already has a target, so the mission owns the shot");
			return;
		}

		if (object->Arm == 0 && object->Can_Fire(object->TarCom, 0) == FIRE_OK) {
			object->Fire_At(object->TarCom, 0);
		} else {
			CAC_Fire_Report(object, "has a target but cannot fire at it yet");
		}
		return;
	}

	/*	The cooldown, which Can_Fire reports as FIRE_REARM.  weaponCharge. */
	if (object->Arm != 0) {
		CAC_Fire_Report(object, "rearming");
		return;
	}

	int best = 1;					/* `_loc4_ = 1` -- threat zero is not worth a shot */
	TechnoClass *pick = NULL;

	/*
	**	Buildings first and then units, so that a unit wins a tie -- the source
	**	compares with `>=` and runs the two loops in that order.
	*/
	for (int i = 0; i < Buildings.Count(); i++) {
		BuildingClass *b = Buildings.Ptr(i);

		/*
		**	A player's unit does not pick an unarmed building out of the air:
		**	`if (!(this.friend && !_loc3_.stats.weapon))`.  It will still shoot one
		**	it is ordered to, which goes through the mission and not through here.
		*/
		if (b != NULL && object->House->IsHuman && b->Class != NULL &&
			 b->Class->Primary == WEAPON_NONE) continue;

		/*
		**	And nobody wastes a shot on a headquarters in Pizza Mode.  Take_Damage
		**	answers RESULT_NONE for one, so a unit that picks it out of the air
		**	stands there firing at a wall for as long as it is in range.
		*/
		if (CAC_Pizza_Protected(b)) continue;

		CAC_Try_Shot(object, b, best, pick);
	}
	for (int i = 0; i < Units.Count(); i++)     CAC_Try_Shot(object, Units.Ptr(i), best, pick);
	for (int i = 0; i < Infantry.Count(); i++)  CAC_Try_Shot(object, Infantry.Ptr(i), best, pick);
	for (int i = 0; i < Aircraft.Count(); i++)  CAC_Try_Shot(object, Aircraft.Ptr(i), best, pick);

	if (pick == NULL) {
		CAC_Fire_Report(object, "nothing in range worth shooting");
		return;
	}

	TARGET shot = pick->As_Target();
	FireErrorType err = object->Can_Fire(shot, 0);
	if (err != FIRE_OK) {
		if (CAC_Logging() && object->What_Am_I() == RTTI_AIRCRAFT) {
			char note[64];
			sprintf(note, "picked a target and Can_Fire answered %d", (int)err);
			CAC_Fire_Report(object, note);
		}
		return;
	}

	object->Fire_At(shot, 0);
}


/***********************************************************************************************
 * CAC_Return_Home -- Send a tethered craft back the moment it has nothing to do.               *
 *                                                                                             *
 *    UnitNav.handle, at the end of every completed path:                                       *
 *                                                                                             *
 *        if (this.parent.stats.boomerang) this.returnHome();                                   *
 *                                                                                             *
 *    which is stronger than Tiberian Dawn's idea of going idle.  A helicopter told to move     *
 *    somewhere arrives and holds station there; a boomerang arrives, and turns straight round.  *
 *    Waiting for Enter_Idle_Mode was not enough -- a craft that has arrived and is guarding is  *
 *    not idle as far as the engine is concerned, so it sat where it was sent.                   *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Contain_Flier -- Nothing of ours leaves the map, whatever told it to.                    *
 *                                                                                             *
 *    CrystAlien has no off-map.  `arena.tiles` is the world, voyage() paths inside it, and a   *
 *    unit that reaches an edge stops at it; there is no retreat, no landing zone beyond the    *
 *    border, and no reinforcement that flies in from one.  Tiberian Dawn has all three, and    *
 *    every one of them is reachable by an aircraft: MISSION_RETREAT, Exit_Map, Good_LZ picking *
 *    a spot at the map edge when a house owns no helipad, and plain Physics carrying a craft   *
 *    forward on PrimaryFacing while nothing is steering it.                                    *
 *                                                                                             *
 *    Each of those has been closed where it was found, and Switch Fighters still went over the *
 *    edge -- so this is the backstop rather than another guess at which one it was.  A craft   *
 *    outside the playable rectangle is turned round and sent to the nearest cell inside it,    *
 *    and the line it writes says what mission it was on when it left, which is the thing that  *
 *    will finally name the culprit.                                                            *
 *                                                                                             *
 *    Loaners are exempt: those really are reinforcements that arrive from off-map and leave    *
 *    the same way, and the campaign depends on it.                                             *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_On_Map -- Is this cell somewhere a unit is allowed to be?                                *
 *                                                                                             *
 *    The playable rectangle, which is not the cell array: a map declares MapCellX/Y and a      *
 *    width and height inside a 64x64 grid, and everything outside that is border the player    *
 *    never sees.  Map.In_Radar answers a different question and answers yes out there.          *
 *=============================================================================================*/
bool CAC_On_Map(CELL cell)
{
	if ((unsigned)cell >= MAP_CELL_TOTAL) return(false);

	int x = Cell_X(cell);
	int y = Cell_Y(cell);

	return(x >= Map.MapCellX && x < Map.MapCellX + Map.MapCellWidth &&
			 y >= Map.MapCellY && y < Map.MapCellY + Map.MapCellHeight);
}


void CAC_Contain_Flier(AircraftClass *craft)
{
	if (craft == NULL || craft->Class == NULL) return;
	if (!craft->IsActive || craft->IsInLimbo) return;
	if (!CAC_Is_CrystAlien_Type(craft->Class->IniName)) return;

	/*
	**	Loaners included, which they were not.
	**
	**	IsALoaner means "the house did not pay for this and does not keep it" --
	**	a reinforcement flying in along a delivery path, and excluding one is
	**	right for that.  But a building's survivors are loaners too, and a Viper
	**	handed out by a refunded Experiment Lab is one nobody ordered, nobody is
	**	watching, and nothing was containing: the report is a survivor found at
	**	the far right of the map that its owner did not know he had.
	**
	**	A delivery is IsALoaner and in a Team and on a MISSION_MOVE down its
	**	entry path, so that is what is excused here rather than the flag itself.
	*/
	if (craft->IsALoaner && craft->Team != NULL) return;

	CELL cell = Coord_Cell(craft->Center_Coord());
	int x = Cell_X(cell);
	int y = Cell_Y(cell);

	int left   = Map.MapCellX;
	int top    = Map.MapCellY;
	int right  = left + Map.MapCellWidth - 1;
	int bottom = top + Map.MapCellHeight - 1;

	/*
	**	The top edge is not where the top edge is, for something in the air.
	**
	**	A flier is drawn its altitude above the cell it is over, so one sitting on
	**	the map's first row has its body off the top of the world and only its
	**	shadow, which is drawn on the ground, left in view.  The client points at
	**	the body, so there is nothing to click: "can't select infiltrators/switch
	**	fighters floating past the world border at the very top, I can only see
	**	their shadow".
	**
	**	So the boundary this routine enforces is lifted by however many cells of
	**	altitude the craft is carrying.  A craft on the ground carries none and is
	**	bounded exactly as before.
	*/
	int lift = (craft->Altitude > 0)
					 ? ((craft->Altitude + (CELL_PIXEL_H - 1)) / CELL_PIXEL_H) : 0;
	if (lift > 0) top += lift;
	if (top > bottom) top = bottom;

	if (x >= left && x <= right && y >= top && y <= bottom) return;

	CELL back = XY_Cell(Bound(x, left, right), Bound(y, top, bottom));
	if (!Map.In_Radar(back)) return;

	/*
	**	Damped, because the first version of this wrote a line a frame.
	*/
	static long _said = -99999;
	if ((long)Frame - _said > TICKS_PER_SECOND) {
		_said = (long)Frame;
		CAC_Log_Volatile("drift", "%s (%s) left the map at %d,%d on mission %d -- put back at %d,%d",
							  craft->Class->IniName,
							  (craft->House != NULL && craft->House->IsHuman) ? "player" : "computer",
							  x, y, (int)craft->Mission, Cell_X(back), Cell_Y(back));
	}

	/*
	**	Put back, not asked to come back.
	**
	**	The first version handed it a destination and MISSION_MOVE, and the log of
	**	the round that followed is a hundred and eighteen lines of one JetPack
	**	Explorer sitting at 61,45 -- one cell past the right-hand edge -- being told
	**	to move to 60,45 every frame and not going.  Mission 2 in every line after
	**	the first is this routine's own order, re-issued and never carried out.
	**
	**	Something outside is still pushing it east, and it wins the argument because
	**	Physics moves a craft along PrimaryFacing every frame while a mission takes
	**	several to turn one round.  Asking politely cannot be made to work against
	**	that.  It is already off ground it is allowed to be on, so it is moved back
	**	onto the nearest legal cell and stopped there -- speed to nothing, orders
	**	cleared, standing guard.
	*/
	craft->Mark(MARK_UP);
	craft->Coord = ::Cell_Coord(back);
	craft->Mark(MARK_DOWN);

	craft->Set_Speed(0);		/*	An aircraft's throttle is FlyClass::SpeedAdd (Get_Speed); FootClass::Speed stays 0. */
	craft->Assign_Target(TARGET_NONE);
	craft->Assign_Destination(TARGET_NONE);
	craft->Assign_Mission(MISSION_GUARD);
}


void CAC_Return_Home(AircraftClass *craft)
{
	if (craft == NULL || craft->Class == NULL) return;
	if (craft->IsInLimbo || !craft->IsActive || craft->Strength <= 0) return;
	if (!CAC_Is_Boomerang(*craft)) return;

	/*
	**	A target is not a reason to stay out.  Spent rounds are a reason to come in.
	**
	**	This used to return on any legal TarCom, and that one line is most of what
	**	was wrong with these.  CrystAlien never asks: UnitNav::endMove ends with
	**	
	**	    if (this.parent.stats.boomerang) this.returnHome();
	**	
	**	whenever a path runs out, target or no target, and UnitHAL re-aims at the
	**	target every `reaction` frames -- a hundred of them for a Fighter.  So it
	**	shuttles: out far enough to shoot, back to the plate, out again.  That
	**	shuttle is the drive-by, and it is also how a Fighter ever gets home to
	**	rearm, since shoot() only sends it back once the payload is empty.
	**
	**	Holding it out here while it had a target meant it never left, and because
	**	it keeps the target until the last round is gone it hovered over whatever
	**	it was shooting until one of them died.  Both complaints, one line.
	**
	**	So: still leave it alone while it is on its way somewhere, and while it has
	**	a full magazine and something to shoot -- that is the outbound leg.  Once it
	**	has actually spent a round, it is coming home whatever it is looking at.
	*/
	if (Target_Legal(craft->NavCom)) return;

	/*
	**	Out while it can still shoot, and home when the magazine is empty.
	**
	**	shoot() sends a boomerang back on exactly one condition:
	**
	**	    if(this.weaponPayload <= 0) {
	**	       this.target = false;
	**	       this.nav.returnHome();
	**	       return undefined;
	**	    }
	**	    this.weaponPayload--;
	**
	**	-- an empty magazine, not a fired round.  This read `Ammo < MaxAmmo`,
	**	which is true the instant the first rocket leaves the rail, so an Astro
	**	Fighter sent to attack something shot once and turned for home with nine
	**	rounds still aboard.  That is the "insta-return", and it was hidden until
	**	the lander stopped latching, because a craft mid-ramp could not commence
	**	the mission that took it back.
	**
	**	Ammo is -1 for an unlimited magazine, so the test is against zero rather
	**	than a comparison with the maximum.
	*/
	if (Target_Legal(craft->TarCom) && craft->Ammo != 0) return;

	/*
	**	And drop the target on the way, as shoot() does with `this.target = false`.
	**	Left set, TechnoClass keeps trying to line the shot up on a craft that has
	**	nothing to fire.
	*/
	if (craft->Ammo == 0) craft->Assign_Target(TARGET_NONE);
	if (craft->Mission == MISSION_ENTER || craft->MissionQueue == MISSION_ENTER) return;

	/*
	**	Already docked at its own station -- nothing to do.
	*/
	BuildingClass *home = CAC_Home_Building(craft);
	if (home == NULL) return;

	/*
	**	Asked once a second rather than every frame.  This runs from
	**	AircraftClass::AI, and CAC_Exit_Cell below walks thirty-seven tiles asking
	**	the map about each -- for every boomerang, every frame.  One session's log
	**	had a hundred and sixty-eight thousand of those calls in it, which is both
	**	work for nothing and enough noise to bury everything else.
	*/
	if (((int)Frame + craft->As_Target()) % TICKS_PER_SECOND) return;


	/*
	**	Docking is exclusive -- a station already in radio contact with one
	**	fighter turns the next one away -- and that alone was enough to leave the
	**	rest sitting wherever they were last sent.  CrystAlien has no such
	**	handshake: returnHome is a plain voyage to the station's dock, and a
	**	squadron simply gathers there.
	**
	**	So dock if the station is free, since that is what rearms a shot, and
	**	otherwise just fly home and wait nearby.
	*/
	/*
	**	All of them to the same tile, and none of them docking.
	**
	**	returnHome is one line -- `voyage(home.stats.dockX, home.stats.dockY)` -- and
	**	every fighter of a station flies to that one tile.  They do not obstruct, so
	**	they settle into each other and sit in a stack on the front of the plate.
	**
	**	Going through the radio and MISSION_ENTER could not do that: a building holds
	**	one contact at a time, so the first fighter docked and every one after it fell
	**	through to a move on Docking_Coord -- which is the building's middle, not its
	**	plate.  They arrived somewhere they were not meant to be, in ones, and the
	**	rest circled waiting for a berth that was never going to free up.
	**
	**	Nothing is lost by dropping it.  Rearming is CAC_Rearm_At_Home, which measures
	**	the distance rather than asking who is docked, and the descent is the lander
	**	logic, which asks only whether the craft is home and idle.
	*/
	CELL berth = CAC_Exit_Cell(home, craft);

	/*
	**	Already home means standing on the dock tile, which is what `atHome` is:
	**
	**	    this.stats.home.stats.dockPos.x == this.parent.tilePos.x &&
	**	    this.stats.home.stats.dockPos.y == this.parent.tilePos.y
	**
	**	This used to read the two-cell radius, so a Fighter that stopped near its
	**	plate was declared home and never given the last hop -- it settled beside
	**	the artwork instead of on it.  The hop was briefly done from the descent
	**	gate instead, which was worse: that code has none of the guards above, so
	**	a Fighter told to attack something had its order overwritten with a move
	**	to its own berth on the first frame.  An attack order sets TarCom and not
	**	NavCom, so the idle count was still high and the hop fired immediately --
	**	"insta-seats when told to attack or move".
	**
	**	It belongs here, behind the guards: a craft with somewhere to be or
	**	something to shoot at is already excluded, and asking once a second costs
	**	nothing.  Re-ordering no longer disturbs the descent either, because the
	**	idle count resets on a *change* of destination and this asks for the same
	**	tile every time -- and once the craft is on it, this returns before asking.
	*/
	if (CAC_On_Berth(craft)) {
		/*
		**	Once a minute at most.  One static shared by every craft meant two of them
		**	taking turns defeated it entirely, and a settled squadron wrote a line a
		**	frame -- which buried the engineer arrival that mattered.
		*/
		if (CAC_Logging() && ((int)Frame % (TICKS_PER_SECOND * 60)) == 0) {
			CAC_Log("berth", "%s is on its berth at %d,%d (altitude %d, mission %d)",
						craft->Class->IniName, Cell_X(berth), Cell_Y(berth),
						craft->Altitude, (int)craft->Mission);
		}
		return;
	}

	/*
	**	Volatile: they are told to go home and the berth is stable -- one session's
	**	log has BFGOOD answering 31,22 and BFEVIL 3,42, candidate 0 of 37, every
	**	time -- and they still do not settle on it.  So the order is not the thing
	**	going wrong, and this says what state they were in when it was given.
	*/
	CAC_Log_Volatile("berth", "%s sent home to %d,%d from %d,%d "
							 "(altitude %d, mission %d, ammo %d)",
							 craft->Class->IniName, Cell_X(berth), Cell_Y(berth),
							 Cell_X(Coord_Cell(craft->Center_Coord())),
							 Cell_Y(Coord_Cell(craft->Center_Coord())),
							 craft->Altitude, (int)craft->Mission, (int)craft->Ammo);

	craft->Assign_Destination(::As_Target(berth));
	craft->Assign_Mission(MISSION_MOVE);
	craft->Commence();
}


/***********************************************************************************************
 * CAC_Rearm_At_Home -- A boomerang tops itself up while it is standing at its station.        *
 *                                                                                             *
 *    UnitNav, on finishing a path at the home dock:                                            *
 *                                                                                             *
 *        this.atHome = true;                                                                   *
 *        this.parent.weaponPayload = min(++this.parent.weaponPayload, max);                    *
 *        this.hesitate = 20;                                                                   *
 *                                                                                             *
 *    One round per arrival, not a magazine, and it is the unit that does it rather than the    *
 *    building.  That distinction is the reason this exists at all: Tiberian Dawn reloads       *
 *    aircraft through RADIO_RELOAD, which is sent from one place -- the DURING branch of        *
 *    Mission_Repair, inside `if (*this == STRUCT_REPAIR)`.  A Radar Station is not a repair     *
 *    bay, so a Fighter that flew home empty was going to stand there empty, and ten rockets     *
 *    was the whole of its life.                                                                 *
 *                                                                                             *
 *    Doing it the way CrystAlien does it sidesteps the question, and is what the source says    *
 *    anyway.  Twenty frames a round, so a spent Fighter is worth something again after about    *
 *    thirteen seconds and full after two hundred -- which is what keeps a station from being    *
 *    an airfield and makes the sortie rate the thing that limits them.                          *
 *=============================================================================================*/
void CAC_Rearm_At_Home(AircraftClass * craft)
{
	if (craft == NULL || craft->Class == NULL) return;
	if (craft->IsInLimbo || !craft->IsActive || craft->Strength <= 0) return;
	if (!CAC_Is_Boomerang(*craft)) return;

	/*
	**	A negative count is the engine's "no magazine at all", and a full one has
	**	nothing to do here.
	*/
	if (craft->Ammo < 0 || craft->Ammo >= craft->Class->MaxAmmo) return;

	BuildingClass * home = CAC_Home_Building(craft);
	if (home == NULL) return;

	/*
	**	CrystAlien asks for the dock tile exactly.  Here the craft is sent to
	**	Docking_Coord and parks near it rather than on it, so the test is a short
	**	radius round the station instead -- three cells, which covers the footprint
	**	and the tile in front of it without reaching a neighbouring building.
	*/
	if (::Distance(craft->Center_Coord(), home->Center_Coord()) > (CELL_LEPTON_W * 3)) return;

	if ((Frame % 20) != 0) return;
	craft->Ammo++;
}


/***********************************************************************************************
 * CAC_Aircraft_Hovers -- Does this flier stay up when it has nothing to do?                   *
 *                                                                                             *
 *    Tiberian Dawn sends an idle helicopter home to land.  Most of CrystAlien's fliers simply  *
 *    hold station: their weaponPayload is effectively unlimited, so there is nothing to return *
 *    for, and the JetPack Explorer and the Dropship stay where they are left.                  *
 *                                                                                             *
 *    Two do not.  The Fighter is a `boomerang` and goes back to its station the moment it has  *
 *    nothing else to do, rearming a shot each time it arrives.  The Switch Fighter is a        *
 *    transformer, and settling on the ground is the whole point of it.                         *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Berth_Is_Clear -- May this flier settle where it is standing?                           *
 *                                                                                             *
 *    CrystAlien asks one question before letting a lander down:                                *
 *                                                                                             *
 *        if (still > 20 && (!boomerang || atHome) && !arena.tiles[tilePos.x][tilePos.y])       *
 *                                                                                             *
 *    and `arena.tiles` is the *obstruction* map, which a flier never writes to.  setBlock only *
 *    marks a tile `if (this.stats.obstruct)`, and every boomerang carries `obstruct: false`.   *
 *    So a landed Astro Fighter does not stand in the way of the next one, and all four of them *
 *    come down on the same dock tile and sit in each other.  That stack is what the plate is   *
 *    supposed to look like.                                                                    *
 *                                                                                             *
 *    Tiberian Dawn has no such distinction.  Cell_Techno() answers with whatever is in the     *
 *    cell, a landed aircraft included, so the first fighter home took the tile and the other   *
 *    three were refused for the rest of the game -- hovering over the station, being sent back *
 *    to a berth they could not occupy, rising and settling and rising again.                    *
 *                                                                                             *
 *    So the test is the same test, over the same set: anything that would obstruct in the      *
 *    source game blocks a landing, and anything that would not does not.                        *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Log_Berth_Refusal / CAC_Log_Berth_Restless -- Why a Fighter is still in the air.         *
 *                                                                                             *
 *    Reported twice now as "they still do not seat properly at the station", and the log has   *
 *    had no way to answer it: it records the sends and the hovers and nothing about the        *
 *    descent, so a session shows seven orders home, four craft over the plate and no landings, *
 *    with no indication of which gate held them.                                               *
 *                                                                                             *
 *    There are only two ways to stay airborne, and they need telling apart:                    *
 *                                                                                             *
 *      * Restless -- the idle count keeps being cleared, so the gate is never reached.  That    *
 *        is something re-issuing an order, or a speed that never settles to zero.               *
 *      * Refused -- the gate is reached and says no, because the berth is occupied by          *
 *        something that counts as an obstruction or because the craft is not close enough to   *
 *        its station to qualify.                                                                *
 *                                                                                             *
 *    Damped hard.  Both of these sit in code that runs every frame for every aircraft, and an  *
 *    undamped line here is the santa mistake again -- so one line per craft per five seconds,  *
 *    and only when the answer has changed.                                                     *
 *=============================================================================================*/
static bool CAC_Berth_Say(AircraftClass const * craft, int code)
{
	if (!CAC_Logging() || craft == NULL) return(false);

	/*
	**	Indexed by the aircraft's own heap id, which is the thing that varies --
	**	fifth time of asking.  A craft that dies and whose slot is reused simply
	**	starts the damping again, which is harmless.
	*/
	static int  _said[64];
	static long _when[64];
	static bool _init = false;
	if (!_init) {
		_init = true;
		for (int i = 0; i < 64; i++) { _said[i] = -1; _when[i] = -9999; }
	}

	int id = Aircraft.ID((AircraftClass *)craft);
	if (id < 0 || id >= 64) return(false);
	if (_said[id] == code && (long)Frame - _when[id] < (long)(TICKS_PER_SECOND * 5)) return(false);

	_said[id] = code;
	_when[id] = (long)Frame;
	return(true);
}


/***********************************************************************************************
 * CAC_Log_Lander -- What a lander did, and what state it was in when it did it.                *
 *                                                                                             *
 *    Volatile.  "Transports insta-seat instead of flying properly" is not a shape the code     *
 *    reads back: a grounded lander is supposed to set IsTakingOff, ride the altitude ramp up   *
 *    and fly, and every path through Mission_Move and the descent gate says it does.  So this  *
 *    records the three moments that matter -- it settles, it climbs, it is held on the ground  *
 *    -- with everything that gates them, and the next round answers it.                        *
 *                                                                                             *
 *    The door is in there because it is the one thing that can stop the ramp dead:             *
 *                                                                                             *
 *        if (Is_Door_Closed() && (IsLanding || IsTakingOff))                                   *
 *                                                                                             *
 *    A transport that has unloaded holds its door open for the rest of its life in stock       *
 *    Tiberian Dawn, and a craft whose ramp never runs sits exactly where it is.                *
 *=============================================================================================*/
void CAC_Log_Lander(AircraftClass const * craft, char const * what,
						  int still, bool doorclosed, int status)
{
	if (craft == NULL || craft->Class == NULL) return;
	if (!CAC_Is_Lander(*craft)) return;

	/*
	**	Keyed on the phrase, so a craft that alternates between two of them reports
	**	both rather than being damped down to one.
	*/
	int code = 300;
	for (char const * p = what; *p; p++) code += (int)*p;
	if (!CAC_Berth_Say(craft, code)) return;

	/*
	**	The destination and the distance to it, because "settles while holding a
	**	destination with speed nought" has two completely different causes and the
	**	line above could not tell them apart: either the craft has arrived and
	**	something keeps handing it somewhere it already is, or it has somewhere
	**	real to be and the mission is not flying it there.
	*/
	int away = -1;
	int navx = -1, navy = -1;
	if (Target_Legal(craft->NavCom)) {
		COORDINATE nav = ::As_Coord(craft->NavCom);
		if (nav != 0) {
			away = ::Distance(craft->Center_Coord(), nav);
			navx = Cell_X(Coord_Cell(nav));
			navy = Cell_Y(Coord_Cell(nav));
		}
	}

	CAC_Log_Volatile("lander", "%s %s at %d,%d (altitude %d, speed %d, still %d, "
						  "mission %d, status %d, door %s, destination %d,%d at %d, "
						  "radio %s, house %s)",
						  craft->Class->IniName, what,
						  Cell_X(Coord_Cell(craft->Center_Coord())),
						  Cell_Y(Coord_Cell(craft->Center_Coord())),
						  craft->Altitude, (int)craft->Get_Speed(), still,
						  (int)craft->Mission, status,
						  doorclosed ? "shut" : "open",
						  navx, navy, away,
						  craft->In_Radio_Contact() ? "yes" : "no",
						  (craft->House != NULL && craft->House->IsHuman) ? "player" : "computer");
}


void CAC_Log_Berth_Refusal(AircraftClass const * craft, bool clear, bool berthed)
{
	if (!CAC_Berth_Say(craft, 100 + (clear ? 2 : 0) + (berthed ? 1 : 0))) return;

	BuildingClass * home = CAC_Home_Building(craft);
	CELL here = Coord_Cell(craft->Center_Coord());
	CELL berth = (home != NULL) ? CAC_Exit_Cell(home, craft) : (CELL)0;

	CAC_Log_Volatile("berth",
		"%s will not settle at %d,%d: berth %d,%d is %s and it is %s (altitude %d)",
		craft->Class->IniName, Cell_X(here), Cell_Y(here),
		Cell_X(berth), Cell_Y(berth),
		clear ? "clear" : "blocked",
		berthed ? "on it" : "not on it",
		craft->Altitude);
}


void CAC_Log_Berth_Restless(AircraftClass const * craft, bool hasnav, int speed, int mission)
{
	if (!CAC_Berth_Say(craft, 200 + (hasnav ? 1 : 0) + (speed != 0 ? 2 : 0))) return;

	CAC_Log_Volatile("berth",
		"%s never sits still: %s, speed %d, mission %d (altitude %d)",
		craft->Class->IniName,
		hasnav ? "has somewhere to be" : "no destination",
		speed, mission, craft->Altitude);
}


/***********************************************************************************************
 * CAC_At_Berth -- Is this flier standing on its own station's dock tile?                      *
 *                                                                                             *
 *    UnitNav sets `atHome` only where the tile matches the home's dock:                        *
 *                                                                                             *
 *        if (... || this.stats.home.stats.dockPos.x == this.parent.tilePos.x &&                *
 *                   this.stats.home.stats.dockPos.y == this.parent.tilePos.y)                  *
 *            this.atHome = true;                                                               *
 *                                                                                             *
 *    and the descent is gated on it for a boomerang -- `(!this.stats.boomerang || this.atHome)`*
 *    -- so an Astro Fighter caught out on the map stays up.  Only its own plate brings it down.*
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Never_Lands -- Is this one of the fliers that stays up whatever happens?                *
 *                                                                                             *
 *    `lander` in UnitStats is what puts a craft on the ground, and UnitNav's whole descent is  *
 *    wrapped in it:                                                                            *
 *                                                                                             *
 *        if (this.parent.stats.lander && (this.still || this.landerPerc)) { ... }              *
 *                                                                                             *
 *    The JetPack Explorer and the Viper Attack carry `lander: false`, so that block never runs *
 *    for them and their altitude stays at maxAltitude -- fifty -- from the moment they are     *
 *    built until they die.  They arrive at a destination in the air and stop there.            *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Ignores_LZ -- Does a busy landing zone turn this flier away?                            *
 *                                                                                             *
 *    Not for one of CrystAlien's.  `obstruct: false` means its fliers do not occupy a tile and *
 *    do not see each other occupying one, which is why a squadron piles onto a single dock.    *
 *                                                                                             *
 *    Tiberian Dawn asks Is_LZ_Clear before the last leg of a move, and sends the craft to      *
 *    New_LZ -- some other cell entirely -- when the answer is no.  That is why the boomerangs  *
 *    settled correctly at first and drifted apart as the round went on: the first one home     *
 *    landed on the plate, and every one after it found the plate taken and was quietly given   *
 *    somewhere else to be.  They ended up crowding the station instead of stacking on it.      *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Ground_Name -- The name this mod's repainted ground answers to.                         *
 *                                                                                             *
 *    Base terrain does not reach the client the way anything else does.  Get_Template_Info     *
 *    hands over a template name and an icon, DLLInterface makes "CLEAR1_i7.tga" of them, and   *
 *    the client resolves that pair through the tileset.                                        *
 *                                                                                             *
 *    Pointing a *stock* tile name at a mod archive does not work.  Sixteen frames of CLEAR1    *
 *    were repointed at cacd_clear1 and came back as missing-texture quads, while BR1, S05,     *
 *    S06 and S32 -- left stock -- drew correctly in the same frame of the same map.  The       *
 *    archive was not the problem: it is byte-for-byte the same shape as the baseplate sitting  *
 *    beside it, 128x128, 32bpp, type 2, the same 65554 bytes and the same metadata.            *
 *                                                                                             *
 *    And the baseplates draw.  They are a mod archive under a mod tile name -- CACPAD1G, its   *
 *    own entry in the same tileset, in the same folder.  So a mod archive resolves perfectly   *
 *    well; what does not resolve is a mod archive reached through a name the shipped game      *
 *    already owns.                                                                             *
 *                                                                                             *
 *    Which is answerable, because the name is ours to give.  IniName is what Get_Template_Info *
 *    reports, so ground that has been repainted is reported under a name of its own and gets   *
 *    a tileset entry of its own, exactly as the plates do.  The INI name is untouched, since   *
 *    that is what map files are written against.                                               *
 *                                                                                             *
 * OUTPUT:  the name to report, or NULL to report the stock one unchanged.                      *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Selected_Capturer -- The engineer in the current selection, if there is one.             *
 *                                                                                             *
 *    CAC_Group_Capturer already answers this for DisplayClass, and it was the wrong half of    *
 *    the game.  Best_Object_With_ActionT is the *legacy* renderer's group order; the Remaster  *
 *    never calls it.  What the client actually reads is ActionWithSelected, built in           *
 *    Get_Player_Info_State, and that picks its representative like this:                       *
 *                                                                                             *
 *        // Use first object with a weapon, or first object if none                            *
 *                                                                                             *
 *    -- which is precisely the ranking CrystAlien does not use.  So a Saboteur picked up with  *
 *    an escort still could not be told to walk into anything: the escort answered for the      *
 *    whole selection and the building got shot at.  The fix went in a year of code away from   *
 *    where the behaviour lives, and passed its own test, and changed nothing a player saw.     *
 *                                                                                             *
 *    This is the same rule -- one engineer anywhere in the selection wins over an enemy        *
 *    building -- asked where the answer is used.                                               *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Selected_Boarder -- Somebody in the selection who could get aboard.                     *
 *                                                                                             *
 *    The client's action table is built by asking one representative of the selection what it  *
 *    would do with each cell -- the first armed member, or failing that the first member at    *
 *    all.  A Dropship carries no weapon, so a selection it happens to lead answers every cell  *
 *    in its own voice, and a transport has no reason to board itself.  The squad standing next *
 *    to it is never asked, and the order is simply not offered.                                *
 *                                                                                             *
 *    CrystAlien does not nominate a representative.  It asks the question of the whole         *
 *    selection at once:                                                                        *
 *                                                                                             *
 *        if(this.activeTarget.stats.carrier &&                                                 *
 *           this.activeTarget.stats.contents.length < this.activeTarget.stats.capacity)        *
 *           for(_loc2_ in this.selected)                                                       *
 *              if(this.selected[_loc2_].stats.carriable)                                       *
 *                 this.cursorState = "infiltrate";                                             *
 *                                                                                             *
 *    -- any one carriable member is enough.  This is the same override the engineer already    *
 *    has for enemy buildings, for the same reason.                                             *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Rehouse_Prebuilt -- Give a pre-placed base to whoever actually spawns on it.            *
 *                                                                                             *
 *    A skirmish map cannot name the players.  The INI has to say Multi1 and Multi2, and the    *
 *    lobby hands the two players whichever house slots it likes -- a two player game logged     *
 *    Multi1 and Multi3, so everything written as Multi2 belonged to a house nobody was         *
 *    playing.  Both bases ended up under one flag, which is "both spawns are GDI".             *
 *                                                                                             *
 *    That is why no shipped skirmish map has a player-owned structure in it: the stock ones    *
 *    place civilian buildings under Neutral and nothing else.  This one has to, because the    *
 *    whole point of it is the base layout the source game drew.                                *
 *                                                                                             *
 *    The rule that holds regardless of slots is the start position.  The base written as       *
 *    Multi1 stands on the first waypoint, so it belongs to whoever spawns there --             *
 *    StartLocationOverride is that index, and it survives random start assignment too.         *
 *                                                                                             *
 *    Ownership moves through Captured(), which is the engine's own transfer and keeps the      *
 *    house's building scan, power and counts in step.  Assigning House by hand does not.       *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Watch_Factories -- Report a queue that has stopped moving.                              *
 *                                                                                             *
 *    Reported as a Training Camp that cannot be clicked, with the suggestion that it is stuck  *
 *    trying to produce.  The three mechanisms that would cause that are already ruled out --   *
 *    Begin_Production leaves no factory behind when it refuses, CAC_At_Type_Cap counts what is *
 *    in the queue so it cannot be over-filled, and CAC_Cancel_Unbuildable fires on             *
 *    prerequisites and never on caps -- but a factory can still stall for reasons none of      *
 *    those cover: a finished unit with nowhere to stand keeps its factory at full and waits.   *
 *                                                                                             *
 *    So watch rather than argue.  A queue whose completion has not moved in half a minute is   *
 *    stuck, and this says which building, what it holds and how far it got.                    *
 *=============================================================================================*/
void CAC_Watch_Factories(void)
{
	if (!CAC_Logging()) return;
	if (((long)Frame % (long)(TICKS_PER_SECOND * 5)) != 0) return;

	static int _was[64];
	static long _since[64];
	static bool _init = false;
	if (!_init) {
		for (int i = 0; i < 64; i++) { _was[i] = -1; _since[i] = 0; }
		_init = true;
	}

	for (int i = 0; i < Factories.Count() && i < 64; i++) {
		FactoryClass * f = Factories.Ptr(i);
		if (f == NULL) { _was[i] = -1; continue; }

		TechnoClass const * o = f->Get_Object();
		if (o == NULL || o->Techno_Type_Class() == NULL) { _was[i] = -1; continue; }

		int at = f->Completion();
		if (at != _was[i]) {
			_was[i] = at;
			_since[i] = (long)Frame;
			continue;
		}

		if ((long)Frame - _since[i] > (long)(TICKS_PER_SECOND * 30)) {
			_since[i] = (long)Frame;		/* say it once every half minute */
			HouseClass const * h = f->Get_House();
			CAC_Log_Volatile("stuck", "%s has sat at %d%% for half a minute for %s%s",
								 o->Techno_Type_Class()->IniName,
								 (at * 100) / 256,
								 (h && h->Class) ? h->Class->IniName : "?",
								 f->Has_Completed() ? " -- finished, with nowhere to put it" : "");
		}
	}
}


/***********************************************************************************************
 * CAC_Log_Kill / CAC_Roll_Call -- A record of who lost what, and who still holds what.        *
 *                                                                                             *
 *    Asked for after a round that could not be won with nothing visibly left alive.  Two       *
 *    halves of the same question:  what died, and what is still standing.                      *
 *                                                                                             *
 *    The kill line names the victim, its owner, where it fell and who did it -- an unattributed *
 *    death is the environment or a scripted removal, and saying so is worth as much as naming  *
 *    a killer.                                                                                 *
 *                                                                                             *
 *    The roll call is every half minute: each side, its buildings, and every unit it holds with *
 *    the mission it is on.  A straggler parked in the dark shows up as a line rather than as a  *
 *    round that will not end.                                                                  *
 *=============================================================================================*/
void CAC_Log_Kill(TechnoClass const * victim, TechnoClass const * killer)
{
	if (!CAC_Logging() || victim == NULL) return;

	TechnoTypeClass const * vt = victim->Techno_Type_Class();
	if (vt == NULL) return;

	CELL at = Coord_Cell(victim->Center_Coord());
	TechnoTypeClass const * kt = (killer != NULL) ? killer->Techno_Type_Class() : NULL;

	CAC_Log("kill", "%s's %s dies at %d,%d -- %s%s%s",
				(victim->House && victim->House->Class)
					? victim->House->Class->IniName : "?",
				vt->IniName, Cell_X(at), Cell_Y(at),
				(kt != NULL) ? "killed by " : "no attributed killer",
				(kt != NULL) ? kt->IniName : "",
				(kt != NULL && killer->House && killer->House->Class)
					? "" : "");
}


/***********************************************************************************************
 * CAC_Log_Flags -- What is standing on every flag, and what thinks it owns the ground.          *
 *                                                                                             *
 *    "I cannot click a unit onto an enemy flag" is a claim about one cell, and three different  *
 *    things can refuse it: something is standing there, so the click is an attack on that; a    *
 *    structure's drawn area reaches across it, so the click is an attack on the structure; or   *
 *    the ground itself will not take a mover.  Nothing anywhere wrote down which.               *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Log_Crates -- The state of every pickup still on the board.                              *
 *                                                                                             *
 *    A crate that cannot be collected has half a dozen possible reasons and they are all       *
 *    invisible: something standing on the cell, a structure drawn across it, ground a mover    *
 *    cannot cross, or the overlay having quietly gone while the art stayed.  One line each,    *
 *    every half minute, and there are rarely more than a handful on the map.                    *
 *=============================================================================================*/
static void CAC_Log_Crates(void)
{
	for (CELL cell = 0; cell < MAP_CELL_TOTAL; cell++) {
		CellClass const &c = Map[cell];
		if (c.Overlay == OVERLAY_NONE) continue;
		if (!OverlayTypeClass::As_Reference(c.Overlay).IsCrate) continue;

		ObjectClass const *on = c.Cell_Occupier();
		BuildingClass const *over = CAC_Building_Over(cell);

		BuildingClass const *sits = c.Cell_Building();
		TerrainClass const *tree = c.Cell_Terrain();
		CAC_Log("crate", "%s at %d,%d: standing on it %s, drawn over by %s, "
					  "inside %s, terrain %s, revealed %d, "
					  "ground %d, smudge %d, owner %d",
				  (c.Overlay == OVERLAY_STEEL_CRATE) ? "an Alien capsule" :
				  (c.Overlay == OVERLAY_CAC_GIFT)    ? "a present" : "a crystal box",
				  Cell_X(cell), Cell_Y(cell),
				  (on != NULL && on->Is_Techno()
					  ? ((TechnoClass const *)on)->Techno_Type_Class()->IniName : "nothing"),
				  (over != NULL && over->Class != NULL) ? over->Class->IniName : "nothing",
				  (sits != NULL && sits->Class != NULL) ? sits->Class->IniName : "nothing",
				  (tree != NULL) ? tree->Class_Of().IniName : "none",
				  (int)c.IsMapped,
				  (int)c.Land_Type(), (int)c.Smudge, (int)c.Owner);
	}
}


static void CAC_Log_Flags(void)
{
	if (!Special.IsCaptureTheFlag || GameToPlay == GAME_NORMAL) return;

	for (int h = 0; h < Houses.Count(); h++) {
		HouseClass const *house = Houses.Ptr(h);
		if (house == NULL || !house->IsActive || house->Class == NULL) continue;

		CELL cell = CAC_Flag_Cell(house);
		if (cell == 0 || !Map.In_Radar(cell)) continue;

		ObjectClass const *on = Map[cell].Cell_Occupier();
		BuildingClass const *over = CAC_Building_Over(cell);

		CAC_Log("flag", "%s flag at %d,%d: standing on it %s, drawn over by %s, ground %d",
				  house->Class->IniName, Cell_X(cell), Cell_Y(cell),
				  (on != NULL && on->Is_Techno()
					? ((TechnoClass const *)on)->Techno_Type_Class()->IniName : "nothing"),
				  (over != NULL && over->Class != NULL) ? over->Class->IniName : "nothing",
				  (int)Map[cell].Land_Type());
	}
}


void CAC_Roll_Call(void)
{
	if (!CAC_Logging()) return;
	if (((long)Frame % (long)(TICKS_PER_SECOND * 30)) != 0) return;

	/*
	**	Once a frame, not once a house.
	**
	**	This walks every house itself and is called from HouseClass::AI, which runs
	**	per house -- so with four of them on the board every dump came out four
	**	times, census lines and all.  Four fifths of a playtest log was this routine
	**	repeating itself, and only two generations of log are kept, so the noise was
	**	costing real evidence.
	*/
	static long _said = -1;
	if (_said == (long)Frame) return;
	_said = (long)Frame;

	CAC_Log_Flags();
	CAC_Log_Crates();

	for (int h = 0; h < Houses.Count(); h++) {
		HouseClass const * house = Houses.Ptr(h);
		if (house == NULL || !house->IsActive || house->Class == NULL) continue;

		HousesType which = house->Class->House;
		if (which == HOUSE_NEUTRAL || which == HOUSE_JP) continue;

		int builds = 0;
		for (int i = 0; i < Buildings.Count(); i++) {
			BuildingClass const * b = Buildings.Ptr(i);
			if (b != NULL && b->IsActive && !b->IsInLimbo && b->House == house) builds++;
		}

		int mobile = 0;
		for (int pass = 0; pass < 3; pass++) {
			int count = (pass == 0) ? Units.Count()
					  : (pass == 1) ? Infantry.Count() : Aircraft.Count();
			for (int i = 0; i < count; i++) {
				TechnoClass const * o =
					(pass == 0) ? (TechnoClass const *)Units.Ptr(i)
				  : (pass == 1) ? (TechnoClass const *)Infantry.Ptr(i)
								: (TechnoClass const *)Aircraft.Ptr(i);
				if (o == NULL || !o->IsActive || o->House != house) continue;
				if (o->Strength == 0) continue;
				mobile++;
			}
		}

		CAC_Log("rollcall", "%s: %d building(s), %d unit(s), %ld credits",
					house->Class->IniName, builds, mobile, house->Available_Money());

		/*
		**	And each of them, while the list is short enough to be worth reading.
		**	Sixteen is more than a stalled round ever has left and less than a
		**	running one would fill the log with.
		*/
		if (mobile > 0 && mobile <= 16) {
			for (int pass = 0; pass < 3; pass++) {
				int count = (pass == 0) ? Units.Count()
						  : (pass == 1) ? Infantry.Count() : Aircraft.Count();
				for (int i = 0; i < count; i++) {
					TechnoClass * o =
						(pass == 0) ? (TechnoClass *)Units.Ptr(i)
					  : (pass == 1) ? (TechnoClass *)Infantry.Ptr(i)
									: (TechnoClass *)Aircraft.Ptr(i);
					if (o == NULL || !o->IsActive || o->House != house) continue;
					if (o->Strength == 0) continue;

					CELL at = Coord_Cell(o->Center_Coord());
					bool out = !Map.In_Radar(at);
					CAC_Log("rollcall", "    %-8s at %2d,%2d  %s%s%s",
								o->Techno_Type_Class()
									? o->Techno_Type_Class()->IniName : "?",
								Cell_X(at), Cell_Y(at),
								MissionClass::Mission_Name(o->Mission),
								o->IsInLimbo ? " (in limbo)" : "",
								out ? " (OFF THE MAP)" : "");

					/*
					**	And taken off the board if it is not on it.
					**
					**	Nothing should ever be outside the playable rectangle: it
					**	cannot be seen, cannot be reached, and cannot be shot, so a
					**	round holding one is a round that cannot be finished -- which
					**	is exactly the report.  Whatever put it there is a bug in its
					**	own right, but the game should not become unwinnable while it
					**	goes unfound.
					**
					**	Only checked here, so a craft has up to half a minute outside
					**	the border before it is picked up; anything legitimately
					**	crossing the edge for a moment is long back by then.
					*/
					if (out && !o->IsInLimbo) {
						CAC_Log("rollcall", "      -- outside the map, removing it");
						int damage = o->Strength * 4 + 64;
						o->Take_Damage(damage, 0, WARHEAD_HE, NULL);
					}
				}
			}
		}
	}
}


void CAC_Rehouse_Prebuilt(void)
{
	if (GameToPlay == GAME_NORMAL) return;

	HousesType want[MAX_PLAYERS];
	for (int i = 0; i < MAX_PLAYERS; i++) want[i] = HOUSE_NONE;

	for (int i = 0; i < MPlayerCount; i++) {
		HouseClass * hptr = HouseClass::As_Pointer(MPlayerHouses[i]);
		if (hptr == NULL) continue;
		int at = hptr->StartLocationOverride;
		if (at >= 0 && at < MAX_PLAYERS) want[at] = MPlayerHouses[i];
	}

	int moved = 0;
	for (int pass = 0; pass < 4; pass++) {
		int count = (pass == 0) ? Buildings.Count()
				  : (pass == 1) ? Units.Count()
				  : (pass == 2) ? Infantry.Count() : Aircraft.Count();
		for (int i = 0; i < count; i++) {
			TechnoClass * obj = (pass == 0) ? (TechnoClass *)Buildings.Ptr(i)
							 : (pass == 1) ? (TechnoClass *)Units.Ptr(i)
							 : (pass == 2) ? (TechnoClass *)Infantry.Ptr(i)
											: (TechnoClass *)Aircraft.Ptr(i);
			if (obj == NULL || !obj->IsActive || obj->House == NULL) continue;
			if (obj->House->Class == NULL) continue;

			int slot = (int)obj->House->Class->House - (int)HOUSE_MULTI1;
			if (slot < 0 || slot >= MAX_PLAYERS) continue;
			if (want[slot] == HOUSE_NONE) continue;
			if (want[slot] == obj->House->Class->House) continue;

			HouseClass * to = HouseClass::As_Pointer(want[slot]);
			if (to == NULL) continue;
			if (obj->Captured(to)) moved++;
		}
	}

	if (moved > 0) {
		CAC_Log("session", "handed %d pre-placed object(s) to the houses that "
						   "actually spawned on them", moved);
	}
}


ObjectClass * CAC_Selected_Boarder(void)
{
	if (CurrentObject.Count() < 2) return(NULL);

	for (int i = 0; i < CurrentObject.Count(); i++) {
		ObjectClass * obj = CurrentObject[i];

		/*
		**	As with the capturer:  the selection can outlive what is in it, and this
		**	runs from the client's per-frame table.
		*/
		if (!CAC_Is_Live_Object(obj)) continue;
		if (!obj->Is_Techno()) continue;

		TechnoTypeClass const * type = ((TechnoClass *)obj)->Techno_Type_Class();
		if (type == NULL || !CAC_Is_CrystAlien_Type(type->IniName)) continue;

		/*
		**	`carriable` in UnitStats.  A carrier is not carriable, so a Dropship in
		**	the selection never nominates itself and never tries to board anything.
		*/
		if (!CAC_Is_Carriable(obj)) continue;
		return(obj);
	}
	return(NULL);
}


/***********************************************************************************************
 * CAC_Is_Capturer -- Is this one of the men who take buildings rather than shoot them?         *
 *                                                                                             *
 *    Split out of CAC_Selected_Capturer so the click can ask it of each member of a selection  *
 *    rather than only find the first one.                                                     *
 *=============================================================================================*/
bool CAC_Is_Capturer(ObjectClass const *obj)
{
	if (!CAC_Is_Live_Object((ObjectClass *)obj)) return(false);
	if (obj->What_Am_I() != RTTI_INFANTRY) return(false);

	InfantryClass const *man = (InfantryClass const *)obj;
	if (man->Class == NULL || !man->Class->IsCapture) return(false);

	return(CAC_Is_CrystAlien_Type(man->Class->IniName));
}


/***********************************************************************************************
 * CAC_Band_Excluded -- Dropped from a drag box that caught something else as well.             *
 *                                                                                             *
 *    Answers for should_exclude_from_selection, which is the rule that already keeps a Crystal *
 *    Miner out of a mixed drag box.  The terms are that rule's and they are the point of it:   *
 *    only when the box caught something that is not excluded too, and only when the drag is    *
 *    not additive.  A box over the army leaves the carriers on their pad; a box over nothing   *
 *    but carriers still selects them; shift-drag still adds them.                              *
 *                                                                                             *
 *    A Recon Dropship swept up with the army is a Recon Dropship flown into the attack, and    *
 *    the player does not find out until it is shot down with the raid inside it.               *
 *                                                                                             *
 *    Read off IsTransporter rather than a list of codes, so anything carrying troops is        *
 *    covered whether or not it existed when this was written.                                  *
 *=============================================================================================*/
bool CAC_Band_Excluded(ObjectClass const *obj)
{
	if (obj == NULL) return(false);
	if (obj->What_Am_I() != RTTI_AIRCRAFT) return(false);

	AircraftClass const *craft = (AircraftClass const *)obj;
	if (craft->Class == NULL) return(false);
	if (!CAC_Is_CrystAlien_Type(craft->Class->IniName)) return(false);

	return(craft->Class->IsTransporter != 0);
}


/***********************************************************************************************
 * CAC_Engineers_Answer -- Does this click belong to the engineers alone?                       *
 *                                                                                             *
 *    CrystAlien decides a group order by what the click is worth to each unit, and a building  *
 *    an engineer can enter is worth more to the engineer than to anything else in the          *
 *    selection.  Select a squad with a Saboteur among it, click an enemy structure, and only   *
 *    the Saboteur goes.                                                                       *
 *                                                                                             *
 *    Tiberian Dawn asks every selected object what it would do and lets each do it, so the     *
 *    troopers opened fire on the building the Saboteur was walking into and usually levelled   *
 *    it first.  The cursor already knew better -- DLLInterface's action table has asked the    *
 *    capturer since the baseplates went in -- so the player was shown "capture" and given an   *
 *    attack, which is the worst of both.                                                      *
 *                                                                                             *
 *    Only when a capture is genuinely on offer: a selection with no engineer in it, or a       *
 *    target no engineer would enter, is left exactly as it was.                                *
 *=============================================================================================*/
bool CAC_Engineers_Answer(ObjectClass *target)
{
	if (target == NULL) return(false);
	if (target->What_Am_I() != RTTI_BUILDING) return(false);

	ObjectClass *capturer = CAC_Selected_Capturer();
	if (capturer == NULL) return(false);

	return(capturer->What_Action(target) == ACTION_CAPTURE);
}


ObjectClass * CAC_Selected_Capturer(void)
{
	if (CurrentObject.Count() < 2) return(NULL);

	for (int i = 0; i < CurrentObject.Count(); i++) {
		ObjectClass * obj = CurrentObject[i];

		/*
		**	The selection is a list like any other and can outlive what is in it.  This
		**	runs from the client's per-frame action table, so something that died this
		**	frame can still be listed, and What_Am_I is virtual.
		*/
		if (!CAC_Is_Capturer(obj)) continue;
		return(obj);
	}
	return(NULL);
}


/***********************************************************************************************
 * CAC_Group_Capturer -- On an enemy building, the engineer in the group wins.                 *
 *                                                                                             *
 *    CrystAlien decides a mixed selection over an enemy structure before it has looked at      *
 *    weapons at all:                                                                          *
 *                                                                                             *
 *        if (!this.activeTarget.friend) {                                                     *
 *            if (this.activeTarget.isBuilding) {                                              *
 *                for (i in this.selected) {                                                    *
 *                    if (this.selected[i].stats.repair) {                                      *
 *                        this.cursorState = "infiltrate";                                      *
 *                        return undefined;                                                     *
 *                    }                                                                         *
 *                }                                                                             *
 *            }                                                                                 *
 *            ... only now does it look for a weapon and offer "target"                         *
 *                                                                                             *
 *    One engineer anywhere in the selection and the whole group is capturing.  Tiberian Dawn   *
 *    ranks it the other way -- Best_Object_With_ActionT takes the best-priority action any     *
 *    member offers and ACTION_ATTACK outranks ACTION_CAPTURE -- so a Saboteur picked up along  *
 *    with an escort could not be told to walk into anything: the escort's attack answered      *
 *    first, every time, and the building simply got shot at instead.                            *
 *                                                                                             *
 *    Only for a building, and only for an enemy one.  Everything else the group might be       *
 *    pointed at is left to the engine's own ranking.                                           *
 *=============================================================================================*/
ObjectClass * CAC_Group_Capturer(DynamicVectorClass<ObjectClass *> &objects,
                                 ObjectClass * subject)
{
	if (subject == NULL || subject->What_Am_I() != RTTI_BUILDING) return(NULL);
	if (objects.Count() < 2) return(NULL);

	BuildingClass * target = (BuildingClass *)subject;
	if (target->Class == NULL || !CAC_Is_CrystAlien_Type(target->Class->IniName)) return(NULL);
	if (PlayerPtr != NULL && PlayerPtr->Is_Ally(target)) return(NULL);

	for (int i = 0; i < objects.Count(); i++) {
		ObjectClass * obj = objects[i];
		if (obj == NULL || !obj->IsActive) continue;
		if (obj->What_Am_I() != RTTI_INFANTRY) continue;

		InfantryClass * man = (InfantryClass *)obj;
		if (man->Class == NULL || !man->Class->IsCapture) continue;
		if (!CAC_Is_CrystAlien_Type(man->Class->IniName)) continue;

		if (man->What_Action(subject) == ACTION_CAPTURE) return(obj);
	}
	return(NULL);
}


/*
**	And the same question asked of a bare cell, which is never a building.
**	Best_Object_With_ActionT is a template over both, so both have to answer.
*/
ObjectClass * CAC_Group_Capturer(DynamicVectorClass<ObjectClass *> &, CELL)
{
	return(NULL);
}


/***********************************************************************************************
 * The scorch a baseplate was standing on, kept until the plate lifts.                          *
 *                                                                                             *
 *    A cell holds one smudge.  Every CrystAlien structure stands on a baseplate, and the plate *
 *    is drawn as that cell's smudge, so a scorch landing on plated ground would erase the       *
 *    plate of a building that is very often still alive.  SmudgeClass::Mark therefore refuses   *
 *    it -- which is right, and which is also why a superweapon leaves a hole in its own mark    *
 *    wherever a plate was, and why a destroyed building's scorch is missing the strip its own   *
 *    plate occupied.                                                                           *
 *                                                                                             *
 *    The refusal is not the whole story, because the plate does not stay.  BuildingClass::Mark  *
 *    calls SmudgeClass::Disown on MARK_UP and that blanks the plate's cells outright, so the    *
 *    ground the plate was hiding ends up bare -- a clean hole in the middle of a blast that     *
 *    covered everything around it.  The order is what does it: Take_Damage lays the scorches    *
 *    while the building is still standing and its plate still owns those cells, and the         *
 *    building is not actually removed until its death countdown runs out several frames later.  *
 *                                                                                             *
 *    So the scorch is not thrown away; it is put here and applied when the plate lifts.  Both   *
 *    faults close at once: the superweapon's mark completes across a base as the buildings in   *
 *    it come down, and a structure's own scorch covers the strip it was standing on.  A plate   *
 *    under a building that survives keeps it, which is the behaviour worth protecting.          *
 *                                                                                             *
 *    CrystAlien has no such problem and no such rule -- Level.explode attaches its scorch to    *
 *    arena.scorchMC, a display layer of its own that draws over the tile and everything on it.  *
 *    This is the nearest a single smudge per cell can get to that.                              *
 *=============================================================================================*/
static unsigned char CACScorchType[MAP_CELL_TOTAL];
static unsigned char CACScorchData[MAP_CELL_TOTAL];
static unsigned int  CACScorchFrame[MAP_CELL_TOTAL];


void CAC_Reset_Scorches(void)
{
	for (int i = 0; i < MAP_CELL_TOTAL; i++) {
		CACScorchType[i] = (unsigned char)SMUDGE_NONE;
		CACScorchData[i] = 0;
		CACScorchFrame[i] = 0;
	}
}


void CAC_Defer_Scorch(CELL cell, SmudgeType type, unsigned char data)
{
	if ((unsigned)cell >= MAP_CELL_TOTAL) return;
	if (type == SMUDGE_NONE) return;

	/*
	**	The superweapon's mark outranks an ordinary scorch, since one blast that
	**	covers nine cells should not be broken up by whatever died on the edge of
	**	it a moment earlier.  Otherwise the most recent wins.
	*/
	if (CACScorchType[cell] == (unsigned char)SMUDGE_CAC_SUPERSC &&
		 type != SMUDGE_CAC_SUPERSC) return;

	CACScorchType[cell] = (unsigned char)type;
	CACScorchData[cell] = data;
	CACScorchFrame[cell] = (unsigned int)Frame;
}


bool CAC_Pending_Scorch(CELL cell, SmudgeType &type, unsigned char &data)
{
	if ((unsigned)cell >= MAP_CELL_TOTAL) return(false);
	if (CACScorchType[cell] == (unsigned char)SMUDGE_NONE) return(false);

	/*
	**	Held for a few minutes and no longer.  A plate that lifts long after the
	**	blast -- a building sold in the endgame on ground something died on early
	**	-- should not suddenly scorch, and without an expiry these would sit for
	**	the whole round waiting to surprise somebody.
	*/
	bool live = ((unsigned int)Frame - CACScorchFrame[cell]) <= CAC_SCORCH_MEMORY;

	type = (SmudgeType)CACScorchType[cell];
	data = CACScorchData[cell];

	CACScorchType[cell] = (unsigned char)SMUDGE_NONE;
	CACScorchData[cell] = 0;
	CACScorchFrame[cell] = 0;

	return(live);
}


char const * CAC_Ground_Name(char const * stock)
{
	if (stock == NULL) return(NULL);

	/*
	**	Empty, and staying empty.  The answer was no.
	**
	**	CLEAR1 was reported as CACCLEAR1 with sixteen tileset shapes of its own
	**	pointing at a present archive, which is exactly the shape of the baseplate
	**	entries that do work -- and the log shows the client asking for
	**	CACCLEAR1_i0 through _i15 and drawing every one of them as a
	**	missing-texture quad.
	**
	**	That was the third of three.  A stock tile name pointed at a mod archive
	**	fails, a mod frame path under a stock tile name fails, and a tile name of
	**	our own fails as well.  What separates the baseplates from all three is not
	**	the name: it is the channel.  A smudge reaches the client through
	**	CNCDynamicMapEntryStruct with an AssetName, and that path consults the mod
	**	folder.  Base terrain goes through CNCStaticCellStruct as a template name
	**	and an icon, and that path resolves against the shipped textures only.
	**
	**	So the ground cannot be repainted from a mod, and no naming makes it so.
	**	Left here rather than deleted because the next person to have the idea will
	**	find the reason it does not work instead of the room to try it again.
	*/
	return(NULL);
}


bool CAC_Ignores_LZ(AircraftClass const * craft)
{
	if (craft == NULL || craft->Class == NULL) return(false);
	return(CAC_Is_CrystAlien_Type(craft->Class->IniName));
}


bool CAC_Never_Lands(AircraftClass const * craft)
{
	if (craft == NULL || craft->Class == NULL) return(false);
	if (!CAC_Is_CrystAlien_Type(craft->Class->IniName)) return(false);
	return(!CAC_Is_Lander(*craft));
}


/***********************************************************************************************
 * CAC_Home_Cursor_Ok -- The selection-count half of CAC_Sends_Home, and why it said no.        *
 *                                                                                             *
 *    Split out so that the refusal can be reported.  By the time this is called the hovered    *
 *    building is known to be this unit's own home, so anything it turns down is a case where   *
 *    the player was asking for exactly the right thing and got a plain cursor back.            *
 *=============================================================================================*/
static bool CAC_Home_Cursor_Ok(TechnoClass const * object, int kind)
{
	int picked = CurrentObject.Count();
	if (picked == 1) return(true);

	/*
	**	Said once per selection size, per kind, so hovering does not write a line a
	**	frame -- What_Action runs continuously while the pointer sits on a building.
	*/
	if (CAC_Logging()) {
		static int _said[2] = { -1, -1 };
		if (kind >= 0 && kind < 2 && _said[kind] != picked) {
			_said[kind] = picked;
			CAC_Log_Volatile("berth",
				"home cursor withheld: %d selected, and CrystAlien offers it at one",
				picked);
		}
	}
	object;
	return(false);
}


/***********************************************************************************************
 * CAC_Sends_Home -- Is this a click that should send the thing back to its station?           *
 *                                                                                             *
 *    CrystAlien offers it, and this had none of it:                                            *
 *                                                                                             *
 *        if (this.selected.length == 1 && this.selected[0].stats.home == this.activeTarget     *
 *            && (this.selected[0].stats.miner || this.selected[0].stats.boomerang))            *
 *        {                                                                                     *
 *            this.cursorState = "infiltrate";                                                  *
 *            return undefined;                                                                 *
 *        }                                                                                     *
 *                                                                                              *
 *    One selected, over its own home, and it is a miner or a boomerang.  A player can send a    *
 *    Fighter back early that way -- to rearm it, or to get it out of trouble -- and there was   *
 *    no way to ask for that here at all.                                                       *
 *                                                                                              *
 *    Exactly one selected, as the source says.  Pointing a squadron at its station is not this  *
 *    gesture, and the cursor should not claim it is.                                            *
 *=============================================================================================*/
bool CAC_Sends_Home(TechnoClass const * object, ObjectClass const * target)
{
	if (object == NULL || target == NULL) return(false);
	if (target->What_Am_I() != RTTI_BUILDING) return(false);

	/*
	**	The selection count is asked *last* rather than first.
	**	
	**	It is the source's own rule -- `this.selected.length == 1` -- and it is the
	**	most likely reason a player who has a squadron picked and hovers the station
	**	sees nothing.  Asked first, the routine could not tell the difference between
	**	"this is not a station" and "this is your station but you have four selected",
	**	which is the difference that matters when somebody reports the cursor missing.
	*/

	TechnoTypeClass const * type = object->Techno_Type_Class();
	if (type == NULL || !CAC_Is_CrystAlien_Type(type->IniName)) return(false);

	/*
	**	A boomerang, or a laden miner going back to unload.  Both are "home" in the
	**	source, and both are reached the same way.
	*/
	bool boomerang = (object->What_Am_I() == RTTI_AIRCRAFT &&
							  CAC_Is_Boomerang(*(AircraftClass const *)object));
	bool miner = (object->What_Am_I() == RTTI_UNIT &&
					  CAC_Is_Miner(*(UnitClass const *)object));
	if (!boomerang && !miner) return(false);

	/*
	**	No cast.  CAC_Home_Building takes a TechnoClass and asks What_Am_I for
	**	itself; casting a miner to an AircraftClass on the way in was wrong however
	**	harmless it happened to be, and this runs on every hover over a building.
	*/
	BuildingClass * home = CAC_Home_Building(object);
	if (boomerang) {
		if (home == NULL || home != (BuildingClass const *)target) return(false);
		return(CAC_Home_Cursor_Ok(object, 1));
	}

	/*
	**	A miner's home is whichever headquarters it delivers to, which the player
	**	may have chosen; CAC_Is_Refinery is the same question asked of a building.
	*/
	if (((TechnoClass *)object)->House != ((TechnoClass const *)target)->House) return(false);
	if (!CAC_Is_Refinery((StructType)*(BuildingClass const *)target)) return(false);
	return(CAC_Home_Cursor_Ok(object, 0));
}


bool CAC_At_Berth(AircraftClass const * craft)
{
	if (craft == NULL) return(false);

	BuildingClass * home = CAC_Home_Building(craft);
	if (home == NULL) return(false);

	/*
	**	Close enough, not exactly on it.
	**
	**	CrystAlien compares tiles and gets an exact answer, because voyage() puts
	**	the unit on the tile it was given.  Tiberian Dawn flies an aircraft toward
	**	a coordinate and stops it when it is near, so a Fighter settles a cell or
	**	so short of its berth and never satisfies an equality test.
	**
	**	That was the whole of "they will not reseat".  One session's log has nine
	**	hundred and twenty-one sends against five arrivals, and the sends read
	**	"to 3,59 from 2,59" and "to 42,50 from 42,51" -- already there, told to go
	**	again, once a second, forever.  Worse than untidy: each order resets the
	**	idle count the descent is waiting on, so being repeatedly sent home is
	**	exactly what stopped them landing.
	*/
	CELL berth = CAC_Exit_Cell(home, craft);
	if ((unsigned)berth >= MAP_CELL_TOTAL) return(false);

	CELL here = Coord_Cell(craft->Center_Coord());
	return(ABS(Cell_X(here) - Cell_X(berth)) <= CAC_BERTH_SLACK &&
	       ABS(Cell_Y(here) - Cell_Y(berth)) <= CAC_BERTH_SLACK);
}


COORDINATE CAC_Berth_Coord(AircraftClass const * craft)
{
	if (craft == NULL) return(0);

	BuildingClass * home = CAC_Home_Building(craft);
	if (home == NULL) return(0);

	CELL berth = CAC_Exit_Cell(home, craft);
	if ((unsigned)berth >= MAP_CELL_TOTAL) return(0);
	return(Cell_Coord(berth));
}


/***********************************************************************************************
 * CAC_On_Berth -- Is this flier standing on its plate, rather than merely near it?             *
 *                                                                                             *
 *    CAC_At_Berth answers a looser question and has to: it decides whether CAC_Return_Home     *
 *    should order the craft home again, and asking that exactly meant re-ordering a Fighter    *
 *    that was already standing there, once a second, forever.                                  *
 *                                                                                             *
 *    But the descent gate was reading the same loose answer, so a craft that stopped two       *
 *    cells short of its station set down two cells short of it -- "they land away from the     *
 *    actual baseplate".  Two cells is what CAC_BERTH_SLACK allows and two cells is what they   *
 *    were doing.                                                                               *
 *                                                                                             *
 *    So the two questions are separated.  This one is the landing test, and it is tight:       *
 *    Process_Fly_To stops inside 0x0010 of its destination, so one short approach satisfies    *
 *    it and the craft comes down on the artwork.                                               *
 *=============================================================================================*/
bool CAC_On_Berth(AircraftClass const * craft)
{
	COORDINATE berth = CAC_Berth_Coord(craft);
	if (berth == 0) return(false);
	return(::Distance(craft->Center_Coord(), berth) < CAC_BERTH_EXACT);
}


bool CAC_Berth_Is_Clear(TechnoClass const * craft)
{
	if (craft == NULL) return(false);

	CELL cell = Coord_Cell(craft->Center_Coord());
	if ((unsigned)cell >= MAP_CELL_TOTAL) return(false);

	/*
	**	And not onto rock or water.  The landing test in UnitNav ends with a
	**	terrain term this port did not have:
	**
	**	    if (this.still > 20 && (!this.stats.boomerang || this.atHome) &&
	**	        !this.parent.parent.arena.tiles[this.parent.tilePos.x][this.parent.tilePos.y])
	**
	**	`tiles` is the rock, cliff and water overlay -- the sparse layer laid over
	**	the Mars backdrop -- so a lander comes down on open ground and nowhere
	**	else.  Crystal is not in it:  resource cells are `bait`, a separate layer
	**	(which is why doSnow can skip them), so settling on a crystal field is
	**	allowed and always was.
	**
	**	Ground's own cost table says the same thing without a list of land types
	**	to keep in step -- a cost of zero is impassable, which is rock, water and
	**	wall, while clear, road, beach and tiberium all carry a real cost.
	*/
	if (Ground[Map[cell].Land_Type()].Cost[SPEED_TRACK] == 0) return(false);

	for (ObjectClass * obj = Map[cell].Cell_Occupier(); obj != NULL; obj = obj->Next) {
		if (obj == (ObjectClass const *)craft || !obj->IsActive) continue;

		/*
		**	Another of CrystAlien's fliers is not in the way -- it has no block on the
		**	tile to be in the way with.  Anything else is.
		*/
		if (obj->What_Am_I() == RTTI_AIRCRAFT) {
			TechnoTypeClass const * type = ((TechnoClass *)obj)->Techno_Type_Class();
			if (type != NULL && CAC_Is_CrystAlien_Type(type->IniName)) continue;
		}

		/*
		**	Nor is somebody climbing aboard this very craft, and that one is the
		**	difference between the two games rather than an exception to the rule.
		**
		**	CrystAlien has no walk-in.  Unit.doEmbark, called the moment the unit is
		**	beside a carrier that is all the way down --
		**
		**	    this.posX = this.posY = -1000;
		**	    this.target.stats.contents.push(this);
		**
		**	-- puts the passenger at minus a thousand and inside, so it never stands
		**	on the carrier's tile and `arena.tiles[tilePos]` never stops being empty.
		**	Tiberian Dawn walks its passengers in instead, one at a time, through the
		**	very cell this routine is asking about.
		**
		**	So the carrier rose every time somebody stepped up to board it and sank
		**	again once they were swallowed, for as long as anyone was still walking:
		**	"units can't board a transport ship because it keeps raising while
		**	they're trying to board".  The passenger is not an obstruction, it is the
		**	errand.
		*/
		if (obj->Is_Techno() && ((TechnoClass *)obj)->In_Radio_Contact() &&
			 ((TechnoClass *)obj)->Contact_With_Whom() == (TechnoClass const *)craft) {
			continue;
		}

		/*
		**	Nor is a foot soldier, ever.
		**
		**	UnitStats gives UA and UB `obstruct = false`, and Unit.setBlock writes a
		**	tile only `if (this.stats.obstruct)` -- so in CrystAlien a soldier or an
		**	engineer never occupies a tile at all and can never be what keeps a
		**	lander in the air.  Here they occupy cells like anything else, and the
		**	carrier was reading them as an obstruction in both directions: passengers
		**	stepping off onto its own cell, and passengers queued to get on who had not
		**	yet been given the radio.  So it settled, opened its door, the first man
		**	out stood on the square, and it climbed again with the rest still inside --
		**	which is also why the disembark order kept vanishing from under the
		**	cursor, the craft being back in the air.  The log has it doing that every
		**	two to four seconds, still counting past five hundred throughout.
		*/
		if (obj->What_Am_I() == RTTI_INFANTRY) continue;

		/*
		**	And anything on its way aboard this craft, radio or not.  Tiberian Dawn
		**	gives the radio to one passenger at a time, so a squad sent to board has
		**	one man talking and the rest standing about on the square waiting.
		*/
		if (obj->What_Am_I() == RTTI_UNIT) {
			FootClass const *f = (FootClass const *)obj;
			TARGET mine = ((TechnoClass const *)craft)->As_Target();
			if (f->NavCom == mine || f->TarCom == mine) continue;
		}

		return(false);
	}
	return(true);
}


bool CAC_Aircraft_Hovers(AircraftType type)
{
	if (!CAC_Is_CrystAlien_Type(AircraftTypeClass::As_Reference(type).IniName)) return(false);

	/*
	**	A boomerang is the opposite of holding station -- see CAC_Homes_To -- and
	**	the transformer's whole purpose is to set down.
	*/
	if (CAC_Is_Boomerang(type)) return(false);
	return(!CAC_Is_Transformer(type));
}

/***********************************************************************************************
 * CAC_Undeploys_Into -- Which vehicle a structure folds back into.                            *
 *                                                                                             *
 *    The inverse of CAC_Deploys_Into.  Tiberian Dawn already does this for the Construction   *
 *    Yard, which reverts to an MCV when told to move, and the MX-81 Ops Ship wants the same:  *
 *    BK_good is the Astro headquarters drawn as a craft that has set down, so it should be    *
 *    able to take off again.                                                                  *
 *                                                                                             *
 *    The stock routine also insists on a legal ArchiveTarget -- its way of remembering that    *
 *    this building really did arrive as a vehicle.  An Ops Ship built from the sidebar has no  *
 *    such history and would be stuck on the ground forever, so that condition is not applied   *
 *    to it.                                                                                   *
 *                                                                                             *
 * OUTPUT:  UNIT_NONE for a structure that stays where it is put.                              *
 *=============================================================================================*/
UnitType CAC_Undeploys_Into(StructType type)
{
	if (type == STRUCT_NONE) return(UNIT_NONE);
	if (CAC_Is_Stock_Structure(type)) {
		return((type == STRUCT_CONST) ? UNIT_MCV : UNIT_NONE);
	}
	/*
	**	And nothing of CrystAlien's folds back up.
	**
	**	The Ops Ship and the Hive are drawn as craft that have set down, and letting
	**	them take off again was my idea rather than the source's -- there is no
	**	undeploy anywhere in it, and a headquarters that lands is a building from
	**	that point on.
	**
	**	It did not stay a nicety either.  Selling one ran the deconstruction into the
	**	MCV path instead, so the sale turned the building into a vehicle and left it
	**	sitting there: the demolition never happened and the money never came back.
	*/
	return(UNIT_NONE);
}

/***********************************************************************************************
 * CAC_Hover_Offset -- The swirl a flying unit holds station with.                             *
 *                                                                                             *
 *    Unit.draw, for anything with altitude:                                                   *
 *                                                                                             *
 *        _loc2_ = altitude + 100 * Math.sin(count / (100 / vibrate)) * 0.05;                  *
 *        this.MC._x += Math.cos(count / vibrate) * 6;                                         *
 *        this.MC._y += Math.cos(count / vibrate * 0.8) * 3;                                   *
 *                                                                                             *
 *    -- a lateral drift of six pixels by three, and a five pixel bob about the unit's resting  *
 *    altitude, all on the unit's own frame counter.  It is what makes CrystAlien's craft look  *
 *    like they are holding station rather than nailed to the ground.                          *
 *                                                                                             *
 *    Ground units get the other arm of the same routine: a small random rumble, bigger while   *
 *    moving.  That one is left alone, being a per-frame jitter that would only make the C&C    *
 *    sprite look unstable at a different frame rate.                                          *
 *                                                                                             *
 *    Pixels are CrystAlien's, at 96 to a tile against the HD grid's 128, and its frames run    *
 *    at 23 against 15 ticks.                                                                  *
 *=============================================================================================*/
struct CACHoverEntry {
	RTTIType	RTTI;
	int			Type;
	int			Vibrate;
	bool		Drifts;	/* altitude > 0 -- drawn off the ground */
	bool		Lands;
	int			Speed;	/* the source's own, which the ground rumble scales by */
};

static CACHoverEntry const CACHover[] = {
#include "CACHOVER.INC"
};

static CACHoverEntry const *CAC_Hover_Entry(RTTIType rtti, int type)
{
	RTTIType want = rtti;
	if (rtti == RTTI_UNIT)     want = RTTI_UNITTYPE;
	if (rtti == RTTI_INFANTRY) want = RTTI_INFANTRYTYPE;
	if (rtti == RTTI_AIRCRAFT) want = RTTI_AIRCRAFTTYPE;

	for (int i = 0; i < (int)(sizeof(CACHover) / sizeof(CACHover[0])); i++) {
		if (CACHover[i].RTTI == want && CACHover[i].Type == type) return(&CACHover[i]);
	}
	return(NULL);
}


/***********************************************************************************************
 * CAC_Is_Lander -- Does this craft settle on the ground when it stops?                        *
 *                                                                                             *
 *    `lander` in UnitStats, which UnitNav reads as landerPerc: twenty frames without moving    *
 *    and the craft comes down; the moment it moves it goes back up.  Three carry it -- the     *
 *    Astro Fighter, the Recon Dropship and the Switch Fighter, with their Alien opposites --   *
 *    and the conversion knew about one.                                                        *
 *                                                                                             *
 *    Landing is not transforming.  Only the Switch Fighter's clip has banks drawn for being    *
 *    on the ground; the other two simply lose altitude, and asking for a landing bank they do  *
 *    not have would draw the client's missing-texture quad.  So CAC_Is_Transformer stays what  *
 *    it always was -- who has the artwork -- and this is who does the behaviour.               *
 *=============================================================================================*/
int CAC_Source_Speed(TechnoClass const * object)
{
	if (object == NULL) return(CAC_UPGOOD_SPEED);

	TechnoTypeClass const * type = object->Techno_Type_Class();
	if (type == NULL || !CAC_Is_CrystAlien_Type(type->IniName)) return(CAC_UPGOOD_SPEED);

	CACHoverEntry const * e = CAC_Hover_Entry(object->What_Am_I(),
															 CAC_Type_Index_Of_Type(type));
	if (e == NULL || e->Speed <= 0) return(CAC_UPGOOD_SPEED);
	return(e->Speed);
}


bool CAC_Is_Lander(AircraftType type)
{
	CACHoverEntry const *e = CAC_Hover_Entry(RTTI_AIRCRAFTTYPE, (int)type);
	return(e != NULL && e->Lands);
}


void CAC_Hover_Offset(ObjectClass const *object, int &x, int &y)
{
	if (object == NULL || !object->Is_Techno()) return;

	TechnoTypeClass const *type = ((TechnoClass const *)object)->Techno_Type_Class();
	if (type == NULL || !CAC_Is_CrystAlien_Type(type->IniName)) return;

	CACHoverEntry const *e = CAC_Hover_Entry(object->What_Am_I(),
											CAC_Type_Index_Of_Type(type));
	if (e == NULL || e->Vibrate <= 0) return;

	/*
	**	The other half of Unit.draw, which was recorded and never written.
	**	
	**	    if (this.stats.altitude > 0) { ...the two cosines... }
	**	    else {
	**	        _loc2_ = random(this.stats.vibrate) / 10;
	**	        if (this.nav.progress) _loc2_ += this.stats.speed * random(this.stats.vibrate) / 40;
	**	    }
	**	    this.MCsprite.unit._y = (-bank._height) / 2 - _loc2_;
	**	
	**	So anything drawn *on* the ground with a vibrate shivers vertically -- a
	**	fresh random lift every frame, and more of it while it is moving.  Every
	**	foot soldier has it, the Astro vehicles have it, and the reindeer have the
	**	strongest of any of them at fifteen.  The generator has been emitting the
	**	numbers for all of them since the table was written and this routine threw
	**	them away: `!e->Drifts` sent every ground unit straight back out.
	**	
	**	Vertical only, and no horizontal term at all -- the cosines belong to the
	**	airborne branch, and giving a walking soldier a sideways wobble would look
	**	like a pathing fault rather than an idle.
	*/
	bool airborne = e->Drifts;
	if (object->What_Am_I() == RTTI_AIRCRAFT) {
		airborne = (((AircraftClass const *)object)->Altitude > 0);
	}

	if (!airborne) {
		/*
		**	The presentation generator, not the simulation's.
		**	
		**	Sim_Random_Pick is Sim_IRandom -- the deterministic stream the game logic
		**	runs on, and everything has to draw from it in the same order or the
		**	simulation stops agreeing with itself.  This routine is called from
		**	DLL_Draw_Intercept, once per object per *rendered* frame, at a rate set by
		**	the camera, the framerate and how much is on screen.  Advancing the sim
		**	from there makes the game's own decisions depend on what was being looked
		**	at -- which is a class of bug that shows up as unexplainable divergence
		**	rather than as a crash, and would be at its worst in a long game with six
		**	houses all making decisions.
		**	
		**	Random_Pick is IRandom, which is for exactly this: cosmetic, unsynchronised
		**	and nobody's business but the local machine's.
		*/
		int lift = Random_Pick(0, e->Vibrate - 1) / 10;

		/*
		**	`nav.progress` is a step in flight, so the extra applies while the thing
		**	is actually crossing ground rather than merely holding an order.
		*/
		RTTIType kind = object->What_Am_I();
		bool moves = (kind == RTTI_UNIT || kind == RTTI_INFANTRY || kind == RTTI_AIRCRAFT);
		if (e->Speed > 0 && moves && ((FootClass const *)object)->Speed != 0) {
			lift += (e->Speed * Random_Pick(0, e->Vibrate - 1)) / 40;
		}

		y -= lift;
		return;
	}

	/*
	**	Unit.draw only drifts something drawn off the ground:
	**
	**	    if (this.stats.altitude > 0) { ...the two cosines... }
	**
	**	which is not the same as flying.  The Alien Crystal Extractor, Speeder and
	**	Dragon Cruiser are ground vehicles that float -- `flying: false`,
	**	`obstruct: true`, `altitude: 20` -- where their Astro opposites sit at
	**	zero.  It is the one stat in the roster the two factions do not share.
	**
	**	and `stats.altitude` is the live value UnitNav rolls down to zero as
	**	landerPerc climbs.  So a craft that has set down, or is on its way down or
	**	up, holds still -- it is only in the air that it drifts.
	*/
	if (object->What_Am_I() == RTTI_AIRCRAFT &&
		((AircraftClass const *)object)->Altitude <= 0) return;

	int vibrate = e->Vibrate;

	/*
	**	The source counter runs per unit and per frame; ours is the game frame
	**	staggered by the object's own id, converted to its frame rate.
	*/
	double count = ((double)Frame * CAC_FPS_NUM / CAC_FPS_DEN) + (object->As_Target() & 0x3F);

	/*
	**	Unit.draw does three things to an airborne craft, and only two of them
	**	carry across:
	**
	**	    MC._x += cos(count / vibrate) * 6
	**	    MC._y += cos(count / vibrate * 0.8) * 3
	**	    MCsprite.unit._y -= altitude + 100 * sin(count / (100/vibrate)) * 0.05
	**
	**	The first two drift the whole craft and are reproduced here at the source's
	**	own amplitudes.  The third is not a drift at all: it moves the body against
	**	its *own shadow*, which is a separate clip in the source and is baked into
	**	the frame here, so applying it to the object would rock the shadow with the
	**	craft -- reading as the ground swaying rather than the craft hovering, and
	**	doubling the vertical travel into the bargain.  It is left out.
	*/
	/*
	**	The source's six and three are *its* screen pixels, and its tile is 96 of
	**	them across.  These offsets go into the draw position in Tiberian Dawn's
	**	own pixels, where a cell is 24 -- and the client then scales that up by
	**	better than five to reach its 128.  Passing six through unconverted was
	**	asking for a drift of a third of a cell, which is why it read as a lurch
	**	rather than a hover.  Six pixels in 96 is a quarter of one in 24.
	*/
	double amp = 24.0 / 96.0;

	x += (int)(cos(count / vibrate) * 6.0 * amp);
	y += (int)(cos(count / vibrate * 0.8) * 3.0 * amp);
}

/***********************************************************************************************
 * CAC_Death_Effect -- The fireball or the puff, for the kinds that name neither.               *
 *                                                                                             *
 *    Level.explode plays the "explosion" clip's default sequence, or its `pulse` label when    *
 *    the dying thing is not `mechanical` -- `obstruct || flying`.  A UnitTypeClass names its   *
 *    animation in the type row, so vehicles are already covered; infantry and aircraft have    *
 *    no such field and are handled here.                                                      *
 *=============================================================================================*/
/***********************************************************************************************
 * CAC_Death_Shake -- Everything that dies rocks the camera, because in the source it does.      *
 *                                                                                             *
 *    Level.explode ends with one line carrying no condition whatever:                          *
 *                                                                                             *
 *        this.camera.shake += 5;                                                               *
 *                                                                                             *
 *    Tiberian Dawn shakes far more sparingly.  A vehicle rocks the screen only when             *
 *    `Class->MaxStrength > 400`, which on this roster is most of them and on the light end is   *
 *    none, and infantry and aircraft never do at all -- so a Jetpacker or a Speeder went up      *
 *    with a fireball, a scorch and no weight behind either.                                     *
 *                                                                                             *
 *    Five frames at the source's twenty-three is about a fifth of a second.  Shake_The_Screen   *
 *    banks `shakes + shakes` ticks at fifteen a second, so two lands on the same fifth.         *
 *                                                                                             *
 *    Structures are left to the rule they already have.  Building.freeBlock calls explode four  *
 *    times per tile, which for a two-by-two is sixteen shakes and three and a half seconds of   *
 *    camera -- bearable in a campaign with one player watching one base fall, not in a six-way  *
 *    skirmish.  Cost_Of()/400 is already in the same territory for anything worth noticing.     *
 *                                                                                             *
 *    And who feels it cannot be ported literally either, for the same reason: the owner and     *
 *    whoever killed it, which is the pair Tiberian Dawn already shakes and the only two the     *
 *    death is about.                                                                            *
 *=============================================================================================*/
void CAC_Death_Shake(TechnoClass const *obj, TechnoClass const *source)
{
	if (obj == NULL) return;
	if (obj->What_Am_I() == RTTI_BUILDING) return;

	TechnoTypeClass const *type = obj->Techno_Type_Class();
	if (type == NULL || !CAC_Is_CrystAlien_Type(type->IniName)) return;

	Shake_The_Screen(CAC_DEATH_SHAKE, obj->Owner());
	if (source != NULL && source->Owner() != obj->Owner()) {
		Shake_The_Screen(CAC_DEATH_SHAKE, source->Owner());
	}
}


void CAC_Death_Effect(TechnoClass *obj)
{
	if (obj == NULL) return;

	RTTIType what = obj->What_Am_I();
	if (what != RTTI_INFANTRY && what != RTTI_AIRCRAFT) return;

	TechnoTypeClass const *type = obj->Techno_Type_Class();
	if (type == NULL || !CAC_Is_CrystAlien_Type(type->IniName)) return;

	/*
	**	Aircraft are flying and so always mechanical; infantry neither obstruct
	**	nor fly, so they always puff.
	*/
	AnimType death = (what == RTTI_AIRCRAFT) ? ANIM_CAC_BOOM : ANIM_CAC_PUFF;
	/*
	**	Volatile: a Petroglyph square has been reported where infantry and aircraft
	**	die, and every frame this mod registers has been checked and resolves.  So
	**	something drawn here is asked for by a name nobody provides, and this is the
	**	line that will say which.
	*/
	CAC_Log_Volatile("death", "%s (%s) -> anim %s at %d,%d", type->IniName,
							 (what == RTTI_AIRCRAFT) ? "aircraft" : "infantry",
							 AnimTypeClass::As_Reference(death).IniName,
							 Cell_X(Coord_Cell(obj->Center_Coord())),
							 Cell_Y(Coord_Cell(obj->Center_Coord())));

	/*
	**	And what that animation will stamp on the ground when it reaches its middle
	**	frame.  AnimClass::Middle places a smudge for a scorcher or a crater-former,
	**	which is a terrain decal rather than part of the animation -- a different
	**	asset, resolved by a different name, in a different folder.
	*/
	{
		AnimTypeClass const & dc = AnimTypeClass::As_Reference(death);
		if (dc.IsScorcher || dc.IsCraterForming) {
			/*
			**	Name the decal, not the category.
			**
			**	This said "scorch" or "crater" whichever mark was really stamped, so it
			**	read identically while the conversion was quietly laying Tiberian Dawn's
			**	SC1..SC6 and CR1 instead of its own -- for however long that was true.
			**	Nothing looked broken: the stock decals resolve, so there is no missing
			**	texture to notice, only the wrong game's marks underfoot.  Naming what is
			**	placed is the difference between a log that can show that and one that
			**	cannot.
			*/
			bool ours = CAC_Is_CrystAlien_Effect(dc.IniName);
			CAC_Log_Volatile("death", "  marks the ground with %s (%s)",
									 ours ? "CACSCRCH" : (dc.IsScorcher ? "SC1..SC6" : "CR1"),
									 ours ? "CrystAlien's" : "STOCK -- see CAC_Is_CrystAlien_Effect");
		}
	}
	/*
	**	Where it was, not the ground under it.
	**
	**	Center_Coord is the cell the craft is over; an aircraft's height lives in
	**	Altitude and the renderer lifts the sprite by it at draw time, so an
	**	animation placed on the centre appears on the floor while the wreck is still
	**	in the air.  Target_Coord is the virtual that already carries the lift --
	**
	**	    return(Coord_Add(XYP_Coord(0, -Altitude), Coord));
	**
	**	-- and it is what Tiberian Dawn's own aircraft death uses for exactly this
	**	reason.  For infantry and vehicles it is the same point, so the one call
	**	serves both.
	**
	**	The source passes the lifted position too, and passes it for the scorch as
	**	well as the fireball:
	**
	**	    this.parent.explode(this.posX, this.posY - this.stats.altitudeT2, ...)
	**
	**	-- one coordinate, both attachments, altitude already taken off.  So the
	**	mark landing where the craft was rather than under it is the source's
	**	behaviour and not an accident of this one.
	*/
	new AnimClass(death, obj->Target_Coord());
}



/***********************************************************************************************
 * CAC_Deploys_Into -- Which structure a vehicle turns into where it stands.                   *
 *                                                                                             *
 *    Tiberian Dawn has exactly one of these and names it inline everywhere: `*this ==         *
 *    UNIT_MCV`, deploying into CAC_SLOT_BAGOOD.  The supplemental build needs a second, because   *
 *    the MX-81 Ops Ship is the one CrystAlien structure that is a *ship* -- BK_good is the     *
 *    Astro headquarters and it is drawn as a craft that has set down.  CrystAlien itself       *
 *    never offers BK in a sidebar (Construction.checkPrereq never lists it; a level places     *
 *    it, or it is where you start), but a GDI or Nod player has to be able to reach the        *
 *    Ops Ship half of the tree -- the Driller and the Switch Fighter both hang off it -- and   *
 *    "it flies in and sets down" is what the thing is.                                        *
 *                                                                                             *
 *    So it is built as a vehicle with the winged locomotion and deploys into the structure,    *
 *    which is the MCV's own shape.  Turning the type test into this lookup is all that the     *
 *    engine needed.                                                                           *
 *                                                                                             *
 * OUTPUT:  STRUCT_NONE for anything that does not deploy.                                     *
 *=============================================================================================*/
StructType CAC_Deploys_Into(UnitType type, HousesType owner)
{
	if (type == UNIT_MCV) {
#ifndef CAC_SUPPLEMENTAL
		/*
		**	Nothing unfolds into a headquarters in the conversion.
		**
		**	CrystAlien has no construction vehicle and no deploy: a level either
		**	places the Eagle Command Base and the Mothership or it does not, and
		**	Construction.checkPrereq never offers either -- they are where a side
		**	begins, and losing one is meant to be final.  Borrowing Tiberian Dawn's
		**	MCV gave the conversion a way to rebuild one, which is a bigger
		**	departure than it looks: CAC_Is_Refinery carries a note about losing an
		**	Eagle Command being unrecoverable precisely because nothing rebuilds it.
		**
		**	So the headquarters is placed rather than unfolded -- by Create_Units at
		**	the start of a match, and directly by the scenario in the campaign, where
		**	make_missions rewrites every MCV into the side's own headquarters.  An
		**	MCV that turns up regardless is simply a vehicle that does nothing.
		*/
		owner;
		return(STRUCT_NONE);
#endif
#ifdef CAC_SUPPLEMENTAL
		/*
		**	GDI and Nod still have their own Construction Yard here, and unfolding
		**	their MCV into an Eagle Command Base would take their whole game away
		**	to hand them somebody else's.  They reach CrystAlien's tree through the
		**	yard instead -- see CAC_Owns_HQ.
		*/
		owner;
		return(STRUCT_CONST);
#else
		/*
		**	A mobile construction vehicle unfolds into the headquarters of
		**	whichever faction is driving it.  With a type per side there is no
		**	single Construction Yard to name any more, so the owner decides.
		*/
		int slot = CAC_Slot_For(RTTI_BUILDINGTYPE, CAC_CODE_BA,
								CAC_Side(owner) == HOUSE_BAD);
		return((slot >= 0) ? (StructType)slot : STRUCT_CONST);
#endif
	}

	if (CAC_Is_Code(RTTI_UNITTYPE, (int)type, CAC_CODE_BS)) {
		/*	And an Ops Ship sets down as the ship it is, not the captor's. */
		bool evil = (CAC_Code_Of(RTTI_UNITTYPE, (int)type) & CAC_CODE_EVIL) != 0;
		int slot = CAC_Slot_For(RTTI_BUILDINGTYPE, CAC_CODE_BK, evil);
		if (slot >= 0) return((StructType)slot);
	}
	return(STRUCT_NONE);
}


/***********************************************************************************************
 * CAC_Is_Stock_Refinery -- Is this slot Tiberian Dawn's own Refinery?                         *
 *                                                                                             *
 *    The conversion puts the MT-201 Driller on STRUCT_REFINERY, which drags two of the stock  *
 *    Refinery's special cases along with it:                                                  *
 *                                                                                             *
 *      BuildingTypeClass::Raw_Cost()  subtracts a Harvester's price, because a stock Refinery *
 *                                     comes with one -- so the Driller was charged 1600 of    *
 *                                     its 3000.                                               *
 *      BuildingClass::Grand_Opening() hands out that free Harvester.                          *
 *                                                                                             *
 *    Neither is CrystAlien behaviour.  Its Driller costs 3000 flat and comes with nothing: it *
 *    is a passive income building that pays `revenue` every `revenueCount` frames and         *
 *    re-seeds crystal under its own footprint (Building.refine).  No miner docks at it --     *
 *    CrystAlien's miner homes to findBuilding("BA_" + team), the Command Base.                *
 *                                                                                             *
 * OUTPUT:  true only for a genuine GDI/Nod Refinery, which exists in the supplemental build   *
 *          and not in the conversion.                                                         *
 *=============================================================================================*/
bool CAC_Is_Stock_Refinery(StructType type)
{
	if (type != STRUCT_REFINERY) return(false);
	return(CAC_Is_Stock_Structure(type));
}



/***********************************************************************************************
 * CAC_Mix_Type -- In the supplemental build, may this placed stock object be CrystAlien?     *
 *                                                                                             *
 *    "Placed and immediately available units and buildings should randomize, still           *
 *    satisfying scenario conditions, between C&C and CrystAlien -- a GDI mission would have   *
 *    random GDI troops and Astro units."  So each stock object a scenario places, and each    *
 *    of a skirmish's starting units, is a coin toss between itself and the CrystAlien type   *
 *    the conversion puts in its place (make_missions.SWAP -- the verifier keeps the two       *
 *    tables agreeing), on its owner's side: GDI's are Astros, Nod's are Aliens.              *
 *                                                                                             *
 *    What a mission's scripting depends on is left alone, which is everything not in the     *
 *    table below and anything the caller says is locked:                                      *
 *                                                                                             *
 *      - an object carrying a trigger ("Destroyed", "Attacked", capture-to-win);             *
 *      - MCVs, harvesters, transports, the gunboat and the A-10, which the engine handles by  *
 *        type, and the civilians and named characters an evacuation counts;                   *
 *      - factories, the Construction Yard and the refinery, which the stock tree and the     *
 *        computer's production hang off;                                                      *
 *      - any building the computer's [Base] plan names, which it rebuilds by exact type;     *
 *      - a human's power plants, which the stock tree's prerequisites ask for by bit.        *
 *                                                                                             *
 *    Defences and the computer's spare power plants may change; a CrystAlien defence still   *
 *    answers a "Built It" for the stock one (CAC_Built_Counts_As).  Random_Pick is the       *
 *    game's own generator, which Create_Units already draws its picks from, so every peer    *
 *    agrees.                                                                                  *
 *                                                                                             *
 * INPUT:   rtti   -- RTTI_UNITTYPE, _INFANTRYTYPE, _AIRCRAFTTYPE or _BUILDINGTYPE.           *
 *          type   -- the stock type the scenario names.                                       *
 *          owner  -- the house that will own it.                                              *
 *          locked -- the caller knows the object must stay as it is.                          *
 *                                                                                             *
 * OUTPUT:  The type to create: the same one, or a CrystAlien slot.  Always the same one in    *
 *          the total conversion, whose missions are converted ahead of time.                  *
 *=============================================================================================*/
#ifdef CAC_SUPPLEMENTAL
struct CACMixEntry {
	char const *	Stock;
	RTTIType			RTTI;
	int				Good;
	int				Evil;
	bool				ComputerOnly;
};

static CACMixEntry const CACMix[] = {
	{"E1",	RTTI_INFANTRYTYPE,	CAC_CODE_UA, CAC_CODE_UA, false},
	{"E2",	RTTI_INFANTRYTYPE,	CAC_CODE_UA, CAC_CODE_UA, false},
	{"E3",	RTTI_INFANTRYTYPE,	CAC_CODE_UA, CAC_CODE_UA, false},
	{"E4",	RTTI_INFANTRYTYPE,	CAC_CODE_UA, CAC_CODE_UA, false},
	{"E5",	RTTI_INFANTRYTYPE,	CAC_CODE_UA, CAC_CODE_UA, false},
	{"E6",	RTTI_INFANTRYTYPE,	CAC_CODE_UB, CAC_CODE_UB, false},
	{"RMBO",	RTTI_INFANTRYTYPE,	CAC_CODE_UA, CAC_CODE_UQ, false},
	{"JEEP",	RTTI_UNITTYPE,		CAC_CODE_UE, CAC_CODE_UE, false},
	{"BGGY",	RTTI_UNITTYPE,		CAC_CODE_UE, CAC_CODE_UE, false},
	{"BIKE",	RTTI_UNITTYPE,		CAC_CODE_UE, CAC_CODE_UE, false},
	{"LTNK",	RTTI_UNITTYPE,		CAC_CODE_UF, CAC_CODE_UF, false},
	{"MTNK",	RTTI_UNITTYPE,		CAC_CODE_UF, CAC_CODE_UF, false},
	{"HTNK",	RTTI_UNITTYPE,		CAC_CODE_UF, CAC_CODE_UF, false},
	{"FTNK",	RTTI_UNITTYPE,		CAC_CODE_UF, CAC_CODE_UF, false},
	{"STNK",	RTTI_UNITTYPE,		CAC_CODE_UF, CAC_CODE_UF, false},
	{"APC",	RTTI_UNITTYPE,		CAC_CODE_UF, CAC_CODE_UF, false},
	{"ARTY",	RTTI_UNITTYPE,		CAC_CODE_UF, CAC_CODE_UF, false},
	{"MSAM",	RTTI_UNITTYPE,		CAC_CODE_UF, CAC_CODE_UF, false},
	{"ORCA",	RTTI_AIRCRAFTTYPE,	CAC_CODE_UG, CAC_CODE_UG, false},
	{"HELI",	RTTI_AIRCRAFTTYPE,	CAC_CODE_UG, CAC_CODE_UG, false},
	{"GUN",	RTTI_BUILDINGTYPE,	CAC_CODE_BD, CAC_CODE_BD, false},
	{"GTWR",	RTTI_BUILDINGTYPE,	CAC_CODE_BD, CAC_CODE_BD, false},
	{"ATWR",	RTTI_BUILDINGTYPE,	CAC_CODE_BD, CAC_CODE_BD, false},
	{"OBLI",	RTTI_BUILDINGTYPE,	CAC_CODE_BD, CAC_CODE_BD, false},
	{"SAM",	RTTI_BUILDINGTYPE,	CAC_CODE_BD, CAC_CODE_BD, false},
	{"NUKE",	RTTI_BUILDINGTYPE,	CAC_CODE_BB, CAC_CODE_BB, true},
	{"NUK2",	RTTI_BUILDINGTYPE,	CAC_CODE_BB, CAC_CODE_BB, true},
};

static char const * CAC_Mix_Name(RTTIType rtti, int type)
{
	switch (rtti) {
		case RTTI_UNITTYPE:			return(UnitTypeClass::As_Reference((UnitType)type).IniName);
		case RTTI_INFANTRYTYPE:		return(InfantryTypeClass::As_Reference((InfantryType)type).IniName);
		case RTTI_AIRCRAFTTYPE:		return(AircraftTypeClass::As_Reference((AircraftType)type).IniName);
		case RTTI_BUILDINGTYPE:		return(BuildingTypeClass::As_Reference((StructType)type).IniName);
		default:							return(NULL);
	}
}
#endif

int CAC_Mix_Type(RTTIType rtti, int type, HousesType owner, bool locked)
{
#ifndef CAC_SUPPLEMENTAL
	rtti; owner; locked;
	return(type);
#else
	if (locked || type < 0) return(type);
	if (owner != HOUSE_GOOD && owner != HOUSE_BAD && (owner < HOUSE_MULTI1 || owner > HOUSE_MULTI6)) {
		return(type);
	}
	HouseClass * house = HouseClass::As_Pointer(owner);
	if (house == NULL) return(type);

	char const * name = CAC_Mix_Name(rtti, type);
	if (name == NULL) return(type);

	for (int i = 0; i < (int)(sizeof(CACMix) / sizeof(CACMix[0])); i++) {
		CACMixEntry const & e = CACMix[i];
		if (e.RTTI != rtti || stricmp(e.Stock, name) != 0) continue;

		bool human = (GameToPlay == GAME_NORMAL) ? (house == PlayerPtr) : house->IsHuman;
		if (e.ComputerOnly && human) return(type);

		bool evil = (house->ActLike == HOUSE_BAD);
		int slot = CAC_Slot_For(rtti, evil ? e.Evil : e.Good, evil);
		if (slot < 0) return(type);
		if (Random_Pick(0, 1) == 0) return(type);

		CAC_Log("mix", "%s's %s placed as %s", house->Class->IniName, name, CAC_Mix_Name(rtti, slot));
		return(slot);
	}
	return(type);
#endif
}


/***********************************************************************************************
 * CAC_INI_Has_Trigger -- Does this scenario object line end in a trigger's name?             *
 *                                                                                             *
 *    The readers parse the trigger last, after the object exists, so CAC_Mix_Type is told    *
 *    from a copy of the line.  Triggers are read before any object section.                  *
 *=============================================================================================*/
bool CAC_INI_Has_Trigger(char const * line)
{
	if (line == NULL) return(false);
	char const * last = strrchr(line, ',');
	if (last == NULL) return(false);
	char name[64];
	int n = 0;
	for (char const * p = last + 1; *p && *p != '\r' && *p != '\n' && n < (int)sizeof(name) - 1; p++) {
		if (*p != ' ' && *p != '\t') name[n++] = *p;
	}
	name[n] = 0;
	return(n > 0 && stricmp(name, "None") != 0 && TriggerClass::As_Pointer(name) != NULL);
}


/***********************************************************************************************
 * CAC_INI_Base_Has_Cell -- Does the scenario's [Base] plan name a building at this cell?     *
 *                                                                                             *
 *    BaseClass matches a node to a building by exact type and coordinate, so a node whose     *
 *    building was swapped reads as unbuilt and the computer tries forever to put the stock   *
 *    one where the CrystAlien one stands.                                                     *
 *=============================================================================================*/
bool CAC_INI_Base_Has_Cell(char * buffer, CELL cell)
{
	if (buffer == NULL) return(false);
	int count = WWGetPrivateProfileInt("Base", "Count", 0, buffer);
	for (int i = 0; i < count; i++) {
		char key[10];
		char value[128];
		sprintf(key, "%03d", i);
		WWGetPrivateProfileString("Base", key, "", value, sizeof(value) - 1, buffer);
		char * comma = strchr(value, ',');
		if (comma == NULL) continue;
		if (Coord_Cell((COORDINATE)atol(comma + 1)) == cell) return(true);
	}
	return(false);
}


/***********************************************************************************************
 * CAC_May_Board -- May this passenger ride in that CrystAlien carrier?                        *
 *                                                                                             *
 *    CrystAlien's own riders are its `carriable` types (CAC_Is_Carriable).  Tiberian Dawn's   *
 *    infantry ride too: the campaign's evacuations put Dr. Mobius and Delphi into the Recon   *
 *    Dropship that stands in for the Chinook, and in the supplemental build a GDI squad may   *
 *    take an Astro carrier.                                                                   *
 *=============================================================================================*/
bool CAC_May_Board(FootClass const * passenger, TechnoClass const * carrier)
{
	if (passenger == NULL || carrier == NULL || !CAC_Is_Own_Carrier(carrier)) return(false);
	if ((ObjectClass const *)passenger == (ObjectClass const *)carrier) return(false);
	TechnoTypeClass const * type = passenger->Techno_Type_Class();
	if (type == NULL) return(false);
	if (passenger->What_Am_I() == RTTI_INFANTRY && !CAC_Is_CrystAlien_Type(type->IniName)) return(true);
	if (passenger->What_Am_I() != RTTI_INFANTRY) return(false);
	return(CAC_Is_Carriable(passenger));
}


/***********************************************************************************************
 * CAC_Try_Embark -- A passenger that has been told to board one of ours does so.              *
 *                                                                                             *
 *    CrystAlien boards by arrival, not by handshake.  The click gives every carriable unit in *
 *    the selection `target = carrier`; each walks to the carrier's tile, and when it stands  *
 *    there doEmbark checks the room and takes it aboard if the carrier is down --            *
 *                                                                                             *
 *        if (this.target.stats.carrier && this.target.team == this.parent.team &&            *
 *            (!this.target.stats.flying || this.target.nav.landerPerc == 100))               *
 *           this.parent.doEmbark(false);                                                      *
 *                                                                                             *
 *    -- and a passenger that finds it full drops the order.  No radio, no tether, no door.   *
 *                                                                                             *
 *    The port drove this through Tiberian Dawn's transport handshake, which talks to one     *
 *    passenger at a time and tethers it before it moves.  The tether was cut at the first    *
 *    cell the passenger walked through, which also ended its radio contact, so only a unit   *
 *    already beside the carrier ever boarded; every other arrived with nobody listening and  *
 *    stood on the carrier for good, and the carrier was left tethered to nothing, which      *
 *    blocked every later boarder and the unload as well.  "May seat only one unit at a time, *
 *    if at all."                                                                              *
 *                                                                                             *
 *    So a passenger holds its carrier in ArchiveTarget for the whole errand, and this is     *
 *    asked from Mission_Enter and on every cell the passenger reaches.                       *
 *                                                                                             *
 * OUTPUT:  CAC_EMBARK_NOT_MINE -- not boarding one of ours; the stock code carries on.         *
 *          CAC_EMBARK_GOING    -- on its way, or waiting under a carrier still in the air.    *
 *          CAC_EMBARK_DONE     -- aboard, or the order dropped because the carrier is full.   *
 *=============================================================================================*/
int CAC_Try_Embark(FootClass * p)
{
	if (p == NULL || p->IsInLimbo || p->Mission != MISSION_ENTER) return(CAC_EMBARK_NOT_MINE);

	TechnoClass * c = As_Techno(p->ArchiveTarget);
	if (c == NULL || !CAC_Is_Own_Carrier(c)) c = As_Techno(p->NavCom);
	if (c == NULL || !CAC_May_Board(p, c)) return(CAC_EMBARK_NOT_MINE);

	p->ArchiveTarget = c->As_Target();

	TechnoTypeClass const * ctype = c->Techno_Type_Class();
	if (!c->IsActive || c->IsInLimbo || c->House != p->House || ctype == NULL ||
		 c->How_Many() >= ctype->Max_Passengers()) {
		p->ArchiveTarget = TARGET_NONE;
		p->Assign_Destination(TARGET_NONE);
		p->Enter_Idle_Mode();
		return(CAC_EMBARK_DONE);
	}

	bool there = (Coord_Cell(p->Center_Coord()) == Coord_Cell(c->Center_Coord())) ||
					 (::Distance(p->Center_Coord(), c->Center_Coord()) < 0x0180);
	if (!there) {
		if (p->NavCom != c->As_Target()) p->Assign_Destination(c->As_Target());
		return(CAC_EMBARK_GOING);
	}

	/*	Arrived.  A carrier still in the air is waited under, as CrystAlien does. */
	if (c->What_Am_I() == RTTI_AIRCRAFT && ((AircraftClass *)c)->Altitude != 0) {
		return(CAC_EMBARK_GOING);
	}

	p->Transmit_Message(RADIO_OVER_OUT);
	p->ArchiveTarget = TARGET_NONE;
	p->Assign_Destination(TARGET_NONE);
	p->Mark(MARK_UP);
	p->Limbo();
	c->Attach(p);
	CAC_Log("board", "%s boards %s (%d of %d)", p->Techno_Type_Class()->IniName, ctype->IniName,
			  c->How_Many(), ctype->Max_Passengers());
	return(CAC_EMBARK_DONE);
}


/***********************************************************************************************
 * CAC_Has_Boarders -- Is anyone still on the way to board this carrier?                       *
 *                                                                                             *
 *    The carrier stays down -- or comes down -- while it has passengers coming.  It asked    *
 *    whether it was in radio contact, which only ever covered one of them.                    *
 *=============================================================================================*/
bool CAC_Has_Boarders(TechnoClass const * carrier)
{
	if (carrier == NULL || !CAC_Is_Own_Carrier(carrier)) return(false);
	TARGET me = carrier->As_Target();
	for (int i = 0; i < Infantry.Count(); i++) {
		InfantryClass const * p = Infantry.Ptr(i);
		if (p == NULL || !p->IsActive || p->IsInLimbo || p->House != carrier->House) continue;
		if (p->Mission != MISSION_ENTER) continue;
		if (p->ArchiveTarget == me || p->NavCom == me) return(true);
	}
	return(false);
}
