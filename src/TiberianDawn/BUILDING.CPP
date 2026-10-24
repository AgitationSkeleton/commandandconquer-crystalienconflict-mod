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

/* $Header:   F:\projects\c&c\vcs\code\building.cpv   2.13   02 Aug 1995 17:00:14   JOE_BOSTIC  $ */
/***********************************************************************************************
 ***             C O N F I D E N T I A L  ---  W E S T W O O D   S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : BUILDING.CPP                                                 *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : September 10, 1993                                           *
 *                                                                                             *
 *                  Last Update : August 20, 1995 [JLB]                                        *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   BuildingClass::AI -- Handles non-graphic AI processing for buildings.                     *
 *   BuildingClass::Active_Click_With -- Handles cell selection for buildings.                 *
 *   BuildingClass::As_Target -- Convert the building into a target value.                     *
 *   BuildingClass::Assign_Target -- Assigns a target to the building.                         *
 *   BuildingClass::Begin_Mode -- Begins an animation mode for the building.                   *
 *   BuildingClass::BuildingClass -- Constructor for buildings.                                *
 *   BuildingClass::Can_Demolish -- Can the player demolish (sell back) the building?          *
 *   BuildingClass::Can_Enter_Cell -- Determines if building can be placed down.               *
 *   BuildingClass::Can_Fire -- Determines if this building can fire.                          *
 *   BuildingClass::Captured -- Captures the building.                                         *
 *   BuildingClass::Center_Coord -- Fetches the center coordinate for the building.            *
 *   BuildingClass::Click_With -- Handles clicking on the map while the building is selected.  *
 *   BuildingClass::Crew_Type -- This determines the crew that this object generates.          *
 *   BuildingClass::Death_Announcement -- Announce the death of this building.                 *
 *   BuildingClass::Debug_Dump -- Displays building status to the monochrome screen.           *
 *   BuildingClass::Detach -- Handles target removal from the game system.                     *
 *   BuildingClass::Detach_All -- Possibly abandons production according to factory type.      *
 *   BuildingClass::Draw_It -- Displays the building at the location specified.                *
 *   BuildingClass::Drop_Debris -- Drops rubble when building is destroyed.                    *
 *   BuildingClass::Enter_Idle_Mode -- The building will enter its idle mode.                  *
 *   BuildingClass::Exit_Object -- Initiates an object to leave the building.                  *
 *   BuildingClass::Fire_At -- Fires weapon at specified target.                               *
 *   BuildingClass::Fire_Coord -- Calculates the coordinate that projectile would appear.      *
 *   BuildingClass::Fire_Direction -- Fetches the direction of firing.                         *
 *   BuildingClass::Fire_Out -- Handles when attached animation expires.                       *
 *   BuildingClass::Flush_For_Placement -- Handles clearing a zone for object placement.       *
 *   BuildingClass::Grand_Opening -- Handles construction completed special operations.        *
 *   BuildingClass::Greatest_Threat -- Searches for target that building can fire upon.        *
 *   BuildingClass::Init -- Initialize the building system to an empty null state.             *
 *   BuildingClass::Limbo -- Handles power adjustment as building goes into limbo.             *
 *   BuildingClass::Look -- Reveal map around building.                                        *
 *   BuildingClass::Mark -- Building interface to map rendering system.                        *
 *   BuildingClass::Mission_Attack -- Handles attack mission for building.                     *
 *   BuildingClass::Mission_Construction -- Handles mission construction.                      *
 *   BuildingClass::Mission_Deconstruction -- Handles building deconstruction.                 *
 *   BuildingClass::Mission_Guard -- Handles guard mission for combat buildings.               *
 *   BuildingClass::Mission_Harvest -- Handles refinery unloading harvesters.                  *
 *   BuildingClass::Mission_Missile -- State machine for nuclear missile launch.               *
 *   BuildingClass::Mission_Repair -- Handles the repair (active) state for building.          *
 *   BuildingClass::Mission_Unload -- Handles the unload mission for a building.               *
 *   BuildingClass::Pip_Count -- Determines "full" pips to display for building.               *
 *   BuildingClass::Power_Output -- Fetches the current power output from this building.       *
 *   BuildingClass::Read_INI -- Reads buildings from INI file.                                 *
 *   BuildingClass::Receive_Message -- Handle an incoming message to the building.             *
 *   BuildingClass::Refund_Amount -- Fetches the refund amount if building is sold.            *
 *   BuildingClass::Remap_Table -- Fetches the remap table to use for this building.           *
 *   BuildingClass::Repair -- Initiates or terminates the repair process.                      *
 *   BuildingClass::Revealed -- Reveals the building to the specified house.                   *
 *   BuildingClass::Sell_Back -- Controls the sell back (demolish) operation.                  *
 *   BuildingClass::Sort_Y -- Returns the building coordinate used for sorting.                *
 *   BuildingClass::Take_Damage -- Inflicts damage points upon a building.                     *
 *   BuildingClass::Toggle_Primary -- Toggles the primary factory state.                       *
 *   BuildingClass::Unlimbo -- Removes a building from limbo state.                            *
 *   BuildingClass::Update_Buildables -- Informs sidebar of additional construction options.   *
 *   BuildingClass::Update_Specials -- removes computer specials for lost bld                  *
 *   BuildingClass::What_Action -- Determines action to perform if click on specified object.  *
 *   BuildingClass::What_Action -- Determines what action will occur.                          *
 *   BuildingClass::Write_INI -- Writes all building data to an INI file.                      *
 *   BuildingClass::delete -- Deallocates building object.                                     *
 *   BuildingClass::new -- Allocates a building object from building pool.                     *
 *   BuildingClass::~BuildingClass -- Destructor for building type objects.                    *
 *   BuildingClass::Validate -- validates building pointer												  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"function.h"
#include	"CACSLOTS.INC"
#include	"CACLOGIC.H"

/*
** New sidebar for GlyphX multiplayer. ST - 3/26/2019 12:24PM
*/
#include "SidebarGlyphx.h"


enum SAMState {
	SAM_NONE=-1,					// Used for non SAM site buildings.
	SAM_UNDERGROUND,			// Launcher is underground and awaiting orders.
	SAM_RISING,					// Doors open and launcher rises to normal locked down position.
	SAM_READY,					// Launcher can be facing any direction tracking targets.
	SAM_FIRING,					// Stationary while missile is being fired.
	SAM_READY2,					// Launcher can be facing any direction tracking targets.
	SAM_FIRING2,				// Stationary while missile is being fired.
	SAM_LOCKING,				// Rotating to locked position in preparation for lowering.
	SAM_LOWERING,				// Launcher is lowering into the ground.
};


/***************************************************************************
**	Center of building offset table.
*/
COORDINATE const BuildingClass::CenterOffset[BSIZE_COUNT] = {
	0x00800080L,
	0x008000FFL,
	0x00FF0080L,
	0x00FF00FFL,
	0x018000FFL,
	0x00FF0180L,
	0x01800180L,

	0x00FF0200L,

	0x02800280L,
};

/*
** This contains the value of the Virtual Function Table Pointer
*/
void * BuildingClass::VTable;


/***********************************************************************************************
 * BuildingClass::Validate -- validates building pointer													  *
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
int BuildingClass::Validate(void) const
{
	int num;

	num = Buildings.ID(this);
	if (num < 0 || num >= BUILDING_MAX) {
		Validate_Error("BUILDING");
		return (0);
	}
	else
		return (1);
}
#else
#define	Validate()
#endif


/***********************************************************************************************
 * BuildingClass::Receive_Message -- Handle an incoming message to the building.               *
 *                                                                                             *
 *    This routine handles an incoming message to the building. Messages regulate the          *
 *    various cooperative ventures between buildings and units. This might include such        *
 *    actions as coordinating the construction yard animation with the actual building's       *
 *    construction animation.                                                                  *
 *                                                                                             *
 * INPUT:   from     -- The originator of the message received.                                *
 *                                                                                             *
 *          message  -- The radio message received.                                            *
 *                                                                                             *
 *          param    -- Reference to an optional parameter that might be used to return        *
 *                      extra information to the message originator.                           *
 *                                                                                             *
 * OUTPUT:  Returns with the response to the message (typically, this is just RADIO_OK).       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/09/1994 JLB : Created.                                                                 *
 *   06/26/1995 JLB : Forces refinery load anim to start immediately.                          *
 *   08/13/1995 JLB : Uses ScenarioInit for special loose "CAN_LOAD" check.                    *
 *=============================================================================================*/
RadioMessageType BuildingClass::Receive_Message(RadioClass * from, RadioMessageType message, long & param)
{
	Validate();
	switch (message) {

		/*
		**	This message is received as a request to attach/load/dock with this building.
		**	Verify that this is allowed and return the appropriate response.
		*/
		case RADIO_HELLO:
			//Refineries can't be interupted while they're processing a harvester - LLL April 22, 2020
			if (Mission == MISSION_HARVEST) {
				return(RADIO_NEGATIVE);
			}
			break;

		case RADIO_CAN_LOAD:
			TechnoClass::Receive_Message(from, message, param);
			if (BState == BSTATE_CONSTRUCTION || (!ScenarioInit && !CAC_Is_Refinery(Class->Type) && Class->Type != STRUCT_REFINERY && In_Radio_Contact())) return(RADIO_NEGATIVE);
			switch (Class->Type) {
				case STRUCT_AIRSTRIP:
					if (from->What_Am_I() == RTTI_AIRCRAFT && *((AircraftClass const *)from) == AIRCRAFT_CARGO) {
						return(RADIO_ROGER);
					}
					break;

				case STRUCT_HELIPAD:
					if (from->What_Am_I() == RTTI_AIRCRAFT && !((AircraftClass const *)from)->Class->IsFixedWing) {
						return(RADIO_ROGER);
					}
					break;

				case STRUCT_REPAIR:
					if (/*from->Health_Ratio() < 0x0100 &&*/ from->What_Am_I() == RTTI_UNIT || from->What_Am_I() == RTTI_AIRCRAFT) {
						return(RADIO_ROGER);
					}
					return(RADIO_NEGATIVE);

				/*
				**	CrystAlien's miner unloads at the headquarters, so those slots
				**	speak the docking protocol too.  See CAC_Is_Refinery.
				*/
				case CAC_SLOT_BAGOOD:
				case CAC_SLOT_BAEVIL:
				case CAC_SLOT_BKGOOD:
				case CAC_SLOT_BKEVIL:
				case STRUCT_REFINERY:
					if (from->What_Am_I() == RTTI_UNIT &&
						CAC_Is_Miner(*((UnitClass *)from)) &&
						(ScenarioInit || !Is_Something_Attached())) {

						return((Contact_With_Whom() != from) ? RADIO_ROGER : RADIO_NEGATIVE);
					}
					break;

				default:
					break;
			}
			return(RADIO_NEGATIVE);

		/*
		**	This message is received when the object has attached itself to this
		**	building.
		*/
		case RADIO_IM_IN:
			if (Mission == MISSION_DECONSTRUCTION) {
				return(RADIO_NEGATIVE);
			}
			switch (Class->Type) {
				case STRUCT_REPAIR:
					IsReadyToCommence = true;
					Assign_Mission(MISSION_REPAIR);
					from->Assign_Mission(MISSION_SLEEP);
					return(RADIO_ROGER);

				case STRUCT_HELIPAD:
					Assign_Mission(MISSION_REPAIR);
					from->Assign_Mission(MISSION_SLEEP);
					return(RADIO_ROGER);

				case CAC_SLOT_BAGOOD:
				case CAC_SLOT_BAEVIL:
				case CAC_SLOT_BKGOOD:
				case CAC_SLOT_BKEVIL:
				case STRUCT_REFINERY:
					ScenarioInit++;
					Begin_Mode(BSTATE_ACTIVE);
					ScenarioInit--;
					Mark(MARK_CHANGE);
					Assign_Mission(MISSION_HARVEST);
					return(RADIO_ATTACH);
			}
			break;

		/*
		**	Docking maneuver maintenance message. See if new order should be given to the
		**	unit trying to dock.
		*/
		case RADIO_DOCKING:
			TechnoClass::Receive_Message(from, message, param);

			/*
			**	When in radio contact for loading, the refinery starts
			**	flashing the lights.
			*/
			//Fix for refinery animation bug when mission is harvest - LLL April 22, 2020
			if (CAC_Is_Refinery(Class->Type) && BState != BSTATE_FULL && Mission != MISSION_HARVEST) {
				Begin_Mode(BSTATE_FULL);
			}

			/*
			**	If this building is already in radio contact, then it might
			**	be able to satisfy the request to load by bumping off any
			**	preoccupying task.
			*/
			if (*this == STRUCT_REPAIR) {
				if (Contact_With_Whom() != from) {
					if (Transmit_Message(RADIO_ON_DEPOT) == RADIO_ROGER) {
						if (Transmit_Message(RADIO_NEED_REPAIR) == RADIO_NEGATIVE) {
							Transmit_Message(RADIO_RUN_AWAY);
							Transmit_Message(RADIO_OVER_OUT);
							return(RADIO_ROGER);
						}
					}
				}
			}

			/*
			**	Establish contact with the object if this building isn't already in contact
			**	with another.
			*/
			if (!In_Radio_Contact()) {
				Transmit_Message(RADIO_HELLO, from);
			}

			if (Transmit_Message(RADIO_NEED_TO_MOVE) == RADIO_ROGER) {
				if (*this == STRUCT_HELIPAD) {
					param = As_Target();
				} else {
					if (*this == STRUCT_REPAIR) {
						Transmit_Message(RADIO_TETHER);
						param = ::As_Target(Coord_Cell(Center_Coord()));
					} else if (CAC_Is_Refinery(Class->Type) && !CAC_Is_Stock_Structure(Class->Type)) {

						/*
						**	Straight to the dock, tethered before it sets off.
						**
						**	The dock is a cell the headquarters itself stands on, and a unit
						**	may only enter its contact's cells once tethered -- so an
						**	untethered miner cannot finish the journey and stops on the last
						**	cell it is allowed to occupy, beside the building.
						**
						**	This used to walk it in two steps, to the free tile in front and
						**	then onto the plate, tethering in between -- but the tether is
						**	only sent when the move request comes back "already there", and
						**	arriving at the front tile does not ask again.  So it stood
						**	there.  Tiberian Dawn's refinery needs the two steps because its
						**	harvester reverses into a bay; CrystAlien has no such
						**	manoeuvre.  returnHome voyages to dockX/dockY and hands the load
						**	over on arrival, so tether now and let it drive in.
						*/
						Transmit_Message(RADIO_TETHER);
						param = ::As_Target(Coord_Cell(Docking_Coord()));
					} else {
						param = ::As_Target(Coord_Cell(Adjacent_Cell(Center_Coord(), DIR_SW)));
					}
				}

				/*
				**	Tell the harvester to move to the docking pad of the refinery.
				*/
				if (Transmit_Message(RADIO_MOVE_HERE, param) == RADIO_YEA_NOW_WHAT) {

					/*
					**	Since the harvester is already there, tell it to begin the backup
					**	procedure now. If it can't, then tell it to get outta here.
					*/
					Transmit_Message(RADIO_TETHER);

					/*
					**	Backing in belongs to the Tiberian Dawn refinery.  A CrystAlien
					**	miner is already standing on the dock at this point, so asking it
					**	to reverse only sends it away again.
					*/
					if (CAC_Is_Refinery(Class->Type) && !CAC_Is_Stock_Structure(Class->Type)) {

						/*
						**	Already standing on the dock, which is where the load changes
						**	hands.  Backing in stays with the Tiberian Dawn refinery.
						*/
					} else if (CAC_Is_Refinery(Class->Type) &&
						Transmit_Message(RADIO_BACKUP_NOW, from) != RADIO_ROGER) {
						from->Scatter(NULL, true);
					}
				}
			}
			return(RADIO_ROGER);

		/*
		**	If a transport or harvester is requesting permission to head toward, dock
		**	and load/unload, check to make sure that this is allowed given the current
		**	state of the building.
		*/
		case RADIO_ARE_REFINERY:
			if (Is_Something_Attached() || In_Radio_Contact() || IsInLimbo || House->Class->House != from->Owner() || (!CAC_Is_Refinery(Class->Type))) {
				return(RADIO_NEGATIVE);
			}
			return(RADIO_ROGER);

		/*
		**	Someone is telling us that it is starting construction. This should only
		**	occur if this is a construction yard and a building was just placed on
		**	the map.
		*/
		case RADIO_BUILDING:
			Assign_Mission(MISSION_REPAIR);
			TechnoClass::Receive_Message(from, message, param);
			return(RADIO_ROGER);

		/*
		**	Someone is telling us that they have finished construction. This should
		**	only occur if this is a construction yard and the building that was being
		**	constructed has finished. In this case, stop the construction yard
		**	animation.
		*/
		case RADIO_COMPLETE:
			if (Mission != MISSION_DECONSTRUCTION) {
				Assign_Mission(MISSION_GUARD);
			}
			TechnoClass::Receive_Message(from, message, param);
			return(RADIO_ROGER);

		/*
		**	This message may occur unexpectedly if the unit in contact with this
		**	building is suddenly destroyed. Handle any cleanup necessary. For example,
		**	a construction yard should stop its construction animation in this case.
		*/
		case RADIO_OVER_OUT:
			Begin_Mode(BSTATE_IDLE);
			TechnoClass::Receive_Message(from, message, param);
			return(RADIO_ROGER);

		/*
		**	This message is received when an object has completely left
		** building. Sometimes special cleanup action is required when
		**	this event occurs.
		*/
		case RADIO_UNLOADED:
			if (*this == STRUCT_REPAIR) {
				if (Distance(from) < 0x0180) {
					return(RADIO_ROGER);
				}
			}

			//Turn off the refinery lights - LLL April 22, 2020
			if (CAC_Is_Refinery(Class->Type)) {
				Begin_Mode(BSTATE_IDLE);
			}

			TechnoClass::Receive_Message(from, message, param);
			if (*this == STRUCT_WEAP || *this == STRUCT_AIRSTRIP || *this == STRUCT_REPAIR) return(RADIO_RUN_AWAY);
			return(RADIO_ROGER);
	}

	/*
	**	Pass along the message to the default message handler in the radio itself.
	*/
	return(TechnoClass::Receive_Message(from, message, param));
}


#ifdef CHEAT_KEYS
/***********************************************************************************************
 * BuildingClass::Debug_Dump -- Displays building status to the monochrome screen.             *
 *                                                                                             *
 *    This utility function will output the current status of the building class to the        *
 *    monochrome screen. It is through this data that bugs may be fixed or detected.           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/31/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void BuildingClass::Debug_Dump(MonoClass *mono) const
{
	Validate();
	mono->Set_Cursor(0, 0);
	mono->Print(
		"�Name:���������������Mission:����TarCom:���������Radio:�Coord:������������St:Ŀ\n"
		"�                   �           �       �       �      �        �        �    �\n"
		"����������������N�Y�Health:��������Turret:���������Building:���Cargo:��������Ĵ\n"
		"�Active........� � �        �     �       �      �            �               �\n"
		"�Limbo.........� � ����������������������������������������������������������Ĵ\n"
		"�Owned.........� � �Last Message:                                             �\n"
		"�Discovered....� � �Timer:�Arm:��������Tiberium:�Flash:�Stage:�����������������\n"
		"�Selected......� � �      �    �      �         �      �      �                \n"
		"�Teathered.....� � ��������������������������������������������                \n"
		"�Locked on Map.� � �                                                           \n"
		"�Is A Loaner...� � �                                                           \n"
		"�              � � �                                                           \n"
		"�              � � �                                                           \n"
		"�              � � �                                                           \n"
		"�Repairing.....� � �                                                           \n"
		"�              � � �                                                           \n"
		"�              � � �                                                           \n"
		"�Recoiling.....� � �                                                           \n"
		"�To Display....� � �                                                           \n"
		"��������������������                                                           \n");
	mono->Set_Cursor(1, 1);mono->Printf("%s:%s", House->Class->IniName, Class->IniName);
	mono->Set_Cursor(35, 3);mono->Printf("%02X:%02X", PrimaryFacing.Current(), PrimaryFacing.Desired());
	mono->Set_Cursor(50, 3);
	if (Factory) {
		mono->Printf(Factory->Get_Object()->Class_Of().IniName);
		mono->Printf(" ");
		mono->Printf("%d%%", Factory->Completion());
	} else {
		mono->Printf("(empty)");
	}

	mono->Text_Print("X", 16 + (IsRepairing?2:0), 14);
//	mono->Set_Cursor(44, 3);mono->Printf("%d", SAM);
	mono->Set_Cursor(34, 1);mono->Printf("%04X", TarCom);
	mono->Set_Cursor(28, 7);mono->Printf("%2d", Arm);

	TechnoClass::Debug_Dump(mono);
}
#endif


/***********************************************************************************************
 * BuildingClass::Draw_It -- Displays the building at the location specified.                  *
 *                                                                                             *
 *    This is the low level graphic routine that displays the building at the location         *
 *    specified.                                                                               *
 *                                                                                             *
 * INPUT:   x,y   -- The coordinate to draw the building at.                                   *
 *                                                                                             *
 *          window   -- The clipping window to use.                                            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/20/1994 JLB : Created.                                                                 *
 *   06/27/1994 JLB : Takes a clipping window parameter.                                       *
 *   07/06/1995 JLB : Handles damaged silos correctly.                                         *
 *=============================================================================================*/
void BuildingClass::Draw_It(int x, int y, WindowNumberType window)
{
	Validate();
	void const * shapefile;	// Pointer to loaded shape file.
	int shapenum;

	shapenum = Fetch_Stage();

	/*
	**	The shape file to use for rendering depends on whether the building
	**	is undergoing construction or not.
	*/
	if (BState == BSTATE_CONSTRUCTION) {
		shapefile = Class->Get_Buildup_Data();

		/*
		**	If the building is deconstructing, then the display frame progresses
		**	from the end to the beginning. Reverse the shape number accordingly.
		*/
		if (Mission == MISSION_DECONSTRUCTION) {
			shapenum = (Class->Anims[BState].Start+Class->Anims[BState].Count-1)-shapenum;
		}

	} else {

		shapefile = Class->Get_Image_Data();

		/*
		**	The obelisk has a stage value than can be overridden by
		**	its current state.
		*/
		if (*this == STRUCT_OBELISK) {
			if (IsCharged) {
				shapenum = 3;
			} else {
				if (IsCharging) {
					shapenum = Fetch_Stage();
				} else {
					shapenum = 0;
				}
			}
		}

		/*
		**	Buildings that contain a turret handle their shape determination
		**	differently than normal buildings. They need to take into consideration
		**	the direction the turret is facing.
		*/
		if (Class->IsTurretEquipped) {
			shapenum = UnitClass::BodyShape[Facing_To_32(PrimaryFacing.Current())];

			if (*this == STRUCT_SAM) {

				/*
				**	SAM sites that are free to rotate fetch their animation frame
				**	from the building's turret facing. All other animation stages
				**	fetch their frame from the embedded animation sequencer.
				*/
				if (Status == SAM_READY || Status == SAM_FIRING || Status == SAM_READY2 || Status == SAM_FIRING2 || Status == SAM_LOCKING) {
					shapenum += 16;
				} else {
					shapenum = Fetch_Stage();
				}
			} else {
				if (IsInRecoilState) {
					shapenum += 32;
				}
			}
			if (Health_Ratio() < 0x0080) {
				shapenum += 64;
			}
		} else {

			/*
			**	If it has only one point of strength left, it is shown in the
			**	worst state possible.
			*/
			if (Strength <= 1) {
				shapenum = Get_Build_Frame_Count(shapefile)-1;
			} else {

				if (*this == STRUCT_WEAP) {
					shapenum = 0;
					if (Health_Ratio() < 0x0080) {
						shapenum = 1;
					}

				} else {

					/*
					**	Special render stage for silos. The stage is dependant on the current
					**	Tiberium collected as it relates to Tiberium capacity.
					*/
					if (*this == STRUCT_STORAGE) {
						int level = 0;
						if (House->Capacity) {
							level = (House->Tiberium * 5) / House->Capacity;
						}
//						int level = Fixed_To_Cardinal(4, Cardinal_To_Fixed(House->Capacity, House->Tiberium));

						shapenum += Bound(level, 0, 4);
						if (Health_Ratio() < 0x0080) {
							shapenum += 5;
						}

					} else {

						/*
						**	pain() swaps to the damaged still at `healthPerc <= 25`, not at a
						**	half, and only for a structure that does not keep animating --
						**	`if (!stats.weapon || stats.isHQ)`.  An armed structure that is not
						**	the headquarters, which is to say the turret, holds its facings.
						*/
						if (CAC_Shows_Damage(this)) {

							/*
							**	Special damage stage for pump.
							*/
							if (!Class->IsSimpleDamage) {
								int last1 = Class->Anims[BSTATE_IDLE].Start + Class->Anims[BSTATE_IDLE].Count;
								int last2 = Class->Anims[BSTATE_ACTIVE].Start + Class->Anims[BSTATE_ACTIVE].Count;
								int largest = MAX(last1, last2);
								last2 = Class->Anims[BSTATE_AUX1].Start + Class->Anims[BSTATE_AUX1].Count;
								largest = MAX(largest, last2);
								last2 = Class->Anims[BSTATE_AUX2].Start + Class->Anims[BSTATE_AUX2].Count;
								largest = MAX(largest, last2);

								shapenum += largest;
							} else {

								/*
								**	Presume that the damage stage is the end frame.
								*/
								shapenum = Get_Build_Frame_Count(shapefile) - 2;
							}
						}
					}
				}
			}
		}
	}

	/*
	**	Actually draw the building shape.
	*/
	IsTheaterShape = Class->IsTheater;
	Techno_Draw_Object(shapefile, shapenum, x, y, window);
	IsTheaterShape = false;

	/*
	** Patch for adding overlay onto weapon factory.  Only add the overlay if
	** the building has more than 1 hp.  Also, if the building's in radio
	** contact, he must be unloading a constructed vehicle, so draw that
	** vehicle before drawing the overlay.
	*/
	if (BState != BSTATE_CONSTRUCTION) {

		/*
		**	A Tethered object is always rendered AFTER the building.
		*/
		if (*this == STRUCT_WEAP && IsTethered && In_Radio_Contact() && !Contact_With_Whom()->IsInLimbo) {
			TechnoClass * contact = Contact_With_Whom();

			assert(contact->IsActive);
			int xxx = x + ((int)Lepton_To_Pixel((int)Coord_X(contact->Render_Coord())) - (int)Lepton_To_Pixel((int)Coord_X(Render_Coord())));
			int yyy = y + ((int)Lepton_To_Pixel((int)Coord_Y(contact->Render_Coord())) - (int)Lepton_To_Pixel((int)Coord_Y(Render_Coord())));
			contact->Draw_It(xxx, yyy, window);
			contact->IsToDisplay = false;
		}

		/*
		**	Draw the weapon factory custom overlay graphic.
		*/
		/*
		**	The weapon factory is drawn in two pieces: the building, and this
		**	overlay carrying its roof and door.  CrystAlien's Vehicle Factory
		**	occupies that slot but has no such second piece -- its door is part
		**	of its own clip -- so drawing the stock overlay over it is what put
		**	a GDI weapons factory on top of the CrystAlien one the moment the
		**	build-up finished.
		*/
		if (*this == STRUCT_WEAP && Strength > 1 &&
			!CAC_Is_CrystAlien_Type(Class->IniName)) {
			shapenum = Door_Stage();
			if (Health_Ratio() < 0x0080) shapenum += 10;
			// Added override shape file name. ST - 6/20/2019 1:35PM
			//Techno_Draw_Object(WarFactoryOverlay, shapenum, x, y, window);
			Techno_Draw_Object_Virtual(WarFactoryOverlay, shapenum, x, y, window, "WEAP2");
		}

		/*
		**	Draw any repair feedback graphic required.
		*/
		if (IsRepairing && IsWrenchVisible) {
			CC_Draw_Shape(ObjectTypeClass::SelectShapes, SELECT_WRENCH, x, y, window, SHAPE_CENTER|SHAPE_WIN_REL);
		}
	}

	TechnoClass::Draw_It(x, y, window);
}


