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
#include "rtmp.h"
#ifdef __cplusplus
extern "C" {
#endif
    

    /* 模块定义 */
    typedef struct panda_push_module_s {
        
        const char *module_name;
        /* (PILI_RTMP*, RTMP_Error*) */
        int (*init)(void*, void*);
        
        /* PILI_RTMP* */
        int (*release)(void*);
       
        /* PILI_RTMP*, const char*, int, RTMP_Error* */
            int (*push_message_push)(void *rtmp, void *buf, uint32_t size, void *err);
        
    }panda_push_module_t;
    
    int
    rtmp_packet_to_flv(struct PILI_RTMPPacket *packet, char *flv_tag, int flv_tag_size);

    int expore_all_module(char *negotiate);
    panda_push_module_t *select_module(PILI_AVal *negotiate);
    
#ifdef __cplusplus
}
#endif


#endif /* PushModule_h */
