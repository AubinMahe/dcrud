#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#ifdef STATIC_ALLOCATION

#define utilCmdLineImpl_POOL_SIZE             10U
#define utilString_20_POOL_SIZE             5000U
#define utilString_40_POOL_SIZE            10000U
#define utilString_80_POOL_SIZE            10000U
#define utilString_160_POOL_SIZE            5000U
#define utilString_1000_POOL_SIZE           5000U
#define utilString_5000_POOL_SIZE           2000U
#define utilString_10000_POOL_SIZE          1000U
#define utilString_65536_POOL_SIZE           500U

#endif
#ifdef __cplusplus
}
#endif