/***********************************************************************************************
 * BuildingClass::Mark -- Building interface to map rendering system.                          *
 *                                                                                             *
 *    This routine is used to mark the map cells so that when it renders                       *
 *    the underlying icons will also be updated as necessary.                                  *
 *                                                                                             *
 * INPUT:   mark  -- Type of image change (MARK_UP, _DOWN, _CHANGE)                            *
 *             MARK_UP  -- Building is removed.                                                *
 *             MARK_CHANGE -- Building changes shape.                                          *
 *             MARK_DOWN -- Building is added.                                                 *
 *                                                                                             *
 * OUTPUT:  bool; Did the mark operation succeed? Failure could be the result of marking down  *
 *                when the building is already marked down, or visa versa.                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/31/1994 JLB : Created.                                                                 *
 *   04/15/1994 JLB : Converted to member function.                                            *
 *   04/16/1994 JLB : Added health bar tracking.                                               *
 *   12/23/1994 JLB : Calls low level check before proceeding.                                 *
 *   01/27/1995 JLB : Special road spacer template added.                                      *
 *=============================================================================================*/
bool BuildingClass::Mark(MarkType mark)
{
	Validate();
	if (TechnoClass::Mark(mark)) {
		short const *offset = Overlap_List();
		short const *occupy = Occupy_List();
		CELL cell = Coord_Cell(Coord);
		SmudgeType bib;

		switch (mark) {
			case MARK_UP:
				Map.Pick_Up(cell, this);
				if (Class->Bib_And_Offset(bib, cell, House->Class->House)) {
					SmudgeClass * smudge = new SmudgeClass(bib);
					if (smudge) {
						smudge->Disown(cell);
						delete smudge;
					}
				}
				break;

			case MARK_DOWN:

				/*
				**	Special wall logic is handled here. A building that is really a wall
				**	gets converted into an overlay wall type when it is placed down. The
				**	actual building object itself is destroyed.
				*/
				if (Class->IsWall) {
					switch (Class->Type) {
						case STRUCT_BRICK_WALL:
							new OverlayClass(OVERLAY_BRICK_WALL, cell, House->Class->House);
							break;

						case STRUCT_BARBWIRE_WALL:
							new OverlayClass(OVERLAY_BARBWIRE_WALL, cell, House->Class->House);
							break;

						case STRUCT_SANDBAG_WALL:
							new OverlayClass(OVERLAY_SANDBAG_WALL, cell, House->Class->House);
							break;

						case STRUCT_WOOD_WALL:
							new OverlayClass(OVERLAY_WOOD_WALL, cell, House->Class->House);
							break;

						case STRUCT_CYCLONE_WALL:
							new OverlayClass(OVERLAY_CYCLONE_WALL, cell, House->Class->House);
							break;
					}
					Transmit_Message(RADIO_OVER_OUT);
					Delete_This();

				} else {

					if (Can_Enter_Cell(cell) == MOVE_OK) {

						/*
						**	Determine if a bib is required for this building. If one is, then
						**	create and place it.
						*/
						CELL newcell = cell;
						if (Class->Bib_And_Offset(bib, newcell, House->Class->House)) {
							new SmudgeClass(bib, Cell_Coord(newcell), House->Class->House);
						}

						Map.Place_Down(cell, this);
					} else {
						return(false);
					}
				}
				break;

			default:
				Map.Refresh_Cells(cell, offset);
				Map.Refresh_Cells(cell, occupy);
				break;
		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * BuildingClass::Fire_At -- Fires weapon at specified target.                                 *
 *                                                                                             *
 *    This routine does the actual firing of a projectile from the                             *
 *    building toward the specified target. Prior to calling this                              *
 *    routine, the building must have rotated into position and acquired                       *
 *    a suitable target.                                                                       *
 *                                                                                             *
 * INPUT:   target   -- The target to fire upon.                                               *
 *                                                                                             *
 *          which    -- Which weapon to use for firing. 0=primary, 1=secondary.                *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the projectile just launched. This                       *
 *          may come in handy if additional adjustments to the projectile                      *
 *          are required.                                                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/16/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
BulletClass * BuildingClass::Fire_At(TARGET target, int which)
{
	Validate();
	BulletClass * bullet;		// Projectile.
	WeaponTypeClass const * weapon = (which == 0) ? &Weapons[Class->Primary] : &Weapons[Class->Secondary];

	bullet = TechnoClass::Fire_At(target, which);
	if (bullet) {
		if (*this == STRUCT_SAM) {
			AnimClass *anim = new AnimClass((AnimType)(ANIM_SAM_N + Dir_Facing(PrimaryFacing.Current())), Center_Coord());
			if (anim) {
				anim->Attach_To(this);
			}

			// MBL 04.17.2020
			Sound_Effect(weapon->Sound, Coord);

		} else {

			/*
			**	Flash the muzzle, play sound, and perform any firing animation.
			*/
			Sound_Effect(weapon->Sound, Coord);

			AnimClass* anim = NULL;
			if (weapon->Fires == BULLET_BULLET) {
				anim = new AnimClass((AnimType)(ANIM_GUN_N + Dir_Facing(PrimaryFacing.Current())), Fire_Coord(which));
			} else {
				switch (weapon->Fires) {
				case BULLET_SPREADFIRE:
					break;
				case BULLET_LASER:
					IsCharging = false;
					IsCharged = false;
					Set_Stage(0);
					Set_Rate(0);
					break;
				default:
					anim = new AnimClass(ANIM_MUZZLE_FLASH, Fire_Coord(which));
					break;
				}
			}
			if (anim != NULL) {
				anim->Attach_To(this);
			}
			Mark(MARK_CHANGE);
		}
	}

	return(bullet);
}


/***********************************************************************************************
 * BuildingClass::AI -- Handles non-graphic AI processing for buildings.                       *
 *                                                                                             *
 *    This function is to handle the AI logic for the building. The graphic logic (facing,     *
 *    firing, and animation) is handled elsewhere.                                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/31/1994 JLB : Created.                                                                 *
 *   12/26/1994 JLB : Handles production.                                                      *
 *   06/11/1995 JLB : Revamped.                                                                *
 *=============================================================================================*/
void BuildingClass::AI(void)
{
	/*	Why an armed building is not shooting -- see CAC_Log_Gun_Idle. */
	CAC_Log_Gun_Idle(this);

	Validate();

	/*
	**	CrystAlien's income buildings pay out on their own clock rather than
	**	taking deliveries.  See CAC_Building_Revenue.
	*/
	CAC_Building_Revenue(this);

	/*
	**	A defence turret with nothing to shoot sweeps.  See CAC_Idle_Turret_Spin.
	*/
	CAC_Idle_Turret_Spin(this);
	CAC_Damage_Smoke(this);

	/*
	**	Process building animation state changes. Transition to a following state
	**	if there is one specified and the current animation sequence has expired.
	**	This process must occur before mission AI since the mission AI relies on
	**	the bstate change to occur immediately before the MissionClass::AI.
	*/
	bool stagechange = Graphic_Logic();
	bool toloop = false;

	/*
	**	Always refresh the SAM site if it has an animation change.
	*/
	if (*this == STRUCT_SAM && stagechange) Mark(MARK_CHANGE);

	if ((!Class->IsTurretEquipped && *this != STRUCT_OBELISK) || Mission == MISSION_CONSTRUCTION || Mission == MISSION_DECONSTRUCTION) {
		if (stagechange) {

			/*
			**	Check for animation end or if special case of MCV deconstructing when it is allowed
			**	to convert back into an MCV.
			*/
			BuildingTypeClass::AnimControlType const * ctrl = Fetch_Anim_Control();

			/*
			**	When the last frame of the current animation sequence is reached, flag that
			**	a new mission may be started. This must occur before the animation actually
			**	loops so that if a mission change does occur, it will have a chance to change
			**	the building graphic before the last frame is replaced by the first frame of
			**	the loop.
			*/
			/*
			**	`>=`, because a one-frame build-up can never hit it exactly.
			**
			**	This only runs on a stage change, and a stage change is the counter
			**	having already moved.  With Count 1 the last frame is stage 0, which is
			**	where the animation starts -- so by the time there is a change to react
			**	to, the stage is 1 and the test for 0 has been missed.  The next line
			**	then sees stage >= Count and loops it back to 0, and it is missed again,
			**	every time round.
			**
			**	The Defence Station is the one building that lands on this: CrystAlien
			**	draws its construction in a single frame where everything else takes
			**	twelve or more.  It never left BSTATE_CONSTRUCTION -- and a building
			**	that never leaves it never opens for business, so the turret stood
			**	there not firing, which reads exactly like a turret with no power.
			**	Can_Demolish refuses anything still in that state, so it could not be
			**	sold either, which is the other half of the same report.
			**
			**	At every other length this is the same test: the stage arrives on
			**	Start+Count-1 exactly and the two agree.  It differs only where the
			**	counter has stepped past it, which is the case being dropped.
			*/
			if (Fetch_Stage() >= ctrl->Start+ctrl->Count-1  || (Special.IsMCVDeploy && CAC_Undeploys_Into(Class->Type) != UNIT_NONE && Mission == MISSION_DECONSTRUCTION && Fetch_Stage() == (42-19))) {
				IsReadyToCommence = true;
			}

			/*
			**	If the animation advances beyond the last frame, then start the animation
			**	sequence over from the beginning.
			*/
			if (Fetch_Stage() >= ctrl->Start+ctrl->Count) {
				toloop = true;
			}
			Mark(MARK_CHANGE);
		} else {
			if (BState == BSTATE_NONE || Fetch_Rate() == 0) {
				IsReadyToCommence = true;
			}
		}
	}

	/*
	**	If there is a door that is animating, then it might cause this building
	**	to be redrawn. Check for and flag to redraw as necessary.
	*/
	if (Time_To_Redraw()) {
		Clear_Redraw_Flag();
		Mark(MARK_CHANGE);
	}

	/*
	**	The animation sequence has looped. Restart it and flag this loop condition.
	**	This is used to tell the mission system that the animation has completed. It
	**	also signals that now is a good time to act on any pending mission.
	*/
	if (toloop) {
		BuildingTypeClass::AnimControlType const * ctrl = Fetch_Anim_Control();
		if (BState == BSTATE_CONSTRUCTION || BState == BSTATE_IDLE) {
			Set_Rate(Options.Normalize_Delay(ctrl->Rate));
		} else {
			Set_Rate(ctrl->Rate);
		}
		Set_Stage(ctrl->Start);
		Mark(MARK_CHANGE);
	}

	/*
	**	If now is a good time to act on a new mission, then do so. This process occurs
	**	here because some outside event may have requested a mission change for the building.
	**	Such outside requests (player input) must be initiated BEFORE the normal AI process.
	*/
	if (IsReadyToCommence && BState != BSTATE_CONSTRUCTION) {

		/*
		**	Clear the commencement flag ONLY if something actually occured. By acting
		**	this way, a building can set the IsReadyToCommence flag before it goes
		**	to "sleep" knowing that it will wake up as soon as a new mission comes
		**	along.
		*/
		if (Commence()) {
			IsReadyToCommence = false;
		}
	}

	/*
	**	Proceed with normal logic processing. This is where the mission processing
	**	occurs. This call must be located after the animation sequence makes the
	**	transition to the next frame (see above) in order for the mission logic to
	**	act at the exact moment of graphic transition BEFORE it has a chance to
	**	be displayed.
	*/
	TechnoClass::AI();

	/*
	**	If now is a good time to act on a new mission, then do so. This occurs here because
	**	some AI event may have requested a mission change (usually from another mission
	**	state machine). This must occur here before it has a chance to render.
	*/
	if (IsReadyToCommence) {

		/*
		**	Clear the commencement flag ONLY if something actually occured. By acting
		**	this way, a building can set the IsReadyToCommence flag before it goes
		**	to "sleep" knowing that it will wake up as soon as a new mission comes
		**	along.
		*/
		if (Commence()) {
			IsReadyToCommence = false;
		}
	}

	/*
	**	If a change of animation was requested, then make the change
	**	now. The building animation system acts independantly but subordinate
	**	to the mission state machine system. By performing the animation change-up
	**	here, the mission AI system is ensured of immediate visual affect when it
	**	decides to change the animation state of the building.
	*/
	if (QueueBState != BSTATE_NONE) {
		if (BState != QueueBState) {
			BState = QueueBState;
			BuildingTypeClass::AnimControlType const * ctrl = Fetch_Anim_Control();
			if (BState == BSTATE_CONSTRUCTION || BState == BSTATE_IDLE) {
				Set_Rate(Options.Normalize_Delay(ctrl->Rate));
			} else {
				Set_Rate(ctrl->Rate);
			}
			Set_Stage(ctrl->Start);
		}
		QueueBState = BSTATE_NONE;
	}

	/*
	**	If the building's strength has changed, then update the power
	**	accordingly.
	*/
	if (Strength != LastStrength) {
		int oldpower = Power_Output();
		LastStrength = Strength;
		int newpower = Power_Output();
		House->Adjust_Power(newpower - oldpower);
	}

	/*
	**	Check to see if the destruction countdown timer is active. If so, then decrement it.
	**	When this timer reaches zero, the building is removed from the map. All the explosions
	**	are presumed to be in progress at this time.
	*/
	if (!Strength) {
		if (CountDown.Expired()) {
			Limbo();
			Drop_Debris(WhomToRepay);
			Delete_This();
		}
		return;
	}

	/*
	**	Obelisk charging logic.
	*/
	if (*this == STRUCT_OBELISK && BState != BSTATE_CONSTRUCTION) {
		/*
		**	Level.engageEnemy returns without firing when the owner is powerOff,
		**	and only for buildings -- units are unaffected.
		*/
		if (Target_Legal(TarCom) && !CAC_Power_Off(House)) {
			if (!IsCharged) {
				if (IsCharging) {
					if (stagechange) {
						Mark(MARK_CHANGE);
						if (Fetch_Stage() >= 4) {
							IsCharged = true;
							IsCharging = false;
							Set_Rate(0);
						}
					}
				} else {
					IsCharged = false;
					IsCharging = true;
					Set_Stage(0);
					Set_Rate(OBELISK_ANIMATION_RATE);
					Sound_Effect(CAC_Stock_Sound(this, VOC_LASER_POWER), Coord);
				}
			}
		} else {
			if (IsCharging || IsCharged) {
				Mark(MARK_CHANGE);
				IsCharging = false;
				IsCharged = false;
				Set_Stage(0);
				Set_Rate(0);
			}
		}
	}

	/*
	**	Handle any repair process that may be going on.
	*/
	if (IsRepairing) {
		if ((Frame % 15) == 0) {
			IsWrenchVisible = (IsWrenchVisible == false);
			Mark(MARK_CHANGE);
			int cost = Class->Repair_Cost();
			int step = Class->Repair_Step();

			/*
			**	Check for and expend any necessary monies to continue the repair.
			*/
			if (House->Available_Money() >= cost) {
				House->Spend_Money(cost);
				Strength += step;

				if (Strength >= Class->MaxStrength) {
					Strength = Class->MaxStrength;
					IsRepairing = false;
				}
			} else {
				IsRepairing = false;
			}
		}
	}

	/*
	**	Handle any production tied to this building. Only computer controlled buildings have
	**	production attached to the building itself. The player uses the sidebar interface for
	**	all production control.
	*/
	if (Factory && Factory->Has_Completed() && PlacementDelay.Expired()) {

		switch (Exit_Object(Factory->Get_Object())) {

			/*
			**	If the object could not leave the factory, then either request
			**	a transport, place the (what must be a) building using another method, or
			**	abort the production and refund money.
			*/
			case 0:
				Factory->Abandon();
				delete Factory;
				Factory = 0;
				break;

			case 1:
				PlacementDelay = TICKS_PER_SECOND*3;
				break;

			case 2:
				Factory->Completed();
				delete Factory;
				Factory = 0;
				break;

		}
	}

	/*
	**	For computer controlled buildings, determine what should be produced and start
	**	production accordingly.
	*/
	if (!House->IsHuman && Mission != MISSION_CONSTRUCTION && Mission != MISSION_DECONSTRUCTION) {

		/*
		**	Possibly start repair process if the building is below half strength.
		*/
		/*
		**	The scenario scaling below is campaign pacing, and a skirmish is not a
		**	campaign.
		**	
		**	Scenario is the number in the mission name, so a multiplayer map called
		**	SCM97EA sets it to ninety-seven.  Both tests downstream read it: the ratio
		**	becomes 0x00C0, so a building is a candidate for abandonment at three
		**	quarters health, and `Random_Pick(0, 50) < Scenario` is then always true.
		**	So every damaged computer building whose owner holds less than
		**	REPAIR_THRESHHOLD is sold, immediately, every time.
		**	
		**	That is what "the Alien computers sell everything the moment they are
		**	attacked" is, and why it is mostly the Aliens: they have no Driller, so
		**	they are the side that is usually under a thousand credits.  A log of one
		**	session has them at two, four, five and eight.
		*/
		bool campaign = (GameToPlay == GAME_NORMAL);

		int ratio = 0x0040;
		if (campaign && Scenario > 6) ratio = 0x0080;
		if (campaign && Scenario > 10) ratio = 0x00C0;
		if (Class->IsRepairable && Health_Ratio() <= (unsigned) ratio) {
			if (House->Available_Money() >= REPAIR_THRESHHOLD) {
				Repair(1);
			} else {
				/*
				**	And the same for the chance itself -- one in fifty in a skirmish, which
				**	is "occasionally gives up on a wreck" rather than "sells the base".
				*/
				int chance = campaign ? (int)Scenario : 1;
				if (IsTickedOff && chance > 0 && Random_Pick(0, 50) < chance && !Trigger) {
					if (GameToPlay != GAME_NORMAL || Scenario != 15 || PlayerPtr->ActLike != HOUSE_GOOD || *this != STRUCT_TEMPLE) {
						Sell_Back(1);
					}
				}
			}
		}

		/*
		**	Buildings that produce other objects have special factory logic handled here.
		*/
		if (Class->ToBuild != RTTI_NONE ||
			CAC_Builds_Any(Class->Type, RTTI_UNITTYPE) ||
			CAC_Builds_Any(Class->Type, RTTI_INFANTRYTYPE) ||
			CAC_Builds_Any(Class->Type, RTTI_AIRCRAFTTYPE)) {

			if (Factory) {

				/*
				**	If production has halted, then just abort production and make the
				**	funds available for something else.
				*/
				if (PlacementDelay.Expired() && !Factory->Is_Building()) {
					Factory->Abandon();
					delete Factory;
					Factory = 0;
				}

			} else {

				/*
				**	Only look to start production if there is at least a small amount of
				**	money available. In cases where there is no practical money left, then
				**	production can never complete -- don't bother starting it.
				*/
				if (House->IsStarted && House->Available_Money() > 10) {
					/*
					**	Everything this building hosts, not only the one category its
					**	ToBuild has room to name.  See CAC_Host_Suggestion.
					*/
					TechnoTypeClass const * techno =
							CAC_Host_Suggestion(House, Class->Type, Class->ToBuild);

					/*
					**	If a suitable object type was selected for production, then start
					**	producing it now.
					*/
					if (techno) {
						Factory = new FactoryClass;
						if (Factory) {
							if (!Factory->Set(*techno, *House)) {
								delete Factory;
								Factory = 0;
							} else {
#ifdef USE_RA_AI
								House->Production_Begun(Factory->Get_Object());		// Added for RA AI in TD. ST - 7/26/2019 9:46AM
#endif
								Factory->Start();
							}
						}
					}
				}
			}
		}
	}

	/*
	**	Check for demolition timeout. When timeout has expired, the building explodes.
	*/
	if (IsGoingToBlow && CountDown.Expired()) {
		
		/*
		** Maybe trigger an achivement. ST - 11/14/2019 1:53PM
		*/
		TechnoTypeClass const *object_type = Techno_Type_Class();
		if (object_type) {
			TechnoClass *saboteur = As_Techno(WhomToRepay);
			if (saboteur && saboteur->IsActive && saboteur->House && saboteur->House->IsHuman) {
				On_Achievement_Event(saboteur->House, "BUILDING_DESTROYED_C4", object_type->IniName);
			}
		}			
		
		SabotagedType = Class->Type;
		int damage = 5000;
		Take_Damage(damage, 0, WARHEAD_FIRE, As_Techno(WhomToRepay));
		Mark(MARK_CHANGE);
	}

	/*
	**	If the building was in a recoil state (as it would be just as it fires), then
	**	restore the building.
	*/
	if (IsInRecoilState) {
		IsInRecoilState = false;
		Mark(MARK_CHANGE);
	}

	/*
	**	Turret equiped buildings must handle turret rotation logic here. This entails
	**	rotating the turret to the desired facing as well as figuring out what that
	**	desired facing should be.
	*/
	if (Class->IsTurretEquipped && Mission != MISSION_CONSTRUCTION && Mission != MISSION_DECONSTRUCTION) {

		/*
		**	Rotate turret to match desired facing.
		*/
		if (PrimaryFacing.Is_Rotating()) {
			if (*this == STRUCT_SAM) {
				if (PrimaryFacing.Rotation_Adjust(15)) {
					Mark(MARK_CHANGE);
				}
			} else {
				if (PrimaryFacing.Rotation_Adjust(12)) {
					Mark(MARK_CHANGE);
				}
			}
		}
	}
}


/***********************************************************************************************
 * BuildingClass::Unlimbo -- Removes a building from limbo state.                              *
 *                                                                                             *
 *    Use this routine to transform a building that has been held in limbo                     *
 *    state, into one that really exists on the map. Once a building as                        *
 *    been unlimboed, then it becomes a normal object in the game world.                       *
 *                                                                                             *
 * INPUT:   pos   -- The position to place the building on the map.                            *
 *                                                                                             *
 *          dir (optional) -- not used for this class                                          *
 *                                                                                             *
 * OUTPUT:  bool; Was the unlimbo successful?                                                  *
 *                                                                                             *
 * WARNINGS:   The unlimbo operation might not be successful if the                            *
 *             building could not be placed at the location specified.                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/16/1994 JLB : Created.                                                                 *
 *   06/07/1994 JLB : Matches virtual function format for base class.                          *
 *   05/09/1995 JLB : Handles wall placement.                                                  *
 *   06/18/1995 JLB : Checks for wall legality before placing down.                            *
 *=============================================================================================*/
bool BuildingClass::Unlimbo(COORDINATE coord, DirType dir)
{
	Validate();
#ifdef OBSOLETE
	if (*this == STRUCT_ROAD) {
		if (Can_Enter_Cell(Coord_Cell(coord), FACING_NONE) == MOVE_OK) {
			ObjectClass * o = OverlayTypeClass::As_Reference(OVERLAY_ROAD).Create_One_Of(House);
			if (o && o->Unlimbo(coord)) {
				Transmit_Message(RADIO_OVER_OUT);
				Delete_This();
				return(true);
			}
		}
		return(false);
	}
#endif

	/*
	**	If this is a wall type building, then it never gets unlimboed. Instead, it gets
	**	converted to an overlay type.
	*/
	if (Class->IsWall) {
		if (Can_Enter_Cell(Coord_Cell(coord), FACING_NONE) == MOVE_OK) {
			OverlayType otype = OVERLAY_NONE;
			switch (Class->Type) {
				case STRUCT_SANDBAG_WALL:
					otype	= OVERLAY_SANDBAG_WALL;
					break;

				case STRUCT_CYCLONE_WALL:
					otype = OVERLAY_CYCLONE_WALL;
					break;

				case STRUCT_BRICK_WALL:
					otype = OVERLAY_BRICK_WALL;
					break;

				case STRUCT_BARBWIRE_WALL:
					otype = OVERLAY_BARBWIRE_WALL;
					break;

				case STRUCT_WOOD_WALL:
					otype = OVERLAY_WOOD_WALL;
					break;
			}
			if (otype != OVERLAY_NONE) {
				ObjectClass * o = OverlayTypeClass::As_Reference(otype).Create_One_Of(House);
				if (o && o->Unlimbo(coord)) {
					Map[Coord_Cell(coord)].Owner = House->Class->House;
					Transmit_Message(RADIO_OVER_OUT);
					Delete_This();
					return(true);
				}
			}
		}
		return(false);
	}

	/*
	**	Normal building unlimbo process.
	*/
	if (TechnoClass::Unlimbo(coord, dir)) {

		/*
		**	Ensure that the owning house knows about the
		**	new object.
		*/
		House->BScan |= CAC_Scan_Bit(Class->Type);
		House->ActiveBScan |= CAC_Scan_Bit(Class->Type);

#ifdef USE_RA_AI
		//
		// Added for RA AI in TD. ST - 7/26/2019 9:25AM
		//
		House->Recalc_Center();
#endif

		/*
		**	Update the total factory type, assuming this building has a factory.
		*/
		/*
		**	A building that hosts a type counts as a factory of that kind,
		**	whatever its own ToBuild says.  See CAC_Builds_Any.
		*/
		if (CAC_Builds_Any(Class->Type, RTTI_UNITTYPE)) House->UnitFactories++;
		if (CAC_Builds_Any(Class->Type, RTTI_INFANTRYTYPE)) House->InfantryFactories++;
		if (CAC_Builds_Any(Class->Type, RTTI_AIRCRAFTTYPE)) House->AircraftFactories++;

		switch (Class->ToBuild) {
			case RTTI_AIRCRAFTTYPE:
				House->AircraftFactories++;
				break;

			case RTTI_INFANTRYTYPE:
				House->InfantryFactories++;
				break;

			case RTTI_UNITTYPE:
				House->UnitFactories++;
				break;

			case RTTI_BUILDINGTYPE:
				House->BuildingFactories++;
				break;

			default:
				break;
		}

		/*
		**	Possibly the sidebar will be affected by this addition.
		*/
		House->IsRecalcNeeded = true;
		LastStrength = 0;

		// Changed for new multiplayer. ST - 4/3/2019 11:20AM
		//if ((!IsDiscoveredByPlayer && Map[Coord_Cell(coord)].IsVisible) || GameToPlay != GAME_NORMAL) {
		//	Revealed(PlayerPtr);
		//}
		if (!Is_Discovered_By_Player(House) && Map[Coord_Cell(coord)].Is_Visible(House) || GameToPlay != GAME_NORMAL) {
			if (House->IsHuman) {
				Revealed(House);
			}
		}

		if (!House->IsHuman) {
			Revealed(House);
		}

		//Changed for multiplayer ST - 3/13/2019 5:31PM
		if (Is_Owned_By_Player()) {
		//if (IsOwnedByPlayer) {
			Map.PowerClass::IsToRedraw = true;
			Map.Flag_To_Redraw(false);
		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * BuildingClass::Take_Damage -- Inflicts damage points upon a building.                       *
 *                                                                                             *
 *    This routine will inflict damage points upon the specified building.                     *
 *    It will handle the damage animation and building destruction. Use                        *
 *    this routine whenever a building is attacked.                                            *
 *                                                                                             *
 * INPUT:   damage   -- Amount of damage to inflict.                                           *
 *                                                                                             *
 *          distance -- The distance from the damage center point to the object's center point.*
 *                                                                                             *
 *          warhead  -- The kind of damage to inflict.                                         *
 *                                                                                             *
 *          source   -- The source of the damage. This is used to change targeting.            *
 *                                                                                             *
 * OUTPUT:  true/false; Was the building destroyed?                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/21/1991     : Created.                                                                 *
 *   04/15/1994 JLB : Converted to member function.                                            *
 *   04/16/1994 JLB : Added warhead modifier to damage.                                        *
 *   06/03/1994 JLB : Added source of damage as target value.                                  *
 *   06/20/1994 JLB : Source is a base class pointer.                                          *
 *   11/22/1994 JLB : Shares base damage handler for techno objects.                           *
 *   07/15/1995 JLB : Power ratio gets adjusted.                                               *
 *=============================================================================================*/
ResultType BuildingClass::Take_Damage(int & damage, int distance, WarheadType warhead, TechnoClass * source)
{
	Validate();

	/*
	**	A headquarters cannot be hurt in Pizza Mode.
	**
	**	The whole point of the mode is that a base cannot be removed, so the only
	**	way to end a game is the pizza.  Everything else on the field is as
	**	destructible as it ever was -- including the Ops Ship and the Alien Hive,
	**	which answer `hq` to the build tree but are not protected here.  See
	**	CAC_Pizza_Protected.
	**
	**	RESULT_NONE rather than zero damage, because the callers read the result:
	**	answering "nothing happened" stops the scorches, the shake and the kill
	**	credit as well as the health loss.
	*/
	if (CAC_Pizza_Protected(this)) {
		damage = 0;
		return(RESULT_NONE);
	}

	/*
	**	Write down that this house is under attack, which nothing was doing.
	**
	**	LATime, LAEnemy, LAType and LAZone are declared, initialised in the
	**	constructor and never assigned anywhere -- so STATE_ATTACKED was
	**	unreachable and the two tests in HouseClass::AI that read LATime could
	**	only ever see zero.  A computer player therefore had no notion of being
	**	attacked at all, which is why its answer to losing a building was the same
	**	as its answer to a quiet minute.
	**
	**	Buildings only, deliberately: a skirmish between units in the field is not
	**	the base being threatened, and CAC_AI_Raid reads this to decide how much
	**	to commit and whether to keep it near home.
	*/
	if (House != NULL && damage > 0 && source != NULL && !House->Is_Ally(source)) {
		House->LATime = (int)Frame;
		House->LAEnemy = source->Owner();
		House->LAType = source->What_Am_I();
		House->LAZone = House->Which_Zone(this);
	}
	ResultType res = RESULT_NONE;
	int shakes;

	if (this != source) {
		if (source) Base_Is_Attacked(source);

		short const *offset = Occupy_List();

		/*
		**	SPECIAL CASE:
		**	SAM sites that are closed will take half damage, but never less than one point.
		*/
		if (*this == STRUCT_SAM && Status == SAM_UNDERGROUND) {
			damage /= 2;
			damage++;
		}

		/*
		**	Damage from an ion cannon against the Temple of Nod does more damage than
		**	usual.
		*/
		if (GameToPlay == GAME_NORMAL && *this == STRUCT_TEMPLE && warhead == WARHEAD_PB) {
			damage += damage/2;
		}

		/*
		**	Perform the low level damage assessment.
		*/
		int landed = damage;
		res = TechnoClass::Take_Damage(damage, distance, warhead, source);

		/*
		**	"if (weapon.dmg) { ...; sfx.play(pool[random(6) + 1]); }" -- a struck
		**	structure borrows a unit's pain recording.  See CAC_Building_Hit.
		*/
		if (landed > 0 && res != RESULT_NONE) CAC_Building_Hit(this);

		switch (res) {
			case RESULT_DESTROYED:

				/*
				**	Destroy all attached objects.
				*/
				while (Attached_Object()) {
					FootClass * obj = Detach_Object();

					Detach_All(true);
					delete obj;
				}

				Sound_Effect(CAC_Stock_Sound(this, VOC_XPLOBIG4), Coord);
				while (*offset != REFRESH_EOL) {
					CELL cell = Coord_Cell(Coord) + *offset++;

					/*
					**	If the building is destroyed, then lots of
					**	explosions occur.
					*/
					/*
					**	Ours goes up the way Building.freeBlock does it: four
					**	explode() calls per cell of the footprint, each scattered
					**	somewhere inside that cell, and each dropping the one
					**	scorch that explode() always drops.  No craters and no
					**	burning wreck -- CrystAlien has neither.
					**
					**	Only the mark differs in count.  Four scorches land in
					**	the same cell there and stack; a cell here holds one
					**	smudge, so the fourth simply lands where the first did.
					*/
					if (CAC_Is_CrystAlien_Type(Class->IniName)) {
						for (int burst = 0; burst < 4; burst++) {
							new AnimClass(ANIM_CAC_BOOM, Coord_Scatter(Cell_Coord(cell), 0x0080));
						}
						new SmudgeClass(SMUDGE_CAC_SCORCH, Cell_Coord(cell));
					} else {
						new SmudgeClass(Random_Pick(SMUDGE_CRATER1, SMUDGE_CRATER6), Cell_Coord(cell));
						if (Random_Pick(0, 1) == 0) {
							new AnimClass(ANIM_FIRE_SMALL, Coord_Scatter(Cell_Coord(cell), 0x0080), Random_Pick(0, 7), Random_Pick(1, 3));
							if (Random_Pick(0, 1) == 0) {
								new AnimClass(ANIM_FIRE_MED, Coord_Scatter(Cell_Coord(cell), 0x0040), Random_Pick(0, 7), Random_Pick(1, 3));
							}
						}
					}
					/*
					**	And not Tiberian Dawn's own fireball on top of ours.
					**
					**	Building.freeBlock calls explode four times per tile and
					**	does nothing else, so the four CACBOOMs above are the whole
					**	of a CrystAlien structure coming down.  This line was
					**	putting a fifth explosion from the wrong game in the middle
					**	of them, every cell, which is why the deaths read as busy
					**	rather than as theirs.
					*/
					//Start_Profiler();
					if (!CAC_Is_CrystAlien_Type(Class->IniName)) {
						new AnimClass(ANIM_FBALL1, Coord_Scatter(Cell_Coord(cell), 0x0040), Random_Pick(0,3));
					}
				}

				shakes = Class->Cost_Of() / 400;
				if (shakes) {
					Shake_The_Screen(shakes, Owner());
					if (source && Owner() != source->Owner()) {
						Shake_The_Screen(shakes, source->Owner());
					}
				}
				Sound_Effect(CAC_Stock_Sound(this, VOC_CRUMBLE), Coord);
				if (Mission == MISSION_DECONSTRUCTION) {
					CountDown = 0;
					Set_Rate(0);
				} else {
					CountDown = 8;
				}

				/*
				**	A destuction of the Temple by an ion cannon requires a global
				**	remembering of this fact. The finale uses this information to
				**	play the correct movie.
				*/
				if (*this == STRUCT_TEMPLE && warhead == WARHEAD_PB) {
					TempleIoned = true;

					/*
					** Maybe trigger an achivement if the structure is owned by an AI house in campaign mode. ST - 11/14/2019 1:53PM
					*/
					if (GameToPlay == GAME_NORMAL && !House->IsHuman && source && source->House && source->House->IsHuman) {
						TechnoTypeClass const *object_type = Techno_Type_Class();
						if (object_type) {
							On_Achievement_Event(source->House, "ION_DESTROYS_TEMPLE", object_type->IniName);
						}			
					}

				} else {
					TempleIoned = false;
				}
				
				if (House) {
					House->Check_Pertinent_Structures();
				}
				break;

			case RESULT_HALF:
				if (*this == STRUCT_PUMP) {
					AnimClass *anim = new AnimClass(ANIM_OILFIELD_BURN, Coord_Add(Coord, 0x00400130L), 1);
					if (anim) {
						anim->Attach_To(this);
					}
				}
				// Fall into next case.

			case RESULT_MAJOR:
				Sound_Effect(CAC_Stock_Sound(this, VOC_XPLOBIG4), Coord);
				while (*offset != REFRESH_EOL) {
					CELL cell = Coord_Cell(Coord) + *offset++;
					AnimClass * anim = NULL;
					AnimType atype = ANIM_NONE;

					/*
					**	Show pieces of fire to indicate that a significant change in
					**	damage level has occurred.
					*/
					if (warhead == WARHEAD_FIRE) {
						switch (Random_Pick(0, 13)) {
							case 0:
							case 1:
							case 2:
							case 3:
							case 4:
								atype = ANIM_ON_FIRE_SMALL;
								anim = new AnimClass(atype, Coord_Scatter(Cell_Coord(cell), 0x0060), 0, Random_Pick(1, 3));
								break;

							case 5:
							case 6:
							case 7:
								atype = ANIM_ON_FIRE_MED;
								anim = new AnimClass(atype, Coord_Scatter(Cell_Coord(cell), 0x0060), 0, Random_Pick(1, 3));
								break;

							case 8:
								atype = ANIM_ON_FIRE_BIG;
								anim = new AnimClass(atype, Coord_Scatter(Cell_Coord(cell), 0x0060), 0, 1);
								break;

							case 9:
							case 10:
							case 11:
							case 12:
							case 13:
								break;
						}
					} else {
						if (Random_Pick(0, 1) == 0) {
							atype = ANIM_FIRE_SMALL;
							anim = new AnimClass(atype, Coord_Scatter(Cell_Coord(cell), 0x0060), Random_Pick(0, 7), Random_Pick(1, 3));
						}
					}

					/*
					**	If the animation was created, then attach it to the building.
					**
					**	Except that attaching it is what burns the building down.
					**	AnimClass::AI reads Class->Damage and calls Take_Damage on
					**	whatever the anim is attached to -- ANIM_ON_FIRE_SMALL carries
					**	eight, MED sixteen, BIG twenty-four, and the accumulator pays
					**	out a point once it passes 255 -- so a structure that was hit
					**	once goes on losing health by itself for as long as the fires
					**	burn.  Which is the report: "sometimes randomly my buildings
					**	will take a tiny bit of splash damage despite nothing hitting
					**	em".  Nothing is.  It was hit a while ago and is still on fire.
					**
					**	CrystAlien has no such thing.  Damage is applied in exactly two
					**	places in the whole of it, `this.health -= weapon.dmg` both
					**	times, and neither is on a timer: a weapon fires, a thing loses
					**	health, and that is the entire model.  Nothing burns, nothing
					**	bleeds, nothing is on fire afterwards.
					**
					**	So the fire is kept and the damage is not.  An unattached anim
					**	has no Object, and AnimClass::AI reads Class->Damage only when
					**	there is one, so the building smokes exactly as before and stops
					**	paying for it.  Tiberian Dawn's own structures are left alone.
					*/
					if (anim) {
						bool ours = (Class != NULL) && CAC_Is_CrystAlien_Type(Class->IniName);
						bool harmful = (atype != ANIM_NONE) &&
										  (AnimTypeClass::As_Reference(atype).Damage != 0);
						if (!ours || !harmful) {
							anim->Attach_To(this);
						}
					}
				}
				break;

			case RESULT_NONE:
				break;
		}

		if (source && res != RESULT_NONE) {

			/*
			**	If any damage occurred, then inform the house of this fact. If it is the player's
			**	house, it might announce this fact.
			*/
			House->Attacked(this);

			/*
			** Save the type of the house that's doing the damage, so if the building burns
			** to death credit can still be given for the kill
			*/
			WhoLastHurtMe = source->Owner();

			/*
			**	When certain buildings are hit, they "snap out of it" and
			**	return fire if they are able and allowed.
			*/
			if (*this != STRUCT_SAM &&
				!House->Is_Ally(source) &&
				Class->Primary != WEAPON_NONE &&
				(!Target_Legal(TarCom) || !In_Range(TarCom))) {

				if (source->What_Am_I() != RTTI_AIRCRAFT && (!House->IsHuman || Special.IsSmartDefense)) {
					Assign_Target(source->As_Target());
				} else {

					/*
					**	Generate a random rotation effect since there is nothing else that this
					**	building can do.
					*/
					if (!PrimaryFacing.Is_Rotating()) {
						PrimaryFacing.Set_Desired(Random_Pick(DIR_N, DIR_MAX));
					}
				}
			}
		}
	}

	return(res);
}


/***********************************************************************************************
 * BuildingClass::Look -- Reveal map around building.                                          *
 *                                                                                             *
 *    Given a building, reveal the cells around the building in accordance                     *
 *    with the building's sighting range.                                                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This is a very slow routine. Call only when necessary.                          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/05/1992 JLB : Created.                                                                 *
 *   04/15/1994 JLB : Converted to member function.                                            *
 *=============================================================================================*/
void BuildingClass::Look(bool)
{
	/*
	** Changed this function to reveal for the appropriate players in GlyphX multiplayer. ST - 4/17/2019 11:50AM
	*/
	Validate();

	if (Class->SightRange) {
		Map.Sight_From(House, Coord_Cell(Center_Coord()), Class->SightRange, false);
	}

#if (0)
	if (GameToPlay != GAME_GLYPHX_MULTIPLAYER) {

		if (Is_Owned_By_Player(PlayerPtr) || Is_Discovered_By_Player(PlayerPtr)) {
			Map.Sight_From(PlayerPtr, Coord_Cell(Center_Coord()), Class->SightRange, false);
		}
	} else {
		
		for (int i = 0; i < MPlayerCount; i++) {
			HousesType house_type = MPlayerHouses[i];
			HouseClass *house = HouseClass::As_Pointer(house_type);

			if (Is_Owned_By_Player(house) || Is_Discovered_By_Player(house)) {
				Map.Sight_From(house, Coord_Cell(Center_Coord()), Class->SightRange, false);
			}
		}
	}
#endif
}


#if (0)                                 // For reference. ST - 4/17/2019 11:38AM
void BuildingClass::Look(bool)
{
	Validate();
	if (IsOwnedByPlayer || IsDiscoveredByPlayer) {
		Map.Sight_From(PlayerPtr, Coord_Cell(Center_Coord()), Class->SightRange, false);
	}
}
#endif


/***********************************************************************************************
 * BuildingClass::new -- Allocates a building object from building pool.                       *
 *                                                                                             *
 *    This routine will allocate a building slot from the building alloc                       *
 *    system.                                                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the allocated building. If NULL is                       *
 *          returned, then this indicates a failure to allocate.                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/11/1994 JLB : Created.                                                                 *
 *   04/21/1994 JLB : Converted to operator new.                                               *
 *   05/17/1994 JLB : Revamped allocation scheme                                               *
 *   07/29/1994 JLB : Simplified.                                                              *
 *=============================================================================================*/
void * BuildingClass::operator new(size_t )
{
	void * ptr = Buildings.Allocate();
	if (ptr) {
		((BuildingClass *)ptr)->Set_Active();
	}
	return(ptr);
}


/***********************************************************************************************
 * BuildingClass::delete -- Deallocates building object.                                       *
 *                                                                                             *
 *    This is the memory deallocation operation for a building object.                         *
 *    Since buildings are allocated out of a fixed memory block, all that                      *
 *    is needed is to flag the unit as inactive.                                               *
 *                                                                                             *
 * INPUT:   ptr   -- Pointer to building to deallocate.                                        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/21/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void BuildingClass::operator delete(void *ptr)
{
	if (ptr) {
		((BuildingClass *)ptr)->IsActive = false;
	}
	Buildings.Free((BuildingClass *)ptr);

	//Map.Validate();
}


/***********************************************************************************************
 * BuildingClass::BuildingClass -- Constructor for buildings.                                  *
 *                                                                                             *
 *    This routine inserts a building into the object tracking system.                         *
 *    It is placed into a limbo state unless a location is provided for                        *
 *    it to unlimbo at.                                                                        *
 *                                                                                             *
 * INPUT:   type  -- The structure type to make this object.                                   *
 *                                                                                             *
 *          house -- The owner of this building.                                               *
 *                                                                                             *
 *          pos   -- The position to unlimbo the building. If -1 is                            *
 *                   specified, then the building remains in a limbo                           *
 *                   state.                                                                    *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/21/1994 JLB : Created.                                                                 *
 *   08/07/1995 JLB : Fixed act like value to match expected value.                            *
 *=============================================================================================*/
BuildingClass::BuildingClass(StructType type, HousesType house) :
	Class(&BuildingTypeClass::As_Reference(type)),
	TechnoClass(house)
{
	PlacementDelay = 0;
	LastStrength = 0;
	ActLike = House->ActLike;
	BState = BSTATE_NONE;
	CACSmoke = 0;
	CountDown.Set(0);
	Factory = 0;
#ifndef USE_RA_AI
	House->CurBuildings++;		       // Removed for RA AI in TD
#endif
	WhomToRepay = TARGET_NONE;
	IsCaptured = false;
	IsCharged = false;
	IsCharging = false;
	IsSurvivorless = false;
	IsGoingToBlow = false;
	IsReadyToCommence = false;
	IsRepairing = false;
	IsSecondShot = !Class->IsTwoShooter;
	IsWrenchVisible = false;
	QueueBState = BSTATE_NONE;
	Strength = Class->MaxStrength;
	WhoLastHurtMe = house;
	Ammo = Class->MaxAmmo;

	/*
	**	Make sure that newly built house specific building types will act like
	**	the house they are supposed to act like, regardless of who the current
	**	owner may happen to be.
	*/
	if ((type == STRUCT_AIRSTRIP || type == STRUCT_HAND) && house != HOUSE_BAD) {
		ActLike = HOUSE_BAD;
		IsCaptured = true;
	}
	if ((type == STRUCT_WEAP || type == STRUCT_BARRACKS) && house != HOUSE_GOOD) {
		ActLike = HOUSE_GOOD;
		IsCaptured = true;
	}

	if (GameToPlay == GAME_INTERNET){
		House->BuildingTotals->Increment_Unit_Total( (int) type);
	}

#ifdef USE_RA_AI
	//
	// Added for RA AI in TD. ST - 7/26/2019 9:12AM
	//
	House->Tracking_Add(this);
#endif // USE_RA_AI
}


/***********************************************************************************************
 * BuildingClass::~BuildingClass -- Destructor for building type objects.                      *
 *                                                                                             *
 *    This destructor for building objects will put the building in limbo if possible.         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
BuildingClass::~BuildingClass(void)
{
	if (GameActive && Class) {
		if (House) {
#ifndef USE_RA_AI
			House->CurBuildings--;
#else
			//
			// Added for RA AI in TD. ST - 7/26/2019 9:12AM
			//
			House->Tracking_Remove(this);
#endif
		}
		Limbo();
	}
}


/***********************************************************************************************
 * BuildingClass::Drop_Debris -- Drops rubble when building is destroyed.                      *
 *                                                                                             *
 *    This routine is called when a building is destroyed. It handles                          *
 *    placing the rubble down.                                                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/14/1994 JLB : Created.                                                                 *
 *   06/13/1995 JLB : Added smoke and normal infantry survivor possibility.                    *
 *   07/16/1995 JLB : Survival rate depends on if captured or sabotaged.                       *
 *=============================================================================================*/
void BuildingClass::Drop_Debris(TARGET source)
{
	Validate();
	CELL	const *offset;
	CELL	cell;

	/*
	**	Special case for Chan to run from destroyed technology
	**	building.
	*/
	if (GameToPlay == GAME_NORMAL && *this ==  STRUCT_MISSION && PlayerPtr->ActLike == HOUSE_BAD && Scenario == 10) {
		InfantryClass * i = new InfantryClass(INFANTRY_CHAN, House->Class->House);

		ScenarioInit++;
		if (i->Unlimbo(Center_Coord(), DIR_N)) {
			i->Trigger = TriggerClass::As_Pointer("win");
			i->Strength = Random_Pick(5, (int)i->Class->MaxStrength);
			ScenarioInit--;
			i->Scatter(0, true);
			ScenarioInit++;
			i->Assign_Mission(MISSION_GUARD_AREA);
		} else {
			delete i;
			PlayerPtr->Flag_To_Win();
		}
		ScenarioInit--;
	}


	/*
	**	Whoever a CrystAlien structure gives up, which is the same list whether it
	**	was sold or shot out from under its owner -- Building.destroy takes `sold`
	**	as an argument and never consults it for this.  See CAC_Spawn_Survivors.
	**
	**	The roll-per-cell below is Tiberian Dawn's, and it is a different idea
	**	entirely: a chance of one man on each cell of the footprint, always the same
	**	type.  It stays for GDI and Nod in the supplemental build.
	*/
	if (!House->IsToDie && !IsSurvivorless && CAC_Spawn_Survivors(this) >= 0) {
		IsSurvivorless = true;
	}

	/*
	**	Generate random survivors from the destroyed building.
	*/
	cell = Coord_Cell(Coord);
	offset = Occupy_List();
	int odds = 2;
	if (Target_Legal(WhomToRepay)) odds -= 1;
	if (IsCaptured) odds += 6;
	while (*offset != REFRESH_EOL) {
		CELL	newcell;

		newcell = cell + *offset++;

		/*
		**	Infantry could run out of a destroyed building.
		*/
		if (!House->IsToDie && !IsSurvivorless) {
			InfantryClass * i = NULL;

			if (Random_Pick(0, odds) == 1) {
				i = new InfantryClass(Crew_Type(), House->Class->House);
				if (i) {
					if (Class->Get_Buildup_Data() != NULL && i->Class->IsNominal) i->IsTechnician = true;
					ScenarioInit++;
					if (i->Unlimbo(Cell_Coord(newcell), DIR_N)) {
						i->Strength = Random_Pick(5, (int)i->Class->MaxStrength);
						i->Scatter(0, true);
						if (source != TARGET_NONE && !House->Is_Ally(As_Object(source))) {
							i->Assign_Mission(MISSION_ATTACK);
							i->Assign_Target(source);
						} else {
							if (House->IsHuman) {
								i->Assign_Mission(MISSION_GUARD);
							} else {
								i->Assign_Mission(MISSION_HUNT);
							}
						}
					} else {
						delete i;
					}
					ScenarioInit--;
				}
			}
		}

		/*
		**	Possibly add some smoke rising from the ashes of the building.
		*/
		switch (Random_Pick(0, 5)) {
			case 0:
			case 1:
			case 2:
				new AnimClass(ANIM_SMOKE_M, Coord_Scatter(Cell_Coord(newcell), 0x0050, false), Random_Pick(0, 5), Random_Pick(1, 2));
				break;

			default:
				break;
		}

		/*
		**	The building always scars the ground in some fashion.
		*/
		/*
		**	Same again for the debris trail: one scorch, never a crater.
		*/
		if (CAC_Is_CrystAlien_Type(Class->IniName)) {
			new SmudgeClass(SMUDGE_CAC_SCORCH, Cell_Coord(newcell));
		} else if (Random_Pick(0, 3) == 0) {
			new SmudgeClass(Random_Pick(SMUDGE_SCORCH1, SMUDGE_SCORCH6), Cell_Coord(newcell));
		} else {
			new SmudgeClass(Random_Pick(SMUDGE_CRATER1, SMUDGE_CRATER6), Coord_Scatter(Cell_Coord(newcell), 0x0080, false));
		}
	}
}


/***********************************************************************************************
 * BuildingClass::Active_Click_With -- Handles clicking on the map while the building is selected.*
 *                                                                                             *
 *    This interface routine handles when the player clicks on the map while this building     *
 *    is currently selected. This is used to assign an override target to a turret or          *
 *    guard tower.                                                                             *
 *                                                                                             *
 * INPUT:   target   -- The target that was clicked upon.                                      *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/28/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void BuildingClass::Active_Click_With(ActionType action, ObjectClass * object)
{
	Validate();
	if (action == ACTION_ATTACK) {
		Player_Assign_Mission(MISSION_ATTACK, object->As_Target());
	}

	if (action == ACTION_TOGGLE_PRIMARY && Class->IsFactory) {
		OutList.Add(EventClass(EventClass::PRIMARY, As_Target()));
	}
}


/***********************************************************************************************
 * BuildingClass::Active_Click_With -- Handles cell selection for buildings.                   *
 *                                                                                             *
 *    This routine really only serves one purpose -- to allow targeting of the ground for      *
 *    buildings that are euipped with weapons.                                                 *
 *                                                                                             *
 * INPUT:   action   -- The requested action to perform.                                       *
 *                                                                                             *
 *          cell     -- The cell location to perform the action upon.                          *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/04/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void BuildingClass::Active_Click_With(ActionType action, CELL cell)
{
	Validate();
	if (action == ACTION_ATTACK) {
		Player_Assign_Mission(MISSION_ATTACK, ::As_Target(cell));
	}

	if (action == ACTION_MOVE && *this == STRUCT_CONST) {
		OutList.Add(EventClass(EventClass::ARCHIVE, As_Target(), ::As_Target(cell)));
		OutList.Add(EventClass(EventClass::SELL, As_Target()));

		COORDINATE coord = Map.Pixel_To_Coord(Get_Mouse_X(), Get_Mouse_Y());
		OutList.Add(EventClass(ANIM_MOVE_FLASH, PlayerPtr->Class->House, coord, 1 << PlayerPtr->Class->House));
	}
}


/***********************************************************************************************
 * BuildingClass::Assign_Target -- Assigns a target to the building.                           *
 *                                                                                             *
 *    Assigning of a target to a building makes sense if the building is one that can attack.  *
 *    This routine would be used to assign the attack target to a turret or guard tower.       *
 *                                                                                             *
 * INPUT:   target   -- The target that was clicked on while this building was selected.       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/28/1994 JLB : Created.                                                                 *
 *   11/02/1994 JLB : Checks for range before assigning target.                                *
 *=============================================================================================*/
void BuildingClass::Assign_Target(TARGET target)
{
	Validate();
	if (*this != STRUCT_SAM && !In_Range(target, 0)) {
		target = TARGET_NONE;
	}

	TechnoClass::Assign_Target(target);
}


/***********************************************************************************************
 * BuildingClass::Init -- Initialize the building system to an empty null state.               *
 *                                                                                             *
 *    This routine initializes the building system in preparation for a scenario load.         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/19/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void BuildingClass::Init(void)
{
	BuildingClass *ptr;

	Buildings.Free_All();

	ptr = new BuildingClass();
	VTable = ((void **)(((char *)ptr) + sizeof(AbstractClass) - 4))[0];
	delete ptr;
}


/***********************************************************************************************
 * BuildingClass::Exit_Object -- Initiates an object to leave the building.                    *
 *                                                                                             *
 *    This function is used to cause an object to exit the building. It is called when a       *
 *    factory produces a vehicle or other mobile object and that object needs to exit the      *
 *    building to join the ranks of a regular unit. Typically, the object is placed down on    *
 *    the map such that it overlaps the building and then it is given a movement order so that *
 *    it will move to an adjacent free cell.                                                   *
 *                                                                                             *
 * INPUT:   base  -- Pointer to the object that is to exit the building.                       *
 *                                                                                             *
 * OUTPUT:  Returns the success rating for the exit attempt;                                   *
 *             0  = complete failure (refund money please)                                     *
 *             1  = temporarily prevented (try again later please)                             *
 *             2  = successful                                                                 *
 *                                                                                             *
 * WARNINGS:   The building is placed in radio contact with the object. The object is in a     *
 *             teathered condition. This condition will be automatically broken when the       *
 *             object reaches the adjacent square.                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/28/1994 JLB : Created.                                                                 *
 *   04/10/1995 JLB : Handles building production by computer.                                 *
 *   06/17/1995 JLB : Handles refinery exit.                                                   *
 *=============================================================================================*/
int BuildingClass::Exit_Object(TechnoClass * base)
{
	Validate();
	if (!base) return(0);

	TechnoTypeClass const * ttype = (TechnoTypeClass const *)&base->Class_Of();

	/*
	**	A unit exiting a building is always considered to be "locked". That means, it
	**	will be considered as to have legally entered the visible map domain.
	*/
	base->IsLocked = true;

	/*
	**	Find a good cell to unload the object to. The object, probably a vehicle
	**	will drive/walk to the adjacent free cell.
	*/
	/*
	**	How the object gets out depends on the building, and the switch below
	**	names the engine's own slots to say so.  A CrystAlien building has a slot
	**	of its own and so was named by none of them, which means Exit_Object fell
	**	through and returned zero -- and a zero is a unit that never leaves, which
	**	is what "Ready" that never clears looks like.  See CAC_Exit_Style.
	*/
	StructType style = Class->Type;
	{
		StructType cacstyle = CAC_Exit_Style(Class->Type);
		if (cacstyle != STRUCT_NONE) style = cacstyle;
	}

	switch (base->What_Am_I()) {
		CELL cell;

		case RTTI_AIRCRAFT:
			/*
			**	A CrystAlien station puts its aircraft on the ground in front of itself.
			**
			**	The branch below is the engine's, and its second half flies the aircraft in
			**	from off the edge of the map -- which is where the Astro Fighters were coming
			**	from.  It is not an odd case either: the first one out tethers itself to the
			**	station, so every one after it finds the station in radio contact and takes
			**	that path.  One fighter appeared at the Radar Station and the rest flew in.
			**
			**	CrystAlien has no such thing.  A fighter is placed by the same
			**	closestAvailable every other unit is placed by, and because it never claims a
			**	tile the next one is handed the same one -- so they gather in front of the
			**	station and sit in each other.  No tether, or the second would be back to
			**	flying in from the map edge.
			*/
			if (CAC_Exit_Style(Class->Type) != STRUCT_NONE) {
				AircraftClass * air = (AircraftClass *)base;
				CELL spot = CAC_Exit_Cell(this, air);

				/*
				**	In the air, which is where the constructor already put it.
				**
				**	The engine's own path sets Altitude to zero first, because it is landing
				**	the craft on a helipad.  Doing that here put a transport flat on a tile
				**	chosen without asking whether anything was standing on it -- aircraft
				**	deliberately skip that test, so that they stack -- and a landed aircraft
				**	and a vehicle both claiming one cell is not a thing the occupier list is
				**	prepared for.
				**
				**	It is also not what CrystAlien does.  A new flier has landerPerc at zero,
				**	which is maxAltitude, so it comes into being flying; a lander settles later
				**	and only once it is idle at home.  AircraftClass sets FLIGHT_LEVEL in its
				**	own constructor, so the way to get that is to leave it alone.
				*/
				ScenarioInit++;
				if (air->Unlimbo(Cell_Coord(spot), (DirType)(Random_Pick(0, 7) * 32))) {
					ScenarioInit--;

					/*
					**	And stopped, because Unlimbo starts it moving.
					**
					**	    if (Altitude == FLIGHT_LEVEL) Set_Speed(0xFF); else Set_Speed(0);
					**
					**	It presumes anything unlimboed at flight level is arriving from
					**	somewhere and wants to be under way.  These are not: they have just
					**	been built and belong over the building that built them.  Full speed
					**	on the random facing chosen above, with no destination to stop at,
					**	flew every new JetPack Explorer off the edge of the map in a straight
					**	line.
					**
					**	CrystAlien's new flier has no path at all -- landerPerc 0, which is
					**	full altitude -- so it hovers where it was made until told otherwise.
					*/
					air->Set_Speed(0);
					air->Assign_Destination(TARGET_NONE);
					air->Assign_Target(TARGET_NONE);
					air->Assign_Mission(MISSION_GUARD);

					CAC_Log("exit", "%s hovers %s at %d,%d (altitude %d)",
								Class->IniName, air->Class->IniName,
								Cell_X(spot), Cell_Y(spot), air->Altitude);

					return(2);
				}
				ScenarioInit--;
				break;
			}

			if (!In_Radio_Contact()) {
				AircraftClass *air = (AircraftClass *)base;

				air->Altitude = 0;
				ScenarioInit++;
				if (air->Unlimbo(Docking_Coord(), air->Pose_Dir())) {
					Transmit_Message(RADIO_HELLO, air);
					Transmit_Message(RADIO_TETHER);
					ScenarioInit--;
					return(2);
				}
				ScenarioInit--;
			} else {
				AircraftClass *air = (AircraftClass *)base;

				if (Cell_X(Coord_Cell(Center_Coord())) - Map.MapCellX < Map.MapCellWidth/2) {
					cell = XY_Cell(Map.MapCellX-1, Random_Pick(0, Map.MapCellHeight-1)+Map.MapCellY);
				} else {
					cell = XY_Cell(Map.MapCellX+Map.MapCellWidth, Random_Pick(0, Map.MapCellHeight-1)+Map.MapCellY);
				}
				ScenarioInit++;
				if (air->Unlimbo(Cell_Coord(cell), DIR_N)) {
					air->Assign_Destination(::As_Target(Nearby_Location(air)));
					air->Assign_Mission(MISSION_MOVE);
					ScenarioInit--;
					return(2);
				}
				ScenarioInit--;
			}
			break;

		case RTTI_INFANTRY:
		case RTTI_UNIT:
			switch (style) {
				case STRUCT_REFINERY:
					if (base->What_Am_I() == RTTI_UNIT) {
						CELL cell = Coord_Cell(Center_Coord());
						UnitClass * unit = (UnitClass *)base;

						cell = Adjacent_Cell(cell, FACING_SW);
						ScenarioInit++;
						if (unit->Unlimbo(Coord_Add(unit->Coord, 0x00550060L), DIR_SW_X2)) {
							unit->PrimaryFacing = DIR_SW_X2;
							Transmit_Message(RADIO_HELLO, unit);
							Transmit_Message(RADIO_TETHER);
							unit->Assign_Mission(MISSION_HARVEST);
							unit->Force_Track(DriveClass::OUT_OF_REFINERY, Cell_Coord(cell));
							unit->Set_Speed(128);
						}
						ScenarioInit--;
					} else {
						base->Scatter(true);
					}
					break;

				case STRUCT_AIRSTRIP:
					if (Create_Special_Reinforcement(House, &AircraftTypeClass::As_Reference(AIRCRAFT_CARGO), ttype, TMISSION_UNLOAD, As_Target())) {
						delete base;
						return(2);
					}
					return(0);

				case STRUCT_WEAP:
					ScenarioInit++;
					if (base->Unlimbo(Coord_Add(Coord, Class->ExitPoint), DIR_SW)) {
//						base->Assign_Mission(MISSION_MOVE);
//						base->Assign_Destination(::As_Target(As_Cell(Coord)+MAP_CELL_W*2));
						base->Mark(MARK_UP);
						base->Coord = Coord_Add(Coord, Class->ExitPoint);
						base->Mark(MARK_DOWN);
						Transmit_Message(RADIO_HELLO, base);
						Transmit_Message(RADIO_TETHER);
						Assign_Mission(MISSION_UNLOAD);
						ScenarioInit--;
						return(2);
					}
					ScenarioInit--;
					break;

				/*
				**	Every CrystAlien host building arrives here, by way of
				**	CAC_Exit_Style: Building.spawn puts the new object at the dock and
				**	lets it drive away, which is what this does.
				*/
				case STRUCT_BARRACKS:
				case STRUCT_HAND:
					CELL cell;
					bool found = false;

					/*
					**	A CrystAlien building places its own, and cannot decline to.
					**
					**	The walk below is the engine's: it picks a cell from the exit list and then
					**	puts the object down at the building's fixed exit point to drive there.  If
					**	something is standing on that point the Unlimbo fails and Exit_Object returns
					**	zero -- the factory holds a finished unit until the way is clear, which is
					**	the waiting for the bib to empty.
					**
					**	CrystAlien never waits, because it never uses a fixed point: closestAvailable
					**	hands back a tile that is free and the unit is put there.  The one after it
					**	finds that tile taken and is given the next along the ring, so a queue comes
					**	out in a spread rather than one at a time through a door.
					**
					**	The facing is CrystAlien's too -- `0.125 * random(8)`, one of eight.
					*/
					if (CAC_Exit_Style(Class->Type) != STRUCT_NONE) {
						cell = CAC_Exit_Cell(this, base);

						ScenarioInit++;
						if (base->Unlimbo(Cell_Coord(cell), (DirType)(Random_Pick(0, 7) * 32))) {

							/*
							**	A carrier goes to work the moment it is out, as it does below.
							*/
							if (base->What_Am_I() == RTTI_UNIT &&
								((UnitClass *)base)->Class->IsToHarvest) {
								base->Assign_Mission(MISSION_HARVEST);
							}
							ScenarioInit--;
							return(2);
						}
						ScenarioInit--;
						break;
					}

					cell = Find_Exit_Cell(base);
					if (cell) found = true;

					/*
					**	And if every cell on the exit list is taken, anywhere nearby.
					**
					**	The list is a ring of cells around the building, so a structure
					**	with neighbours on every side has none of them free and this
					**	returns zero -- which means the object never leaves the factory at
					**	all.  CrystAlien's buildings are small and players pack them
					**	tight, and a computer player packs them tighter still, so its
					**	factories could stop producing entirely once the base filled in.
					*/
					if (!found) {
						for (int radius = 2; radius <= 4 && !found; radius++) {
							for (FacingType face = FACING_N; face < FACING_COUNT && !found; face++) {
								CELL look = Coord_Cell(Coord_Move(Center_Coord(), Facing_Dir(face),
																		 	 (unsigned)(radius * CELL_LEPTON_W)));
								if (!Map.In_Radar(look)) continue;
								if (base->Can_Enter_Cell(look) != MOVE_OK) continue;
								cell = look;
								found = true;
							}
						}
					}

#ifdef OBSOLETE
					CELL	const *ptr;
					bool	found = false;

					ptr = Class->ExitList;
					while (*ptr != REFRESH_EOL) {
						cell = Coord_Cell(Coord) + *ptr++;
						if (base->Can_Enter_Cell(cell) == MOVE_OK) {
							found = true;
							break;
						}
					}
#endif

					if (found) {
						DirType	dir = Direction(cell);
						COORDINATE		start = Coord_Add(Coord, Class->ExitPoint);

						ScenarioInit++;
						if (base->Unlimbo(start, dir)) {

							base->Assign_Mission(MISSION_MOVE);
							base->Assign_Destination(::As_Target(cell));

							/*
							**	A carrier goes to work the moment it is out.  Leaving on a
							**	move order, it depends on the tether being cut in one
							**	particular way to pick up MISSION_HARVEST -- the building has
							**	to answer RADIO_UNLOADED with RADIO_RUN_AWAY -- and on any
							**	other answer it falls back on an ArchiveTarget it does not have
							**	yet, and stands beside the building.  A player clicks a crystal
							**	field and never notices; a computer player does not, and then
							**	has no income at all.  The engine's own refinery assigns the
							**	mission outright as the harvester rolls out, so do that.
							*/
							if (base->What_Am_I() == RTTI_UNIT &&
								((UnitClass *)base)->Class->IsToHarvest) {
								base->Assign_Mission(MISSION_HARVEST);
							}

							/*
							**	Establish radio contact so unload coordination can occur. This
							**	radio contact should always succeed.
							*/
							if (Transmit_Message(RADIO_HELLO, base) == RADIO_ROGER) {
								Transmit_Message(RADIO_UNLOAD);
							}
							ScenarioInit--;
							return(2);
						}
						ScenarioInit--;
					}
					break;
			}
			break;

		case RTTI_BUILDING:

			if (!House->IsHuman) {
				/*
				**	Find the next available spot to place this newly created building. If the
				**	building could be placed at the desired location, fine. If not, then this
				**	routine will return failure. The calling routine will probably abandon this
				**	building in preference to building another.
				*/
				
				BaseNodeClass * node = Base.Next_Buildable(((BuildingClass *)base)->Class->Type);
				// Replaced with RA AI functioality. ST - 7/25/2019 4:14PM
#ifndef USE_RA_AI
				if (node) {
					if (Flush_For_Placement(base, Coord_Cell(node->Coord))) {
						return(1);
					}
					if (base->Unlimbo(node->Coord)) {
						return(2);
					}
				}
#else
				if (GameToPlay == GAME_NORMAL) {
					if (node) {
						if (Flush_For_Placement(base, Coord_Cell(node->Coord))) {
							return(1);
						}
						if (base->Unlimbo(node->Coord)) {
							return(2);
						}
					}
				} else {
					
					COORDINATE coord = 0;
					if (node) {
						coord = node->Coord;
					} else {

						/*
						**	Find a suitable new spot to place.
						*/
						coord = House->Find_Build_Location((BuildingClass *)base);
					}

					if (coord) {
						if (Flush_For_Placement(base, Coord_Cell(coord))) {
							return(1);
						}
						if (base->Unlimbo(coord)) {
							if (node && ((BuildingClass *)base)->Class->Type == House->BuildStructure) {
								House->BuildStructure = STRUCT_NONE;
							}
							return(2);
						}
					}
				}
#endif
			}
			break;
	}

	/*
	**	Failure to exit the object results in a false return value.
	*/
	return(0);
}





/***********************************************************************************************
 * CAC_Sidebar_Add -- Put a hosted type on whichever sidebar this game actually shows.          *
 *                                                                                             *
 *    There are two sidebars and a game only ever shows one.  A skirmish reads the per-player  *
 *    Glyphx strips (MultiplayerSidebars); a campaign reads the stock one, Map.Column -- see   *
 *    DLLExportClass::Get_Sidebar_State, which switches on GAME_NORMAL.  Tiberian Dawn's own   *
 *    loops in Update_Buildables pick between Sidebar_Glyphx_Add and Map.Add accordingly.       *
 *                                                                                             *
 *    The loops CrystAlien added for hosted types -- the Miner that only comes off the Command *
 *    Base, infantry that only comes off a Training Camp, and the rest of CACHOSTS.INC -- went   *
 *    straight to Sidebar_Glyphx_Add, and the stock branch below them skips hosted types on     *
 *    purpose.  So in a campaign they reached a sidebar nobody was looking at and were never    *
 *    offered at all: "the levels don't properly update your build trees".                      *
 *=============================================================================================*/
static void CAC_Sidebar_Add(RTTIType type, int id, HouseClass *house, bool via_capture)
{
	if (GameToPlay == GAME_GLYPHX_MULTIPLAYER) {
		Sidebar_Glyphx_Add(type, id, house, via_capture);
	} else {
		Map.Add(type, id, via_capture);
	}
}


/***********************************************************************************************
 * BuildingClass::Update_Buildables -- Informs sidebar of additional construction options.     *
 *                                                                                             *
 *    This routine will tell the sidebar of objects that can be built. The function is called  *
 *    whenever a building matures.                                                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/11/1994 JLB : Created.                                                                 *
 *   12/23/1994 JLB : Only updates for PLAYER buildings.                                       *
 *=============================================================================================*/
void BuildingClass::Update_Buildables(void)
{
	Validate();
	
	/*
	** Only do this for real human players. ST - 3/22/2019 1:38PM
	*/
	if (House != PlayerPtr) {
		if (GameToPlay != GAME_GLYPHX_MULTIPLAYER || House->IsHuman == false) {
			return;
		}
	}

	bool buildable_via_capture = (IsCaptured && ActLike != House->ActLike) ? true : false;

	/*
	**	The engine's own answer, unchanged.
	**
	**	This used to be forced false for CrystAlien's types, on the reasoning that the
	**	flag sends the client looking for the other faction's version of a shared
	**	cameo and CrystAlien has no shared cameos.  But Tiberian Dawn has almost no
	**	shared cameos either -- GDI and Nod build very nearly the same list, and the
	**	few that differ, the Hand of Nod among them, have a picture each.  So the flag
	**	cannot mean what I took it to mean, and saying "this is one of mine" about an
	**	Astro Fighter an Alien player captured a station for is simply false.
	**
	**	It is a captured cross-faction buildable.  Say so, and let the client do
	**	whatever it does with that.
	*/

	if (!IsInLimbo && Is_Discovered_By_Player()) {
		/*
		**	CrystAlien produces a unit at its `home` building, which is not
		**	always the factory Tiberian Dawn would pick: the Miner comes out
		**	of the Command Base, the JetPack Explorer out of the Training
		**	Camp, the Dropship out of the Vehicle Factory, the Switch Fighter
		**	out of the Ops Ship.  Offer whatever this building hosts.
		*/
		for (UnitType hu = UNIT_FIRST; hu < UNIT_COUNT; hu++) {
			if (CAC_Hosts_Unit(Class->Type, RTTI_UNITTYPE, hu) && PlayerPtr->Can_Build(hu, ActLike)) {
				CAC_Sidebar_Add(RTTI_UNITTYPE, hu, House, buildable_via_capture);
			}
		}
		for (InfantryType hi = INFANTRY_FIRST; hi < INFANTRY_COUNT; hi++) {
			if (CAC_Hosts_Unit(Class->Type, RTTI_INFANTRYTYPE, hi) && PlayerPtr->Can_Build(hi, ActLike)) {
				CAC_Sidebar_Add(RTTI_INFANTRYTYPE, hi, House, buildable_via_capture);
			}
		}
		for (AircraftType ha = AIRCRAFT_FIRST; ha < AIRCRAFT_COUNT; ha++) {
			if (CAC_Hosts_Unit(Class->Type, RTTI_AIRCRAFTTYPE, ha) && PlayerPtr->Can_Build(ha, ActLike)) {
				CAC_Sidebar_Add(RTTI_AIRCRAFTTYPE, ha, House, buildable_via_capture);
			}
		}

		switch (Class->ToBuild) {
			StructType i;
			UnitType u;
			InfantryType f;
			AircraftType a;

			case RTTI_BUILDINGTYPE:
				for (i = STRUCT_FIRST; i < STRUCT_COUNT; i++) {
					if (PlayerPtr->Can_Build(i, ActLike)) {
//						if (BuildingTypeClass::As_Reference(i).Who_Can_Build_Me(true, true, ActLike)) {
							if (GameToPlay == GAME_GLYPHX_MULTIPLAYER) {
								Sidebar_Glyphx_Add(RTTI_BUILDINGTYPE, i, House, buildable_via_capture);
							} else {
								Map.Add(RTTI_BUILDINGTYPE, i, buildable_via_capture);
							}
//						}
					}
				}
				break;

			case RTTI_UNITTYPE:
				for (u = UNIT_FIRST; u < UNIT_COUNT; u++) {
					/*  A hosted type is offered by its home building, above, and by
				**  nothing else. */
				if (!CAC_Has_Host(RTTI_UNITTYPE, u) && PlayerPtr->Can_Build(u, ActLike)) {
//						if (UnitTypeClass::As_Reference(u).Who_Can_Build_Me(true, true, ActLike)) {
							if (GameToPlay == GAME_GLYPHX_MULTIPLAYER) {
								Sidebar_Glyphx_Add(RTTI_UNITTYPE, u, House, buildable_via_capture);
							} else {
								Map.Add(RTTI_UNITTYPE, u, buildable_via_capture);
							}
//						}
					}
				}
				break;

			case RTTI_INFANTRYTYPE:
				for (f = INFANTRY_FIRST; f < INFANTRY_COUNT; f++) {
					/*  A hosted type is offered by its home building, above, and by
				**  nothing else. */
				if (!CAC_Has_Host(RTTI_INFANTRYTYPE, f) && PlayerPtr->Can_Build(f, ActLike)) {
//						if (InfantryTypeClass::As_Reference(f).Who_Can_Build_Me(true, true, ActLike)) {
							if (GameToPlay == GAME_GLYPHX_MULTIPLAYER) {
								Sidebar_Glyphx_Add(RTTI_INFANTRYTYPE, f, House, buildable_via_capture);
							} else {
								Map.Add(RTTI_INFANTRYTYPE, f, buildable_via_capture);
							}
//						}
					}
				}
				break;

			case RTTI_AIRCRAFTTYPE:
				for (a = AIRCRAFT_FIRST; a < AIRCRAFT_COUNT; a++) {
					/*  A hosted type is offered by its home building, above, and by
				**  nothing else. */
				if (!CAC_Has_Host(RTTI_AIRCRAFTTYPE, a) && PlayerPtr->Can_Build(a, ActLike)) {
//						if (AircraftTypeClass::As_Reference(a).Who_Can_Build_Me(true, true, ActLike)) {
							if (GameToPlay == GAME_GLYPHX_MULTIPLAYER) {
								Sidebar_Glyphx_Add(RTTI_AIRCRAFTTYPE, a, House, buildable_via_capture);
							} else {
								Map.Add(RTTI_AIRCRAFTTYPE, a, buildable_via_capture);
							}
//						}
					}
				}
				break;
		}
	}
}

#if (0)	//ST - 3/22/2019 1:33PM
/***********************************************************************************************
 * BuildingClass::Update_Buildables -- Informs sidebar of additional construction options.     *
 *                                                                                             *
 *    This routine will tell the sidebar of objects that can be built. The function is called  *
 *    whenever a building matures.                                                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/11/1994 JLB : Created.                                                                 *
 *   12/23/1994 JLB : Only updates for PLAYER buildings.                                       *
 *=============================================================================================*/
void BuildingClass::Update_Buildables(void)
{
	Validate();

	if (House == PlayerPtr && !IsInLimbo && IsDiscoveredByPlayer) {
		switch (Class->ToBuild) {
			StructType i;
			UnitType u;
			InfantryType f;
			AircraftType a;

			case RTTI_BUILDINGTYPE:
				for (i = STRUCT_FIRST; i < STRUCT_COUNT; i++) {
					if (PlayerPtr->Can_Build(i, ActLike)) {
//						if (BuildingTypeClass::As_Reference(i).Who_Can_Build_Me(true, true, ActLike)) {
							Map.Add(RTTI_BUILDINGTYPE, i);
//						}
					}
				}
				break;

			case RTTI_UNITTYPE:
				for (u = UNIT_FIRST; u < UNIT_COUNT; u++) {
					if (PlayerPtr->Can_Build(u, ActLike)) {
//						if (UnitTypeClass::As_Reference(u).Who_Can_Build_Me(true, true, ActLike)) {
							Map.Add(RTTI_UNITTYPE, u);
//						}
					}
				}
				break;

			case RTTI_INFANTRYTYPE:
				for (f = INFANTRY_FIRST; f < INFANTRY_COUNT; f++) {
					if (PlayerPtr->Can_Build(f, ActLike)) {
//						if (InfantryTypeClass::As_Reference(f).Who_Can_Build_Me(true, true, ActLike)) {
							Map.Add(RTTI_INFANTRYTYPE, f);
//						}
					}
				}
				break;

			case RTTI_AIRCRAFTTYPE:
				for (a = AIRCRAFT_FIRST; a < AIRCRAFT_COUNT; a++) {
					if (PlayerPtr->Can_Build(a, ActLike)) {
//						if (AircraftTypeClass::As_Reference(a).Who_Can_Build_Me(true, true, ActLike)) {
							Map.Add(RTTI_AIRCRAFTTYPE, a);
//						}
					}
				}
				break;
		}
	}
}
#endif

/***********************************************************************************************
 * BuildingClass::Fire_Out -- Handles when attached animation expires.                         *
 *                                                                                             *
 *    This routine is used to perform any fixups necessary when the attached animation has     *
 *    terminated. This occurs when the fire & smoke animation that a SAM site produces stops.  *
 *    At that point, normal reload procedures can commence.                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/30/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void BuildingClass::Fire_Out(void)
{
	Validate();
//	SAM = SAM_READY;
//	IsFiring = false;
}


/***********************************************************************************************
 * BuildingClass::Limbo -- Handles power adjustment as building goes into limbo.               *
 *                                                                                             *
 *    This routine will handle the power adjustments for the associated house when the         *
 *    building goes into limbo. This means that its power drain or production is subtracted    *
 *    from the house accumulated totals.                                                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the building limboed?                                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool BuildingClass::Limbo(void)
{
	Validate();
//	HouseClass *housep;
//	RTTIType bld_type;

	if (!IsInLimbo) {

		/*
		**	Update the total factory type, assuming this building has a factory.
		*/
		/*
		**	A building that hosts a type counts as a factory of that kind,
		**	whatever its own ToBuild says.  See CAC_Builds_Any.
		*/
		/*
		**	Whatever this building was making goes with it.  See Abandon_Host.
		*/
		if (House != NULL) House->Abandon_Host(Class->Type);

		if (CAC_Builds_Any(Class->Type, RTTI_UNITTYPE)) House->UnitFactories--;
		if (CAC_Builds_Any(Class->Type, RTTI_INFANTRYTYPE)) House->InfantryFactories--;
		if (CAC_Builds_Any(Class->Type, RTTI_AIRCRAFTTYPE)) House->AircraftFactories--;

		switch (Class->ToBuild) {
			case RTTI_AIRCRAFTTYPE:
				House->AircraftFactories--;
				break;

			case RTTI_INFANTRYTYPE:
				House->InfantryFactories--;
				break;

			case RTTI_UNITTYPE:
				House->UnitFactories--;
				break;

			case RTTI_BUILDINGTYPE:
				House->BuildingFactories--;
				break;

			default:
				break;
		}

		House->IsRecalcNeeded = true;
		
#ifdef USE_RA_AI
		//
		// Added for RA AI in TD. ST - 7/26/2019 9:25AM
		//
		House->Recalc_Center();
#endif
		/*
		**	Update the power status of the owner's house.
		*/
		House->Adjust_Power(-Power_Output());
		House->Adjust_Drain(-Class->Drain);
		House->Adjust_Capacity(-(int)Class->Capacity, true);
		if (House == PlayerPtr) {
			Map.PowerClass::IsToRedraw = true;
			Map.Flag_To_Redraw(false);
		}

#ifdef OBSOLETE
		/*
		**	If this building is building something, the FactoryClass doing the
		**	building must be deleted, as well as the object being built.
		**	- If this building is controlled by Computer AI, this building's Factory
		**	  pointer will point to the factory doing the building.
		**	- Otherwise, the owner House's Factory indices will indicate what's
		**	  being built.  (The player's sidebar also contains indices to indicate
		**	  what's being built, but those just echo the house's indices.)
		*/
		if (Factory) {
			Factory->Abandon();
			delete Factory;
			Factory = 0;
		}

		/*
		** If the owner HouseClass is building something, and this building can
		** build that thing, we may be the last building for that house that can
		** build that thing; if so, abandon production of it.
		*/
		housep = HouseClass::As_Pointer(Owner());
		if (housep) {
			FactoryClass * factory = 0;
			bld_type = RTTI_NONE;
			if (housep->AircraftFactory != -1 && Class->ToBuild == RTTI_AIRCRAFTTYPE) {
				bld_type = RTTI_AIRCRAFTTYPE;
				factory = Factories.Raw_Ptr(housep->AircraftFactory);
			}
			if (housep->InfantryFactory != -1 && Class->ToBuild==RTTI_INFANTRYTYPE) {
				bld_type = RTTI_INFANTRYTYPE;
				factory = Factories.Raw_Ptr(housep->InfantryFactory);
			}
			if (housep->UnitFactory != -1 && Class->ToBuild==RTTI_UNITTYPE) {
				bld_type = RTTI_UNITTYPE;
				factory = Factories.Raw_Ptr(housep->UnitFactory);
			}
			if (housep->BuildingFactory != -1 && Class->ToBuild==RTTI_BUILDINGTYPE) {
				bld_type = RTTI_BUILDINGTYPE;
				factory = Factories.Raw_Ptr(housep->BuildingFactory);
			}
			if (housep->SpecialFactory != -1 && Class->ToBuild==RTTI_SPECIAL) {
				bld_type = RTTI_SPECIAL;
			}

			if (bld_type != RTTI_NONE) {
				if (factory) {
					TechnoClass * object = factory->Get_Object();
					IsInLimbo = true;
					if (object && !object->Techno_Type_Class()->Who_Can_Build_Me(true, false, housep->Class->House)) {
						housep->Abandon_Production(bld_type);
					}
					IsInLimbo = false;
				}
			}
		}
#endif

		/*
		**	This could be a building that builds. If so, then the sidebar may need adjustment.
		** Set IsInLimbo to true to "fool" the sidebar into knowing that this building
		** isn't available.  Set it back to false so the rest of the Limbo code works.
		** Otherwise, the sidebar won't properly remove non-available buildables.
		*/
		if (IsOwnedByPlayer && !ScenarioInit) {
			IsInLimbo = true;
			Map.Recalc();
			IsInLimbo = false;
		}
#ifdef NEVER
		if (!House->IsHuman) {
			Update_Specials();
		}
#endif

	}
	return(TechnoClass::Limbo());
}


/***********************************************************************************************
 * BuildingClass::Fire_Coord -- Calculates the coordinate that projectile would appear.        *
 *                                                                                             *
 *    This routine is used to determine where a projectile would appear if this building       *
 *    were to fire. The location usually depends on the current rotation setting of the        *
 *    turret or else on the direction of the target that will be fired upon.                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with coordinate that the projectile should appear at if the building       *
 *          were to fire.                                                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
FireDataType BuildingClass::Fire_Data(int ) const
{
	Validate();
	COORDINATE coord = Center_Coord();
	int dist = 0;

	/*
	**	Make adjustments to the firing coordinate to account for turret
	**	position. This depends on building type and turret facing.
	*/
	switch (Class->Type) {
		default:
		case STRUCT_GTOWER:
		case STRUCT_ATOWER:
			coord = Coord_Move(coord, DIR_N, 0x0030);
			dist = 0x0040;
			break;
		case STRUCT_OBELISK:
			coord = Coord_Move(coord, DIR_N, 0x00A8);
			coord = Coord_Move(coord, DIR_W, 0x0018);
			break;

		case STRUCT_SAM:
		case STRUCT_TURRET:
			coord = Coord_Move(coord, DIR_N, 0x0030);
			dist = 0x0080;
			break;
	}

	return{coord,dist};
}


COORDINATE BuildingClass::Fire_Coord(int ) const
{
	Validate();
	COORDINATE coord = Center_Coord();			// Center of firing building.

	/*
	**	Make adjustments to the firing coordinate to account for turret
	**	position. This depends on building type and turret facing.
	*/
	switch (Class->Type) {
		default:
		case STRUCT_GTOWER:
		case STRUCT_ATOWER:
			coord = Coord_Move(coord, DIR_N, 0x0030);
			if (Target_Legal(TarCom)) {
				coord = Coord_Move(coord, ::Direction(coord, As_Coord(TarCom)), 0x0040);
			}
			break;
		case STRUCT_OBELISK:
			coord = Coord_Move(coord, DIR_N, 0x00A8);
//			coord = Coord_Move(coord, DIR_N, 0x0058);
			coord = Coord_Move(coord, DIR_W, 0x0018);
			break;

		case STRUCT_SAM:
		case STRUCT_TURRET:
			coord = Coord_Move(coord, DIR_N, 0x0030);
			coord = Coord_Move(coord, PrimaryFacing.Current(), 0x0080);
			break;
	}

	return(coord);
}


/***********************************************************************************************
 * BuildingClass::Greatest_Threat -- Searches for target that building can fire upon.          *
 *                                                                                             *
 *    This routine intercepts the Greatest_Threat function so that it can add the ability      *
 *    to search for ground targets, if this isn't a SAM site.                                  *
 *                                                                                             *
 * INPUT:   threat   -- The base threat control value. Typically, it might be THREAT_RANGE     *
 *                      or THREAT_NORMAL.                                                      *
 *                                                                                             *
 * OUTPUT:  Returns with a suitable target. If none could be found, then TARGET_NONE is        *
 *          returned instead.                                                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/01/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
TARGET BuildingClass::Greatest_Threat(ThreatType threat) const
{
	Validate();
	if (*this != STRUCT_SAM) {
		threat = threat | (THREAT_INFANTRY|THREAT_BOATS|THREAT_VEHICLES|THREAT_RANGE);

		if (!House->IsHuman) {
			threat = threat | THREAT_BUILDINGS;
		}
//		threat = threat & ~THREAT_AIR;
	} else {
		threat = threat | THREAT_AREA;
	}

	if (Class->Primary != WEAPON_NONE && BulletTypeClass::As_Reference(Weapons[Class->Primary].Fires).IsAntiAircraft) {
		threat = threat | THREAT_AIR;
	}
	return(TechnoClass::Greatest_Threat(threat));
}


/***********************************************************************************************
 * BuildingClass::Grand_Opening -- Handles construction completed special operations.          *
 *                                                                                             *
 *    This routine is called when construction has finished. Typically, this enables           *
 *    new production options for factories.                                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/08/1995 JLB : Created.                                                                 *
 *   06/13/1995 JLB : Added helipad.                                                           *
 *=============================================================================================*/
void BuildingClass::Grand_Opening(bool captured)
{
	Validate();

	/*
	**	Whose it is, now it is anybody's.
	**
	**	"A bot was able to build a Driller without the Ops Ship" could not be
	**	answered from the log, because nothing in it said who owned a finished
	**	structure: the first Astro Ops Ship appears at one frame and Multi5 is
	**	offered a Driller the frame before, which fits Multi5 placing its own and
	**	fits nothing else in particular either.  This fires when a structure is
	**	finished and when it is captured, which are the two ways a house comes to
	**	own one, so the next question like that has an answer on the page.
	*/
	if (Class != NULL && House != NULL && House->Class != NULL &&
		 CAC_Is_CrystAlien_Type(Class->IniName)) {
		CAC_Log("owner", "%s's %s %s at %d,%d",
					House->Class->IniName, Class->IniName,
					captured ? "changes hands" : "opens",
					Cell_X(Coord_Cell(Coord)), Cell_Y(Coord_Cell(Coord)));
	}

	/*
	**	Adjust the owning house according to the power, drain, and Tiberium capacity that
	**	this building has.
	*/
	House->Adjust_Drain(Class->Drain);
	House->Adjust_Capacity(Class->Capacity);
	House->IsRecalcNeeded = true;

	/*	SPECIAL CASE:
	**	Refineries get a free harvester. Add a harvester to the reinforcement list
	**	at this time.
	*/
	if (CAC_Is_Stock_Refinery(Class->Type) && !ScenarioInit && !captured && !Debug_Map && (!House->IsHuman || PurchasePrice == 0 || PurchasePrice > Class->Raw_Cost())) {
		CELL cell = Coord_Cell(Adjacent_Cell(Center_Coord(), DIR_SW));
//		if (!Map[cell].Cell_Unit()) {
			UnitClass * unit = new UnitClass(UNIT_HARVESTER, House->Class->House);
			if (unit) {

				/*
				**	Try to place down the harvesters. If it could not be placed, then try
				**	to place it in a nearby location.
				*/
				if (!unit->Unlimbo(Cell_Coord(cell), DIR_SW)) {
					cell = Nearby_Location(unit);

					/*
					**	If the harvester could still not be placed, then refund the money
					**	to the owner and then bail.
					*/
					if (!unit->Unlimbo(Cell_Coord(cell), DIR_SW)) {
						House->Refund_Money(unit->Class->Cost_Of());
						delete unit;
					}
				}
			} else {

				/*
				**	If the harvester could not be created in the first place, then give
				**	the full refund price to the owning player.
				*/
				House->Refund_Money(UnitTypeClass::As_Reference(UNIT_HARVESTER).Cost_Of());
			}
		}
//	}

	/*
	**	Helicopter pads get a free attack helicopter.
	*/
	if (*this == STRUCT_HELIPAD && !captured && (!House->IsHuman || PurchasePrice == 0 || PurchasePrice > Class->Raw_Cost())) {
		ScenarioInit++;
		AircraftClass * air = 0;
		if (House->ActLike == HOUSE_GOOD) {
			air = new AircraftClass(AIRCRAFT_ORCA, House->Class->House);
		} else {
			air = new AircraftClass(AIRCRAFT_HELICOPTER, House->Class->House);
		}
		if (air) {
			air->Altitude = 0;
			if (air->Unlimbo(Docking_Coord(), air->Pose_Dir())) {
				air->Assign_Mission(MISSION_GUARD);
				air->Transmit_Message(RADIO_HELLO, this);
				Transmit_Message(RADIO_TETHER);
			}
		}
		ScenarioInit--;
	}
}


/***********************************************************************************************
 * BuildingClass::Repair -- Initiates or terminates the repair process.                        *
 *                                                                                             *
 *    This routine will start, stop, or toggle the repair process. When a building repairs, it *
 *    occurs incrementally over time.                                                          *
 *                                                                                             *
 * INPUT:   control  -- Determines how to control the repair process.                          *
 *                      0: Turns repair process off (if it was on).                            *
 *                      1: Turns repair process on (if it was off).                            *
 *                      -1:Toggles repair process to other state.                              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void BuildingClass::Repair(int control)
{
	Validate();
	switch (control) {
		case -1:
			IsRepairing = (IsRepairing == false);
			break;

		case 1:
			if (IsRepairing) return;
			IsRepairing = true;
			break;

		case 0:
			if (!IsRepairing) return;
			IsRepairing = false;
			break;
	}

	/*
	**	At this point, we know that the repair state has changed. Perform
	**	appropriate action.
	*/
	VocType sound = VOC_NONE;
	if (IsRepairing) {
		if (Strength == Class->MaxStrength) {
			sound = VOC_SCOLD;
		} else {
			sound = VOC_BUTTON;
			Clicked_As_Target(PlayerPtr->Class->House); // 2019/09/20 JAS - Added record of who clicked on the object
			IsWrenchVisible = true;
		}
	} else {
		sound = VOC_BUTTON;
	}
	//Changed for multiplayer ST - 3/13/2019 5:31PM
	if (Is_Owned_By_Player()) {
	//if (IsOwnedByPlayer) {
		Sound_Effect(CAC_Stock_Sound(this, sound), Coord);
	}
}


/***********************************************************************************************
 * BuildingClass::Sell_Back -- Controls the sell back (demolish) operation.                    *
 *                                                                                             *
 *    This routine will initiate or stop the sell back process for a building. It is called    *
 *    when the player clicks on a building when the sell mode is active.                       *
 *                                                                                             *
 * INPUT:   control  -- The action to perform. 0 = turn deconstruction off, 1 = deconstruct,   *
 *                      -1 = toggle deconstruction state.                                      *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void BuildingClass::Sell_Back(int control)
{
	Validate();

	/*
	**	The computer never sells a headquarters.  Not its own, and not a captured one.
	**	
	**	The Eagle Command Base and the Alien Mothership are where a side's whole tree
	**	hangs from -- CAC_Prereq_Met opens everything off BA or BK -- so a computer
	**	that sells one has ended its own game to raise the price of a Power Pump.
	**	Captured ones are worth more again: taking one is how this game is won, and
	**	the computer was throwing away the prize it had just paid a Saboteur for.
	**	
	**	Asked here rather than at the callers because there are four of them and
	**	they arrive by different routes -- the damaged-and-broke path in
	**	BuildingClass::AI, AI_Raise_Power, AI_Lower_Power and the repair bay.
	**	
	**	A human may still sell whatever they like; this is about a decision nobody
	**	made on purpose.
	*/
	if (House != NULL && !House->IsHuman && Class != NULL) {
		int code = CAC_Code_Of(RTTI_BUILDINGTYPE, (int)Class->Type) & ~CAC_CODE_EVIL;
		if (code == CAC_CODE_BA || code == CAC_CODE_BK) {
			CAC_Log_Volatile("sell", "%s refused to sell its %s",
							 House->Class->IniName, Class->IniName);
			return;
		}
	}
	if (Class->Get_Buildup_Data()) {
		bool decon = false;
		switch (control) {
			case -1:
				decon = (Mission != MISSION_DECONSTRUCTION);
				break;

			case 1:
				if (Mission == MISSION_DECONSTRUCTION) return;
				decon = true;
				break;

			case 0:
				if (Mission != MISSION_DECONSTRUCTION) return;
				decon = false;
				break;
		}

		/*
		**	At this point, we know that the repair state has changed. Perform
		**	appropriate action.
		*/
		if (decon) {
//			Transmit_Message(RADIO_RUN_AWAY);
//			Transmit_Message(RADIO_OVER_OUT);
			Assign_Mission(MISSION_DECONSTRUCTION);
			//Changed for multiplayer ST - 3/13/2019 5:31PM
			if (Is_Owned_By_Player()) {
			//if (IsOwnedByPlayer) {
				Clicked_As_Target(PlayerPtr->Class->House); // 2019/09/20 JAS - Added record of who clicked on the object
			}
		}
		//Changed for multiplayer ST - 3/13/2019 5:31PM
		if (Is_Owned_By_Player()) {
		//if (IsOwnedByPlayer) {
			Sound_Effect(VOC_BUTTON);
		}
	}
}


/***********************************************************************************************
 * BuildingClass::What_Action -- Determines action to perform if click on specified object.    *
 *                                                                                             *
 *    This routine will determine what action to perform if the mouse was clicked on the       *
 *    object specified. This determination is used to control the mouse imagery and the        *
 *    function process when the mouse button is pressed.                                       *
 *                                                                                             *
 * INPUT:   object   -- Pointer to the object that, if clicked on, will control what action    *
 *                      is to be performed.                                                    *
 *                                                                                             *
 * OUTPUT:  Returns with the ActionType that will occur if the mouse is clicked over the       *
 *          object specified while the building is currently selected.                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ActionType BuildingClass::What_Action(ObjectClass * object) const
{
	Validate();
	ActionType action = TechnoClass::What_Action(object);

	if (action == ACTION_SELF) {
		if (Class->IsFactory && PlayerPtr == House) {
			switch (Class->ToBuild) {
				case RTTI_AIRCRAFTTYPE:
					if (House->AircraftFactories < 2) {
						action = ACTION_NONE;
					}
					else {
						action = ACTION_TOGGLE_PRIMARY;
					}
					break;

				case RTTI_INFANTRYTYPE:
					if (House->InfantryFactories < 2) {
						action = ACTION_NONE;
					}
					else {
						action = ACTION_TOGGLE_PRIMARY;
					}
					break;

				case RTTI_UNITTYPE:
					if (House->UnitFactories < 2) {
						action = ACTION_NONE;
					}
					else {
						action = ACTION_TOGGLE_PRIMARY;
					}
					break;

				default:
					action = ACTION_NONE;
					break;
			}

		} else {
			action = ACTION_NONE;
		}
	}

	/*
	**	Don't allow targeting of SAM sites, even if the CTRL key
	**	is held down.
	*/
	if (action == ACTION_ATTACK && *this == STRUCT_SAM) {
		action = ACTION_NONE;
	}

	if (action == ACTION_MOVE) {
		action = ACTION_NONE;
	}

	return(action);
}


