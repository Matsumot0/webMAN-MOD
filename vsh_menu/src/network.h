#ifndef __NETWORK_H__
#define __NETWORK_H__



#define DB_IP      "192.168.178.23"          // debug host ip 
#define DB_PORT    9999                      // debug port 


int32_t create_conn(const char *ip, uint16_t port, uint8_t pro);
int32_t listen_conn(int32_t port, int32_t backlog);
void close_conn(int32_t *s);
void dbg_init(void);
void dbg_fini(void);
void dbg_printf(const char* fmt, ...);


#endif // __NETWORK_H__ 
