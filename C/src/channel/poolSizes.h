#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#ifdef STATIC_ALLOCATION

#define channelCoDec_POOL_SIZE                  100U
#define channelListItems_POOL_SIZE              1000U
#define channelUDPChannelImpl_POOL_SIZE         100U

#endif

#define channelUDPChannelImplHandlersMaxCount   100U
#define channelFactoryMaxCount                  100

#ifdef __cplusplus
}
#endif
