#include <servicesync/moat.h>
#include <ev.h>
#include "packet_listener.h"

#define SERVER_PORT (12345)

static TPacketListener sPacketListener;

static void 
DumpPacketData(sse_char* in_buf, sse_int in_buflen) {
  sse_char buf[512];
  sse_int i, nwritten;
  for (i = 0, nwritten = 0; i < in_buflen; i++) {
    nwritten += sprintf(buf + nwritten, "%x", in_buf[i] & 0xff);
  }
  APP_LOG_DEBUG("%s", buf);
}

static void
TimerProc(struct ev_loop* loop, ev_periodic* in_arg, int in_revents) {
  APP_LOG_DEBUG(">>> TimerProc");

  TPacketListener_Progress(&sPacketListener);
}

static void 
PacketCallback(net_ip_addr* in_remote_addr, sse_int in_remote_port, sse_char* in_buf, sse_int in_buflen, sse_pointer in_callback_arg) {
  APP_LOG_DEBUG(">>> PacketCallback");

  APP_LOG_DEBUG("%d bytes data read from end-point: %d.%d.%d.%d:%d", 
		in_buflen, in_remote_addr->value[0], in_remote_addr->value[1], in_remote_addr->value[2], in_remote_addr->value[3], in_remote_port);

  /* NOTICE: You must store the received data and parse it. 
   * Offcource you must define the protocol over the TCP preliminarily.
   * This program is an only example, so we don't care about the protocol.
   */
  DumpPacketData(in_buf, in_buflen);
}

int main(int argc, char* argv[]) {
  sse_int ecode;
  net_ip_addr addr;
  sse_int port;
  struct ev_loop* loop;
  ev_periodic timer;

  if (!TPacketListener_Initialize(&sPacketListener)) {
    APP_LOG_ERROR("TPacketListener_Initialize failed");
    return EXIT_FAILURE;
  }

  sse_memset(&addr, 0, sizeof(addr));
  addr.type = IP_ADDR_TYPE_V4;
  addr.value[0] = 127;
  addr.value[1] = 0;
  addr.value[2] = 0;
  addr.value[3] = 1;
  port = SERVER_PORT;

  ecode = TPacketListener_Start(&sPacketListener, &addr, port);
  if (ecode != SSE_E_OK) {
    APP_LOG_ERROR("TPacketListener_Start failed");
    goto error_exit_1;
  }
  
  TPacketListener_SetPacketCallback(&sPacketListener, PacketCallback, NULL);

  /* We use 'libev' for message loop to poll socket every 1 seconds and read data.
   * This is because the polling way is most simple. */
  loop = ev_default_loop(0);
  ev_periodic_init(&timer, TimerProc, 0, (ev_tstamp)1, 0);
  ev_periodic_start(loop, &timer);

  /* Forever blocks until ev_quit() is called. In this program, we don't call ev_quit().
   * You can stop this program by CTRL-C. */
  ev_run(loop, 0);

  TPacketListener_Stop(&sPacketListener);
  TPacketListener_Finalize(&sPacketListener);

  return EXIT_SUCCESS;

 error_exit_1:
  TPacketListener_Finalize(&sPacketListener);

  return EXIT_FAILURE;
}
