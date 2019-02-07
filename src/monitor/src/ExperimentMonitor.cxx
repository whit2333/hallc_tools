#include "monitor/ExperimentMonitor.h"

//#include "runplan/RunStatus.h"
//
//#include <fstream>
//
//#include "TCanvas.h"
//#include "TRootCanvas.h"
//#include "TBufferJSON.h"
//#include "TRandom3.h"
//
//// nlohmann json library (header only)
//#include "nlohmann/json.hpp"
//
//// fmt - string formatting library
//#include "fmt/core.h"
//#include "fmt/ostream.h"
//R__LOAD_LIBRARY(libfmt.so)
//
//// date library (while waiting on c++20) https://github.com/HowardHinnant/date
//#include "date/date.h"
//
//#include "runplan/hallc_settings.h"
//#include "runplan/RunTable.h"
//
//using json        = nlohmann::json;
//using Q2Table     = std::vector<RunPlanTableEntry>;
//using TableVector = std::vector<std::pair<double, Q2Table>>;
//using day_point   = std::chrono::time_point<std::chrono::system_clock, date::days>;


//void ExperimentMonitor() {

  //std::vector<std::vector<std::pair<int,RunPlanTableEntry>>>* run_order = nullptr;
  //{
  //  json j; 
  //  std::ifstream infile("tables/run_order_by_Q2.json");
  //  infile >> j;
  //  std::string sj = j.dump();
  //  TBufferJSON::FromJSON( run_order, sj.c_str());
  //}
  //std::cout << " asdf\n";

  //auto random = new TRandom3();

  //hallc::RunSet run_set;

  //int ikine = 1;
  //for (auto& Q2_table : *run_order) {
  //  for (auto& [num, en] : Q2_table) {

  //    int rand_cnts = (int)(en.counts * random->Uniform());
  //    int entry_cnts = (int)en.counts;
  //    hallc::RunStatus rs0 {num, ikine, entry_cnts , rand_cnts };
  //    run_set.Add(rs0);
  //  }

  //  ikine++;
  //}

  ////run_set.Add(rs1);

  //auto mon_hists  = new hallc::MonitoringHistograms(run_set);

  //std::cout << " Monitoring Histograms :\n";
  //std::cout  << TBufferJSON::ToJSON(mon_hists);

  //TCanvas* c = new TCanvas("hallc_montior","HallC Monitor",10,500,800,500);
  //((TRootCanvas *)c->GetCanvasImp())->ShowMenuBar(false);
  //((TRootCanvas *)c->GetCanvasImp())->ShowToolBar(false);
  //((TRootCanvas *)c->GetCanvasImp())->ShowStatusBar(false);

  //auto hs = mon_hists->GetStack(0);
  //hs->Draw("nostack");

  //  //std::ofstream json_ofile("tables/LD2_run_plan_table.json",std::ios_base::trunc);
  // // json_ofile.close();

namespace hallc {

  //ExperimentMonitor::ExperimentMonitor() : _sock(new TSocket("localhost", 9090)) {
  // char str[32];
  // _sock->Recv(str, 32);
  // // server tells us who we are
  // int idx = !strcmp(str, "go") ? 0 : 1;
  // _sock->SetCompressionLevel(1);
  //}

  //void ExperimentMonitor::RegisterPlot(DisplayPlot* plot) {
  //  _plots[plot->_plot_data._id] = plot;
  //  _plot_map[plot->_plot_data._id] = &(plot->_plot_data);
  //}

  //DisplayPlot* ExperimentMonitor::CreateDisplayPlot(std::string name, InitFunction_t&& f_init,
  //                                                UpdateFunction_t&& f_update) {
  //  auto plt = new DisplayPlot(name, std::forward<InitFunction_t>(f_init),
  //                             std::forward<UpdateFunction_t>(f_update));
  //  plt->_plot_data._id = _plots.size() + 1;
  //  RegisterPlot(plt);
  //  return plt;
  //}

  //void ExperimentMonitor::InitAll() {

  //  for (auto& [i, plt] : _plots) {
  //    plt->Init();
  //    if (plt->_plot_data._canvas) {
  //      //_server->Register("/detectors", plt->_canvas);
  //    } else {
  //    }
  //  }
  //}

  //void ExperimentMonitor::Process() {
  //  for (auto& [i, plt] : _plots) {
  //    plt->Update();
  //  }
  //}

  //void ExperimentMonitor::UpdateAll() {
  //  //_plot_map.clear();
  //  for (auto& [i, plt] : _plots) {
  //    plt->_plot_data._canvas->Modified();
  //    //_plot_map[i] = (hallc::display::PlotData*)plt;
  //  }
  //  for (auto& [i, plt] : _plots) {
  //    plt->_plot_data._canvas->Update();
  //  }
  //  TMessage::EnableSchemaEvolutionForAll(false);
  //  TMessage mess(kMESS_OBJECT);
  //  mess.Reset();              // re-use TMessage object
  //  mess.WriteObject(&_plot_map);     // write object in message buffer
  //  _sock->Send(mess);          // send message
  //  std::cout << "message sent\n";
  //  //messlen  += mess.Length();
  //  //cmesslen += mess.CompLength();
  //}
  ////_________________________________________________________________________

  //____________________________________________________________________________

  Int_t ExperimentMonitorPostProcess::Init(const TDatime&) {
    _det_display->InitAll();
    return 0;
  }

  Int_t ExperimentMonitorPostProcess::Process(const THaEvData* evt, const THaRunBase*, Int_t code) {
    _det_display->Process();
    if ((evt->GetEvNum() > _N_ped_skip) && (_counter > _N_event_skip)) {
      _det_display->UpdateAll();
      gSystem->ProcessEvents();
      //WritePlots();
      _counter = 0;
    }
    _counter++;
    return 0;
  }

  Int_t ExperimentMonitorPostProcess::WritePlots() {
    // std::thread thObj([&]() {
    gSystem->mkdir(fmt::format("monitoring/{}", _run_number).c_str(), true);
    for (auto& [id, aplot] : _det_display->_plots) {
      std::string plt_output = fmt::format("monitoring/{}/plot_{}.json", _run_number, id);
      aplot->_plot_data._canvas->SaveAs(plt_output.c_str());
    }
    //});
    // thObj.detach();
    return 0;
  }

  Int_t ExperimentMonitorPostProcess::Close() {
    gSystem->mkdir(fmt::format("monitoring/{}", _run_number).c_str(), true);
    for (auto& [id, aplot] : _det_display->_plots) {
      std::string plt_output = fmt::format("monitoring/{}/plot_{}.json", _run_number, id);
      aplot->_plot_data._canvas->SaveAs(plt_output.c_str());
    }
    return 0;
  }

}

