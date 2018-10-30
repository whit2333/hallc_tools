#!/usr/bin/python
from epics import caget, caput, camonitor, PV
import time
import subprocess
from optparse import OptionParser
import json
import sys
#from multiprocessing import Pool
import copy
import os.path
import threading

#latest_run_number = subprocess.check_output(['latest_run'])

target_mass_amu = {"1":2.014101, "2":26.92,  "3":1.00794} 
target_desc   = {"2":"LH2", "3": "LD2", "5":"DUMMY"}


class Trip:
    """Simple trip data"""
    def __init__(self,num=0):
        self.num = num
        self.start_time = time.ctime()
        self.end_time = None
        self.pv_name = "IBC1H04CRCUR2"
        self.is_tripped = False
    def stop(self):
        self.end_time = time.ctime()
    def start(self):
        self.start_time = time.ctime()
        if self.is_tripped :
            self.stop()
    def GetJSONObject(self):
        return {"id":self.num, "start_time":self.start_time, "end_time":self.end_time}

class PVIntegrator:
    """PVIntegrator"""
    def __init__(self,pv = "IBC1H04CRCUR2",name=None,outpv=None):
        self.output_name = outpv
        if self.output_name != None :
            self.output_pv = PV(self.output_name)
        #else :
        #    self.output_pv  = None
        self.total = 0.0
        self.name = name
        if name is None:
            self.name = pv
        self.pv_name = pv
        self.process_var =  PV(self.pv_name)

    def Reset(self):
        self.total = 0.0

    def PVChangeCallback(self,pvname=None, value=None, char_value=None, **kws):
        value = float(char_value)
        self.total = self.total + 2.0*value # 2.0 s readout 

    def AddCallback(self):
        self.process_var.add_callback(self.PVChangeCallback)

    def ClearCallback(self):
        self.process_var.clear_callbacks()

    def Dump(self):
        print str("total charge from {} = {} mC").format(self.pv_name, self.total*0.001)

    def Print(self):
        self.ClearCallback()
        print str("total charge from {} = {} mC").format(self.pv_name, self.total*0.001)

    def GetJSONObject(self):
        self.ClearCallback()
        trip_dict = {'total': float(self.total)}
        trip_dict.update({'PV': self.pv_name})
        return {self.name: trip_dict }

    def PrintJSON(self):
        print json.dumps(self.GetJSONObject(), sort_keys=True, indent=2, separators=(',', ': '))

class TripCounter:
    """Trip Counter"""
    def __init__(self,pv = "IBC1H04CRCUR2",th=7.6,name=None):
        self.name = name
        if name is None:
            self.name = pv
        self.trip_count = 0
        self.currently_tripped = False
        self.trip_threshold = th
        self.pv_name = pv
        self.process_var =  PV(self.pv_name)
        self.trips = []
        self.latest_trip = None

    def Reset(self):
        self.trip_count = 0
        self.currently_tripped = False
        self.trips = []
        self.latest_trip = None

    def PVChangeCallback(self,pvname=None, value=None, char_value=None, **kws):
        """Call back for epics PV."""
        #self.trip_count
        #self.trip_threshold
        #self.currently_tripped
        value = float(char_value)
        #print 'PV :', pvname, char_value, time.ctime(), " thresh ", self.trip_threshold
        if (value < self.trip_threshold) and not self.currently_tripped: 
            self.latest_trip = Trip(len(self.trips))
            print 'PV tripped!', pvname, char_value, time.ctime()
            self.trip_count = self.trip_count +1
            print self.trip_count
            self.currently_tripped = True
        if self.currently_tripped and (value > self.trip_threshold): 
            print 'PV restored', pvname, char_value, time.ctime()
            self.currently_tripped = False
            if not (self.latest_trip == None) :
                self.latest_trip.stop()
                self.trips.append(self.latest_trip)
        #if self.currently_tripped: 
        #    print 'PV still tripped', pvname, char_value, time.ctime()

    def AddCallback(self):
        self.process_var.add_callback(self.PVChangeCallback)

    def ClearCallback(self):
        self.process_var.clear_callbacks()

    def Dump(self):
        print(self.pv_name, "counts ", self.trip_count )

    def Print(self):
        self.ClearCallback()
        print(self.pv_name, "counts ", self.trip_count )

    def GetJSONObject(self):
        self.ClearCallback()
        all_trips = []
        for num,atrip in enumerate(self.trips):
            all_trips.append(atrip.GetJSONObject())
        trip_dict = {'trips': len(self.trips)}
        trip_dict.update({'PV': self.pv_name})
        trip_dict.update({'trip_events': all_trips})
        return {self.name: trip_dict }

    def PrintJSON(self):
        print json.dumps(self.GetJSONObject(), sort_keys=True, indent=2, separators=(',', ': '))


