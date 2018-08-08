#include "runplan/RunStatus.h"
#include "monitor/ExperimentMonitor.h"

#include <fstream>

#include "TCanvas.h"
#include "TRootCanvas.h"
#include "TBufferJSON.h"
#include "TRandom3.h"

// nlohmann json library (header only)
#include "nlohmann/json.hpp"

// fmt - string formatting library
#include "fmt/core.h"
#include "fmt/ostream.h"
R__LOAD_LIBRARY(libfmt.so)

// date library (while waiting on c++20) https://github.com/HowardHinnant/date
#include "date/date.h"

#include "hallc_settings.h"

using json        = nlohmann::json;
using Q2Table     = std::vector<RunPlanTableEntry>;
using TableVector = std::vector<std::pair<double, Q2Table>>;
using day_point   = std::chrono::time_point<std::chrono::system_clock, date::days>;


void ExperimentMonitor() {

  std::vector<std::vector<std::pair<int,RunPlanTableEntry>>>* run_order = nullptr;
  {
    json j; 
    std::ifstream infile("tables/run_order_by_Q2.json");
    infile >> j;
    std::string sj = j.dump();
    TBufferJSON::FromJSON( run_order, sj.c_str());
  }

  auto random = new TRandom3();

  hallc::RunSet run_set;

  int ikine = 1;
  for (auto& Q2_table : *run_order) {
    for (auto& [num, en] : Q2_table) {

      int rand_cnts = (int)(en.counts * random->Uniform());
      int entry_cnts = (int)en.counts;
      hallc::RunStatus rs0 {num, ikine, entry_cnts , rand_cnts };
      run_set.Add(rs0);
    }

    ikine++;
  }

  //run_set.Add(rs1);

  auto mon_hists  = new hallc::MonitoringHistograms(run_set);

  std::cout << " Monitoring Histograms :\n";
  std::cout  << TBufferJSON::ToJSON(mon_hists);

  TCanvas* c = new TCanvas("hallc_montior","HallC Monitor",10,500,800,500);
  ((TRootCanvas *)c->GetCanvasImp())->ShowMenuBar(false);
  ((TRootCanvas *)c->GetCanvasImp())->ShowToolBar(false);
  ((TRootCanvas *)c->GetCanvasImp())->ShowStatusBar(false);

  auto hs = mon_hists->GetStack(0);
  hs->Draw("nostack");

    //std::ofstream json_ofile("tables/LD2_run_plan_table.json",std::ios_base::trunc);
   // json_ofile.close();


}

