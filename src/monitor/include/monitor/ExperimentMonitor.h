#ifndef hallc_ExperimentMonitor_hh
#define hallc_ExperimentMonitor_hh 1

#include "monitor/DisplayPlots.h"
#include "monitor/DetectorDisplay.h"
#include "monitor/MonitoringDisplay.h"

class THcHodoscope;
class THcCherenkov;
class THcDC;

namespace hcana {
  class Scandalizer;
}

namespace hallc {


  ///** Experiment monitor display.
  // *
  // * This keeps a running tally of interesting and useful plots.
  // */
  //struct ExperimentMonitor  : public MonitoringDisplay {

  //public:
  //  ExperimentMonitor(){}
  //  ExperimentMonitor(int rn) : MonitoringDisplay(rn) {}
  //  virtual ~ExperimentMonitor(){}

  //  ClassDef(ExperimentMonitor,2)
  //};


  /** Simple  PostProcess implementation with some lambda hooks.
   *
   */
  class ExperimentMonitorPostProcess : public podd2::AnalysisLogging<THaPostProcess> {
  private:
    int _counter        = 0;
    int _N_event_update = 5000;
    int _N_ped_skip     = 1200;

  public:
    int                 _run_number        = 0;
    MonitoringDisplay*  _det_display       = nullptr;
    hcana::Scandalizer* _analyzer          = nullptr;
    THcHodoscope*       _hod               = nullptr;
    THcCherenkov*       _hgcer             = nullptr;
    THcCherenkov*       _cer               = nullptr;
    THcDC*              _dc                = nullptr;
    std::string         _spectrometer_name = "SHMS";

  public:
    ExperimentMonitorPostProcess(MonitoringDisplay* d);
    ExperimentMonitorPostProcess();
    virtual ~ExperimentMonitorPostProcess();

    virtual Int_t Init(const TDatime&);

    /** Process event. All the MonitoringDisplay's have process invoked.
     *
     */
    virtual Int_t Process(const THaEvData* evt, const THaRunBase*, Int_t code);

    virtual Int_t Close();

    Int_t WritePlots();

    ClassDef(ExperimentMonitorPostProcess, 1)
  };

}


#endif

