//
//  PushModule.h
//  rtmpdump_test
//
//  Created by 杜卫坤 on 2017/6/20.
//  Copyright © 2017年 wangsu. All rights reserved.
//

#ifndef WSPushModule_h
#define WSPushModule_h

#include <stdio.h>
#include "amf.h"
#include "log.h"
#include "rtmp.h"
#ifdef __cplusplus
extern "C" {
#endif
    

    int ws_wmp_module_init(void *arg, void *err);
    int ws_wmp_module_release(void *arg);
    int ws_wmp_module_push(void* rtmp, void* buf, uint32_t size, void* err);
    
   
    
#ifdef __cplusplus
}
#endif


#endif /* PushModule_h */
