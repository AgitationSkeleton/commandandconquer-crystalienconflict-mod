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

/* $Header:   F:\projects\c&c\vcs\code\factory.cpv   2.18   16 Oct 1995 16:51:26   JOE_BOSTIC  $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : FACTORY.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 12/26/94                                                     *
 *                                                                                             *
 *                  Last Update : May 22, 1995 [JLB]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   FactoryClass::AI -- Process factory production logic.                                     *
 *   FactoryClass::Abandon -- Abandons current construction with money refunded.               *
 *   FactoryClass::Completed -- Clears factory object after a completed production process.    *
 *   FactoryClass::Completion -- Fetchs the completion step for this factory.                  *
 *   FactoryClass::Cost_Per_Tick -- Breaks entire production cost into managable chunks.       *
 *   FactoryClass::FactoryClass -- Default constructor for factory objects.                    *
 *   FactoryClass::Get_Object -- Fetches pointer to object being constructed.                  *
 *   FactoryClass::Get_Special_Item -- gets factorys spc prod item                             *
 *   FactoryClass::Has_Changed -- Checks to see if a production step has occurred?             *
 *   FactoryClass::Has_Completed -- Checks to see if object has completed production.          *
 *   FactoryClass::Set -- Assigns a factory to produce an object.                              *
 *   FactoryClass::Set -- Fills a factory with an already completed object.                    *
 *   FactoryClass::Set -- Force factory to "produce" special object.                           *
 *   FactoryClass::Start -- Resumes production after suspension or creation.                   *
 *   FactoryClass::Suspend -- Temporarily stop production.                                     *
 *   FactoryClass::operator delete -- Returns a factory to the free factory pool.              *
 *   FactoryClass::operator new -- Allocates a factory object from the free factory pool.      *
 *   FactoryClass::~FactoryClass -- Default destructor for factory objects.                    *
 *   FactoryClass::Validate -- validates factory pointer													  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"function.h"
#include	"CACLOGIC.H"


/***********************************************************************************************
 * FactoryClass::Validate -- validates factory pointer													  *
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
int FactoryClass::Validate(void) const
{
	int num;

	num = Factories.ID(this);
	if (num < 0 || num >= FACTORY_MAX) {
		Validate_Error("FACTORY");
		return (0);
	}
	else
		return (1);
}
#else
#define	Validate()
#endif


/***********************************************************************************************
 * FactoryClass::FactoryClass -- Default constructor for factory objects.                      *
 *                                                                                             *
 *    This brings the factory into a null state. It is called when a factory object is         *
 *    created.                                                                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
FactoryClass::FactoryClass(void)
{
	IsSuspended = false;
	IsDifferent = false;
	IsBlocked	= false;
	Balance		= 0;
	SpecialItem = SPC_NONE;
	Object 		= NULL;
	House			= NULL;
	Set_Rate(0);
	Set_Stage(0);
}


/***********************************************************************************************
 * FactoryClass::~FactoryClass -- Default destructor for factory objects.                      *
 *                                                                                             *
 *    This cleans up a factory object in preparation for deletion. If there is currently       *
 *    an object in production, it is abandoned and money is refunded.                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
FactoryClass::~FactoryClass(void)
{
	if (GameActive) {
		Abandon();
	}
}


/***********************************************************************************************
 * FactoryClass::Init -- Clears all units for scenario preparation.                            *
 *                                                                                             *
 *    This routine will zero out the factory list and objects. This routine is typically       *
 *    used in preparation for a new scenario load. All factorys are guaranteed to be eliminated*
 *    by this routine.                                                                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/15/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void FactoryClass::Init(void)
{
	Factories.Free_All();
}


/***********************************************************************************************
 * FactoryClass::operator new -- Allocates a factory object from the free factory pool.        *
 *                                                                                             *
 *    This routine allocates a factory from the free factory pool. If there is no more room    *
 *    to allocate a factory, then NULL is returned.                                            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with pointer to the newly allocated factory object.                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void * FactoryClass::operator new(size_t)
{
	void * ptr = Factories.Allocate();
	if (ptr) {
		((FactoryClass *)ptr)->IsActive = true;
	}
	return(ptr);
}


/***********************************************************************************************
 * FactoryClass::operator delete -- Returns a factory to the free factory pool.                *
 *                                                                                             *
 *    This returns the factory object back to the factory allocation pool. The factory is then *
 *    available to be allocated.                                                               *
 *                                                                                             *
 * INPUT:   ptr   -- Pointer to the factory object to delete.                                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void FactoryClass::operator delete(void *ptr)
{
	if (ptr) {
		((FactoryClass *)ptr)->IsActive = false;
	}
	Factories.Free((FactoryClass *)ptr);
}


/***********************************************************************************************
 * FactoryClass::AI -- Process factory production logic.                                       *
 *                                                                                             *
 *    This routine should be called once per game tick. It handles the production process.     *
 *    As production proceeds, money is deducted from the owner object's house. When production *
 *    completes, the factory stop processing. A call to Abandon, Delete, or Completed is       *
 *    required after that point.                                                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *   01/04/1995 JLB : Uses exact installment payment method.                                   *
 *=============================================================================================*/
