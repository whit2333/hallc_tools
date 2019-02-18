#ifndef hallc_scandalizer_YieldMonitors_HH
#define hallc_scandalizer_YieldMonitors_HH

#include "CodaDecoder.h"
#include "Scandalizer.h"
#include "THaEvData.h"
#include "THaPostProcess.h"
#include "THcParmList.h"
#include "THcScalerEvtHandler.h"

#include "scandalizer/SpectrometerMonitor.h"

#include "simple_epics/PVList.h"
#include <functional>
#include <iostream>

#include "podd2/Logger.h"

namespace hallc {
  namespace scandalizer {

    class YieldMonitor : public podd2::AnalysisLogging<THaPostProcess> {
    protected:
      THcScalerEvtHandler* _scaler      = nullptr;
      std::string          _scaler_name = "H.pEL_CLEAN.scaler";
      std::string          _charge_name = "H.BCM2.scalerCharge";
      std::string          _yield_name  = "SHMS:EL_CLEAN";
      int                  _ivar = -1;
      int                  _icharge = -1;
      hallc::PVList       _pv_list;

      std::map<std::string, int> scalers_wanted;

      double last_charge = 0;
      int    last_scaler = 0;
      int    counter     = 0;

    public:
      YieldMonitor(std::string name = "SHMS:EL_CLEAN") : _yield_name(name) {}

      YieldMonitor(THcScalerEvtHandler* scaler, 
                   std::string name        = "SHMS:EL_CLEAN",
                   std::string scaler_name = "H.pEL_CLEAN.scaler",
                   std::string charge_name = "H.BCM2.scalerCharge")
          : _yield_name(name), _scaler(scaler), _scaler_name(scaler_name),_charge_name(charge_name) {}

      virtual ~YieldMonitor() {}

      virtual Int_t Init(const TDatime&);
      virtual Int_t Process(const THaEvData* evt, const THaRunBase*, Int_t code);
      virtual Int_t Close();

      ClassDef(YieldMonitor,1)
    };


  } // namespace scandalizer
} // namespace hallc

#endif
