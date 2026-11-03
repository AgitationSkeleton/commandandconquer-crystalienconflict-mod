//
// Copyright 2020 Electronic Arts Inc.
//
// TiberianDawn.DLL and RedAlert.dll and corresponding source code is free 
// software: you can redistribute it and/or modify it under the terms of 
// the GNU General Public License as published by the Free Software Foundation, 
// either version 3 of the License, or (at your option) any later version.

// TiberianDawn.DLL and RedAlert.dll and corresponding source code is distributed 
// in the hope that it will be useful, but with permitted additional restrictions 
// under Section 7 of the GPL. See the GNU General Public License in LICENSE.TXT 
// distributed with this program. You should have received a copy of the 
// GNU General Public License along with permitted additional restrictions 
// with this program. If not, see https://github.com/electronicarts/CnC_Remastered_Collection

/* $Header:   F:\projects\c&c\vcs\code\aircraft.cpv   2.12   19 Jun 1995 09:27:22   JOE_BOSTIC  $ */
/***********************************************************************************************
 ***             C O N F I D E N T I A L  ---  W E S T W O O D   S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : AIRCRAFT.CPP                                                 *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : July 22, 1994                                                *
 *                                                                                             *
 *                  Last Update : August 13, 1995 [JLB]                                        *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   AircraftClass::AI -- Processes the normal non-graphic AI for the aircraft.                *
 *   AircraftClass::Active_Click_With -- Handles clicking over specified cell.                 *
 *   AircraftClass::Active_Click_With -- Handles clicking over specified object.               *
 *   AircraftClass::AircraftClass -- The constructor for aircraft objects.                     *
 *   AircraftClass::As_Target -- Returns aircraft as a target number.                          *
 *   AircraftClass::Can_Enter_Cell -- Determines if the aircraft can land at this location.    *
 *   AircraftClass::Cell_Seems_Ok -- Checks to see if a cell is good to enter.                 *
 *   AircraftClass::Desired_Load_Dir -- Determines where passengers should line up.            *
 *   AircraftClass::Draw_It -- Renders an aircraft object at the location specified.           *
 *   AircraftClass::Enter_Idle_Mode -- Gives the aircraft an appropriate mission.              *
 *   AircraftClass::Exit_Object -- Unloads passenger from aircraft.                            *
 *   AircraftClass::Fire_At -- Handles firing a projectile from an aircraft.                   *
 *   AircraftClass::Fire_Coord -- Calculates the point of origin for a bullet.                 *
 *   AircraftClass::Fire_Direction -- Determines the direction of fire.                        *
 *   AircraftClass::Good_Fire_Location -- Searches for and finds a good spot to fire from.     *
 *   AircraftClass::Good_LZ -- Locates a good spot ot land.                                    *
 *   AircraftClass::In_Which_Layer -- Determine which render layer the aircraft lies.          *
 *   AircraftClass::Init -- Initialize the aircraft system to an empty state.                  *
 *   AircraftClass::Is_LZ_Clear -- Determines if landing zone is free for landing.             *
 *   AircraftClass::Mark -- Flags cells under the aircraft so that they will be redrawn.       *
 *   AircraftClass::Mission_Attack -- Handles the attack mission for aircraft.                 *
 *   AircraftClass::Mission_Enter -- Control aircraft to fly to the helipad or repair center.  *
 *   AircraftClass::Mission_Guard -- Handles aircraft in guard mode.                           *
 *   AircraftClass::Mission_Guard_Area -- Handles the aircraft guard area logic.               *
 *   AircraftClass::Mission_Hunt -- Maintains hunt AI for the aircraft.                        *
 *   AircraftClass::Mission_Move -- Handles movement mission.                                  *
 *   AircraftClass::Mission_Retreat -- Handles the aircraft logic for leaving the battlefield. *
 *   AircraftClass::Mission_Unload -- Handles unloading cargo.                                 *
 *   AircraftClass::New_LZ -- Find a good landing zone.                                        *
 *   AircraftClass::Overlap_List -- Returns with list of cells the aircraft overlaps.          *
 *   AircraftClass::Pip_Count -- Returns the number of "objects" in aircraft.                  *
 *   AircraftClass::Player_Assign_Mission -- Handles player input to assign a mission.         *
 *   AircraftClass::Pose_Dir -- Fetches the natural landing facing.                            *
 *   AircraftClass::Process_Fly_To -- Handles state machine for flying to destination.         *
 *   AircraftClass::Process_Landing -- Landing process state machine handler.                  *
 *   AircraftClass::Process_Take_Off -- State machine support for taking off.                  *
 *   AircraftClass::Read_INI -- Reads aircraft object data from an INI file.                   *
 *   AircraftClass::Rearm_Delay -- Returns the delay between shots for this aircraft.          *
 *   AircraftClass::Receive_Message -- Handles receipt of radio messages.                      *
 *   AircraftClass::Response_Attack -- Gives audio response to attack order.                   *
 *   AircraftClass::Response_Move -- Gives audio response to move request.                     *
 *   AircraftClass::Response_Select -- Gives audio response when selected.                     *
 *   AircraftClass::Scatter -- Causes the aircraft to move away a bit.                         *
 *   AircraftClass::Set_Speed -- Sets the speed for the aircraft.                              *
 *   AircraftClass::Sort_Y -- Figures the sorting coordinate.                                  *
 *   AircraftClass::Take_Damage -- Applies damage to the aircraft.                             *
 *   AircraftClass::Threat_Range -- Returns with a range to scan for targets.                  *
 *   AircraftClass::Unlimbo -- Removes an aircraft from the limbo state.                       *
 *   AircraftClass::What_Action -- Determines what action to perform.                          *
 *   AircraftClass::What_Action -- Determines what action to perform.                          *
 *   AircraftClass::Write_INI -- Writes the current aircraft objects to an INI file.           *
 *   AircraftClass::operator delete -- Deletes the aircraft object.                            *
 *   AircraftClass::operator new -- Allocates a new aircraft object from the pool              *
 *   AircraftClass::~AircraftClass -- Destructor for aircraft object.                          *
 *   AircraftClass::Validate -- validates aircraft pointer												  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"function.h"
#include	"CACSLOTS.INC"
#include	"CACLOGIC.H"


/*
** This contains the value of the Virtual Function Table Pointer
*/
void * AircraftClass::VTable;


/***********************************************************************************************
 * AircraftClass::Validate -- validates aircraft pointer													  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *		none.																												  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *		1 = ok, 0 = error																								  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *		none.																												  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/09/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
#ifdef CHEAT_KEYS
int AircraftClass::Validate(void) const
{
	int num;

	num = Aircraft.ID(this);
	if (num < 0 || num >= AIRCRAFT_MAX) {
		Validate_Error("AIRCRAFT");
		return (0);
	}
	else
		return (1);
}
#else
#define	Validate()
#endif


/***********************************************************************************************
 * AircraftClass::As_Target -- Returns aircraft as a target number.                            *
 *                                                                                             *
 *    This routine will convert the aircraft into a target number. This target number can      *
 *    then be assigned to a targeting or navigation computer.                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the aircraft as a target number.                                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
TARGET AircraftClass::As_Target(void) const
{
	Validate();
	return(Build_Target(KIND_AIRCRAFT, Aircraft.ID(this)));
}


/***********************************************************************************************
 * AircraftClass::operator new -- Allocates a new aircraft object from the pool                *
 *                                                                                             *
 *    This routine will allocate an aircraft object from the free aircraft object pool. If     *
 *    there are no free object available, then this routine will fail (return NULL).           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the allocate aircraft object or NULL if none were        *
 *          available.                                                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void * AircraftClass::operator new(size_t)
{
	void * ptr = Aircraft.Allocate();
	if (ptr) {
		((AircraftClass *)ptr)->Set_Active();
	}
	return(ptr);
}


/***********************************************************************************************
 * AircraftClass::operator delete -- Deletes the aircraft object.                              *
 *                                                                                             *
 *    This routine will return the aircraft object back to the free aircraft object pool.      *
 *                                                                                             *
 * INPUT:   ptr   -- Pointer to the aircraft object to delete.                                 *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::operator delete(void *ptr)
{
	if (ptr) {
		((AircraftClass *)ptr)->IsActive = false;
	}
	Aircraft.Free((AircraftClass *)ptr);

	//Map.Validate();
}


/***********************************************************************************************
 * AircraftClass::AircraftClass -- The constructor for aircraft objects.                       *
 *                                                                                             *
 *    This routine is the constructor for aircraft objects. An aircraft object can be          *
 *    created and possibly placed into the game system by this routine.                        *
 *                                                                                             *
 * INPUT:   classid  -- The type of aircraft to create.                                        *
 *                                                                                             *
 *          house    -- The owner of this aircraft.                                            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
AircraftClass::AircraftClass(AircraftType classid, HousesType house) :
	Class(&AircraftTypeClass::As_Reference(classid)),
	FootClass(house)
{
	/*
	**	For two shooters, clear out the second shot flag -- it will be set the first time
	**	the object fires. For non two shooters, set the flag since it will never be cleared
	**	and the second shot flag tells the system that normal rearm times apply -- this is
	**	what is desired for non two shooters.
	*/
	if (Class->IsTwoShooter) {
		IsSecondShot = false;
	} else {
		IsSecondShot = true;
	}
	Ammo = Class->MaxAmmo;
	AttacksRemaining = 3;
	Altitude = FLIGHT_LEVEL;
	CACStill = 0;
	CACLastNav = TARGET_NONE;
	IsLanding = false;
	IsTakingOff = false;
	IsHovering = false;
	IsHoming = false;
	Strength = Class->MaxStrength;
	NavCom = TARGET_NONE;
	SecondaryFacing = PrimaryFacing;
	Jitter = 0;
	ReinforcementStart = -1;

	/*
	** Keep count of the number of units created. Dont track cargo planes as they are created
	** automatically, not bought.
	*/
	if (classid != AIRCRAFT_CARGO && GameToPlay == GAME_INTERNET){
		House->AircraftTotals->Increment_Unit_Total((int)classid);
	}

#ifdef USE_RA_AI
	//
	// Added for RA AI in TD. ST - 7/26/2019 9:12AM
	//
	House->Tracking_Add(this);
#endif
}


/***********************************************************************************************
 * AircraftClass::Unlimbo -- Removes an aircraft from the limbo state.                         *
 *                                                                                             *
 *    This routine is used to transition the aircraft from the limbo to the non limbo state.   *
 *    It occurs when the aircraft is placed on the map for whatever reason. When it is         *
 *    unlimboed, only then will normal game processing recognize it.                           *
 *                                                                                             *
 * INPUT:   coord -- The coordinate that the aircraft should appear at.                        *
 *                                                                                             *
 *          dir   -- The direction it should start facing.                                     *
 *                                                                                             *
 *				strength (optional) -- sets initial strength													  *
 *                                                                                             *
 *				mission (optional) -- sets initial mission													  *
 *                                                                                             *
 * OUTPUT:  bool; Was the aircraft unlimboed successfully?                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool AircraftClass::Unlimbo(COORDINATE coord, DirType dir)
{
	Validate();
	if (FootClass::Unlimbo(coord, dir)) {

		/*
		**	Ensure that the owning house knows about the
		**	new object.
		*/
		House->AScan |= (1L << Class->Type);
		House->ActiveAScan |= (1L << Class->Type);

		/*
		**	Forces the body of the helicopter to face the correct direction.
		*/
		SecondaryFacing = dir;

		/*
		**	Start rotor animation.
		*/
		Set_Rate(1);
		Set_Stage(0);

		/*
		**	Presume it starts in flight?
		**
		**	Not one of ours: a CrystAlien craft with no path does not move at all
		**	(UnitNav.handle returns while `path.length <= 1`).  Launched at full
		**	throttle with no destination, it flew along whatever it was unlimboed
		**	facing -- east, for a Technology Centre's survivor -- until the map's
		**	edge caught it.  Whatever gives it somewhere to go sets its speed.
		*/
		if (Altitude == FLIGHT_LEVEL && !CAC_Is_CrystAlien_Type(Class->IniName)) {
			Set_Speed(0xFF);
		} else {
			Set_Speed(0);
		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * AircraftClass::Draw_It -- Renders an aircraft object at the location specified.             *
 *                                                                                             *
 *    This routine is used to display the aircraft object at the coordinates specified.        *
 *    The tactical map display uses this routine for all aircraft rendering.                   *
 *                                                                                             *
 * INPUT:   x,y      -- The coordinates to render the aircraft at.                             *
 *                                                                                             *
 *          window   -- The window that the coordinates are based upon.                        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::Draw_It(int x, int y, WindowNumberType window)
{
	Validate();
	void const * shapefile;		// Working shape file pointer.
	int shapenum = 0;
	int facing = Facing_To_32(SecondaryFacing);

	/*
	**	Don't draw Cargo aircraft that are delayed.
	*/
	if (Special.ModernBalance) {
		if (*this == AIRCRAFT_CARGO && !Map.In_Radar(Coord_Cell(Coord)) && ReinforcementStart > Frame) {
			return;
		}
	}

	/*
	**	Verify the legality of the unit class.
	*/
	shapefile = Class->Get_Image_Data();
	if (!shapefile) return;
	shapenum = UnitClass::BodyShape[facing];

	/*
	**	The orca attack helicopter uses a special shape set when it is travelling
	**	forward above a certain speed.
	*/
	if (*this == AIRCRAFT_ORCA && Get_Speed() >= MPH_MEDIUM_FAST) {
		shapenum += 32;
	}

	/*
	**	If there is a door on this aircraft (Chinook), then adjust the
	**	shape number to match the door open state.
	*/
	if (!Is_Door_Closed()) {
		shapenum = 32 + Door_Stage();
	}

	/*
	**	Helicopters that are flying have a "bobbing" effect.
	*/
	int jitter = 0;
	if (Altitude == FLIGHT_LEVEL && !Class->IsFixedWing) {
		Jitter++;

		static int _jitter[] = {0,0,0,0,1,1,1,0,0,0,0,0,-1,-1,-1,0};
		jitter = _jitter[Jitter % 16];
	}

	/*
	**	A transformer is drawn from its landing or ground bank as it settles;
	**	CAC_Transformer_Bank mirrors UnitNav's choice, and the tileset lays those
	**	banks out above the flying shapes.
	**
	**	Chosen here rather than inside the branch below, because there are two
	**	draw paths and only the HD one had it -- so a Switch Fighter sitting on
	**	the ground in classic mode was still drawn in its flying shape.  The bank
	**	is a property of the craft, not of which renderer is asking.
	*/
	if (CAC_Is_Transformer(*this)) {
		int bank = CAC_Transformer_Bank(Altitude, FLIGHT_LEVEL);
		if (bank == 1) shapenum += CAC_BANK_LANDING;
		else if (bank == 2) shapenum += CAC_BANK_GROUND;
	}

	// Virtual window needs to draw the body first so it's considered the primary object and the shadow is a sub-object
	if (window == WINDOW_VIRTUAL) {
		/*
		**	Draw the root body of the unit.
		*/
		Techno_Draw_Object(shapefile, shapenum, x, (y - Altitude) + jitter, window);

		/*
		**	Special manual shadow draw code.
		*/
		if (Visual_Character() <= VISUAL_DARKEN) {
			CC_Draw_Shape(this, shapefile, shapenum, x + 1, y + 2, window, SHAPE_PREDATOR | SHAPE_CENTER | SHAPE_WIN_REL | SHAPE_FADING, Map.FadingShade, NULL);
		}
	} else {
		/*
		**	Special manual shadow draw code.
		*/
		if (Visual_Character() <= VISUAL_DARKEN) {
			CC_Draw_Shape(this, shapefile, shapenum, x+1, y+2, window, SHAPE_PREDATOR|SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_FADING, Map.FadingShade, NULL);
		}

		/*
		**	Draw the root body of the unit.
		*/
		Techno_Draw_Object(shapefile, shapenum, x, (y-Altitude)+jitter, window);
	}

	/*
	**	Draw rotor effects. The rotor art can be either generic or custom. Custom rotor
	**	art has a different rotor set for each facing. Rotor shapes occur after the first
	**	32 shapes of the helicopter body.
	*/
	if (Class->IsRotorEquipped) {
		ShapeFlags_Type flags = SHAPE_CENTER|SHAPE_WIN_REL;

		/*
		**	The rotor shape number depends on whether the helicopter is idling
		**	or not. A landed helicopter uses slow moving "idling" blades.
		*/
		if (Altitude == 0) {
			shapenum = (Fetch_Stage()%8)+4;
			flags = flags | SHAPE_GHOST;
		} else {
			shapenum = Fetch_Stage()%4;
			flags = flags | SHAPE_FADING|SHAPE_PREDATOR;
		}

		if (*this == AIRCRAFT_TRANSPORT) {
			int _stretch[FACING_COUNT] = {8, 9, 10, 9, 8, 9, 10, 9};

			/*
			**	Dual rotors offset along flight axis.
			*/
			short xx = x;
			short yy = y-Altitude;
			FacingType face = Dir_Facing(SecondaryFacing);
			Move_Point(xx, yy, SecondaryFacing.Current(), _stretch[face]);
			//CC_Draw_Shape(Class->RRotorData, shapenum, xx, yy-2, window, flags, NULL, Map.UnitShadow);		// 6/18/2019 - ST
			CC_Draw_Shape(this, "RROTOR", Class->RRotorData, shapenum, xx, yy - 2, window, flags, NULL, Map.UnitShadow);

			Move_Point(xx, yy, SecondaryFacing.Current()+DIR_S, _stretch[face]*2);
			//CC_Draw_Shape(this, Class->LRotorData, shapenum, xx, yy-2, window, flags, NULL, Map.UnitShadow);			// 6/18/2019 - ST
			CC_Draw_Shape(this, "LROTOR", Class->LRotorData, shapenum, xx, yy - 2, window, flags, NULL, Map.UnitShadow);

		} else {

			/*
			**	Single rotor centered about shape.
			*/
			//CC_Draw_Shape(this, Class->RRotorData, shapenum, x, (y-Altitude)-2, window, flags, NULL, Map.UnitShadow);	// 6/18/2019 - ST
			CC_Draw_Shape(this, "RROTOR", Class->RRotorData, shapenum, x, (y - Altitude) - 2, window, flags, NULL, Map.UnitShadow);
		}
	}

	FootClass::Draw_It(x, y-Altitude, window);
}


/***********************************************************************************************
 * AircraftClass::Read_INI -- Reads aircraft object data from an INI file.                     *
 *                                                                                             *
 *    This routine is used to read the aircraft object data from the INI file buffer           *
 *    specified. This is used by the scenario loader code to interpret the INI file and        *
 *    create the specified objects therein.                                                    *
 *                                                                                             *
 * INPUT:   buffer   -- Pointer to the INI buffer.                                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::Read_INI(char *buffer)
{
	AircraftClass	*air;			// Working unit pointer.
	char				*tbuffer;		// Accumulation buffer of unit IDs.
	HousesType		inhouse;			// Unit house.
	AircraftType	classid;			// Unit class.
	int				len;				// Length of data in buffer.
	char				buf[128];

	len = strlen(buffer) + 2;
	tbuffer = buffer + len;

	WWGetPrivateProfileString(INI_Name(), NULL, NULL, tbuffer, ShapeBufferSize-len, buffer);
	while (*tbuffer != '\0') {

		WWGetPrivateProfileString(INI_Name(), tbuffer, NULL, buf, sizeof(buf)-1, buffer);
		inhouse = HouseTypeClass::From_Name(strtok(buf, ","));
		if (inhouse != HOUSE_NONE) {
			classid = AircraftTypeClass::From_Name(strtok(NULL, ","));

			/*
			**	The supplemental build mixes the rosters (aircraft carry no
			**	trigger).  See CAC_Mix_Type.
			*/
			if (classid != AIRCRAFT_NONE) {
				classid = (AircraftType)CAC_Mix_Type(RTTI_AIRCRAFTTYPE, classid, inhouse, false);
			}

			if (classid != AIRCRAFT_NONE) {

				if (HouseClass::As_Pointer(inhouse) != NULL) {
					air = new AircraftClass(classid, inhouse);
					if (air) {
						COORDINATE	coord;
						int	strength;
						DirType dir;

						/*
						**	Read the raw data.
						*/
						strength = atoi(strtok(NULL, ","));
						coord = Cell_Coord((CELL)atoi(strtok(NULL, ",")));
						dir = (DirType)atoi(strtok(NULL, ","));

						if (!Map.In_Radar(Coord_Cell(coord))) {
							delete air;
						} else {

							air->Strength = Fixed_To_Cardinal(air->Class->MaxStrength, strength);
							if (air->Unlimbo(coord, dir)) {
								air->Assign_Mission(AircraftClass::Mission_From_Name(strtok(NULL, ",\n\r")));
							} else {
								delete air;
							}
						}
					}
				}
			}
		}
		tbuffer += strlen(tbuffer)+1;
	}
}


