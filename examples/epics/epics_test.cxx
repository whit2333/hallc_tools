
R__LOAD_LIBRARY(libsimple_epics.so)
#include "simple_epics/PVGetList.h"

void epics_test() {

    std::vector<std::string> pvs = {"whit:circle:angle","root:test"};
    hallc::PVGetList get_list2(pvs);
    get_list2.PrintAll();

    get_list2.TestPut("root:test",123.4);
    std::cout << "derp\n";
    get_list2.TestPut("derp",123.4); // cdoes nothing because there is no "derp" PV

    get_list2.TestPut("root:test",56.789);
}


