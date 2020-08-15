#ifndef defines_h
#define defines_h
#include <CAN-Registers.h>

#define Speedintegrationmax 10 //~1s After how many Receives we should integrate SPEED (make it more granular, avoid too many Values)

#define FILTERLOG

#ifndef INT8U
#define INT8U byte
#endif

#ifndef INT16U
#define INT16U word
#endif

#ifndef INT32U
#define INT32U unsigned long
#endif

#ifndef INT64U
#define INT64U int64_t 
#endif


#endif