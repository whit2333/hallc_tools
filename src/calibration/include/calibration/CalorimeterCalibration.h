#ifndef hallc_calibration_CalorimeterCalibration_HH
#define hallc_calibration_CalorimeterCalibration_HH

#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace hallc {
  namespace calibration {

    struct CalorimeterCalibration {

      // Coordinate correction constants for Preshower blocks
      static constexpr double fAcor = 106.73;
      static constexpr double fBcor = 2.329;

      // Calorimeter geometry constants.
      static const unsigned int fNrows_pr = 14; // Row number for Preshower
      static const unsigned int fNrows_sh = 16; // Row number for Shower
      static const unsigned int fNcols_pr = 2;  // 2 columns in Preshower
      static const unsigned int fNcols_sh = 14; // 14 columnsin Shower
      static const unsigned int fNpmts_pr = fNrows_pr * fNcols_pr;
      static const unsigned int fNpmts    = fNpmts_pr + fNrows_sh * fNcols_sh;

      // Quantities for calculations of the calibration constants.
      double                                         fe0;
      std::array<double, fNpmts>                     fqe;
      std::array<double, fNpmts>                     fq0;
      std::array<std::array<double, fNpmts>, fNpmts> fQ;
      std::array<double, fNpmts>                     falphaU; // 'unconstrained' calib. constants
      std::array<double, fNpmts>                     falphaC; // the sought calibration constants
      std::array<double, fNpmts>                     falpha0; // initial gains
      std::array<double, fNpmts>                     falpha1; // unit gains
      std::array<unsigned int, fNpmts>               fHitCount;

      double fDeltaMin    = 0.;
      double fDeltaMax    = 1.;
      double fBetaMin     = 0.;
      double fBetaMax     = 2.;
      double fHGCerMin    = 999.;
      double fNGCerMin    = 999.;
      double fMinHitCount = 999999;

      std::vector<double> neg_gain_cor;
      std::vector<double> pos_gain_cor;
      std::vector<double> arr_gain_cor;

      std::string input_cal_file_name;
      std::string output_cal_file_name;
      int         run_number = 0;

      double       fEuncLoLo;
      double       fEuncHiHi; // Range of uncalibrated Edep histogram
      unsigned int fEuncNBin; // Binning of uncalibrated Edep histogram
      double       fEuncGFitLo;
      double       fEuncGFitHi; // Gaussian fit range of uncalib. Edep histo.

      CalorimeterCalibration(){}
      CalorimeterCalibration(int rn);

      CalorimeterCalibration(const CalorimeterCalibration&) = default;

      void ReadLegacyCalibration(const std::string& fname = "input.dat" );

      /** Load the calibration constants for a specific run.
       */
      void LoadJsonCalibration(const std::string& fname = "pcal_calib.json", int run_number = 0);

      /** Load the calibration constants for a specific run.
       *  Takes the current values and builds a json object string for the main database file.
       */
      std::string PrepareJson() const;

      void BuildTester(int run_num) const;

      void Print() const {
        std::cout << " fDeltaMin    " << fDeltaMin << "\n";
        std::cout << " fDeltaMax    " << fDeltaMax << "\n";
        std::cout << " fBetaMin     " << fBetaMin     << "\n";
        std::cout << " fBetaMax     " << fBetaMax     << "\n";
        std::cout << " fHGCerMin    " << fHGCerMin    << "\n";
        std::cout << " fNGCerMin    " << fNGCerMin    << "\n";
        std::cout << " fMinHitCount " << fMinHitCount << "\n";
        std::cout << " input_cal_file_name  " << input_cal_file_name << "\n";
        std::cout << " output_cal_file_name " << output_cal_file_name << "\n";
        std::cout << " run_number           " << run_number << "\n";


      }

    };
  }
} // namespace hallc

#endif