/***********************************************************************************************
 * AircraftClass::Write_INI -- Writes the current aircraft objects to an INI file.             *
 *                                                                                             *
 *    This routine is used to output the current list of aircraft objects to the INI file      *
 *    buffer specified. It is typically used by the scenario editor.                           *
 *                                                                                             *
 * INPUT:   buffer   -- Pointer to the buffer that holds the INI file data.                    *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::Write_INI(char *buffer)
{
	int	index;
	char	uname[10];
	char	buf[128];
	char	*tbuffer;		// Accumulation buffer of unit IDs.

	/*
	**	First, clear out all existing unit data from the ini file.
	*/
	tbuffer = buffer + strlen(buffer) + 2;
	WWGetPrivateProfileString(INI_Name(), NULL, NULL, tbuffer, ShapeBufferSize-strlen(buffer), buffer);
	while (*tbuffer != '\0') {
		WWWritePrivateProfileString(INI_Name(), tbuffer, NULL, buffer);
		tbuffer += strlen(tbuffer)+1;
	}

	/*
	**	Write the unit data out.
	*/
	for (index = 0; index < Aircraft.Count(); index++) {
		AircraftClass * unit;

		unit = Aircraft.Ptr(index);
		if (!unit->IsInLimbo) {

			sprintf(uname, "%03d", index);
			sprintf(buf, "%s,%s,%d,%u,%d,%s",
				unit->House->Class->IniName,
				unit->Class->IniName,
				unit->Health_Ratio(),
				Coord_Cell(unit->Coord),
				unit->PrimaryFacing.Current(),
				MissionClass::Mission_Name(unit->Mission)
				);
			WWWritePrivateProfileString(INI_Name(), uname, buf, buffer);
		}
	}
}


/***********************************************************************************************
 * AircraftClass::Mission_Hunt -- Maintains hunt AI for the aircraft.                          *
 *                                                                                             *
 *    Hunt AI consists of finding a target and attacking it. If there is no target assigned    *
 *    and this unit doesn't automatically hunt for more targets, then it will change           *
 *    mission to a more passive (land and await further orders) type.                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of ticks before calling this routine again.                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int AircraftClass::Mission_Hunt(void)
{
	Validate();
	if (Class->IsFixedWing) {
		if (Class->Primary == WEAPON_NONE && Class->Secondary == WEAPON_NONE) {
			Assign_Mission(MISSION_RETREAT);
			return(1);
		}
		enum {
			LOOK_FOR_TARGET,
			FLY_TO_TARGET,
			DROP_BOMBS
		};
		switch (Status) {

			/*
			**	Acquiring target stage.
			*/
			case LOOK_FOR_TARGET:
				if (Target_Legal(TarCom)) {
					Status = FLY_TO_TARGET;
					return(1);
				} else {
					Assign_Target(Greatest_Threat(THREAT_NORMAL));

					/*
					**	If there is no target, then this aircraft should just do its normal thing.
					*/
					if (!Target_Legal(TarCom)) {
						Enter_Idle_Mode();
					}
				}
				break;

			/*
			**	Homing in on target stage.
			*/
			case FLY_TO_TARGET:
				if (Target_Legal(TarCom)) {
					IsHoming = true;
					if (!PrimaryFacing.Is_Rotating()) {
						PrimaryFacing.Set_Desired(Direction(TarCom));
					}
					if (Distance(TarCom) < 0x0380) {
						IsHoming = false;
						Status = DROP_BOMBS;
						return(1);
					}
				} else {
					Status = LOOK_FOR_TARGET;
				}
				break;

			/*
			**	Dropping a stream of bombs phase.
			*/
			case DROP_BOMBS:
				if (!Ammo) {
					AttacksRemaining--;
					if (!AttacksRemaining) {
						Assign_Mission(MISSION_RETREAT);
						Commence();
					} else {
						Ammo = Class->MaxAmmo;
						Status = LOOK_FOR_TARGET;
					}
				}
				if (!Target_Legal(TarCom)) {
					Status = LOOK_FOR_TARGET;
				} else {
					Fire_At(TarCom, 0);
					Map[::As_Cell(TarCom)].Incoming(Coord, true);
					return(5);
				}
				break;
		}
	} else {
		if (!Ammo) {
			Enter_Idle_Mode();
		} else {
			Assign_Mission(MISSION_ATTACK);
			return(1);
//			return(FootClass::Mission_Hunt());
		}
	}
	return(TICKS_PER_SECOND);
}


/***********************************************************************************************
 * AircraftClass::AI -- Processes the normal non-graphic AI for the aircraft.                  *
 *                                                                                             *
 *    This handles the non-graphic AI processing for the aircraft. This usually entails        *
 *    maintenance and other AI functions.                                                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::AI(void)
{
	Validate();

	/*
	**	"if (this.stats.boomerang && !this.stats.home.active) this.destroy()"
	**	-- a tethered fighter does not outlive its station.  See CAC_Homes_To.
	*/
	CAC_Check_Home(this);

	/*
	**	A carrier with somewhere to be shuts its door.
	**
	**	Tiberian Dawn closes a transport's door in one place only:
	**
	**	    case RADIO_IM_IN:
	**	        if (How_Many() == Class->Max_Passengers()) Close_Door(5, 4);
	**
	**	-- when it is completely full.  That is fine for a Chinook, which exists to
	**	be filled and sent, and wrong for a Hyper Carrier carrying two of five.
	**	The door then stays open for the rest of its life, and the door is not
	**	cosmetic: the altitude driver reads
	**
	**	    if (Is_Door_Closed() && (IsLanding || IsTakingOff))
	**
	**	so a craft with an open door can neither rise nor settle.  It accepts the
	**	order, sets IsTakingOff, and stays exactly where it is -- "transports don't
	**	move at all".
	**
	**	CrystAlien has no door and no such rule.  A unit refuses to board a carrier
	**	that is already full -- `if(target.stats.contents.length >= capacity)` in
	**	doEmbark -- and the carrier goes when it is told, however many it has
	**	aboard.  So the door closes here the moment the craft is given somewhere to
	**	go, which is the same moment the source would have it leave.
	*/
	if (Class != NULL && CAC_Is_CrystAlien_Type(Class->IniName) &&
		Class->IsTransporter && !Is_Door_Closed() && Target_Legal(NavCom)) {
		Close_Door(5, 4);
	}

	/*
	**	A craft on its last quarter trails smoke, the same as a structure does.
	**	See CAC_Unit_Smoke.
	*/
	CAC_Unit_Smoke(this);
	if (!IsActive) return;

	/*
	**	And it shoots what it passes, on the way to wherever it is going.
	**	
	**	engageEnemy runs from the unit's think every frame and is independent of
	**	movement, so a CrystAlien craft fires as soon as something is inside its
	**	range and keeps flying.  See CAC_Strafe -- it touches neither the mission
	**	nor the destination, so this changes what the craft shoots and nothing about
	**	where it goes.
	*/
	CAC_Strafe(this);
	if (!IsActive) return;

	/*
	**	And everything that is not a boomerang takes the opportunistic scan the
	**	rest of the roster runs.  See CAC_Opportunity_Fire.
	*/
	CAC_Opportunity_Fire(this);
	if (!IsActive) return;
	if (!IsActive) return;

	/*
	**	UnitNav gives a lander a look at the ground the moment it settles:
	**
	**		if (this.landerPerc == 100) { this.bank = "ground";
	**			if (this.parent.friend) this.checkCollect(); }
	**
	**	so a craft that has set down can take a pickup like anything else.
	**	Nothing in Tiberian Dawn offers that, because its aircraft have no
	**	reason to be standing on a crate.
	*/
	/*
	**	The transformer, and only it.  The crate pickup hangs off the bank change
	**	that none of the others has:
	**
	**	    if (this.stats.transformer) {
	**	        if (this.landerPerc > 0)   this.bank = "landing";
	**	        if (this.landerPerc == 100) { this.bank = "ground";
	**	            if (this.parent.friend) this.checkCollect(); }
	**	    }
	**
	**	A Hyper Carrier parked on a crate leaves it there.
	*/
	if (Altitude == 0 && Class != NULL && CAC_Is_CrystAlien_Type(Class->IniName) &&
		 CAC_Is_Transformer(*this)) {

		/*
		**	Both halves, because Goodie_Check is two routines wearing one name:
		**
		**	    if (check_steel && steel)        ... the Alien capsule
		**	    else if (!check_steel && !steel) ... the crystal box and the present
		**
		**	A walker reaches both by different doors -- FootClass asks with false as
		**	it steps into a cell and with true from Per_Cell_Process -- and a craft
		**	that settles onto a crate goes through neither.  Asking only with true,
		**	as this did, let a Switch Fighter open a capsule and sit on a crystal box
		**	for the rest of the round.
		**
		**	The first call takes the crate if it matches, so the second finds
		**	OVERLAY_NONE and does nothing.  Both are guarded on the craft still being
		**	alive, since a capsule can put a Claw Tank on the ground beside it.
		*/
		CELL here = Coord_Cell(Coord);

		Map[here].Goodie_Check(this, false);
		if (IsActive) {
			Map[here].Goodie_Check(this, true);
		}
		if (!IsActive) return;
	}

	/*
	**	Volatile: reported as drifting away instead of holding station.
	**
	**	A hovering CrystAlien flier has no business anywhere but MISSION_GUARD
	**	unless somebody put it somewhere else, and reading the code has ruled out
	**	the obvious suspects -- Mission_Guard returns for ours before it reaches
	**	the two lines that park a helicopter, Enter_Idle_Mode hands a hoverer
	**	GUARD with no destination, and CAC_Return_Home refuses anything that is
	**	not a boomerang.  So whatever is moving them is handing out a mission,
	**	and this says which one and what it is aimed at.
	*/
	if (CAC_Logging() && CAC_Aircraft_Hovers(*this) && !IsInLimbo &&
		Mission != MISSION_GUARD && (Frame % TICKS_PER_SECOND) == 0) {
		CAC_Log_Volatile("drift", "%s (%s) on mission %d at %d,%d -- nav %s, target %s at %d,%d",
							  Class->IniName,
							  (House != NULL && House->IsHuman) ? "player" : "computer",
							  (int)Mission,
							  Cell_X(Coord_Cell(Center_Coord())),
							  Cell_Y(Coord_Cell(Center_Coord())),
							  Target_Legal(NavCom) ? "set" : "none",
							  Target_Legal(TarCom) ? "set" : "none",
							  Target_Legal(TarCom) ? Cell_X(::As_Cell(TarCom)) : -1,
							  Target_Legal(TarCom) ? Cell_Y(::As_Cell(TarCom)) : -1);
	}

	/*
	**	And the answer that line was asking for: the mission is MISSION_ATTACK.
	**
	**	Every drift report in two logs is one of these -- 785 in one and 526 in
	**	the other, all of them UC, the JetPack Explorer and the Viper Attack --
	**	and the telling half read "nav none, target set".  So it is not that
	**	something put them on the wrong mission.  It is that Mission_Attack does
	**	its job: it flies the craft to a standing-off point three quarters of its
	**	range from the target and the engine clears NavCom on arrival, and then
	**	nothing puts the brakes on.  Speed is still whatever it took to get there,
	**	so the craft sails straight past the thing it came to shoot.
	**
	**	Which is also why it survived the earlier rounds of this.  Mission_Guard
	**	and Enter_Idle_Mode both stop a hoverer, and both are about a craft with
	**	nothing to do; this one has something to do and is doing it.
	**
	**	A hoverer with no destination has arrived, whatever the mission.  Landing
	**	and taking off move it vertically and are left alone.
	*/
	if (CAC_Aircraft_Hovers(*this) && !IsInLimbo && !IsLanding && !IsTakingOff &&
		 !Target_Legal(NavCom) && Get_Speed() != MPH_IMMOBILE) {
		Set_Speed(0);
	}

	/*
	**	And nothing of ours leaves the map, whatever told it to.  See
	**	CAC_Contain_Flier -- the backstop, and the line that will name whichever
	**	routine keeps doing it.
	*/
	CAC_Contain_Flier(this);
	if (!IsActive) return;

	/*
	**	And a boomerang turns round the moment it is free.  See CAC_Return_Home.
	*/
	CAC_Return_Home(this);

	/*
	**	And tops up while it is there, a round at a time.  See CAC_Rearm_At_Home.
	*/
	CAC_Rearm_At_Home(this);
	/*
	**	A Mission change can always occur if the aircraft is landed or flying.
	*/
	/*
	**	Ours commences whatever its altitude is doing.
	**
	**	landerPerc and the order queue are unrelated in the source -- the ramp is
	**	driven from UnitNav.handle every frame and nothing about it defers a
	**	path.  Tiberian Dawn couples them, and with the ramp now running both
	**	ways (see the lander block above) that coupling would leave a craft that
	**	is climbing or settling unable to start the very order that is making it
	**	climb.
	*/
	if (CAC_Is_Lander(*this) || (!IsLanding && !IsTakingOff)) {
		Commence();
	}

	FootClass::AI();

	/*
	**	CrystAlien's transformers set down wherever they happen to be rather than
	**	only at a pad.  UnitNav counts idle ticks and, once past twenty, drives
	**	`landerPerc` toward the ground; any movement reverses it:
	**
	**	    if (still > 20 && (!boomerang || atHome) && tile not blocked)
	**	        landerPerc += speed;  else  landerPerc -= speed;
	**
	**	The equivalent here is to steer Tiberian Dawn's own landing flags, which
	**	already handle the render-layer change and the altitude ramp.
	*/
	/*
	**	Except while a mission is flying the altitude itself.
	**
	**	Mission_Unload runs SEARCH_FOR_LZ -> FLY_TO_LZ -> LAND_ON_LZ ->
	**	UNLOAD_PASSENGERS, and LAND_ON_LZ drives the descent through
	**	Process_Landing.  That is already what doDeploy asks for -- `if
	**	(this.stats.flying && this.nav.landerPerc != 100) return;`, a carrier
	**	unloads only once it is fully down -- so the ramp has nothing to add and
	**	would fight it: still is nought on arrival, so the ramp would call for a
	**	climb, LAND_ON_LZ would see IsTakingOff and start over.  MISSION_ENTER
	**	docks the same way.
	*/
	if (CAC_Is_Lander(*this) && !IsInLimbo &&
		 Mission != MISSION_UNLOAD && Mission != MISSION_ENTER) {

		/*
		**	The whole of UnitNav's lander, which is a ramp and not a decision:
		**
		**	    if (this.parent.stats.lander && (this.still || this.landerPerc)) {
		**	        if (this.still > 20 && (!this.stats.boomerang || this.atHome) &&
		**	            !this.parent.parent.arena.tiles[tilePos.x][tilePos.y])
		**	                this.landerPerc += this.parent.stats.speed;
		**	        else
		**	                this.landerPerc -= this.parent.stats.speed;
		**	        this.landerPerc = limit(this.landerPerc, 0, 100);
		**	        this.parent.stats.altitude = maxAltitude * (1 - this.landerPerc / 100);
		**	    }
		**
		**	Every frame, both directions, and nothing latched.  A craft that is
		**	coming down and is given something to do goes straight back up from
		**	wherever it had got to, because the same test that lowered it raises
		**	it the moment it stops being true.
		**
		**	This gate used to be shut while either flag was set -- `!IsLanding &&
		**	!IsTakingOff` -- so a descent, once begun, ran to the ground whatever
		**	happened next.  Order a Switch Fighter away mid-settle and it landed
		**	anyway; worse, AI() only calls Commence() when neither flag is set, so
		**	the order could not even start while it was coming down.  It seated,
		**	and only then noticed it had somewhere to be.
		**
		**	`still` is the same count the source keeps: reset when a new path is
		**	set, and otherwise counting frames of not moving.
		*/
		if (NavCom != CACLastNav) {
			CACLastNav = NavCom;
			CACStill = 0;
		}

		/*
		**	Somewhere real to be is not standing still.
		**
		**	`still` in UnitNav counts frames without movement, and a craft that is
		**	supposed to be crossing the map but happens to read speed nought this
		**	frame -- a mission that has not called Process_Fly_To yet, a fifteen
		**	tick delay out of VALIDATE_LZ, a turn -- is not still in that sense.
		**	Counting it as idle is what let a flier settle in mid-air with a
		**	destination twenty-five cells away.
		**
		**	Measured, not merely asked.  Keying on whether a destination *exists*
		**	is the older mistake and cost the boomerangs their berth: one that has
		**	arrived still holds its station as a destination, so it never counted
		**	as idle and never came down.  A cell's distance separates the two --
		**	arrived is inside it, travelling is not.
		*/
		bool travelling = false;
		if (Target_Legal(NavCom)) {
			COORDINATE want = ::As_Coord(NavCom);
			if (want != 0 && ::Distance(Center_Coord(), want) > 0x0100) travelling = true;
		}

		/*
		**	And a transport being boarded is not restless either.
		**
		**	Loading holds radio contact -- Mission_Enter answers RADIO_TETHER, see
		**	the Receive_Message branch below -- and during it the craft picks up
		**	and puts down speed as passengers arrive and it shuffles.  Every blip
		**	reset the count, the count is what lowers a lander, and the reversal is
		**	immediate in both directions: so the carrier rose and sank for as long
		**	as anyone was walking into it, which is the report.
		**
		**	It is on the ground because it is being loaded.  That is a reason to
		**	stay there, not a reason to keep asking.
		*/
		bool loading = (Class != NULL && Class->IsTransporter && (In_Radio_Contact() || CAC_Has_Boarders(this)));

		/*
		**	A tether nobody holds the other end of.  The old boarding path left one
		**	behind, and it blocks every boarder and the unload; nothing CrystAlien
		**	carries is ever tethered now (CAC_Try_Embark).
		*/
		if (Class != NULL && Class->IsTransporter && IsTethered && !In_Radio_Contact() &&
			 Mission != MISSION_UNLOAD && CAC_Is_Own_Carrier(this)) {
			IsTethered = false;
		}

		/*
		**	And fighting is not idling.
		**
		**	Mission_Attack holds a craft that is in range at speed nought, which is
		**	right -- it should stand and shoot rather than circle.  But standing
		**	still is exactly what this count is looking for, so the descent began
		**	in the middle of an engagement, and the craft is not actually still:
		**	the target moves, range is won and lost, and Process_Fly_To nudges it a
		**	cell at a time.  The ground under it therefore changes between the
		**	frame that starts the descent and the frame that completes it, and
		**	CAC_Berth_Is_Clear answers differently to each:
		**
		**	    UPEVIL settles at 50,7 (altitude 24, speed 0, still 21, mission 1)
		**	    UPEVIL climbs  at 51,6 (altitude 18, speed 0, still 24, mission 1)
		**	    UPEVIL settles at 52,5 (altitude 24, speed 0, still 33, mission 1)
		**
		**	-- which is the Switch Fighter seating itself after being commanded,
		**	and the reason it has to be told twice.  Both ends already ask the same
		**	question; what they cannot agree on is a cell that keeps changing.
		**
		**	So it does not try to land while it is fighting.  The transformer still
		**	fights from the ground, because CAC_Engage fires from wherever it
		**	happens to be and a guarding craft settles as before -- what stops is
		**	trying to touch down in the middle of a firefight it is still flying.
		*/
		bool fighting = (Mission == MISSION_ATTACK) && Target_Legal(TarCom);

		bool idle = (Speed == 0 || loading) && Mission != MISSION_ENTER &&
						!travelling && !fighting;

		if (idle) {
			if (CACStill < 0xFFFF) CACStill++;
		} else {
			if (CACStill > 0 && CAC_Is_Boomerang(*this)) {
				CAC_Log_Berth_Restless(this, Target_Legal(NavCom) != 0, Speed, (int)Mission);
			}
			CACStill = 0;
		}

		/*
		**	A boomerang comes down on its plate and nowhere else, which is the
		**	`(!this.stats.boomerang || this.atHome)` half.  atHome in the source is
		**	an exact tile comparison against the station's dock:
		**
		**	    this.stats.home.stats.dockPos.x == this.parent.tilePos.x &&
		**	    this.stats.home.stats.dockPos.y == this.parent.tilePos.y
		**
		**	so CAC_On_Berth is the one asked here.  CAC_At_Berth is the looser
		**	radius and answers a different question -- whether CAC_Return_Home
		**	should bother ordering it home again -- and reading that one here is
		**	what let them settle two cells off the artwork.
		*/
		bool clear = CAC_Berth_Is_Clear(this);
		bool home  = !CAC_Is_Boomerang(*this) || CAC_On_Berth(this);
		bool settle = idle && CACStill > CAC_TRANSFORMER_IDLE_TICKS && clear && home;

		if (settle) {
			if (Altitude > 0 && !IsLanding) {
				CAC_Log_Lander(this, "settles", CACStill, Is_Door_Closed(), (int)Status);
			}
			if (Altitude > 0) {
				IsTakingOff = false;
				IsLanding = true;
			}
		} else {
			if (Altitude < FLIGHT_LEVEL && !IsTakingOff) {
				CAC_Log_Lander(this, "climbs", CACStill, Is_Door_Closed(), (int)Status);

				/*
				**	And which of the four said so, when it had every reason to stay.
				**	The line above prints idle's inputs but not clear or home, and a
				**	carrier climbing with `still` past five hundred could only be one
				**	of those two -- which took reading the arithmetic to work out.
				*/
				if (idle && CACStill > CAC_TRANSFORMER_IDLE_TICKS) {
					CAC_Log_Volatile("lander", "  %s would have stayed down: clear %s, home %s",
											 Class->IniName, clear ? "yes" : "NO",
											 home ? "yes" : "NO");
				}
			}
			if (Altitude < FLIGHT_LEVEL) {
				IsLanding = false;
				IsTakingOff = true;
			}
			if (idle && CACStill > CAC_TRANSFORMER_IDLE_TICKS &&
				 CAC_Is_Boomerang(*this) && Altitude > 0) {
				CAC_Log_Berth_Refusal(this, clear, CAC_At_Berth(this));
			}
		}
	}

	/*
	**	A Mission change can always occur if the aircraft is landed or flying.
	*/
	/*
	**	Ours commences whatever its altitude is doing.
	**
	**	landerPerc and the order queue are unrelated in the source -- the ramp is
	**	driven from UnitNav.handle every frame and nothing about it defers a
	**	path.  Tiberian Dawn couples them, and with the ramp now running both
	**	ways (see the lander block above) that coupling would leave a craft that
	**	is climbing or settling unable to start the very order that is making it
	**	climb.
	*/
	if (CAC_Is_Lander(*this) || (!IsLanding && !IsTakingOff)) {
		Commence();
	}

	/*
	**	Handle any body rotation at this time. Body rotation can occur even if the
	**	flying object is not actually moving.
	*/
	if (PrimaryFacing.Is_Rotating()) {
		if (PrimaryFacing.Rotation_Adjust(Class->ROT)) {
			Mark();
		}
	}
	if (Class->IsFixedWing) {
		SecondaryFacing = PrimaryFacing;
	}
	if (SecondaryFacing.Is_Rotating()) {
		if (SecondaryFacing.Rotation_Adjust(Class->ROT)) {
			Mark();
		}
	}

	/*
	**	Handle reinforcement delay.
	*/
	bool do_physics = true;
	if (Special.ModernBalance) {
		if (*this == AIRCRAFT_CARGO && !Map.In_Radar(Coord_Cell(Coord)) && ReinforcementStart > Frame) {
			do_physics = false;
		}
	}
	if (do_physics && Physics(Coord, PrimaryFacing) != RESULT_NONE) {
		Mark();
	}

	/*
	**	Perform sighting every so often as controlled by the sight timer.
	*/
	//if (IsOwnedByPlayer && Class->SightRange && SightTimer.Expired()) {		// Changed for multiple player mapping
	if (House->IsHuman && Class->SightRange && SightTimer.Expired()) {
		Map.Sight_From(House, Coord_Cell(Coord), Class->SightRange, false);
		SightTimer = TICKS_PER_SECOND;
	}

	/*
	**	Handle landing and taking off logic. Helicopters are prime users of this technique. The
	**	aircraft will either gain or lose altitude as appropriate. As the aircraft transitions
	**	between flying level and ground level, it will be moved into the appropriate render
	**	layer.
	*/
	/*
	**	CrystAlien has no door, so ours is never held on the ground by one.
	**
	**	A stock transport closes its door in one place -- RADIO_IM_IN, and only
	**	when it is completely full -- so a Hyper Carrier that unloaded, or that
	**	was carrying two of six, kept an open door for the rest of its life.  An
	**	open door here means the altitude ramp below never runs at all, so the
	**	craft accepted orders, set IsTakingOff and stayed exactly where it was.
	**
	**	It is closed above when the craft is given somewhere to go, which keeps
	**	the passenger logic honest, but the ramp should not have depended on it
	**	in the first place: altitude in UnitNav is a consequence of landerPerc
	**	and nothing else.
	*/
	bool cacramp = (Class != NULL) && CAC_Is_CrystAlien_Type(Class->IniName);

	if ((cacramp || Is_Door_Closed()) && (IsLanding || IsTakingOff)) {
		Mark();
		LayerType layer = In_Which_Layer();

		if (IsLanding) {
			int step = CAC_Is_Lander(*this) ? CAC_Lander_Step(this) : 1;
			Altitude -= step;
			if (Altitude < 0) Altitude = 0;
			if (!Altitude) {
				IsLanding = false;
				Set_Speed(0);
				if (Target_Legal(NavCom) && As_Techno(NavCom) == Contact_With_Whom()) {
					if (In_Radio_Contact() && Transmit_Message(RADIO_IM_IN) != RADIO_ROGER) {
						Scatter(0, true);
					}
				}
			}
		}
		if (IsTakingOff) {
			Altitude += CAC_Is_Lander(*this) ? CAC_Lander_Step(this) : 1;
			if (Altitude >= FLIGHT_LEVEL) {
				Altitude = FLIGHT_LEVEL;
				IsTakingOff = false;
			}
		}

		/*
		**	Make adjustments for altitude by moving from one layer to another as
		**	necessary.
		*/
		if (layer != In_Which_Layer()) {

			/*
			**	When the aircraft is about to enter the ground layer, perform on last
			**	check to see if it is legal to enter that location. If not, then
			**	start the take off process. Let the normal logic handle this
			**	change of plans.
			*/
			bool ok = true;
			if (In_Which_Layer() == LAYER_GROUND) {

				/*
				**	Both ends of a landing have to agree about the ground.
				**
				**	The descent is started by CAC_Berth_Is_Clear, which is
				**	CrystAlien's rule -- open terrain, and another of our own fliers
				**	is not in the way because it has no block to be in the way with.
				**	This is the other end of the same landing, and it asked
				**	Is_LZ_Clear instead: no object at all in the cell, and
				**	Is_Generally_Clear on top, which is false for rock *and* for
				**	tiberium.
				**
				**	So one end said come down and the other said go back up, every
				**	cycle, and the craft hung there rising and settling and never
				**	getting anywhere.  On a map with cliffs and crystal fields on it
				**	that is most of the ground.
				**
				**	Ask the same question at both ends.  A CrystAlien flier settles
				**	where CrystAlien would let it settle, which includes a crystal
				**	field -- resource cells are `bait` and not `tiles`, so the source
				**	has never minded -- and stock aircraft keep the rule they had.
				*/
				bool clear;
				if (Class != NULL && CAC_Is_CrystAlien_Type(Class->IniName)) {
					clear = CAC_Berth_Is_Clear(this);
				} else {
					clear = Is_LZ_Clear(::As_Target(Coord_Cell(Coord)));
				}

				if (!clear) {
					IsTakingOff = true;
					Altitude++;
					ok = false;
				}
			}

			if (ok) {
				/*
				**	If landing in a cell that already contains an object, then
				**	the landing attempt must be aborted.
				*/
				Map.Remove(this, layer);
				Map.Submit(this, In_Which_Layer());

				/*
				**	When the aircraft is close to the ground, it should exist as a ground object.
				**	This aspect is controlled by the Place_Down and Pick_Up functions.
				*/
				if (In_Which_Layer() == LAYER_GROUND) {
					Assign_Destination(TARGET_NONE);		// Clear the navcom.
					Transmit_Message(RADIO_TETHER);
					Map.Place_Down(Coord_Cell(Coord), this);
					//if (IsOwnedByPlayer) {				// Changed for multiple player mapping. ST - 3/6/2019 1:31PM
					if (House->IsHuman) {
						Map.Sight_From(House, Coord_Cell(Coord), 1, false);
					}
				} else  {
					Transmit_Message(RADIO_UNTETHER);
					Map.Pick_Up(Coord_Cell(Coord), this);

					/*
					**	If the navigation computer is not attached to the object this
					**	aircraft is in radio contact with, then assume that radio
					**	contact is now superfluous. Break radio contact.
					*/
					if (In_Radio_Contact() && Target_Legal(NavCom) && NavCom != Contact_With_Whom()->As_Target()) {
						Transmit_Message(RADIO_OVER_OUT);
					}
				}
			}
		}
	}

	/*
	**	Always flag the map draw process to occur if there is an aircraft in the view.
	**	This ensures that it will be rendered even if there is nothing else that flagged
	**	the map to be redrawn.
	*/
	if (Map.In_View(Coord_Cell(Coord))) {
		Map.Flag_To_Redraw(false);
		Map.DisplayClass::IsToRedraw = true;
	}

	/*
	**	When aircraft leave the edge of the map, they might get destroyed. This occurs if the
	**	aircraft is a non-player produced unit and it has completed its mission. A transport
	**	helicopter that has already delivered reinforcements is a good example of this.
	*/
	if (!Map.In_Radar(Coord_Cell(Coord))) {
		if (Mission == MISSION_RETREAT /*|| (*this == AIRCRAFT_CARGO && !Is_Something_Attached())*/) {

			/*
			**	Check to see if there are any civilians aboard. If so, then flag the house
			**	that the civilian evacuation trigger event has been fulfilled.
			*/
			while (Is_Something_Attached()) {
				FootClass * obj = Detach_Object();
				if (obj->What_Am_I() == RTTI_INFANTRY && ((InfantryClass *)obj)->Class->IsCivilian && !((InfantryClass *)obj)->IsTechnician) {
					House->IsCivEvacuated = true;
				}

				/*
				**	Transport planes that leave can only be because they carry purchased
				**	equipment and must be have their cost refunded.
				*/
				if (*this == AIRCRAFT_CARGO) {
					House->Refund_Money(obj->Class_Of().Cost_Of());
				}
				delete obj;
			}
			Stun();
			Delete_This();
			return;
		}
	} else {
		IsLocked = true;
//		House->NewAScan |= (1L << Class->Type);

#ifdef NEVER
		/*
		**	Transport helicopters must ensure that their passengers are properly
		**	considered "alive" by setting the appropriate scan bits.
		*/
		FootClass const * foot = Attached_Object();
		while (foot) {
			switch (foot->What_Am_I()) {
				case RTTI_UNIT:
					House->NewUScan |= (1L << ((UnitTypeClass const &)Class_Of()).Type);
					break;

				case RTTI_INFANTRY:
					House->NewIScan |= (1L << ((InfantryTypeClass const &)Class_Of()).Type);
					break;
			}

			foot = (FootClass const *)foot->Next;
		}
#endif
	}
}


