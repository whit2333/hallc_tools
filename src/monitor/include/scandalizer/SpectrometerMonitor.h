#ifndef hallc_scandalizer_SpectrometerMonitors_HH
#define hallc_scandalizer_SpectrometerMonitors_HH

#include "THcParmList.h"
#include "CodaDecoder.h"
#include "THaEvData.h"
#include "THaPostProcess.h"
#include "Scandalizer.h"

#include "THcHodoscope.h"
#include "THcCherenkov.h"
#include "THcDC.h"

#include "simple_epics/PVList.h"

#include <functional>
#include <iostream>

#include "podd2/Logger.h"

namespace hallc {
  namespace scandalizer {

    class SpectrometerMonitor : public podd2::AnalysisLogging<THaPostProcess> {
    public:
      hcana::Scandalizer* _analyzer = nullptr;
      THcHodoscope*       _hod      = nullptr;
      THcCherenkov*       _hgcer    = nullptr;
      THcDC*              _dc       = nullptr;
      hallc::PVList       _pv_list;
      std::string         _spectrometer_name = "SHMS";

      using Function_t     = std::function<int(const THaEvData*)>;
      using InitFunction_t = std::function<int()>;

    public:
      SpectrometerMonitor(); 
      SpectrometerMonitor(THcHodoscope* phod, THcCherenkov* phgcer, THcDC* pdc);

      virtual ~SpectrometerMonitor() {}

      virtual Int_t Init(const TDatime&);
      virtual Int_t Process(const THaEvData* evt, const THaRunBase*, Int_t code);
      virtual Int_t Close();

      ClassDef(SpectrometerMonitor, 1)
    };

  }
}

#endif
