#!/usr/bin/env ruby
require 'socket'
$socketPath = "/tmp/foo-wm.socket"

def sendCommand(command)
  sock = UNIXSocket.open($socketPath)
  sock.send command, 0 
  loop do
    if (sock.recv(1, Socket::MSG_PEEK) == "") then
      break
    end 
    print sock.recv(1)
  end
end

if (ARGV.size == 0) then
  puts "No command provided"
else 
  sendCommand(ARGV.join(' '))
end
