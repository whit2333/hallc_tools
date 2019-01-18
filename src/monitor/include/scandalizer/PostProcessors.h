#ifndef hallc_scandalizer_PostProcessors_HH
#define hallc_scandalizer_PostProcessors_HH

#include "THcParmList.h"
#include "CodaDecoder.h"
#include "THaEvData.h"
#include "THaPostProcess.h"
#include "Scandalizer.h"

#include <functional>
#include <iostream>

namespace hallc {
  namespace scandalizer {

    /** Simple  PostProcess implementation with some lambda hooks.
     */
    class SimplePostProcess : public THaPostProcess {
    public:
      using Function_t     = std::function<int(const THaEvData*)>;
      using InitFunction_t = std::function<int()>;
      Function_t     _event_lambda;
      InitFunction_t _init_lambda;

      hcana::Scandalizer*   _analyzer = nullptr;

    public:
      SimplePostProcess(Function_t&& f) : 
        _event_lambda(std::forward<Function_t>(f)), 
        _init_lambda([](){return 0;}) { }

      SimplePostProcess(InitFunction_t&& initf, Function_t&& f) : 
        _event_lambda(std::forward<Function_t>(f)), 
        _init_lambda(std::forward<InitFunction_t>(initf)) { }

      //ofstream   _output_file;
      //SimplePostProcess()  { }
      virtual ~SimplePostProcess(){ }

      virtual Int_t Init(const TDatime& ) ;
      virtual Int_t Process( const THaEvData* evt, const THaRunBase*, Int_t code );
      virtual Int_t Close();

      ClassDef(SimplePostProcess,1)
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

    class SkipPeriodicAfterPedestal  : public SimplePostProcess {
    public:
      SkipPeriodicAfterPedestal(int N_skip = 3000)
          : SimplePostProcess([&]() { return 0; },
                              [=](const THaEvData* evt) {
                                static int counter = 0;
                                if (evt->GetEvNum() > 2000) {
                                  if (counter == 0) {
                                    _analyzer->_skip_events = N_skip;
                                    counter                = 1000;
                                  } else {
                                    counter--;
                                  }
                                }
                                return 0;
                              }) {}
      virtual ~SkipPeriodicAfterPedestal() {}
    };
  //SimplePostProcess* pp1 = new SimplePostProcess(
  //  [&](){
  //    return 0;
  //  },
  //  [&](const THaEvData* evt){
  //    static int counter = 0;
  //    static double eff_num             = 0.0000001;
  //    static double eff_den             = 0.0;
  //    static int    n_num               = 0;
  //    static int    n_den               = 0;
  //    int           shmsDC1Planes_nhits = 0;
  //    int           shmsDC2Planes_nhits = 0;
  //    for (int ip = 0; ip < 6; ip++) {
  //      shmsDC1Planes_nhits += pdc->GetPlane(ip)->GetNHits();
  //    }
  //    for (int ip = 6; ip < 12; ip++) {
  //      shmsDC2Planes_nhits += pdc->GetPlane(ip)->GetNHits();
  //    }
  //    bool   shms_DC_too_many_hits = (shmsDC1Planes_nhits > 8) || (shmsDC2Planes_nhits > 8);
  //    double beta                  = phod->GetBetaNotrk();
  //    bool   good_beta             = beta > 0.4;
  //    bool   shms_good_hodoscope   = phod->fGoodScinHits;
  //    bool   good_ntracks          = (pdc->GetNTracks() > 0);
  //    bool   good_hgc              = phgcer->GetCerNPE() > 1;
  //    if ((good_beta && shms_good_hodoscope) && (!shms_DC_too_many_hits) && good_hgc) {
  //      eff_den = eff_den + 1.0;
  //      n_den++;
  //      if (good_ntracks) {
  //        eff_num = eff_num + 1.0;
  //        n_num++;
  //      }
  //    }
  //    if ((evt->GetEvNum() > 1200) && (counter > 1000)) {
  //      std::cout << " efficiency :  " << eff_num / eff_den << "\n";
  //      //std::cout << " Event : " << evt->GetEvNum() << "  ( " << evt->GetEvType() << ")\n";
  //      pv_list.Put("hcSHMSTrackingEff",eff_num/eff_den);
  //      pv_list.Put("hcSHMSTrackingEff:Unc",std::sqrt(double(n_num))/(n_num+n_den+0.0000001));
  //      eff_num                = 0.000000001;
  //      eff_den                = 0.0;
  //      //analyzer->_skip_events = 300;
  //      counter = 0;
  //    }
  //    counter++;
  //    return 0; 
  //  });
  //}


}
}


#endif

