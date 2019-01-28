#ifndef hallc_scandalizer_PostProcessors_HH
#define hallc_scandalizer_PostProcessors_HH

#include "THcParmList.h"
#include "CodaDecoder.h"
#include "THaEvData.h"
#include "THaPostProcess.h"
#include "Scandalizer.h"

#include "THcHodoscope.h"
#include "THcHodoscope.h"
#include "THcDC.h"

#include "simple_epics/PVList.h"

#include <functional>
#include <iostream>

#include "podd2/Logger.h"

namespace hallc {
  namespace scandalizer {

    /** Simple  PostProcess implementation with some lambda hooks.
     */
    class SimplePostProcess : public podd2::AnalysisLogging<THaPostProcess> {
    public:
      using Function_t     = std::function<int(const THaEvData*)>;
      using InitFunction_t = std::function<int()>;

      Function_t          _event_lambda;
      InitFunction_t      _init_lambda;
      hcana::Scandalizer* _analyzer = nullptr;

    public:
      SimplePostProcess(Function_t&& f)
          : podd2::AnalysisLogging<THaPostProcess>(), _event_lambda(std::forward<Function_t>(f)), _init_lambda([]() { return 0; }) {}

      SimplePostProcess(InitFunction_t&& initf, Function_t&& f)
          : podd2::AnalysisLogging<THaPostProcess>(), _event_lambda(std::forward<Function_t>(f)),
            _init_lambda(std::forward<InitFunction_t>(initf)) {}

      // ofstream   _output_file;
      // SimplePostProcess()  { }
      virtual ~SimplePostProcess() {}

      virtual Int_t Init(const TDatime&);
      virtual Int_t Process(const THaEvData* evt, const THaRunBase*, Int_t code);
      virtual Int_t Close();

      ClassDef(SimplePostProcess, 1)
    };

    /** Skips a fixed number of events after 1000 pedestal events.  
     */
    class SkipAfterPedestal  : public SimplePostProcess {
    public:

      SkipAfterPedestal(int N_skip = 3000)
          : SimplePostProcess([&]() { return 0; },
                              [=](const THaEvData* evt) {
                                static int counter = 0;
                                if ((evt->GetEvNum() > 1000) && (counter == 0)) {
                                    _analyzer->_skip_events = N_skip;
                                    counter                = 1;
                                }
                                return 0;
                              }) {}
      virtual ~SkipAfterPedestal() {}
    };


    /** Perodically skips a fixed number of events after 1000 pedestal events.  
     *
     */
    class SkipPeriodicAfterPedestal final : public SimplePostProcess {
    public:
      SkipPeriodicAfterPedestal(int N_skip = 3000, int N_process = 1000)
          : SimplePostProcess([&]() { return 0; },
                              [=](const THaEvData* evt) {
                                static int counter = 0;
                                if (evt->GetEvNum() > 2000) {
                                  if (counter == 0) {
                                    _analyzer->_skip_events = N_skip;
                                    counter                = N_process;
                                  } else {
                                    counter--;
                                  }
                                }
                                return 0;
                              }) {}
      virtual ~SkipPeriodicAfterPedestal() {}
    };

    class SpectrometerMonitor : public podd2::AnalysisLogging<THaPostProcess> {
    public:
      hcana::Scandalizer* _analyzer = nullptr;
      THcHodoscope* _hod   = nullptr;
      THcCherenkov* _hgcer = nullptr;
      THcDC*        _dc    = nullptr;
      hallc::PVList _pv_list;
      std::string _spectrometer_name = "SHMS";

    public:
      SpectrometerMonitor(); 
      SpectrometerMonitor(THcHodoscope* phod, THcCherenkov* phgcer, THcDC* pdc);

      virtual ~SpectrometerMonitor() {}

      virtual Int_t Init(const TDatime&);
      virtual Int_t Process(const THaEvData* evt, const THaRunBase*, Int_t code);
      virtual Int_t Close();

      ClassDef(SpectrometerMonitor, 1)
    };


    /** TrackingEfficiencyMonitor.
     *
     * Needs to be constructed with the detectors
     *
     */
    class TrackingEfficiencyMonitor: public  SimplePostProcess {
    public:
      THcHodoscope* _hod   = nullptr;
      THcCherenkov* _hgcer = nullptr;
      THcDC*        _dc    = nullptr;
      hallc::PVList _pv_list;

