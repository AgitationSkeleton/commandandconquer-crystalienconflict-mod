/*
**	CACHOVER.INC -- how each unit holds station.
**
**	GENERATED FILE -- regenerate with:  python tools/gen_dll_types.py
**
**	Straight from `vibrate`, `altitude` and `lander` in UnitStats.
**
**	The third column is `vibrate`; the fourth is whether the thing is
**	drawn off the ground, which is `altitude > 0` and not `flying`;
**	the last is `speed`, which the ground rumble scales by while moving.
*/

	{RTTI_INFANTRYTYPE,	CAC_SLOT_UAEVIL,	5,	false,	false,	2},	/* rumbles on the ground */
	{RTTI_INFANTRYTYPE,	CAC_SLOT_UAGOOD,	5,	false,	false,	2},	/* rumbles on the ground */
	{RTTI_INFANTRYTYPE,	CAC_SLOT_UBEVIL,	5,	false,	false,	1},	/* rumbles on the ground */
	{RTTI_INFANTRYTYPE,	CAC_SLOT_UBGOOD,	5,	false,	false,	1},	/* rumbles on the ground */
	{RTTI_AIRCRAFTTYPE,	CAC_SLOT_UCEVIL,	20,	true,	false,	5},	/* drifts, held off the ground */
	{RTTI_AIRCRAFTTYPE,	CAC_SLOT_UCGOOD,	20,	true,	false,	5},	/* drifts, held off the ground */
	{RTTI_UNITTYPE,	CAC_SLOT_UDEVIL,	10,	true,	false,	6},	/* drifts, held off the ground */
	{RTTI_UNITTYPE,	CAC_SLOT_UDGOOD,	10,	false,	false,	6},	/* rumbles on the ground */
	{RTTI_UNITTYPE,	CAC_SLOT_UEEVIL,	10,	true,	false,	10},	/* drifts, held off the ground */
	{RTTI_UNITTYPE,	CAC_SLOT_UEGOOD,	10,	false,	false,	10},	/* rumbles on the ground */
	{RTTI_UNITTYPE,	CAC_SLOT_UFEVIL,	20,	true,	false,	5},	/* drifts, held off the ground */
	{RTTI_UNITTYPE,	CAC_SLOT_UFGOOD,	20,	false,	false,	5},	/* rumbles on the ground */
	{RTTI_AIRCRAFTTYPE,	CAC_SLOT_UGEVIL,	10,	true,	true,	10},	/* settles when still */
	{RTTI_AIRCRAFTTYPE,	CAC_SLOT_UGGOOD,	10,	true,	true,	10},	/* settles when still */
	{RTTI_AIRCRAFTTYPE,	CAC_SLOT_UHEVIL,	5,	true,	true,	6},	/* settles when still */
	{RTTI_AIRCRAFTTYPE,	CAC_SLOT_UHGOOD,	5,	true,	true,	6},	/* settles when still */
	{RTTI_INFANTRYTYPE,	CAC_SLOT_UNEVIL,	15,	false,	false,	3},	/* rumbles on the ground */
	{RTTI_INFANTRYTYPE,	CAC_SLOT_UNGOOD,	15,	false,	false,	3},	/* rumbles on the ground */
	{RTTI_AIRCRAFTTYPE,	CAC_SLOT_UPEVIL,	10,	true,	true,	10},	/* settles when still */
	{RTTI_AIRCRAFTTYPE,	CAC_SLOT_UPGOOD,	10,	true,	true,	10},	/* settles when still */
	{RTTI_INFANTRYTYPE,	CAC_SLOT_UQEVIL,	5,	false,	false,	5},	/* rumbles on the ground */
	{RTTI_UNITTYPE,	CAC_SLOT_URGOOD,	20,	false,	false,	6},	/* rumbles on the ground */