class RunSummary:
    """Run Summary class"""
    def __init__(self,run=0):
        self.kine_was_printed = True
        self.run_number = int(run)
        self.counter_names = []
        self.counters = []
        self.integrators = []
        self.run_info = {}
        self.shms_angle_offset = 0.075
        self.hms_angle_offset  = 0.005
        self.shms_angle_pv = PV('ecSHMS_Angle')
        self.shms_momentum_pv = PV('SHMS_Momentum')
        self.hms_angle_pv = PV('ecHMS_Angle')
        self.hms_momentum_pv = PV('HMS_Momentum')
        self.beam_energy_pv = PV('HALLC:p')
        self.target_select_pv =  PV('hcBDSSELECT')
        self.target_busy_pv   =  PV('hcBDSBUSY')
        self.target_busy_pv.add_callback(self.TargetBusyCallback)
        self.target_sel_val  = int(self.target_select_pv.get())
        self.target_sel_pv_name = str("hcBDSSEL1:but"+str(int(self.target_sel_val+1)))
        self.target_sel_name_pvs = {}
        self.target_sel_name = ""
        if self.target_sel_pv_name in self.target_sel_name_pvs:
            self.target_sel_name = self.target_sel_name_pvs[self.target_sel_pv_name].get(as_string=True)
        else :
            self.target_sel_name_pvs.update({ self.target_sel_pv_name : PV(self.target_sel_pv_name)})
            self.target_sel_name = self.target_sel_name_pvs[self.target_sel_pv_name].get(as_string=True)
        self.target_changed = True
        print("target pv     name ", self.target_sel_pv_name)
        print("target select name ", self.target_sel_name)

    def GetTargetName(self):
        self.target_sel_val     = int(self.target_select_pv.get())
        self.target_sel_pv_name = str("hcBDSSEL1:but")+str(int(self.target_sel_val)+1)
        if self.target_sel_pv_name in self.target_sel_name_pvs:
            self.target_sel_name = self.target_sel_name_pvs[self.target_sel_pv_name].get(as_string=True)
        else :
            self.target_sel_name_pvs.update({ self.target_sel_pv_name : PV(self.target_sel_pv_name)})
            self.target_sel_name = self.target_sel_name_pvs[self.target_sel_pv_name].get(as_string=True)
        #self.target_sel_name    = str(caget(self.target_sel_pv_name))
        print "sel val  : " , self.target_sel_val
        print "pv  name : " , self.target_sel_pv_name
        print "sel name : " , self.target_sel_name
        return self.target_sel_name

    def BuildTargetReport(self):
        res = {"target" :
                { "target_id" : self.target_sel_val, 
                    "target_name": self.target_sel_name,
                    "target_label": target_desc[ str(int(self.target_sel_val))]
                    } }
        return res 
    def BuildSpecReport(self):
        res = {"Spectrometers" :
                {
                    "shms_momentum" : self.shms_momentum_pv.get(), 
                    "hms_momentum"  : self.hms_momentum_pv.get(), 
                    "shms_angle"    : self.shms_angle_pv.get()   + self.shms_angle_offset, 
                    "hms_angle"     : self.hms_angle_pv.get()    + self.hms_angle_offset   
                    } 
                }
        return res 
    def BuildBeamReport(self):
        res = {"beam": {"beam_energy":10.60, "beam_pol": 0.0} }
        return res

    def TargetBusyCallback(self,pvname=None, value=None, char_value=None, **kws):
        #global pool
        val = float(char_value)
        if int(val) != 0:
            print "busy value : ",val
            self.target_changed = True
            #pool.apply_async(self.GetTargetSelName,(int(self.target_sel_val),))
            #self.target_sel_name = caget("hcBDSSEL1:but"+str(int(self.target_sel_val+1)))

    def CreateIntegrator(self, pv=None, name=None,outpv=None):
        if name == None :
            name = str(pv)
        self.integrators.append(( name, PVIntegrator(pv=pv,name=name,outpv=outpv)))

    def CreateTripCounter(self, pv=None, thresh=0.0, name=None):
        if name == None :
            name = str(pv)
        self.counters.append(( name, TripCounter(pv=pv,name=name,th=thresh))) 
    def Reset(self):
        self.kine_was_printed = False
        for n, cnt in self.counters :
            cnt.Reset()
        for n, cnt in self.integrators :
            cnt.Reset()
        
    def SetCallbacks(self):
        for n, cnt in self.counters :
            cnt.AddCallback()
        for n, cnt in self.integrators :
            cnt.AddCallback()

    def ClearCallbacks(self):
        for n, cnt in self.counters :
            cnt.ClearCallback()
        for n, cnt in self.integrators :
            cnt.ClearCallback()

    def Dump(self):
        print str("Run Number : {} ").format(  self.run_number ) 
        #for n, cnt in self.counters :
        #    cnt.Dump()
        for n, cnt in self.integrators :
            cnt.Dump()

    def Print(self):
        for n, cnt in self.counters :
            cnt.Print()
        for n, cnt in self.integrators :
            cnt.Print()

    def start(self):
        """start the counting"""
        self.PrintKinematics()
        self.SetCallbacks()
        self.GetTargetName()

    def stop(self):
        """stop the counting"""
        self.ClearCallbacks()
        #self.PrintKinematics()

    def GetJSONObject(self):
        all_counters = { }
        for n, cnt in self.counters :
            all_counters.update(cnt.GetJSONObject())
        counters_dict = {"counters": all_counters}
        all_inte = { }
        for n, cnt in self.integrators :
            all_inte.update(cnt.GetJSONObject())
        integ_dict = {"integrators": all_inte}
        counters_dict.update(integ_dict)
        counters_dict.update(self.BuildTargetReport())
        counters_dict.update(self.BuildSpecReport())
        counters_dict.update(self.BuildBeamReport())
        return {str(int(self.run_number)): counters_dict}

    def PrintJSON(self):
        print json.dumps(self.GetJSONObject(), sort_keys=True, indent=4, separators=(',', ': '))

    def PrintKinematics(self):
        P0_shms = self.shms_momentum_pv.get()
        th_shms = self.shms_angle_pv.get()   + self.shms_angle_offset
        th_hms  = self.hms_angle_pv.get() + self.hms_angle_offset 
        P0_hms  = self.hms_momentum_pv.get()
        E_hallc = 10.60
        print self.run_number, '-', self.run_number
        print 'pbeam = ',E_hallc
        print 'gtargmass_amu = ',2.014101
        print 'htheta_lab = ',th_hms
        print 'ptheta_lab = ',th_shms
        print 'hpcentral = ',P0_hms
        print 'ppcentral = ',P0_shms
        print 'ppartmass = ', 0.1395706
        print 'hpartmass = ', 0.0005109 
        if not self.kine_was_printed:
            with open('test.standard.kinematics', 'a') as f:
                f.write(str('{} - {}\n').format(self.run_number, self.run_number))
                f.write(str('pbeam = {}\n').format(E_hallc))
                f.write(str('gtargmass_amu = {}\n').format(2.014101))
                f.write(str('htheta_lab = {}\n').format(th_hms))
                f.write(str('ptheta_lab = {}\n').format(th_shms))
                f.write(str('hpcentral = {}\n').format(P0_hms))
                f.write(str('ppcentral = {}\n').format(P0_shms))
                f.write(str('ppartmass = {}\n').format(0.1395706))
                f.write(str('hpartmass = {}\n').format( 0.0005109 ))
                f.write('\n')
        self.kine_was_printed = True