/***********************************************************************************************
 * BuildingClass::What_Action -- Determines what action will occur.                            *
 *                                                                                             *
 *    This routine examines the cell specified and returns with the action that will be        *
 *    performed if that cell were clicked upon while the building is selected.                 *
 *                                                                                             *
 * INPUT:   cell  -- The cell to examine.                                                      *
 *                                                                                             *
 * OUTPUT:  Returns the ActionType that indicates what should occur if the mouse is clicked    *
 *          on this cell.                                                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ActionType BuildingClass::What_Action(CELL cell) const
{
	Validate();
	ActionType action = TechnoClass::What_Action(cell);

	/*
	**	A structure that folds back into a vehicle accepts a move order; that
	**	is what starts the deconstruction.  See CAC_Undeploys_Into.
	*/
	if (action == ACTION_MOVE && (CAC_Undeploys_Into(Class->Type) == UNIT_NONE || !Special.IsMCVDeploy)) {
		action = ACTION_NONE;
	}

	/*
	**	Don't allow targeting of SAM sites, even if the CTRL key
	**	is held down.
	*/
	if (action == ACTION_ATTACK && *this == STRUCT_SAM) {
		action = ACTION_NONE;
	}

	return(action);
}


/***********************************************************************************************
 * BuildingClass::Begin_Mode -- Begins an animation mode for the building.                     *
 *                                                                                             *
 *    This routine will start the building animating. This animation will loop indefinately    *
 *    until explicitly stopped.                                                                *
 *                                                                                             *
 * INPUT:   bstate   -- The animation state to initiate.                                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The buliding graphic state will reflect the first stage of this animation the   *
 *             very next time it is rendered.                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1995 JLB : Created.                                                                 *
 *   07/02/1995 JLB : Uses normalize animation rate where applicable.                          *
 *=============================================================================================*/
