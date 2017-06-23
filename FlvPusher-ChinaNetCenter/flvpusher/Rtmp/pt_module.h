#ifndef PT_MODULE_H
#define PT_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

struct stat_info {
    unsigned int conv;
    unsigned int trans_len;
    unsigned int retrans_len;
    unsigned int unack_len;
    unsigned int unack_seq;
    unsigned int rtt;
    unsigned int snd_cwnd;
};

int pt_module_init(void);
int pt_socket_new(void);
int pt_socket_connect(int fd, const struct sockaddr *svr_addr, socklen_t addr_len);
int pt_socket_send(int fd, const void *buf, size_t len);
int pt_socket_recv(int fd, void *buf, size_t len);
int pt_socket_close(int fd);
int pt_socket_get_stat(int fd, struct stat_info *si);
void pt_module_fini(void);

#ifdef __APPLE__
void pt_log_enable(void);
#else
int pt_log_enable(const char *log_dir);
#endif
void pt_log_disable(void);

#ifdef __cplusplus
}
#endif

#endif
