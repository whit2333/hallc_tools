#!/usr/bn/env python
from epics import caget, caput, camonitor, PV
import npyscreen
import datetime

run_number_pv = PV('hcCOINRunNumber')

class TestApp(npyscreen.NPSApp):
    def main(self):
        # These lines create the form and populate it with widgets.
        # A fairly complex screen in only 8 or so lines of code - a line for each control.
        npyscreen.setTheme(npyscreen.Themes.ColorfulTheme)
        F = npyscreen.ActionFormWithMenus(name = "Welcome to Npyscreen",)
        t = F.add(npyscreen.TitleText, name = "Run Number:",  value=str(int(run_number_pv.get())))
        fn = F.add(npyscreen.TitleFilename, name = "Filename:")
        dt = F.add(npyscreen.TitleDateCombo, name = "Date:", value=datetime.datetime.now(), use_datetime=True)
        s = F.add(npyscreen.TitleSlider, out_of=12, name = "Slider")

        ml_value= str(datetime.datetime.now())+ "\n";

        ml= F.add(npyscreen.MultiLineEdit, 
            value = ml_value, 
            max_height=5, rely=9)
        ms= F.add(npyscreen.TitleSelectOne, max_height=4, value = [1,], name="Pick One", 
                values = ["Option1","Option2","Option3"], scroll_exit=True)
        ms2= F.add(npyscreen.TitleMultiSelect, max_height=4, value = [1,], name="Pick Several", 
                values = ["Option1","Option2","Option3"], scroll_exit=True)
        
        # This lets the user play with the Form.
        F.edit()


if __name__ == "__main__":
    App = TestApp()
    App.run()
