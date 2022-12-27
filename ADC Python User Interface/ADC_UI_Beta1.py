import customtkinter # Main Import
import json
from _support_Beta1 import * 
from _widgets import *
import sys
# your imports from here
import socket
import os

#import fcntl, os
import errno
from MQueues import MQueues
from Threads import spawn_thread
from random import randint
"""
#################   INSTRUCTIONS  #################
Required files (all in same directory):
ADC_UI_Beta1.py
MQueues.py
_support_Beta1.py
Threads.py

Install CustomTkInter  link: https://github.com/TomSchimansky/CustomTkinter
pip3 install customtkinter
A copy of the customtkinter folder from the Github repository is placed in the 
same folder as this script in case you cannot use pip3

-  Connect power to the  ESP32C3 ADC Hat and wait for the Green LED to turn on.
-  The ADC will be in Hotspot mode. Using a WiFi enabled device, phone, tablet
   or Raspberry Pi. Connect to the Network ADC_xxx, xxx is part of the mac address
   of the ADC. Ignore no Internet messages.
-  Navigate to the Page 192.168.4.1  and enter the SSID and Password of your WiFi
-  The ADC will reboot and the green LED will turn off. If not, retry the previous steps.
-  Run the ADC_UI_Beta1.py python script. A control panel window will open in the center of the screen.
-  The scrit should find the ADC. Enable the ADC channels and you should see the voltages
   being displayed.
   
- Known issues: at startup ADC ch 0 may display a previous value instead of 0 mV
                Sliders are not very responsive on Raspberry PI computers.
                The Seenov Logo is not displayed

-  Find more information at www.seenov.com
-  email us at info@seenov.com for support.


---------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------
The MIT License (MIT)
Copyright © 2022 <Seenov inc.>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the “Software”), to deal in the Software without restriction, i
ncluding without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or s
ubstantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.


######################################################################################
"""   

global configmsg
configmsg ={
 "type" : "ADC1",
 "name" : "my ADC1 for test",
 "category": "setting",
 "ench0" : 99,
 "ench1" : 99,
 "ench2" : 99,
 "ench3" : 99,
 "ench4" : 99,
 "ench5" : 99,
 "delay" : 100,
 "ota" : 10,
 "dac" : 0,
 "led2" : 0,
 "io7" : 0,
 "io18" : 0,
 "io19" : 0 
}

global device1
device1={
"type":"ADC1",
"localip":"0.0.0.0",
"name":"my ADC1 for test",
"category": "init",   # init, settings,
"ch0":45,
"ch1":234,
"ch2":9872,
"ch3":10123,
"ch4":12345,
"ch5":0,
"out1":0,
"out2":0,
"out3":0,
"link":"down"  # for future use
}


############## Threading basics in Python ######################
'''
The threads in python are not realistic as you see in other languages such as C, Java
the effects are achieved with task scheduling, instead of real threads. But overall working
is the same in Python as in any other language, you have to manage their life cycle and execution
through Queues and Listener loops, I wrote MQueues class to help you get started without 
much trouble with the Python queues.  

'''
global UDP_LISTENER_THREAD, SLIDER_VALUE_THREAD, T_UDP_LISTENER
UDP_LISTENER_THREAD = MQueues.create_queue() 
SLIDER_VALUE_THREAD = MQueues.create_queue()
T_UDP_LISTENER = MQueues.create_queue()

#destination_ip= "172.16.1.99" #"192.168.1.5"  #  this is my network IP for the ADC simulator enter your own ip for the simulator
def main(*args, **kwargs):
    global root # Making it global so I can manage it in my custom exit function
    global debug_ip
    #debug_ip = "172.16.1.99" #'100.64.100.6'
    customtkinter.set_appearance_mode("light") # change to "dark" 
    customtkinter.set_default_color_theme("blue") 
    root = customtkinter.CTk()
    app_setup(ctk_instance=root)
    LOGO(parent=root)
    T_LABEL(parent=root)
    # We import components for the application and start to attach them one by one
    buttons(root=root)
    sliders(root=root)
    labels(root=root)
    value_labels(root=root)
    checkboxes(root=root)
    textarea(root=root)
    setup_UDP()
    get_rpi_ip_address()
    find_adc()
    # Launch Non Blocking Components like this in their own threads slider_values_update()
    spawn_thread(function=listen_to_UDP)
    root.protocol("WM_DELETE_WINDOW", on_exit)
    global APP_INSTANCE # General Variable to exert control over threads which have to end on application exit
    APP_INSTANCE = True # So, all gui updating related threads have to end on false value of the variable
    root.mainloop()
    
