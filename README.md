Linux socket programming.
======

You can build and run this program on general Linux distribution perhaps.
For now, tested on debian linux.

## Prerequisites ##

You must install the depending libraries as below.
(Don't care about the ruby version. Install your favorite version.)

```
apt-get install libev-dev ruby1.9.3
```

## How to build ##

```
cd tcp_server
./configure
make
```

## How to run ##

### Run the TCP server. ###

This TCP server listens on localhost:12345 and dump data received from the TCP client.

```
./src/serial_sample
```

### Run the TCP client. ###

This TCP client send 'HELLO' message to the TCP server listening on localhost:12345 every 3 seconds.

```
ruby tcp_client.rb
```

You will see the debug message on the console show below:

```
...
[DEBUG] <tcp-server-sample> packet_listener.c:TPacketListener_Progress():L100 Accepted a new end-point: 127.0.0.1:15760
[DEBUG] <tcp-server-sample> main.c:TimerProc():L11 >>> TimerProc
[DEBUG] <tcp-server-sample> main.c:TimerProc():L11 >>> TimerProc
[DEBUG] <tcp-server-sample> main.c:TimerProc():L11 >>> TimerProc
[DEBUG] <tcp-server-sample> packet_listener.c:TPacketListener_Progress():L110 5 bytes data read from end-point: 127.0.0.1:36925
[DEBUG] <tcp-server-sample> packet_listener.c:PacketListener_DumpData():L10 48454c4c4f
[DEBUG] <tcp-server-sample> main.c:TimerProc():L11 >>> TimerProc
[DEBUG] <tcp-server-sample> main.c:TimerProc():L11 >>> TimerProc
[DEBUG] <tcp-server-sample> main.c:TimerProc():L11 >>> TimerProc
[DEBUG] <tcp-server-sample> packet_listener.c:TPacketListener_Progress():L110 5 bytes data read from end-point: 127.0.0.1:36925
[DEBUG] <tcp-server-sample> packet_listener.c:PacketListener_DumpData():L10 48454c4c4f
[DEBUG] <tcp-server-sample> main.c:TimerProc():L11 >>> TimerProc
[DEBUG] <tcp-server-sample> main.c:TimerProc():L11 >>> TimerProc
[DEBUG] <tcp-server-sample> main.c:TimerProc():L11 >>> TimerProc
[DEBUG] <tcp-server-sample> packet_listener.c:TPacketListener_Progress():L110 5 bytes data read from end-point: 127.0.0.1:36925
[DEBUG] <tcp-server-sample> packet_listener.c:PacketListener_DumpData():L10 48454c4c4f
[DEBUG] <tcp-server-sample> main.c:TimerProc():L11 >>> TimerProc
[DEBUG] <tcp-server-sample> main.c:TimerProc():L11 >>> TimerProc
[DEBUG] <tcp-server-sample> main.c:TimerProc():L11 >>> TimerProc
[DEBUG] <tcp-server-sample> packet_listener.c:TPacketListener_Progress():L110 5 bytes data read from end-point: 127.0.0.1:36925
[DEBUG] <tcp-server-sample> packet_listener.c:PacketListener_DumpData():L10 48454c4c4f
...
```


