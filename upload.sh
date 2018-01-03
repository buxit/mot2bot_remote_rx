#!/bin/sh
bp=$(pwd)/build
~/arduino-1.8.5/arduino --pref build.path=${bp} --verbose --port /dev/ttyUSB0 --board arduino:avr:uno --upload mot2bot_remote_rx.ino
echo