##################################################################
#
#  Find ADC  this assumes we are on the same network
#  and ADC wifi is connected to this network
#  normally the send is a broadcast because the destination
#  ip  is not known. Unfortunatly there is a bug to be fixed
#  for now we need to enter the destination ip
#  manually
#
##################################################################
def find_adc():
    global server, configmsg
    configmsg["category"] = "init"    # set to init to start communications
    configmsg_msg= json.dumps(configmsg)
    configmsg_json_msg = bytes( configmsg_msg, 'utf-8')
    server.sendto(configmsg_json_msg, ('<broadcast>', 5044))
    print("Finding and connecting to ADC")
    configmsg["category"] = "settings"  # set back to default
    configmsg_msg= json.dumps(configmsg)
    configmsg_json_msg = bytes( configmsg_msg, 'utf-8')
    server.sendto(configmsg_json_msg, ('<broadcast>', 5044))
    print("sending default settings")
# find  Raspberry Pi local IP needed for UDP messaging
def get_rpi_ip_address():
    rpi_ip_address = '';
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(("8.8.8.8",80))
    rpi_ip_address = s.getsockname()[0]
    s.close()
    return rpi_ip_address
    
    
#############################################################
#   setup UDP channels
############################################################
def setup_UDP():
    global server
    UDPRX_IP = get_rpi_ip_address()
    UDPRX_PORT = 5045
    print("The local IP address is:  ", UDPRX_IP, UDPRX_PORT)
    # for testing
    #print(UDPRX_IP)
    # for testing
    #xx= is_port_in_use(5045)
    #print (xx)

    global socka
    socka = socket.socket(socket.AF_INET, # Internet
                            socket.SOCK_DGRAM) # UDP
    # Set a timeout so the socket does not block
    # indefinitely when trying to receive data.
    #socka.close()
    try:
        socka.bind((UDPRX_IP, UDPRX_PORT))
    except socket.error as messg1:
         print('Bind failed. Error Code : '
              + str(messg1)+ 'Exitting')
         sys.exit()
    socka.settimeout(0.5)
    socka.setblocking(0)
    #define variable data in tkinter first
    data=  ""
    server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    # Enable port reusage so we will be able to run multiple clients and servers on single (host, port). 
    # Do not use socket.SO_REUSEADDR except you using linux(kernel<3.9): goto https://stackoverflow.com/questions/14388706/how-do-so-reuseaddr-and-so-reuseport-differ for more information.
    # For linux hosts all sockets that want to share the same address and port combination must belong to processes that share the same effective user ID!
    # So, on linux(kernel>=3.9) you have to run multiple servers and clients under one user to share the same (host, port).
    # Thanks to @stevenreddie
    server.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    # Enable broadcasting mode for initial communication with ADC
    server.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

def listen_to_UDP():
    global device1
    global remote_ip
    L_CONTROL = True # use control variables for loops not True and break as they won't work sometimes  
    while L_CONTROL:
        #DAC_value_label.configure(text=float(DAC_slider.get().__round__(2)))
        #Rate_value_label.configure(text=float(Rate_slider.get().__round__(2)))
        #sleep(0.1)
        # Use this Managed Queues Class to get elements from Queues without having to run another loop
        # Inside this loop keep force_no_wait to True for it to not block
        value = MQueues.get_first_from_queue(UDP_LISTENER_THREAD, force_no_wait=True)
        if value == 'EXIT':
            socka.close()
            L_CONTROL = False
        else:
            #print("LOOP listen_to_UDP") 
            try:
                #print("UDP Loop Running")
                if L_CONTROL is True:
                    #print("UDP Loop Running")
                    data, addr = socka.recvfrom(1024) # buffer size is 1024 bytes
                    #print(addr[0])
                    remote_ip = addr[0]   # find the remote ip in the local socket rx message 
                    #print("data=") for testing
                    #print(data[0])  for testing
                    # got a message, do something :)
                    rx2_json = json.loads(data)
                    device1 = rx2_json
                    update_labels(device1) # Will update labels like this
                    #  will updating device1 automatically update value_labels?
            except socket.error:
                e= socket.error
                #print("socket err:  ", e)
    MQueues.add_to_queue(T_UDP_LISTENER, "CONFIRMED")
    socka.close()
    print("Listening Ended")


def C_termination_udp_listener():
    " This send the termination value to the thread via a queue and waits for that function to "
    " End before it goes ahead and tried to kill the main process otherwise runtimeerrors will "
    " appear "
    MQueues.add_to_queue(UDP_LISTENER_THREAD, 'EXIT') # Send Stop Signal to Slider Value
    while True:
        print("Waiting for UDP Confirmation",  randint(0, 10))
        value = MQueues.get_first_from_queue(T_UDP_LISTENER, force_no_wait=True)
        socka.close()
        print("exitting")
        sys.exit()
        if value == 'CONFIRMED':
            return None