void FactoryClass::AI(void)
{
	Validate();
	if (!IsSuspended && (Object != NULL || SpecialItem)) {

		/*
		**	Construction.think reads its speed fresh every frame:
		**
		**		var speed = this.speed;
		**		if (this.parent.powerLow) speed *= 0.5;
		**
		**	so a base that browns out mid-build slows immediately rather than
		**	keeping the rate it was queued at.  Tiberian Dawn counts a delay
		**	between steps instead of a speed, so half the speed is twice the
		**	delay.  Only CrystAlien's own types are paced this way.
		**
		**	The delay is worked out one step at a time, and that is the point.
		**	A factory runs twenty-seven steps at a fixed number of ticks each,
		**	so `Time_To_Build / STEP_COUNT` can only ever produce a multiple of
		**	twenty-seven ticks: a Trike wanting forty-one got twenty-seven, a
		**	third off, and cheaper things fared worse.  Asking instead where
		**	step k *ends* --
		**
		**		floor((k+1) * total / 27) - floor(k * total / 27)
		**
		**	-- spreads the remainder across the steps that need it, and the
		**	twenty-seven add back up to `total` exactly.  The stage counter says
		**	which step is running, so nothing has to be remembered between
		**	ticks, and because this sits above the Graphic_Logic call the reset
		**	Set_Rate performs lands on the same tick as that step's first
		**	decrement rather than costing one.
		*/
		int cacsteps = 0;
		if (Object != NULL && House != NULL && Object->Techno_Type_Class() != NULL &&
			CAC_Is_CrystAlien_Type(Object->Techno_Type_Class()->IniName)) {
			int total = Object->Class_Of().Time_To_Build(House->Class->House);
			if (CAC_Power_Low(House)) total *= 2;

			int done = Fetch_Stage();
			cacsteps = 1;

			if (done < STEP_COUNT) {
				if (total >= STEP_COUNT) {

					/*
					**	The ordinary case: one step every so many ticks.
					*/
					int want = (((done + 1) * total) / STEP_COUNT) - ((done * total) / STEP_COUNT);
					want = Bound(want, 1, 255);
					if (Fetch_Rate() != want) Set_Rate(want);
				} else {

					/*
					**	And the fast case, which the Astro Explorer and its Alien
					**	counterpart are squarely in: two hundred credits is sixteen
					**	ticks, and twenty-seven steps do not fit into sixteen ticks
					**	one at a time.  A step cannot take less than a tick, so take
					**	several in a tick instead -- the loop below already runs the
					**	step body more than once, for factory acceleration.
					**
					**	Which tick this is follows from how many steps are done, so
					**	again nothing is remembered between ticks: step k cannot
					**	have finished before tick ceil(k * total / 27).
					*/
					if (Fetch_Rate() != 1) Set_Rate(1);
					int tick = (((done * total) + STEP_COUNT - 1) / STEP_COUNT) + 1;
					cacsteps = ((tick * STEP_COUNT) / total) - done;
					cacsteps = Bound(cacsteps, 1, STEP_COUNT - done);
				}
			}
		}

		int stages = 1;

		/*
		**	Determine the acceleration factor for factory production.
		**	This applies only to human players. The computer builds
		**	units on a building by building basis -- quantity of building
		**	factory types doesn't affect individual factories.
		*/
		if (Object && House->IsHuman) {
			switch (Object->What_Am_I()) {
				case RTTI_AIRCRAFT:
					stages = House->AircraftFactories;
					break;

				case RTTI_INFANTRY:
					stages = House->InfantryFactories;
					break;

				case RTTI_UNIT:
					stages = House->UnitFactories;
					break;

				case RTTI_BUILDING:
					stages = House->BuildingFactories;
					break;
			}
			stages = MAX(stages, 1);
		}

		/*
		**	A CrystAlien type ignores all of that.  Tiberian Dawn builds faster
		**	the more factories of a kind you own, stepping production once per
		**	factory per tick; Construction.think advances by a flat eight a frame
		**	however many buildings there are, and with a queue per host the count
		**	runs up quickly, so this was multiplying the tempo several times over.
		**	What is left is the schedule worked out above, which is the same for
		**	the computer as for the player -- the source draws no distinction.
		*/
		if (cacsteps > 0) stages = cacsteps;


		for (int index = 0; index < stages; index++) {
			if (!Has_Completed() && Graphic_Logic()) {
				IsDifferent = true;

				int cost = Cost_Per_Tick();

				cost = MIN(cost, Balance);

				/*
				**	Enough time has expired so that another production step can occur.
				**	If there is insufficient funds, then go back one production step and
				**	continue the countdown. The idea being that by the time the next
				**	production step occurs, there may be sufficient funds available.
				*/
				if (cost > House->Available_Money()) {
					Set_Stage(Fetch_Stage()-1);
				} else {
					House->Spend_Money(cost);
					Balance -= cost;
				}
				if ( Debug_Instant_Build ) {
					Set_Stage(STEP_COUNT);
				}
				/*
				**	If the production has completed, then suspend further production.
				*/
				if (Fetch_Stage() == STEP_COUNT) {
					IsSuspended = true;
					Set_Rate(0);
					House->Spend_Money(Balance);
					Balance = 0;
				}
			}
		}
	}
}