void BuildingClass::Begin_Mode(BStateType bstate)
{
	Validate();
	QueueBState = bstate;
	if (BState == BSTATE_NONE || bstate == BSTATE_CONSTRUCTION || ScenarioInit) {
		BState = bstate;
		QueueBState = BSTATE_NONE;
		BuildingTypeClass::AnimControlType const * ctrl = Fetch_Anim_Control();

		int rate = ctrl->Rate;
		if (Class->IsRegulated && bstate != BSTATE_CONSTRUCTION) {
			rate = Options.Normalize_Delay(rate);
		}
		Set_Rate(rate);
		Set_Stage(ctrl->Start);
	}
}


/***********************************************************************************************
 * BuildingClass::Read_INI -- Reads buildings from INI file.                                   *
 *                                                                                             *
 *    This is the basic scenario initialization of building function. It                       *
 *    is called when reading the scenario startup INI file and it handles                      *
 *    creation of all specified buildings.                                                     *
 *                                                                                             *
 *    INI entry format:                                                                        *
 *      Housename, Typename, Strength, Cell, Facing, Triggername                               *
 *                                                                                             *
 * INPUT:   buffer   -- Pointer to the loaded INI file data.                                   *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void BuildingClass::Read_INI(char *buffer)
{
	BuildingClass			*b;			// Working unit pointer.
	char						*tbuffer;	// Accumulation buffer of unit IDs.
	HousesType				bhouse;		// Building house.
	StructType				classid;		// Building type.
	int						len;			// Size of data in buffer.
	CELL						cell;			// Cell of building.
	char						buf[128];
	char						*trigname;	// building's trigger's name

	len = strlen(buffer) + 2;
	tbuffer = buffer + len;

	/*
	**	Read the entire building INI section into HIDBUF
	*/
	WWGetPrivateProfileString(INI_Name(), NULL, NULL, tbuffer, ShapeBufferSize-len, buffer);

	while (*tbuffer != '\0') {

		/*
		**	Get a building entry.
		*/
		WWGetPrivateProfileString(INI_Name(), tbuffer, NULL, buf, sizeof(buf)-1, buffer);

		/*
		**	1st token: house name.
		*/
		bhouse = HouseTypeClass::From_Name(strtok(buf, ","));

		/*
		**	2nd token: building name.
		*/
		classid = BuildingTypeClass::From_Name(strtok(NULL, ","));

		if (bhouse != HOUSE_NONE && classid != STRUCT_NONE) {
			int	strength;
			DirType facing;

			/*
			**	3rd token: strength.
			*/
			strength = atoi(strtok(NULL, ","));

			/*
			**	4th token: cell #.
			*/
			cell = atoi(strtok(NULL, ","));

			/*
			**	5th token: facing.
			*/
			facing = (DirType)atoi(strtok(NULL, ","));

			/*
			**	6th token: triggername (can be NULL).
			*/
			trigname = strtok(NULL,",");

			/*
			**	The supplemental build mixes the rosters, leaving alone a
			**	building with a trigger and one the computer's [Base] plan will
			**	rebuild by type.  See CAC_Mix_Type.
			*/
			{
				bool locked = (trigname != NULL && TriggerClass::As_Pointer(trigname) != NULL);
#ifdef CAC_SUPPLEMENTAL
				if (!locked) locked = CAC_INI_Base_Has_Cell(buffer, cell);
#endif
				classid = (StructType)CAC_Mix_Type(RTTI_BUILDINGTYPE, classid, bhouse, locked);
			}

			if (HouseClass::As_Pointer(bhouse) != NULL) {
				b = new BuildingClass(classid, bhouse);
				if (b) {
					if (b->Unlimbo(Cell_Coord(cell), facing)) {
						strength = MIN(strength, 0x100);
						strength = Fixed_To_Cardinal(b->Class->MaxStrength, strength);
						b->Strength = strength;
						b->IsALemon = false;
						b->Trigger = TriggerClass::As_Pointer(trigname);
						if (b->Trigger) {
							b->Trigger->AttachCount++;
						}
					} else {

						/*
						**	If the building could not be unlimboed on the map, then this indicates
						**	a serious error. Delete the building.
						*/
						delete b;
					}
				}
			}
		}
		tbuffer += strlen(tbuffer)+1;
	}
}


