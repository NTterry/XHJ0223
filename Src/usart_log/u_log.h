/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef _UTILS_PARAM_CHECK_H_
#define _UTILS_PARAM_CHECK_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "string.h"

//#define IOT_DEBUG

#define _IN_            /* indicate an input parameter */
#define _OU_            /* indicate a output parameter */

typedef enum {
    eLOG_DISABLE = 0,
    eLOG_ERROR = 1,
    eLOG_WARN = 2,
    eLOG_INFO = 3,
    eLOG_DEBUG = 4
} LOG_LEVEL;

void IOT_Log_Gen(const char *file, const char *func, const int line, const char *fmt, ...);

/* Simple APIs for log generation in different level */
/* Macro for debug mode */
#ifdef IOT_DEBUG
	#define IOT_FUNC_ENTRY    \
		{\
		printf("FUNC_ENTRY:   %s L#%d \r\n", __FUNCTION__, __LINE__);  \
		}
	#define IOT_FUNC_EXIT    \
		{\
		printf("FUNC_EXIT:   %s L#%d \r\n", __FUNCTION__, __LINE__);  \
		return;\
		}
	#define IOT_FUNC_EXIT_RC(x)    \
		{\
		printf("FUNC_EXIT:   %s L#%d Return Code : %ld \r\n", __FUNCTION__, __LINE__, (long)(x));  \
		return x; \
		}

	#define Log_e(fmt, ...) IOT_Log_Gen(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
	
	#define LPrint(format,...) printf (format,##__VA_ARGS__)
#else
	#define IOT_FUNC_ENTRY
	#define IOT_FUNC_EXIT 			\
		{\
			return;\
		}
	#define IOT_FUNC_EXIT_RC(x)     \
		{\
			return x; \
		}
	#define Log_e(fmt, ...)
	#define LPrint(format,...)
#endif




#define NUMBERIC_SANITY_CHECK(num, err) \
    do { \
        if (0 == (num)) { \
            Log_e("Invalid argument, numeric 0"); \
            return (err); \
        } \
    } while(0)

#define NUMBERIC_SANITY_CHECK_RTN(num) \
    do { \
        if (0 == (num)) { \
            Log_e("Invalid argument, numeric 0"); \
            return; \
        } \
    } while(0)

#define POINTER_SANITY_CHECK(ptr, err) \
    do { \
        if (NULL == (ptr)) { \
            Log_e("Invalid argument, %s = %p", #ptr, ptr); \
            return (err); \
        } \
    } while(0)

#define POINTER_SANITY_CHECK_RTN(ptr) \
    do { \
        if (NULL == (ptr)) { \
            Log_e("Invalid argument, %s = %p", #ptr, ptr); \
            return; \
        } \
    } while(0)

#define STRING_PTR_SANITY_CHECK(ptr, err) \
    do { \
        if (NULL == (ptr)) { \
            Log_e("Invalid argument, %s = %p", #ptr, (ptr)); \
            return (err); \
        } \
        if (0 == strlen((ptr))) { \
            Log_e("Invalid argument, %s = '%s'", #ptr, (ptr)); \
            return (err); \
        } \
    } while(0)

#define STRING_PTR_SANITY_CHECK_RTN(ptr) \
    do { \
        if (NULL == (ptr)) { \
            Log_e("Invalid argument, %s = %p", #ptr, (ptr)); \
            return; \
        } \
        if (0 == strlen((ptr))) { \
            Log_e("Invalid argument, %s = '%s'", #ptr, (ptr)); \
            return; \
        } \
    } while(0)

#if defined(__cplusplus)
}
#endif

#endif /* _UTILS_PARAM_CHECK_H_ */
