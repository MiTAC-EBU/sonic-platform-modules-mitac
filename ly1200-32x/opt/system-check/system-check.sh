#!/bin/bash

SW_READY_STAMP='/tmp/.BcmSdkReady'
SYSTEM_INIT=0
CURRENTR_LED=0

while [ true ]
do
	if [ "$SYSTEM_INIT" -eq "0" ]; then
		if [  `docker ps |grep -c syncd` -eq "1" ]; then
			SYSTEM_INIT=1
			# set SYSTEM LED to Green
			echo 0x01 > /sys/bus/i2c/devices/1-0032/system_led_fld
			CURRENTR_LED=1
		fi
	else
		if [ `docker ps |grep -c syncd` -eq "0" ]; then
			# set SYSTEM LED to Amber
			if [ "$CURRENTR_LED" -ne "2" ]; then
				echo 0x02 > /sys/bus/i2c/devices/1-0032/system_led_fld
				CURRENTR_LED=2
			fi
		else
			# set SYS LED to Green
			if [ "$CURRENTR_LED" -ne "1" ]; then
				echo 0x01 > /sys/bus/i2c/devices/1-0032/system_led_fld
				CURRENTR_LED=1
			fi
		fi
	fi
	sleep 10
done

exit 0