/***********************************************************************************************
 * FactoryClass::Force_Complete -- Force the factory to finish what it's building              *
 *                                                                                             *
 *    For debugging/testing support                                                            *
 *                                                                                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/23/2019 3:54PM ST : Created.                                                            *
 *=============================================================================================*/
void FactoryClass::Force_Complete(void)
{
	Validate();
	if (!IsSuspended && (Object != NULL || SpecialItem)) {
		Set_Stage(STEP_COUNT);
		IsSuspended = true;
		Set_Rate(0);
		Balance = 0;
		IsDifferent = true;
	}
}


/***********************************************************************************************
 * FactoryClass::Has_Changed -- Checks to see if a production step has occurred?               *
 *                                                                                             *
 *    Use this routine to determine if production has advanced at least one step. By using     *
 *    this function, intelligent rendering may be performed.                                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Has the production process advanced one step since the last time this        *
 *                function was called?                                                         *
 *                                                                                             *
 * WARNINGS:   This function clears the changed status flag as a side effect.                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FactoryClass::Has_Changed(void)
{
	Validate();
	bool changed = IsDifferent;
	IsDifferent = false;
	return(changed);
}


/***********************************************************************************************
 * FactoryClass::Set -- Assigns a factory to produce an object.                                *
 *                                                                                             *
 *    This routine initializes a factory to produce the object specified. The desired object   *
 *    type is created and placed in suspended animation (limbo) until such time as production  *
 *    completes. Production is not actually started by this routine. An explicit call to       *
 *    Start() is required to begin production.                                                 *
 *                                                                                             *
 * INPUT:   object   -- Reference to the object type class that is to be produced.             *
 *                                                                                             *
 *          house    -- Reference to the owner of the object to be produced.                   *
 *                                                                                             *
 * OUTPUT:  bool; Was production successfully prepared for this factory object. Failure means  *
 *                that the object could not be created. This is catastrophic and in such       *
 *                cases, the factory object should be deleted.                                 *
 *                                                                                             *
 * WARNINGS:   Be sure to examine the return value from this function. Failure to initialize   *
 *             the factory means that the factory is useless and should be deleted.            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FactoryClass::Set(TechnoTypeClass const & object, HouseClass & house)
{
	Validate();
	/*
	**	If there is any production currently in progress, abandon it.
	*/
	Abandon();

	/*
	**	Set up the factory for the new production process.
	*/
	IsDifferent = true;
	IsSuspended = true;
	Set_Rate(0);
	Set_Stage(0);

	/*
	**	Create an object of the type requested.
	*/
	Object = (TechnoClass *)object.Create_One_Of(&house);

	if (Object) {
		House  = Object->House;
		Balance = object.Cost_Of() * house.CostBias;
		Object->PurchasePrice = Balance;
	}

	/*
	**	If all was set up successfully, then return true.
	*/
	return(Object != NULL);
}


