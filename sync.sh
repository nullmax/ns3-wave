#!/bin/bash
export NS3_DIR="/mnt/d/Code/ns-3-allinone/ns-3.30"
cp -r $NS3_DIR/scratch/ASApplication ./
cp -r $NS3_DIR/scratch/DCApplication ./
cp -r $NS3_DIR/scratch/LowIdApplication ./
cp $NS3_DIR/run_app.sh ./
cp $NS3_DIR/show_result.sh ./
cp $NS3_DIR/rwaf.sh ./