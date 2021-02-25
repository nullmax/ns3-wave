#!/bin/bash

# clean up
rm -rf cross*/
rm trace*.xml
rm *.tcl
echo "clean up"

export SUMO_HOME=/usr/share/sumo

for((i=0;i<6;i++))
do
{
    suffix=`expr $i \* 5 + 5` 
    echo $suffix
    mkdir cross$suffix
    cp cross.net.xml cross$suffix/
    cp cross.sumocfg cross$suffix/
    ./build/GenVehicle $i > cross$suffix/cross.rou.xml
    sumo -c cross$suffix/cross.sumocfg --fcd-output trace$suffix.xml
    python $SUMO_HOME/tools/traceExporter.py -i trace$suffix.xml --ns2mobility-output=mobility$suffix.tcl
} 
done

wait