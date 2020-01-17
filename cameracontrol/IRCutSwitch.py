from time import sleep
import RPi.GPIO as gp
import os
import signal
import sys

import argparse
import socket
import struct

import configparser
import binascii
import socket
import re



parser = argparse.ArgumentParser()
parser.add_argument("-OpenIRCutAfter", type=int, help="")
parser.add_argument("-IsBCM", type=int, help="")
parser.add_argument("-EN1Pin", type=int, help="")
parser.add_argument("-IN1Pin", type=int, help="")
parser.add_argument("-IN2Pin", type=int, help="")



args = parser.parse_args()
############################# Assigning values ...
OpenIRCutAfter = args.OpenIRCutAfter
IsBCM = args.IsBCM
EN1Pin = args.EN1Pin
IN1Pin = args.IN1Pin
IN2Pin = args.IN2Pin

if IsBCM != 0 and IsBCM  != 1:
    print("IsBCM must be set to 1 or 0")
    sys.exit(1)

State = 0

if IsBCM == 1:
    gp.setmode(gp.BCM)
else:
    gp.setmode(gp.BOARD)

gp.setup(EN1Pin, gp.OUT)
gp.setup(IN1Pin, gp.OUT)
gp.setup(IN2Pin, gp.OUT)



def DisableOutput():
    gp.output(EN1Pin, False) #EN1
    gp.output(IN1Pin, False) #IN1
    gp.output(IN2Pin, False) #IN2

DisableOutput()

def OpenIRCut():
    print("OpenIRCut")
    gp.output(EN1Pin, False) #EN1
    gp.output(IN1Pin, True) #IN1
    gp.output(IN2Pin, False) #IN2
    gp.output(EN1Pin, True) #EN1
    sleep(0.7)
    DisableOutput()

def CloseIRCut():
    print("CloseIRCut")
    gp.output(EN1Pin, False) #EN1
    gp.output(IN1Pin, False) #IN1
    gp.output(IN2Pin, True) #IN2
    gp.output(EN1Pin, True) #EN1
    sleep(0.7)
    DisableOutput()

def StartRecvUDP():
    global State
    UDP_IP = ""
    UDP_PORT = 1255
    try:
        SocketBand = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
        SocketBand.bind((UDP_IP, UDP_PORT))
    except Exception as e:
        print("IRCutSwitch. Except: Bind to port 1255 " + str(e) )
        exit(0)

    while True:
        d = SocketBand.recvfrom(2)
        try:
            data = d[0]
            RecvPWM = struct.unpack("H", data)[0]
            print("RecvPWM = " + str(RecvPWM) + " state = " + str(State) ) 
            if RecvPWM >= OpenIRCutAfter  and State == 0:
                print("OpenIRCutAfter: Turn On command received.")
                State=1
                OpenIRCut()
            if RecvPWM < OpenIRCutAfter and State == 1:
                print("OpenIRCutAfter: Turn Off command received.")
                State=0
                CloseIRCut()

        except Exception as e:
            print("IRCutSwitch.py: Cant convert incoming UDP to int: " + str(e) )
            continue


StartRecvUDP()
print("Something was wrong. End.")