def on_exit():
    """ Call this function before quiting """
    C_termination_udp_listener() # Confirm termination of UDP listener
    sys.exit()

def buttons(root):
    global OTA_btn, Find_ADC_btn
    OTA_btn = BUTTON(parent=root, text='OTA', x=140, y=400, width=120, height=50, command=OTA_command)
    Find_ADC_btn = BUTTON(parent=root, text='Find ADC', x=330, y=400, width=120, height=50, command= find_adc)


def DAC_slider_update(*args, **kwargs):
    # ALERT! do not try to update values directly or by a thread, using get method as they are already passed in with args
    DAC_value_label.configure(text=int(args[0].__round__(0)))
    slider_command("dac",int(args[0].__round__(0)))
def Rate_slider_update(*args, **kwargs):
    # ALERT! do not try to update values directly or by a thread, using get method as they are already passed in with args
    Rate_value_label.configure(text=int(args[0].__round__(0)))
    slider_command("delay",int(args[0].__round__(0)))
def sliders(root):
    """ Each Component is seperately attached to the root = customtkinter.CTk()"""
    global DAC_slider, DAC_value_label, Rate_slider, Rate_value_label
    y = 90
    DAC_slider = SLIDER(parent=root, x=280, y=120, from_=0, to_=255)
    # this is not working   also DAC is 0 to 255  Rate is 50 to 10000
    DAC_slider.configure(command=DAC_slider_update) # On click event called function
    #add a command= function  to update configmsg and send over UDP configmsg["dac"] = DAC_value
    DAC_value_label = S_LABEL(parent=root, text=int(DAC_slider.get()), x=250, y=y + (40)*6, width=80, height=30)
    Rate_slider = SLIDER(parent=root, x=380, y=120, from_=50, to_=1000)
    Rate_slider.configure(command=Rate_slider_update) # On click event called function
    Rate_value_label = S_LABEL(parent=root, text=int(Rate_slider.get()), x=350, y=y + (40)*6, width=80, height=30)

def labels(root):
    global mV_label, DAC_label, Rate_label
    mV_label = LABEL(parent=root, text="mV", x=140, y=80, width=70, height=30)
    DAC_label = LABEL(parent=root, text="DAC", x=250, y=81, width=80, height=30)
    Rate_label = LABEL(parent=root, text="Rate", x=350, y=80, width=80, height=30)

def value_labels(root):
    global ADC_label_0, ADC_label_1, ADC_label_2, ADC_label_3, ADC_label_4, ADC_label_5, device1
    x = 140
    y = 87
    ADC_label_0 = S_LABEL(parent=root, text=device1["ch0"], x=x, y=y + (40)*1, width=80, height=30)
    ADC_label_1 = S_LABEL(parent=root, text=device1["ch1"], x=x, y=y + (40)*2, width=80, height=30)
    ADC_label_2 = S_LABEL(parent=root, text=device1["ch2"], x=x, y=y + (40)*3, width=80, height=30)
    ADC_label_3 = S_LABEL(parent=root, text=device1["ch3"], x=x, y=y + (40)*4, width=80, height=30)
    ADC_label_4 = S_LABEL(parent=root, text=device1["ch4"], x=x, y=y + (40)*5, width=80, height=30)
    ADC_label_5 = S_LABEL(parent=root, text=device1["ch5"], x=x, y=y + (40)*6, width=80, height=30)
    print("Loaded")

def checkboxes(root):
    global ADC_ckbox_0, ADC_ckbox_1, ADC_ckbox_2, ADC_ckbox_3, ADC_ckbox_4, ADC_ckbox_5, LED_2_ckbox, IO_07_ckbox, IO_18_ckbox, IO_19_ckbox
    x = 30
    y = 60
    ADC_ckbox_0 = CHECKBOX(parent=root, text='ADC 0',  x=x, y=y + 70,  command=switch_command)
    ADC_ckbox_1 = CHECKBOX(parent=root, text='ADC 1',  x=x, y=y + 110, command=switch_command)
    ADC_ckbox_2 = CHECKBOX(parent=root, text='ADC 2',  x=x, y=y + 150, command=switch_command)
    ADC_ckbox_3 = CHECKBOX(parent=root, text='ADC 3',  x=x, y=y + 190, command=switch_command)
    ADC_ckbox_4 = CHECKBOX(parent=root, text='ADC 4',  x=x, y=y + 230, command=switch_command)
    ADC_ckbox_5 = CHECKBOX(parent=root, text='ADC 5',  x=x, y=y + 270, command=switch_command)
    LED_2_ckbox = CHECKBOX(parent=root, text='LED 2',  x=x, y=y + 310, command=switch_command)
    IO_07_ckbox = CHECKBOX(parent=root, text='I/O 7',  x=x, y=y + 350, command=switch_command)
    IO_18_ckbox = CHECKBOX(parent=root, text='I/O 18', x=x, y=y + 390, command=switch_command)
    IO_19_ckbox = CHECKBOX(parent=root, text='I/O 19', x=x, y=y + 430, command=switch_command)

