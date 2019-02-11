#ifndef hallc_DetectorDisplay_HH
#define hallc_DetectorDisplay_HH

#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "THttpServer.h"
#include "TMessage.h"
#include "TMonitor.h"
#include "TServerSocket.h"
#include "TSocket.h"
#include "TSystem.h"

#include <functional>
#include <map>
#include <memory>
#include <vector>

#include "CodaDecoder.h"
#include "THaEvData.h"
#include "THaPostProcess.h"
#include "THcParmList.h"

//#include "cppitertools/zip.hpp"
#include "podd2/Logger.h"
#include "monitor/DisplayPlots.h"
#include "monitor/MonitoringDisplay.h"

namespace hallc {

  /** Detector Displays.
   *
   * \code{.cpp}
   * auto plt2 = ddisplay->CreateDisplayPlot(
   *     [&](hallc::DisplayPlot& plt) {
   *       plt._canvas = new TCanvas();
   *       // initialize plots here
   *       return 0;
   *     },
   *     [&](hallc::DisplayPlot& plt) {
   *       // update histograms here
   *       return 0;
   *     });
   * \endcode
   *
   */
  //struct DetectorDisplay  : public hallc::MonitoringDisplay {
  //public:
  //  DetectorDisplay(){}
  //  DetectorDisplay(int num) : hallc::MonitoringDisplay(num){}
  //  virtual ~DetectorDisplay(){}

  //  //DisplayPlot* CreateDisplayPlot(std::string name, InitFunction_t&& f_init,
  //  //                               UpdateFunction_t&& f_update);

  //  //void                         RegisterPlot(DisplayPlot* plot);
  //  //void                         InitAll();
  //  //void                         Process();
  //  //void                         UpdateAll();

  //  virtual void UpdateAll();

  //  ClassDef(DetectorDisplay,2)
  //};



  /** Simple  PostProcess implementation with some lambda hooks.
   *
   */
  class DisplayPostProcess : public podd2::AnalysisLogging<THaPostProcess> {
  private:
    int _counter      = 0;
    int _N_ped_skip   = 1200;
    int _N_event_skip = 1000;

  public:
    int              _run_number  = 0;
    MonitoringDisplay* _det_display = nullptr;

  public:
    DisplayPostProcess(MonitoringDisplay* d)
        : podd2::AnalysisLogging<THaPostProcess>(), _det_display(d) {}

    DisplayPostProcess() : podd2::AnalysisLogging<THaPostProcess>() {}
    virtual ~DisplayPostProcess() {}

    virtual Int_t Init(const TDatime&);
    virtual Int_t Process(const THaEvData* evt, const THaRunBase*, Int_t code);
    virtual Int_t Close();

    Int_t WritePlots();

    ClassDef(DisplayPostProcess, 1)
  };

}

#endif 

