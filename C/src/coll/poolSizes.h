#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#ifdef STATIC_ALLOCATION

/*-- List -----------------------------------------------*/

#define collListImpl_POOL_SIZE         512
#define collList_ITEM_MAX_COUNT        100

/*-- Set ------------------------------------------------*/

#define collSetImpl_POOL_SIZE          512
#define collSet_ITEM_MAX_COUNT         100

/*-- Map ------------------------------------------------*/

#define collMapImpl_POOL_SIZE          512
#define collMap_ITEM_MAX_COUNT         100

#endif

#ifdef __cplusplus
}
#endif
