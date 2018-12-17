
R__LOAD_LIBRARY(libsimple_epics.so)
#include "simple_epics/PVList.h"

void epics_test() {

    std::vector<std::string> pvs = {"whit:circle:angle","root:test"};
    hallc::PVList get_list2(pvs);
    get_list2.PrintAll();

    get_list2.Put("root:test",0.1234);
    //std::cout << "derp\n";
    //get_list2.TestPut("derp",0.1234); // cdoes nothing because there is no "derp" PV

    get_list2.Poll();
    std::cout << "Printing all in pvlist ....\n";
    get_list2.PrintAll();

    get_list2.Put("root:test",0.789);
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    get_list2.PollAndPrintAll();

    get_list2.Put("root:test",0.12);
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    get_list2.PollAndPrintAll();

}


