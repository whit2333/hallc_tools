import sys, os
import curses
#import epics
from epics import caget, caput, camonitor, PV
import epics
import threading
from time import sleep

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
print args.input_file


cursor_x = 0
cursor_y = 0
k = 0


def inputThread(stdscr):
    global cursor_x
    global cursor_y
    global k
    while True:
        c = stdscr.getch()
        k = c
        if k == 'q':
            break
        #curses.flushinp()
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
        #sleep(0.05)
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

    def __init__(self):
        self.target = 0
        self.run_number_pv = PVWithDefault(
            name='hcCOINRunNumber', default=int(0))
        self.target_sel_pv = PV(
            'hcBDSSELECT_mirror', callback=self.onTargetChange)
        self.bcm1_pv = PV('ibcm1', callback=self.onBCMChange)
        self.bcm1 = self.bcm1_pv.get()
        self.beam_current_pv = PV(
            'hcCOINRunAverageBeamCurrent', callback=self.onBeamCurrentChange)
        self.run_charge_pv = PV(
            'hcCOINRunAccumulatedCharge', callback=self.onChargeChange)
        self.setting_charge_pv = PV(
            'hcRunSettingAccumulatedCharge', callback=self.onTotChargeChange)
        self.run_number = self.run_number_pv.value
        self.run_charge = float(self.run_charge_pv.get()) * 0.001
        self.beam_current = self.beam_current_pv.get()
        self.setting_charge = float(self.setting_charge_pv.get()) * 0.001
        self.target = self.target_sel_pv.get()
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
        curses.init_pair(7, curses.COLOR_RED, curses.COLOR_BLACK)

        t = threading.Thread(
            name='daemon', target=inputThread, args=(stdscr, ))
        t.setDaemon(True)
        t.start()

        #t2 = threading.Thread(name ='stats', target=statsThread, args=(stdscr,hc_epics,))
        #t2.setDaemon(True)
        #t2.start()

        # Loop where k is the last character pressed
        while (k != ord('q')):

            # Initialization
            stdscr.clear()
            height, width = stdscr.getmaxyx()

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

            # Declaration of strings
            title = "Curses example" [:width - 1]
            subtitle = "Written by Clay McLeod" [:width - 1]
            keystr = "Last key pressed: {}".format(k)[:width - 1]
            statusbarstr = "Press 'q' to exit | STATUS BAR | Pos: {}, {}".format(
                cursor_x, cursor_y)
            if k == 0:
                keystr = "No key press detected..." [:width - 1]

            # Centering calculations
            start_x_title = int((width // 2) - (len(title) // 2) -
                                len(title) % 2)
            start_x_subtitle = int((width // 2) - (len(subtitle) // 2) -
                                   len(subtitle) % 2)
            start_x_keystr = int((width // 2) - (len(keystr) // 2) -
                                 len(keystr) % 2)
            start_y = int((height // 2) - 2)

            # Rendering some text
            whstr = "Width: {}, Height: {}".format(width, height)
            stdscr.addstr(0, 0, whstr, curses.color_pair(1))

            # Render status bar
            stdscr.attron(curses.color_pair(3))
            stdscr.addstr(height - 1, 0, statusbarstr)
            stdscr.addstr(height - 1, len(statusbarstr),
                          " " * (width - len(statusbarstr) - 1))
            stdscr.attroff(curses.color_pair(3))

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
            begin_y = 2
            height = 25
            width  = 42
            win    = stdscr.subwin(height, width, begin_y, begin_x)
            win.box()
            win.bkgd(' ', curses.color_pair(5))
            win.attron(curses.color_pair(5))
            win.attron(curses.A_BOLD)
            the_line = 1
            win.addstr(
                    the_line, 1, "     RUN {}  , setting {} (arbitrary)".format(
                        int(epics.pv.get_pv("hcCOINRunNumber").get()),
                        int(epics.pv.get_pv("hcRunSettingNumber").get())))
            the_line = the_line+1
            win.addstr( the_line, 1, "{:20} : {:.2f} minutes".format("beam on run time",
                        float(epics.pv.get_pv("hcCOINRunTime").get()) / 60.0))
            #win.attroff(curses.A_BOLD)
            the_line = the_line+1
            win.addstr(the_line, 1,
                    "{:20} : {:.2f} mC".format("total run charge",self.run_charge))
            the_line = the_line+1

            the_line = the_line+1
            win.addstr( the_line, 1, "Beam Current     : {:.2f} uA".format(self.beam_current))
            the_line = the_line+1
            win.addstr( the_line, 1, "    run avg.     : {:.2f} uA ".format(self.beam_current))
            the_line = the_line+1

            the_line = the_line+1

            win.addstr( the_line, 1,
                "setting charge   : {:.2f} mC".format(self.setting_charge))
            the_line = the_line+1
            the_line = the_line+1

            win.attron(curses.color_pair(6))

            win.addstr( the_line, 1, "Target           : {}".format(self.target_names[self.target]))
            the_line = the_line+1

            win.attron(curses.color_pair(5))

            the_line = the_line+1
            win.addstr(the_line, 1, "  HMS   : {:.2f} GeV/c".format(
                        float(epics.pv.get_pv("hcHMSMomentum").get()) ))
            the_line = the_line+1
            win.addstr(the_line, 1, "        : {:.2f} degrees".format(
                    float(epics.pv.get_pv("hcHMSCorrectedAngle").get()) ))
            the_line = the_line+1
            win.addstr(the_line, 1, "  SHMS  : {:.2f} GeV/c".format(
                    float(epics.pv.get_pv("hcSHMSMomentum").get()) ))
            the_line = the_line+1
            win.addstr(the_line, 1,"        : {:.2f} degrees".format(
                    float(epics.pv.get_pv("hcSHMSCorrectedAngle").get()) ))
            the_line = the_line+1
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


def main():
    hc_epics = HallcEpics()
    curses.wrapper(hc_epics.draw_menu)


if __name__ == "__main__":
    main()
