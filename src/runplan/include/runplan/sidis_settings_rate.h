#ifndef runplan_sidis_settings_rate_HH
#define runplan_sidis_settings_rate_HH

#include <chrono>
#include <cmath>
#include <fstream>
#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <typeinfo>


#include "hallc_settings.h"


namespace hallc {

  std::vector<double> sidis_config_sigma_rate( RunPlanTableEntry& run_setting);
  void sidis_settings_rate() ;

}

#endif
