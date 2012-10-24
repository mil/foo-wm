#!/usr/bin/env ruby
require 'socket'
require 'json'
require 'pp'
$socketPath = "/tmp/foo-wm.socket"

def sendCommand(command)
  response = ""
  sock = UNIXSocket.open($socketPath)
  sock.send command, 0 
  loop do
    if (sock.recv(1, Socket::MSG_PEEK) == "") then
      break
    end 
    response = "#{response}#{sock.recv(1)}"
  end

  return response
end

tree = sendCommand('get marks')[0..-2]
(JSON tree)["marks"].each do |mark|
  puts mark 
end

