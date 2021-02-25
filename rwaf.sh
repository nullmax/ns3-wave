#!/bin/bash
export MOB_DIR=/mnt/d/Code/ns3-wave/cross

startTime=(1000 550 400 350 350 350)
simTime=(1200 750 600 550 550 550)
app=("ASApplication" "LowIdApplication" "DCApplication")

app_i=$1
i=$2
suffix=`expr $i \* 5 + 5` 
echo "$(date) app=${app[$app_i]} startTime=${startTime[$i]} simTime=${simTime[$i]} interval=1 traceFile=$MOB_DIR/mobility$suffix.tcl log/${app[$app_i]}_$suffix.log"

./waf --run "${app[$app_i]} --nNodes=200 --startTime=${startTime[$i]} --simTime=${simTime[$i]} --interval=1 --traceFile=$MOB_DIR/mobility$suffix.tcl" 
# > log/${app[$app_i]}_$suffix.log 2>&1

echo "$(date) app=${app[$app_i]} startTime=${startTime[$i]} simTime=${simTime[$i]} interval=1 traceFile=$MOB_DIR/mobility$suffix.tcl log/${app[$app_i]}_$suffix.log"