/***********************************************************************************************
 * AircraftClass::Mark -- Flags cells under the aircraft so that they will be redrawn.         *
 *                                                                                             *
 *    This routine is used to flag the cells under the aircraft so that those cells will       *
 *    be redrawn during the next map drawing process. This is a necessary step whenever the    *
 *    aircraft moves or changes shape.                                                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool AircraftClass::Mark(MarkType mark)
{
	Validate();
	if (FootClass::Mark(mark)) {
		Map.Refresh_Cells(Coord_Cell(Coord), Occupy_List());
		Map.Refresh_Cells(Coord_Cell(Coord), Overlap_List());
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * AircraftClass::Overlap_List -- Returns with list of cells the aircraft overlaps.            *
 *                                                                                             *
 *    When aircraft are flying, they can overlap quite a number of cells. These cells can      *
 *    be determined from the coordinate where the aircraft is centered and the size of the     *
 *    aircraft's shape. Landed aircraft are a special case and are usually much smaller        *
 *    than when flying.                                                                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to a cell offset list that specifies all cells that         *
 *          the aircraft overlaps given the aircraft's current state.                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
short const *AircraftClass::Overlap_List(void) const
{
	Validate();
	static short const _list[] = {
		-(MAP_CELL_W-1), -MAP_CELL_W, -(MAP_CELL_W+1),
		-1, 0, 1,
		(MAP_CELL_W-1), MAP_CELL_W, (MAP_CELL_W+1),
		-((MAP_CELL_W*2)-1), -(MAP_CELL_W*2), -((MAP_CELL_W*2)+1),
		-((MAP_CELL_W*3)-1), -(MAP_CELL_W*3), -((MAP_CELL_W*3)+1),
		REFRESH_EOL
	};

	if (Altitude) {
		return(_list);
		//return Coord_Spillage_List(Coord, 25);
	}
	return(Class->Overlap_List());
}


/***********************************************************************************************
 * AircraftClass::Init -- Initialize the aircraft system to an empty state.                    *
 *                                                                                             *
 *    This routine is used to clear out the aircraft allocation system. It is called in        *
 *    preparation for a scenario load or save game load.                                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::Init(void)
{
	AircraftClass *ptr;

	Aircraft.Free_All();

	ptr = new AircraftClass();
	VTable = ((void **)(((char *)ptr) + sizeof(AbstractClass) - 4))[0];
	delete ptr;
}


/***********************************************************************************************
 * AircraftClass::Mission_Unload -- Handles unloading cargo.                                   *
 *                                                                                             *
 *    This function is used to handle finding, heading toward, landing, and unloading the      *
 *    cargo from the aircraft. Once unloading of cargo has occurred, then the aircraft follows *
 *    a different mission.                                                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the number of game ticks to delay before calling this function again.      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/31/94   JLB : Created.                                                                 *
 *=============================================================================================*/
int AircraftClass::Mission_Unload(void)
{
	Validate();
	if (Class->IsFixedWing) {
		enum {
			PICK_AIRSTRIP,
			FLY_TO_AIRSTRIP,
			BUG_OUT
		};

		switch (Status) {

			/*
			**	Find a suitable airfield to land at.
			*/
			case PICK_AIRSTRIP:
				if (!Target_Legal(NavCom) || !In_Radio_Contact()) {

					BuildingClass * target_building = As_Building(NavCom);
					BuildingClass * building = (target_building != NULL && *target_building == STRUCT_AIRSTRIP) ? target_building : CAC_Find_Air_Home();
					if (building) {
						if (Transmit_Message(RADIO_HELLO, building) == RADIO_ROGER) {
							Set_Speed(0xFF);
							Assign_Destination(building->As_Target());
							if (Team) {
								Team->Target = NavCom;
							}
							Status = FLY_TO_AIRSTRIP;
						}
					}

					/*
					**	If a suitable airfield could not be found, then just randomly change
					**	direction and then try again later.
					*/
					if (Status == PICK_AIRSTRIP) {

						/*
						**	If there are no more airstrips, regardless of busy state, then
						**	abort this transport plane completely.
						*/
						if (!(House->ActiveBScan & STRUCTF_AIRSTRIP)) {
							Assign_Mission(MISSION_RETREAT);
						}

						/*
						**	Pick a new direction and fly off.
						*/
						PrimaryFacing.Set_Desired(Random_Pick(DIR_N, DIR_MAX));
						SecondaryFacing.Set_Desired(PrimaryFacing.Desired());
						return(TICKS_PER_SECOND*3);
					}
				} else {
					Status = FLY_TO_AIRSTRIP;
				}
				break;

			/*
			**	Home in on target. When close enough, drop the cargo.
			*/
			case FLY_TO_AIRSTRIP:
				if (!Target_Legal(NavCom) || !In_Radio_Contact()) {
					Status = PICK_AIRSTRIP;
				} else {

					/*
					**	If, for some reason, there is no cargo, then don't stick around.
					*/
					if (!Is_Something_Attached()) {
						Status = BUG_OUT;
						return(1);
					}

					if (!PrimaryFacing.Is_Rotating()) {
						PrimaryFacing.Set_Desired(Direction(As_Movement_Coord(NavCom)));
					}
					SecondaryFacing.Set_Desired(PrimaryFacing.Desired());

					int navdist = Distance(As_Movement_Coord(NavCom));
					Altitude = FLIGHT_LEVEL;
					if (navdist < 0x0600) {
						Altitude = Fixed_To_Cardinal(FLIGHT_LEVEL, Cardinal_To_Fixed(0x0600, navdist));
					}

					if (navdist < 0x0080) {
						FootClass * unit = (FootClass *)Detach_Object();

						if (unit) {
							CELL cell = Contact_With_Whom()->Find_Exit_Cell(unit);
							if (cell) {
								ScenarioInit++;
								if (!unit->Unlimbo(Cell_Coord(cell))) {
									Attach(unit);
								} else {

									/*
									**	Cargo planes announce reinforcements when they unload
									**	their cargo.
									*/
									if (*this == AIRCRAFT_CARGO && House == PlayerPtr) {
										Speak(VOX_REINFORCEMENTS, NULL, Cell_Coord(cell));
									}
									unit->IsALoaner = false;
									unit->IsLocked = true;
									unit->Scatter(0, true);
								}
								ScenarioInit--;

								Transmit_Message(RADIO_OVER_OUT);
								Assign_Target(TARGET_NONE);
							} else {
								Attach(unit);
							}

//							if (Is_Something_Attached()) {
//								Status = PICK_AIRSTRIP;
//							} else {
								Status = BUG_OUT;
//							}
						} else {
							Status = BUG_OUT;
						}
					}
					return(1);
				}
				break;

			/*
			**	All cargo unloaded, head off the map.
			*/
			case BUG_OUT:
				Assign_Mission(MISSION_RETREAT);
				return(1);
		}

	} else {
		enum {
			SEARCH_FOR_LZ,
			FLY_TO_LZ,
			LAND_ON_LZ,
			UNLOAD_PASSENGERS,
			TAKE_OFF,
		};

		switch (Status) {

			/*
			**	Search for an appropriate destination spot if one isn't already assigned.
			*/
			case SEARCH_FOR_LZ:
				if (Altitude == 0 && (Target_Legal(NavCom) || Coord == As_Coord(NavCom))) {
					Status = UNLOAD_PASSENGERS;
				} else {
					if (!Is_LZ_Clear(NavCom)) {
						Assign_Destination(New_LZ(::As_Target(Waypoint[WAYPT_REINF])));
					} else {
						if (Altitude == FLIGHT_LEVEL) {
							Status = FLY_TO_LZ;
						} else {
							Status = TAKE_OFF;
						}
					}
				}
				break;

			/*
			**	Fly to destination.
			*/
			case FLY_TO_LZ:
				/*
				**	A CrystAlien flier is not turned away by its own kind -- see
				**	CAC_Ignores_LZ.  Without this the second boomerang home found the plate
				**	occupied and was sent to New_LZ instead, which is the squadron slowly
				**	spreading out around the station over the course of a round.
				*/
				if (CAC_Ignores_LZ(this) || Is_LZ_Clear(NavCom)) {
					int distance = Process_Fly_To(true);

					if (distance < 0x0100) {
						SecondaryFacing.Set_Desired(Pose_Dir());

						if (distance < 0x0010) {
							Status = LAND_ON_LZ;
						}
						return(1);
					} else {
						SecondaryFacing.Set_Desired(PrimaryFacing.Desired());
						return(5);
					}
				} else {
					Status = SEARCH_FOR_LZ;
				}
				break;

			/*
			**	Landing phase. Just delay until landing is complete. At that time,
			**	transition to the unloading phase.
			*/
			case LAND_ON_LZ:
				if (IsTakingOff) {
					Status = TAKE_OFF;
				} else {
					if (Process_Landing()) {
						Status = UNLOAD_PASSENGERS;
					}
				}
				return(1);

			/*
			**	Hold while unloading passengers. When passengers are unloaded the order for this
			**	transport gets changed to MISSION_RETREAT.
			*/
			case UNLOAD_PASSENGERS:
				if (!IsTethered) {
					if (Is_Something_Attached()) {
						FootClass * unit = (FootClass *)Detach_Object();

						/*
						**	First thing is to lift the transport off of the map so that the unlimbo
						**	process for the passengers is more likely to succeed.
						*/
						Map.Pick_Up(Coord_Cell(Coord), this);

						if (!Exit_Object(unit)) {
							delete unit;
						}

						/*
						**	Restore the transport back down on the map.
						*/
						Map.Place_Down(Coord_Cell(Coord), this);

						if (!Is_Something_Attached()) {

							/*
							**	Ours steps off the drop and climbs away, which is
							**	the tail of doDeploy:
							**
							**	    this.nav.voyage(this.nav.currentX, this.nav.currentY + 1);
							**	    this.nav.landerPerc = 75;
							**	    this.nav.still = 0;
							**	    this.nav.hesitate = 0;
							**
							**	One tile down the screen, and back into the air from
							**	a quarter height -- so the carrier clears the troops
							**	it just put on the ground instead of sitting on top
							**	of them waiting to be told something.
							*/
							CELL below = (CELL)(Coord_Cell(Coord) + MAP_CELL_W);

							if (Class != NULL && CAC_Is_CrystAlien_Type(Class->IniName) &&
								 (unsigned)below < MAP_CELL_TOTAL && Map.In_Radar(below)) {

								Assign_Destination(::As_Target(below));
								Assign_Mission(MISSION_MOVE);
								IsLanding = false;
								IsTakingOff = true;
								Altitude = FLIGHT_LEVEL / 4;
								CACStill = 0;
								Commence();
							} else {
								Enter_Idle_Mode();
							}
						}

					} else  {

						Enter_Idle_Mode();
					}
				}
				break;

			/*
			**	Aircraft is now taking off. Once the aircraft reaches flying altitude then it
			**	will either take off or look for another landing spot to try again.
			*/
			case TAKE_OFF:  {
				if (Process_Take_Off()) {
					if (Is_Something_Attached()) {
						Status = SEARCH_FOR_LZ;

						/*
						**	Break off radio contact with the helipad it is taking off from.
						*/
						if (In_Radio_Contact() && Map[Coord_Cell(Coord)].Cell_Building() == Contact_With_Whom()) {
							Transmit_Message(RADIO_OVER_OUT);
						}
					} else  {
						Enter_Idle_Mode();
					}
				}
				return(1);
			}
		}
	}
	return(10);
}


