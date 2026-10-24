/*
**	CACSTATS.H -- CrystAlien Conflict balance data.
**
**	GENERATED FILE -- do not edit by hand.
**	Regenerate with:  python tools/gen_dll_stats.py
**
**	Every value here was lifted out of CrystAlien Conflict's own
**	UnitStats()/BuildingStats() by tools/extract_stats.py, so the ported
**	units keep the source game's balance exactly.
**
**	The two factions share each type slot and differ only in artwork
**	(see CAC_Faction_Suffix in DLLInterface.cpp), so the '_good' and
**	'_evil' variants -- which carry identical stats in the original --
**	collapse to one entry keyed by the two-letter code.
*/

#ifndef CACSTATS_H
#define CACSTATS_H

/* Weapon range converts from CrystAlien world pixels (96 per tile) to
** C&C leptons (256 per cell), one tile mapping to one cell. */

/* ---------------------------------------------------------------- */
/*  STRUCTURES                                                      */
/* ---------------------------------------------------------------- */

/* BA_evil    2x4  tiles -> 2x2 cells */
#define CAC_BAEVIL_STRENGTH   750   /* 1500 in CrystAlien; BuildingTypeClass doubles it */
#define CAC_BAEVIL_COST       3000
#define CAC_BAEVIL_POWER      1
#define CAC_BAEVIL_DRAIN      0
#define CAC_BAEVIL_SIZE       BSIZE_22
#define CAC_BAEVIL_WIDTH      2
#define CAC_BAEVIL_DEPTH      2
#define CAC_BAEVIL_MAX        1
#define CAC_BAEVIL_THREAT     2

/* BA_good    2x4  tiles -> 2x2 cells */
#define CAC_BAGOOD_STRENGTH   750   /* 1500 in CrystAlien; BuildingTypeClass doubles it */
#define CAC_BAGOOD_COST       3000
#define CAC_BAGOOD_POWER      1
#define CAC_BAGOOD_DRAIN      0
#define CAC_BAGOOD_SIZE       BSIZE_22
#define CAC_BAGOOD_WIDTH      2
#define CAC_BAGOOD_DEPTH      2
#define CAC_BAGOOD_MAX        1
#define CAC_BAGOOD_THREAT     2

/* BB_evil    1x2  tiles -> 1x1 cells */
#define CAC_BBEVIL_STRENGTH   375   /* 750 in CrystAlien; BuildingTypeClass doubles it */
#define CAC_BBEVIL_COST       800
#define CAC_BBEVIL_POWER      200
#define CAC_BBEVIL_DRAIN      0
#define CAC_BBEVIL_SIZE       BSIZE_11
#define CAC_BBEVIL_WIDTH      1
#define CAC_BBEVIL_DEPTH      1
#define CAC_BBEVIL_MAX        8
#define CAC_BBEVIL_THREAT     3

/* BB_good    1x2  tiles -> 1x1 cells */
#define CAC_BBGOOD_STRENGTH   375   /* 750 in CrystAlien; BuildingTypeClass doubles it */
#define CAC_BBGOOD_COST       800
#define CAC_BBGOOD_POWER      200
#define CAC_BBGOOD_DRAIN      0
#define CAC_BBGOOD_SIZE       BSIZE_11
#define CAC_BBGOOD_WIDTH      1
#define CAC_BBGOOD_DEPTH      1
#define CAC_BBGOOD_MAX        8
#define CAC_BBGOOD_THREAT     3

/* BC_evil    2x2  tiles -> 2x1 cells */
#define CAC_BCEVIL_STRENGTH   250   /* 500 in CrystAlien; BuildingTypeClass doubles it */
#define CAC_BCEVIL_COST       500
#define CAC_BCEVIL_POWER      0
#define CAC_BCEVIL_DRAIN      10
#define CAC_BCEVIL_SIZE       BSIZE_21
#define CAC_BCEVIL_WIDTH      2
#define CAC_BCEVIL_DEPTH      1
#define CAC_BCEVIL_MAX        2
#define CAC_BCEVIL_THREAT     3

