#!/bin/bash
export NS3_DIR="/mnt/d/Code/ns-3-allinone/ns-3.30"
cp -r ./ASApplication $NS3_DIR/scratch/ 
cp -r ./DCApplication $NS3_DIR/scratch/ 
cp -r ./LowIdApplication $NS3_DIR/scratch/ 
cp ./run_app.sh $NS3_DIR/ 
cp ./show_result.sh $NS3_DIR/ 