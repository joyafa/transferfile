#pragma once
#include <sys/types.h>
#ifdef __cplusplus
extern "C" {
#endif

unsigned int CRC32(void *pData, size_t iLen);

#ifdef __cplusplus
}
#endif /* end of __cplusplus */

