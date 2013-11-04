#include "packet_listener.h"

static struct sockaddr* 
net_ipaddr_to_sockaddr(int* out_addrsz, struct sockaddr_in* out_sin, struct sockaddr_in6* out_sin6, const net_ip_addr* in_src, int in_port) {
  struct sockaddr *addr;
  switch (in_src->type) {
  case IP_ADDR_TYPE_V4:
    memset(out_sin, 0, sizeof(*out_sin));
    out_sin->sin_family = AF_INET;
    memcpy(&out_sin->sin_addr.s_addr, in_src->value, IP_ADDR_LEN_V4);
    out_sin->sin_port = htons((unsigned short)in_port);
    addr = (struct sockaddr*)out_sin;
    *out_addrsz = sizeof(*out_sin);
    break;
  case IP_ADDR_TYPE_V6:
    memset(out_sin6, 0, sizeof(*out_sin6));
    out_sin6->sin6_family = AF_INET6;
    memcpy(out_sin6->sin6_addr.s6_addr, in_src->value, IP_ADDR_LEN_V6);
    out_sin6->sin6_port = htons((unsigned short)in_port);
    addr = (struct sockaddr*)out_sin6;
    *out_addrsz = sizeof(*out_sin6);
    break;
  default:
    return 0;
  }

  return addr;
}

static int
net_sockaddr_to_ipaddr(net_ip_addr *out_dst, struct sockaddr *in_src) {
  struct sockaddr_in* sin;
  struct sockaddr_in6* sin6;

  switch (in_src->sa_family) {
  case AF_INET:
    sin = (struct sockaddr_in*)in_src;
    if (out_dst) {
      out_dst->type = IP_ADDR_TYPE_V4;
      memcpy(out_dst->value, &sin->sin_addr.s_addr, 4);
    }
    return (sse_int)ntohs(sin->sin_port);
  case AF_INET6:
    sin6 = (struct sockaddr_in6*)in_src;
    if (out_dst) {
      out_dst->type = IP_ADDR_TYPE_V6;
      memcpy(out_dst->value, &sin6->sin6_addr, 16);
    }
    return (sse_int)ntohs(sin6->sin6_port);
  default:
    APP_LOG_ERROR("unknown socket address family:[%d]", in_src->sa_family);
    return -1;
  }
}

sse_int
Net_TcpOpen(sse_int* out_desc) {
  sse_int desc, ecode, nonblock;
  struct linger lin;

  desc = socket(AF_INET, SOCK_STREAM, 0);
  if (desc == INVALID_SOCKET) {
    APP_LOG_ERROR("socket failed. %s", strerror(errno));
    return SSE_E_GENERIC;
  }

  nonblock = 1;
  ecode = ioctl(desc, FIONBIO, &nonblock);
  if (ecode < 0) {
    APP_LOG_ERROR("ioctl failed: %s", strerror(errno));
    goto error_exit_1;
  }

  lin.l_onoff = 1;
  lin.l_linger = 0;
  if (setsockopt(desc, SOL_SOCKET, SO_LINGER, (char*)&lin, sizeof(lin))){
    APP_LOG_ERROR("setsockopt: %s", strerror(errno));
    goto error_exit_2;
  }

  *out_desc = desc;

  return SSE_E_OK;

 error_exit_2:
 error_exit_1:
  close(desc);

  return SSE_E_GENERIC;
}

sse_int 
Net_TcpClose(sse_int in_desc) {
  close((int)in_desc);
  return SSE_E_OK;
}

sse_int 
Net_TcpIsConnected(sse_int in_desc) {
  fd_set fds;
  struct timeval tv;
  int n, error;
  socklen_t error_len;

  n = in_desc + 1;

  FD_ZERO(&fds);
  FD_SET((int)in_desc, &fds);
  tv.tv_usec = 0;
  tv.tv_sec = 0;
    
  n = select(n, NULL, NULL, &fds, &tv);
  if (n != 0) {
    APP_LOG_ERROR("select");
    return SSE_E_GENERIC;
  }

  n = in_desc + 1;

  FD_ZERO(&fds);
  FD_SET(in_desc, &fds);

  n = select(n, NULL, &fds, NULL, &tv);
  if (n != 0){
    if (n < 0) {
      APP_LOG_ERROR("select failed");
      return SSE_E_GENERIC;
    } else {
      error_len = sizeof(error);
      if (getsockopt(in_desc, SOL_SOCKET, SO_ERROR, &error, &error_len) < 0) {
	APP_LOG_ERROR("getsockopt failed");
	return SSE_E_GENERIC;
      }

      if (error == 0) {
	return n;
      } else {
	return SSE_E_GENERIC;
      }
    }
  }

  return SSE_E_INPROGRESS;  
}