/***********************************************************************************************
 * BuildingClass::Write_INI -- Writes all building data to an INI file.                        *
 *                                                                                             *
 *    This routine is used to write the buildings into an INI file. It is necessary for the    *
 *    scenario editor save game option.                                                        *
 *                                                                                             *
 *    INI entry format:                                                                        *
 *      Housename, Typename, Strength, Cell, Facing, Triggername                               *
 *                                                                                             *
 * INPUT:   buffer   -- The buffer that holds the INI data.                                    *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/28/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void BuildingClass::Write_INI(char *buffer)
{
	int			index;
	char			uname[10];
	char			buf[127];
	char			*tbuffer;		// Accumulation buffer of unit IDs.

	/*
	**	First, clear out all existing building data from the ini file.
	*/
	tbuffer = buffer + strlen(buffer) + 2;
	WWGetPrivateProfileString(INI_Name(), NULL, NULL, tbuffer, ShapeBufferSize-strlen(buffer), buffer);
	while (*tbuffer != '\0') {
		WWWritePrivateProfileString(INI_Name(), tbuffer, NULL, buffer);
		tbuffer += strlen(tbuffer)+1;
	}

	/*
	**	Write the data out.
	*/
	for (index = 0; index < Buildings.Count(); index++) {
		BuildingClass * building;

		building = Buildings.Ptr(index);
		if (!building->IsInLimbo) {

			sprintf(uname, "%03d", index);
			sprintf(buf, "%s,%s,%d,%u,%d,%s",
				building->House->Class->IniName,
				building->Class->IniName,
				building->Health_Ratio(),
				Coord_Cell(building->Coord),
				building->PrimaryFacing.Current(),
				building->Trigger ? building->Trigger->Get_Name() : "None"
				);
			WWWritePrivateProfileString(INI_Name(), uname, buf, buffer);
		}
	}
}


