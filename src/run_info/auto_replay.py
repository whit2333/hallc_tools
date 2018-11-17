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

#parser = argparse.ArgumentParser(description='Accumulate run information.')
##parser.add_argument('-k', '--kine', 
##        default = 'DBASE/COIN/auto_standard.kinematics', 
##        help = 'output put for automatic standard.kinematics',
##        dest='kinematics')
#parser.add_argument('-i','--input', 
#        default = 'db2/run_list.json', 
#        help='input json run database',
#        dest='input_file')
#args = parser.parse_args()
#
##print args.kinematics
#print args.input_file
#
#
#cursor_x = 0
#cursor_y = 0
#k = 0


class AutoReplay:
    run_in_progress = 0
    run_number = 0
    last_replay_run_number = 0

    def onRunNumberChange(self, pvname=None, value=None, host=None, **kws):
        self.run_number = int(value)
        print "run number changed {}".format(self.run_number)

    def onRunInProgressChange(self, pvname=None, value=None, host=None, **kws):
        new_value = int(value)
        print "test" 
        if (self.run_in_progress == 1) and (new_value == 0) :
            subprocess.call(
                    ["cd /home/cdaq/hallc-online/hallc_replay_sidis_fall18 && ./run_full_auto_sidis.sh {} &> logs/auto_replay_{}.log & ".format(int(self.run_number),int(self.run_number))],shell=True) 
            self.run_in_progress = new_value
            self.last_replay_run_number = self.run_number
        self.run_in_progress = new_value

    def Print(self):
        print self.run_number
        print self.run_in_progress

    def __init__(self):
        print "derp"
        self.last_replay_run_number = 0
        self.run_number_pv      = PV( 'hcCOINRunNumber')
        self.run_number_pv.add_callback(self.onRunNumberChange)
        self.run_in_progress_pv = PV( 'hcCOINRunInProgress')
        self.run_in_progress_pv.add_callback(self.onRunInProgressChange)
        self.run_number  = self.run_number_pv.get()
        self.run_in_progress = self.run_in_progress_pv.get()

        #run_in_progress_pv.get()

def main():
    auto_replay = AutoReplay()
    auto_replay.Print()
    while True:
        sleep(0.01)


if __name__ == "__main__":
    main()
