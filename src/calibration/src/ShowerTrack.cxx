#include "calibration/ShowerTrack.h"
//#include "calibration/THcPShHit.h"
#include "calibration/CalorimeterCalibration.h"

#include "TMath.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

#define D_CALO_FP 292.64

using namespace std;

namespace hallc {
  namespace calibration {

    // Track class for the SHMS calorimeter calibration.
    // Comprises the spectrometer track parameters and calorimeter hits.

    ShowerTrack::ShowerTrack(){}

    ShowerTrack::ShowerTrack(double p, double dp, double x, double xp, double y, double yp)
        : _track({p, dp, x + xp*D_CALO_FP, xp, y+ yp*D_CALO_FP, yp}) {}
    ShowerTrack::ShowerTrack(double p, double dp, double x, double xp, double y, double yp,
                             CalorimeterCalibration* cal)
        : _track({p, dp, x + xp*D_CALO_FP, xp, y+ yp*D_CALO_FP, yp}), _calibration(cal) {}

    ShowerTrack::ShowerTrack(const ShowerTrackInfo& t, CalorimeterCalibration* cal )
        : _track(t), _calibration(cal) {}

    //------------------------------------------------------------------------------

    void ShowerTrack::AddHit(uint64_t blk_number, double adc) {
        double   y_cor     = GetYcor(blk_number);
        double   gain_coef = 1.0;
        if (_calibration) {
          gain_coef = _calibration->GetGainCoeff(blk_number);
        }
        double  energy       = adc * y_cor * gain_coef;
        _Hits.push_back(CalHit{blk_number, adc, energy});
    }

    void ShowerTrack::AddHit(double adc, double edep, uint64_t blk_number) {
      // Add a hit to the hit list.
      //THcPShHit* hit = new THcPShHit(adc, blk_number);
      //hit->SetEdep(edep);
      _Hits.push_back(CalHit{blk_number,adc,edep});
    }

    //------------------------------------------------------------------------------

    CalHit ShowerTrack::GetHit(uint64_t k) {
      return _Hits.at(k);
    }

    //------------------------------------------------------------------------------

    void ShowerTrack::Print(ostream& ostrm) const {
      ostrm << "Track variables : " << _track._P << " " << _track._Dp << " " << _track._X << " " << _track._Xp << " " << _track._Y << " " << _track._Yp << " "
            << _Hits.size() << "\n";
      for (const auto& ahit : _Hits) {
        ostrm << "(" << ahit._channel << ", " << ahit._adc << ", " << ahit._energy << ")\n";
      }
    }

    //------------------------------------------------------------------------------

    double ShowerTrack::GetYcor(uint64_t block) const {
      if (block < fNrows_pr) {
        return Ycor(_track._Y, 1);
      } else if (block < 2 * fNrows_pr) {
        return Ycor(_track._Y, 2);
      }
      return 1.0;
    }

    ShowerTrack  ShowerTrack::UpdatedTrack(CalorimeterCalibration* cc) const {
      // If cc is null, unity gain coeffs are used.

      ShowerTrack new_trk(*this);
      // Set hit energy depositions by use of calibration (gain) constants alpha.
      //for (THcPShHitIt iter = Hits.begin(); iter != Hits.end(); iter++) {
      for (auto& ahit : new_trk._Hits) {
        double   adc       = ahit._adc;
        uint64_t nblk      = ahit._channel;
        double   y_cor     = GetYcor(nblk);
        double   gain_coef = 1.0;
        if(cc) {
          gain_coef = cc->GetGainCoeff(nblk);
        }
        ahit._energy       = adc * y_cor * gain_coef;
      }
      return new_trk;
    }

    //------------------------------------------------------------------------------

    double ShowerTrack::Enorm() const {

      // Normalized to the track momentum energy depostion in the calorimeter.

      double sum = 0;
      for (const auto& ahit : _Hits) {
        sum+= ahit._energy;
      }
      return sum / _track._P / 1000.; // Momentum in MeV.
    }

    //------------------------------------------------------------------------------

    double ShowerTrack::EPRnorm() const {

      // Normalized to the track momentum energy depostion in Preshower.
      double sum = 0;
      for (auto it = _Hits.cbegin(); it != _Hits.cbegin()+fNpmts_pr; it++) {
        sum+= (*it)._energy;
      }
      return sum / _track._P / 1000.; // Momentum in MeV.
    }

    //------------------------------------------------------------------------------

    double ShowerTrack::ESHnorm() const {

      // Normalized to the track momentum energy depostion in Shower.

      double sum = 0;
      for (auto it = _Hits.cbegin() + fNpmts_pr; it != _Hits.cend(); it++) {
        sum+= (*it)._energy;
      }
      return sum / _track._P / 1000.; // Momentum in MeV.
    }

    //------------------------------------------------------------------------------

    // Coordinate correction for Preshower modules.
    // Fit to GEANT pion data @ 5 GeV/c (Simon).
    float ShowerTrack::Ycor(double yhit, uint64_t ncol) const {
      float cor;
      // Warn if hit does not belong to Preshower.
      //
      if (ncol > fNcols_pr || ncol < 1)
        cout << "*** ShowerTrack::Ycor: wrong ncol = " << ncol << " ***" << endl;

      // Check if the hit coordinate matches the fired block's column.
      //
      if ((yhit < 0. && ncol == 2) || (yhit > 0. && ncol == 1))
        cor = 1. / (1. + TMath::Power(TMath::Abs(yhit) / fAcor, fBcor));
      else
        cor = 1.;

      // Debug output.
      //  cout << "THcShTrack::Ycor = " << cor << "  yhit = " << yhit
      //       << "  ncol = " << ncol << endl;

      return cor;
    }
  } // namespace calibration
} // namespace hallc
