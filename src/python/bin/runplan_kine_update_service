#!/usr/bin/env python
# encoding: utf-8

import sys, os
import curses
#import epics
from epics import caget, caput, camonitor, PV
import epics
import threading
from time import sleep
from subprocess import call
import subprocess

import argparse

parser = argparse.ArgumentParser(description='Accumulate run information.')
#parser.add_argument('-k', '--kine', 
#        default = 'DBASE/COIN/auto_standard.kinematics', 
#        help = 'output put for automatic standard.kinematics',
#        dest='kinematics')
parser.add_argument('-i','--input', 
        default = 'db2/run_list.json', 
        help='input json run database',
        dest='input_file')
args = parser.parse_args()

kine_setting_num_pv   = PV('hcKinematicSettingNumber')
kine_setting_group_pv = PV('hcKinematicSettingGroup')
kine_setting_id_pv    = PV('hcKinematicSettingID')
run_in_progress_pv    = PV('hcCOINRunInProgress')

def update_epics_kinematics(pvname=None, value=None, host=None, **kws):
    p = subprocess.Popen(['bash','/group/c-csv/cdaq/csv_run_plan/bin/get_current_setting.sh'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p.communicate()
    #print out
    values = out.split()
    epics.caput('hcKinematicSettingGroup', int(values[0]))
    epics.caput('hcKinematicSettingNumber',int(values[1]))
    epics.caput('hcKinematicSettingID',    int(values[2]))
    print values[0]
    print values[1]
    print values[2]
    sleep(1)
    print kine_setting_group_pv.get()
    print kine_setting_num_pv.get()
    print kine_setting_id_pv.get()

def main():
    run_in_progress_pv.add_callback(update_epics_kinematics)
    try:
        while True:
            sleep(0.01)
    except KeyboardInterrupt:
        """user wants control back"""
        print "done"

if __name__ == "__main__":
    main()

#         subprocess.call(
#                   ["cd /home/cdaq/hallc-online/hallc_replay_sidis_fall18 && ./run_full_auto_sidis.sh {} &> logs/auto_replay_{}.log & ".format(int(self.run_number),int(self.run_number))],shell=True) 
#            self.run_in_progress = new_value