def textarea(root):
    global textbox
    textbox = TEXTBOX(parent=root, x=140, y=470, width=310, height=150)
    
def OTA_command():
     global device1
     global configmsg
     global remote_ip
     configmsg["ota"]=1234
     config_msg= json.dumps(configmsg)
     config_json_msg = bytes( config_msg, 'utf-8')
     server.sendto(config_json_msg, (remote_ip, 5044))  #server.sendto(config_json_msg, (device1["localip"] , 5044))    #server.sendto(config_json_msg, ('<broadcast>', 5044))  
     configmsg["ota"]=10   
     textbox.delete("0.0","end")
     textbox.insert("0.0","OTA message sent!")
     print("OTA message sent!")
     
def switch_command():
    global configmsg, device1
    global remote_ip
    
    if ADC_ckbox_0._check_state == True:
        configmsg["ench0"] = 100    
    else:
        configmsg["ench0"] = 99
    if ADC_ckbox_1._check_state == True:
        configmsg["ench1"] = 100    
    else:
        configmsg["ench1"] = 99
    if ADC_ckbox_2._check_state == True:
        configmsg["ench2"] = 100   
    else:
        configmsg["ench2"] = 99 
    if ADC_ckbox_3._check_state == True:
        configmsg["ench3"] = 100    
    else:
        configmsg["ench3"] = 99
    if ADC_ckbox_4._check_state == True:
        configmsg["ench4"] = 100    
    else:
        configmsg["ench4"] = 99
    if ADC_ckbox_5._check_state == True:
        configmsg["ench5"] = 100    
    else:
        configmsg["ench5"] = 99
        
    if LED_2_ckbox._check_state == True:
        configmsg["led2"] = 1    
    else:
        configmsg["led2"] = 0
    if IO_07_ckbox._check_state == True:
        configmsg["io7"] = 1   
    else:
        configmsg["io7"] = 0 
    if IO_18_ckbox._check_state == True:
        configmsg["io18"] = 1    
    else:
        configmsg["io18"] = 0
    if IO_19_ckbox._check_state == True:
        configmsg["io19"] = 1    
    else:
        configmsg["io19"] = 0
    configmsg["category"] = "setting"
    config_msg= json.dumps(configmsg)
    config_json_msg = bytes( config_msg, 'utf-8')
    #print(device1["localip"])
    #debug_ip = "172.16.0.214" #'100.64.100.6'
    server.sendto(config_json_msg, (remote_ip, 5044))  
        
    #print("Config message sent!") 
    #print(configmsg)  
###  send config msg ###################################
def slider_command(sliderno,value1):
    global previous_dac
    global previous_delay
    global configmsg
    global device1
    global remote_ip
    previous_dac =0
    previous_delay =0
    if sliderno =="dac":
         if previous_dac != value1:
              configmsg["dac"]=value1
              previous_dac = value1
         else:
            return
    if sliderno =="delay":
          if previous_delay != value1:
              configmsg["delay"]=value1
              previous_delay = value1
          else:
             return
    config_msg= json.dumps(configmsg)
    config_json_msg = bytes( config_msg, 'utf-8')
     #server.sendto(config_json_msg, (device1["localip"] , 5044))    #server.sendto(config_json_msg, ('<broadcast>', 5044))  
     #debug_ip = "172.16.0.214" #debug_ip = '100.64.100.6'
    server.sendto(config_json_msg, (remote_ip, 5044))    #server.sendto(config_json_msg, ('<broadcast>', 5044))
     #textbox.delete("0.0","end")
     #textbox.insert("0.0",value1)
     #textbox.insert("0.0","   sent")
     #print("Config message sent!") 
     #print(configmsg)  
    
    
    
    
# Label Updates
def update_labels(response):
    #print("Labels Refreshed")
    ADC_label_0.configure(text=response['ch0'])
    ADC_label_1.configure(text=response['ch1'])
    ADC_label_2.configure(text=response['ch2'])
    ADC_label_3.configure(text=response['ch3'])
    ADC_label_4.configure(text=response['ch4'])
    ADC_label_5.configure(text=response['ch5'])

if __name__ == '__main__':
    import sys
    main(sys.argv)
