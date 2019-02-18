#include "scandalizer/SpectrometerMonitor.h"

namespace hallc {
  namespace scandalizer {
      SpectrometerMonitor::SpectrometerMonitor()
          : podd2::AnalysisLogging<THaPostProcess>(), _hod(nullptr), _hgcer(nullptr), _dc(nullptr) {
      }

      SpectrometerMonitor::SpectrometerMonitor(THcHodoscope* phod, THcCherenkov* phgcer, THcDC* pdc)
          : podd2::AnalysisLogging<THaPostProcess>(), _hod(phod), _hgcer(phgcer), _dc(pdc) {}

      Int_t SpectrometerMonitor::Init(const TDatime& ) {
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

//hcSHMS:DC:Mult
//hcSHMS:DC:Occupancy
//hcSHMS:TrackingEff
//hcSHMS:TrackingEff:Unc
//hcSHMS:Hod:Mult
//hcSHMS:incl:e:yields
//hcSHMS:incl:pion:yields
        std::vector<std::string> pvs = {":DC:Mult",         ":Hod:Mult",        ":DC:Occupancy",
                                        ":TrackingEff",     ":TrackingEff:Unc", ":TrackingEff.LOW",
                                        ":TrackingEff.LOLO"};
        for (const auto& n : pvs) {
          _pv_list.AddPV(std::string("hc")  + _spectrometer_name + n);
        }
        _ana_logger->info("Init for SpectrometerMonitor {}", _spectrometer_name);
        //_event_lambda(evt);
        //_output_file << " Event : " << evt->GetEvNum() << "  ( " << evt->GetEvType() << ")\n";
        //if( evt->GetEvNum()%100 == 0) {
        //  gSystem->ProcessEvents();
        //  //_output_file.flush();
        //}
        return 0;
      }

      Int_t SpectrometerMonitor::Process( const THaEvData* evt, const THaRunBase*, Int_t code ){ 
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
          /// \todo: fix this PV naming code
          _pv_list.Put(std::string("hc")  + _spectrometer_name+ ":Hod:Mult", hod_multiplicity / hod_multiplicity_n);
          _pv_list.Put(std::string("hc")  + _spectrometer_name+ ":DC:Mult", dc_multiplicity / dc_multiplicity_n);
          _pv_list.Put(std::string("hc")  + _spectrometer_name+ ":TrackingEff", eff_num / eff_den);
          _pv_list.Put(std::string("hc")  + _spectrometer_name+ ":TrackingEff:Unc",
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
        //if( evt->GetEvNum()%100 == 0) {
        //  gSystem->ProcessEvents();
        //  //_output_file.flush();
        //}
        return 0;
      }

      Int_t SpectrometerMonitor::Close(){ 
        std::cout << "close\n";
        //_output_file.flush();
        //_output_file.close();
        return 0; 
      }
  }
}

