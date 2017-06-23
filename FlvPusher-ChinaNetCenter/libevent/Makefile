TARGET = libevent.a
CFLAGS = -fPIC -D_LINUX_TEST_ $(EXTRA_FLAGS) -Iinclude -DHAVE_CONFIG_H -D_EVENT_HAVE_FD_MASK -fvisibility=hidden


TARGET_DEPS =\
             event.o\
             evthread.o\
             buffer.o\
             bufferevent.o\
             bufferevent_sock.o\
             bufferevent_filter.o\
             bufferevent_pair.o\
             listener.o\
             bufferevent_ratelim.o\
             evmap.o\
             signal.o\
             epoll.o\
             poll.o\
             select.o\
             log.o\
             evutil.o\
             evutil_rand.o\
             strlcpy.o\
             event_tagging.o\
             http.o\
             evdns.o\
             evrpc.o
$(TARGET):$(TARGET_DEPS)
	$(AR) rcs $(TARGET) $(TARGET_DEPS)

clean:
	$(RM) $(TARGET) $(TARGET_DEPS)
