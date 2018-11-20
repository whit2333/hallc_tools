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
        #required = True,
        help = 'the run to add a comment',
        dest='run')
parser.add_argument('-a','--append', 
        action = "store_true",
        help='append to existing comments',
        dest='append')
parser.add_argument('-u','--update', 
        action = "store_true",
        help='update run file')
parser.add_argument('-f', '--file', 
        default = 'db2/run_list_extra.json',
        help = 'The json run file to add a comment in',
        dest='file')
parser.add_argument('-c', '--comment', 
        help = 'specify the comment on the commandline',
        default = None,
        dest='comment')
parser.add_argument('-k', '--kine', 
        help = 'specify the kinematic setting',
        default = None,
        dest='kinematic')
args = parser.parse_args()

#@print args.run
#@print args.append

target_mass_amu = {"2":1.00794,"3":2.014101, "5":26.92} 
target_desc     = {"2":"LH2",  "3": "LD2",   "5":"DUMMY"}

def get_json(filename):
    the_json = {}
    if os.path.isfile(args.file) :
        with open(args.file) as data_file:
            the_json = json.loads(data_file.read())
    return the_json

def main():
    """ """
    if not args.comment :
        comment = raw_input(str("Comment for run {} :").format(args.run))
    else : 
        comment = str(args.comment)
    

    the_json = {}
    if os.path.isfile(args.file) :
        with open(args.file) as data_file:
            the_json = json.loads(data_file.read())

    if str(args.run) in the_json.keys(): 
        old_comment = None
        if "comment" in the_json[str(args.run)].keys(): 
            old_comment =  the_json[str(args.run)]["comment"]
            #print old_comment
        if args.append and old_comment:
            comment = old_comment + ", " + comment
        the_json[str(args.run)]["comment"] = comment
    else :
        the_json.update({str(args.run) : {"comment": comment}})

    if args.kinematic :
        kinematic = str(args.kinematic)
        the_json[str(args.run)]["kinematic"] = kinematic
    #print  the_json
    #update(the_json, {"comment": comment})
    #with open(out_file_name,'w') as data_file:
    #    json.dump(the_json,data_file)
    #print json.dumps(the_json, sort_keys=True, ensure_ascii=False, indent=2)
    #jif os.path.isdir(os.path.basename(args.file)) :
    #if os.path.isfile(args.file) and not args.update:
    if os.path.exists(args.file) and not args.update:
        print "Cannot overwrite existing file: {}".format(args.file)
        print "use -u to  update existing file "
    elif not os.path.exists(os.path.dirname(args.file)):
        print "Cannot create file: {}".format(args.file)
        print "Directory {} does not exist".format(os.path.dirname(args.file))
    elif not os.path.isfile(args.file) or args.update:
        with open(args.file, 'w') as f:
            f.write(json.dumps(the_json, sort_keys=True, ensure_ascii=False, indent=2))
    else : 
        print "aborted"
    #            print "Writing to output file : {}".format(args.file)
    #run_list.current_run.PrintJSON()


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print 'aborted.'
        sys.exit(0)