/***********************************************************************************************
 * BuildingClass::As_Target -- Convert the building into a target value.                       *
 *                                                                                             *
 *    Use this routine to take the building and convert it into a target number.               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the target number for this building.                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
TARGET BuildingClass::As_Target(void) const
{
	Validate();
	return(Build_Target(KIND_BUILDING, Buildings.ID(this)));
}


/***********************************************************************************************
 * BuildingClass::Center_Coord -- Fetches the center coordinate for the building.              *
 *                                                                                             *
 *    This routine is used to set the center coordinate for this building.                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the coordinate for the center location for the building.              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
COORDINATE BuildingClass::Center_Coord(void) const
{
	Validate();
	return(Coord_Add(Coord, CenterOffset[Class->Size]));
}

static bool Occupy_List_Contains(const short * list, short cell)
{
	while (*list != REFRESH_EOL) {
		if (*list == cell) {
			return true;
		}
		list++;
	}
	return false;
}

COORDINATE BuildingClass::Target_Coord(void) const
{
	static constexpr int _num_facings = 3;
	static const FacingType _facings[_num_facings] = { FACING_S, FACING_E, FACING_SE };
	static const COORDINATE _offsets[_num_facings] = { 0x00800000, 0x00000080, 0x00800080 };

	Validate();
	COORDINATE offset = CenterOffset[Class->Size];

	const short * list = Occupy_List();
	CELL cell = Coord_Cell(offset);
	if (!Occupy_List_Contains(list, cell)) {
		for (int i = 0; i < _num_facings; ++i) {
			CELL adjcell = Adjacent_Cell(cell, _facings[i]);
			if (Occupy_List_Contains(list, adjcell)) {
				offset = Coord_Add(offset, _offsets[i]) & 0xFF80FF80;
				break;
			}
		}
	}
	return(Coord_Add(Coord, offset));
}


COORDINATE BuildingClass::Docking_Coord(void) const
{
	Validate();
	if (*this == STRUCT_HELIPAD) {
		return(Coord_Add(Coord, XYP_COORD(24, 18)));
	}
	if (*this == STRUCT_AIRSTRIP) {
		return(Coord_Add(Coord, XYP_COORD(18, 30)));
	}

	/*
	**	A CrystAlien miner unloads at the headquarters, which is not a refinery
	**	and so has no docking point of its own.  The default is the building's
	**	centre -- a cell the building itself occupies, which the miner can never
	**	reach, so it nudged the side and eventually gave up and drove back to the
	**	field.
	**
	**	BuildingStats works the dock out as
	**
	**	    dockX = (topLeftX + (width2 - 1)) * tileSize
	**	    dockY = (topLeftY + (height - 1)) * tileSize
	**
	**	so the dock is the structure's own near row -- the row its baseplate is
	**	drawn on, which is where the miner is meant to finish up standing.  It
	**	can get there even though the building occupies the cell: a unit may
	**	enter the cells of whatever it is in radio contact with once tethered.
	*/
	if (CAC_Is_Refinery(Class->Type) && !CAC_Is_Stock_Structure(Class->Type)) {
		return(Coord_Add(Coord, XYP_COORD(Class->Width() * 12,
		                                  MAX(12, Class->Height() * 24 - 12))));
	}
	return(TechnoClass::Docking_Coord());
}


/***********************************************************************************************
 * BuildingClass::Can_Fire -- Determines if this building can fire.                            *
 *                                                                                             *
 *    Use this routine to see if the building can fire its weapon.                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:   target   -- The target that firing upon is desired.                                *
 *                                                                                             *
 *          which    -- Which weapon to use when firing. 0=primary, 1=secondary.               *
 *                                                                                             *
 * OUTPUT:  Returns with the fire possibility code. If firing is allowed, then FIRE_OK is      *
 *          returned. Other cases will result in appropriate fire code value that indicates    *
 *          why firing is not allowed.                                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/03/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
FireErrorType BuildingClass::Can_Fire(TARGET target, int which) const
{
	Validate();

	/*
	**	A building with no power does not shoot.
	**
	**	Level.engageEnemy opens with it, and it is about buildings alone:
	**
	**	    if (this.isBuilding) {
	**	        if (this.friend && this.parent.powerOff)     return undefined;
	**	        if (!this.friend && this.parent.powerOffOppo) return undefined;
	**	    }
	**
	**	so a brownout silences every Defence Station and Assault Turret on the map,
	**	both sides, and leaves the units alone.  The conversion had the test in one
	**	place only -- the Obelisk's charging logic in BuildingClass::AI, which is
	**	guarded by `*this == STRUCT_OBELISK` and so never reached by anything of
	**	CrystAlien's.  Turrets went on firing through a blackout.
	*/
	if (CAC_Is_CrystAlien_Type(Class->IniName) && CAC_Power_Off(House)) {
		return(FIRE_BUSY);
	}

	FireErrorType canfire = TechnoClass::Can_Fire(target, which);

	if (canfire == FIRE_OK) {

		/*
		**	Double check to make sure that the facing is roughly toward
		**	the target. If the difference is too great, then firing is
		**	temporarily postponed.
		*/
		if (Class->IsTurretEquipped) {
			/*
			**	If the turret is rotating then firing must be delayed.
			*/
			if (PrimaryFacing.Is_Rotating()) {
				return(FIRE_ROTATING);
			}

			int diff = PrimaryFacing.Difference(Direction(TarCom));
			if (ABS(diff) > 8) {
				return(FIRE_FACING);
			}
		}

		/*
		**	Advanced guard towers need power to fire.
		*/
		if (*this == STRUCT_ATOWER && House->Power_Fraction() < 0x0100) {
			return(FIRE_BUSY);
		}

		/*
		** If an obelisk can fire, check the state of charge.  If it isn't charging
		** up, start it charging up and return FIRE_BUSY.  If it is charging but
		** isn't done yet, return FIRE_BUSY.  If it's done charging, stop the
		** charging process, clear the stage timer, and return FIRE_OK.
		*/
		if (Class->Primary == WEAPON_OBELISK_LASER && !IsCharged) {
			return(FIRE_BUSY);
		}
	}
	return(canfire);
}


/***********************************************************************************************
 * BuildingClass::Toggle_Primary -- Toggles the primary factory state.                         *
 *                                                                                             *
 *    This routine will change the primary factory state of this building. The primary         *
 *    factory is the one that units will be produced from (by default).                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Is this building NOW the primary factory?                                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/03/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool BuildingClass::Toggle_Primary(void)
{
	Validate();
	if (IsLeader) {
		IsLeader = false;
	} else {
		for (int index = 0; index < Buildings.Count(); index++) {
			BuildingClass * building = Buildings.Ptr(index);

			if (!building->IsInLimbo && building->Owner() == Owner() && building->Class->ToBuild == Class->ToBuild) {
				building->IsLeader = false;
			}
		}
		IsLeader = true;
		//
		// MBL 07.22.2020 - Update so that each player in multiplayer will properly hear this when it applies to them
		//
		// if (House == PlayerPtr) {
		// 	Speak(VOX_PRIMARY_SELECTED);
		// }
		if ((HouseClass *)House->IsHuman) {
			Speak(VOX_PRIMARY_SELECTED, House);
		}
	}
	Mark(MARK_CHANGE);
	return(IsLeader);
}


/***********************************************************************************************
 * BuildingClass::Captured -- Captures the building.                                           *
 *                                                                                             *
 *    This routine will change the owner of the building. It handles updating any related      *
 *    game systems as a result. Factories are the most prone to have great game related        *
 *    consequences when captured. This could also affect the sidebar and building ownership.   *
 *                                                                                             *
 * INPUT:   newowner -- Pointer to the house that is now the new owner.                        *
 *                                                                                             *
 * OUTPUT:  Was the capture attempt successful?                                                *
 *                                                                                             *
 * WARNINGS:   Capturing could fail if the house is already owned by the one specified or      *
 *             the building isn't allowed to be captured.                                      *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/03/1995 JLB : Created.                                                                 *
 *   07/05/1995 JLB : Fixed production problem with capturing enemy buildings.                 *
 *=============================================================================================*/
bool BuildingClass::Captured(HouseClass * newowner)
{
	Validate();
	if (Can_Capture() && newowner != House) {
		switch (Owner()) {
			case HOUSE_GOOD:
				Speak(VOX_GDI_CAPTURED);
				break;

			case HOUSE_BAD:
				Speak(VOX_NOD_CAPTURED);
				break;
		}

		if (House == PlayerPtr) {
			Map.PowerClass::IsToRedraw = true;
			Map.Flag_To_Redraw(false);
		}

		/*
		** Maybe trigger an achivement. ST - 11/14/2019 1:53PM
		*/
		if (newowner->IsHuman) {
			TechnoTypeClass const *object_type = Techno_Type_Class();
			if (object_type) {
				if (newowner->ActLike != House->ActLike) {
					On_Achievement_Event(newowner, "OPPOSING_BUILDING_CAPTURED", object_type->IniName);
				} else {
					On_Achievement_Event(newowner, "BUILDING_CAPTURED", object_type->IniName);
				}	
			}
		}

		/*
		** Add this building to the list of buildings captured this game. For internet stats purposes
		*/
		if (GameToPlay == GAME_INTERNET){
			newowner->CapturedBuildings->Increment_Unit_Total (Class->Type);
		}

		House->Adjust_Power(-Power_Output());
		LastStrength = 0;
		House->Adjust_Drain(-Class->Drain);
		int booty = House->Adjust_Capacity(-(int)Class->Capacity, true);

		/*
		**	If there is something loaded, then it gets captured as well.
		*/
		TechnoClass * tech = Attached_Object();
		if (tech) tech->Captured(newowner);

		/*
		**	If something isn't technically attached, but is sitting on this
		**	building for another reason (e.g., helicopter on helipad), then it
		**	gets captured as well.
		*/
		tech = Contact_With_Whom();
		if (tech) {
			if (Transmit_Message(RADIO_NEED_TO_MOVE) == RADIO_ROGER && ::Distance(tech->Center_Coord(), Docking_Coord()) < 0x0040) {
				tech->Captured(newowner);
			} else {
				Transmit_Message(RADIO_RUN_AWAY);
				Transmit_Message(RADIO_OVER_OUT);
			}
		}

		/*
		**	Decrement the factory counter for the original owner.
		*/
		/*
		**	A building that hosts a type counts as a factory of that kind,
		**	whatever its own ToBuild says.  See CAC_Builds_Any.
		*/
		/*
		**	Whatever this building was making goes with it.  See Abandon_Host.
		*/
		if (House != NULL) House->Abandon_Host(Class->Type);

		if (CAC_Builds_Any(Class->Type, RTTI_UNITTYPE)) House->UnitFactories--;
		if (CAC_Builds_Any(Class->Type, RTTI_INFANTRYTYPE)) House->InfantryFactories--;
		if (CAC_Builds_Any(Class->Type, RTTI_AIRCRAFTTYPE)) House->AircraftFactories--;

		switch (Class->ToBuild) {
			case RTTI_UNITTYPE:
				House->UnitFactories--;
				break;

			case RTTI_INFANTRYTYPE:
				House->InfantryFactories--;
				break;

			case RTTI_BUILDINGTYPE:
				House->BuildingFactories--;
				break;

			case RTTI_AIRCRAFTTYPE:
				House->AircraftFactories--;
				break;

			default:
				break;
		}

#ifdef NEVER
		if (IsOwnedByPlayer && !ScenarioInit) {
			Map.Recalc();
		}
		if (!House->IsHuman) {
			Update_Specials();
		}
#endif

		/*
		**	Flag that both owners now need to update their buildable lists.
		*/
		House->IsRecalcNeeded = true;
		newowner->IsRecalcNeeded = true;

		HouseClass * oldowner = House;				// Added for RA AI in TD. ST - 7/26/2019 9:25AM

		IsCaptured = true;
		TechnoClass::Captured(newowner);

		/*
		**	And the new owner takes on what the old one just gave up.
		**
		**	The lines above subtract this building's output and draw from the house
		**	losing it, and nothing anywhere added them to the house gaining it -- so a
		**	captured Energy Generator produced nothing for its captor, for good, and a
		**	captured turret drew nothing either.  Stock Tiberian Dawn has the same
		**	hole; it shows less there because its power ratio is forgiving and few
		**	captured buildings are plants.  Here the turrets hang off the meter.
		**
		**	The CrystAlien sidebar reads CAC_Power_Sums, which counts the house's
		**	buildings afresh every time and never saw this -- but Power_Fraction and
		**	everything in the engine that asks the house directly did.
		*/
		House->Adjust_Power(Power_Output());
		House->Adjust_Drain(Class->Drain);


#ifdef USE_RA_AI
		//
		// Added for RA AI in TD. ST - 7/26/2019 9:25AM
		//
		oldowner->Recalc_Center();
		House->Recalc_Center();
#endif

		SmudgeType bib;
		CELL cell = Coord_Cell(Coord);
		if (Class->Bib_And_Offset(bib, cell, House->Class->House)) {
			SmudgeClass * smudge = new SmudgeClass(bib);
			if (smudge) {
				smudge->Disown(cell);
				delete smudge;
			}
			new SmudgeClass(bib, Cell_Coord(cell), House->Class->House);
		}

		/*
		**	Increment the factory count for the new owner.
		*/
		/*
		**	A building that hosts a type counts as a factory of that kind,
		**	whatever its own ToBuild says.  See CAC_Builds_Any.
		*/
		if (CAC_Builds_Any(Class->Type, RTTI_UNITTYPE)) House->UnitFactories++;
		if (CAC_Builds_Any(Class->Type, RTTI_INFANTRYTYPE)) House->InfantryFactories++;
		if (CAC_Builds_Any(Class->Type, RTTI_AIRCRAFTTYPE)) House->AircraftFactories++;

		switch (Class->ToBuild) {
			case RTTI_UNITTYPE:
				House->UnitFactories++;
				break;

			case RTTI_INFANTRYTYPE:
				House->InfantryFactories++;
				break;

			case RTTI_BUILDINGTYPE:
				House->BuildingFactories++;
				break;

			case RTTI_AIRCRAFTTYPE:
				House->AircraftFactories++;
				break;

			default:
				break;
		}

		IsRepairing = false;

		/*
		**	And it stops shooting at what its last owner was shooting at.
		**
		**	Neither engine clears this on its own.  Captured() moves the power, the
		**	drain, the capacity and the ownership and never touches TarCom, and
		**	CrystAlien is the same shape -- Level.engageEnemy opens with
		**
		**	    if(this.target) { target = this.target; }
		**
		**	and only looks for a new one when it has none, while Building.handle
		**	drops the old one solely when it dies.  So a turret taken mid-burst kept
		**	firing, and everything it had been aiming at was now on its own side.
		**
		**	Evaluate_Object already refuses an ally when a target is being *chosen*
		**	-- `if (House->Is_Ally(object)) return(false)` -- so clearing it here is
		**	enough: the next pass picks an enemy or nothing, by the rule that was
		**	already there.
		*/
		Assign_Target(TARGET_NONE);
		Assign_Mission(MISSION_GUARD);

		Grand_Opening(true);
		House->Harvested(booty);

		Mark(MARK_CHANGE);

		/*
		**	Perform a look operation when catpured if it was the player
		**	that performed the capture.
		*/
		if (GameToPlay == GAME_GLYPHX_MULTIPLAYER && House->IsHuman) {
			Look(false);
		} else {
			if (House == PlayerPtr) {
				Look(false);
			}
		}

		if (oldowner) {
			oldowner->Check_Pertinent_Structures();
		}

		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * BuildingClass::Sort_Y -- Returns the building coordinate used for sorting.                  *
 *                                                                                             *
 *    The coordinate value returned from this function should be used for sorting purposes.    *
 *    It has special offset adjustment applied so that vehicles don't overlap (as much).       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a coordinate value suitable to be used for sorting.                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/23/1995 JLB : Created.                                                                 *
 *   06/19/1995 JLB : Handles buildings that come with bibs built-in.                          *
 *=============================================================================================*/
COORDINATE BuildingClass::Sort_Y(void) const
{
	Validate();
	if (*this == STRUCT_REPAIR) {
		return(Coord);
	}
	if (*this == STRUCT_HELIPAD) {
		return(Center_Coord());
	}
	if (*this == STRUCT_BARRACKS /*|| *this == STRUCT_POWER*/) {
		return(Center_Coord());
	}
	if (CAC_Is_Refinery(Class->Type)) {
		return(Center_Coord());
	}
	return(Coord_Add(Center_Coord(), XY_Coord(0, (Class->Height()*256)/3)));
}


/***********************************************************************************************
 * BuildingClass::Can_Enter_Cell -- Determines if building can be placed down.                 *
 *                                                                                             *
 *    This routine will determine if the building can be placed down at the location           *
 *    specified.                                                                               *
 *                                                                                             *
 * INPUT:   cell  -- The cell to examine. This is usually the cell of the upper left corner    *
 *                   of the building if it were to be placed down.                             *
 *                                                                                             *
 * OUTPUT:  Returns with the move legality value for placement at the location specified. This *
 *          will either be MOVE_OK or MOVE_NO.                                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
MoveType BuildingClass::Can_Enter_Cell(CELL cell, FacingType) const
{
	Validate();

	if (*this == STRUCT_CONST && IsDown) {
		return(Map[cell].Is_Generally_Clear() ? MOVE_OK : MOVE_NO);
	}

	return(Class->Legal_Placement(cell) ? MOVE_OK : MOVE_NO);
}


/***********************************************************************************************
 * BuildingClass::Can_Demolish -- Can the player demolish (sell back) the building?            *
 *                                                                                             *
 *    Determines if the player can sell this building. Selling is possible if the building     *
 *    is not currently in construction or deconstruction animation.                            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Can the building be demolished at this time?                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1995 JLB : Created.                                                                 *
 *   07/01/1995 JLB : If there is no buildup data, then the building can't be sold.            *
 *   07/17/1995 JLB : Cannot sell a refinery that has a harvester attached.                    *
 *=============================================================================================*/
bool BuildingClass::Can_Demolish(void) const
{
	Validate();
	// !Mission !=  sounds a bit fishy. ST - 2018
	//if (Class->Get_Buildup_Data() && BState != BSTATE_CONSTRUCTION && !Mission != MISSION_DECONSTRUCTION && Mission != MISSION_CONSTRUCTION) {
	if (Class->IsUnsellable) return(false);

	/*
	**	Nor a headquarters, nor the last of them.  See CAC_HQ_Unsellable.
	*/
	if (CAC_HQ_Unsellable(this)) return(false);

	if (Class->Get_Buildup_Data() && BState != BSTATE_CONSTRUCTION && Mission != MISSION_DECONSTRUCTION && Mission != MISSION_CONSTRUCTION) {
		if (CAC_Is_Refinery(Class->Type) && Is_Something_Attached()) return(false);
		return(true);
	}
	return(false);
}


bool BuildingClass::Can_Demolish_Unit(void) const
{
	return(*this == STRUCT_REPAIR && In_Radio_Contact() && Distance(Contact_With_Whom()) < 0x0080);
}


bool BuildingClass::Can_Capture(void) const
{
	/*
	**	Nor taken by an engineer -- see CAC_Pizza_Protected.  Invincibility that
	**	an Engineer can walk through is not invincibility, and a captured
	**	headquarters would hand over the mode's one permanent asset.
	*/
	if (CAC_Pizza_Protected(this)) return(false);

	bool can_capture = Class->IsCaptureable && Mission != MISSION_DECONSTRUCTION;

	// Override capturable state if this building has a capture win trigger
	if (GameToPlay == GAME_NORMAL) {
		if (!House->IsHuman && Trigger != NULL && Trigger->Action == TriggerClass::ACTION_WINLOSE) {
			can_capture = true;
		}
	}

	return(can_capture);
}


/***********************************************************************************************
 * BuildingClass::Mission_Guard -- Handles guard mission for combat buildings.                 *
 *                                                                                             *
 *    Buildings that can attack are given this mission. They will wait until a suitable target *
 *    comes within range and then launch into the attack mission. Buildings that have no       *
 *    weaponry will just sit in this routine forever.                                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of game frames to delay before this routine will be called *
 *          again.                                                                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int BuildingClass::Mission_Guard(void)
{
	Validate();
	/*
	**	If this building has a weapon, then search for a target to attack. When
	**	a target is found, switch into attack mode to deal with the threat.
	*/
	if (Class->Primary != WEAPON_NONE) {

		/*
		**	Weapon equipped buildings are ALWAYS ready to launch into another mission if
		**	they are sitting around in guard mode.
		*/
		IsReadyToCommence = true;

		/*
		**	If there is no target available, then search for one.
		*/
		if (!Target_Legal(TarCom)) {
			ThreatType threat = THREAT_NORMAL;
			Assign_Target(Greatest_Threat(threat));
		}

		/*
		**	There is a valid target. Switch into attack mode right away.
		*/
		if (Target_Legal(TarCom)) {
			Assign_Mission(MISSION_ATTACK);
			return(1);
		}
	} else {

		/*
		**	This is the very simple state machine that basically does
		**	nothing. This is the mode that non weapon equipped buildings
		**	are normally in.
		*/
		enum {
			INITIAL_ENTRY,
			IDLE
		};
		switch (Status) {
			case INITIAL_ENTRY:
				Begin_Mode(BSTATE_IDLE);
				Status = IDLE;
				break;

			case IDLE:
				/*
				**	Special case to break out of guard mode if this is a repair
				**	facility and there is a customer waiting at the grease pit.
				*/
				if (*this == STRUCT_REPAIR &&
					In_Radio_Contact() &&
					Contact_With_Whom()->Is_Techno() &&
					((TechnoClass *)Contact_With_Whom())->Mission == MISSION_ENTER &&
					Distance(Contact_With_Whom()) < 0x0040 &&
					Transmit_Message(RADIO_NEED_TO_MOVE) == RADIO_ROGER) {

					Assign_Mission(MISSION_REPAIR);
					return(1);
				}
				break;
		}
		return(TICKS_PER_SECOND*5);
	}
	return(TICKS_PER_SECOND/2);
}


/***********************************************************************************************
 * BuildingClass::Mission_Construction -- Handles mission construction.                        *
 *                                                                                             *
 *    This routine will handle mission construction. When this mission is complete, the        *
 *    building will begin normal operation.                                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of game frames to delay before calling this routine        *
 *          again.                                                                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int BuildingClass::Mission_Construction(void)
{
	Validate();
	enum {
		INITIAL,
		DURING
	};
	switch (Status) {
		case INITIAL:
			Begin_Mode(BSTATE_CONSTRUCTION);
			Transmit_Message(RADIO_BUILDING);
			//Changed for multiplayer ST - 3/13/2019 5:31PM
			if (Is_Owned_By_Player()) {
			//if (IsOwnedByPlayer) {
				Sound_Effect(CAC_Stock_Sound(this, VOC_CONSTRUCTION), Coord);
			}
			Status = DURING;
			break;

		case DURING:
			if (IsReadyToCommence) {

				/*
				**	When construction is complete, then transmit this
				**	to the construction yard so that it can stop its
				**	construction animation.
				*/
				Transmit_Message(RADIO_COMPLETE);		// "I'm finished."
				Transmit_Message(RADIO_OVER_OUT);		// "You're free."
				Begin_Mode(BSTATE_IDLE);
				// Construction yard already called this on reveal in normal game mode, so don't do twice
				if (*this != STRUCT_CONST || GameToPlay != GAME_NORMAL) {
					Grand_Opening();
				}
				Assign_Mission(MISSION_GUARD);
				PrimaryFacing = Class->StartFace;

				/*
				**	And work out what can be built now that this one is standing.
				**
				**	Unlimbo raises IsRecalcNeeded when a building is *placed*, which is
				**	one build-up too early: the structure is still BSTATE_CONSTRUCTION
				**	at that point, so it does not count towards anything -- findBuilding
				**	only answers for an active building, and CAC_Owns follows it -- and
				**	the flag is spent by the time it does.  Nothing raised it again, so
				**	the tree ran one placement behind: a Training Camp unlocked its
				**	infantry only when the *next* structure went down, and a captured
				**	building opened its faction's half the same way.
				*/
				if (House != NULL) House->IsRecalcNeeded = true;
			}
			break;
	}
	return(1);
}


