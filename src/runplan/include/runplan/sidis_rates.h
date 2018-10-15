#ifndef hallc_tools_runplan_settings_rates_HH
#define hallc_tools_runplan_settings_rates_HH

#include <chrono>
#include <cmath>
#include <fstream>
#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <typeinfo>


#include "hallc_settings.h"
#include "RunTable.h"


namespace hallc {

  /** Builds a run-plan table etnry for a single kinematic.
   *
   */
  RunPlanTableEntry build_table_entry(const Kinematic& kine);

  /** Builds a table of 
   *
   */
  RunTable build_sidis_table(const csv::CSV_Settings& settings);

  /**
   *
   */
  void save_tables(const RunTable& table_rows, std::string table_name,
                   const std::ios_base::openmode& mode = std::ios_base::app);

  void save_json_tables(const RunTable& table_rows, std::string table_name,
                        const std::ios_base::openmode& mode = std::ios_base::app);

  /**
   *
   */
  std::vector<double> sidis_config_xs_and_rates(RunPlanTableEntry& run_setting);

}

#endif
