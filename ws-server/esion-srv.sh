#!/bin/bash
/home/rostislav/bin/esion-server &
PID=$!
echo $PID > esion-srv.pid
