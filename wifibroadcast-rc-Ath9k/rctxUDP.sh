#!/bin/bash

ChannelBandSwitcher=$1
ChannelIPCamera=$2
IsBandSwitcherEnabled=$3
IsIPCameraSwitcherEnabled=$4
IsEncrypt=$5
ChannelToSwitchDownlink=$6
WlanName=$7

while true; do

    nice -n -5 /home/pi/wifibroadcast-rc-Ath9k/rctxUDP_IN $ChannelBandSwitcher $ChannelIPCamera $IsBandSwitcherEnabled $IsIPCameraSwitcherEnabled $IsEncrypt  $ChannelToSwitchDownlink $WlanName
    sleep 1

done
