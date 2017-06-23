#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include "rtmp.h"
#include "FlvParser.h"
#include "rtmp_sys.h"
#include "log.h"

const int WIDTH = 368;
const int HEIGHT = 640;
const int VIDEO_BITRATE = 1024; //1M
const int VIDEO_FRAMERATE = 25;
const int AUDIO_SAMPLERATE = 22050;
const int AUDIO_CHANNEL = 1;
const int AUDIO_BITRATE = 90;
//const char* URL = "rtmp://183.134.38.17:39900/zsvspush.8686c.com/live/livetest_zs";
const char* URL = "rtmp://61.147.211.184:39900/live/stream";


static bool s_inited = false;
static volatile bool s_exit = false;
static int64_t s_begin = 0;

void rtmp_write_log(char* buf, int len)
{
    LOGI("%s", buf);
}

int64_t get_time()
{
    struct timeval tv = {};
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void sleep_by_ts(int ts)
{
    int64_t now = get_time();
    while (now - s_begin < (int64_t)ts) {
        now = get_time();
        usleep(1);
    }
}

void onTag(CFlvParser::Tag* tag)
{
    }
#define HTON16(x)  ((x>>8&0xff)|(x<<8&0xff00))
#define HTON24(x)  ((x>>16&0xff)|(x<<16&0xff0000)|(x&0xff00))
#define HTON32(x)  ((x>>24&0xff)|(x>>8&0xff00)|\
(x<<8&0xff0000)|(x<<24&0xff000000))
#define HTONTIME(x) ((x>>16&0xff)|(x<<16&0xff0000)|(x&0xff00)|(x&0xff000000))

/*read 1 byte*/
int ReadU8(uint32_t *u8,FILE*fp){
    if(fread(u8,1,1,fp)!=1)
        return 0;
    return 1;
}
/*read 2 byte*/
int ReadU16(uint32_t *u16,FILE*fp){
    if(fread(u16,2,1,fp)!=1)
        return 0;
    *u16=HTON16(*u16);
    return 1;
}
/*read 3 byte*/
int ReadU24(uint32_t *u24,FILE*fp){
    if(fread(u24,3,1,fp)!=1)
        return 0;
    *u24=HTON24(*u24);
    return 1;
}
/*read 4 byte*/
int ReadU32(uint32_t *u32,FILE*fp){
    if(fread(u32,4,1,fp)!=1)
        return 0;
    *u32=HTON32(*u32);
    return 1;
}
/*read 1 byte,and loopback 1 byte at once*/
int PeekU8(uint32_t *u8,FILE*fp){
    if(fread(u8,1,1,fp)!=1)
        return 0;
    fseek(fp,-1,SEEK_CUR);
    return 1;
}
/*read 4 byte and convert to time format*/
int ReadTime(uint32_t *utime,FILE*fp){
    if(fread(utime,4,1,fp)!=1)
        return 0;
    *utime=HTONTIME(*utime);
    return 1;
}

int InitSockets()
{
#ifdef WIN32
    WORD version;
    WSADATA wsaData;
    version=MAKEWORD(2,2);
    return (WSAStartup(version, &wsaData) == 0);
#endif
    return TRUE;
}

