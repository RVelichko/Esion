#!/bin/bash
/home/rostislav/bin/esion-server &
PID=$!
echo $PID > /var/run/esion-srv.pid
