#pragma once

#ifdef STATIC_ALLOCATION

/*-- List -----------------------------------------------*/

#define COLL_LIST_MAX_COUNT            12
#define COLL_LIST_ITEM_MAX_COUNT       100

extern unsigned int collLimitsListCountMax;
extern unsigned int collLimitsListItemCountMax;

/*-- Set ------------------------------------------------*/

#define COLL_SET_MAX_COUNT             12
#define COLL_SET_ITEM_MAX_COUNT        100

extern unsigned int collLimitsSetCountMax;
extern unsigned int collLimitsSetItemCountMax;

/*-- Map ------------------------------------------------*/

#define COLL_MAP_MAX_COUNT             12
#define COLL_MAP_ITEM_MAX_COUNT        100

extern unsigned int collLimitsMapCountMax;
extern unsigned int collLimitsMapItemCountMax;

#endif