class MyThread(threading.Thread):

    def __init__(self,func=None):
        threading.Thread.__init__(self)
        self.sleep_event = threading.Event()
        self.damon = True
        self.stopped = False
        self.function = func
        self.functions = []
    def add_func(self, func):
        self.functions.append(func)

    def run(self):
        while not self.stopped:
            self.sleep_event.clear()
            self.sleep_event.wait(10)
            threading.Thread(target=self._run).start()

    def _run(self):
        self.function()
        for f in self.functions :
            f()

class SummaryList:
    """Run summary list.
    Useful in retaining the async futures  and print to file. 
    """
    def __init__(self):
        self.results = {}
        self.current_run = RunSummary(0)
        self.is_ready = False
        self.printing_thread = MyThread(self.Dump)
        self.printing_thread.add_func(self.check_run_number)
        self.printing_thread.start()

    def CreateSummary(self,run=0):
        self.stop()
        self.current_run.run_number = int(run)
        self.is_ready = True

    def CreateTripCounter(self, pv=None, thresh=0.0, name=None):
        self.current_run.CreateTripCounter(pv, thresh, name)

    def CreateIntegrator(self, pv=None, name=None,outpv=None):
        self.current_run.CreateIntegrator(pv,  name,outpv)

    def Reset(self):
        self.stop()
        self.current_run.Reset()

    def StartNewRun(self,run):
        self.CreateSummary(run)
        self.current_run.start()

    def check_run_number(self):
        global coda_run_number_pv
        epics_val = coda_run_number_pv.get()
        if int(epics_val) !=  int(self.current_run.run_number):
            print str("Error DAQ must have crashed. Starting new counts for run {}").format(int(epics_val))
            #self.Reset()
            #self.StartNewRun(epics_val)

    def stop(self):
        """Stop the counting and append to list"""
        if self.is_ready:
            self.current_run.stop()
            self.results.update(self.current_run.GetJSONObject())
            self.is_ready = False

    def GetJSONObject(self):
        self.stop()
        return self.results
        #runs = {}
        #for rn,asummary in self.results:
        #    runs.update({rn,asummary})
        #return runs

    def Dump(self):
        self.current_run.Dump()

    def Print(self):
        self.stop()

    def PrintJSON(self):
        self.stop()
        print "Printing JSON ...."
        print json.dumps(self.GetJSONObject(), sort_keys=True, indent=4, separators=(',', ': '))
        self.Print()


