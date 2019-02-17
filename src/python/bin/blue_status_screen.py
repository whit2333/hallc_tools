#!/usr/bin/python
import sys, os
import curses
#import epics
from epics import caget, caput, camonitor, PV
import epics
import threading
from time import sleep
import sys
import StringIO

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

#print args.kinematics
print(args.input_file)


cursor_x = 0
cursor_y = 0
k = 0


def inputThread(stdscr, e):
    global cursor_x
    global cursor_y
    global k
    while True:

        event_is_set = e.wait()

        c = stdscr.getch()
        k = c
        if k == 'q':
            break
        height, width = stdscr.getmaxyx()

        if k == curses.KEY_DOWN:
            cursor_y = cursor_y + 1
        elif k == curses.KEY_UP:
            cursor_y = cursor_y - 1
        elif k == curses.KEY_RIGHT:
            cursor_x = cursor_x + 1
        elif k == curses.KEY_LEFT:
            cursor_x = cursor_x - 1

        cursor_x = max(0, cursor_x)
        cursor_x = min(width - 1, cursor_x)

        cursor_y = max(0, cursor_y)
        cursor_y = min(height - 1, cursor_y)
        #if c == ord('a'):
        #    x -= 1
        #elif c == ord('d'):
        #    x += 1
        sleep(0.15)
        #stdscr.addstr("inputThread:" + str(x) + "\n" + "c:" + str(c))


class PVWithDefault:
    def onChange(self, pvname=None, value=None, host=None, **kws):
        self.value = value

    def __init__(self, name=None, default=None):
        self.named_pv = PV(name, callback=self.onChange)
        self.value = default

    def get(self):
        return self.named_pv.get()


