#!/bin/bash
export MOB_DIR=/mnt/d/Code/ns3-wave/cross

startTime=(1000 550 400 350 350 350)
simTime=(1200 750 600 550 550 550)
app=("ASApplication" "LowIdApplication" "DCApplication")

# ./waf --run "DCApplication --nNodes=200 --startTime=550 --simTime=750 --interval=1 --traceFile=/home/mak/Code/ns3-wave/cross/mobility10.tcl"

# for((i=0;i<6;i++))
# do
# {
#     suffix=`expr $i \* 5 + 5` 
#     echo $suffix
#     ./waf --run "ASApplication --nNodes=200 --startTime=${startTime[$i]} --simTime=${simTime[$i]} --interval=1 --traceFile=/home/mak/Code/ns3-wave/cross/mobility$suffix.tcl" > log/as$suffix.log 2>&1
# }& 
# done
# wait

# for((i=0;i<6;i++))
# do
# {
#     suffix=`expr $i \* 5 + 5` 
#     echo $suffix
#     ./waf --run "DCApplication --nNodes=200 --startTime=${startTime[$i]} --simTime=${simTime[$i]} --interval=1 --traceFile=/home/mak/Code/ns3-wave/cross/mobility$suffix.tcl" > log/dc$suffix.log 2>&1
# }& 
# done
# wait


# for((i=0;i<6;i++))
# do
# {
#     suffix=`expr $i \* 5 + 5` 
#     echo $suffix
#     ./waf --run "LowIdApplication --nNodes=200 --startTime=${startTime[$i]} --simTime=${simTime[$i]} --interval=1 --traceFile=$MOB_DIR/mobility$suffix.tcl" > log/li$suffix.log 2>&1
# }& 
# done
# wait

for((j=1;j<3;j++))
do
{
    for((i=0;i<6;i++))
    do
    {
        suffix=`expr $i \* 5 + 5` 
        echo "Start: $(date) app=${app[$j]} startTime=${startTime[$i]} simTime=${simTime[$i]} interval=1 traceFile=$MOB_DIR/mobility$suffix.tcl log/${app[$j]}_$suffix.log"

        ./waf --run "${app[$j]} --nNodes=200 --startTime=${startTime[$i]} --simTime=${simTime[$i]} --interval=1 --traceFile=$MOB_DIR/mobility$suffix.tcl" > log/${app[$j]}_$suffix.log 2>&1

        echo "End: $(date) app=${app[$j]} startTime=${startTime[$i]} simTime=${simTime[$i]} interval=1 traceFile=$MOB_DIR/mobility$suffix.tcl log/${app[$j]}_$suffix.log"
    }& 
    done
    wait
}
done