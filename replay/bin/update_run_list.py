#!/usr/bin/python
from epics import caget, caput, camonitor, PV
import epics
import time
import subprocess
import json
import sys
#from multiprocessing import Pool
import copy
import os.path
import threading

import argparse

parser = argparse.ArgumentParser(description='Accumulate run information.')
parser.add_argument('-r', '--run', 
        default = int(caget("hcCOINRunNumber")),
        help = 'the run to update',
        dest='run')
parser.add_argument('-i', '--input-file', 
        default = 'db2/run_list.json',
        help = 'The json run list file',
        dest='input_file')
parser.add_argument('-o', '--output-file', 
        default = 'db2/run_list_extra.json',
        help = 'The json run file to add a comment in',
        dest='output_file')
args = parser.parse_args()

#@print args.run
#@print args.append

target_mass_amu = {"2":1.00794,"3":2.014101, "5":26.92} 
target_desc     = {"2":"LH2",  "3": "LD2",   "5":"DUMMY"}

def get_json(filename):
    the_json = {}
    if not os.path.exists(os.path.dirname(os.path.abspath(filename))):
        print "Cannot create file: {}".format(filename)
        print "Directory {} does not exist".format(os.path.dirname(filename))
        sys.exit( "aborted")
    if os.path.isfile(filename) :
        with open(filename) as data_file:
            the_json = json.loads(data_file.read())
    else:
        print "error: input run list, {}, does not exist ".format(filename)
        sys.exit( "aborted")
    return the_json

def get_run_json(filename, run_list):
    the_json = get_json(filename)

def main():
    """ """

    the_json = get_json(str(args.input_file))

    spec_setting   = None
    target_setting = None

    if str(args.run) in the_json.keys(): 
        if "spectrometers" in the_json[str(args.run)].keys(): 
            spec_setting =  the_json[str(args.run)]["spectrometers"]
        if "target" in the_json[str(args.run)].keys(): 
            target_setting =  the_json[str(args.run)]["target"]
    else :
        print "Run not found in run list, {}. Aborting. ".format(str(args.input_file))
        sys.exit( "aborted")

    runable= ["bash","bin/get_setting.sh",
        "-p", str(spec_setting["hms_momentum"]),  
        "-t", str(spec_setting["hms_angle"]), 
        "-P", str(spec_setting["shms_momentum"]), 
        "-T", str(spec_setting["shms_angle"]),
        "-b", str(target_setting["target_id"])
        ]
    #print runable

    print os.path.dirname(os.path.abspath(args.output_file))
    kinematics = str(subprocess.check_output(runable)).split()
    print kinematics

    if not (len(kinematics) is 3) :
        print "error: kinematics list is wrong size. Aborting."
        sys.exit( "aborted")

    if not os.path.isdir(os.path.dirname(os.path.abspath(args.output_file))):
        print "Cannot create file: {}".format(args.output_file)
        print "Directory {} does not exist".format(os.path.dirname(args.output_file))
        print "derp"
        return
    else:
        extra_json = get_json(str(args.output_file))
        with open(args.output_file, 'w') as f:
            f.write(json.dumps(the_json, sort_keys=True, ensure_ascii=False, indent=2))
    #            print "Writing to output file : {}".format(args.file)
    #run_list.current_run.PrintJSON()
    #elif os.path.exists(args.output_file):
    #    print "Cannot overwrite existing file: {}".format(args.file)
    #    print "use -u to  update existing file "


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print 'aborted.'
        sys.exit(0)




