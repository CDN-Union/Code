//
//  PushModule.h
//  rtmpdump_test
//
//  Created by 李雪岩 on 2017/4/26.
//  Copyright © 2017年 hongduoxing. All rights reserved.
//

#ifndef PushModule_h
#define PushModule_h

#include <stdio.h>
#include "amf.h"
#include "log.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    
    /* 模块定义 */
    typedef struct panda_push_module_s {
        
        const char *module_name;
        
        int (*init)(void*);
        
        int (*release)(void*);
        
        int (*push_message_write)(void* rtmp, void* buf, uint32_t size);
        int (*push_message_packet)(void* rtmp, void* packet);
        
    }panda_push_module_t;
    
    
    int expore_all_module(char *negotiate);
    panda_push_module_t *select_module(AVal *negotiate);
    
#ifdef __cplusplus
}
#endif


#endif /* PushModule_h */
