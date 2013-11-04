#ifndef __NET_UTIL_H__
#define __NET_UTIL_H__

SSE_BEGIN_C_DECLS

#define IP_ADDR_TYPE_V4 (4)
#define IP_ADDR_TYPE_V6 (6)
#define IP_ADDR_LEN_V4 (4)
#define IP_ADDR_LEN_V6 (16)

#define STREAM_READ (0x01)
#define STREAM_WRITE (0x02)
#define STREAM_READWRITE (STREAM_READ | STREAM_WRITE)

#define INVALID_SOCKET (-1)

typedef struct net_ip_addr_ net_ip_addr;
struct net_ip_addr_ {
  /*! IP address type. IP_ADDR_TYPE_V4 or IP_ADDR_TYPE_V6. */
  sse_int type;
  /*! IP address value. */
  sse_uchar value[16];
};

sse_int Net_TcpOpen(sse_int* out_desc);
sse_int Net_TcpIsConnected(sse_int in_desc);
sse_int Net_TcpCanReadWrite(sse_int in_desc, sse_int in_rw);
sse_int Net_TcpConnect(sse_int in_desc, net_ip_addr* in_addr, sse_int in_port);
sse_int Net_TcpRead(sse_int in_desc, sse_char* out_buf, sse_int in_len);
sse_int Net_TcpWrite(sse_int in_desc, sse_char* in_buf, sse_int in_len);
sse_int Net_TcpGetLocalAddrPort(sse_int in_desc, net_ip_addr* out_addr, sse_int* out_port);
sse_int Net_TcpGetRemoteAddrPort(sse_int in_desc, net_ip_addr* out_addr, sse_int* out_port);
sse_int Net_TcpAccept(sse_int in_desc, net_ip_addr* out_addr, sse_int* out_port);
sse_int Net_TcpBind(sse_int in_desc, net_ip_addr* in_addr, sse_int in_port);
sse_int Net_TcpListen(sse_int in_desc, sse_int in_qlen);
sse_int Net_TcpClose(sse_int in_desc);

SSE_END_C_DECLS

#endif /* __NET_UTIL_H__ */