sse_int 
Net_TcpCanReadWrite(sse_int in_desc, sse_int in_rw) {
  fd_set fds, wfds;
  struct timeval tv;
  int selected, n;

  tv.tv_sec = 0; 
  tv.tv_usec = 0;

  FD_ZERO(&fds);
  FD_SET((int)in_desc, &fds);
    
  n = in_desc + 1;

  switch (in_rw) {
  case STREAM_READ:
    selected = select(n, &fds, NULL, NULL, &tv);
    APP_LOG_DEBUG("selected=%d", selected);
    if (selected != 1) {
      return selected;
    }
    return STREAM_READ;
  case STREAM_WRITE:
    selected = select(n, NULL, &fds, NULL, &tv);
    if (selected != 1) {
      return selected;
    }
    return STREAM_WRITE;
  default:
    FD_ZERO(&wfds);
    FD_SET((int)in_desc, &wfds);
    selected = select(n, &fds, &wfds, NULL, &tv);
    if (selected <= 0) {
      return selected;
    }
    if (selected == 2) {
      return STREAM_READWRITE;
    }
    if (FD_ISSET((int)in_desc, &fds)) {
      return STREAM_READ;
    }
    return STREAM_WRITE;
  }  
}

sse_int 
Net_TcpConnect(sse_int in_desc, net_ip_addr* in_addr, sse_int in_port) {
  struct sockaddr_in name;
  struct sockaddr_in6 name6;
  struct sockaddr* addr;
  sse_int addrsize;

  addr = net_ipaddr_to_sockaddr(&addrsize, &name, &name6, in_addr, in_port);
  if (addr == NULL) {
    APP_LOG_ERROR("net_ipaddr_to_sockaddr");
    return SSE_E_GENERIC;
  }
  
  if (connect((int)in_desc, addr, addrsize) == 0) {
    return SSE_E_OK;
  }

  if (errno == EINPROGRESS) {
    return SSE_E_INPROGRESS;
  }

  return SSE_E_GENERIC;
}

sse_int 
Net_TcpRead(sse_int in_desc, sse_char* out_buf, sse_int in_len) {
  int nread;
  nread = recv((int)in_desc, (char*)out_buf, in_len, 0);        
  if (nread == -1){
    if (errno == EINPROGRESS) {
      return SSE_E_INPROGRESS;
    }
    APP_LOG_ERROR("recv failed: %s", strerror(errno));
    return SSE_E_GENERIC;
  }
  return nread;
}

sse_int
Net_TcpWrite(sse_int in_desc, sse_char* in_buf, sse_int in_len) {
  int nwritten;
  nwritten = send((int)in_desc, (char*)in_buf, in_len, 0);
  if (nwritten == -1){
    if (errno == EINPROGRESS) {
      return SSE_E_INPROGRESS;
    }
    APP_LOG_ERROR("send");
    return SSE_E_GENERIC;
  }
  return nwritten;
}

sse_int 
Net_TcpGetLocalAddrPort(sse_int in_desc, net_ip_addr* out_addr, sse_int* out_port) {
  int retval;
  socklen_t size;
  struct sockaddr_in local_sock;
  size = sizeof(local_sock);
  retval = getsockname(in_desc, (struct sockaddr*)&local_sock, &size);
  if (retval == 0) {
    *out_port = net_sockaddr_to_ipaddr(out_addr, (struct sockaddr*)&local_sock);
    return SSE_E_OK;
  } else {
    APP_LOG_ERROR("getsockname");
    return SSE_E_GENERIC;
  }
}

