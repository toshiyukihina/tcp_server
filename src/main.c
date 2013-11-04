#include <servicesync/moat.h>
#include <ev.h>
#include "packet_listener.h"

#define SERVER_PORT (12345)

static TPacketListener sPacketListener;

static void
TimerProc(struct ev_loop* loop, ev_periodic* in_arg, int in_revents) {
  APP_LOG_DEBUG(">>> TimerProc");
  TPacketListener_Progress(&sPacketListener);
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

  loop = ev_default_loop(0);
  ev_periodic_init(&timer, TimerProc, 0, (ev_tstamp)1, 0);
  ev_periodic_start(loop, &timer);
  ev_run(loop, 0);

  TPacketListener_Stop(&sPacketListener);
  TPacketListener_Finalize(&sPacketListener);

  return EXIT_SUCCESS;

 error_exit_1:
  TPacketListener_Finalize(&sPacketListener);

  return EXIT_FAILURE;
}