def print_everything(*args):
    for count, thing in enumerate(args):
        print( '{0}. {1}'.format(count, thing))

def mycallback(x):
    print('mycallback is called')
    global results
    x.Print()
    results.append(x)
    

def codaInProgress(pvname=None, value=None, char_value=None, **kws):
    global coda_running
    global coda_run_number
    global summary
    #global pool
    global res
    global run_list
    global out_file_name
    in_progress = int(char_value)
    print("run is in progress: ", in_progress)
    if in_progress and not coda_running: 
        coda_run_number = coda_run_number_pv.get()
        print("new run: ",coda_run_number)
        coda_running = True
        run_list.Reset()
        run_list.StartNewRun(coda_run_number)
    if not in_progress and coda_running: 
        print("end of run: ",coda_run_number)
        coda_running = False
        run_list.current_run.stop()
        #run_list.current_run.Print()
        your_json = {}
        if os.path.isfile(out_file_name) :
            with open(out_file_name) as data_file:
                your_json = json.loads(data_file.read())
        your_json.update(run_list.current_run.GetJSONObject())
        #with open(out_file_name,'w') as data_file:
        #    json.dump(your_json,data_file)
        print your_json
        with open(out_file_name, 'w') as f:
              f.write(json.dumps(your_json, sort_keys=True, ensure_ascii=False, indent=2))
        run_list.current_run.PrintJSON()

##################################################################
#parser = OptionParser()
#parser.add_option("-f", "--file", dest="filename",
#                  help="write report to FILE", metavar="FILE")
#parser.add_option("-q", "--quiet",
#                  action="store_false", dest="verbose", default=True,
#                                    help="don't print status messages to stdout")
#
#(options, args) = parser.parse_args()

results = []
#pool = Pool(5)
coda_in_progress_pv = PV("hcSHMSRunInProgress")
coda_run_number_pv  = PV("hcSHMSRunNumber")
coda_running = False
coda_run_number = coda_run_number_pv.get()
run_list = SummaryList()
#summary = run_list.current_run
out_file_name = 'run_list_4.json'

def main():
    """Runs continuously"""
    #bcm_pv = PV("IBC3H00CRCUR4")
    #flow_rate = PV("CFI60DLP")
    #print_everything("x", "Y", "ZZZZ")
    #IBC1H04CRCUR2
    #run_list.CreateTripCounter("CFI60DLP",name="cryo pressure",thresh=7.5)
    #run_list.current_run.CreateTripCounter("IBC3H00CRCUR4",name="Hall C bcm",thresh=1.0)
    run_list.CreateTripCounter("ibcm1",name="ibcm1",thresh=1.0)
    run_list.CreateTripCounter("ibcm2",name="ibcm2",thresh=1.0)
    run_list.CreateIntegrator("ibcm1",name="bcm1",outpv="hcRunTotalCharge")
    run_list.CreateIntegrator("ibcm2",name="bcm2")
    #summary.SetCallbacks()
    coda_in_progress_pv.add_callback(codaInProgress)
    
    #pool.apply_async(summary_copy.PrintJSON(),callback=mycallback)
    
    t0 = time.time()
    try:
        while True:
            time.sleep(1.e-2)
            raw_input("Hit ENTER to print at anytime\n")
            run_list.printing_thread.sleep_event.set()
    except KeyboardInterrupt:
        if coda_running:
           run_list.current_run.Print()
        print 'All runs...'
        #run_list.Print()
        run_list.printing_thread.stopped = True
        run_list.PrintJSON()
        print 'Done.'
        sys.exit(0)
    
    #run_list.current_run.Print()
    #run_list.current_run.PrintJSON()
    print 'derp.'
    #m1 = caget("IBC3H00CRCUR4")
    #print m1

if __name__ == "__main__":
    main()