/* BC_good    2x2  tiles -> 2x1 cells */
#define CAC_BCGOOD_STRENGTH   250   /* 500 in CrystAlien; BuildingTypeClass doubles it */
#define CAC_BCGOOD_COST       500
#define CAC_BCGOOD_POWER      0
#define CAC_BCGOOD_DRAIN      10
#define CAC_BCGOOD_SIZE       BSIZE_21
#define CAC_BCGOOD_WIDTH      2
#define CAC_BCGOOD_DEPTH      1
#define CAC_BCGOOD_MAX        2
#define CAC_BCGOOD_THREAT     3

/* BD_evil    1x2  tiles -> 1x1 cells */
#define CAC_BDEVIL_STRENGTH   300   /* 600 in CrystAlien; BuildingTypeClass doubles it */
#define CAC_BDEVIL_COST       1500
#define CAC_BDEVIL_POWER      0
#define CAC_BDEVIL_DRAIN      75
#define CAC_BDEVIL_SIZE       BSIZE_11
#define CAC_BDEVIL_WIDTH      1
#define CAC_BDEVIL_DEPTH      1
#define CAC_BDEVIL_MAX        12
#define CAC_BDEVIL_THREAT     8
#define CAC_BDEVIL_RANGE      800  /* 300 px */
#define CAC_BDEVIL_ROF        4

/* BD_good    1x2  tiles -> 1x1 cells */
#define CAC_BDGOOD_STRENGTH   300   /* 600 in CrystAlien; BuildingTypeClass doubles it */
#define CAC_BDGOOD_COST       1500
#define CAC_BDGOOD_POWER      0
#define CAC_BDGOOD_DRAIN      75
#define CAC_BDGOOD_SIZE       BSIZE_11
#define CAC_BDGOOD_WIDTH      1
#define CAC_BDGOOD_DEPTH      1
#define CAC_BDGOOD_MAX        12
#define CAC_BDGOOD_THREAT     8
#define CAC_BDGOOD_RANGE      800  /* 300 px */
#define CAC_BDGOOD_ROF        4

/* BE_evil    2x3  tiles -> 2x2 cells */
#define CAC_BEEVIL_STRENGTH   500   /* 1000 in CrystAlien; BuildingTypeClass doubles it */
#define CAC_BEEVIL_COST       2000
#define CAC_BEEVIL_POWER      0
#define CAC_BEEVIL_DRAIN      25
#define CAC_BEEVIL_SIZE       BSIZE_22
#define CAC_BEEVIL_WIDTH      2
#define CAC_BEEVIL_DEPTH      2
#define CAC_BEEVIL_MAX        2
#define CAC_BEEVIL_THREAT     3

/* BE_good    2x3  tiles -> 2x2 cells */
#define CAC_BEGOOD_STRENGTH   500   /* 1000 in CrystAlien; BuildingTypeClass doubles it */
#define CAC_BEGOOD_COST       2000
#define CAC_BEGOOD_POWER      0
#define CAC_BEGOOD_DRAIN      25
#define CAC_BEGOOD_SIZE       BSIZE_22
#define CAC_BEGOOD_WIDTH      2
#define CAC_BEGOOD_DEPTH      2
#define CAC_BEGOOD_MAX        2
#define CAC_BEGOOD_THREAT     3

/* BF_evil    2x3  tiles -> 2x2 cells */
#define CAC_BFEVIL_STRENGTH   300   /* 600 in CrystAlien; BuildingTypeClass doubles it */
#define CAC_BFEVIL_COST       1000
#define CAC_BFEVIL_POWER      0
#define CAC_BFEVIL_DRAIN      50
#define CAC_BFEVIL_SIZE       BSIZE_22
#define CAC_BFEVIL_WIDTH      2
#define CAC_BFEVIL_DEPTH      2
#define CAC_BFEVIL_MAX        1
#define CAC_BFEVIL_THREAT     3

/* BF_good    2x3  tiles -> 2x2 cells */
#define CAC_BFGOOD_STRENGTH   300   /* 600 in CrystAlien; BuildingTypeClass doubles it */
#define CAC_BFGOOD_COST       1000
#define CAC_BFGOOD_POWER      0
#define CAC_BFGOOD_DRAIN      50
#define CAC_BFGOOD_SIZE       BSIZE_22
#define CAC_BFGOOD_WIDTH      2
#define CAC_BFGOOD_DEPTH      2
#define CAC_BFGOOD_MAX        1
#define CAC_BFGOOD_THREAT     3