/***********************************************************************************************
 * AircraftClass::Is_LZ_Clear -- Determines if landing zone is free for landing.               *
 *                                                                                             *
 *    This routine examines the landing zone (as specified by the target parameter) in order   *
 *    to determine if it is free to be landed upon. Call this routine when it is necessary     *
 *    to double check this. Typically this occurs right before a helicopter lands and also     *
 *    when determining the landing zone in the first place.                                    *
 *                                                                                             *
 * INPUT:   target   -- The target that is the "landing zone".                                 *
 *                                                                                             *
 * OUTPUT:  bool; Is the landing zone clear for landing?                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/31/94   JLB : Created.                                                                 *
 *=============================================================================================*/
bool AircraftClass::Is_LZ_Clear(TARGET target) const
{
	Validate();
	if (!Target_Legal(target)) return(false);
	CELL cell = ::As_Cell(target);
	if (!Map.In_Radar(cell)) return(false);

	ObjectClass * object = Map[cell].Cell_Object();
	if (object) {
		if (object == this) return(true);

		if (In_Radio_Contact() && Contact_With_Whom() == object) {
			return(true);
		}
		return(false);
	}

	if (!Map[cell].Is_Generally_Clear()) return(false);

	return(true);
}


/***********************************************************************************************
 * AircraftClass::In_Which_Layer -- Determine which render layer the aircraft lies.            *
 *                                                                                             *
 *    This routine is used to figure out which rendering layer the aircraft is located in.     *
 *    It can be determined from the aircraft's height. The layer value is used to handle the   *
 *    display sequence. Objects in lower layers appear beneath objects in higher layers.       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the layer that the aircraft is currently located in.                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/01/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
LayerType AircraftClass::In_Which_Layer(void) const
{
	Validate();
	if (Class->IsFixedWing) return(LAYER_TOP);

	if (Altitude < FLIGHT_LEVEL - (FLIGHT_LEVEL/3)) {
		return(LAYER_GROUND);
	}
	return(LAYER_TOP);
}


/***********************************************************************************************
 * AircraftClass::Sort_Y -- Figures the sorting coordinate.                                    *
 *                                                                                             *
 *    This routine is used to determine the coordinate to use for sorting the aircraft. This   *
 *    sorting value is used when the aircraft is on the ground. At that time the aircraft      *
 *    must be rendered in proper relationship to the other ground objects.                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the coordinate to use when sorting the aircraft with other ground     *
 *          objects.                                                                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/02/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
COORDINATE AircraftClass::Sort_Y(void) const
{
	Validate();
	return(Coord_Add(Coord, 0x00800000L));
}


/***********************************************************************************************
 * AircraftClass::Mission_Retreat -- Handles the aircraft logic for leaving the battlefield.   *
 *                                                                                             *
 *    This mission will be followed when the aircraft decides that it is time to leave the     *
 *    battle. Typically, this occurs when a loaner transport has dropped off its load or when  *
 *    an attack air vehicle has expended its ordinance.                                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of game ticks to delay before calling this routine again.  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/19/1995 JLB : Created.                                                                 *
 *   08/13/1995 JLB : Handles aircraft altitude gain after takeoff logic.                      *
 *=============================================================================================*/
int AircraftClass::Mission_Retreat(void)
{
	Validate();
	if (Class->IsFixedWing) {
		if (*this == AIRCRAFT_CARGO) {
			PrimaryFacing.Set_Desired(DIR_W);
			SecondaryFacing.Set_Desired(PrimaryFacing.Desired());
		}
		if (Altitude < FLIGHT_LEVEL) {
			Altitude++;
			return(1);
		}
		return(TICKS_PER_SECOND*10);
	}

	enum {
		TAKE_OFF,
		FACE_MAP_EDGE,
		KEEP_FLYING
	};
	switch (Status) {

		/*
		**	Take off if landed.
		*/
		case TAKE_OFF:
			if (Process_Take_Off()) {
				Status = FACE_MAP_EDGE;
			}
			return(1);

		/*
		**	Set facing and speed toward the friendly map edge.
		*/
		case FACE_MAP_EDGE:
			Set_Speed(0xFF);

			/*
			**	Take advantage of the fact that the source map edge enumerations happen to
			**	occur in a clockwise order and are the first four enumerations of the map
			**	edge default for the house. If this value is masked and then shifted, a
			**	normalized direction value results. Use this value to head the aircraft
			**	toward the "friendly" map edge.
			*/
			PrimaryFacing.Set_Desired((DirType)((House->Edge & 0x03) << 6));
			SecondaryFacing.Set_Desired(PrimaryFacing.Desired());
			Status = KEEP_FLYING;
			break;

		/*
		**	Just do nothing since we are headed toward the map edge. When the edge is
		**	reached, the aircraft should be automatically eliminated.
		*/
		case KEEP_FLYING:
			break;

		default:
			break;
	}
	return(TICKS_PER_SECOND);
}


/***********************************************************************************************
 * AircraftClass::Exit_Object -- Unloads passenger from aircraft.                              *
 *                                                                                             *
 *    This routine is called when the aircraft is to unload a passenger. The passenger must    *
 *    be able to move under its own power. Typical situation is when a transport helicopter    *
 *    is to unload an infantry unit.                                                           *
 *                                                                                             *
 * INPUT:   unit  -- Pointer to the unit that is to be unloaded from this aircraft.            *
 *                                                                                             *
 * OUTPUT:  bool; Was the unit unloaded successfully?                                          *
 *                                                                                             *
 * WARNINGS:   The unload process is merely started by this routine. Radio contact is          *
 *             established with the unloading unit and when the unit is clear of the aircraft  *
 *             the radio contact will be broken and then the aircraft is free to pursue        *
 *             other.                                                                          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int AircraftClass::Exit_Object(TechnoClass * unit)
{
	Validate();
	static FacingType _toface[FACING_COUNT] = {FACING_S, FACING_SW, FACING_SE, FACING_NW, FACING_NE, FACING_N, FACING_W, FACING_E};
	CELL	cell;

	/*
	**	Find a free cell to drop the unit off at.
	*/
	FacingType face;
	for (face = FACING_N; face < FACING_COUNT; face++) {
		cell = Adjacent_Cell(Coord_Cell(Coord), _toface[face]);
		if (unit->Can_Enter_Cell(cell) == MOVE_OK) break;
	}

	// Should perform a check here to see if no cell could be found.

	/*
	**	If the passenger can be placed on the map, then start it moving toward the
	**	destination cell and establish radio contact with the transport. This is used
	**	to make sure that the transport waits until the passenger is clear before
	**	unloading the next passenger or taking off.
	*/
	if (unit->Unlimbo(Coord, Facing_Dir(_toface[face]))) {
		unit->Assign_Mission(MISSION_MOVE);
		unit->Assign_Destination(::As_Target(cell));
		if (Transmit_Message(RADIO_HELLO, unit) == RADIO_ROGER) {
			Transmit_Message(RADIO_UNLOAD);
		}
		unit->Look(false);
		return(1);
	}
	return(0);
}


/***********************************************************************************************
 * AircraftClass::Fire_At -- Handles firing a projectile from an aircraft.                     *
 *                                                                                             *
 *    Sometimes, aircraft firing needs special handling. Example: for napalm bombs, the        *
 *    bomb travels forward at nearly the speed of the delivery aircraft, not necessarily the   *
 *    default speed defined in the BulletTypeClass structure.                                  *
 *                                                                                             *
 * INPUT:   target   -- The target that the projectile is heading for.                         *
 *                                                                                             *
 *          which    -- Which weapon to use in the attack. 0=primary, 1=secondary.             *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the bullet that was created as a result of this attack.  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
BulletClass * AircraftClass::Fire_At(TARGET target, int which)
{
	Validate();
	BulletClass * bullet = FootClass::Fire_At(target, which);

	if (bullet) {

		/*
		**	Aircraft reveal when firing
		*/
		HouseClass *player = HouseClass::As_Pointer(Owner());
		if (player != nullptr && player->IsHuman) {
			Map.Sight_From(player, Coord_Cell(Center_Coord()), 1, false);
		}

		/*
		**	Play the sound effect associated with this weapon.
		*/
		WeaponTypeClass const * weapon = (which == 0) ? &Weapons[Class->Primary] : &Weapons[Class->Secondary];
		Sound_Effect(weapon->Sound, Coord);


		/*
		**	Falling bullets move at a speed proportionate to the delivery craft.
		*/
		if (bullet->Class->IsDropping) {
			bullet->Fly_Speed(40, bullet->Class->MaxSpeed);
		}
	}
	return(bullet);
}


/***********************************************************************************************
 * AircraftClass::Take_Damage -- Applies damage to the aircraft.                               *
 *                                                                                             *
 *    This routine is used to apply damage to the specified aircraft. This is where any        *
 *    special crash animation will be initiated.                                               *
 *                                                                                             *
 * INPUT:   damage   -- Reference to the damage that will be applied to the aircraft.          *
 *                      This value will be filled in with the actual damage that was           *
 *                      applied.                                                               *
 *                                                                                             *
 *          distance -- Distance from the source of the explosion to this aircraft.            *
 *                                                                                             *
 *          warhead  -- The warhead type that the damage occurs from.                          *
 *                                                                                             *
 *          source   -- Pointer to the originator of the damage. This can be used so that      *
 *                      proper "thank you" can be delivered.                                   *
 *                                                                                             *
 * OUTPUT:  Returns with the result of the damage as it affects this aircraft.                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/26/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ResultType AircraftClass::Take_Damage(int & damage, int distance, WarheadType warhead, TechnoClass * source)
{
	Validate();
	ResultType res = RESULT_NONE;

	/*
	**	Flying aircraft take half damage.
	**
	**	Tiberian Dawn's rule, and CrystAlien has no equivalent.  Unit.pain is the
	**	whole of its damage model --
	**
	**	    this.health -= weapon.dmg;
	**
	**	-- flat, with no altitude term and no armour table, which is the same
	**	reason every CrystAlien warhead in CONST.CPP carries 0x100 across the
	**	board.  Halving here is the shape of mistake BuildingTypeClass's doubled
	**	strength was: a stock convention quietly rewriting a ported number, so
	**	nobody plays against the figure in the table.
	**
	**	It lands hardest on the transformer, which is the one type that chooses
	**	its altitude.  Setting down is what a Switch Fighter is *for*, and under
	**	this rule doing it doubles everything that hits it.
	*/
	if (Altitude && !CAC_Is_CrystAlien_Type(Class->IniName)) {
		damage /= 2;
	}

	/*
	**	In order for a this to be damaged, it must either be a unit
	**	with a crew or a sandworm.
	*/
	res = FootClass::Take_Damage(damage, distance, warhead, source);

	switch (res) {
		case RESULT_DESTROYED:
			{
				Kill_Cargo(source);
				Death_Announcement();
				COORDINATE coord = Target_Coord();

				/*
				**	And not Tiberian Dawn's fireball on top of ours.
				**
				**	CAC_Death_Effect already gives a CrystAlien flier CACBOOM, from
				**	TechnoClass::Take_Damage, so this raised a second explosion from
				**	the wrong game over every one of them.  Worse than cosmetic:
				**	ANIM_FBALL1 forms a crater, so every aircraft that died stamped
				**	one of Tiberian Dawn's CR1..CR6 on the ground -- which is what a
				**	session's asset log shows, asking for FBALL1 and then CR1 in the
				**	same frame and never once for CACSCORCH.
				**
				**	Same correction as the structure death in BUILDING.CPP.
				*/
				if (!(coord & 0xC000C000L) && !CAC_Is_CrystAlien_Type(Class->IniName)) {
					new AnimClass(ANIM_FBALL1, coord);
				}
				Delete_This();
			}
			break;

		default:
		case RESULT_HALF:
			break;
	}

	return(res);
}


/***********************************************************************************************
 * AircraftClass::Mission_Move -- Handles movement mission.                                    *
 *                                                                                             *
 *    This state machine routine is used when an aircraft (usually helicopter) is to move      *
 *    from one location to another. It will handle any necessary take off and landing this     *
 *    may require.                                                                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of game frames that should elapse before this routine      *
 *          is called again.                                                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int AircraftClass::Mission_Move(void)
{
	Validate();
	if (Class->IsFixedWing) {

		/*
		**	Force aircraft in movement mission into a retreat
		**	mission so that it leaves the map.
		*/
		if (*this == AIRCRAFT_A10) {
			Assign_Mission(MISSION_RETREAT);
			Commence();
			return(1);
		}

		enum {
			FLY_TO_AIRSTRIP,
			BUG_OUT
		};
		switch (Status) {
			/*
			**	Home in on target. When close enough, drop the cargo.
			*/
			case FLY_TO_AIRSTRIP:
				if (!Target_Legal(NavCom) || !In_Radio_Contact()) {
					return(TICKS_PER_SECOND);
				} else {

					/*
					**	If, for some reason, there is no cargo, then don't stick around.
					*/
					if (!Is_Something_Attached()) {
						Status = BUG_OUT;
						return(1);
					}


					if (!PrimaryFacing.Is_Rotating()) {
						PrimaryFacing.Set_Desired(Direction(NavCom));
					}
					SecondaryFacing.Set_Desired(PrimaryFacing.Desired());
					if (Distance(NavCom) < 0x0080) {
						FootClass * unit = (FootClass *)Detach_Object();

						if (unit) {
							ScenarioInit++;
							if (!unit->Unlimbo(Coord_Snap(Contact_With_Whom()->Coord))) {
								Attach(unit);
							}
							ScenarioInit--;

							Transmit_Message(RADIO_OVER_OUT);
							Assign_Target(TARGET_NONE);
						}
						Status = BUG_OUT;
					}
				}
				return(1);

			case BUG_OUT:
				return(TICKS_PER_SECOND);
		}
		return(5);
	}

	enum {
		VALIDATE_LZ,
		TAKE_OFF,
		FLY_TO_LZ,
		LAND
	};
	switch (Status) {

		/*
		**	Double check and change LZ if necessary.
		*/
		case VALIDATE_LZ:
			if (!Target_Legal(NavCom)) {
				Enter_Idle_Mode();
			} else {
				/*
				**	The third of these, and the one that was never guarded.
				**	
				**	Two other places send a craft to New_LZ when its landing spot looks taken
				**	and both already ask CAC_Ignores_LZ first, because a CrystAlien flier does
				**	not claim its tile and a squadron is supposed to gather on one plate.  This
				**	one was missed, and it is the one on the way *in*: a Fighter arriving at a
				**	berth that already has somebody on it was quietly redirected somewhere
				**	else, four to six cells off, where it then sat too far away to count as
				**	home and never settled.
				**	
				**	That is "they sometimes land properly but not every time" exactly: it
				**	depended on whether the plate happened to be clear when that craft
				**	arrived.  A log of one round has two hundred and seventy-six refusals
				**	reading "berth is clear and it is not on it", the craft stranded four to
				**	six cells short of a berth that was free by the time it was asked.
				*/
				if (!CAC_Ignores_LZ(this) &&
					(!Is_LZ_Clear(NavCom) || !Cell_Seems_Ok(As_Cell(NavCom)))) {
					Assign_Destination(New_LZ(NavCom));
				} else if (CAC_Is_Lander(*this)) {

					/*
					**	A CrystAlien lander climbs on its way, not before it.
					**
					**	UnitNav.think walks the path and then, further down the same
					**	function, moves `landerPerc` toward the ground when the craft
					**	has been still and away from it otherwise:
					**
					**	    if (stats.lander && (this.still || this.landerPerc)) {
					**	        if (still > 20 && ...) landerPerc += speed;
					**	        else                   landerPerc -= speed;
					**	        stats.altitude = maxAltitude * (1 - landerPerc / 100);
					**	    }
					**
					**	Altitude is a consequence of moving, never a condition of it.
					**	Tiberian Dawn's TAKE_OFF is the other way round: it holds the
					**	craft on the spot until Process_Take_Off answers true, and that
					**	only answers on exact altitudes, so a craft that misses one
					**	sits there.  A transport told to move rose, found the state
					**	machine unwilling to hand it on, and settled again -- which is
					**	the raising and lowering, and why the log caught it running a
					**	fly-to with its destination already cleared.
					**
					**	So ours goes straight to flying, and the descent gate lifts it
					**	as it goes: a craft with a destination is not idle, and not
					**	idle below flight level is IsTakingOff.  Two things happening
					**	together, which is what the source does.
					*/
					Status = FLY_TO_LZ;
				} else {
					Status = TAKE_OFF;
				}
			}
			break;

		/*
		**	Take off if necessary.
		*/
		case TAKE_OFF:
			if (!Target_Legal(NavCom)) {
				Status = VALIDATE_LZ;
			} else {
				if (Process_Take_Off()) {
					/*
					**	After takeoff is complete, break radio contact with any helipad that this
					**	helicopter is taking off from.
					*/
					if (In_Radio_Contact() && Map[Coord_Cell(Coord)].Cell_Building() == Contact_With_Whom()) {
						Transmit_Message(RADIO_OVER_OUT);
					}

					Status = FLY_TO_LZ;
				}
				return(1);
			}
			break;

		/*
		**	Fly toward target.
		*/
		case FLY_TO_LZ:

			/*
			**	A flier that never lands finishes its journey in the air.
			**
			**	The state machine below ends every move by putting the craft on the
			**	ground -- close enough to the destination and `Status = LAND`.  That is
			**	right for a helicopter and wrong for a JetPack Explorer, which carries
			**	`lander: false` and holds fifty altitude from the day it is built.
			**
			**	Is_LZ_Clear is the second half of the same mistake.  A landing zone with
			**	something standing in it sends the craft off to New_LZ() -- a different
			**	cell entirely -- so ordering one of these to a spot with a tank on it
			**	moved it somewhere else instead.  Nothing that flies at altitude in
			**	CrystAlien cares what is underneath it: `obstruct` is false both ways.
			**
			**	So for these the arrival is the end of it -- stop, and hold station.
			*/
			if (CAC_Never_Lands(this)) {
				int distance = Process_Fly_To(true);

				if (distance < 0x0010) {
					Assign_Destination(TARGET_NONE);
					Enter_Idle_Mode();
				} else if (Target_Legal(TarCom)) {
					SecondaryFacing.Set_Desired(Direction(TarCom));
				} else {
					SecondaryFacing.Set_Desired(::Direction(Fire_Coord(0), As_Coord(NavCom)));
				}
				return(1);
			}

			/*
			**	A CrystAlien flier is not turned away by its own kind -- see
			**	CAC_Ignores_LZ.  Without this the second boomerang home found the plate
			**	occupied and was sent to New_LZ instead, which is the squadron slowly
			**	spreading out around the station over the course of a round.
			*/
			/*
			**	Off the ground before it slides along it.
			**
			**	A landed aircraft has been Place_Down onto its cell, so translating
			**	it while it is still there would leave the occupier list holding a
			**	craft that has gone.  The ramp is driven rather than waited on: set
			**	IsTakingOff and let the altitude code lift it, which is the same
			**	thing the descent gate does when a craft stops being idle, and
			**	nothing here waits on Process_Take_Off answering true.
			*/
			if (CAC_Is_Lander(*this) && Altitude <= 0) {
				IsLanding = false;
				IsTakingOff = true;
				CAC_Log_Lander(this, "is on the ground with somewhere to be",
									CACStill, Is_Door_Closed(), (int)Status);
				return(1);
			}

			if (CAC_Ignores_LZ(this) || Is_LZ_Clear(NavCom)) {
				int distance = Process_Fly_To(true);

				if (distance < 0x0080) {
					if (Target_Legal(TarCom)) {
						SecondaryFacing.Set_Desired(Direction(TarCom));
					} else {
						SecondaryFacing.Set_Desired(Pose_Dir());
					}

					if (distance < 0x0010) {
						/*
						**	A boomerang that has arrived somewhere that is not its
						**	berth turns round rather than setting down.
						**
						**	Every completed path in the source ends the same way --
						**	`if (this.parent.stats.boomerang) this.returnHome();` --
						**	so a Fighter which has flown its run goes home, and lands
						**	only once it is there, because UnitNav will not put a
						**	boomerang down anywhere else.
						**
						**	This state machine ends every move by landing, so ours came
						**	down wherever the sortie happened to finish.  The descent
						**	gate then correctly refused to seat it, and CAC_Return_Home
						**	sent it home, and the round went on like that: one log has
						**	1,378 refusals against 16 craft actually reaching a berth,
						**	882 of them reading "berth is clear and it is not on it"
						**	from four to seven cells away.  The seating was never the
						**	problem; they were landing in the wrong place first.
						*/
						if (CAC_Is_Boomerang(*this) && !CAC_At_Berth(this)) {
							/*
							**	Sent straight here rather than through
							**	CAC_Return_Home, which paces itself to once a second
							**	per craft -- borrowing it would leave the destination
							**	unset on fourteen frames in fifteen, and the fallback
							**	below would land the craft anyway.
							*/
							BuildingClass * home = CAC_Home_Building(this);
							CELL berth = (home != NULL) ? CAC_Exit_Cell(home, this) : 0;

							if (home != NULL && (unsigned)berth < MAP_CELL_TOTAL) {
								Assign_Destination(::As_Target(berth));
								return(1);
							}

							Status = LAND;		/* nowhere left to go home to */
							return(1);
						}

						/*
						**	Ours does not land because it arrived.
						**
						**	UnitNav.endMove ends the path and nothing else.  The
						**	craft comes down later, and only if it stays put:
						**
						**	    if (this.still > 20 && (!boomerang || atHome) &&
						**	        !arena.tiles[tilePos.x][tilePos.y])
						**	            this.landerPerc += this.parent.stats.speed;
						**
						**	Status = LAND is the other thing entirely.  It runs
						**	Process_Landing, which sets IsLanding on the spot, so
						**	the craft was seating the instant it arrived -- no
						**	twenty frames, no test of what it was seating onto.
						**	That is "transports insta-seat instead of flying".
						**
						**	It cost the orders as well.  Process_Landing holds
						**	IsLanding true all the way down, and AI() only calls
						**	Commence() when the craft is neither landing nor
						**	taking off, so a queued mission could not start while
						**	it was coming down.  Then Enter_Idle_Mode fires at the
						**	bottom, and its grounded arm is
						**
						**	    Assign_Destination(TARGET_NONE);
						**	    Assign_Target(TARGET_NONE);
						**	    mission = MISSION_GUARD;
						**
						**	-- so the destination was wiped and the craft dropped
						**	back to guard.  One session's roll call has the Hyper
						**	Carrier in Guard 2,432 times against 8 in Move.
						**
						**	And a re-order in that window was swallowed whole:
						**	Assign_Mission returns early when the mission is
						**	already the one being assigned, and the Status = 0
						**	beside it is commented out, so telling a landing
						**	transport to go somewhere else left it in LAND with a
						**	new destination it would never read.
						**
						**	So the path simply ends here, the way the source ends
						**	it, and the descent gate in AI() decides the rest.
						*/
						if (CAC_Is_Lander(*this)) {
							Assign_Destination(TARGET_NONE);
							Assign_Mission(MISSION_GUARD);
							return(1);
						}

						Status = LAND;
					}
					return(1);
				}

				SecondaryFacing.Set_Desired(::Direction(Fire_Coord(0), As_Coord(NavCom)));
			} else {
				Assign_Destination(New_LZ(NavCom));
				if (!Target_Legal(NavCom)) {
					Status = LAND;
				}
			}
			return(1);

		/*
		**	Land on target.
		*/
		case LAND:
			if (IsTakingOff) {
				Assign_Destination(New_LZ(NavCom));
				Status = TAKE_OFF;
			}
			if (Process_Landing()) {
				if (MissionQueue == MISSION_NONE) {
					Enter_Idle_Mode();
				}
			}
			return(1);
	}

	return(TICKS_PER_SECOND);
}


