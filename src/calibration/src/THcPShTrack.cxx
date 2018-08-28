#include "calibration/THcPShTrack.h"
#include "TMath.h"
#include "calibration/THcPShHit.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

using namespace std;

namespace hallc {
  namespace calibration {

    // Track class for the SHMS calorimeter calibration.
    // Comprises the spectrometer track parameters and calorimeter hits.

    // Container (collection) of hits and its iterator.
    //
    using THcPShHitList = vector<THcPShHit*>;
    using THcPShHitIt   = THcPShHitList::iterator;

    THcPShTrack::THcPShTrack(){};

    THcPShTrack::THcPShTrack(Double_t p, Double_t dp, Double_t x, Double_t xp, Double_t y,
                             Double_t yp) {
      P  = p;
      Dp = dp;
      X  = x;
      Xp = xp;
      Y  = y;
      Yp = yp;
    };

    //------------------------------------------------------------------------------

    void THcPShTrack::Reset(Double_t p, Double_t dp, Double_t x, Double_t xp, Double_t y,
                            Double_t yp) {

      // Reset track parameters, clear hit list.

      P  = p;
      Dp = dp;
      X  = x;
      Xp = xp;
      Y  = y;
      Yp = yp;
      Hits.clear();
    };

    //------------------------------------------------------------------------------

    void THcPShTrack::AddHit(Double_t adc, Double_t edep, UInt_t blk_number) {

      // Add a hit to the hit list.

      THcPShHit* hit = new THcPShHit(adc, blk_number);
      hit->SetEdep(edep);
      Hits.push_back(hit);
    };

    //------------------------------------------------------------------------------

    THcPShHit* THcPShTrack::GetHit(UInt_t k) {
      THcPShHitIt it = Hits.begin();
      for (UInt_t i = 0; i < k; i++)
        it++;
      return *it;
    }

    //------------------------------------------------------------------------------

    void THcPShTrack::Print(ostream& ostrm) {

      // Output the track parameters and hit list through the stream ostrm.

      ostrm << P << " " << Dp << " " << X << " " << Xp << " " << Y << " " << Yp << " "
            << Hits.size() << endl;

      for (THcPShHitIt iter = Hits.begin(); iter != Hits.end(); iter++) {
        (*iter)->Print(ostrm);
      };
    };

    //------------------------------------------------------------------------------

    THcPShTrack::~THcPShTrack() {
      for (THcPShHitIt i = Hits.begin(); i != Hits.end(); ++i) {
        delete *i;
        *i = 0;
      }
    };

    //------------------------------------------------------------------------------

    void THcPShTrack::SetEs(Double_t* alpha) {

      // Set hit energy depositions by use of calibration (gain) constants alpha.

      for (THcPShHitIt iter = Hits.begin(); iter != Hits.end(); iter++) {

        Double_t adc  = (*iter)->GetADC();
        UInt_t   nblk = (*iter)->GetBlkNumber();

        if (nblk <= fNrows_pr * fNcols_pr) {
          // Preshower block, correct for Y coordinate
          UInt_t ncol = 1;
          if (nblk > fNrows_pr)
            ncol = 2;
          (*iter)->SetEdep(adc * Ycor(Y, ncol) * alpha[nblk - 1]);
        } else
          // Shower block, no coordinate correction.
          (*iter)->SetEdep(adc * alpha[nblk - 1]);
      };
    }

    //------------------------------------------------------------------------------

    Double_t THcPShTrack::Enorm() {

      // Normalized to the track momentum energy depostion in the calorimeter.

      Double_t sum = 0;

      for (THcPShHitIt iter = Hits.begin(); iter != Hits.end(); iter++) {
        sum += (*iter)->GetEdep();
      };

      return sum / P / 1000.; // Momentum in MeV.
    }

    //------------------------------------------------------------------------------

    Double_t THcPShTrack::EPRnorm() {

      // Normalized to the track momentum energy depostion in Preshower.

      Double_t sum = 0;

      for (THcPShHitIt iter = Hits.begin(); iter != Hits.end(); iter++) {
        if ((*iter)->GetBlkNumber() <= fNpmts_pr)
          sum += (*iter)->GetEdep();
      };

      return sum / P / 1000.; // Momentum in MeV.
    }

    //------------------------------------------------------------------------------

    Double_t THcPShTrack::ESHnorm() {

      // Normalized to the track momentum energy depostion in Shower.

      Double_t sum = 0;

      for (THcPShHitIt iter = Hits.begin(); iter != Hits.end(); iter++) {
        if ((*iter)->GetBlkNumber() > fNpmts_pr)
          sum += (*iter)->GetEdep();
      };

      return sum / P / 1000.; // Momentum in MeV.
    }

    //------------------------------------------------------------------------------

    // Coordinate correction for Preshower modules.
    // Fit to GEANT pion data @ 5 GeV/c (Simon).

    Float_t THcPShTrack::Ycor(Double_t yhit, UInt_t ncol) {

      Float_t cor;

      // Warn if hit does not belong to Preshower.
      //
      if (ncol > fNcols_pr || ncol < 1)
        cout << "*** THcPShTrack::Ycor: wrong ncol = " << ncol << " ***" << endl;

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