/* BG_evil    1x2  tiles -> 1x1 cells */
#define CAC_BGEVIL_STRENGTH   250   /* 500 in CrystAlien; BuildingTypeClass doubles it */
#define CAC_BGEVIL_COST       2000
#define CAC_BGEVIL_POWER      0
#define CAC_BGEVIL_DRAIN      100
#define CAC_BGEVIL_SIZE       BSIZE_11
#define CAC_BGEVIL_WIDTH      1
#define CAC_BGEVIL_DEPTH      1
#define CAC_BGEVIL_MAX        1
#define CAC_BGEVIL_THREAT     5

/* BG_good    1x2  tiles -> 1x1 cells */
#define CAC_BGGOOD_STRENGTH   250   /* 500 in CrystAlien; BuildingTypeClass doubles it */
#define CAC_BGGOOD_COST       2000
#define CAC_BGGOOD_POWER      0
#define CAC_BGGOOD_DRAIN      100
#define CAC_BGGOOD_SIZE       BSIZE_11
#define CAC_BGGOOD_WIDTH      1
#define CAC_BGGOOD_DEPTH      1
#define CAC_BGGOOD_MAX        1
#define CAC_BGGOOD_THREAT     5

/* BH_evil    1x2  tiles -> 1x1 cells */
#define CAC_BHEVIL_STRENGTH   500   /* 1000 in CrystAlien; BuildingTypeClass doubles it */
#define CAC_BHEVIL_COST       1500
#define CAC_BHEVIL_POWER      0
#define CAC_BHEVIL_DRAIN      100
#define CAC_BHEVIL_SIZE       BSIZE_11
#define CAC_BHEVIL_WIDTH      1
#define CAC_BHEVIL_DEPTH      1
#define CAC_BHEVIL_MAX        5
#define CAC_BHEVIL_THREAT     7

/* BH_good    1x2  tiles -> 1x1 cells */
#define CAC_BHGOOD_STRENGTH   500   /* 1000 in CrystAlien; BuildingTypeClass doubles it */
#define CAC_BHGOOD_COST       1500
#define CAC_BHGOOD_POWER      0
#define CAC_BHGOOD_DRAIN      100
#define CAC_BHGOOD_SIZE       BSIZE_11
#define CAC_BHGOOD_WIDTH      1
#define CAC_BHGOOD_DEPTH      1
#define CAC_BHGOOD_MAX        5
#define CAC_BHGOOD_THREAT     7

/* BI_evil    4x3  tiles -> 4x2 cells */
#define CAC_BIEVIL_STRENGTH   500   /* 1000 in CrystAlien; BuildingTypeClass doubles it */
#define CAC_BIEVIL_COST       19660
#define CAC_BIEVIL_POWER      0
#define CAC_BIEVIL_DRAIN      0
#define CAC_BIEVIL_SIZE       BSIZE_42
#define CAC_BIEVIL_WIDTH      4
#define CAC_BIEVIL_DEPTH      2
#define CAC_BIEVIL_MAX        1
#define CAC_BIEVIL_THREAT     0

/* BI_good    4x3  tiles -> 4x2 cells */
#define CAC_BIGOOD_STRENGTH   500   /* 1000 in CrystAlien; BuildingTypeClass doubles it */
#define CAC_BIGOOD_COST       19660
#define CAC_BIGOOD_POWER      0
#define CAC_BIGOOD_DRAIN      0
#define CAC_BIGOOD_SIZE       BSIZE_42
#define CAC_BIGOOD_WIDTH      4
#define CAC_BIGOOD_DEPTH      2
#define CAC_BIGOOD_MAX        1
#define CAC_BIGOOD_THREAT     0

