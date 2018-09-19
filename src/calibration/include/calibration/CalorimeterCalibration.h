#ifndef hallc_calibration_CalorimeterCalibration_HH
#define hallc_calibration_CalorimeterCalibration_HH

#include <array>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>

namespace hallc {
  namespace calibration {

    template<size_t NShowerRows = 16, size_t NShowerCols = 14, size_t NPreshowerRows = 14>
    struct CalorimeterConfig {
      static const size_t N_rows_shower    = NShowerRows;
      static const size_t N_cols_shower    = NShowerCols;
      static const size_t N_rows_preshower = NPreshowerRows;
      static const size_t N_cols_preshower = 2;
    };

    struct CalorimeterCalibration {

      // Coordinate correction constants for Preshower blocks
      //static constexpr double fAcor = 106.73;
      //static constexpr double fBcor = 2.329;
      using Config = CalorimeterConfig<16,14,14>;
      // Calorimeter geometry constants.
      static const unsigned int fNrows_pr = Config::N_rows_preshower;
      static const unsigned int fNcols_pr = 2;
      static const unsigned int fNrows_sh = Config::N_rows_shower;
      static const unsigned int fNcols_sh = Config::N_cols_shower;
      static const unsigned int fNpmts_pr = fNrows_pr * fNcols_pr;
      static const unsigned int fNpmts    = fNpmts_pr + fNrows_sh * fNcols_sh;

      // Quantities for calculations of the calibration constants.
      //  do these need to be here?
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

      std::string input_cal_file_name  = "pcal_calib.json";
      std::string output_cal_file_name = "pcal_calib_new.json";
      int         run_number = 0;

      double       fEuncLoLo;
      double       fEuncHiHi; // Range of uncalibrated Edep histogram
      unsigned int fEuncNBin; // Binning of uncalibrated Edep histogram
      double       fEuncGFitLo;
      double       fEuncGFitHi; // Gaussian fit range of uncalib. Edep histo.

    public:
      CalorimeterCalibration(){}
      CalorimeterCalibration(int rn);
      CalorimeterCalibration(const CalorimeterCalibration&) = default;

      double GetGainCoeff(uint64_t block) const {
        if (block < Config::N_rows_preshower) {
          return neg_gain_cor.at(block);
        } else if (block < 2 * Config::N_rows_preshower) {
          return pos_gain_cor.at(block - Config::N_rows_preshower);
        } else if (block <
                   2 * Config::N_rows_preshower + Config::N_rows_shower * Config::N_cols_shower) {
          return arr_gain_cor.at(block - 2 * Config::N_rows_preshower);
        }
        return 0.0;
      }

      void SetGainCoeffs(const std::array<double,fNpmts>& coeffs) {
        neg_gain_cor.clear();
        pos_gain_cor.clear();
        arr_gain_cor.clear();
        std::copy(coeffs.begin(), 
                  coeffs.begin() + fNrows_pr, 
                  std::back_inserter(neg_gain_cor));
        std::copy(coeffs.begin() + fNrows_pr,
                  coeffs.begin() + fNpmts_pr,
                  std::back_inserter(pos_gain_cor));
        std::copy(coeffs.begin() + fNpmts_pr, 
                  coeffs.end(), 
                  std::back_inserter(arr_gain_cor));
        if (pos_gain_cor.size() != fNrows_pr) {
          std::cout << pos_gain_cor.size() << "  != " << fNrows_pr << "\n";
        }
      }

      void ReadLegacyCalibration(const std::string& fname = "input.dat" );

      /** Load the calibration constants for a specific run.
       */
      void LoadJsonCalibration(const std::string& fname = "pcal_calib.json", int run_number = 0);
      void WriteCalibration(int run_num) const;


      /** Load the calibration constants for a specific run.
       *  Takes the current values and builds a json object string for the main database file.
       */
      std::string PrepareJson() const;

      void BuildTester(int run_num) const;

      void Print() const ;
    };

    //namespace fmt {
    //  template <>
    //  struct formatter<CalorimeterCalibration> {
    //    template <typename ParseContext>
    //    constexpr auto parse(ParseContext& ctx) {
    //      return ctx.begin();
    //    }
    //    template <typename FormatContext>
    //    auto format(const CalorimeterCalibration& p, FormatContext& ctx) {
    //      return format_to(ctx.begin(), "pos_{:.1f}, {:.1f})", p.x, p.y);
    //    }
    //  };
    //} // namespace fmt
  }
} // namespace hallc

#endif
