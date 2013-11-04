#ifndef __PACKET_LISTENER_H__
#define __PACKET_LISTENER_H__

#include <servicesync/moat.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/ioctl.h> 
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "net_util.h"
#include "app_debug.h"

SSE_BEGIN_C_DECLS

typedef void (*PacketListener_PacketCallback)(net_ip_addr* in_remote_addr, sse_int in_remote_port, 
					      sse_char* in_buf, sse_int in_buflen, sse_pointer in_callback_arg);

typedef struct TPacketListener_ TPacketListener;
struct TPacketListener_ {
  int fListenDesc;
  fd_set fDescs;
  sse_int fMaxDesc;
  PacketListener_PacketCallback fCallback;
  sse_pointer fCallbackArg;
};

sse_bool TPacketListener_Initialize(TPacketListener* self);
void TPacketListener_Finalize(TPacketListener* self);
sse_int TPacketListener_Start(TPacketListener* self, net_ip_addr* in_addr, sse_int in_port);
sse_int TPacketListener_Stop(TPacketListener* self);
sse_int TPacketListener_Progress(TPacketListener* self);
sse_int TPacketListener_SetPacketCallback(TPacketListener* self, PacketListener_PacketCallback in_callback, sse_pointer in_callback_arg);

SSE_END_C_DECLS

#endif /* __PACKET_LISTENER_H__ */