/* BJ_evil    1x2  tiles -> 1x1 cells */
#define CAC_BJEVIL_STRENGTH   500   /* 1000 in CrystAlien; BuildingTypeClass doubles it */
#define CAC_BJEVIL_COST       1500
#define CAC_BJEVIL_POWER      100
#define CAC_BJEVIL_DRAIN      0
#define CAC_BJEVIL_SIZE       BSIZE_11
#define CAC_BJEVIL_WIDTH      1
#define CAC_BJEVIL_DEPTH      1
#define CAC_BJEVIL_MAX        1
#define CAC_BJEVIL_THREAT     3

/* BK_evil    2x4  tiles -> 2x2 cells */
#define CAC_BKEVIL_STRENGTH   750   /* 1500 in CrystAlien; BuildingTypeClass doubles it */
#define CAC_BKEVIL_COST       3000
#define CAC_BKEVIL_POWER      1
#define CAC_BKEVIL_DRAIN      0
#define CAC_BKEVIL_SIZE       BSIZE_22
#define CAC_BKEVIL_WIDTH      2
#define CAC_BKEVIL_DEPTH      2
#define CAC_BKEVIL_MAX        1
#define CAC_BKEVIL_THREAT     2
#define CAC_BKEVIL_RANGE      1333  /* 500 px */
#define CAC_BKEVIL_ROF        10

/* BK_good    2x4  tiles -> 2x2 cells */
#define CAC_BKGOOD_STRENGTH   750   /* 1500 in CrystAlien; BuildingTypeClass doubles it */
#define CAC_BKGOOD_COST       3000
#define CAC_BKGOOD_POWER      1
#define CAC_BKGOOD_DRAIN      0
#define CAC_BKGOOD_SIZE       BSIZE_22
#define CAC_BKGOOD_WIDTH      2
#define CAC_BKGOOD_DEPTH      2
#define CAC_BKGOOD_MAX        1
#define CAC_BKGOOD_THREAT     2
#define CAC_BKGOOD_RANGE      1333  /* 500 px */
#define CAC_BKGOOD_ROF        10

/* BL_good    2x3  tiles -> 2x2 cells */
#define CAC_BLGOOD_STRENGTH   1500   /* 3000 in CrystAlien; BuildingTypeClass doubles it */
#define CAC_BLGOOD_COST       3000
#define CAC_BLGOOD_POWER      0
#define CAC_BLGOOD_DRAIN      0
#define CAC_BLGOOD_SIZE       BSIZE_22
#define CAC_BLGOOD_WIDTH      2
#define CAC_BLGOOD_DEPTH      2
#define CAC_BLGOOD_MAX        4
#define CAC_BLGOOD_THREAT     1
#define CAC_BLGOOD_REVENUE    100
#define CAC_BLGOOD_REVENUE_TICKS 196  /* 300 CrystAlien frames */

/* BX_good    2x4  tiles -> 2x2 cells */
#define CAC_BXGOOD_STRENGTH   750   /* 1500 in CrystAlien; BuildingTypeClass doubles it */
#define CAC_BXGOOD_COST       3000
#define CAC_BXGOOD_POWER      0
#define CAC_BXGOOD_DRAIN      0
#define CAC_BXGOOD_SIZE       BSIZE_22
#define CAC_BXGOOD_WIDTH      2
#define CAC_BXGOOD_DEPTH      2
#define CAC_BXGOOD_MAX        1
#define CAC_BXGOOD_THREAT     2

/* ---------------------------------------------------------------- */
/*  UNITS                                                           */
/* ---------------------------------------------------------------- */

/* UA_evil  */
#define CAC_UAEVIL_STRENGTH   125
#define CAC_UAEVIL_COST       200
#define CAC_UAEVIL_SPEED      2
#define CAC_UAEVIL_MAX        5
#define CAC_UAEVIL_SIZE       30
#define CAC_UAEVIL_THREAT     3
#define CAC_UAEVIL_RANGE      533  /* 200 px */
#define CAC_UAEVIL_ROF        20

/* UA_good  */
#define CAC_UAGOOD_STRENGTH   125
#define CAC_UAGOOD_COST       200
#define CAC_UAGOOD_SPEED      2
#define CAC_UAGOOD_MAX        5
#define CAC_UAGOOD_SIZE       30
#define CAC_UAGOOD_THREAT     3
#define CAC_UAGOOD_RANGE      533  /* 200 px */
#define CAC_UAGOOD_ROF        20

