#include "packet_listener.h"

static void 
TPacketListener_FireEvent(TPacketListener* self, net_ip_addr* in_addr, sse_int in_port, sse_char* in_buf, sse_int in_buflen) {
  if (self->fCallback) {
    (*self->fCallback)(in_addr, in_port, in_buf, in_buflen, self->fCallbackArg);
  }
}

sse_bool 
TPacketListener_Initialize(TPacketListener* self) {
  sse_memset(self, 0, sizeof(TPacketListener));
  FD_ZERO(&self->fDescs);
  return sse_true;
}

void 
TPacketListener_Finalize(TPacketListener* self) {
}

sse_int 
TPacketListener_Start(TPacketListener* self, net_ip_addr* in_addr, sse_int in_port) {
  sse_int ecode;
  sse_int desc;

  ecode = Net_TcpOpen(&desc);
  if (ecode != SSE_E_OK) {
    APP_LOG_ERROR("TNet_TcpOpen failed");
    return ecode;
  }

  ecode = Net_TcpBind(desc, in_addr, in_port);
  if (ecode != SSE_E_OK) {
    APP_LOG_ERROR("Net_TcpBind failed");
    goto error_exit_1;
  }

  ecode = Net_TcpListen(desc, 32);
  if (ecode != SSE_E_OK) {
    APP_LOG_ERROR("Net_TcpListen failed");
    goto error_exit_2;
  }

  self->fListenDesc = desc;
  self->fMaxDesc = desc;
  FD_SET(desc, &self->fDescs);

  return SSE_E_OK;

 error_exit_2:
 error_exit_1:
  Net_TcpClose(desc);

  return SSE_E_GENERIC;
}

sse_int 
TPacketListener_Stop(TPacketListener* self) {
  sse_int desc;
  for (desc = 0; desc <= self->fMaxDesc; desc++) {
    if (FD_ISSET(desc, &self->fDescs)) {
      Net_TcpClose(desc);
    }
  }
  return SSE_E_OK;
}

sse_int 
TPacketListener_Progress(TPacketListener* self) {
  struct timeval tv;
  fd_set descs; /* working copy */
  int n, desc, new_desc;
  net_ip_addr remote_addr;
  sse_int remote_port;
  sse_char buf[512];
  sse_int nread;

  memcpy(&descs, &self->fDescs, sizeof(self->fDescs));

  tv.tv_usec = 0;
  tv.tv_sec = 0;
  n = select(self->fMaxDesc + 1, &descs, NULL, NULL, &tv);
  if (n < 0) {
    APP_LOG_ERROR("select failed: %s", strerror(errno));
    return SSE_E_GENERIC;
  } else if (n == 0) {
    /* Timeout. No readable sockets are ready. */
    return SSE_E_INPROGRESS;
  }

  for (desc = 0; desc <= self->fMaxDesc && n > 0;  desc++) {
    if (FD_ISSET(desc, &descs)) {
      n--;
      /* check if the desc is listening socket. */
      if (desc == self->fListenDesc) {
	new_desc = Net_TcpAccept(self->fListenDesc, &remote_addr, &remote_port);
	APP_LOG_DEBUG("Accepted a new end-point: %d.%d.%d.%d:%d", 
		      remote_addr.value[0], remote_addr.value[1], remote_addr.value[2], remote_addr.value[3], remote_port);
	FD_SET(new_desc, &self->fDescs);
	if (new_desc > self->fMaxDesc) {
	  self->fMaxDesc = new_desc;
	}
      } else {
	nread = Net_TcpRead(desc, buf, sizeof(buf));
	if (nread > 0) {
	  Net_TcpGetRemoteAddrPort(desc, &remote_addr, &remote_port);
	  TPacketListener_FireEvent(self, &remote_addr, remote_port, buf, nread);
	} else {
	  if (nread != SSE_E_INPROGRESS) {
	    APP_LOG_DEBUG("Connection closed by the client or some error occurred.");
	    Net_TcpClose(desc);
	    FD_CLR(desc, &self->fDescs);
	    if (desc == self->fMaxDesc) {
	      for (; FD_ISSET(self->fMaxDesc, &self->fDescs) == 0; ) {
		self->fMaxDesc--;
	      }
	    }
	  }
	}
      }
    }
  }

  return SSE_E_OK;
}

sse_int 
TPacketListener_SetPacketCallback(TPacketListener* self, PacketListener_PacketCallback in_callback, sse_pointer in_callback_arg) {
  self->fCallback = in_callback;
  self->fCallbackArg = in_callback_arg;
  return SSE_E_OK;
}



