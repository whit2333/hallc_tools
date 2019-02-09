#include "monitor/DetectorDisplay.h"
#include <thread>

#include "TPaveLabel.h"

namespace hallc {

  //DetectorDisplay::DetectorDisplay() : _sock(new TSocket("localhost", 9090)) {
  // char str[32];
  // _sock->Recv(str, 32);
  // // server tells us who we are
  // int idx = !strcmp(str, "go") ? 0 : 1;
  // _sock->SetCompressionLevel(1);
  //}

  //void DetectorDisplay::RegisterPlot(DisplayPlot* plot) {
  //  _plots[plot->_plot_data._id] = plot;
  //  _plot_map[plot->_plot_data._id] = &(plot->_plot_data);
  //}

  //DisplayPlot* DetectorDisplay::CreateDisplayPlot(std::string name, InitFunction_t&& f_init,
  //                                                UpdateFunction_t&& f_update) {
  //  auto plt = new DisplayPlot(name, std::forward<InitFunction_t>(f_init),
  //                             std::forward<UpdateFunction_t>(f_update));
  //  plt->_plot_data._id = _plots.size() + 1;
  //  RegisterPlot(plt);
  //  return plt;
  //}

  //void DetectorDisplay::InitAll() {

  //  for (auto& [i, plt] : _plots) {
  //    plt->Init();
  //    if (plt->_plot_data._canvas) {
  //      //_server->Register("/detectors", plt->_canvas);
  //    } else {
  //    }
  //  }
  //}

  //void DetectorDisplay::Process() {
  //  for (auto& [i, plt] : _plots) {
  //    plt->Update();
  //  }
  //}

  //void DetectorDisplay::UpdateAll() {

  //  for (auto& [i, plt] : _plots) {
  //    plt->_plot_data._canvas->SetTitle(fmt::format("{} Run:{} Event:{}",plt->_plot_data._name, _data._run_number,_event_number).c_str());
  //    //TPaveText *t = new TPaveText(0.0, 0.9, 0.3, 1.0, "brNDC"); // left-up
  //    plt->_plot_data._canvas->cd();
  //    TPaveLabel *t = new TPaveLabel(0.0, 0.9, 0.3, 1.0, fmt::format("{} Run:{} Event:{}",plt->_plot_data._name, _data._run_number,_event_number).c_str(), "brNDC"); // left-up
  //    t->Draw();
  //  }
  //  MonitoringDisplay::UpdateAll();
  //}
  //_________________________________________________________________________

  Int_t DisplayPostProcess::Init(const TDatime&) {
    _det_display->InitAll();
    return 0;
  }

  Int_t DisplayPostProcess::Process(const THaEvData* evt, const THaRunBase*, Int_t code) {
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

  Int_t DisplayPostProcess::WritePlots() {
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

  Int_t DisplayPostProcess::Close() {
    gSystem->mkdir(fmt::format("monitoring/{}", _run_number).c_str(), true);
    for (auto& [id, aplot] : _det_display->_plots) {
      std::string plt_output = fmt::format("monitoring/{}/plot_{}.json", _run_number, id);
      aplot->_plot_data._canvas->SaveAs(plt_output.c_str());
    }
    return 0;
  }

} // namespace hallc