/* UB_evil  */
#define CAC_UBEVIL_STRENGTH   75
#define CAC_UBEVIL_COST       500
#define CAC_UBEVIL_SPEED      1
#define CAC_UBEVIL_MAX        5
#define CAC_UBEVIL_SIZE       30
#define CAC_UBEVIL_THREAT     5

/* UB_good  */
#define CAC_UBGOOD_STRENGTH   75
#define CAC_UBGOOD_COST       500
#define CAC_UBGOOD_SPEED      1
#define CAC_UBGOOD_MAX        5
#define CAC_UBGOOD_SIZE       30
#define CAC_UBGOOD_THREAT     5

/* UC_evil  */
#define CAC_UCEVIL_STRENGTH   125
#define CAC_UCEVIL_COST       600
#define CAC_UCEVIL_SPEED      5
#define CAC_UCEVIL_MAX        5
#define CAC_UCEVIL_SIZE       25
#define CAC_UCEVIL_THREAT     4
#define CAC_UCEVIL_RANGE      800  /* 300 px */
#define CAC_UCEVIL_ROF        20

/* UC_good  */
#define CAC_UCGOOD_STRENGTH   125
#define CAC_UCGOOD_COST       600
#define CAC_UCGOOD_SPEED      5
#define CAC_UCGOOD_MAX        5
#define CAC_UCGOOD_SIZE       25
#define CAC_UCGOOD_THREAT     4
#define CAC_UCGOOD_RANGE      800  /* 300 px */
#define CAC_UCGOOD_ROF        20

/* UD_evil  */
#define CAC_UDEVIL_STRENGTH   1500
#define CAC_UDEVIL_COST       1400
#define CAC_UDEVIL_SPEED      6
#define CAC_UDEVIL_MAX        4
#define CAC_UDEVIL_SIZE       50
#define CAC_UDEVIL_THREAT     2
#define CAC_UDEVIL_CARGO      200

/* UD_good  */
#define CAC_UDGOOD_STRENGTH   1500
#define CAC_UDGOOD_COST       1400
#define CAC_UDGOOD_SPEED      6
#define CAC_UDGOOD_MAX        4
#define CAC_UDGOOD_SIZE       50
#define CAC_UDGOOD_THREAT     2
#define CAC_UDGOOD_CARGO      200

/* UE_evil  */
#define CAC_UEEVIL_STRENGTH   200
#define CAC_UEEVIL_COST       600
#define CAC_UEEVIL_SPEED      10
#define CAC_UEEVIL_MAX        5
#define CAC_UEEVIL_SIZE       50
#define CAC_UEEVIL_THREAT     4
#define CAC_UEEVIL_RANGE      667  /* 250 px */
#define CAC_UEEVIL_ROF        10

/* UE_good  */
#define CAC_UEGOOD_STRENGTH   200
#define CAC_UEGOOD_COST       600
#define CAC_UEGOOD_SPEED      10
#define CAC_UEGOOD_MAX        5
#define CAC_UEGOOD_SIZE       50
#define CAC_UEGOOD_THREAT     4
#define CAC_UEGOOD_RANGE      667  /* 250 px */
#define CAC_UEGOOD_ROF        10

/* UF_evil  */
#define CAC_UFEVIL_STRENGTH   800
#define CAC_UFEVIL_COST       1200
#define CAC_UFEVIL_SPEED      5
#define CAC_UFEVIL_MAX        5
#define CAC_UFEVIL_SIZE       70
#define CAC_UFEVIL_THREAT     7
#define CAC_UFEVIL_RANGE      800  /* 300 px */
#define CAC_UFEVIL_ROF        5

/* UF_good  */
#define CAC_UFGOOD_STRENGTH   800
#define CAC_UFGOOD_COST       1200
#define CAC_UFGOOD_SPEED      5
#define CAC_UFGOOD_MAX        5
#define CAC_UFGOOD_SIZE       70
#define CAC_UFGOOD_THREAT     7
#define CAC_UFGOOD_RANGE      800  /* 300 px */
#define CAC_UFGOOD_ROF        5

