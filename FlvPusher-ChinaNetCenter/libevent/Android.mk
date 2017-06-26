####################################
# Build libevent as separate library

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE:= libevent

LOCAL_SRC_FILES := \
                   event.c\
                   evthread.c\
                   buffer.c \
                   bufferevent.c\
                   bufferevent_sock.c\
                   bufferevent_filter.c \
                   bufferevent_pair.c\
                   listener.c\
                   bufferevent_ratelim.c \
                   evmap.c\
                   signal.c\
                   epoll.c \
                   poll.c \
                   select.c \
                   log.c\
                   evutil.c\
                   evutil_rand.c\
                   strlcpy.c\
                   event_tagging.c\
                   http.c \
                   evdns.c \
                   evrpc.c\

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/ \
	$(LOCAL_PATH)/include\
	$(LOCAL_PATH)/compat/sys

LOCAL_CFLAGS := -Os -Wall -DHAVE_CONFIG_H -fvisibility=hidden

include $(BUILD_STATIC_LIBRARY)

