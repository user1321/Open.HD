from time import sleep
import RPi.GPIO as gp
import os
import signal
import sys
import psutil
import pyudev
import argparse
import socket
import struct
import threading
lock = threading.Lock()


import configparser
import binascii
from itertools import chain
import os
import sys
import fileinput
import socket
import re

import shlex, subprocess
from subprocess import call
from tempfile import mkstemp
from shutil import move
from os import fdopen, remove
from sh import mount
from sh import umount
import random
import string

parser = argparse.ArgumentParser()
parser.add_argument("-SaveVideoOnUSBAfter", type=int, help="")
parser.add_argument("-RTPSaveToUSBUrl")

args = parser.parse_args()

############################# Assigning values ...
SaveVideoOnUSBAfter = args.SaveVideoOnUSBAfter
RTPSaveToUSBUrl = args.RTPSaveToUSBUrl

print("RTPSaveToUSBUrl: " + RTPSaveToUSBUrl)

mountpoint = "/mnt/usbdisk/"
FreeSpaceFilePath = "/tmp/usbdrivefreespace"
IsRecordingFilePath = "/tmp/isrecording"
State = 0
isLowDiskSpace = False
isUSBDiskIsNotAvaliable = False
DiskSpaceLimit = 400
MountDevName = ""
hMonitorThread = ""
IsMonitorThreadRunning = False
isRecording = False
GstHandler = 0

print("SaveVideoOnUSBAfter: ", SaveVideoOnUSBAfter)


def GetNextFileName(mountpoint):
    result = "GetNextFileNameError"
    try:
        i = 0
        while os.path.exists(mountpoint + "video_%s.avi" % i):
            i += 1
        result = "video_" + str(i) + ".avi"
    except Exception as e:
        print("SaveRTPToUSB exception in GetNextFileName function: " + str(e) ) 
    return result


def WriteToFile(filename,str):
    try:
        f = open(filename, "w")
        f.write(str)
        f.close()
    except Exception as e:
        print("SaveRTPToUSB exception in WriteToFile function: " + str(e) )

def StartRecordRTP(pipeline):
    global isUSBDiskIsNotAvaliable
    global isLowDiskSpace
    global hMonitorThread
    global IsMonitorThreadRunning
    global MountDevName
    global isRecording
    global GstHandler

    print("StartRecordRTP")
    usbdev = GetUSBDevName()
    try:
        if usbdev is not None:
            print("USB disk: " + usbdev)
            MountDevName = usbdev
            if CheckIfPathIsMount(mountpoint) == False:
                print("Trying to mount usb disk...")
                Mount(usbdev)
            if CheckIfPathIsMount(mountpoint) == True:
                print("USB disk is mounted to: " + mountpoint )
                FreeSpaceInMB = CheckFreeSpace(mountpoint)
                WriteToFile(FreeSpaceFilePath, str(FreeSpaceInMB) )
                if FreeSpaceInMB < DiskSpaceLimit:
                    print("Free disk space on usb drive is less than required. Free space: " + str(FreeSpaceInMB) + " Required: " + str(DiskSpaceLimit) )
                    isLowDiskSpace = True
                    return None
                else:
                    print("Free disk space: " + str(FreeSpaceInMB) + " MB")
                    NextFileName = GetNextFileName(mountpoint)
                    pipeline = pipeline + NextFileName
                    print("Start video using: " + pipeline)
                    args = shlex.split(pipeline)
                    GstHandler = subprocess.Popen(  args )
                    isRecording = True
                    WriteToFile(IsRecordingFilePath, "1" )
                    if IsMonitorThreadRunning == False:
                        hMonitorThread = threading.Thread(target=MonitorThread)
                        hMonitorThread.daemon = True
                        hMonitorThread.start()
                        IsMonitorThreadRunning = True

            else:
                print("Cant mount usb disk: " + usbdev + " to: " + mountpoint)
                isUSBDiskIsNotAvaliable = True
        else:
            print("Cant find USB disk")
            isUSBDiskIsNotAvaliable = True
    except Exception as e:
        isUSBDiskIsNotAvaliable = True
        print("SaveRTPToUSB exception in StartRecordRTP function: " + str(e) )

def StopRecordRTP():
    global isRecording
    print("StopRecordRTP")
    isRecording = False
    WriteToFile(IsRecordingFilePath, "0" )
    try:
        isTerminated = False

        while isTerminated == False:
            print("trying to stop gst-launch-1.0")
            #GstHandler.terminate()
            GstHandler.send_signal(signal.SIGINT)
            sleep(1)
            res = GstHandler.poll()
            if res is not None:
                print("Terminated.")
                isTerminated = True

    except Exception as e:
        print("SaveRTPToUSB: exception in stopRecordRTP function: " +  + str(e) )
  
    Umount(MountDevName)

