#!/usr/bin/env ruby
require 'socket'
require 'json'
require 'pp'
$socketPath = "/tmp/stable-wm.socket"

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

jsonTree = sendCommand('get focus')[0..-2]

tree = JSON.parse(jsonTree);
if (tree["type"] != "container") then
  puts "uncontainerize may only be used on a container"
  exit
end

tree["children"].each do |child|
  nodeId = child["id"]
  sendCommand("focus id #{child['id']}")
  sendCommand("shift pc -1")
end
