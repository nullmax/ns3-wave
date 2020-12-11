sumo -c quickstart.sumocfg --fcd-output trace.xml
python $SUMO_HOME/tools/traceExporter.py -i trace.xml --ns2mobility-output=mobility.tcl