void CleanupSockets()
{
#ifdef WIN32
    WSACleanup();
#endif
}
int publish_using_write( const char* filename, char* url);
int publish_using_write( const char* filename, char* url){
    uint32_t start_time=0;
    uint32_t now_time=0;
    uint32_t pre_frame_time=0;
    uint32_t lasttime=0;
    int bNextIsKey=0;
    char* pFileBuf=NULL;
    
    //read from tag header
    uint32_t type=0;
    uint32_t datalength=0;
    uint32_t timestamp=0;
    
    PILI_RTMP *rtmp=NULL;
    RTMPError *err;
    FILE*fp=NULL;
    fp=fopen(filename,"rb");
    if (!fp){
        PILI_RTMP_LogPrintf("Open File Error. %s \n",filename);
        CleanupSockets();
        return -1;
    }
    
    /* set log level */
    //PILI_RTMP_LogLevel loglvl=PILI_RTMP_LOGDEBUG;
    //PILI_RTMP_LogSetLevel(loglvl);
    
    if (!InitSockets()){
        PILI_RTMP_LogPrintf("Init Socket Err\n");
        return -1;
    }
    
    rtmp=PILI_RTMP_Alloc();
    PILI_RTMP_Init(rtmp);
    //set connection timeout,default 30s
    rtmp->Link.timeout=5;
    if(!PILI_RTMP_SetupURL(rtmp,url,err))
    {
        PILI_RTMP_Log(PILI_RTMP_LOGERROR,"SetupURL Err\n");
        PILI_RTMP_Free(rtmp);
        CleanupSockets();
        return -1;
    }
    
    PILI_RTMP_EnableWrite(rtmp);
    //1hour
    PILI_RTMP_SetBufferMS(rtmp, 3600*1000);
    if (!PILI_RTMP_Connect(rtmp,NULL, err)){
        PILI_RTMP_Log(PILI_RTMP_LOGERROR,"Connect Err\n");
        PILI_RTMP_Free(rtmp);
        CleanupSockets();
        return -1;
    }
    
    if (!PILI_RTMP_ConnectStream(rtmp,0,err)){
        PILI_RTMP_Log(PILI_RTMP_LOGERROR,"ConnectStream Err\n");
        PILI_RTMP_Close(rtmp,err);
        PILI_RTMP_Free(rtmp);
        CleanupSockets();
        return -1;
    }
    
    printf("Start to send data ...\n");
    
    //jump over FLV Header
    fseek(fp,9,SEEK_SET);
    //jump over previousTagSizen
    fseek(fp,4,SEEK_CUR);
    start_time=PILI_RTMP_GetTime();
    while(1)
    {
        /*if((((now_time=PILI_RTMP_GetTime())-start_time)
         <(pre_frame_time)) && bNextIsKey){
         //wait for 1 sec if the send process is too fast
         //this mechanism is not very good,need some improvement
         if(pre_frame_time>lasttime){
         PILI_RTMP_LogPrintf("TimeStamp:%8lu ms\n",pre_frame_time);
         lasttime=pre_frame_time;
         }
         sleep(1000);
         continue;
         }*/
        
        //jump over type
        fseek(fp,1,SEEK_CUR);
        if(!ReadU24(&datalength,fp))
            break;
        if(!ReadTime(&timestamp,fp))
            break;
        //jump back
        fseek(fp,-8,SEEK_CUR);
        
        pFileBuf=(char*)malloc(11+datalength+4);
        memset(pFileBuf,0,11+datalength+4);
        if(fread(pFileBuf,1,11+datalength+4,fp)!=(11+datalength+4))
            break;
        if(timestamp - pre_frame_time > 0&&timestamp - pre_frame_time < 200)
        {
            PILI_RTMP_Log(PILI_RTMP_LOGERROR,"sleep %u \n",timestamp - pre_frame_time);
            usleep(timestamp - pre_frame_time);
        }
        pre_frame_time=timestamp;
        
        if (!PILI_RTMP_IsConnected(rtmp)){
            PILI_RTMP_Log(PILI_RTMP_LOGERROR,"rtmp is not connect\n");
            break;
        }
        PILI_RTMP_Log(PILI_RTMP_LOGERROR,"PILI_RTMP_Write %d \n",11+datalength+4);
        
        if (!PILI_RTMP_Write(rtmp,pFileBuf,11+datalength+4, err)){
            PILI_RTMP_Log(PILI_RTMP_LOGERROR,"Rtmp Write Error\n");
            break;
        }
        
        free(pFileBuf);
        pFileBuf=NULL;
        
        if(!PeekU8(&type,fp))
            break;
        if(type==0x09){
            if(fseek(fp,11,SEEK_CUR)!=0)
                break;
            if(!PeekU8(&type,fp)){
                break;
            }
            if(type==0x17)
                bNextIsKey=1;
            else
                bNextIsKey=0;
            fseek(fp,-11,SEEK_CUR);
        }
    }
    
    PILI_RTMP_LogPrintf("\nSend Data Over\n");
    
    if(fp)
        fclose(fp);
    
    if (rtmp!=NULL){
        PILI_RTMP_Close(rtmp, err);
        PILI_RTMP_Free(rtmp);
        rtmp=NULL;
    }
    
    if(pFileBuf){
        free(pFileBuf);
        pFileBuf=NULL;
    }
    
    CleanupSockets();
    return 0;
}
int Process(char* url, const char *flv, const char* report, int useWmp)
{
    //LOGI("Process, url=%s, flv=%s, report=%s, usewmp=%d", url, flv, report, useWmp)
    publish_using_write(flv,url);
   


    return 0;
}

void pusherInit()
{
    s_exit = false;

    s_begin = get_time();
    if (!s_inited) {
        s_inited = true;
    }
}

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////////
//   Android
//////////////////////////////////////////////////////////////////////////////////

#if defined(ANDROID) //Android

JNIEXPORT int Java_com_example_flvpusher_FlvPusher_startPush(JNIEnv* env, jobject thiz,
    jstring jurl, jstring jflv, jstring jlog, jstring jreport, int useWmp)
{
    LOGI("native startPush");
    
    pusherInit();
    
    int ret = 0; 
    
	char* url = (char*)env->GetStringUTFChars(jurl, NULL); //flv file path
	char* flv = (char*)env->GetStringUTFChars(jflv, NULL); //flv file path
	char* log = (char*)env->GetStringUTFChars(jlog, NULL); //debug log path
	char* report = (char*)env->GetStringUTFChars(jreport, NULL); //report log path

	ret = Process(url, flv, report, useWmp);

	env->ReleaseStringUTFChars(jurl, url);
	env->ReleaseStringUTFChars(jflv, flv);
	env->ReleaseStringUTFChars(jlog, log);
	env->ReleaseStringUTFChars(jreport, report);
    
    LOGI("native push done");
    return ret;
}

JNIEXPORT int Java_com_example_flvpusher_FlvPusher_stopPush(JNIEnv* env, jobject thiz)
{
    s_exit = true;
}

#endif

//////////////////////////////////////////////////////////////////////////////////
//   iOS
//////////////////////////////////////////////////////////////////////////////////

#ifdef __APPLE__  //iOS

#include "flv_pusher.h"

void startPushStream(const char* url, const char* flv, const char* log, int useWmp)
{
    pusherInit();
    Process((char*)url, flv, log, useWmp);
}

void stopPushStream()
{
    s_exit = true;
}

#endif

//////////////////////////////////////////////////////////////////////////////////
//   Linux
//////////////////////////////////////////////////////////////////////////////////

#ifdef __LINUX__
int main(int argc, char *argv[])
{
    set_log_fun(rtmp_write_log);
    int ret = Process((char*)URL, "./ws_cut.flv", "./report.log", 0);
    return ret;
}
#endif

#ifdef __cplusplus
}
#endif




