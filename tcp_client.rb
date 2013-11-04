#!/usr/bin/ruby

require "socket"

begin
  sock = TCPSocket.open("127.0.0.1", 12345)
  10.times do
    sleep 3
    sock.write("HELLO")
  end    
  sock.close
rescue
  puts $!
end

