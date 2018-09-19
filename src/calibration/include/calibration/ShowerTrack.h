#ifndef hallc_calibration_ShowerTrack_HH
#define hallc_calibration_ShowerTrack_HH


#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

#include "TMath.h"
#include "TObject.h"

#include "calibration/THcPShHit.h"

namespace hallc {
  namespace calibration {

    class CalorimeterCalibration;

    struct ShowerTrackInfo {
    //public:
      double _P  = 0.0; // track momentum
      double _Dp = 0.0; // track momentum deviation, %/
      double _X  = 0.0; // at the Preshower face
      double _Xp = 0.0; // slope
      double _Y  = 0.0; // at the Preshower face
      double _Yp = 0.0; // slope
      //ClassDef(ShowerTrackInfo,1)
    };
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
    class ShowerTrack   {
    public:
      //double _P  = 0.0; // track momentum
      //double _Dp = 0.0; // track momentum deviation, %/
      //double _X  = 0.0; // at the Preshower face
      //double _Xp = 0.0; // slope
      //double _Y  = 0.0; // at the Preshower face
      //double _Yp = 0.0; // slope
      ShowerTrackInfo         _track;
      CalorimeterCalibration* _calibration = nullptr;
      std::vector<CalHit>     _Hits;

      // Coordinate correction constants for Preshower blocks
      static constexpr double fAcor = 106.73;
      static constexpr double fBcor = 2.329;

      // Calorimeter geometry constants.
      //
      static const UInt_t fNrows_pr = 14; // Row number for Preshower
      static const UInt_t fNrows_sh = 16; // Row number for Shower
      static const UInt_t fNcols_pr = 2;  // 2 columns in Preshower
      static const UInt_t fNcols_sh = 14; // 14 columnsin Shower
      static const UInt_t fNpmts_pr = fNrows_pr * fNcols_pr;
      static const UInt_t fNpmts    = fNpmts_pr + fNrows_sh * fNcols_sh;

      ShowerTrack();
      ShowerTrack(const ShowerTrack&) = default;
      ShowerTrack(const ShowerTrackInfo&, CalorimeterCalibration* cal = nullptr );
      ShowerTrack(double p, double dp, double x, double xp, double y, double yp);
      ShowerTrack(double p, double dp, double x, double xp, double y, double yp, CalorimeterCalibration* cal);
      virtual ~ShowerTrack() = default;

      void Reset(double p, double dp, double x, double xp, double y, double yp);

      void AddHit(uint64_t blk_number, double adc);
      void AddHit(double adc, double edep, uint64_t blk_number);

      CalHit GetHit(uint64_t k);

      uint64_t GetNhits() const { return _Hits.size(); };

      const std::vector<CalHit>& GetHits() const { return _Hits; };

      void Print(std::ostream& ostrm = std::cout) const;

      ShowerTrack UpdatedTrack(CalorimeterCalibration* cc) const;


      double Enorm() const ;
      double EPRnorm()const;
      double ESHnorm() const;

      double GetP() const { return _track._P * 1000.; } // MeV
      double GetDp() const { return _track._Dp; }
      double GetX() const { return _track._X; }
      double GetY() const { return _track._Y; }

      double GetYcor(uint64_t block) const;
      float  Ycor(double, uint64_t) const; // coord. corection for Preshower module
    };
  } // namespace calibration
} // namespace hallc

#endif