/***********************************************************************************************
 * BuildingClass::Mission_Deconstruction -- Handles building deconstruction.                   *
 *                                                                                             *
 *    This state machine is only used when the building is deconstructing as a result of       *
 *    selling.  When this mission is finished, the building will no longer exist.              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of game frames to delay before calling this routine again. *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1995 JLB : Created.                                                                 *
 *   08/13/1995 JLB : Enable selling of units on a repair bay.                                 *
 *   08/20/1995 JLB : Scatters infantry from scattered starting points.                        *
 *=============================================================================================*/
int BuildingClass::Mission_Deconstruction(void)
{
	Validate();
	/*
	**	Always force repair off.
	*/
	Repair(0);

	enum {
		INITIAL,
		HOLDING,
		DURING
	};
	switch (Status) {
		case INITIAL:

			/*
			**	Special check for the repair bay which has the ability to sell
			**	whatever is on it. If there is something on the repair bay, then
			**	it will be sold. If there is nothing on the repair bay, then
			**	the repair bay itself will be sold.
			*/
			if (Can_Demolish_Unit() && Transmit_Message(RADIO_NEED_TO_MOVE) == RADIO_ROGER) {
				TechnoClass * tech = Contact_With_Whom();
				Transmit_Message(RADIO_OVER_OUT);
				tech->Sell_Back(1);
//				House->Refund_Money(tech->Refund_Amount());
//				tech->Limbo();
				IsReadyToCommence = true;
				Assign_Mission(MISSION_GUARD);
				return(1);
			}

			IsReadyToCommence = false;
			Transmit_Message(RADIO_RUN_AWAY);
			Status = HOLDING;
			break;

		case HOLDING:
			if (!IsTethered) {

				/*
				**	The crew will evacuate from the building. The number of crew
				**	members leaving is equal to the unrecovered cost of the building
				**	divided by 100 (the typical cost of a minigunner infantryman).
				*/
				if ((!Target_Legal(ArchiveTarget) && CAC_Is_Stock_Structure(Class->Type)) || !Special.IsMCVDeploy || CAC_Undeploys_Into(Class->Type) == UNIT_NONE) {
					int divisor = 200;
					if (IsCaptured) divisor *= 2;
					int count = (Class->Raw_Cost()+(divisor-1)) / divisor;
					bool engine = false;
					count = Bound(count, 1, 5);

					/*
					**	A CrystAlien structure gives up the same crew whether it was sold or
					**	shot out from under its owner -- one routine, one list.  See
					**	CAC_Spawn_Survivors.
					*/
					if (CAC_Spawn_Survivors(this) >= 0) count = 0;

					while (count) {

						InfantryType typ = Crew_Type();
						while (typ == INFANTRY_E7 && engine) {
							typ = Crew_Type();
						}
						if (typ == INFANTRY_E7) engine = true;

						InfantryClass * infantry = new InfantryClass(typ, House->Class->House);
						if (infantry) {
							ScenarioInit++;
							COORDINATE coord = Coord_Add(Center_Coord(), XYP_COORD(0, -12));
							coord = Map[Coord_Cell(coord)].Closest_Free_Spot(coord, false);

							if (infantry->Unlimbo(coord, DIR_N)) {
								if (infantry->Class->IsNominal) infantry->IsTechnician = true;
								ScenarioInit--;
								infantry->Scatter(0, true);
								ScenarioInit++;
								infantry->Assign_Mission(MISSION_GUARD_AREA);
							} else {
								delete infantry;
							}
							ScenarioInit--;
						}
						count--;
					}
				}

				// MBL 07.10.2020 - In 1v1, sometimes both players will hear this SFX, or neither player will hear it
				// Making it so all players hear it positionally in the map; Per thread discussion in https://jaas.ea.com/browse/TDRA-7245
				//
				#if 0
					//Changed for multiplayer ST - 3/13/2019 5:31PM
					if (Is_Owned_By_Player()) {
					//if (IsOwnedByPlayer) {
						Sound_Effect(CAC_Stock_Sound(this, VOC_CASHTURN), Coord);
					}
				#else
					Sound_Effect(CAC_Stock_Sound(this, VOC_CASHTURN), Coord);
				#endif
				
				/*
				**	Destroy all attached objects. ST - 4/24/2020 9:38PM
				*/
				while (Attached_Object()) {
					FootClass * obj = Detach_Object();

					Detach_All(true);
					delete obj;
				}
				
				Transmit_Message(RADIO_OVER_OUT);
				Status = DURING;
				Begin_Mode(BSTATE_CONSTRUCTION);
				IsReadyToCommence = false;
				IsSurvivorless = true;
				break;
			}
			Transmit_Message(RADIO_RUN_AWAY);
			break;

		case DURING:
			if (IsReadyToCommence) {

				/*
				**	Construction yards that deconstruct, really just revert back
				**	to an MCV.
				*/
				/*
				**	An Ops Ship built from the sidebar has no ArchiveTarget to remember
				**	arriving as a vehicle, so that history is only required of the
				**	stock Construction Yard.
				*/
				if ((Target_Legal(ArchiveTarget) || !CAC_Is_Stock_Structure(Class->Type)) &&
				    Special.IsMCVDeploy && CAC_Undeploys_Into(Class->Type) != UNIT_NONE && House->IsHuman) {
					ScenarioInit++;
					UnitClass * unit = new UnitClass(CAC_Undeploys_Into(Class->Type), House->Class->House);
					ScenarioInit--;
					if (unit) {

						/*
						**	Unlimbo the MCV onto the map. The MCV should start in the same
						**	health condition that the construction yard was in.
						*/
						int ratio = Health_Ratio();
						int money = Refund_Amount();
						TARGET arch = ArchiveTarget;
						COORDINATE place = Coord_Snap(Adjacent_Cell(Coord, DIR_SE));

						Delete_This();

						if (unit->Unlimbo(place, DIR_SW)) {
							unit->Strength = Fixed_To_Cardinal(unit->Class_Of().MaxStrength, ratio);

							/*
							**	Lift the move destination from the building and assign
							**	it to the unit.
							*/
							if (Target_Legal(arch)) {
								unit->Assign_Destination(arch);
								unit->Assign_Mission(MISSION_MOVE);
							}
						} else {

							/*
							**	If, for some strange reason, the MCV could not be placed on the
							**	map, then give the player some money to compensate.
							*/
							House->Refund_Money(money);
						}
					} else {
						House->Refund_Money(Refund_Amount());
						Delete_This();
					}

				} else {

					/*
					**	A sold building still counts as a kill, but it just isn't directly
					**	attributed to the enemy.
					*/
					WhoLastHurtMe = HOUSE_NONE;
					Record_The_Kill(NULL);

					/*
					**	The player gets part of the money back for the sell.
					*/
					House->Refund_Money(Refund_Amount());
					Limbo();

					if (House) {
						House->Check_Pertinent_Structures();
					}

					/*
					**	Finally, delete the building from the game.
					*/
					Delete_This();
				}
				House->IsRecalcNeeded = true;
			}
			break;
	}
	return(1);
}


/***********************************************************************************************
 * BuildingClass::Mission_Attack -- Handles attack mission for building.                       *
 *                                                                                             *
 *    Buildings that can attack are processed by this attack mission state machine.            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of game frames to delay before calling this routine        *
 *          again.                                                                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int BuildingClass::Mission_Attack(void)
{
	Validate();
	if (*this == STRUCT_SAM) {
		switch (Status) {

			/*
			**	The launcher is underground and awaiting the acquisition of
			**	a target.
			*/
			case SAM_UNDERGROUND:
				IsReadyToCommence = true;
				if (Target_Legal(TarCom)) {
					Set_Rate(2);
					Set_Stage(0);
					Status = SAM_RISING;
					return(1);
				} else {
					Assign_Mission(MISSION_GUARD);
				}
				break;

			/*
			**	The launcher is rising into the ready position so that it
			**	may rotate to face the target.
			*/
			case SAM_RISING:
				if (Fetch_Stage() == 15) {
					Set_Rate(0);
					PrimaryFacing = DIR_N;
					if (!Target_Legal(TarCom)) {
						Status = SAM_LOWERING;
					} else {
						Status = SAM_READY;
					}
				}
				return(1);

			/*
			**	This is the target tracking state of the launcher. It will rotate
			**	to face the current TarCom of the launcher.
			*/
			case SAM_READY:
				if (!Target_Legal(TarCom) || !Is_Target_Aircraft(TarCom) || As_Aircraft(TarCom)->Altitude == 0) {
					Assign_Target(TARGET_NONE);
					Status = SAM_LOCKING;
					return(TICKS_PER_SECOND);
				} else {
					if (!PrimaryFacing.Is_Rotating()) {
						DirType facing = Direction(TarCom);
						if (PrimaryFacing.Difference(facing)) {
							PrimaryFacing.Set_Desired(facing);
						} else {
							Status = SAM_FIRING;
						}
					}
				}
				return(1);

			/*
			**	The launcher is in the process of firing.
			*/
			case SAM_FIRING:
				if (!Target_Legal(TarCom) || !Is_Target_Aircraft(TarCom) || As_Aircraft(TarCom)->Altitude == 0) {
					Assign_Target(TARGET_NONE);
					Status = SAM_LOCKING;
				} else {
					FireErrorType error = Can_Fire(TarCom, 0);
					if (error == FIRE_ILLEGAL || error == FIRE_CANT || error == FIRE_RANGE) {
						Assign_Target(TARGET_NONE);
						Status = SAM_LOCKING;
					} else {
						if (error == FIRE_FACING) {
							Status = SAM_READY;
						} else {
							if (error == FIRE_OK) {
								Fire_At(TarCom, 0);
								Status = SAM_READY2;
								return(1);
							}
						}
					}
				}
				return(1);

			case SAM_READY2:
				if (!Target_Legal(TarCom) || !Is_Target_Aircraft(TarCom) || As_Aircraft(TarCom)->Altitude == 0) {
					Assign_Target(TARGET_NONE);
					Status = SAM_LOCKING;
					return(TICKS_PER_SECOND);
				} else {
					if (!PrimaryFacing.Is_Rotating()) {
						DirType facing = Direction(TarCom);
						if (PrimaryFacing.Difference(facing)) {
							PrimaryFacing.Set_Desired(facing);
						} else {
							Status = SAM_FIRING2;
						}
					}
				}
				return(1);

			case SAM_FIRING2:
				if (!Target_Legal(TarCom) || !Is_Target_Aircraft(TarCom) || As_Aircraft(TarCom)->Altitude == 0) {
					Assign_Target(TARGET_NONE);
					Status = SAM_LOCKING;
				} else {
					FireErrorType error = Can_Fire(TarCom, 0);
					if (error == FIRE_ILLEGAL || error == FIRE_CANT || error == FIRE_RANGE) {
						Assign_Target(TARGET_NONE);
						Status = SAM_LOCKING;
					} else {
						if (error == FIRE_FACING) {
							Status = SAM_READY2;
						} else {
							if (error == FIRE_OK) {
								Fire_At(TarCom, 0);
								Status = SAM_LOCKING;
								return(TICKS_PER_SECOND*3);
							}
						}
					}
				}
				return(1);

			/*
			**	Rotating to face north in preparation for lowering to reload.
			*/
			case SAM_LOCKING:
				if (!PrimaryFacing.Is_Rotating()) {
					if (PrimaryFacing == DIR_N) {
						Set_Rate(2);
						Set_Stage(48);
						Status = SAM_LOWERING;
					} else {
						PrimaryFacing.Set_Desired(DIR_N);
					}
				}
				return(1);

			/*
			**	Lowering into the ground in order to reload.
			*/
			case SAM_LOWERING:
				if (Fetch_Stage() >= 63) {
					Set_Rate(0);
					Set_Stage(0);
					Status = SAM_UNDERGROUND;
					return(TICKS_PER_SECOND);
				} else {
					if (Fetch_Rate() == 0) {
						Set_Rate(2);
					}
				}
				return(1);

			default:
				break;
		}

	} else {
		IsReadyToCommence = true;
		switch (Can_Fire(TarCom, 0)) {
			case FIRE_ILLEGAL:
			case FIRE_CANT:
			case FIRE_RANGE:
			case FIRE_AMMO:
				Assign_Target(TARGET_NONE);
				Assign_Mission(MISSION_GUARD);
				Commence();
				break;

			case FIRE_FACING:
				PrimaryFacing.Set_Desired(Direction(TarCom));
				return(2);

			case FIRE_REARM:
			case FIRE_BUSY:
				return(1);

			case FIRE_CLOAKED:
				Do_Uncloak();
				break;

			case FIRE_OK:
				Fire_At(TarCom, 0);
				return(1);
		}
	}
	return(TICKS_PER_SECOND);
}


/***********************************************************************************************
 * BuildingClass::Mission_Harvest -- Handles refinery unloading harvesters.                    *
 *                                                                                             *
 *    This state machine handles the refinery when it unloads the harvester.                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of game frames to delay before calling this routine        *
 *          again.                                                                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int BuildingClass::Mission_Harvest(void)
{
	Validate();
	enum {
		INITIAL,					// Dock the Tiberium canister.
		WAIT_FOR_DOCK,			// Waiting for docking to complete.
		MIDDLE,					// Offload "bails" of tiberium.
		WAIT_FOR_UNDOCK,		// Waiting for undocking to complete.
		EXITING					// Cause the harvester to drive away.
	};
	/*
	**	A CrystAlien headquarters unloads on its own clock.
	**
	**	The sequence below steps on IsReadyToCommence, which a building raises as
	**	each of its docking animations finishes -- ACTIVE, then AUX1, then AUX2.
	**	CrystAlien's headquarters has no such animations, so nothing ever raised
	**	it: the machine stopped at WAIT_FOR_DOCK with the miner limboed inside,
	**	and neither the credits nor the miner came back out.
	**
	**	The source has no docking sequence to wait on either.  UnitNav hands the
	**	cargo over the moment it arrives, pauses briefly (`hesitate = 20`) and
	**	sends the miner out again.
	*/
	if (CAC_Is_Refinery(Class->Type) && !CAC_Is_Stock_Structure(Class->Type)) {
		FootClass *load = Attached_Object();

		if (load != NULL && load->Tiberium_Load() > 0) {
			int bail;
			while ((bail = load->Offload_Tiberium_Bail()) != 0) {

				/*
				**	Straight into the account, not into storage.
				**
				**	HouseClass::Harvested puts the load in a silo first and clamps
				**	it to the house's capacity -- and a CrystAlien headquarters has
				**	no capacity, because CrystAlien has no notion of storage at all:
				**	cashUp adds to the cash and that is the end of it.  So every
				**	load was being clamped to nothing and thrown away, which is
				**	exactly what "deposits but no money arrives" looks like.
				*/
				CAC_Credit_Harvest(House, bail);
			}

			/*
			**	One note for the delivery.  HUD.handle plays INT_cashup_6 while its
			**	counter catches up, which is a short run of them per payment -- not
			**	the continuous ring that following Tiberian Dawn's credit trickle
			**	frame by frame produced.
			*/
			if (House == PlayerPtr && Map.In_View(Coord_Cell(Center_Coord()))) {
				CAC_Interface_Sound(VOC_CAC_INT_CASH6);
			}
			return(CAC_UNLOAD_DELAY);
		}

		/*
		**	Back out and straight to work, as the source does -- returnHome ends
		**	with the miner heading for the field it was last feeding on.
		*/
		FootClass *leaving = Attached_Object();
		if (leaving != NULL && leaving->What_Am_I() == RTTI_UNIT) {
			UnitClass *miner = (UnitClass *)Detach_Object();

			/*
			**	Put it back on the map where it stood, and give it a track off the
			**	building.  This used to go through Exit_Object, which walks a unit
			**	out to a free cell from the exit list the way a barracks does --
			**	and a base built tight around its headquarters has no free cell,
			**	so the miner was detached and never put back: standing inside the
			**	building, unable to move.
			**
			**	A Tiberian Dawn refinery does not use the exit list either.  It
			**	unlimbos the harvester where it is, tethers it, sets it harvesting
			**	and forces a track out of the bay.  The same, from the dock.
			*/
			ScenarioInit++;
			bool out = miner->Unlimbo(Docking_Coord(), DIR_SW_X2);
			ScenarioInit--;
			if (!out && !Scan_Place_Object(miner, Coord_Cell(Center_Coord()))) {
				Attach(miner);
				return(CAC_UNLOAD_DELAY);
			}

			miner->PrimaryFacing = DIR_SW_X2;
			Transmit_Message(RADIO_HELLO, miner);
			Transmit_Message(RADIO_TETHER);

			/*
			**	And back to the tile it was feeding on.  A target left over from
			**	being shot at on the way home would otherwise send it round again
			**	without ever going back to work.
			*/
			miner->Assign_Target(TARGET_NONE);
			miner->Assign_Mission(MISSION_HARVEST);
			miner->Force_Track(DriveClass::OUT_OF_REFINERY,
									Cell_Coord(Adjacent_Cell(Coord_Cell(Docking_Coord()), FACING_SW)));
			miner->Set_Speed(128);
		}
		Assign_Mission(MISSION_GUARD);
		return(1);
	}

	switch (Status) {
		case INITIAL:
			Begin_Mode(BSTATE_ACTIVE);
			Status = WAIT_FOR_DOCK;
			break;

		case WAIT_FOR_DOCK:
			if (IsReadyToCommence) {
				IsReadyToCommence = false;
				Status = MIDDLE;
				Begin_Mode(BSTATE_AUX1);
			}
			break;

		case MIDDLE:
			if (IsReadyToCommence) {
				IsReadyToCommence = false;

				/*
				**	Force any bib squaters to scatter.
				*/
				bool old = Special.IsScatter;
				Special.IsScatter = true;
				Map[Adjacent_Cell(Coord_Cell(Center_Coord()), DIR_SW)].Incoming(0, true);
				Special.IsScatter = old;

				FootClass * techno = Attached_Object();
				if (techno) {
					int bail = techno->Offload_Tiberium_Bail();

					if (bail) {
						House->Harvested(bail);
						if (techno->Tiberium_Load()) {
							return(1);
						}
					}
				}
				Begin_Mode(BSTATE_AUX2);
				Status = WAIT_FOR_UNDOCK;
			}
			break;

		case WAIT_FOR_UNDOCK:
			if (IsReadyToCommence) {

				/*
				**	Detach harvester and go back into idle state.
				*/
				Exit_Object(Detach_Object());
				Assign_Mission(MISSION_GUARD);
			}
			break;
	}
	return(1);
}


/***********************************************************************************************
 * BuildingClass::Mission_Repair -- Handles the repair (active) state for building.            *
 *                                                                                             *
 *    This state machine is used when the building is active in some sort of repair or         *
 *    construction mode. The construction yard will animate. The repair facility will repair   *
 *    anything that it docked on it.                                                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of game frames to delay before calling this routine again. *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1995 JLB : Created.                                                                 *
 *   06/25/1995 JLB : Handles repair facility                                                  *
 *   07/29/1995 JLB : Repair rate is controlled by power rating.                               *
 *=============================================================================================*/
int BuildingClass::Mission_Repair(void)
{
	Validate();
	if (*this == STRUCT_CONST) {
		enum {
			INITIAL,
			DURING
		};
		switch (Status) {
			case INITIAL:
				Begin_Mode(BSTATE_ACTIVE);
				Status = DURING;
				break;

			case DURING:
				if (!In_Radio_Contact()) {
					Assign_Mission(MISSION_GUARD);
				}
				break;
		}
		return(1);
	}

	if (*this == STRUCT_REPAIR) {
		enum {
			INITIAL,
			IDLE,
			DURING
		};
		switch (Status) {
			case INITIAL:
				if (!In_Radio_Contact()) {
					Begin_Mode(BSTATE_IDLE);
					Assign_Mission(MISSION_GUARD);
					return(1);
				}
				IsReadyToCommence = false;
				if (Transmit_Message(RADIO_NEED_TO_MOVE) == RADIO_ROGER && Distance(Contact_With_Whom()) < 0x0010) {
					Status = IDLE;
					return(TICKS_PER_SECOND/4);
				}
				break;

			case IDLE:
				if (!In_Radio_Contact()) {
					Assign_Mission(MISSION_GUARD);
					return(1);
				}

				if (Transmit_Message(RADIO_NEED_TO_MOVE) == RADIO_ROGER) {
					if (Contact_With_Whom()->Health_Ratio() < 0x0100 && Transmit_Message(RADIO_REPAIR) == RADIO_ROGER) {

						// MBL 07.06.2020 - Patch 3: Change to TD Legacy: Adding "Repairing" VO for units on repair bay
						// Per https://jaas.ea.com/browse/TDRA-7271
						if (IsOwnedByPlayer && House) Speak(VOX_REPAIRING, House);

						Status = DURING;
						Begin_Mode(BSTATE_ACTIVE);
						IsReadyToCommence = false;
					} else {
						if (!House->IsHuman) {
							Transmit_Message(RADIO_RUN_AWAY);
						}
					}
//				} else {
//					Assign_Mission(MISSION_GUARD);
//					return(1);
				}
				break;

			case DURING:
				if (!In_Radio_Contact()) {
					Begin_Mode(BSTATE_IDLE);
					Status = IDLE;
					return(1);
				}
				if (IsReadyToCommence && Transmit_Message(RADIO_NEED_TO_MOVE) == RADIO_ROGER) {
					IsReadyToCommence = false;
					long param = Health_Ratio();
					if (Transmit_Message(RADIO_REPAIR, param) != RADIO_ROGER) {
#ifdef OBSOLETE
						if (House->Available_Money() < 10) {
							Transmit_Message(RADIO_RUN_AWAY);
						}
#endif
						Begin_Mode(BSTATE_IDLE);
						Status = IDLE;
#ifdef OBSOLETE
					} else {
						int time = Bound(Fixed_To_Cardinal(TICKS_PER_SECOND, House->Power_Fraction()), 0, TICKS_PER_SECOND);
						time = (TICKS_PER_SECOND + (TICKS_PER_SECOND/2)) - time;
						return(time);
#endif
					}
				}
				break;
		}
		return(TICKS_PER_SECOND/2);
	}

	if (*this == STRUCT_HELIPAD) {
		enum {
			INITIAL,
			DURING
		};
		switch (Status) {
			case INITIAL:
				if (Transmit_Message(RADIO_NEED_TO_MOVE) == RADIO_ROGER && Transmit_Message(RADIO_PREPARED) == RADIO_NEGATIVE) {
					Begin_Mode(BSTATE_ACTIVE);
					Contact_With_Whom()->Assign_Mission(MISSION_SLEEP);
					Status = DURING;
					return(1);
				}
				Assign_Mission(MISSION_GUARD);
				break;

			case DURING:
				if (IsReadyToCommence) {
					if (!In_Radio_Contact() || Transmit_Message(RADIO_NEED_TO_MOVE) == RADIO_NEGATIVE) {
						Assign_Mission(MISSION_GUARD);
						return(1);
					}

					if (Transmit_Message(RADIO_PREPARED) == RADIO_ROGER) {
						Contact_With_Whom()->Assign_Mission(MISSION_GUARD);
						Assign_Mission(MISSION_GUARD);
						return(1);
					}

					if (Transmit_Message(RADIO_RELOAD) != RADIO_ROGER) {
						Assign_Mission(MISSION_GUARD);
						Contact_With_Whom()->Assign_Mission(MISSION_GUARD);
						return(1);
					} else {
						int time = Bound(Fixed_To_Cardinal(TICKS_PER_SECOND, House->Power_Fraction()), 0, TICKS_PER_SECOND);
						time = (TICKS_PER_SECOND*3) - time;
						IsReadyToCommence = false;
						return(time);
					}
				}
				break;
		}
		return(3);
	}
	return(TICKS_PER_SECOND);
}