/***********************************************************************************************
 * AircraftClass::Enter_Idle_Mode -- Gives the aircraft an appropriate mission.                *
 *                                                                                             *
 *    Use this routine when the mission for the aircraft is in doubt. This routine will find   *
 *    an appropriate mission for the aircraft and dispatch it.                                 *
 *                                                                                             *
 * INPUT:   initial  -- Is this called when the unit just leaves a factory or is initially     *
 *                      or is initially placed on the map?                                     *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/05/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::Enter_Idle_Mode(bool )
{
	Validate();
	MissionType mission = MISSION_GUARD;
	if (In_Which_Layer() == LAYER_GROUND) {
		if (IsALoaner) {
			if (Is_Something_Attached()) {
				mission = MISSION_UNLOAD;
			} else {
				if (Team) {
					Team->Remove(this);
				}
				mission = MISSION_RETREAT;
			}
		} else {
#ifdef NEVER
			if (In_Radio_Contact() && Contact_With_Whom() == Map[Coord_Cell(Coord)].Cell_Techno()) {
				Transmit_Message(RADIO_IM_IN);
			}
#endif
			Assign_Destination(TARGET_NONE);
			Assign_Target(TARGET_NONE);
			mission = MISSION_GUARD;
		}
	} else {
		if (Is_Something_Attached()) {
			if (IsALoaner) {
				if (Team) {
					mission = MISSION_GUARD;
				} else {
					mission = MISSION_UNLOAD;
					Assign_Destination(Good_LZ());
				}
			} else if (CAC_Is_CrystAlien_Type(Class->IniName)) {

				/*
				**	A loaded Carrier holds station too, and this one matters more.
				**	
				**	The same Good_LZ that was flying empty Switch Fighters off the map is
				**	reached here by anything of ours with passengers aboard and nothing to
				**	do -- so a Hyper Carrier that finished a job with troops still in it
				**	would leave, taking them with it.  CAC_AI_Airlift usually has something
				**	for them, which is the only reason this was not the louder symptom.
				*/
				Assign_Destination(TARGET_NONE);
				mission = MISSION_GUARD;

			} else {
				Assign_Destination(Good_LZ());
				mission = MISSION_MOVE;
			}
		} else {

			/*
			**	If this transport is a loaner and part of a team, then remove it from
			**	the team it is attached to.
			*/
			if (IsALoaner) {
				if (Team) {
					Team->Remove(this);
				}
			}

			/*
			**	CrystAlien's fliers hold station instead of going home to land --
			**	their payload is effectively unlimited, so there is nothing to
			**	return for.  The Switch Fighter is the exception, settling on the
			**	ground being the whole point of it.  See CAC_Aircraft_Hovers.
			*/
			if (CAC_Aircraft_Hovers(*this)) {
				Assign_Destination(TARGET_NONE);
				mission = MISSION_GUARD;

			} else if (CAC_Is_Boomerang(*this)) {

				/*
				**	A boomerang goes to its berth, and does not negotiate for it.
				**
				**	The branch below asks the station for permission by radio and takes
				**	MISSION_ENTER if it is granted.  A building holds one contact at a time,
				**	so the first fighter home docked and every one after it was refused and
				**	sent to Good_LZ() instead -- a landing spot chosen with no reference to
				**	the station at all.  That is the swoop away from the plate.
				**
				**	CrystAlien does not negotiate either.  returnHome is one line --
				**	`voyage(home.stats.dockX, home.stats.dockY)` -- issued by every fighter
				**	of that station whenever its path runs out, and they simply gather.
				**	CAC_Return_Home does the same from AircraftClass::AI, so the right
				**	thing here is to stand still and let it.
				*/
				Assign_Destination(TARGET_NONE);
				mission = MISSION_GUARD;

			} else if (CAC_Is_CrystAlien_Type(Class->IniName)) {

				/*
				**	And the transformer, which is the only one of ours left.
				**
				**	CAC_Aircraft_Hovers exempts it, correctly -- setting down is the
				**	whole point of a Switch Fighter -- but that exemption dropped it
				**	into the helipad logic below, which either flies it home to a pad
				**	or, with no pad to be had, asks Good_LZ for a landing zone and is
				**	answered with a cell at the map edge.  Neither is CrystAlien:
				**	UnitNav brings a lander down where it is standing, and there are
				**	no landing zones at all.
				**
				**	Setting down is the descent gate's business in AI(), where the
				**	craft happens to be.  It is not a reason to go looking for a
				**	runway, and the runway search is how these left the map.
				*/
				Assign_Destination(TARGET_NONE);
				mission = MISSION_GUARD;

			} else if (Class->Primary != WEAPON_NONE) {

				/*
				**	Weapon equipped helicopters that run out of ammo and were
				**	brought in as reinforcements will leave the map.
				*/
				if (Ammo == 0 && !House->IsHuman && IsALoaner) {
					mission = MISSION_RETREAT;
				} else {

					/*
					**	Continue with the current helipad if there is one.
					*/
					if (!In_Radio_Contact() ||
						Contact_With_Whom()->What_Am_I() != RTTI_BUILDING ||
						!CAC_Is_Air_Home((StructType)*((BuildingClass*)Contact_With_Whom()))) {
						/*
						**	Normal aircraft try to find a good landing spot to rest.
						*/
						BuildingClass * building = CAC_Find_Air_Home();
						Assign_Destination(TARGET_NONE);
						if (building && Transmit_Message(RADIO_HELLO, building) == RADIO_ROGER) {
							mission = MISSION_ENTER;
						} else if (CAC_Is_CrystAlien_Type(Class->IniName)) {

							/*
							**	Ours stands still instead of looking for somewhere to land.
							**	
							**	Good_LZ answers with a landing zone, and when a house owns no
							**	helipad the zone it picks is off at the edge of the map -- which
							**	is where the Switch Fighters were going.  Straight off the screen
							**	as soon as they were built, and again every time they finished
							**	whatever they had been told to do.
							**	
							**	They take this branch because they are armed, are not boomerangs,
							**	and are not a holding-station type -- CAC_Aircraft_Hovers exempts
							**	the transformer, correctly, because setting down is the point of
							**	it.  But setting down is what the descent gate does, where it
							**	stands; it is not a reason to go looking for a runway.
							**	
							**	CrystAlien has no landing zones at all.  A flier with nothing to
							**	do stays where it is, and comes down there if it is a lander.
							*/
							Assign_Destination(TARGET_NONE);
							mission = MISSION_GUARD;

						} else {
							Assign_Destination(Good_LZ());
							mission = MISSION_MOVE;
						}
					} else {
						mission = MISSION_ENTER;
					}
				}
			} else {
				if (Team) return;

				/*	And the unarmed ones -- the Dropship and the Hyper Carrier -- for the
					same reason.  See the note above. */
				if (CAC_Is_CrystAlien_Type(Class->IniName)) {
					Assign_Destination(TARGET_NONE);
					mission = MISSION_GUARD;
				} else {
					Assign_Destination(Good_LZ());
					mission = MISSION_MOVE;
				}
			}
		}
	}

	/*
	**	And a CrystAlien flier that has stopped having something to do stops.
	**
	**	Physics moves a craft along PrimaryFacing at whatever Speed it holds,
	**	every frame, whatever mission it is on.  Enter_Idle_Mode is where every
	**	one of them ends up when its work finishes -- a target that died under it,
	**	an order cancelled, a magazine emptied -- and it changed the mission
	**	without touching the speed, so the craft carried on in the direction it
	**	happened to be pointing.  Mission_Guard zeroes it, but only on its own
	**	timer, and a second of a Switch Fighter's momentum is most of a cell.
	**
	**	Reported as "their momentum if their target dies and they are still flying
	**	in a direction", which is exactly what this is.
	**
	**	Only where the craft is not being sent somewhere: MISSION_MOVE and the
	**	landing missions below still need their speed, and a boomerang on its way
	**	to its berth is not idle.
	*/
	if (Class != NULL && CAC_Is_CrystAlien_Type(Class->IniName) &&
		 mission == MISSION_GUARD && !Target_Legal(NavCom) && Get_Speed() != MPH_IMMOBILE) {
		Set_Speed(0);
	}

	Assign_Mission(mission);
	Commence();
}


/***********************************************************************************************
 * AircraftClass::Process_Fly_To -- Handles state machine for flying to destination.           *
 *                                                                                             *
 *    This support routine is used when the helicopter is to fly to the destination. It can    *
 *    optionally slow the helicopter down as it approaches the destination.                    *
 *                                                                                             *
 * INPUT:   slowdown -- Should the aircraft be slowed down when it approaches the dest?        *
 *                                                                                             *
 * OUTPUT:  Returns with the distance remaining between the aircraft and the destination.      *
 *                                                                                             *
 * WARNINGS:   Because the aircraft can be move at a fast speed, the distance to target value  *
 *             will probably never be zero. The likely case will be that the aircraft          *
 *             overshoots the target.                                                          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/14/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int AircraftClass::Process_Fly_To(bool slowdown)
{
	Validate();
	COORDINATE coord;
	if (Is_Target_Building(NavCom)) {
		BuildingClass * dock = As_Building(NavCom);
		coord = (dock != NULL && dock->IsActive) ? dock->Docking_Coord() : 0x00000000L;
	} else {
		coord = As_Coord(NavCom);
	}

	/*
	**	A destination that does not resolve is the top-left corner of the map.
	**
	**	As_Coord answers 0x00000000 for TARGET_NONE, and -- by way of a kludge it
	**	documents -- for a target whose object has already died.  Coordinate zero
	**	is cell zero, outside the playable border in the corner, and this routine
	**	took it at face value: full speed, on a heading of due nothing, until the
	**	craft left the map.  That is the flier that "sometimes" wanders off after
	**	being sent somewhere -- what it was sent at blew up while it was in the
	**	air, and the corner is where a dead target lives.
	**
	**	It reads as a CrystAlien problem because ours stay airborne between jobs
	**	instead of going home to a pad, so they hold a stale NavCom far longer
	**	than a Tiberian Dawn helicopter ever does.  The bug is older than the mod.
	**
	**	Report no distance instead.  Every caller reads that as arrival and does
	**	the sensible local thing -- land here, hold station, or take the shot --
	**	rather than setting off for a corner nothing asked for.
	*/
	if (!Target_Legal(NavCom) || coord == 0x00000000L) {
		if (CAC_Logging()) {
			static long _when = -99999;
			if ((long)Frame - _when > (long)(TICKS_PER_SECOND * 10)) {
				_when = (long)Frame;
				CAC_Log_Volatile("flyto", "%s at %d,%d had no destination to fly to -- holding",
									 Class->IniName,
									 Cell_X(Coord_Cell(Coord)), Cell_Y(Coord_Cell(Coord)));
			}
		}
		Assign_Destination(TARGET_NONE);
		if (slowdown) {
			Set_Speed(0);
		}
		return(0);
	}

	int distance = Distance(coord);

	PrimaryFacing.Set_Desired(Direction(coord));

	if (slowdown) {
		int speed = MIN(distance, 0x0300);
		speed = Bound(speed/3, 0x0020, 0x00FF);
		if (Speed != speed) {
			Set_Speed(speed);
		}
	}

	if (distance < 0x0010) {
		if (slowdown) {
			Set_Speed(0);
		}
		distance = 0;
	}
	return(distance);
}


