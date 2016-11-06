#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#ifdef STATIC_ALLOCATION

/*-- List -----------------------------------------------*/

#define collList_MAX_COUNT            12
#define collList_ITEM_MAX_COUNT       100

extern unsigned int collLimitsListCountMax;
extern unsigned int collLimitsListItemCountMax;

/*-- Set ------------------------------------------------*/

#define collSet_MAX_COUNT             12
#define collSet_ITEM_MAX_COUNT        100

extern unsigned int collLimitsSetCountMax;
extern unsigned int collLimitsSetItemCountMax;

/*-- Map ------------------------------------------------*/

#define collMap_MAX_COUNT             12
#define collMap_ITEM_MAX_COUNT        100

extern unsigned int collLimitsMapCountMax;
extern unsigned int collLimitsMapItemCountMax;

#endif

#ifdef __cplusplus
}
#endif
