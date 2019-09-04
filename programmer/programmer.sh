#!/bin/bash
/home/pi/bin/programmer.py &
PID=$!
echo $PID > programmer.pid