#ifdef CHEAT_KEYS
/***********************************************************************************************
 * AircraftClass::Debug_Dump -- Displays the status of the aircraft to the mono monitor.       *
 *                                                                                             *
 *    This displays the current status of the aircraft class to the mono monitor. By this      *
 *    display bugs may be tracked down or prevented.                                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/02/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::Debug_Dump(MonoClass *mono) const
{
	Validate();
	mono->Set_Cursor(0, 0);
	mono->Print(
		"�Name:���������������Mission:����TarCom:�NavCom:�Radio:�Coord:���Altitude�St:Ŀ\n"
		"�                   �           �       �       �      �        �        �    �\n"
		"����������������N�Y�Health:��Fdir:��Bdir:��Speed:��������������Cargo:��������Ĵ\n"
		"�Active........� � �        �     �       �      �            �               �\n"
		"�Limbo.........� � ����������������������������������������������������������Ĵ\n"
		"�Owned.........� � �Last Message:                                             �\n"
		"�Discovered....� � �Timer:�Arm:������������������Flash:�Stage:�Team:�����Arch:�\n"
		"�Selected......� � �      �    �      �         �      �      �         �     �\n"
		"�Teathered.....� � ������������������������������������������������������������\n"
		"�Locked on Map.� � �                                                           \n"
		"�              � � �                                                           \n"
		"�Is A Loaner...� � �                                                           \n"
		"�Is Landing....� � �                                                           \n"
		"�Is Taking Off.� � �                                                           \n"
		"�              � � �                                                           \n"
		"�              � � �                                                           \n"
		"�              � � �                                                           \n"
		"�Recoiling.....� � �                                                           \n"
		"�To Display....� � �                                                           \n"
		"��������������������                                                           \n");
	mono->Set_Cursor(1, 1);mono->Printf("%s:%s", House->Class->IniName, Class->IniName);
	mono->Set_Cursor(36, 3);mono->Printf("%02X:%02X", SecondaryFacing.Current(), SecondaryFacing.Desired());
	mono->Set_Cursor(42, 1);mono->Printf("%04X", NavCom);
	mono->Set_Cursor(66, 1);mono->Printf("%d", Altitude);
	mono->Set_Cursor(44, 3);mono->Printf("%d", Get_Speed());
	mono->Text_Print("X", 16 + (IsLanding?2:0), 12);
	mono->Text_Print("X", 16 + (IsTakingOff?2:0), 13);
	FootClass::Debug_Dump(mono);
}
#endif


/***********************************************************************************************
 * AircraftClass::Active_Click_With -- Handles clicking over specified object.                 *
 *                                                                                             *
 *    This routine is used when the player clicks over the speicifed object. It will assign    *
 *    the appropriate mission to the aircraft.                                                 *
 *                                                                                             *
 * INPUT:   action   -- The action that was nominally determined by the What_Action function.  *
 *                                                                                             *
 *          object   -- The object over which the mouse was clicked.                           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This routine will alter the game sequence and causes an event packet to be      *
 *             propogated to all connected machines.                                           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::Active_Click_With(ActionType action, ObjectClass * object)
{
	Validate();
	switch (action) {
		case ACTION_ENTER:
			Player_Assign_Mission(MISSION_ENTER, TARGET_NONE, object->As_Target());
			break;

		case ACTION_SELF:
			Player_Assign_Mission(MISSION_UNLOAD, TARGET_NONE, TARGET_NONE);
			break;

		default:
			break;
	}
	FootClass::Active_Click_With(action, object);
}


/***********************************************************************************************
 * AircraftClass::Active_Click_With -- Handles clicking over specified cell.                   *
 *                                                                                             *
 *    This routine is used when the player clicks the mouse of the specified cell. It will     *
 *    assign the appropriate mission to the aircraft.                                          *
 *                                                                                             *
 * INPUT:   action   -- The action nominally determined by What_Action().                      *
 *                                                                                             *
 *          cell     -- The cell over which the mouse was clicked.                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This routine will affect the game sequence and causes an event object to be     *
 *             propogated to all connected machines.                                           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::Active_Click_With(ActionType action, CELL cell)
{
	Validate();
#ifdef NEVER
	switch (action) {
		case ACTION_MOVE:
			if (Map[cell].IsVisible) {
				Player_Assign_Mission(MISSION_MOVE, TARGET_NONE, ::As_Target(cell));
			}
			break;

		case ACTION_NOMOVE:
			break;

		case ACTION_ATTACK:
			Player_Assign_Mission(MISSION_ATTACK, ::As_Target(cell));
			break;
	}
#endif
	FootClass::Active_Click_With(action, cell);
}


/***********************************************************************************************
 * AircraftClass::Player_Assign_Mission -- Handles player input to assign a mission.           *
 *                                                                                             *
 *    This routine is called as a result of player input with the intent to change the         *
 *    mission of the aircraft.                                                                 *
 *                                                                                             *
 * INPUT:   mission  -- The mission requested of the aircraft.                                 *
 *                                                                                             *
 *          target   -- The value to assign to the aircraft's targeting computer.              *
 *                                                                                             *
 *          dest.    -- The value to assign to the aircraft's navigation computer.             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The mission specified will be executed at an indeterminate future game frame.   *
 *             This is controlled by net/modem propogation delay.                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::Player_Assign_Mission(MissionType mission, TARGET target, TARGET destination)
{
	Validate();
	if (AllowVoice) {
		if (mission == MISSION_ATTACK) {
			Response_Attack();
		} else {
			Response_Move();
		}
	}
	Queue_Mission(As_Target(), mission, target, destination);
}


/***********************************************************************************************
 * AircraftClass::What_Action -- Determines what action to perform.                            *
 *                                                                                             *
 *    This routine is used to determine what action will likely be performed if the mouse      *
 *    were clicked over the object specified. The display system calls this routine to         *
 *    control the mouse shape.                                                                 *
 *                                                                                             *
 * INPUT:   target   -- Pointer to the object that the mouse is currently over.                *
 *                                                                                             *
 * OUTPUT:  Returns with the action that will occur if the mouse were clicked over the         *
 *          object specified.                                                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ActionType AircraftClass::What_Action(ObjectClass * target) const
{
	Validate();
	ActionType action = FootClass::What_Action(target);

	if (action == ACTION_SELF && (!How_Many() || (Altitude > 0) || IsTethered)) {
		action = ACTION_NONE;
	}

	if (action == ACTION_ATTACK && Class->Primary == WEAPON_NONE) {
		action = ACTION_NONE;
	}

	//Changed for multiplayer ST - 3/13/2019 5:31PM
	//if (IsOwnedByPlayer && House->Is_Ally(target) && target->What_Am_I() == RTTI_BUILDING && ((AircraftClass *)this)->Transmit_Message(RADIO_CAN_LOAD, (TechnoClass*)target) == RADIO_ROGER) {
	if (Is_Owned_By_Player() && House->Is_Ally(target) && target->What_Am_I() == RTTI_BUILDING && ((AircraftClass *)this)->Transmit_Message(RADIO_CAN_LOAD, (TechnoClass*)target) == RADIO_ROGER) {
		action = ACTION_ENTER;
	}
	return(action);
}


/***********************************************************************************************
 * AircraftClass::What_Action -- Determines what action to perform.                            *
 *                                                                                             *
 *    This routine will determine what action would occur if the mouse were clicked over the   *
 *    cell specified. The display system calls this routine to determine what mouse shape      *
 *    to use.                                                                                  *
 *                                                                                             *
 * INPUT:   cell  -- The cell over which the mouse is currently positioned.                    *
 *                                                                                             *
 * OUTPUT:  Returns with the action that will be performed if the mouse were clicked at the    *
 *          specified cell location.                                                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ActionType AircraftClass::What_Action(CELL cell) const
{
	Validate();
	ActionType action = FootClass::What_Action(cell);

	/*
	**	A CrystAlien flier goes where it is sent, seen or not.
	**
	**	Tiberian Dawn will not let a helicopter be ordered to a cell the player
	**	cannot currently see -- not merely unexplored, but not under anyone's
	**	sight this instant.  CrystAlien has no such rule.  Control.doCursor reads
	**	the shroud once:
	**
	**	    if(shroud.active && shroud.MC["shroud_" + x + "_" + y]._currentframe == 1)
	**	        return undefined;
	**
	**	-- and all that skips is the search for something to click on.  cursorState
	**	is left at "standard", which is the move cursor, and the order goes out.
	**	You cannot target what you cannot see; you can always go there.
	**
	**	This is the whole of the reported movement trouble.  Every unit named --
	**	Hyper Carrier, Recon Dropship, Switch Fighter, Alien Infiltrator, Astro
	**	Fighter -- is an aircraft, and no ground unit was ever affected, because
	**	TechnoClass::What_Action has no such test.  "Refuses to move more than a
	**	few inches" is the craft accepting orders inside its own sight and
	**	refusing everything past it.
	**
	**	It got worse the day the shroud learned to come back.  While the uplink
	**	revealed the map permanently the whole board stayed visible and the gate
	**	never bit; once cells could close again it bit constantly.
	*/
	bool cac = (Class != NULL) && CAC_Is_CrystAlien_Type(Class->IniName);

	//using function for IsVisible so we have different results for different players - JAS 2019/09/30
	if (!cac && (action == ACTION_MOVE || action == ACTION_ATTACK) &&
		!Map[cell].Is_Visible(PlayerPtr)) {
		action = ACTION_NOMOVE;
	}

	if (action == ACTION_ATTACK && Class->Primary == WEAPON_NONE) {
		action = ACTION_NONE;
	}
	return(action);
}


/***********************************************************************************************
 * AircraftClass::Pose_Dir -- Fetches the natural landing facing.                              *
 *                                                                                             *
 *    Use this routine to get the desired facing the aircraft should assume when landing.      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the normal default facing the aircraft should have when landed.       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
DirType AircraftClass::Pose_Dir(void) const
{
	Validate();
	if (*this == AIRCRAFT_TRANSPORT) {
		return(DIR_N);
	}
	return(DIR_NE);
}


/***********************************************************************************************
 * AircraftClass::Mission_Attack -- Handles the attack mission for aircraft.                   *
 *                                                                                             *
 *    This routine is the state machine that handles the attack mission for aircraft. It will  *
 *    handling homing in on and firing on the target in the aircraft's targeting computer.     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of game ticks to pass before this routine must be called   *
 *          again.                                                                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int AircraftClass::Mission_Attack(void)
{
	Validate();
	if (Class->IsFixedWing) {
		Assign_Mission(MISSION_HUNT);
		return(1);
	}

	/*
	**	Ours shoots from where it is.
	**
	**	The state machine below is an attack run: PICK_ATTACK_LOCATION asks
	**	Good_Fire_Location for a spot, TAKE_OFF and FLY_TO_POSITION fly to it, and
	**	only `distance < 0x0010` -- arrival -- reaches FIRE_AT_TARGET.  Move the
	**	target, or let the location be picked again, and the craft sets off for a
	**	new spot without having fired.  That is the Switch Fighter circling its
	**	target instead of shooting it.
	**
	**	CrystAlien has no attack run and no firing position.  engageEnemy is called
	**	from Unit.think every frame, tests one distance, and fires:
	**
	**	    _loc5_ = distance(this.posX, this.posY, _loc2_.posX, _loc2_.posY);
	**	    if (_loc5_ < this.stats.weaponRange) { target = _loc2_; ... }
	**	    ...
	**	    this.shoot(target);
	**
	**	and shoot() assigns the angle in the same frame it fires, so there is
	**	nothing to line up and nowhere to be.  The navigation is a separate matter
	**	-- UnitNav re-paths at the target on its own cadence and the craft shoots
	**	on the way.
	**
	**	So: fire whenever the weapon is ready, and close only if the target is out
	**	of reach.  Which leaves the transformer doing what it is for -- it may fire
	**	while landing and landed (CAC_Holds_Fire_Landing exempts it), so it settles
	**	where it stopped and keeps shooting.
	**
	**	Boomerangs are excluded: a Fighter's whole engagement is CAC_AI_Sortie out,
	**	CAC_Strafe on the way past, and CAC_Return_Home when the magazine is dry.
	*/
	if (Class != NULL && CAC_Is_CrystAlien_Type(Class->IniName) &&
		 !CAC_Is_Boomerang(*this)) {

		if (!Target_Legal(TarCom)) {
			Enter_Idle_Mode();
			return(1);
		}

		/*
		**	And nothing off the edge of the world.
		**
		**	This branch closes on anything it cannot reach -- `Assign_Destination
		**	(::As_Target(::As_Cell(TarCom)))` and fly -- which is right for a target
		**	on the map and is a one-way trip for one that is not.  The craft flies at
		**	the border, CAC_Contain_Flier puts it back on the last legal cell, the
		**	target is still legal so it sets off again, and the log of it is a
		**	Transformer crossing 50,57 to 60,58 a cell a second for as long as anyone
		**	watches:
		**
		**	    UCEVIL (player) on mission 1 at 53,58 -- nav none, target set
		**	    UCEVIL (player) left the map at 60,58 on mission 4 -- put back at 59,58
		**
		**	It is also why a Switch Fighter given an order seems to abandon it after a
		**	few shots: containment clears the orders to stop the drift, the craft
		**	drops to guard, and guard picks up whatever is nearest -- so the player's
		**	target is quietly replaced by one he did not choose and has to be given
		**	again.  Both reports are this.
		**
		**	A target off the playable rectangle is not a target.  Dropping it here
		**	means the craft goes idle where it is instead of walking into the border.
		*/
		if (!CAC_On_Map(::As_Cell(TarCom))) {
			CAC_Log_Volatile("drift", "%s: target at %d,%d is off the map -- dropped",
								  Class->IniName,
								  Cell_X(::As_Cell(TarCom)), Cell_Y(::As_Cell(TarCom)));
			Assign_Target(TARGET_NONE);
			Enter_Idle_Mode();
			return(1);
		}

		/*
		**	Point at it, because for a flier that is what is drawn.
		**
		**	Draw_It takes the shape from `Facing_To_32(SecondaryFacing)`, and the
		**	attack run this replaced set it every frame -- `SecondaryFacing.
		**	Set_Desired(Direction(TarCom))` on the way in and again at the target.
		**	Dropping the run dropped that with it, so a craft flew whatever way it
		**	happened to be pointing: sideways, or backwards.
		**
		**	Facing the target while it can shoot, and the way it is going while it
		**	closes, which is what the states it replaced each did.
		*/
		if (In_Range(TarCom, 0)) {
			SecondaryFacing.Set_Desired(Direction(TarCom));
		} else if (Target_Legal(NavCom)) {
			SecondaryFacing.Set_Desired(::Direction(Fire_Coord(0), As_Coord(NavCom)));
		}

		if (Can_Fire(TarCom, 0) == FIRE_OK) {
			Fire_At(TarCom, 0);
			Map[::As_Cell(TarCom)].Incoming(Coord, true);
		}

		/*
		**	Out of reach is the only reason to move -- and the craft has to actually
		**	be flown.
		**
		**	This assigned the destination and returned, which is what the state
		**	machine it replaced never had to worry about: FLY_TO_POSITION calls
		**	Process_Fly_To every frame, and that is what sets the facing and the
		**	speed.  Without it the craft sat at speed nought holding a destination
		**	twenty-five cells away, and the descent gate did the only thing it could
		**	with a craft that is not moving -- counted twenty frames and set it down.
		**
		**	    UPGOOD settles at 52,8 (altitude 24, speed 0, still 21, mission 1,
		**	      status 0, destination 32,20 at 6584, house player)
		**
		**	Which is the whole of "Switch Fighters seat after being commanded", and
		**	it was mine: the attack run went out and nothing replaced the flying.
		*/
		if (!In_Range(TarCom, 0)) {
			if (!Target_Legal(NavCom)) {

				/*
				**	Fly to where it can shoot from, not to the target itself.
				**
				**	Handing over the target's own cell tells the craft to arrive on
				**	top of it, and an aircraft turns far more slowly than it flies:
				**	it reaches weapon range, keeps going because the destination is
				**	still ahead, and sails past.  Near a border that means off the
				**	map, and CAC_Contain_Flier hauls it back -- over and over, which
				**	from outside is a flier that will not stop drifting.  The log of
				**	it is a Transformer at 31,34 aimed at 36,4 and leaving the map at
				**	35,0, five cells beyond what it was flying at.
				**
				**	So the destination is a point short of the target, along the line
				**	from here: three quarters of weapon range, which puts the craft
				**	comfortably inside its own reach with room to stop.  The branch
				**	below then clears the destination and holds it there.
				**
				**	Falls back to the target's cell if the weapon has no range worth
				**	the name, which is the behaviour this replaces.
				*/
				COORDINATE tc = ::As_Coord(TarCom);
				int range = Weapon_Range(0);
				CELL want = ::As_Cell(TarCom);

				if (range > 0 && ::Distance(Center_Coord(), tc) > range) {
					COORDINATE stand = Coord_Move(tc, ::Direction(tc, Center_Coord()),
															(unsigned short)((range * 3) / 4));
					CELL standcell = Coord_Cell(stand);
					if (CAC_On_Map(standcell)) want = standcell;
				}

				Assign_Destination(::As_Target(want));
			}
			Process_Fly_To(true);
		} else {
			/*
			**	In range is far enough.  Hold and shoot -- and for a lander that
			**	means the descent gate settles it where it stands, which is the
			**	Switch Fighter fighting from the ground as it is meant to.
			*/
			if (Target_Legal(NavCom)) {
				Assign_Destination(TARGET_NONE);
			}
			if (Get_Speed() != MPH_IMMOBILE) Set_Speed(0);
		}

		return(1);
	}

	enum {
		VALIDATE_AZ,
		PICK_ATTACK_LOCATION,
		TAKE_OFF,
		FLY_TO_POSITION,
		FIRE_AT_TARGET,
		FIRE_AT_TARGET2,
		RETURN_TO_BASE
	};
	switch (Status) {

		/*
		**	Double check target and validate the attack zone.
		*/
		case VALIDATE_AZ:
			if (!Target_Legal(TarCom)) {
				Status = RETURN_TO_BASE;
			} else {
				Status = PICK_ATTACK_LOCATION;
			}
			break;

		/*
		**	Pick a good location to attack from.
		*/
		case PICK_ATTACK_LOCATION:
			if (!Target_Legal(TarCom)) {
				Status = RETURN_TO_BASE;
			} else {
				Assign_Destination(Good_Fire_Location(TarCom));
				if (Target_Legal(NavCom)) {
					Status = TAKE_OFF;
				} else {
					Status = RETURN_TO_BASE;
				}
			}
			break;

		/*
		**	Take off (if necessary).
		*/
		case TAKE_OFF:
			if (!Target_Legal(TarCom)) {
				Status = RETURN_TO_BASE;
			} else {
				if (Process_Take_Off()) {
					Status = FLY_TO_POSITION;

					/*
					**	Break off radio contact with the helipad it is taking off from.
					*/
					if (In_Radio_Contact() && Map[Coord_Cell(Coord)].Cell_Building() == Contact_With_Whom()) {
						Transmit_Message(RADIO_OVER_OUT);
					}

					/*
					**	Start flying toward the destination by skewing at first.
					**	As the flight progresses, the body will rotate to face
					**	the direction of travel.
					*/
					int diff = SecondaryFacing.Difference(Direction(NavCom));
					diff = Bound(diff, -128, 128);
					PrimaryFacing = SecondaryFacing.Current()+diff;
				}
				return(1);
			}
			break;

		/*
		**	Fly to attack location.
		*/
		case FLY_TO_POSITION:
			if (Target_Legal(TarCom)) {

				/*
				**	If the navcom was cleared mysteriously, then try to pick
				**	a new attack location. This is a likely event if the player
				**	clicks on a new target while in flight to an existing target.
				*/
				if (!Target_Legal(NavCom)) {
					Status = PICK_ATTACK_LOCATION;
					return(1);
				}

				int distance = Process_Fly_To(true);

				if (distance < 0x0200) {
					SecondaryFacing.Set_Desired(Direction(TarCom));

					if (distance < 0x0010) {
						Status = FIRE_AT_TARGET;
						Assign_Destination(TARGET_NONE);
					}
				} else {
					SecondaryFacing.Set_Desired(::Direction(Fire_Coord(0), As_Coord(NavCom)));
					return(1);
				}
			} else {
				Status = RETURN_TO_BASE;
			}
			return(1);

		/*
		**	Fire at the target.
		*/
		case FIRE_AT_TARGET:
			if (!Target_Legal(TarCom)) {
				Status = RETURN_TO_BASE;
				return(1);
			}

			/*
			**	Clear second shot flag so fire burst works correctly.
			*/
			IsSecondShot = false;

			PrimaryFacing.Set_Desired(Direction(TarCom));
			SecondaryFacing.Set_Desired(Direction(TarCom));
			switch (Can_Fire(TarCom, 0)) {
				case FIRE_CLOAKED:
					Do_Uncloak();
					break;

				case FIRE_OK:
					Fire_At(TarCom, 0);
					Map[::As_Cell(TarCom)].Incoming(Coord, true);
					Status = FIRE_AT_TARGET2;
					break;

				default:
					if (!Ammo) {
						Status = RETURN_TO_BASE;
					} else {
						Status = FIRE_AT_TARGET2;
					}
					break;
			}
			return(1);

		/*
		**	Fire at the target.
		*/
		case FIRE_AT_TARGET2:
			if (!Target_Legal(TarCom)) {
				Status = RETURN_TO_BASE;
				return(1);
			}

			PrimaryFacing.Set_Desired(Direction(TarCom));
			SecondaryFacing.Set_Desired(Direction(TarCom));
			switch (Can_Fire(TarCom, 0)) {
				case FIRE_CLOAKED:
					Do_Uncloak();
					break;

				case FIRE_REARM:
					break;

				case FIRE_OK:
					Fire_At(TarCom, 0);
					Map[::As_Cell(TarCom)].Incoming(Coord, true);

					if (Ammo) {
						Status = PICK_ATTACK_LOCATION;
					} else {
						Status = RETURN_TO_BASE;
					}
					break;

				default:
					if (!Ammo) {
						Status = RETURN_TO_BASE;
					} else {
						Status = PICK_ATTACK_LOCATION;
					}
					break;
			}
			break;

		/*
		**	Fly back to landing spot.
		*/
		case RETURN_TO_BASE:
			Assign_Destination(TARGET_NONE);
			Enter_Idle_Mode();
			break;
	}

	return(TICKS_PER_SECOND/2);
}


/***********************************************************************************************
 * AircraftClass::New_LZ -- Find a good landing zone.                                          *
 *                                                                                             *
 *    Use this routine to locate a good landing zone that is nearby the location specified.    *
 *    By using this routine it is possible to assign the same landing zone to several          *
 *    aircraft and they will land nearby without conflict.                                     *
 *                                                                                             *
 * INPUT:   oldlz -- Target value of desired landing zone (usually a cell target value).       *
 *                                                                                             *
 * OUTPUT:  Returns with the new good landing zone. It might be the same value passed in.      *
 *                                                                                             *
 * WARNINGS:   The landing zone might be a goodly distance away from the ideal if there is     *
 *             extensive blocking terrain in the vicinity.                                     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
TARGET AircraftClass::New_LZ(TARGET oldlz, bool stable) const
{
	Validate();
	if (Target_Legal(oldlz) && (!Is_LZ_Clear(oldlz) || !Cell_Seems_Ok(As_Cell(oldlz)))) {
		COORDINATE coord = As_Coord(oldlz);

		/*
		**	Scan outward in a series of concentric rings up to certain distance
		**	in cells.
		*/
		for (int radius = 0; radius < 16; radius++) {
			FacingType modifier = stable ? FACING_N : Random_Pick(FACING_N, FACING_NW);
			CELL lastcell = -1;

			/*
			**	Perform a radius scan out from the original center location. Try to
			**	find a cell that is allowed to be a legal LZ.
			*/
			for (FacingType facing = FACING_N; facing < FACING_COUNT; facing++) {
				CELL newcell = Coord_Cell(Coord_Move(coord, Facing_Dir(facing+modifier), radius * ICON_LEPTON_W));
				if (Map.In_Radar(newcell)) {
					TARGET newtarget = ::As_Target(newcell);

					if (newcell != lastcell && Is_LZ_Clear(newtarget) && Cell_Seems_Ok(newcell)) {
						return(newtarget);
					}
					lastcell = newcell;
				}
			}
		}
	}
	return(oldlz);
}