/* UG_evil  */
#define CAC_UGEVIL_STRENGTH   150
#define CAC_UGEVIL_COST       1200
#define CAC_UGEVIL_SPEED      10
#define CAC_UGEVIL_MAX        4
#define CAC_UGEVIL_SIZE       35
#define CAC_UGEVIL_THREAT     8
#define CAC_UGEVIL_RANGE      800  /* 300 px */
#define CAC_UGEVIL_ROF        2
#define CAC_UGEVIL_AMMO       10

/* UG_good  */
#define CAC_UGGOOD_STRENGTH   150
#define CAC_UGGOOD_COST       1200
#define CAC_UGGOOD_SPEED      10
#define CAC_UGGOOD_MAX        4
#define CAC_UGGOOD_SIZE       35
#define CAC_UGGOOD_THREAT     8
#define CAC_UGGOOD_RANGE      800  /* 300 px */
#define CAC_UGGOOD_ROF        2
#define CAC_UGGOOD_AMMO       10

/* UH_evil  */
#define CAC_UHEVIL_STRENGTH   300
#define CAC_UHEVIL_COST       1000
#define CAC_UHEVIL_SPEED      6
#define CAC_UHEVIL_MAX        2
#define CAC_UHEVIL_SIZE       60
#define CAC_UHEVIL_THREAT     4
#define CAC_UHEVIL_CAPACITY   6

/* UH_good  */
#define CAC_UHGOOD_STRENGTH   300
#define CAC_UHGOOD_COST       1000
#define CAC_UHGOOD_SPEED      6
#define CAC_UHGOOD_MAX        2
#define CAC_UHGOOD_SIZE       60
#define CAC_UHGOOD_THREAT     4
#define CAC_UHGOOD_CAPACITY   6

/* UI_evil  */
#define CAC_UIEVIL_STRENGTH   50
#define CAC_UIEVIL_COST       200
#define CAC_UIEVIL_SPEED      0
#define CAC_UIEVIL_MAX        1
#define CAC_UIEVIL_SIZE       35
#define CAC_UIEVIL_THREAT     0

/* UI_good  */
#define CAC_UIGOOD_STRENGTH   50
#define CAC_UIGOOD_COST       200
#define CAC_UIGOOD_SPEED      0
#define CAC_UIGOOD_MAX        1
#define CAC_UIGOOD_SIZE       35
#define CAC_UIGOOD_THREAT     0

/* UJ_good  */
#define CAC_UJGOOD_STRENGTH   50
#define CAC_UJGOOD_COST       50000
#define CAC_UJGOOD_SPEED      0
#define CAC_UJGOOD_MAX        1
#define CAC_UJGOOD_SIZE       35
#define CAC_UJGOOD_THREAT     0

/* UK_evil  */
#define CAC_UKEVIL_STRENGTH   50
#define CAC_UKEVIL_COST       3000
#define CAC_UKEVIL_SPEED      1
#define CAC_UKEVIL_MAX        1
#define CAC_UKEVIL_SIZE       10
#define CAC_UKEVIL_THREAT     0

/* UK_good  */
#define CAC_UKGOOD_STRENGTH   50
#define CAC_UKGOOD_COST       3000
#define CAC_UKGOOD_SPEED      1
#define CAC_UKGOOD_MAX        1
#define CAC_UKGOOD_SIZE       10
#define CAC_UKGOOD_THREAT     0

/* UM_evil  */
#define CAC_UMEVIL_STRENGTH   550
#define CAC_UMEVIL_COST       2000
#define CAC_UMEVIL_SPEED      6
#define CAC_UMEVIL_MAX        1
#define CAC_UMEVIL_SIZE       30
#define CAC_UMEVIL_THREAT     10
#define CAC_UMEVIL_RANGE      733  /* 275 px */
#define CAC_UMEVIL_ROF        3

/* UN_evil  */
#define CAC_UNEVIL_STRENGTH   75
#define CAC_UNEVIL_COST       500
#define CAC_UNEVIL_SPEED      3
#define CAC_UNEVIL_MAX        8
#define CAC_UNEVIL_SIZE       30
#define CAC_UNEVIL_THREAT     5