sse_int 
Net_TcpGetRemoteAddrPort(sse_int in_desc, net_ip_addr* out_addr, sse_int* out_port) {
  int retval;
  socklen_t size;
  struct sockaddr_in local_sock;
  size = sizeof(local_sock);
  retval = getpeername(in_desc, (struct sockaddr*)&local_sock, &size);
  if (retval == 0) {
    *out_port = net_sockaddr_to_ipaddr(out_addr, (struct sockaddr*)&local_sock);
    return SSE_E_OK;
  } else {
    APP_LOG_ERROR("getpeername");
    return SSE_E_GENERIC;
  }
}

sse_int 
Net_TcpAccept(sse_int in_desc, net_ip_addr* out_addr, sse_int* out_port) {
  int sock;
  struct sockaddr_in sockaddr;
  socklen_t addrlen;

  memset(&sock, 0, sizeof(sock));
  memset(&sockaddr, 0, sizeof(sockaddr));
  addrlen = sizeof(sockaddr);
  sock = accept((int)in_desc, (struct sockaddr*)&sockaddr, &addrlen);
  if (sock == INVALID_SOCKET) {
    APP_LOG_ERROR("accept failed: %s", strerror(errno));
    return SSE_E_GENERIC;
  }

  if (sockaddr.sin_family == AF_INET) {
    struct sockaddr_in* sa_in = (struct sockaddr_in*)&sockaddr;
    if (out_addr) {
      out_addr->type = IP_ADDR_TYPE_V4;
      memcpy(out_addr->value, &sa_in->sin_addr.s_addr, IP_ADDR_LEN_V4);
    }
    if (out_port) {
      *out_port = sa_in->sin_port;
    }
  } else if (sockaddr.sin_family == AF_INET6) {
    struct sockaddr_in6* sa_in6 = (struct sockaddr_in6*)&sockaddr;
    if (out_addr) {
      out_addr->type = IP_ADDR_TYPE_V6;
      memcpy(out_addr->value, &sa_in6->sin6_addr, IP_ADDR_LEN_V6);
    }
    if (out_port) {
      *out_port = sa_in6->sin6_port;
    }
  } else {
    APP_LOG_ERROR("unknown socket address family:%d", sockaddr.sin_family);
    return SSE_E_GENERIC;
  }
  
  return (sse_int)sock;
}

sse_int 
Net_TcpBind(sse_int in_desc, net_ip_addr* in_addr, sse_int in_port) {
  struct sockaddr_in name;
  struct sockaddr_in6 name6;
  struct sockaddr* addr;
  int size;

  if (in_addr->type != IP_ADDR_TYPE_V4 && in_addr->type != IP_ADDR_TYPE_V6) {
    APP_LOG_ERROR("unknown IP address type:%d", in_addr->type);
    return SSE_E_GENERIC;
  }
  if (in_addr->type == IP_ADDR_TYPE_V4) {
    size = sizeof(name);
    memset(&name, 0, sizeof(name));
    name.sin_family = AF_INET;
    memcpy(&name.sin_addr, in_addr->value, IP_ADDR_LEN_V4);
    name.sin_port = htons((unsigned short)in_port);
    addr = (struct sockaddr*) &name;
  } else {
    size = sizeof(name6);
    memset(&name6, 0, sizeof(name6));
    name6.sin6_family = AF_INET6;
    memcpy(&name6.sin6_addr, in_addr->value, IP_ADDR_LEN_V6);
    name6.sin6_port = htons((unsigned short)in_port);
    addr = (struct sockaddr*)&name6;
  }
  if (bind((int)in_desc, addr, size) == 0) {
    APP_LOG_DEBUG("bind done");
    return SSE_E_OK;
  } else {
    APP_LOG_ERROR("bind failed: %s", strerror(errno));
    return SSE_E_GENERIC;
  }
}

sse_int 
Net_TcpListen(sse_int in_desc, sse_int in_qlen) {
  if (listen((int)in_desc, (int)in_qlen) == 0) {
    APP_LOG_DEBUG("listen done");
    return SSE_E_OK;
  } else {
    APP_LOG_ERROR("listen failed: %s", strerror(errno));
    return SSE_E_GENERIC;
  }
}