/***********************************************************************************************
 * FactoryClass::Set -- Force factory to "produce" special object.                             *
 *                                                                                             *
 *    Use this routine to force the factory into special production mode. Such production is   *
 *    used for the ion cannon and other timed special weapon events.                           *
 *                                                                                             *
 * INPUT:   type     -- The special weapon type to begin "production" of.                      *
 *                                                                                             *
 *          house    -- The owner of this production object.                                   *
 *                                                                                             *
 * OUTPUT:  Was the assignment successful?                                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/22/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FactoryClass::Set(int const & type, HouseClass & house)
{
	Validate();
	/*
	**	If there is any production currently in progress, abandon it.
	*/
	Abandon();

	/*
	**	Set up the factory for the new production process.
	*/
	IsDifferent = true;
	IsSuspended = true;
	Set_Rate(0);
	Set_Stage(0);

	/*
	**	Create an object of the type requested.
	*/
	SpecialItem = type;
	House = &house;
	Balance = 0;

	/*
	**	If all was set up successfully, then return true.
	*/
	return(SpecialItem != SPC_NONE);
}


/***********************************************************************************************
 * FactoryClass::Set -- Fills a factory with an already completed object.                      *
 *                                                                                             *
 *    This routine is called when a produced object is in placement mode but then placement    *
 *    is suspended. The object must then return to the factory as if it were newly completed   *
 *    and awaiting removal.                                                                    *
 *                                                                                             *
 * INPUT:   object   -- The object to return to the factory.                                   *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This will abandon any current object being produced at the factory in order     *
 *             to set the new object into it.                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void FactoryClass::Set(TechnoClass & object)
{
	Validate();
	Abandon();
	Object = &object;
	House  = Object->House;
	Balance = 0;
	Set_Rate(0);
	Set_Stage(STEP_COUNT);
	IsDifferent = true;
	IsSuspended = true;
}


/***********************************************************************************************
 * FactoryClass::Suspend -- Temporarily stop production.                                       *
 *                                                                                             *
 *    This routine will suspend production until a subsiquent call to Start() or Abandon().    *
 *    Typical use of this function is when the player puts production on hold or when there    *
 *    is insufficient funds.                                                                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was production actually stopped? A false return value indicates that the     *
 *                factory was empty or production was already stopped (or never started).      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FactoryClass::Suspend(void)
{
	Validate();
	if (!IsSuspended) {
		IsSuspended = true;
		Set_Rate(0);
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * FactoryClass::Start -- Resumes production after suspension or creation.                     *
 *                                                                                             *
 *    This function will start the production process. It works for newly created factory      *
 *    objects, as well as if production had been suspended previously.                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was production started? A false return value means that the factory is       *
 *                empty or there is unsufficient credits to begin production.                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FactoryClass::Start(void)
{
	Validate();
	if ((Object || SpecialItem) && IsSuspended && !Has_Completed()) {
		if (House->IsHuman || House->Available_Money() >= Cost_Per_Tick()) {
			int time;

			if (Object) {
				time = Object->Class_Of().Time_To_Build(House->Class->House);
			} else {
				time = TICKS_PER_MINUTE * 5;
			}

			time /= STEP_COUNT;
			time = Bound(time, 1, 255);

			Set_Rate(time);
			IsSuspended = false;
			return(true);
		}
	}
	return(false);
}


/***********************************************************************************************
 * FactoryClass::Abandon -- Abandons current construction with money refunded.                 *
 *                                                                                             *
 *    This routine is used when construction is to be abandoned and current money spend is     *
 *    to be refunded. This function effectively clears out this factory of all record of the   *
 *    producing object so that it may either be deleted or started anew with the Set()         *
 *    function.                                                                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was an object actually abandoned? A false return value indicates that the    *
 *                factory was not producing any object.                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FactoryClass::Abandon(void)
{
	Validate();
	if (Object) {

		if (Object) {
			/*
			**	Refund all money expended so far, back to the owner of the object under construction.
			*/
			int money = Object->Class_Of().Cost_Of() * Object->House->CostBias;
			House->Refund_Money(money - Balance);
			Balance = 0;

			/*
			**	Delete the object under construction.
			*/
			ScenarioInit++;
			delete Object;
			Object = NULL;
			ScenarioInit--;
		}
		if (SpecialItem) {
			SpecialItem = SPC_NONE;
		}

		/*
		**	Set the factory back to the idle and empty state.
		*/
		Set_Rate(0);
		Set_Stage(0);
		IsSuspended = true;
		IsDifferent = true;

		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * FactoryClass::Completion -- Fetchs the completion step for this factory.                    *
 *                                                                                             *
 *    Use this routine to determine what animation (or completion step) the factory is         *
 *    currently on.                                                                            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns a completion step number beteen 0 (uncompleted), to STEP_COUNT (completed) *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int FactoryClass::Completion(void)
{
	Validate();
	return(Fetch_Stage());
}