/* UO_evil  */
#define CAC_UOEVIL_STRENGTH   50
#define CAC_UOEVIL_COST       200
#define CAC_UOEVIL_SPEED      0
#define CAC_UOEVIL_MAX        1
#define CAC_UOEVIL_SIZE       35
#define CAC_UOEVIL_THREAT     0

/* UP_evil  */
#define CAC_UPEVIL_STRENGTH   400
#define CAC_UPEVIL_COST       1500
#define CAC_UPEVIL_SPEED      10
#define CAC_UPEVIL_MAX        2
#define CAC_UPEVIL_SIZE       60
#define CAC_UPEVIL_THREAT     4
#define CAC_UPEVIL_RANGE      800  /* 300 px */
#define CAC_UPEVIL_ROF        10

/* UP_good  */
#define CAC_UPGOOD_STRENGTH   400
#define CAC_UPGOOD_COST       1500
#define CAC_UPGOOD_SPEED      10
#define CAC_UPGOOD_MAX        2
#define CAC_UPGOOD_SIZE       60
#define CAC_UPGOOD_THREAT     4
#define CAC_UPGOOD_RANGE      800  /* 300 px */
#define CAC_UPGOOD_ROF        10

/* UQ_evil  */
#define CAC_UQEVIL_STRENGTH   500
#define CAC_UQEVIL_COST       2000
#define CAC_UQEVIL_SPEED      5
#define CAC_UQEVIL_MAX        5
#define CAC_UQEVIL_SIZE       30
#define CAC_UQEVIL_THREAT     10
#define CAC_UQEVIL_RANGE      933  /* 350 px */
#define CAC_UQEVIL_ROF        3

/* UR_good  */
#define CAC_URGOOD_STRENGTH   500
#define CAC_URGOOD_COST       900
#define CAC_URGOOD_SPEED      6
#define CAC_URGOOD_MAX        2
#define CAC_URGOOD_SIZE       70
#define CAC_URGOOD_THREAT     7
#define CAC_URGOOD_RANGE      800  /* 300 px */
#define CAC_URGOOD_ROF        7
#define CAC_URGOOD_CAPACITY   1


/* ---------------------------------------------------------------- */
/*  One-sided content, under the name the other side asks for       */
/* ---------------------------------------------------------------- */

