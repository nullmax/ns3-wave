#!/bin/bash
app=("ASApplication" "LowIdApplication" "DCApplication")

for((i=0;i<6;i++))
do
{
    suffix=`expr $i \* 5 + 5` 
    echo $suffix
    echo ""
    tail -n 6 log/${app[0]}_$suffix.log
    echo ""
    tail -n 5 log/${app[1]}_$suffix.log
    echo ""
    tail -n 4 log/${app[2]}_$suffix.log
} 
done
wait