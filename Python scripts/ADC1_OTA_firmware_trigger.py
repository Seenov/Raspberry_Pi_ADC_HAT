# Python program to force  our ESP32C3 Raspberry PI ADC hat into OTA over the air
# firmware download mode.
# send ota_key on broadcast UDP channel
# ADC will receive OTA key, caterogy= setting  and set itself in OTA mode. It will display a webpage 
# where a user can log in with admin admin and select a .bin file to program the ADC esp32C3
# The .bin file can be created using the Arduino IDE in Sketch/Export compiled Binary
# Renaming the file with a short name simplifies entering the name in the web page.
# The OTA_Key is 1234
#
import socket
import time
import json
import sys

# a Python object (dict):
x = { 
  "type" : "ADC1",
  "name" : "my ADC1 for tesp",
  "category" : "setting",
  "ota" : 1234
}

# convert into JSON:
y= json.dumps(x)
message = bytes( y, 'utf-8')
# listening for ip from ADC after seending OTA
UDPRX_IP = "172.16.0.238"
UDPRX_PORT = 50450
socka = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
socka.bind((UDPRX_IP, 50450))


server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)

# Enable port reusage so we will be able to run multiple clients and servers on single (host, port). 
# Do not use socket.SO_REUSEADDR except you using linux(kernel<3.9): goto https://stackoverflow.com/questions/14388706/how-do-so-reuseaddr-and-so-reuseport-differ for more information.
# For linux hosts all sockets that want to share the same address and port combination must belong to processes that share the same effective user ID!
# So, on linux(kernel>=3.9) you have to run multiple servers and clients under one user to share the same (host, port).
# Thanks to @stevenreddie
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)

# Enable broadcasting mode
server.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

# Set a timeout so the socket does not block
# indefinitely when trying to receive data.
server.settimeout(0.2)

server.sendto(message, ('<broadcast>', 5044))
print("OTA message sent!, waiting for remote response")



while True:
     data, addr = socka.recvfrom(1024) # buffer size is 1024 bytes
     # parse :
     rx = json.loads(data)

     print(rx["msg"])
     
     print(rx["localip"])
     print("")
     print("The end, exiting")
#print("received message: %s" % data)
     
     
     

#time.sleep(1)
#sys.exit()