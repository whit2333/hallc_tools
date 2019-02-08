#include "monitor/ExperimentMonitor.h"

#include "Scandalizer.h"

#include "THcHodoscope.h"
#include "THcCherenkov.h"
#include "THcDC.h"

namespace hallc {

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

