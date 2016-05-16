#!/bin/bash

# full volume
amixer cset numid=1 -- 100%
# setup all GPIO pins. change pin numbers to your need
sudo bash -c "echo 20 > /sys/class/gpio/export"
sudo bash -c "echo 21 > /sys/class/gpio/export"
sudo bash -c "echo out > /sys/class/gpio/gpio21/direction"
sudo bash -c "echo 1 > /sys/class/gpio/gpio21/value"
GET="cat /sys/class/gpio/gpio20/value"

# activate these lines to prevent screen from blanking
#xset s off
#xset -dpms
#xset s noblank

# start with mode off
echo 1 > mode
# run web server
socat TCP4-LISTEN:8080,fork EXEC:./webserver.sh&
sleep 1s
# run web browser to display interface
midori -e Fullscreen -a http://localhost:8080&

# state machine
while true; do
mode="`cat mode`"
if [ $mode = 1 ]; then kill $player; fi
if [ $mode = 2 ]; then kill $player; if [ "`$GET`" = 0 ]; then mplayer door.wav&player=$!; echo 21 > mode; fi; fi
if [ $mode = 21 ]; then if ! kill -0 $player; then echo 22 > mode ; fi; fi
if [ $mode = 22 ]; then if [ "`$GET`" = 1 ]; then echo 2 > mode; fi; fi
if [ $mode = 3 ]; then kill $player; sleep 60 &player=$!; echo 31 > mode ; fi
if [ $mode = 31 ]; then if ! kill -0 $player; then if [ "`$GET`" = 0 ]; then mplayer door.wav&player=$!; echo 32 > mode; fi; fi; fi
if [ $mode = 32 ]; then if ! kill -0 $player; then sleep 60 &player=$!; echo 33 > mode ; fi; fi
if [ $mode = 33 ]; then if ! kill -0 $player; then mplayer alarm.wav -loop 100 &player=$!; echo 34 > mode ; fi; fi
if [ $mode = 4 ]; then kill $player; if [ "`$GET`" = 0 ]; then mplayer alarm.wav -loop 100 &player=$!; echo 41 > mode; fi; fi
if [ $mode = 5 ]; then kill $player; mplayer baby.wav -softvol -volume 50 &player=$!; echo 51 > mode; fi

sleep 0.1
done
