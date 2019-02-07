#ifndef hallc_ExperimentMonitor_hh
#define hallc_ExperimentMonitor_hh 1

#include "monitor/DisplayPlots.h"
#include "monitor/DetectorDisplay.h"
#include "monitor/MonitoringDisplay.h"

namespace hallc {

  /** Experiment monitor display.
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
  struct ExperimentMonitor  : public hallc::MonitoringDisplay {

  public:
    ExperimentMonitor(){}
    virtual ~ExperimentMonitor(){}

    //DisplayPlot* CreateDisplayPlot(std::string name, InitFunction_t&& f_init,
    //                               UpdateFunction_t&& f_update);

    //void                         RegisterPlot(DisplayPlot* plot);
    //void                         InitAll();
    //void                         Process();
    //void                         UpdateAll();

    ClassDef(ExperimentMonitor,2)
  };


  /** Simple  PostProcess implementation with some lambda hooks.
   *
   */
  class ExperimentMonitorPostProcess : public podd2::AnalysisLogging<THaPostProcess> {
  private:
    int _counter      = 0;
    int _N_ped_skip   = 1200;
    int _N_event_skip = 1000;

  public:
    int              _run_number  = 0;
    DetectorDisplay* _det_display = nullptr;

  public:
    ExperimentMonitorPostProcess(DetectorDisplay* d)
        : podd2::AnalysisLogging<THaPostProcess>(), _det_display(d) {}

    ExperimentMonitorPostProcess() : podd2::AnalysisLogging<THaPostProcess>() {}
    virtual ~ExperimentMonitorPostProcess() {}

    virtual Int_t Init(const TDatime&);
    virtual Int_t Process(const THaEvData* evt, const THaRunBase*, Int_t code);
    virtual Int_t Close();

    Int_t WritePlots();

    ClassDef(ExperimentMonitorPostProcess, 1)
  };

}


#endif

