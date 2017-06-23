#ifndef _FLV_PUSHER_H_
#define _FLV_PUSHER_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "rtmp.h"
void startPushStream(const char* url, const char* flv, const char* log, int useWmp);
void stopPushStream();

#ifdef __cplusplus
}
#endif

#endif //_FLV_PUSHER_H_