/***********************************************************************************************
 * FactoryClass::Has_Completed -- Checks to see if object has completed production.            *
 *                                                                                             *
 *    Use this routine to examine the factory object in order to determine if the associated   *
 *    object has completed production and is awaiting placement.                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Is the associated object to the factory completed and ready for placement?   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FactoryClass::Has_Completed(void)
{
	Validate();
	if (Object && Fetch_Stage() == STEP_COUNT) {
		return(true);
	}
	if (SpecialItem && Fetch_Stage() == STEP_COUNT) {
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * FactoryClass::Get_Object -- Fetches pointer to object being constructed.                    *
 *                                                                                             *
 *    This routine gets the pointer to the currently constructing object.                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the object undergoing construction.                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
TechnoClass * FactoryClass::Get_Object(void) const
{
	Validate();
	return(Object);
}


/***************************************************************************
 * FactoryClass::Get_Special_Item -- gets factorys spc prod item           *
 *                                                                         *
 * INPUT:      none                                                        *
 *                                                                         *
 * OUTPUT:     int the item the factory is currently working on            *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/05/1995 PWG : Created.                                             *
 *=========================================================================*/
int FactoryClass::Get_Special_Item(void) const
{
	Validate();
	return(SpecialItem);
}


/***********************************************************************************************
 * FactoryClass::Cost_Per_Tick -- Breaks entire production cost into managable chunks.         *
 *                                                                                             *
 *    Use this routine to determine the cost per game "tick" to produce the object.            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the number of credits necessary to advance production one game tick.       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int FactoryClass::Cost_Per_Tick(void)
{
	Validate();
	if (Object) {
		int steps = STEP_COUNT - Fetch_Stage();
		if (steps) {
			return(Balance / steps);
		}
		return(Balance);
	}
	return(0);
}


/***********************************************************************************************
 * FactoryClass::Completed -- Clears factory object after a completed production process.      *
 *                                                                                             *
 *    This routine is called after production completes, and the object produced has been      *
 *    placed into the game. It resets the factory for deletion or starting of new production.  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Did any resetting occur? Failure is the result of the factory not having     *
 *                any completed object. An immediate second call to this routine will also     *
 *                yield false.                                                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FactoryClass::Completed(void)
{
	Validate();
	if (Object && Fetch_Stage() == STEP_COUNT) {
		Object = NULL;
		IsSuspended = true;
		IsDifferent = true;
		Set_Stage(0);
		Set_Rate(0);
		return(true);
	}

	if (SpecialItem && Fetch_Stage() == STEP_COUNT) {
		SpecialItem = SPC_NONE;
		IsSuspended = true;
		IsDifferent = true;
		Set_Stage(0);
		Set_Rate(0);
		return(true);
	}
	return(false);
}
