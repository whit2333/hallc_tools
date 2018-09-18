#ifndef hallc_calibration_shms_calorimeter_HH
#define hallc_calibration_shms_calorimeter_HH


#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

#include "TMath.h"

#include "calibration/THcPShHit.h"

namespace hallc {
  namespace calibration {

    /**
     * Track class for the SHMS calorimeter calibration.
     * Comprises the spectrometer track parameters and calorimeter hits.
     *
     *
     * Container (collection) of hits and its iterator.
     *
     *typedef vector<THcPShHit*> THcPShHitList;
     *typedef THcPShHitList::iterator THcPShHitIt;
     */
    class THcPShTrack {
    public:

      using THcPShHitList = std::vector<THcPShHit*>;
      using THcPShHitIt   = THcPShHitList::iterator;

      Double_t P;  // track momentum
      Double_t Dp; // track momentum deviation, %/
      Double_t X;  // at the Preshower face
      Double_t Xp; // slope
      Double_t Y;  // at the Preshower face
      Double_t Yp; // slope

      THcPShHitList Hits;

      THcPShTrack();
      THcPShTrack(Double_t p, Double_t dp, Double_t x, Double_t xp, Double_t y, Double_t yp);
      virtual ~THcPShTrack();

      void Reset(Double_t p, Double_t dp, Double_t x, Double_t xp, Double_t y, Double_t yp);

      void AddHit(Double_t adc, Double_t edep, UInt_t blk_number);

      THcPShHit* GetHit(UInt_t k);

      UInt_t GetNhits() { return Hits.size(); };

      void Print(std::ostream& ostrm);

      void SetEs(Double_t* alpha);

      Double_t Enorm();
      Double_t EPRnorm();
      Double_t ESHnorm();

      Double_t GetP() { return P * 1000.; } // MeV

      Double_t GetDp() { return Dp; }

      Double_t GetX() { return X; }
      Double_t GetY() { return Y; }

      Float_t Ycor(Double_t, UInt_t); // coord. corection for Preshower module

      // Coordinate correction constants for Preshower blocks
      //
      static constexpr Double_t fAcor = 106.73;
      static constexpr Double_t fBcor = 2.329;

      // Calorimeter geometry constants.
      //
      static const UInt_t fNrows_pr = 14; // Row number for Preshower
      static const UInt_t fNrows_sh = 16; // Row number for Shower
      static const UInt_t fNcols_pr = 2;  // 2 columns in Preshower
      static const UInt_t fNcols_sh = 14; // 14 columnsin Shower
      static const UInt_t fNpmts_pr = fNrows_pr * fNcols_pr;
      static const UInt_t fNpmts    = fNpmts_pr + fNrows_sh * fNcols_sh;
      ;
    };
  } // namespace calibration
} // namespace hallc

#endif
