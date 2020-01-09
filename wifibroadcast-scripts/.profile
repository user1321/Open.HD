# Change to script folder and execute main entry point
TTY=`tty`


case $TTY in
    /dev/tty1) # TX/RX
	echo "tty1"
	service ssh start

	mkdir /tmp/ath9k_htc
	ln -s /tmp/ath9k_htc /lib/firmware/

	/usr/bin/python /root/wifibroadcast_misc/gpio-AirForceWiFiOnly.py

	grepIsForceToWiFiOnlyMode=$?
	if [[ $grepIsForceToWiFiOnlyMode -eq 1 ]] ; then
		ifconfig eth0 192.168.0.215 up
		echo "Force to WiFi only mode via GPIO 5"
	
		cp /home/pi/Ath9kFirmware/original/htc_9271-1.4.0.fw /tmp/ath9k_htc/htc_9271-1.4.0.fw

		#IsAth9kAvaliable=$(lsmod | grep ath9k_htc)

		#if [[ $IsAth9kAvaliable != "" ]] ; then
		#	echo "Ath9k detected. Reload driver with low TX power"
			#rmmod ath9k ath9k_htc ath9k_common ath9k_hw ath
			#sleep 2
			modprobe ath9k_htc -C /etc/modprobe.d/original/
			sleep 1
		#fi

		DriveName=$(/usr/bin/python /root/wifibroadcast_misc/GetUSBDriveName.py)
		
		if [[ $DriveName != "None" ]] ; then
			echo "USB drive: $DriveName detected"
			mount -t ext2 $DriveName /mnt/usbdisk/
		fi

		ip link set wlan0 name wifihotspot0
		ionice -c 3 nice dos2unix -n /boot/apconfig2.txt /tmp/apconfig.txt

		ifconfig wifihotspot0 192.168.2.1 up
		#Configure samba
		mkdir /tmp/samba
		mkdir /tmp/samba/run/
		mkdir /tmp/samba/run/samba
		mkdir /tmp/samba/spool
		mkdir /tmp/samba/spool/samba
#
		cp -R /var/lib/samba_real /tmp/samba/lib-samba
		mkdir /tmp/samba/lib-samba/private
		mkdir /tmp/samba/cache-samba
		mkdir /tmp/samba/log-samba
		sudo ln -s /tmp/samba/log-samba /var/log/samba
#
#		service smbd restart
#		/usr/sbin/smbd -D -s /etc/samba/smb.conf

		/usr/sbin/dnsmasq --conf-file=/etc/dnsmasqWifi.conf
		nice -n 5 hostapd -B /tmp/apconfig.txt 

		/root/wifibroadcast_misc/samba.sh &
		sleep 365d
	else
		cp /home/pi/Ath9kFirmware/mod/htc_9271-1.4.0.fw /tmp/ath9k_htc/htc_9271-1.4.0.fw
		#rmmod ath9k ath9k_htc ath9k_common ath9k_hw ath
		#sleep 2
		modprobe ath9k_htc
	fi



	python /root/wifibroadcast_misc/gpio-IsAir.py
	
	i2cdetect -y 1 | grep  "70: 70"
        grepRet=$?
        if [[ $grepRet -eq 0 ]] ; then
            /usr/bin/python3.5 /home/pi/cameracontrol/InitArduCamV21Ch1.py
        fi
	
	CAM=`/usr/bin/vcgencmd get_camera | nice grep -c detected=1`
	
        i2cdetect -y 0 | grep  "30: -- -- -- -- -- -- -- -- -- -- -- 3b -- -- -- --"
        grepRet=$?
	killall omxplayer  > /dev/null 2>/dev/null
	killall omxplayer.bin  > /dev/null 2>/dev/null
        if [[ $grepRet -eq 0 ]] ; then
			/usr/bin/python3.5 /home/pi/RemoteSettings/Air/RemoteSettingSyncAir.py
			CAM="1"
			echo "0" > /tmp/ReadyToGo
        else
			if [ -e /tmp/Air ]; then
				echo "force boot as Air via GPIO"
				CAM="1"
			fi
			
			if [ "$CAM" == "0" ]; then # if we are RX ...
				/home/pi/RemoteSettings/Ground/helper/AirRSSI.sh &
				/home/pi/RemoteSettings/Ground/helper/DisplayProgram/DisplayProgram &
				/home/pi/RemoteSettings/Ground/helper/ConfigureNics.sh
				retCode=$?
				if [ $retCode == 1 ]; then
					# Ret Code is 1. joystick selected as control
					/usr/bin/python3.5 /home/pi/RemoteSettings/Ground/RemoteSettingsSync.py -ControlVia joystick
				fi

				if [ $retCode == 2 ]; then
					# Ret code is 2  GPIO  selected as control
					/usr/bin/python3.5 /home/pi/RemoteSettings/Ground/RemoteSettingsSync.py -ControlVia GPIO
				fi
				killall omxplayer  > /dev/null 2>/dev/null
				killall omxplayer.bin  > /dev/null 2>/dev/null
				/usr/bin/omxplayer --loop /home/pi/RemoteSettings/Ground/helper/DisplayProgram/video/AfterSSync.mp4 > /dev/null 2>/dev/null &
	
				echo "0" > /tmp/ReadyToGo
			else # else we are TX ...
				/usr/bin/python3.5 /home/pi/RemoteSettings/Air/RemoteSettingSyncAir.py
				echo "0" > /tmp/ReadyToGo
			fi
		fi


;;

esac


while [ ! -f /tmp/ReadyToGo ]
do
  echo "sleep..."
  sleep 1
done
echo "go..."

cd /home/pi/wifibroadcast-scripts
source main.sh
