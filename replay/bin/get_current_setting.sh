#/bin/bash

hms_p=$(caget hcHMSMomentum  | sed 's/hcHMSMomentum//')
shms_p=$(caget hcSHMSMomentum  | sed 's/hcSHMSMomentum//')
hms_th=$(caget hcHMSCorrectedAngle  | sed 's/hcHMSCorrectedAngle//')
shms_th=$(caget hcSHMSCorrectedAngle  | sed 's/hcSHMSCorrectedAngle//')
targ=$(caget hcBDSSELECT  | sed 's/hcBDSSELECT//')
root -l -b -q "/group/c-csv/cdaq/csv_run_plan/run_plan/src/master_settings.cxx(${hms_p},${hms_th},${shms_p},${shms_th},${targ})" 2> /dev/null | tail -n 1  | sed 's/ / /'