      /** Construct with detectors.
       *  Detectors need to be given at time of construction.
       */
      TrackingEfficiencyMonitor(THcHodoscope* phod, THcCherenkov* phgcer, THcDC* pdc)
          : SimplePostProcess(
                [&]() { return 0; },
                [=](const THaEvData* evt) {
                  static int    counter             = 0;
                  static double eff_num             = 0.0000001;
                  static double eff_den             = 0.0;
                  static int    n_num               = 0;
                  static int    n_den               = 0;
                  static double dc_multiplicity     = 0.0;
                  static double dc_multiplicity_n   = 0.0000001;
                  static double hod_multiplicity     = 0.0;
                  static double hod_multiplicity_n   = 0.0000001;
                  int           shmsDC1Planes_nhits = 0;
                  int           shmsDC2Planes_nhits = 0;
                  for (int ip = 0; ip < 6; ip++) {
                    shmsDC1Planes_nhits += _dc->GetPlane(ip)->GetNHits();
                  }
                  for (int ip = 6; ip < 12; ip++) {
                    shmsDC2Planes_nhits += _dc->GetPlane(ip)->GetNHits();
                  }
                  hod_multiplicity += _hod->GetTotHits();
                  hod_multiplicity_n += 1.0;

                  int shms_DC_nhits = shmsDC1Planes_nhits + shmsDC2Planes_nhits;
                  bool shms_DC_too_many_hits =
                      (shmsDC1Planes_nhits > 8) || (shmsDC2Planes_nhits > 8);
                  double beta                = _hod->GetBetaNotrk();
                  bool   good_beta           = beta > 0.4;
                  bool   shms_good_hodoscope = _hod->fGoodScinHits;
                  bool   good_ntracks        = (_dc->GetNTracks() > 0);
                  bool   good_hgc            = _hgcer->GetCerNPE() > 1;

                  if (good_beta && shms_good_hodoscope) {
                    dc_multiplicity += shms_DC_nhits;
                    dc_multiplicity_n += 1.0;
                  }
                  if ((good_beta && shms_good_hodoscope) && (!shms_DC_too_many_hits) && good_hgc) {
                    eff_den = eff_den + 1.0;
                    n_den++;
                    if (good_ntracks) {
                      eff_num = eff_num + 1.0;
                      n_num++;
                    }
                  }
                  if ((evt->GetEvNum() > 1200) && (counter > 2000)) {
                    _ana_logger->debug("tracking efficiency : {}/{} = {}", eff_num, eff_den,
                                       eff_num / eff_den);
                    _ana_logger->debug("dc multiplicity : {}/{} = {}", dc_multiplicity, dc_multiplicity_n,
                                       dc_multiplicity / dc_multiplicity_n);
                    _ana_logger->debug("hod multiplicity : {}/{} = {}", hod_multiplicity , hod_multiplicity_n,
                                       hod_multiplicity / hod_multiplicity_n);
                    _pv_list.Put("hcSHMS:Hod:Mult", hod_multiplicity / hod_multiplicity_n);
                    _pv_list.Put("hcSHMSDCMultiplicity", dc_multiplicity / dc_multiplicity_n);
                    _pv_list.Put("hcSHMSTrackingEff", eff_num / eff_den);
                    _pv_list.Put("hcSHMSTrackingEff:Unc",
                                 std::sqrt(double(n_num)) / (n_num + n_den + 0.0000001));
                    eff_num = 0.000000001;
                    eff_den = 0.0;
                    counter = 0;
                    dc_multiplicity = 0.0000001;
                    dc_multiplicity_n = 0.0000001;
                    hod_multiplicity = 0.0000001;
                    hod_multiplicity_n = 0.0000001;
                  }
                  counter++;
                  return 0;
                }),
            _hod(phod), _hgcer(phgcer), _dc(pdc) {
        if (!_hod) {
          _ana_logger->error("hodoscope not defined");
          //std::cerr << "error:  Hodoscope 
        }
        if (!_hgcer) {
          _ana_logger->error("hgcer not defined");
        }
        if (!_dc) {
          _ana_logger->error("drift chamber not defined");
        }
        std::vector<std::string> pvs = {"hcSHMS:Hod:Mult","hcSHMSDCMultiplicity","hcSHMSTrackingEff", "hcSHMSTrackingEff:Unc",
                                        "hcSHMSTrackingEff.LOW", "hcSHMSTrackingEff.LOLO"};
        for (const auto& n : pvs) {
          _pv_list.AddPV(n);
        }
      }
      virtual ~TrackingEfficiencyMonitor(){}

      ClassDef(TrackingEfficiencyMonitor, 1)
    };
  }
}


#endif