def StartRecvUDP():
    global State
    UDP_IP = ""
    UDP_PORT = 1256
    try:
        SocketBand = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
        SocketBand.bind((UDP_IP, UDP_PORT))
    except Exception as e:
        print("Except: Bind to port 1256 " + str(e) )
        exit(0)

    while True:
        d = SocketBand.recvfrom(2)
        try:
            #RecvStr = data.decode('ascii')
            #RecvPWM = int(RecvStr)
            data = d[0]
            RecvPWM = struct.unpack("H", data)[0]
            if RecvPWM >= SaveVideoOnUSBAfter  and State == 0 and isLowDiskSpace == False and isUSBDiskIsNotAvaliable == False:
                print("Turn On command received. Trying to turn on recording function...")
                State=1
                StartRecordRTP(RTPSaveToUSBUrl)
            if RecvPWM < SaveVideoOnUSBAfter and State == 1:
                print("Turn Off command received. Trying to turn off recording function...")
                State=0
                StopRecordRTP()

        except Exception as e:
            print("SaveRTPToUSB: Cant convert incoming UDP to int: " + str(e) )
            continue


def GetUSBDevName():
    result = ""
    try:
        context = pyudev.Context()
        removable = [device for device in context.list_devices(subsystem='block',DEVTYPE='disk') if device.attributes.asstring('removable') == "1"]
        for device in removable:
            partitions = [device.device_node for device in context.list_devices(subsystem='block', DEVTYPE='partition', parent=device)]
            ##print("All removable :  {} ".format(",  ".join(partitions)))
            if len(partitions)  >= 1:
                result = partitions[0]
                return result
            else:
                return None
    except Exception as e:
        print("SaveRTPToUSB detect USB drive exception: " + str(e) )
        return None
    return None

def Mount(devname):
    #mount("-t ntfs", devname, mountpoint )
    os.system("/bin/mount" + " -t ext2 --rw " + devname + " " +  mountpoint)
 
def Umount(devname):
    
    try:
        umount(devname)
    except Exception as e:
        print("SaveRTPToUSB. exception in Umount function:  " + str(e) )

def CheckIfPathIsMount(dirname):
    result = False
    try:
        result = os.path.ismount(dirname)
    except Exception as e:
         print("SaveRTPToUSB: Checking USB mount point " + str(e) )
    return result

def CheckFreeSpace(mountpoint):
    result = 0
    try:
        IsMounted = CheckIfPathIsMount(mountpoint)
        if IsMounted  == True:
            stats = psutil.cpu_percent(percpu=True)
            mem = psutil.virtual_memory()
            disk = psutil.disk_usage(mountpoint)
            DiskFreeInMB = disk.free
            DiskFreeInMB = DiskFreeInMB / 1048576
            result = DiskFreeInMB
        else:
            print("USB disk is not mounted")

    except Exception as e:
        print(e)
        return result
    return result


def MonitorThread():
    global isLowDiskSpac
    global isUSBDiskIsNotAvaliable

    print("Monitor thread started")
    while True:
        usbdev = GetUSBDevName()
        try:
            if usbdev is not None:
                isUSBDiskIsNotAvaliable = False
                if CheckIfPathIsMount(mountpoint) == True:
                    FreeSpaceInMB = CheckFreeSpace(mountpoint)
                    WriteToFile(FreeSpaceFilePath, str(FreeSpaceInMB) )
                    if FreeSpaceInMB < DiskSpaceLimit:
                        isLowDiskSpace = True
                        if isRecording == True:
                            StopRecordRTP()
                    else:
                        isLowDiskSpace = False

            else:
                isUSBDiskIsNotAvaliable = True
                if isRecording == True:
                    StopRecordRTP()

        except Exception as e:
             print("SaveRTPToUSB. Monitor thread exception: " + str(e) )

        sleep(2)

    print("Monitor thread ended")

hRecvThread = threading.Thread(target=StartRecvUDP)
hRecvThread.daemon = True
hRecvThread.start()


#mountpoint = "/mnt/usbdisk"
#CheckFreeSpace(mountpoint)
#StartRecvUDP()

while True:
    #State=0 - nothing is received - do nothing. Or Camera is off.
    #State=1 - Turn Off camera.

    sleep(1)