/***********************************************************************************************
 * BuildingClass::Mission_Missile -- State machine for nuclear missile launch.                 *
 *                                                                                             *
 *    This handles the Temple of Nod launching its nuclear missile.                            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of frames to delay before calling this routine again.      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/04/1995 JLB : Commented.                                                               *
 *=============================================================================================*/
int BuildingClass::Mission_Missile(void)
{
	Validate();
	enum {
		INITIAL,
		DOOR_OPENING,
		LAUNCH_UP,
		LAUNCH_DOWN,
		DONE_LAUNCH
	};

	if (*this == STRUCT_TEMPLE) {
		switch (Status) {

			/*
			** The initial case is responsible for starting the door
			** opening on the building.
			*/
			case INITIAL:
				IsReadyToCommence = false;
				Begin_Mode(BSTATE_ACTIVE);
				Status = DOOR_OPENING;
				return(1);

			/*
			** This polls for the case when the door is actually open and
			** then kicks off the missile smoke.
			*/
			case DOOR_OPENING:
				if (IsReadyToCommence) {
					Begin_Mode(BSTATE_IDLE);
					new AnimClass(ANIM_ATOM_DOOR, Center_Coord());
					Status = LAUNCH_UP;
					return(14);
				}
				return(1);

			/*
			** Once the smoke has been going for a little while this
			** actually handles launching the missile into the air.
			*/
			case LAUNCH_UP:
				{
					BulletClass *bullet = new BulletClass(BULLET_NUKE_UP);
					if (bullet) {
						COORDINATE launch = Coord_Move(Center_Coord(), (DirType)1, 0x1A0);
						bullet->Assign_Target(TARGET_NONE);
						bullet->Payback = NULL;
						bullet->Strength = 1;
						if (!bullet->Unlimbo(launch, DIR_N)) {
							delete bullet;
							bullet = NULL;
						} else {
							bullet->PrimaryFacing.Set_Current(DIR_N);
							Sound_Effect(VOC_NUKE_FIRE, launch);

							// MBL 03.27.2020 This is never getting triggered for any player in multiplayer, so removing the check (https://jaas.ea.com/browse/TDRA-5458)
							// if (House == PlayerPtr) 
							{
								Speak(VOX_NUKE_LAUNCHED); // "NUKLNCH1" - "Nuclear Weapon Launched"
							}
						}
					}

					if (bullet) {
						Status = LAUNCH_DOWN;
						return(8 * TICKS_PER_SECOND);
					}
				}
				return(1);

			/*
			** Once the missile is in the air, this handles waiting for
			** the missile to be off the screen and then launching one down
			** over the target.
			*/
			case LAUNCH_DOWN:
				{
				 	BulletClass *bullet = new BulletClass(BULLET_NUKE_DOWN);
					if (bullet) {
//						Theme.Queue_Song(THEME_NONE);
						COORDINATE start = Cell_Coord(XY_Cell(Cell_X(House->NukeDest), 1));
						bullet->Assign_Target(::As_Target(House->NukeDest));

						// MBL 05.20.2020
						// Fix for Nuke or Atom Bomb killing structures and units during animation sequence and not getting kills tracked
						// Per https://jaas.ea.com/browse/TDRA-6610
						//
						// bullet->Payback = NULL;
						bullet->Payback = this;

						bullet->Strength = 1;
						if (!bullet->Unlimbo(start, DIR_S)) {
							delete bullet;
						} else {
							bullet->PrimaryFacing.Set_Current(DIR_S);
						}
						Speak(VOX_INCOMING_NUKE); // "Nuclear Warhead Approaching" - "NUKE1"
						Sound_Effect(VOC_NUKE_FIRE, start);
					}
					if (bullet) {
						Status = DONE_LAUNCH;
						return(7 * TICKS_PER_SECOND);
					}
				}
				return(1);

			/*
			** Once the missile is done launching this handles allowing
			** the building to sit there with its door open.
			*/
			case DONE_LAUNCH:
				Assign_Mission(MISSION_GUARD);
				return(60);
		}
	}
	return(60);
}


/***********************************************************************************************
 * BuildingClass::Revealed -- Reveals the building to the specified house.                     *
 *                                                                                             *
 *    This routine will reveal the building to the specified house. It will handle updating    *
 *    the sidebar for player owned buildings. A player owned building that hasn't been         *
 *    revealed, is in a state of pseudo-limbo. It cannot be used for any of its special        *
 *    abilities even though it exists on the map for all other purposes.                       *
 *                                                                                             *
 * INPUT:   house -- The house that this building is being revealed to.                        *
 *                                                                                             *
 * OUTPUT:  Was this building revealed by this procedure?                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool BuildingClass::Revealed(HouseClass * house)
{
	Validate();
	if (TechnoClass::Revealed(house)) {

		if (!ScenarioInit) {
			House->JustBuilt = Class->Type;
		}
		House->IsRecalcNeeded = true;

		/*
		**	Perform any grand opening here so that in the scenarios where a player
		**	owned house is not yet revealed, it won't be reflected in the sidebar
		**	selection icons.
		*/
		/*
		** Making a change here to avoid Grand_Opening happening multiple times in MP/skirmish. ST - 7/26/2019 11:26AM
		*/
		//if (!In_Radio_Contact() && (house == House || GameToPlay != GAME_NORMAL) && Mission != MISSION_CONSTRUCTION) {
		if (!In_Radio_Contact() && (house == House && GameToPlay == GAME_NORMAL) && Mission != MISSION_CONSTRUCTION) {
			Grand_Opening();
		}

		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * BuildingClass::Enter_Idle_Mode -- The building will enter its idle mode.                    *
 *                                                                                             *
 *    This routine is called when the exact mode of the building isn't known. By examining     *
 *    the building's condition, this routine will assign an appropriate mission.               *
 *                                                                                             *
 * INPUT:   initial  -- This this being called during scenario init?                           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void BuildingClass::Enter_Idle_Mode(bool initial)
{
	Validate();
	/*
	**	Assign an appropriate mission for the building. If the ScenarioInit flag is true, then
	**	this must be an initial building. Start such buildings in idle state. For other buildings
	**	it indicates that it is being placed during game play and thus it must start in
	**	the "construction" mission.
	*/
	MissionType mission = MISSION_GUARD;
	if (!initial || ScenarioInit || Debug_Map) {
		Begin_Mode(BSTATE_IDLE);
		mission = MISSION_GUARD;
	} else {
		Begin_Mode(BSTATE_CONSTRUCTION);
		mission = MISSION_CONSTRUCTION;
	}
	Assign_Mission(mission);
}


/***************************************************************************
 * BuildingClass::Update_Specials -- removes computer specials             *
 *                                                                         *
 *                                                                         *
 *                                                                         *
 * INPUT:                                                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   06/21/1995 PWG : Created.                                             *
 *=========================================================================*/
void BuildingClass::Update_Specials(void)
{
	Validate();
}


/***********************************************************************************************
 * BuildingClass::Pip_Count -- Determines "full" pips to display for building.                 *
 *                                                                                             *
 *    This routine will determine the number of pips that should be filled in when rendering   *
 *    the building.                                                                            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the number of pips to display as filled in.                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int BuildingClass::Pip_Count(void) const
{
	Validate();
	return(Fixed_To_Cardinal(Class->Max_Pips(), House->Tiberium_Fraction()));
}


/***********************************************************************************************
 * BuildingClass::Death_Announcement -- Announce the death of this building.                   *
 *                                                                                             *
 *    This routine is called when the building is destroyed by "unnatural" means. Typically    *
 *    as a result of combat. If the building is known to the player, then it should be         *
 *    announced.                                                                               *
 *                                                                                             *
 * INPUT:   source   -- The object most directly responsible for the building's death.         *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/04/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void BuildingClass::Death_Announcement(TechnoClass const * ) const
{
	Validate();
	//Changed for multiplayer ST - 3/13/2019 5:31PM
	if (Is_Discovered_By_Player() || Is_Owned_By_Player()) {
	//if (IsDiscoveredByPlayer || IsOwnedByPlayer) {
		if (House != PlayerPtr && GameToPlay != GAME_NORMAL) {
			if (Options.IsDeathAnnounce) Speak(VOX_ENEMY_STRUCTURE);
		} else {
			if (House == PlayerPtr || Options.IsDeathAnnounce) {
				if (!Options.IsDeathAnnounce) {
					Speak(VOX_STRUCTURE_LOST);
				} else {
					switch (House->ActLike) {
						case HOUSE_GOOD:
							Speak(VOX_GDI_STRUCTURE);
							break;

						case HOUSE_BAD:
							Speak(VOX_NOD_STRUCTURE);
							break;

						default:
							break;
					}
				}
			}
		}
	}
}


/***********************************************************************************************
 * BuildingClass::Fire_Direction -- Fetches the direction of firing.                           *
 *                                                                                             *
 *    This routine will return with the default direction to use when firing from this         *
 *    building. This is the facing of the turret except for the case of non-turret equipped    *
 *    buildings that have a weapon (e.g., guard tower).                                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the default firing direction for this building.                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/04/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
DirType BuildingClass::Fire_Direction(void) const
{
	Validate();
	if (Class->IsTurretEquipped) {
		return(PrimaryFacing.Current());
	}
	return(Direction(TarCom));
}


/***********************************************************************************************
 * BuildingClass::Remap_Table -- Fetches the remap table to use for this building.             *
 *                                                                                             *
 *    Use this routine to fetch the remap table to use.  This override function is needed      *
 *    because the default remap table for techno objects presumes the object is a unit.        *
 *    Buildings aren't units.                                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the proper remap table to use for this building.                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void const * BuildingClass::Remap_Table(void)
{
	Validate();
	return(House->Remap_Table(IsBlushing, false));
}


/***********************************************************************************************
 * BuildingClass::Mission_Unload -- Handles the unload mission for a building.                 *
 *                                                                                             *
 *    This is the unload mission for a building. This really only applies to the weapon's      *
 *    factory, since it needs the sophistication of an unload mission due to the door          *
 *    animation.                                                                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of game frames to delay before calling this routine        *
 *          again.                                                                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int BuildingClass::Mission_Unload(void)
{
	Validate();
	if (*this == STRUCT_WEAP) {
		COORDINATE coord = Adjacent_Cell(Center_Coord(), FACING_SW);
		CELL cell = Coord_Cell(coord);
		CellClass * cellptr = &Map[cell];
		enum {
			INITIAL,
			CLEAR_BIB,
			OPEN,
			LEAVE,
			CLOSE
		};
		UnitClass * unit;
		switch (Status) {
			case INITIAL:
				unit = (UnitClass *)Contact_With_Whom();
				if (unit) {
					unit->Assign_Mission(MISSION_GUARD);
					unit->Commence();
				}
				Open_Door(2, 11);
				Status = CLEAR_BIB;
				break;

			/*
			**	Now that the occupants can peek out the door, they will tell
			**	everyone that could be blocking the way, that they should
			**	scatter away.
			*/
			case CLEAR_BIB:
				unit = (UnitClass *)Contact_With_Whom();
				if (cellptr->Cell_Unit() || cellptr->Cell_Infantry()) {
					cellptr->Incoming(0, true, true);

					/*
					**	Scatter everything around the weapon's factory door.
					*/
					for (FacingType f = FACING_FIRST; f < FACING_COUNT; f++) {
						CellClass * cptr = cellptr->Adjacent_Cell(f);
						if (!cptr) continue;
						UnitClass * cellunit = cptr->Cell_Unit();
						if ((cellunit && cellunit != unit) || cptr->Cell_Infantry()) {
							cptr->Incoming(coord, true, true);
						}
					}
				} else {
					Status = OPEN;
				}
				break;

			case OPEN:
				if (Is_Door_Open()) {
					unit = (UnitClass *)Contact_With_Whom();
					if (unit) {
						unit->Assign_Mission(MISSION_MOVE);
						unit->Force_Track(DriveClass::OUT_OF_WEAPON_FACTORY, Adjacent_Cell(Center_Coord(), FACING_SW));
						unit->Set_Speed(128);
						Status = LEAVE;
					} else {
						Close_Door(2, 11);
						Status = CLOSE;
					}
				}
				break;

			case LEAVE:
				if (!IsTethered) {
					Close_Door(2, 11);
					Status = CLOSE;
				}
				break;

			case CLOSE:
				if (Is_Door_Closed()) {
					Enter_Idle_Mode();
				}
				break;
		}
		return(TICKS_PER_SECOND/2);
	}
	Assign_Mission(MISSION_GUARD);
	return(TICKS_PER_SECOND);
}


/***********************************************************************************************
 * BuildingClass::Power_Output -- Fetches the current power output from this building.         *
 *                                                                                             *
 *    This routine will return the current power output for this building. The power output    *
 *    is adjusted according to the damage level of the building.                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the current power output for this building.                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int BuildingClass::Power_Output(void) const
{
	Validate();
	if (Class->Power) {
		return(Fixed_To_Cardinal(Class->Power, Cardinal_To_Fixed(Class->MaxStrength, LastStrength)));
	}
	return(0);
}


/***********************************************************************************************
 * BuildingClass::Detach -- Handles target removal from the game system.                       *
 *                                                                                             *
 *    This routine is called when the specified target is about to be removed from the game    *
 *    system.                                                                                  *
 *                                                                                             *
 * INPUT:   target   -- The target to be removed from this building's targeting computer.      *
 *                                                                                             *
 *          all      -- Is the target about to be completely eliminated?                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void BuildingClass::Detach(TARGET target, bool all)
{
	Validate();
	TechnoClass::Detach(target, all);
	if (target == WhomToRepay) {
		WhomToRepay = TARGET_NONE;
	}
}


/***********************************************************************************************
 * BuildingClass::Refund_Amount -- Fetches the refund amount if building is sold.              *
 *                                                                                             *
 *    This routine will return the amount of money to be refunded to the building's owner      *
 *    if the building is sold.                                                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the refund amount available for this building.                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int BuildingClass::Refund_Amount(void) const
{
	Validate();

	/*
	**	What a CrystAlien structure is worth back:
	**
	**	    cash += Math.round(this.stats.cost * 0.5 * (this.healthPerc / 100));
	**
	**	-- half, scaled by how much of it is still standing, and half for whoever
	**	owns it.  TechnoClass::Refund_Amount halves only for a human, so the
	**	computer was being handed the whole cost back, and neither side lost
	**	anything for selling a building that had been shot to pieces.
	*/
	if (CAC_Is_CrystAlien_Type(Class->IniName)) {
		int full = Class->Raw_Cost() * House->CostBias;
		int perc = 100;
		if (Class->MaxStrength > 0) {
			perc = ((int)Strength * 100 + Class->MaxStrength - 1) / Class->MaxStrength;
		}
		return((full * perc) / 200);
	}

	int cost = TechnoClass::Refund_Amount();

	/*
	**	Add in any Tiberium that was stored within the building.
	*/
	if (IsV107 && Class->Capacity > 0) {
		cost += Fixed_To_Cardinal(Class->Capacity, Cardinal_To_Fixed(House->Capacity, House->Tiberium));
	}
	return(cost);
}


/***********************************************************************************************
 * BuildingClass::Crew_Type -- This determines the crew that this object generates.            *
 *                                                                                             *
 *    When selling very cheap buildings (such as the silo), a technician will pop out since    *
 *    generating minigunners would be overkill -- the player could use this loophole to        *
 *    gain an advantage.                                                                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the infantry type that this building will generate as a survivor.          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/05/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
InfantryType BuildingClass::Crew_Type(void) const
{
	Validate();

	/*
	**	CrystAlien has no civilians, so a CrystAlien structure only ever
	**	gives up its own Infantry.  See CAC_Crew_Type.
	*/
	InfantryType cac = CAC_Crew_Type(Class);
	if (cac != INFANTRY_NONE) return(cac);

	switch (Class->Type) {
		case STRUCT_STORAGE:
			if (Random_Pick(0, 1) == 0) {
				return(INFANTRY_C1);
			} else {
				return(INFANTRY_C7);
			}

		case STRUCT_CONST:
			if (!IsCaptured && House->IsHuman && Random_Pick(0, 3) == 0) {
				return(INFANTRY_E7);
			}
			break;

		default:
			break;
	}
	return(TechnoClass::Crew_Type());
}


/***********************************************************************************************
 * BuildingClass::Detach_All -- Possibly abandons production according to factory type.        *
 *                                                                                             *
 *    When this routine is called, it indicates that the building is about to be destroyed     *
 *    or captured. In such a case any production it may be doing, must be abandoned.           *
 *                                                                                             *
 * INPUT:   all   -- Is the object about the be completely destroyed?                          *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/05/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void BuildingClass::Detach_All(bool all)
{
	Validate();
	/*
	**	If it is producing something, then it must be abandoned.
	*/
	if (Factory) {
		Factory->Abandon();
		delete Factory;
		Factory = 0;
	}

	/*
	** If the owner HouseClass is building something, and this building can
	** build that thing, we may be the last building for that house that can
	** build that thing; if so, abandon production of it.
	*/
	if (House) {
		int fnum = -1;

		switch (Class->ToBuild) {
			case RTTI_AIRCRAFTTYPE:
				fnum = House->AircraftFactory;
				break;

			case RTTI_INFANTRYTYPE:
				fnum = House->InfantryFactory;
				break;

			case RTTI_UNITTYPE:
				fnum = House->UnitFactory;
				break;

			case RTTI_BUILDINGTYPE:
				fnum = House->BuildingFactory;
				break;

			case RTTI_SPECIAL:
				fnum = House->SpecialFactory;
				break;

		}

		/*
		**	Convert the factory number into a real factory pointer.
		*/
		FactoryClass * factory = 0;
		if (fnum != -1) {
			factory = Factories.Raw_Ptr(fnum);
		}

		/*
		**	If a factory was found, then temporarily disable this building and then
		**	detmermine if any object that is being produced can still be produced. If
		**	not, then the object being produced must be abandoned.
		*/
		if (factory) {
			TechnoClass * object = factory->Get_Object();
			IsInLimbo = true;
			if (object && !object->Techno_Type_Class()->Who_Can_Build_Me(true, false, House->Class->House)) {
				House->Abandon_Production(Class->ToBuild);
			}
			IsInLimbo = false;
		}
	}

	TechnoClass::Detach_All(all);
}


/***********************************************************************************************
 * BuildingClass::Flush_For_Placement -- Handles clearing a zone for object placement.         *
 *                                                                                             *
 *    This routine is used to clear the way for placement of the specified object (usually     *
 *    a building). If there are friendly units blocking the placement area, they are told      *
 *    to scatter. Enemy blocking units are attacked.                                           *
 *                                                                                             *
 * INPUT:   techno   -- Pointer to the object that is desired to be placed.                    *
 *                                                                                             *
 *          cell     -- The cell that placement wants to occur at.                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/06/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool BuildingClass::Flush_For_Placement(TechnoClass * techno, CELL cell)
{
	Validate();
	bool again = false;
	if (techno && cell > 0) {
		short const * list = techno->Class_Of().Occupy_List(true);

		while (*list != REFRESH_EOL) {
			CELL newcell = cell + *list++;

			if (Map.In_Radar(newcell)) {
				TechnoClass * occupier = Map[newcell].Cell_Techno();
				if (occupier) {
					again = true;
					if (occupier->House->Is_Ally(this)) {
						Map[newcell].Incoming(0, true);
					} else {
						Base_Is_Attacked(occupier);
					}
				}
			}
		}
	}
	return(again);
}


void BuildingClass::Hidden(void)
{
//	if (IsDiscoveredByPlayer && House->IsHuman) {
//		House->Adjust_Drain(-Class->Drain);
//	}
	TechnoClass::Hidden();
}


CELL BuildingClass::Find_Exit_Cell(TechnoClass const * techno) const
{
	CELL const *ptr;
	CELL origin = Coord_Cell(Coord);
	bool found = false;

	ptr = Class->ExitList;
	if (ptr) {
		while (*ptr != REFRESH_EOL) {
			CELL cell = origin + *ptr++;
			if (Map.In_Radar(cell) && techno->Can_Enter_Cell(cell) == MOVE_OK) {
				return(cell);
			}
		}
	}
	return(0);
}

/***********************************************************************************************
 * BuildingClass::Can_Player_Move -- Can this building be moved?                               *
 *                                                                                             *
 *    This routine answers the question 'can this building be moved?' Typically, only the      *
 *    construction yard can be moved and it does this by undeploying back into a MCV.          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Can the building move to a new location under player control?                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/04/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool BuildingClass::Can_Player_Move(void) const
{
	Validate();
	return(CAC_Undeploys_Into(Class->Type) != UNIT_NONE && (Mission == MISSION_GUARD) && Special.IsMCVDeploy);
}

/***********************************************************************************************
 * BuildingClass::Passes_Proximity_Check -- Determines if building placement is near friendly sq*
 *                                                                                             *
 *    This routine is used by the building placement cursor logic to determine whether the     *
 *    at the current cursor position if the building would be adjacent to another friendly     *
 *    building. In cases where this is not true, then the building cannot be placed at all.    *
 *    This determination is returned by the function.                                          *
 *                                                                                             *
 * INPUT:   homecell -- The cell that the building would like to be placed down at.            *
 *                                                                                             *
 * OUTPUT:  bool; Can the pending building object be placed at the present cursor location     *
 *                checking only for proximity to friendly buildings?  If this isn't for a      *
 *                building type object, then this routine always returns true.                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/06/1994 JLB : Created.                                                                 *
 *   06/07/1994 JLB : Handles concrete check.                                                  *
 *=============================================================================================*/
bool BuildingClass::Passes_Proximity_Check(CELL homecell)
{
	/*
	** In editor mode, the proximity check always passes.
	*/
	if (Debug_Map) {
		return(true);
	}

	/*
	**	The MT-201 Driller, wherever the request came from.
	**	
	**	This is the check HouseClass runs when the building is actually placed --
	**	"even when the building is placed by way of a remote event" -- so it is the
	**	one that has to hold.  The placement overlay the player sees is built
	**	elsewhere and can be told anything; this is where it is settled.
	**	
	**	Asked before the computer's exemption below, not after.  A computer's site
	**	comes from CAC_Find_Crystal_Site and is already on crystal, so this costs it
	**	nothing -- but "the AI is trusted" is not a reason for the rule to have a
	**	hole in it.
	*/
	if (Class != NULL && CAC_Needs_Crystal_Site(Class->Type)) {
		return(CAC_Crystal_Site_Ok(Class->Type, homecell));
	}

	/*
	**	And the rest of ours answer CrystAlien's adjacency rule, which is about
	**	power rather than about buildings.  See CAC_Power_Adjacency_Ok -- the
	**	anchor has to be a friendly building whose power is non-zero, and a
	**	building whose own power is zero needs no anchor.
	*/
	if (Class != NULL && CAC_Is_CrystAlien_Type(Class->IniName)) {
		return(CAC_Power_Adjacency_Ok(Class->Type, House->Class->House,
												Occupy_List(true), homecell));
	}

	if (!House->IsHuman) {
		return(true);
	}

	/*
	**	Scan through all cells that the building foundation would cover. If any adjacent
	**	cells to these are of friendly persuasion, then consider the proximity check to
	**	have been a success.
	*/
	short const * ptr = Occupy_List(true);
	while (*ptr != REFRESH_EOL) {
		CELL cell = homecell + *ptr++;

		if (!Map.In_Radar(cell)) return(false);

		for (FacingType facing = FACING_N; facing < FACING_COUNT; facing++) {
			CELL	newcell = Adjacent_Cell(cell, facing);

			BuildingClass * base = Map[newcell].Cell_Building();

			/*
			**	The special cell ownership flag allows building adjacent
			**	to friendly walls and bibs even though there is no official
			**	building located there.
			*/
			if (Map[newcell].Owner == House->Class->House) {
				return(true);
			}

			if (base && base->House->Class->House == House->Class->House) {
				return(true);
			}
		}
	}
	return(false);
}