/***********************************************************************************************
 * AircraftClass::Fire_Coord -- Calculates the point of origin for a bullet.                   *
 *                                                                                             *
 *    This routine is used to find the exact coordinate where the bullet should appear if      *
 *    fired from this object.                                                                  *
 *                                                                                             *
 * INPUT:   which -- Which weapon to consider.                                                 *
 *                                                                                             *
 * OUTPUT:  Returns with the coordinate of where the projectile will appear if fired.          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/15/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
COORDINATE AircraftClass::Fire_Coord(int ) const
{
	Validate();
	return(Coord_Move(Coord_Add(XYP_Coord(0, -Altitude), Coord), SecondaryFacing, 0x040));
}


COORDINATE AircraftClass::Target_Coord(void) const
{
	Validate();
	return(Coord_Add(XYP_Coord(0, -Altitude), Coord));
}


/***********************************************************************************************
 * AircraftClass::Receive_Message -- Handles receipt of radio messages.                        *
 *                                                                                             *
 *    This routine receives all radio messages directed at this aircraft. It is used to handle *
 *    all inter-object coordination. Typically, this would be for transport helicopters and    *
 *    other complex landing operations required of helicopters.                                *
 *                                                                                             *
 * INPUT:   from     -- The source of this radio message.                                      *
 *                                                                                             *
 *          message  -- The message itself.                                                    *
 *                                                                                             *
 *          param    -- An optional parameter that may be used to transfer additional          *
 *                      data.                                                                  *
 *                                                                                             *
 * OUTPUT:  Returns with the radio response from the aircraft.                                 *
 *                                                                                             *
 * WARNINGS:   Some radio messages are handled by the base classes.                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
RadioMessageType AircraftClass::Receive_Message(RadioClass * from, RadioMessageType message, long & param)
{
	Validate();
	switch (message) {

		case RADIO_PREPARED:
			if (Target_Legal(TarCom)) return(RADIO_NEGATIVE);
			if ((Altitude == 0 && Ammo == Class->MaxAmmo) || (Altitude > 0 && Ammo > 0)) return(RADIO_ROGER);
			return(RADIO_NEGATIVE);

		/*
		**	Something disasterous has happened to the object in contact with. Fall back
		**	and regroup. This means that any landing process is immediately aborted.
		*/
		case RADIO_RUN_AWAY:
			if (IsLanding) {
				IsLanding = false;
				IsTakingOff = true;
			}
			Scatter(0, true);
			break;

		/*
		**	The ground control requests that this specified landing spot be used.
		*/
		case RADIO_MOVE_HERE:
			FootClass::Receive_Message(from, message, param);
			if (Is_Target_Building(param)) {
				if (Transmit_Message(RADIO_CAN_LOAD, As_Techno(param)) != RADIO_ROGER) {
					return(RADIO_NEGATIVE);
				}
				Assign_Mission(MISSION_ENTER);
				Assign_Destination((TARGET)param);
			} else {
				Assign_Mission(MISSION_MOVE);
				Assign_Destination((TARGET)param);
			}
			Commence();
			return(RADIO_ROGER);

		/*
		**	Ground control is requesting if the aircraft requires navigation direction.
		*/
		case RADIO_NEED_TO_MOVE:
			FootClass::Receive_Message(from, message, param);
			if (!Target_Legal(NavCom) && !IsTakingOff && !IsLanding) {
				return(RADIO_ROGER);
			}
			return(RADIO_NEGATIVE);

		/*
		**	This message is sent by the passenger when it determines that it has
		**	entered the transport.
		*/
		case RADIO_IM_IN:
			/*
			**	Down first.  This is the half of the question that belongs here.
			**
			**	CrystAlien asks two different things at two different moments.  To
			**	offer the order at all it wants only a friendly carrier with room --
			**
			**	    if(this.activeTarget.stats.carrier &&
			**	       this.activeTarget.stats.contents.length < this.activeTarget.stats.capacity)
			**
			**	-- and it says nothing about altitude, which is why a Recon Dropship
			**	can be told to expect passengers while it is still in the air.  The
			**	altitude test is in checkNonDestructiveTarget, where the passenger has
			**	arrived and is standing there:
			**
			**	    if(this.target.stats.carrier && this.target.team == this.parent.team &&
			**	       (!this.target.stats.flying || this.target.nav.landerPerc == 100))
			**
			**	Both were on the offer here, so the cursor showed no-entry over
			**	anything airborne and the order could not be given at all.  Moved: the
			**	offer asks about room, arrival asks about altitude.  A passenger that
			**	gets here early simply is not taken aboard this tick and asks again,
			**	which is the waiting CrystAlien does.
			**
			**	Tiberian Dawn's own transports keep the old arrangement.
			*/
			if (CAC_Is_CrystAlien_Type(Class->IniName) && Altitude != 0) {
				return(RADIO_NEGATIVE);
			}

			/*	And never past capacity: the stock radio let only one in at a time. */
			if (How_Many() >= Class->Max_Passengers()) {
				return(RADIO_NEGATIVE);
			}

			if (How_Many() == Class->Max_Passengers()) {
				Close_Door(5, 4);
			}

			/*
			**	If a civilian has entered the transport, then the transport will immediately
			**	fly off the map.
			*/
			if (from->What_Am_I() == RTTI_INFANTRY && ((InfantryClass *)from)->Class->IsCivilian && !((InfantryClass *)from)->IsTechnician) {
				Assign_Mission(MISSION_RETREAT);
			}
			return(RADIO_ATTACH);

		/*
		**	Docking maintenance message received. Check to see if new orders should be given
		**	to the impatient unit.
		*/
		case RADIO_DOCKING:
			if (Class->IsTransporter && How_Many() < Class->Max_Passengers()) {
				FootClass::Receive_Message(from, message, param);

				/*
				**	Ours is boarded from whichever side the walk happens to end on.
				**
				**	The stock sequence sends the passenger to a staging cell first and
				**	only then onto the transport, and the cell comes from
				**	Desired_Load_Dir, which sweeps outward from FACING_S:
				**
				**	    for (int sweep = FACING_N; sweep < FACING_S; sweep++) {
				**	        moveto = Adjacent_Cell(center, FACING_S+sweep);
				**	        ...
				**
				**	-- so the south side is tried first and the passenger walks around
				**	the craft to reach it.  That is "they try to walk in from a specific
				**	angle".  Worse, the loop leaves `moveto` holding the last cell it
				**	tried whether or not that cell passed, and the caller only checks
				**	for nought, so a crowded transport can send a passenger to a cell it
				**	cannot stand in -- one of the ways a load attempt quietly fails.
				**
				**	CrystAlien has no staging cell and no load direction.  UnitNav sends
				**	the boarder at the carrier itself and unblocks the carrier's tile so
				**	it can be walked onto:
				**
				**	    if (this.stats.repair || this.target.friend && this.target.stats.carrier) {
				**	       this.target.freeBlock();
				**	    } else { ...closestAvailable... }
				**	    ...
				**	    this.voyage(_loc4_, _loc3_, false);
				**
				**	and doEmbark runs the moment the passenger is standing on that tile,
				**	from any direction, with no handshake to fail.
				**
				**	Also reachable while the craft is still coming down.  The stock gate
				**	is `Altitude == 0`, and a Hyper Carrier only settles after twenty
				**	idle frames, so every attempt made during the descent did nothing
				**	and returned RADIO_ROGER -- which is the "sometimes takes several
				**	attempts".  CrystAlien asks about landing at the moment of boarding
				**	rather than the moment of asking:  `(!this.target.stats.flying ||
				**	this.target.nav.landerPerc == 100)`.
				*/
				bool cacload = (Class != NULL) && CAC_Is_CrystAlien_Type(Class->IniName);

				/*
				**	Nothing more than "is there room".  The tether and staging move that
				**	were here cut the passenger's contact at its first step, left this
				**	craft tethered to nobody, and blocked every boarder after the first;
				**	the passenger now boards on arrival (CAC_Try_Embark).
				*/
				if (cacload) {
					return(RADIO_ROGER);
				}

				if (!IsTethered && !IsLanding && !IsTakingOff && Altitude == 0) {

					Open_Door(5, 4);

					/*
					**	If the potential passenger needs someplace to go, then figure out a good
					**	spot and tell it to go.
					*/
					if (Transmit_Message(RADIO_NEED_TO_MOVE, from) == RADIO_ROGER) {
						CELL cell;
						DirType dir = Desired_Load_Dir(from, cell);

						/*
						**	If no adjacent free cells are detected, then passenger loading
						**	cannot occur. Break radio contact.
						*/
						if (cell == 0) {
							Transmit_Message(RADIO_OVER_OUT, from);
						} else {
							param = (long)::As_Target(cell);

							/*
							**	Tell the potential passenger where it should go. If the passenger is
							**	already at the staging location, then tell it to move onto the transport
							**	directly.
							*/
							if (Transmit_Message(RADIO_MOVE_HERE, param, from) == RADIO_YEA_NOW_WHAT) {
								param = (long)As_Target();
								Transmit_Message(RADIO_TETHER);
								if (Transmit_Message(RADIO_MOVE_HERE, param, from) != RADIO_ROGER) {
									Transmit_Message(RADIO_OVER_OUT, from);
								} else {
									Contact_With_Whom()->Unselect();
								}
							}
						}
					}
				}
				return(RADIO_ROGER);
			}
			break;

		/*
		**	Asks if the passenger can load on this transport.
		*/
		case RADIO_CAN_LOAD:
			/*
			**	One at a time is Tiberian Dawn's rule, not CrystAlien's.
			**
			**	`!In_Radio_Contact()` means the transport will only entertain a boarding
			**	request while it is talking to nobody, and What_Action asks this question
			**	to decide whether to offer the order at all.  So the first trooper sent
			**	aboard takes the radio and every one after it is refused -- not queued,
			**	refused, with the no-entry cursor -- until that one is inside.  That is
			**	the Dropship that will not take a squad.
			**
			**	Note the ground carrier has no such term (UNIT.CPP), which is why the
			**	Crystal Reaper loads a group and the Dropship does not.
			**
			**	CrystAlien asks two things and neither is about a radio:
			**
			**	    if(this.activeTarget.stats.carrier &&
			**	       this.activeTarget.stats.contents.length < this.activeTarget.stats.capacity)
			**
			**	and then sets `target = carrier` on *every* selected unit that is
			**	carriable.  They walk over and embark as they arrive, and doEmbark
			**	re-checks capacity for each one, so the last one turns away rather than
			**	the second one never being asked.
			**
			**	Dropping the term is safe because the queue below already works: this
			**	handler's RADIO_DOCKING case answers ROGER to any passenger while there
			**	is room, tethered elsewhere or not, and Mission_Enter keeps a passenger
			**	on task as long as it gets ROGER.  Only the offer was missing.
			**
			**	Capacity, ownership and Altitude == 0 all stay -- that last one is
			**	`!flying || landerPerc == 100`, which CrystAlien tests too.
			*/
			if ((!In_Radio_Contact() || CAC_Is_CrystAlien_Type(Class->IniName)) &&
				Class->IsTransporter && How_Many() < Class->Max_Passengers() &&
				from && House->Class->House == from->Owner() &&
				(Altitude == 0 || CAC_Is_CrystAlien_Type(Class->IniName))) {
				return(RADIO_ROGER);
			}
			return(RADIO_NEGATIVE);
	}

	/*
	**	Let the base class take over processing this message.
	*/
	return(FootClass::Receive_Message(from, message, param));
}


/***********************************************************************************************
 * AircraftClass::Desired_Load_Dir -- Determines where passengers should line up.              *
 *                                                                                             *
 *    This routine is used by the transport helicopter to determine the location where the     *
 *    infantry passengers should line up before loading.                                       *
 *                                                                                             *
 * INPUT:   object   -- The object that is trying to load up on this transport.                *
 *                                                                                             *
 *                   -- Reference to the cell that the passengers should move to before the    *
 *                      actual load process may begin.                                         *
 *                                                                                             *
 * OUTPUT:  Returns with the direction that the helicopter should face for the load operation. *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/12/1995 JLB : Created.                                                                 *
 *   07/30/1995 JLB : Revamped to scan all adjacent cells.                                     *
 *=============================================================================================*/
DirType AircraftClass::Desired_Load_Dir(ObjectClass * object, CELL & moveto) const
{
	Validate();
	CELL center = Coord_Cell(Center_Coord());
	for (int sweep = FACING_N; sweep < FACING_S; sweep++) {
		moveto = Adjacent_Cell(center, FACING_S+sweep);
		if (Map.In_Radar(moveto) && (Coord_Cell(object->Center_Coord()) == moveto || Map[moveto].Is_Generally_Clear())) return(DIR_N);

		moveto = Adjacent_Cell(center, FACING_S-sweep);
		if (Map.In_Radar(moveto) && (Coord_Cell(object->Center_Coord()) == moveto || Map[moveto].Is_Generally_Clear())) return(DIR_N);
	}
	return(DIR_N);
}


/***********************************************************************************************
 * AircraftClass::Process_Take_Off -- State machine support for taking off.                    *
 *                                                                                             *
 *    This routine is used by the main game state machine processor. This utility routine      *
 *    handles a helicopter as it transitions from landed to flying state.                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Has the helicopter reached flight level now?                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/12/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool AircraftClass::Process_Take_Off(void)
{
	Validate();
	IsLanding = false;
	IsTakingOff = true;
	switch (Altitude) {
		case 0:
			Close_Door(5, 4);
			PrimaryFacing = SecondaryFacing;
			break;

		case FLIGHT_LEVEL/2:
			PrimaryFacing.Set_Desired(Direction(NavCom));
			break;

		case FLIGHT_LEVEL-(FLIGHT_LEVEL/3):
			SecondaryFacing.Set_Desired(PrimaryFacing.Desired());
			Set_Speed(0x20);
			break;

		case FLIGHT_LEVEL-(FLIGHT_LEVEL/5):
			Set_Speed(0x40);
			break;

		case FLIGHT_LEVEL:
			Set_Speed(0xFF);
			IsTakingOff = false;
			return(true);
	}
	return(false);
}


/***********************************************************************************************
 * AircraftClass::Process_Landing -- Landing process state machine handler.                    *
 *                                                                                             *
 *    This is a support routine that is called by the main state machine routines. This        *
 *    routine is responsible for handling the helicopter as it transitions from flight to      *
 *    landing.                                                                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Has the helicopter completely landed now?                                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/12/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool AircraftClass::Process_Landing(void)
{
	Validate();
	IsTakingOff = false;
	IsLanding = true;
	switch (Altitude) {
		case 0:
			IsLanding = false;
			return(true);

		case FLIGHT_LEVEL/2:
			Set_Speed(0);
			break;

		case FLIGHT_LEVEL:
			break;
	}
	return(false);
}


/***********************************************************************************************
 * AircraftClass::Can_Enter_Cell -- Determines if the aircraft can land at this location.      *
 *                                                                                             *
 *    This routine is used when the passability of a cell needs to be determined. This is      *
 *    necessary when scanning for a location that the aircraft can land.                       *
 *                                                                                             *
 * INPUT:   cell  -- The cell location to check for landing.                                   *
 *                                                                                             *
 * OUTPUT:  Returns a value indicating if the cell is a legal landing spot or not.             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/12/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
MoveType AircraftClass::Can_Enter_Cell(CELL cell, FacingType ) const
{
	Validate();
	if (!Map.In_Radar(cell)) return(MOVE_NO);

	CellClass * cellptr = &Map[cell];

	/*
	**	A landing test, asked as a movement question.
	**
	**	The banner above says what this routine is for -- "determines if the
	**	aircraft can land at this location" -- and Is_Generally_Clear ends on
	**	Ground[Land_Type()].Build, which is false for rock, water, beach and
	**	tiberium.  But What_Action(CELL) calls this to decide whether a move
	**	order is even offered, so refusing to land somewhere refuses to *fly*
	**	there, and the cursor comes back no-entry over every cliff and every
	**	crystal field on the map.
	**
	**	Nothing in CrystAlien stops a flier crossing anything.  UnitNav has no
	**	terrain term at all for a flying unit -- the ground is only consulted
	**	when a lander settles, which is CAC_Berth_Is_Clear's job and is checked
	**	there.  So the two questions are separated here rather than conflated.
	**
	**	Only ours: a Tiberian Dawn helicopter keeps the rule it was written
	**	with.
	*/
	if (Class != NULL && CAC_Is_CrystAlien_Type(Class->IniName)) {
		return(MOVE_OK);
	}

	if (!cellptr->Is_Generally_Clear(true)) return(MOVE_NO);

	if (GameToPlay == GAME_NORMAL && IsOwnedByPlayer && !cellptr->Is_Visible(PlayerPtr)) return(MOVE_NO);

	return(MOVE_OK);
}


/***********************************************************************************************
 * AircraftClass::Good_Fire_Location -- Searches for and finds a good spot to fire from.       *
 *                                                                                             *
 *    Given the specified target, this routine will locate a good spot for the aircraft to     *
 *    fire at the target.                                                                      *
 *                                                                                             *
 * INPUT:   target   -- The target that is desired to be attacked.                             *
 *                                                                                             *
 * OUTPUT:  Returns with the target location of the place that firing should be made from.     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/12/1995 JLB : Created.                                                                 *
 *   06/14/1995 JLB : Finer resolution on ring scan.                                           *
 *=============================================================================================*/
TARGET AircraftClass::Good_Fire_Location(TARGET target) const
{
	Validate();
	if (Target_Legal(target)) {
		int range = Weapon_Range(0);
		COORDINATE tcoord = As_Coord(target);
		CELL bestcell = 0;
		CELL best2cell = 0;
		int bestval = -1;
		int best2val = -1;

		for (int r = range-0x0180; r > 0x0180; r -= 0x0100) {
			for (int face = 0; face < 255; face += 16) {
				COORDINATE newcoord = Coord_Move(tcoord, (DirType)face, r);
				CELL newcell = Coord_Cell(newcoord);
				CELL actualcell = Coord_Cell(Coord_Sub(newcoord, XYPixel_Coord(0, FLIGHT_LEVEL)));

				if (Map.In_Radar(actualcell) && (GameToPlay != GAME_NORMAL || Map[newcell].Is_Visible(PlayerPtr)) && Cell_Seems_Ok(newcell, true)) {
					int dist = Distance(newcoord);
					if (bestval == -1 || dist < bestval) {
						best2val = bestval;
						best2cell = bestcell;
						bestval = dist;
						bestcell = newcell;
					}
				}
			}
			if (bestval != -1) break;
		}

		if (best2val == -1) {
			best2cell = bestcell;
		}

		/*
		**	If it found a good firing location, then return this location as
		**	a target value.
		*/
		if (bestval != -1) {
			if (Random_Pick(0, 1) == 0) {
				return(::As_Target(bestcell));
			} else {
				return(::As_Target(best2cell));
			}
		}
	}
	return(TARGET_NONE);
}


/***********************************************************************************************
 * AircraftClass::Cell_Seems_Ok -- Checks to see if a cell is good to enter.                   *
 *                                                                                             *
 *    This routine examines the navigation computers of other aircraft in order to see if the  *
 *    specified cell is safe to fly to. The intent of this routine is to avoid unneccessary    *
 *    mid-air collisions.                                                                      *
 *                                                                                             *
 * INPUT:   cell     -- The cell to examine for clear airspace.                                *
 *                                                                                             *
 *          strict   -- Should the scan consider the aircraft, that is making this check, a    *
 *                      blocking aircraft. Typically, the aircraft itself is not considered    *
 *                      a blockage -- an aircraft can always exist where it is currently       *
 *                      located. A strict check is useful for helicopters that need to move    *
 *                      around at the slightest provocation.                                   *
 *                                                                                             *
 * OUTPUT:  Is the specified cell free from airspace conflicts?                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool AircraftClass::Cell_Seems_Ok(CELL cell, bool strict) const
{
	Validate();
	/*
	**	Make sure that no other aircraft are heading to the selected location. If they
	**	are, then don't consider the location as valid.
	*/
	TARGET astarget = ::As_Target(cell);
	bool ok = true;
	for (int index = 0; index < Aircraft.Count(); index++) {
		AircraftClass * air = Aircraft.Ptr(index);
		if (air && (strict || air != this) && !air->IsInLimbo) {
			if (Coord_Cell(air->Coord) == cell || air->NavCom == astarget) {
				return(false);
			}
		}
	}
	return(true);
}


/***********************************************************************************************
 * AircraftClass::Pip_Count -- Returns the number of "objects" in aircraft.                    *
 *                                                                                             *
 *    This routine is used by the render logic to draw the little container "pips". This       *
 *    corresponds to the number of passengers for a transport helicopter or the number of      *
 *    shots remaining for an attack helicopter.                                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of "pips" to render on the aircraft.                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/11/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int AircraftClass::Pip_Count(void) const
{
	Validate();
	int retval = 0;

	if (Class->IsTransporter) {
		retval = How_Many();
	} else {
		if (Ammo) {
			retval = Cardinal_To_Fixed(Class->MaxAmmo, Ammo);
			retval = Fixed_To_Cardinal(Class->Max_Pips(), retval);
			if (!retval) retval = 1;
		}
	}
	return(retval);
}


/***********************************************************************************************
 * AircraftClass::Mission_Enter -- Control aircraft to fly to the helipad or repair center.    *
 *                                                                                             *
 *    This routine is used when the aircraft needs to fly for either rearming or repairing.    *
 *    It tries to establish contact with the support building. Once contact is established     *
 *    the ground controller takes care of commanding the aircraft.                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the delay before this routine should be called again.                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/12/1995 JLB : Created.                                                                 *
 *   07/04/1995 JLB : Ground controller gives orders.                                          *
 *=============================================================================================*/
