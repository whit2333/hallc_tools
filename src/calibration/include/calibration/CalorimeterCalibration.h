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

    /** Calorimeter Calibration.
     *
     * Stores the calibration and provides various read/write methods.
     * Actual calibration method is handeled by ShowerCalibrator.
     *
     * \todo Add (non-member?) functions  to combine multiple calibrations.
     *
     */
    struct CalorimeterCalibration {

      // Coordinate correction constants for Preshower blocks
      //static constexpr double fAcor = 106.73;
      //static constexpr double fBcor = 2.329;
      using Config = CalorimeterConfig<16,14,14>;
      // Calorimeter geometry constants.
      static const unsigned int fNrows_pr = Config::N_rows_preshower;
      static const unsigned int fNcols_pr = Config::N_cols_preshower;
      static const unsigned int fNrows_sh = Config::N_rows_shower;
      static const unsigned int fNcols_sh = Config::N_cols_shower;
      static const unsigned int fNpmts_pr = fNrows_pr * fNcols_pr;
      static const unsigned int fNpmts    = fNpmts_pr + fNrows_sh * fNcols_sh;

      double       fDeltaMin    = 0.;
      double       fDeltaMax    = 1.;
      double       fBetaMin     = 0.5;
      double       fBetaMax     = 2.;
      double       fHGCerMin    = 2.;
      double       fNGCerMin    = 2.;
      double       fMinHitCount = 10;
      double       fEuncLoLo    = 0.0;
      double       fEuncHiHi    = 0.0; // Range of uncalibrated Edep histogram
      unsigned int fEuncNBin    = 0;   // Binning of uncalibrated Edep histogram
      double       fEuncGFitLo  = 0.0;
      double       fEuncGFitHi  = 0.0; // Gaussian fit range of uncalib. Edep histo.

      std::vector<double> neg_gain_cor;
      std::vector<double> pos_gain_cor;
      std::vector<double> arr_gain_cor;

      mutable std::string input_cal_file_name  = "pcal_calib.json";
      mutable std::string output_cal_file_name = "pcal_calib_new.json";
      mutable int         run_number           = 0;
    protected:
      static void MergeHelper(double w1, double w2, std::vector<double>& vec1,
                              const std::vector<double>& vec2);

    public:
      CalorimeterCalibration() {}
      CalorimeterCalibration(int rn);
      CalorimeterCalibration(const CalorimeterCalibration&) = default;
      CalorimeterCalibration& operator=(const CalorimeterCalibration&) = default;

      void Merge(const CalorimeterCalibration&, double weight = 0.5);

      double GetGainCoeff(uint64_t block) const;

      /** Sets the calibration coeffs.
       *  (neg_gain_cor, pos_gain_cor, arr_gain_cor)
       */
      void SetGainCoeffs(const std::array<double, fNpmts>& coeffs);

      /** Leagcy reader.
       */
      void ReadLegacyCalibration(const std::string& fname = "input.dat");

      /** Load the calibration constants for a specific run.
       */
      void LoadCalibration(int run_num, const std::string& fname = "pcal_calib.json");
      void LoadJsonCalibration(int run_num, const std::string& fname = "pcal_calib.json");

      void WriteCalibration(int run_num, const std::string& fname = "pcal_calib_new.json") const;

      /** Load the calibration constants for a specific run.
       *  Takes the current values and builds a json object string for the main database file.
       */
      std::string PrepareJson() const;

      void BuildTester(int run_num) const;

      void Print() const;
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
