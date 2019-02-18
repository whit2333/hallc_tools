#include "scandalizer/YieldMonitors.h"

namespace hallc {
  namespace scandalizer {

    Int_t YieldMonitor::Init(const TDatime&) {
      if (!_scaler) {
        _ana_logger->error("scaler not defined");
      }
      _pv_list.AddPV(fmt::format("hcYield:{}:Count",  _yield_name));
      _pv_list.AddPV(fmt::format("hcYield:{}:Charge", _yield_name));
      //_ana_logger->info("Init for YieldMonitor {}", _yield_name);
      //_event_lambda(evt);
      //_output_file << " Event : " << evt->GetEvNum() << "  ( " << evt->GetEvType() << ")\n";
      // if( evt->GetEvNum()%100 == 0) {
      //  gSystem->ProcessEvents();
      //  //_output_file.flush();
      //}
      if( (_ivar == -1) || (_icharge == -1)) {
      for (auto aloc : _scaler->scalerloc) {
        if (aloc) {
          std::cout << aloc->name << "\n";
          if (_scaler_name == aloc->name.Data()) {
            _ana_logger->info("found scaler named {} for pv yield monitor {}", _scaler_name,
                              _yield_name);
            _ivar = aloc->ivar;
            // scalers_wanted[_scaler_name] = ivar;
          }
          if (_charge_name == aloc->name.Data()) {
            _ana_logger->info("found charge scaler named {} for pv yield monitor {}", _charge_name,
                              _yield_name);
            _icharge = aloc->ivar;
            // scalers_wanted[_scaler_name] = ivar;
          }
         }
        }
      }
      return 0;
    }

    Int_t YieldMonitor::Process(const THaEvData* evt, const THaRunBase*, Int_t code) {
      int           pEL_CLEAN   = 0;
      if (_ivar != -1) {
        pEL_CLEAN = _scaler->dvars[_ivar];
      }
      double BCM2 = 0.0;
      if (_icharge != -1) {
        BCM2 = _scaler->dvars[_icharge];
      }
      if ((evt->GetEvNum() > 1200) && (counter > 9000)) {
        //pv_list.Put("hcYield:SHMS:EL_CLEAN:Count" , pEL_CLEAN - last_scaler);
        //pv_list.Put("hcYield:SHMS:EL_CLEAN:charge", BCM2 - last_charge     );
        double charge_diff =  BCM2 - last_charge;
        //std::cout << pEL_CLEAN << " - " <<  last_scaler << " = " << pEL_CLEAN - last_scaler << " counts\n";
        //std::cout << BCM2 << " - " <<  last_charge << " = " << charge_diff << " C\n";
        _pv_list.Put(fmt::format("hcYield:{}:Count",  _yield_name), pEL_CLEAN - last_scaler );
        _pv_list.Put(fmt::format("hcYield:{}:Charge", _yield_name), BCM2 - last_charge      );
        counter = 0;
        last_charge = BCM2;
        last_scaler = pEL_CLEAN;
      }
      counter++;
      return 0;
    }

    Int_t YieldMonitor::Close() {
      std::cout << "close\n";
      //_output_file.flush();
      //_output_file.close();
      return 0;
    }
  } // namespace scandalizer
} // namespace hallc