int AircraftClass::Mission_Enter(void)
{
	Validate();
	enum {
		INITIAL,
		TAKEOFF,
		ALTITUDE,
		TRAVEL,
		LANDING
	};
	switch (Status) {
		case INITIAL:
			if (Altitude < FLIGHT_LEVEL || IsLanding) {
				Status = TAKEOFF;
			} else {
				Status = ALTITUDE;
			}
			break;

		case TAKEOFF:
			if (Process_Take_Off()) {
				/*
				**	After takeoff is complete, break radio contact with any helipad that this
				**	helicopter is taking off from.
				*/
				if (In_Radio_Contact() && Map[Coord_Cell(Coord)].Cell_Building() == Contact_With_Whom()) {
					Transmit_Message(RADIO_OVER_OUT);
				}
				Status = ALTITUDE;
			}
			break;

		case ALTITUDE:
			/*
			**	Establish radio contact with the building this helicopter is trying
			**	to land at.
			*/
			if (In_Radio_Contact()) {
				Status = TRAVEL;
			} else {
				TechnoClass * tech = As_Techno(NavCom);
				if (tech && Transmit_Message(RADIO_CAN_LOAD, tech) == RADIO_ROGER) {
					Transmit_Message(RADIO_HELLO, tech);
					Transmit_Message(RADIO_DOCKING);
					Status = TRAVEL;
				} else {
					Assign_Destination(TARGET_NONE);
					Enter_Idle_Mode();
				}
			}
			break;

		case TRAVEL:
			Transmit_Message(RADIO_DOCKING);
			if (!In_Radio_Contact()) {
				Assign_Destination(TARGET_NONE);
				Enter_Idle_Mode();
			} else {
				int distance = Process_Fly_To(true);

				if (distance < 0x0080) {
					if (Target_Legal(TarCom)) {
						SecondaryFacing.Set_Desired(Direction(TarCom));
					} else {
						SecondaryFacing.Set_Desired(Pose_Dir());
					}

					if (distance < 0x0010) {
						Status = LANDING;
					}
					break;
				} else {
					SecondaryFacing.Set_Desired(::Direction(Fire_Coord(0), As_Coord(NavCom)));
				}
				return(3);
			}
			break;

		case LANDING:
			if (IsTakingOff) {
				Assign_Destination(TARGET_NONE);
				Enter_Idle_Mode();
			}
			if (Process_Landing()) {
				switch (Transmit_Message(RADIO_IM_IN)) {
					case RADIO_ROGER:
						Assign_Mission(MISSION_GUARD);
						break;

					case RADIO_ATTACH:
						Limbo();
						Contact_With_Whom()->Attach(this);
						break;

					default:
						Enter_Idle_Mode();
				}
			}
			break;
	}
	return(1);
}


/***********************************************************************************************
 * AircraftClass::Good_LZ -- Locates a good spot ot land.                                      *
 *                                                                                             *
 *    This routine is used when helicopters need a place to land, but there are no obvious     *
 *    spots (i.e., helipad) available. It will try to land near a friendly helipad or friendly *
 *    building if there are no helipads anywhere. In the event that there are no friendly      *
 *    buildings anywhere on the map, then just land right where it is flying.                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the target location where this aircraft should land. This value may   *
 *          not be a clear cell, but the normal landing logic will resolve that problem.       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/12/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
TARGET AircraftClass::Good_LZ(void) const
{
	Validate();
	/*
	**	Scan through all of the buildings and try to land near
	**	the helipad (if there is one) or the nearest friendly building.
	*/
	CELL bestcell;
	int bestdist = -1;
	for (int index = 0; index < Buildings.Count(); index++) {
		BuildingClass * building = Buildings.Ptr(index);

		if (building && !building->IsInLimbo && building->House == House) {
			int dist = Distance(building);
			if (CAC_Is_Air_Home((StructType)*building)) {
				dist /= 4;
			}
			if (bestdist == -1 || dist < bestdist) {
				bestdist = dist;
				bestcell = Coord_Cell(building->Center_Coord());
			}
		}
	}

	/*
	**	Return with the suitable location if one was found.
	*/
	if (bestdist != -1) {
		return(::As_Target(bestcell));
	}

	/*
	**	No good location was found. Just try to land here.
	*/
	return(::As_Target(Coord_Cell(Coord)));
}


/***********************************************************************************************
 * AircraftClass::Set_Speed -- Sets the speed for the aircraft.                                *
 *                                                                                             *
 *    This routine will set the speed for the aircraft. The speed is specified as a fraction   *
 *    of full speed.                                                                           *
 *                                                                                             *
 * INPUT:   speed -- The fixed point fractional speed setting. 0x00 is stopped, 0xFF is full   *
 *                   speed.                                                                    *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::Set_Speed(int speed)
{
	Validate();
	MPHType sp = MPHType(min((unsigned)(Class->MaxSpeed * House->AirspeedBias), MPH_LIGHT_SPEED));
	Fly_Speed(speed, sp);
}


/***********************************************************************************************
 * AircraftClass::Fire_Direction -- Determines the direction of fire.                          *
 *                                                                                             *
 *    This routine will determine what direction a projectile would take if it were fired      *
 *    from the aircraft. This is the direction that the aircraft's body is facing.             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the direction of projectile fire.                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
DirType AircraftClass::Fire_Direction(void) const
{
	Validate();
	return(SecondaryFacing.Current());
}


/***********************************************************************************************
 * AircraftClass::~AircraftClass -- Destructor for aircraft object.                            *
 *                                                                                             *
 *    This is the destructor for aircraft. It will limbo the aircraft if it isn't already      *
 *    and also removes the aircraft from any team it may be attached to.                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/24/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
AircraftClass::~AircraftClass(void)
{
	if (GameActive && Class) {
		
#ifdef USE_RA_AI
		//
		// Added for RA AI in TD. ST - 7/26/2019 9:12AM
		//
		House->Tracking_Remove(this);
#endif

		/*
		**	If there are any cargo members, delete them.
		*/
		while (Is_Something_Attached()) {
			delete Detach_Object();
		}

		Limbo();
	}

	if (GameActive && Class && Team) Team->Remove(this);
}


/***********************************************************************************************
 * AircraftClass::Scatter -- Causes the aircraft to move away a bit.                           *
 *                                                                                             *
 *    This routine will cause the aircraft to move away from its current location and then     *
 *    enter some idle mode. Typically this is called when the aircraft is attacked while on    *
 *    the ground.                                                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::Scatter(COORDINATE , bool , bool )
{
	Validate();
	if (IsLanding || Altitude == 0) {
		IsLanding = false;
		IsTakingOff = true;
	}
	Enter_Idle_Mode();
}


/***********************************************************************************************
 * AircraftClass::Rearm_Delay -- Returns the delay between shots for this aircraft.            *
 *                                                                                             *
 *    Aircraft have a faster rearm delay than their weapon would otherwise indicate. This is   *
 *    necessary to give helicopters a combat edge while still allowing them to share the       *
 *    weapon types used by ground units.                                                       *
 *                                                                                             *
 * INPUT:   second   -- Is this for the second shot? The second shot uses the full rearm       *
 *                      delay. The first shot, if part of a two shot weapon, is given an       *
 *                      abbreviated rearm time.                                                *
 *                                                                                             *
 * OUTPUT:  Returns with the game frames to delay before the next shot can fire.               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int AircraftClass::Rearm_Delay(bool second) const
{
	Validate();
	return(FootClass::Rearm_Delay(second)/2);
}


/***********************************************************************************************
 * AircraftClass::Threat_Range -- Returns with a range to scan for targets.                    *
 *                                                                                             *
 *    This routine returns with the distance to scan for targets according to the type of      *
 *    search requested. The search type is typically the weapon (or short) range and vicinity  *
 *    distances. This is used by Guard and Guard Area missions. Aircraft never consider their  *
 *    weapon range in this determination since they can fly so fast, weapon range is           *
 *    meaningless.                                                                             *
 *                                                                                             *
 * INPUT:   control  -- The range control parameter;                                           *
 *                      -1 = range doesn't matter -- return -1 for compatability reasons.      *
 *                      0  = short range scan                                                  *
 *                      1  = long range scan                                                   *
 *                                                                                             *
 * OUTPUT:  Returns with the lepton distance to use for the scan operation.                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int AircraftClass::Threat_Range(int control) const
{
	Validate();
	if (control == -1) return(-1);

	int range = 20 * ICON_LEPTON_W;
	if (control == 1) {
		range *= 2;
	}
	return(range);
}


/***********************************************************************************************
 * AircraftClass::Mission_Guard -- Handles aircraft in guard mode.                             *
 *                                                                                             *
 *    Aircraft don't like to be in guard mode if in flight. If this situation is detected,     *
 *    then figure out what the aircraft should be doing and go do it.                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of game frames to delay before calling this routine again. *
 *                                                                                             *
 * WARNINGS:   This routine typically calls the normal guard logic for ground units.           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int AircraftClass::Mission_Guard(void)
{
	Validate();

	/*
	**	Ours holds station, at whatever altitude it happens to be.
	**
	**	Two lines of this routine are the insta-seat, and they are literal:
	**
	**	    if (Class->Primary == WEAPON_NONE) {
	**	        Assign_Destination(::As_Target(Coord_Cell(Coord)));
	**	        Assign_Mission(MISSION_MOVE);
	**	    }
	**
	**	That is how Tiberian Dawn parks an unarmed helicopter -- hand it the cell
	**	it is already standing over and send it there.  The move arrives on its
	**	first frame, because the distance is nought, and arrival puts the craft on
	**	the ground.  Every CrystAlien transport is unarmed, so every one of them
	**	took it the moment it was left to guard.
	**
	**	This was guarded before, but only inside `if (Altitude == FLIGHT_LEVEL)`,
	**	which is where those two lines live -- and a craft that is climbing or
	**	settling is not at flight level, so the rest of the routine was still
	**	reachable on the way up and down.  For a computer-owned craft that
	**	continues into the repair-bay and hunt logic, which assigns destinations of
	**	its own; the log shows the result as a transport ratcheting a cell at a
	**	time, settling and climbing every twenty frames without ever arriving.
	**
	**	CrystAlien has no such rule at any altitude.  A flier whose path is empty
	**	holds where it is -- `if (this.still > 20 && ...)` in UnitNav is the only
	**	thing that ever brings one down, and the descent gate in AI() is that test.
	**	So guard means guard, and the altitude is not this routine's business.
	*/
	if (Class != NULL && CAC_Is_CrystAlien_Type(Class->IniName)) {

		/*
		**	Holding station means not moving, and that has to be said.
		**
		**	Returning here leaves Speed exactly as the last mission left it, and
		**	Physics(Coord, PrimaryFacing) moves a craft along its facing at that
		**	speed every single frame regardless of what mission it is on.  So a
		**	flier that finished an attack, or a Viper handed out by a destroyed
		**	Experiment Lab, sailed off in whatever direction it happened to be
		**	pointing -- east, mostly, since DIR_E is where a good many of them end
		**	up -- for as long as nobody touched it.
		**
		**	That is the drift, and it is why CAC_Contain_Flier kept having to catch
		**	them at the border: the containment was treating the symptom.  Nothing
		**	in UnitNav moves a unit whose path is empty; `still` counts up and the
		**	craft stays where it is.
		**
		**	Anything with somewhere to be is left alone -- guard is not supposed to
		**	be carrying a destination, but if one is set it is not this routine's
		**	business to throw it away.
		*/
		if (!Target_Legal(NavCom) && Get_Speed() != MPH_IMMOBILE) {
			CAC_Log_Volatile("drift", "%s holding station at %d,%d -- speed %d stopped",
								  Class->IniName,
								  Cell_X(Coord_Cell(Center_Coord())),
								  Cell_Y(Coord_Cell(Center_Coord())), (int)Get_Speed());
			Set_Speed(0);
		}

		return(TICKS_PER_SECOND);
	}
	if (Altitude == FLIGHT_LEVEL) {

		/*
		**	If part of a team, then do nothing, since the team
		**	handler will take care of giving this aircraft a
		**	mission.
		*/
		if (Team) {
			if (Target_Legal(NavCom)) {
				Assign_Mission(MISSION_MOVE);
			}
			return(TICKS_PER_SECOND);
		}

		if (Class->Primary == WEAPON_NONE) {
			Assign_Destination(::As_Target(Coord_Cell(Coord)));
			Assign_Mission(MISSION_MOVE);
		} else {
			Enter_Idle_Mode();
		}
		return(1);
	}
	if (House->IsHuman) return(TICKS_PER_SECOND);

	/*
	**	Special case to force the GDI helicopter to be brain dead in the Nod
	**	mission where it is supposed to be captured.
	*/
	if (GameToPlay == GAME_NORMAL && Scenario == 7 && House->Class->House == HOUSE_GOOD) {
		return(TICKS_PER_SECOND*20);
	}

	/*
	**	If the aircraft is very badly damaged, then it will search for a
	**	repair bay first.
	*/
	if (House->Available_Money() >= 100 && Health_Ratio() <= 0x0080) {
		if (!In_Radio_Contact() ||
			(Altitude == 0 &&
				(Contact_With_Whom()->What_Am_I() != RTTI_BUILDING || *((BuildingClass *)Contact_With_Whom()) != STRUCT_REPAIR))) {


			BuildingClass * building = Find_Docking_Bay(STRUCT_REPAIR, true);
			if (building) {
				Assign_Destination(building->As_Target());
				Assign_Target(TARGET_NONE);
				Assign_Mission(MISSION_ENTER);
				return(1);
			}
		}
	}

	/*
	**	If the aircraft cannot attack anything because of lack of ammo,
	**	abort any normal guard logic in order to look for a helipad
	**	to rearm.
	*/
	if (Ammo == 0 && Class->Primary != WEAPON_NONE) {
		if (!In_Radio_Contact()) {
			BuildingClass * building = CAC_Find_Air_Home();
			if (building) {
				Assign_Destination(building->As_Target());
				Assign_Target(TARGET_NONE);
				Assign_Mission(MISSION_ENTER);
				return(1);
			}
		}
//		return(TICKS_PER_SECOND*3);
	}

	/*
	**	If the aircraft already has a target, then attack it if possible.
	*/
	if (Target_Legal(TarCom)) {
		Assign_Mission(MISSION_ATTACK);
		return(1);
	}

	/*
	**	Transport helicopters don't really do anything but just sit there.
	*/
	if (Class->Primary == WEAPON_NONE) {
		return(TICKS_PER_SECOND*3);
	}

	/*
	**	Computer controlled helicopters will defend themselves by bouncing around
	**	and looking for a free helipad.
	*/
	if (Altitude == 0 && !In_Radio_Contact()) {
		Scatter(0, true);
		return(TICKS_PER_SECOND*3);
	}

	return(FootClass::Mission_Guard());
}


/***********************************************************************************************
 * AircraftClass::Mission_Guard_Area -- Handles the aircraft guard area logic.                 *
 *                                                                                             *
 *    This routine handles area guard logic for aircraft. Aircraft require special handling    *
 *    for this mode since they are to guard area only if they are in a position to do so.      *
 *    Otherwise they just defend themselves.                                                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of game frames to delay before calling this routine        *
 *          again.                                                                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int AircraftClass::Mission_Guard_Area(void)
{
	Validate();
	if (Altitude == FLIGHT_LEVEL) {
		Enter_Idle_Mode();
		return(1);
	}
	if (House->IsHuman) return(TICKS_PER_SECOND);

	if (Altitude == 0 && !In_Radio_Contact()) {
		Scatter(0, true);
		return(TICKS_PER_SECOND*3);
	}

	if (Target_Legal(TarCom)) {
		Assign_Mission(MISSION_ATTACK);
		return(1);
	}
	return(FootClass::Mission_Guard_Area());
}


/***********************************************************************************************
 * AircraftClass::Response_Attack -- Gives audio response to attack order.                     *
 *                                                                                             *
 *    This routine is used to give an audio response to an attack order.                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/

/***********************************************************************************************
 * CAC_Find_Air_Home -- Locate a pad this aircraft can dock and rearm at.                      *
 *                                                                                             *
 *    CrystAlien's fighters and dropships set `home = findBuilding("BF_" + team)` -- the        *
 *    Radar/Sonar Station -- and UnitNav restores a shot on arrival                             *
 *    (`weaponPayload = min(++weaponPayload, maxWeaponPayload)`).  The transforming fighter     *
 *    launches from the Ops Ship / Alien Hive (BK) instead, so both count as pads.              *
 *=============================================================================================*/
BuildingClass *AircraftClass::CAC_Find_Air_Home(void) const
{
	static StructType const _pads[] = { CAC_SLOT_BFGOOD, CAC_SLOT_BFEVIL,
										CAC_SLOT_BKGOOD, CAC_SLOT_BKEVIL,
										STRUCT_HELIPAD, STRUCT_AIRSTRIP };
	for (int i = 0; i < (int)(sizeof(_pads)/sizeof(_pads[0])); i++) {
		BuildingClass *bay = Find_Docking_Bay(_pads[i], false);
		if (bay != NULL) return(bay);
	}
	return(NULL);
}

void AircraftClass::Response_Attack(void)
{
	/*
	**	CrystAlien units each have their own voice set; fall through to the
	**	stock response only for anything that is not a CrystAlien type.
	*/
	/*
	**	An order is answered with `comply`, whatever the order was.
	**
	**	Unit.handle plays `this.type + "_comply"` off `this.ordered`, which the order
	**	itself sets, and CrystAlien has no separate line for being told to attack.
	**	The `_action_` recordings are for doing things rather than being told to --
	**	UnitNav plays one when a miner takes a mouthful -- so answering an attack
	**	order with one meant asking for a sound nothing plays in that situation.
	*/
	if (CAC_Speak(this, CAC_VOICE_COMPLY)) return;

	Validate();
	static VocType _response[] = {
		VOC_AFFIRM,
		VOC_ACKNOWL,
		VOC_YESSIR,
		VOC_YESSIR,
		VOC_YESSIR
	};
	VocType response = _response[Sim_Random_Pick(0, (int)(sizeof(_response) / sizeof(_response[0]))-1)];
	if (AllowVoice) {
		Sound_Effect(response, 0, -(Aircraft.ID(this)+1));
	}
}


/***********************************************************************************************
 * AircraftClass::Response_Move -- Gives audio response to move request.                       *
 *                                                                                             *
 *    This routine is used to give an audio response to movement orders.                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::Response_Move(void)
{
	/*
	**	CrystAlien units each have their own voice set; fall through to the
	**	stock response only for anything that is not a CrystAlien type.
	*/
	if (CAC_Speak(this, CAC_VOICE_COMPLY)) return;

	Validate();
	static VocType _response[] = {
		VOC_MOVEOUT,
		VOC_MOVEOUT,
		VOC_MOVEOUT,
		VOC_ACKNOWL,
		VOC_AFFIRM,
		VOC_AFFIRM
	};
	VocType response = _response[Sim_Random_Pick(0, (int)(sizeof(_response) / sizeof(_response[0]))-1)];
	if (AllowVoice) {
		Sound_Effect(response, 0, -(Aircraft.ID(this)+1));
	}
}


/***********************************************************************************************
 * AircraftClass::Response_Select -- Gives audio response when selected.                       *
 *                                                                                             *
 *    This routine is called when an audio response for selection is desired.                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::Response_Select(void)
{
	/*
	**	CrystAlien units each have their own voice set; fall through to the
	**	stock response only for anything that is not a CrystAlien type.
	*/
	if (CAC_Speak(this, CAC_VOICE_COMPLY)) return;

	Validate();
	static VocType _response[] = {
		VOC_VEHIC,
		VOC_UNIT,
		VOC_YESSIR,
		VOC_YESSIR,
		VOC_YESSIR,
		VOC_AWAIT
	};
	VocType response = _response[Sim_Random_Pick(0, (int)(sizeof(_response) / sizeof(_response[0]))-1)];
	if (AllowVoice) {
		Sound_Effect(response, 0, -(Aircraft.ID(this)+1));
	}
}

void AircraftClass::Set_Reinforcement_Delay(long delay)
{
	ReinforcementStart = Frame + delay;
}