#define CAC_BJGOOD_COST              CAC_BJEVIL_COST
#define CAC_BJGOOD_DEPTH             CAC_BJEVIL_DEPTH
#define CAC_BJGOOD_DRAIN             CAC_BJEVIL_DRAIN
#define CAC_BJGOOD_MAX               CAC_BJEVIL_MAX
#define CAC_BJGOOD_POWER             CAC_BJEVIL_POWER
#define CAC_BJGOOD_SIZE              CAC_BJEVIL_SIZE
#define CAC_BJGOOD_STRENGTH          CAC_BJEVIL_STRENGTH
#define CAC_BJGOOD_THREAT            CAC_BJEVIL_THREAT
#define CAC_BJGOOD_WIDTH             CAC_BJEVIL_WIDTH
#define CAC_BLEVIL_COST              CAC_BLGOOD_COST
#define CAC_BLEVIL_DEPTH             CAC_BLGOOD_DEPTH
#define CAC_BLEVIL_DRAIN             CAC_BLGOOD_DRAIN
#define CAC_BLEVIL_MAX               CAC_BLGOOD_MAX
#define CAC_BLEVIL_POWER             CAC_BLGOOD_POWER
#define CAC_BLEVIL_REVENUE           CAC_BLGOOD_REVENUE
#define CAC_BLEVIL_REVENUE_TICKS     CAC_BLGOOD_REVENUE_TICKS
#define CAC_BLEVIL_SIZE              CAC_BLGOOD_SIZE
#define CAC_BLEVIL_STRENGTH          CAC_BLGOOD_STRENGTH
#define CAC_BLEVIL_THREAT            CAC_BLGOOD_THREAT
#define CAC_BLEVIL_WIDTH             CAC_BLGOOD_WIDTH
#define CAC_BXEVIL_COST              CAC_BXGOOD_COST
#define CAC_BXEVIL_DEPTH             CAC_BXGOOD_DEPTH
#define CAC_BXEVIL_DRAIN             CAC_BXGOOD_DRAIN
#define CAC_BXEVIL_MAX               CAC_BXGOOD_MAX
#define CAC_BXEVIL_POWER             CAC_BXGOOD_POWER
#define CAC_BXEVIL_SIZE              CAC_BXGOOD_SIZE
#define CAC_BXEVIL_STRENGTH          CAC_BXGOOD_STRENGTH
#define CAC_BXEVIL_THREAT            CAC_BXGOOD_THREAT
#define CAC_BXEVIL_WIDTH             CAC_BXGOOD_WIDTH
#define CAC_UJEVIL_COST              CAC_UJGOOD_COST
#define CAC_UJEVIL_MAX               CAC_UJGOOD_MAX
#define CAC_UJEVIL_SIZE              CAC_UJGOOD_SIZE
#define CAC_UJEVIL_SPEED             CAC_UJGOOD_SPEED
#define CAC_UJEVIL_STRENGTH          CAC_UJGOOD_STRENGTH
#define CAC_UJEVIL_THREAT            CAC_UJGOOD_THREAT
#define CAC_UMGOOD_COST              CAC_UMEVIL_COST
#define CAC_UMGOOD_MAX               CAC_UMEVIL_MAX
#define CAC_UMGOOD_RANGE             CAC_UMEVIL_RANGE
#define CAC_UMGOOD_ROF               CAC_UMEVIL_ROF
#define CAC_UMGOOD_SIZE              CAC_UMEVIL_SIZE
#define CAC_UMGOOD_SPEED             CAC_UMEVIL_SPEED
#define CAC_UMGOOD_STRENGTH          CAC_UMEVIL_STRENGTH
#define CAC_UMGOOD_THREAT            CAC_UMEVIL_THREAT
#define CAC_UNGOOD_COST              CAC_UNEVIL_COST
#define CAC_UNGOOD_MAX               CAC_UNEVIL_MAX
#define CAC_UNGOOD_SIZE              CAC_UNEVIL_SIZE
#define CAC_UNGOOD_SPEED             CAC_UNEVIL_SPEED
#define CAC_UNGOOD_STRENGTH          CAC_UNEVIL_STRENGTH
#define CAC_UNGOOD_THREAT            CAC_UNEVIL_THREAT
#define CAC_UOGOOD_COST              CAC_UOEVIL_COST
#define CAC_UOGOOD_MAX               CAC_UOEVIL_MAX
#define CAC_UOGOOD_SIZE              CAC_UOEVIL_SIZE
#define CAC_UOGOOD_SPEED             CAC_UOEVIL_SPEED
#define CAC_UOGOOD_STRENGTH          CAC_UOEVIL_STRENGTH
#define CAC_UOGOOD_THREAT            CAC_UOEVIL_THREAT
#define CAC_UQGOOD_COST              CAC_UQEVIL_COST
#define CAC_UQGOOD_MAX               CAC_UQEVIL_MAX
#define CAC_UQGOOD_RANGE             CAC_UQEVIL_RANGE
#define CAC_UQGOOD_ROF               CAC_UQEVIL_ROF
#define CAC_UQGOOD_SIZE              CAC_UQEVIL_SIZE
#define CAC_UQGOOD_SPEED             CAC_UQEVIL_SPEED
#define CAC_UQGOOD_STRENGTH          CAC_UQEVIL_STRENGTH
#define CAC_UQGOOD_THREAT            CAC_UQEVIL_THREAT
#define CAC_UREVIL_CAPACITY          CAC_URGOOD_CAPACITY
#define CAC_UREVIL_COST              CAC_URGOOD_COST
#define CAC_UREVIL_MAX               CAC_URGOOD_MAX
#define CAC_UREVIL_RANGE             CAC_URGOOD_RANGE
#define CAC_UREVIL_ROF               CAC_URGOOD_ROF
#define CAC_UREVIL_SIZE              CAC_URGOOD_SIZE
#define CAC_UREVIL_SPEED             CAC_URGOOD_SPEED
#define CAC_UREVIL_STRENGTH          CAC_URGOOD_STRENGTH
#define CAC_UREVIL_THREAT            CAC_URGOOD_THREAT

#endif
