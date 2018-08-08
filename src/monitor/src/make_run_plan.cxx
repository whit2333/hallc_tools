#include <iostream>
#include <cmath>
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"
#include "TCanvas.h"
#include "TGraph.h"
#include "TBufferJSON.h"


#include "THcParmList.h"
R__LOAD_LIBRARY(libHallA.so)
R__LOAD_LIBRARY(libdc.so)
R__LOAD_LIBRARY(libHallC.so)

#include <range/v3/all.hpp>

#include "nlohmann/json.hpp"

// fmt - string formatting library
#include "fmt/core.h"
#include "fmt/ostream.h"
R__LOAD_LIBRARY(libfmt.so)

#include "hallc_settings.h"

void make_run_plan(Int_t RunNumber = 0, Int_t MaxEvent = 0) {

  std::string hallc_replay_dir = "./";
  //std::string hallc_replay_dir = "/home/whit/projects/hall_AC_projects/new_hallc_replay/EXPERIMENT_replay/hallc_replay/";
  std::string run_list_json  = "DBASE/run_list.json";

  THcParmList* hc_parms = new THcParmList();
  hc_parms->AddString("g_ctp_database_filename", (hallc_replay_dir+"DBASE/COIN/standard.database").c_str() );
  hc_parms->Define("gen_run_number", "Run Number", RunNumber);
  hc_parms->Load(hc_parms->GetString("g_ctp_database_filename"), RunNumber);
  //hc_parms->Load(hc_parms->GetString("g_ctp_parm_filename"));
  hc_parms->Load(hc_parms->GetString("g_ctp_kinematics_filename"), RunNumber);

  std::vector<int> runs(300);
  std::iota(runs.begin(), runs.end(), 3890);

  using namespace ranges;
  auto rng = runs | view::transform([&](int irun){
    hc_parms->Define("gen_run_number", "Run Number", irun);
    hc_parms->Load(hc_parms->GetString("g_ctp_database_filename"), irun);
    hc_parms->Load(hc_parms->GetString("g_ctp_kinematics_filename"), irun);
    return std::make_tuple(
        std::abs(hcana::json::FindVarValueOr(hc_parms, "htheta_lab", 0.0)),
        hcana::json::FindVarValueOr(hc_parms, "ptheta_lab", 0.0),
        hcana::json::FindVarValueOr(hc_parms, "hpcentral", 0.0),
        hcana::json::FindVarValueOr(hc_parms, "ppcentral", 0.0));
  });
  auto spectrometer_settings = view::zip(runs,rng) 
  | view::remove_if([](auto t){
    return std::get<0>(std::get<1>(t)) == 0.0;
  }) | view::remove_if([](auto t){
    return std::get<1>(std::get<1>(t)) == 0.0;
  }) | view::remove_if([](auto t){
    return std::get<2>(std::get<1>(t)) == 0.0;
  }) | view::remove_if([](auto t){
    return std::get<3>(std::get<1>(t)) == 0.0;
  });

  auto spec_settings = spectrometer_settings | view::transform([](auto r) {
                         hallc::HallCSetting setting;
                         setting.HMS_theta  = std::get<0>(std::get<1>(r));
                         setting.HMS_p0     = std::get<2>(std::get<1>(r));
                         setting.SHMS_theta = std::get<1>(std::get<1>(r));
                         setting.SHMS_p0    = std::get<3>(std::get<1>(r));
                         return std::make_tuple(std::get<0>(r), setting);
                       }) |
                       to_<std::vector>();

  // -------------------------------------------------
  //
  TGraph* gr0 = new TGraph();
  TGraph* gr1 = new TGraph();
  TGraph* gr2 = new TGraph();
  TGraph* gr3 = new TGraph();

  gr0->SetTitle("hms theta");
  gr1->SetTitle("shms theta");
  gr2->SetTitle("hms P0");
  gr3->SetTitle("shms P0");

  TMultiGraph* mg = new TMultiGraph();
  mg->Add(gr0,"lp");
  mg->Add(gr1,"lp");
  mg->Add(gr2,"lp");
  mg->Add(gr3,"lp");

  int ip = 0;
  for(const auto& r : spectrometer_settings){
    //shms_p_settings.push_back(r);
    gr0->SetPoint(ip,std::get<0>(r), std::get<0>(std::get<1>(r)));
    gr1->SetPoint(ip,std::get<0>(r), std::get<1>(std::get<1>(r)));
    gr2->SetPoint(ip,std::get<0>(r), std::get<2>(std::get<1>(r)));
    gr3->SetPoint(ip,std::get<0>(r), std::get<3>(std::get<1>(r)));
    ip++;
  }
  //for(const auto& r : shms_p_settings){
  //  //std::cout << std::get<0>(r) << "," << std::get<1>(r) << "\n";
  //}

  gr0->SetMarkerStyle(20);
  gr1->SetMarkerStyle(20);
  gr2->SetMarkerStyle(20);
  gr3->SetMarkerStyle(20);

  gr0->SetMarkerColor(1);
  gr1->SetMarkerColor(2);
  gr2->SetMarkerColor(4);
  gr3->SetMarkerColor(8);

  TCanvas* c = new TCanvas();
  mg->Draw("a");

  c->BuildLegend();

  // -------------------------------------------------
  //
  std::cout << " Settings :\n";
  //std::cout  << TBufferJSON::ToJSON(&spec_settings);
  {
    ofstream  outfile("monitoring/test_settings.json");
    outfile  << TBufferJSON::ToJSON(&spec_settings);
  }

}