class HallcEpics:
    def onBCMChange(self, pvname=None, value=None, host=None, **kws):
        self.bcm1 = float(value)

    def onChargeChange(self, pvname=None, value=None, host=None, **kws):
        self.run_charge = float(value) / 1000.0

    def onTotChargeChange(self, pvname=None, value=None, host=None, **kws):
        self.setting_charge = float(value) / 1000.0

    def onTargetChange(self, pvname=None, value=None, host=None, **kws):
        self.target = int(value)

    def onBeamCurrentChange(self, pvname=None, value=None, host=None, **kws):
        self.beam_current = int(value)

    def onSettingBeamCurrentChange(self, pvname=None, value=None, host=None, **kws):
        self.setting_beam_current = int(value)

    def onKinGroupChange(self, pvname=None, value=None, host=None, **kws):
        self.kine_group = int(value)

    def onKinNumberChange(self, pvname=None, value=None, host=None, **kws):
        self.kine_number = int(value)
    def onKinIDChange(self, pvname=None, value=None, host=None, **kws):
        self.kine_id = int(value)

    def onSHMSAngleChange(self, pvname=None, value=None, host=None, **kws):
        print("onSHMSAngleChange")
        if self.shms_change_value != value :
            # turning off temporarily
            #self.shms_ask_angle = True
            self.shms_change_value = value

    def onHMSAngleChange(self, pvname=None, value=None, host=None, **kws):
        print("onHMSAngleChange")
        if self.hms_change_value != value :
            #self.hms_ask_angle = True
            self.hms_change_value = value
    def __init__(self):
        self.target = 0
        self.t = None
        self.shms_ask_angle = False
        self.shms_angle_changed_pv        = PV('hcSHMSAngleChanged')
        self.shms_change_value = self.shms_angle_changed_pv.get()
        self.shms_angle_changed_pv.add_callback(self.onSHMSAngleChange)
        self.hms_ask_angle = False
        self.hms_angle_changed_pv        = PV('hcHMSAngleChanged')
        self.hms_change_value = self.hms_angle_changed_pv.get()
        self.hms_angle_changed_pv.add_callback(self.onHMSAngleChange)
        self.run_number_pv     = PVWithDefault( name                           = 'hcSHMSRunNumber', default = int(0))
        self.target_sel_pv     = PV('hcBDSSELECT_mirror', callback            = self.onTargetChange)
        self.bcm1_pv           = PV('ibcm1', callback                          = self.onBCMChange)
        self.kine_group_pv     = PV('hcKinematicSettingGroup', callback        = self.onKinGroupChange)
        self.kine_number_pv    = PV('hcKinematicSettingNumber', callback       = self.onKinNumberChange)
        self.kine_id_pv        = PV('hcKinematicSettingID', callback        = self.onKinIDChange)
        try: 
            self.kine_group        = int(self.kine_group_pv.get())
            self.kine_number       = int(self.kine_number_pv.get())
            self.kine_id           = int(self.kine_id_pv.get())
        except TypeError:
            self.kine_group        = 0
            self.kine_number       = 0
            self.kine_id           = 0

        self.bcm1              = self.bcm1_pv.get()
        self.beam_current_pv   = PV('hcCOINRunAverageBeamCurrent', callback   = self.onBeamCurrentChange)
        self.setting_beam_current_pv   = PV('hcRunSettingAverageBeamCurrent', callback   = self.onSettingBeamCurrentChange)
        self.run_charge_pv     = PV('hcCOINRunAccumulatedCharge', callback    = self.onChargeChange)
        self.setting_charge_pv = PV('hcRunSettingAccumulatedCharge', callback = self.onTotChargeChange)
        self.run_number        = self.run_number_pv.value
        try: 
            self.run_charge        = float(self.run_charge_pv.get()) * 0.001
            self.setting_charge    = float(self.setting_charge_pv.get()) * 0.001
        except TypeError:
            self.run_charge        = 0.0
            self.setting_charge    = 0.0
        self.beam_current      = self.beam_current_pv.get()
        self.setting_beam_current   = self.setting_beam_current_pv.get()
        self.target            = self.target_sel_pv.get()
        self.target_names = {
            2: "LH2 Loop 2 10cm",
            3: "LD2 Loop 3 10cm",
            5: "dummy"
        }

    def draw_menu(self, stdscr):

        global x
        global cursor_x
        global cursor_y
        global k

        curses.initscr()
        # Clear and refresh the screen for a blank canvas
        stdscr.clear()
        stdscr.refresh()

        # Start colors in curses
        curses.start_color()
        curses.init_pair(1, curses.COLOR_CYAN, curses.COLOR_BLACK)
        curses.init_pair(2, curses.COLOR_RED, curses.COLOR_BLACK)
        curses.init_pair(3, curses.COLOR_BLACK, curses.COLOR_WHITE)
        curses.init_pair(4, curses.COLOR_WHITE, curses.COLOR_YELLOW)
        curses.init_pair(5, curses.COLOR_YELLOW, curses.COLOR_BLUE)
        curses.init_pair(6, curses.COLOR_WHITE,  curses.COLOR_BLUE)
        curses.init_pair(7, curses.COLOR_WHITE, curses.COLOR_BLACK)

        self.e = threading.Event()
        self.t = threading.Thread(
                name='daemon', target=inputThread, args=(stdscr,self.e ))
        self.t.setDaemon(True)
        self.t.start()

        self.e.set()
        curses.flushinp()

        #oldstdin = sys.stdin
        #sys.stdin = StringIO.StringIO('asdlkj')
        #print raw_input('.')       #  .asdlkj
        #sys.stdin = oldstdin

        #t2 = threading.Thread(name ='stats', target=statsThread, args=(stdscr,hc_epics,))
        #t2.setDaemon(True)
        #t2.start()

        # Loop where k is the last character pressed
        while (k != ord('q')):

            # Initialization
            stdscr.clear()
            height, width = stdscr.getmaxyx()

            if self.shms_ask_angle :
                self.e.clear()
                self.PromptForSHMSAngle(stdscr)
                self.shms_ask_angle = False
                self.e.set()

            if self.hms_ask_angle :
                self.e.clear()
                self.PromptForHMSAngle(stdscr)
                self.hms_ask_angle = False
                self.e.set()

            #if k == curses.KEY_DOWN:
            #    cursor_y = cursor_y + 1
            #elif k == curses.KEY_UP:
            #    cursor_y = cursor_y - 1
            #elif k == curses.KEY_RIGHT:
            #    cursor_x = cursor_x + 1
            #elif k == curses.KEY_LEFT:
            #    cursor_x = cursor_x - 1

            cursor_x = max(0, cursor_x)
            cursor_x = min(width - 1, cursor_x)

            cursor_y = max(0, cursor_y)
            cursor_y = min(height - 1, cursor_y)

            total_charge = 1.093
            average_beam_current = 60.008


            # Render status bar
            #stdscr.attron(curses.color_pair(3))
            #stdscr.addstr(height - 1, 0, statusbarstr)
            #stdscr.addstr(height - 1, len(statusbarstr),
            #              " " * (width - len(statusbarstr) - 1))
            #stdscr.attroff(curses.color_pair(3))

            # Turning on attributes for title
            stdscr.attron(curses.color_pair(2))
            stdscr.attron(curses.A_BOLD)

            # Rendering title
            #stdscr.addstr(start_y, start_x_title, title)

            # Turning off attributes for title
            stdscr.attroff(curses.color_pair(2))
            stdscr.attroff(curses.A_BOLD)

            #stdscr.attroff(curses.A_BOLD)
            #stdscr.addstr(5, 5, "total run charge:" )
            #stdscr.addstr(6, 6, "{}".format(total_charge))

            # Print rest of text
            #stdscr.addstr(start_y + 1, start_x_subtitle, subtitle)
            #stdscr.addstr(start_y + 3, (width // 2) - 2, '-' * 4)
            #stdscr.addstr(start_y + 5, start_x_keystr, keystr)
            #stdscr.move(cursor_y, cursor_x)

            begin_x = 2
            begin_y = 0
            height = 27
            width  = 42
            win    = stdscr.subwin(height, width, begin_y, begin_x)
            win.box()
            win.bkgd(' ', curses.color_pair(5))


            win.attron(curses.color_pair(5))
            win.attron(curses.A_BOLD)
            the_line = 1
            try: 
                win.addstr(
                        the_line, 1, "     RUN {}      setting ID {} ".format(
                            int(epics.pv.get_pv("hcCOINRunNumber").get()),
                            int(epics.pv.get_pv("hcKinematicSettingID").get())))
            except TypeError:
                win.addstr( the_line, 1, "Error : Hall C run information " )
                win.addstr( the_line+1, 1, "IOC is probably not running " )
                #win.addstr( the_line+2, 1, "Contact Whit Armstrong to fix." )
                #the_line = the_line+2

            the_line = the_line+2
            try: 
                win.addstr( the_line, 1, "{:20} : {:.2f} minutes".format("beam on run time",
                            float(epics.pv.get_pv("hcCOINRunTime").get()) / 60.0))
                win.addstr( the_line+1, 1, "whit@jlab.org, 717-341-1080" )
                the_line = the_line+1
                win.addstr(the_line, 1, "{:20} : {:.2f} mC".format("total run charge",self.run_charge))
                the_line = the_line+1
                win.addstr( the_line, 1, "{:20} : {:.2f} uA ".format("avg. beam current ",self.beam_current))
                the_line = the_line+2
                win.addstr( the_line, 1, "{:20} : {:.2f} uA".format("Beam Current ",self.bcm1))
                the_line = the_line+2
            except TypeError:
                win.addstr( the_line, 1, "Contact Whit Armstrong to fix" )
                win.addstr( the_line+1, 1, "whit@jlab.org, 717-341-1080" )
                the_line = the_line+6
            except ValueError:
                the_line = the_line+6

            win_set_height = 8
            win_set    = stdscr.subwin(win_set_height, width-4, the_line, begin_x+2)
            win_set.box()
            win_set.bkgd(' ', curses.color_pair(7))
            win_set.attron(curses.A_BOLD)
            #win_set.attroff(curses.A_BOLD)
            try: 
                win_set.addstr( 1, 1, "    Kinematic Setting  {}-{}".format(self.kine_group,self.kine_number))
                win_set.addstr( 3, 1, "{:19} : {:>5.1f} minutes".format("total time (>1uA)",
                   float(epics.pv.get_pv("hcRunSettingTime").get()) / 60.0))
                win_set.addstr( 4, 1, "{:19} : {:>5.1f} mC".format("Charge", self.setting_charge))
                win_set.addstr( 5, 1, "{:19} : {:>5.1f} mC".format("Charge Goal", float(epics.pv.get_pv("hcRunPlanChargeGoal").get()) ))
                win_set.addstr( 6, 1, "{:19} : {:>5.1f} uA".format("avg. beam current", self.setting_beam_current))
            except TypeError:
                win_set.addstr( 3, 1, "Error : timeout")
            except ValueError:
                win_set.addstr( 3, 1, "Error : timeout")

            the_line = the_line + win_set_height +1

            win.attron(curses.color_pair(6))

            try: 
                win.addstr( the_line, 1, " Target : {}".format(self.target_names[self.target]))
                the_line = the_line+1

                win.attron(curses.color_pair(5))

                win.addstr(the_line, 30, "{:>7}".format("offset"),curses.A_NORMAL)
                the_line = the_line+1
                win.addstr(the_line, 1, "  HMS   : {:.3f} GeV/c".format(
                            float(epics.pv.get_pv("hcHMSMomentum").get()) ))
                the_line = the_line+1
                win.addstr(the_line, 1, "        : {:.2f}  degrees ".format(
                        float(epics.pv.get_pv("hcHMSCorrectedAngle").get())))
                win_set.attroff(curses.A_BOLD)
                win.addstr(the_line, 30, "{:>7.3f}".format(float(epics.pv.get_pv("hcHMSAngleEncoderOffset").get())),curses.A_NORMAL)
                win_set.attron(curses.A_BOLD)
                the_line = the_line+2
                win.addstr(the_line, 1, "  SHMS  : {:.3f}  GeV/c".format(
                        float(epics.pv.get_pv("hcSHMSMomentum").get()) ))
                the_line = the_line+1
                win.addstr(the_line, 1,"        : {:.2f}".format(
                        float(epics.pv.get_pv("hcSHMSCorrectedAngle").get())))
                win.addstr(the_line, 30, "{:>7.3f}".format(float(epics.pv.get_pv("hcSHMSAngleEncoderOffset").get())),curses.A_NORMAL)
                the_line = the_line+1
            except TypeError:
                the_line = the_line+5
                win.addstr( the_line, 1, "Error : timeout")
            except ValueError:
                the_line = the_line+5
                win.addstr( the_line, 1, "Error : timeout")
            except KeyError:
                the_line = the_line+5
                win.addstr( the_line, 1, "Error : timeout")
            #win.refresh()
            # Refresh the screen
            stdscr.refresh()
            # Wait for next input
            #k = stdscr.getch()
            sleep(0.15)
        curses.nocbreak()
        stdscr.keypad(0)
        curses.echo()
        curses.endwin()

    def PromptForSHMSAngle(self, stdscr):
        global x
        global cursor_x
        global cursor_y
        global k
        
        while True:

            # Initialization
            stdscr.clear()
            stdscr.refresh()
            height, width = stdscr.getmaxyx()

            stdscr.bkgd(' ', curses.color_pair(7))
            stdscr.attron(curses.A_BOLD)

            angle = float(epics.pv.get_pv("hcSHMSCorrectedAngle").get())
            angle_offset = float(epics.pv.get_pv("hcSHMSAngleEncoderOffset").get())
            # Declaration of strings
            title = "You appear to have changed the SHMS angle!"
            subtitle = "Please read the angle from the SHMS camera.l"
            keystr = "Is this the correct angle?   {:8.3f}".format(angle)
            question = "Press 'yy' to confirm or 'nn' to enter new value." 

            # Centering calculations
            start_x_title = int((width // 2) - (len(title) // 2) -
                                len(title) % 2)
            start_x_subtitle = int((width // 2) - (len(subtitle) // 2) -
                                   len(subtitle) % 2)
            start_x_keystr = int((width // 2) - (len(keystr) // 2) -
                                 len(keystr) % 2)
            start_y = 2 #int((height // 2) - 4)

            # Rendering some text
            stdscr.addstr(0, 0, "Enter 'yy' or 'nn'", curses.color_pair(1))
            stdscr.attron(curses.color_pair(2))
            stdscr.attron(curses.A_BOLD)

            # Rendering title
            stdscr.addstr(start_y, 0, title)

            # Turning off attributes for title
            stdscr.attroff(curses.color_pair(2))
            stdscr.attroff(curses.A_BOLD)

            # Print rest of text
            stdscr.addstr(start_y + 1, 0, subtitle)
            stdscr.addstr(start_y + 3, 0, '-' * 4)
            stdscr.addstr(start_y + 5, 0, keystr)
            stdscr.move(cursor_y, cursor_x)

            #win.refresh()
            # Refresh the screen
            stdscr.refresh()
            # Wait for next input
            #k = stdscr.getch()
            curses.flushinp()
            k = stdscr.getch()
            if k == ord('y') :
                # recompute 
                self.shms_ask_angle = False
                return None
            if k == ord('n') :
                # recompute 
                curses.echo()
                prompting = "Enter the new angle:"
                stdscr.addstr(8, 1, prompting  )
                stdscr.refresh()
                new_angle = float(stdscr.getstr(start_y + 7, 0+2,15))
                raw_angle = float(epics.pv.get_pv("ecSHMS_Angle").get())
                new_angle_offset = new_angle - raw_angle
                epics.pv.get_pv("hcSHMSAngleEncoderOffset").put(new_angle_offset)
                #self.shms_ask_angle = False

                #
                return angle
        #if k is 'n' :
        #    pass

    def PromptForHMSAngle(self, stdscr):
        global x
        global cursor_x
        global cursor_y
        global k
        
        while True:

            # Initialization
            stdscr.clear()
            stdscr.refresh()
            height, width = stdscr.getmaxyx()

            stdscr.bkgd(' ', curses.color_pair(7))
            stdscr.attron(curses.A_BOLD)

            angle = float(epics.pv.get_pv("hcHMSCorrectedAngle").get())
            angle_offset = float(epics.pv.get_pv("hcHMSAngleEncoderOffset").get())
            # Declaration of strings
            title = "You appear to have changed the HMS angle!"
            subtitle = "Please read the angle from the HMS camera.l"
            keystr = "Is this the correct angle?   {:8.3f}".format(angle)
            question = "Press 'yy' to confirm or 'nn' to enter new value." 

            # Centering calculations
            start_x_title = int((width // 2) - (len(title) // 2) -
                                len(title) % 2)
            start_x_subtitle = int((width // 2) - (len(subtitle) // 2) -
                                   len(subtitle) % 2)
            start_x_keystr = int((width // 2) - (len(keystr) // 2) -
                                 len(keystr) % 2)
            start_y = 2 #int((height // 2) - 4)

            # Rendering some text
            stdscr.addstr(0, 0, "Enter 'yy' or 'nn'", curses.color_pair(1))
            stdscr.attron(curses.color_pair(2))
            stdscr.attron(curses.A_BOLD)

            # Rendering title
            stdscr.addstr(start_y, 0, title)

            # Turning off attributes for title
            stdscr.attroff(curses.color_pair(2))
            stdscr.attroff(curses.A_BOLD)

            # Print rest of text
            stdscr.addstr(start_y + 1, 0, subtitle)
            stdscr.addstr(start_y + 3, 0, '-' * 4)
            stdscr.addstr(start_y + 5, 0, keystr)
            stdscr.move(cursor_y, cursor_x)

            #win.refresh()
            # Refresh the screen
            stdscr.refresh()
            # Wait for next input
            #k = stdscr.getch()
            curses.flushinp()
            k = stdscr.getch()
            if k == ord('y') :
                # recompute 
                self.hms_ask_angle = False
                return None
            if k == ord('n') :
                # recompute 
                curses.echo()
                prompting = "Enter the new angle:"
                stdscr.addstr(8, 1, prompting  )
                stdscr.refresh()
                new_angle = float(stdscr.getstr(start_y + 7, 0+2,15))
                raw_angle = float(epics.pv.get_pv("ecHMS_Angle").get())
                new_angle_offset = new_angle - raw_angle
                epics.pv.get_pv("hcHMSAngleEncoderOffset").put(new_angle_offset)
                #self.hms_ask_angle = False

                #
                return angle
        #if k is 'n' :
        #    pass

def main():
    hc_epics = HallcEpics()
    curses.wrapper(hc_epics.draw_menu)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        """user wants control back"""
        print( "done")
