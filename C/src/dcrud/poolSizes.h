#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#ifdef STATIC_ALLOCATION

#define dcrudArgumentsImpl_POOL_SIZE             250
#define dcrudArgumentsValue_POOL_SIZE            200
#define dcrudClassIDImpl_POOL_SIZE               1000
#define dcrudICRUDImpl_POOL_SIZE                 1000
#define dcrudICacheImpl_POOL_SIZE                10
#define dcrudICallbackImpl_POOL_SIZE             250
#define dcrudIDispatcherImpl_POOL_SIZE           1000
#define dcrudIParticipantImpl_POOL_SIZE          1000
#define dcrudIProvidedImpl_POOL_SIZE             1000
#define dcrudIRegistryImpl_POOL_SIZE             1000
#define dcrudIRequiredImpl_POOL_SIZE             1000
#define dcrudNetworkReceiverImpl_POOL_SIZE       25
#define dcrudNetworkReceiver_POOL_SIZE           250
#define dcrudOperationCall_POOL_SIZE             1000
#define dcrudOperation_POOL_SIZE                 1000
#define dcrudShareableImpl_POOL_SIZE             10000

#endif

#define dcrudGUIDImpl_POOL_SIZE                  1000

#ifdef __cplusplus
}
#endif
