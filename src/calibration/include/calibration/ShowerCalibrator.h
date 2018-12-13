#ifndef hallc_calibration_shms_ShowerCalibrator_HH
#define hallc_calibration_shms_ShowerCalibrator_HH

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>

#include "TDecompLU.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TMath.h"
#include "TMatrixD.h"
#include "TROOT.h"
#include "TTree.h"
#include "TVectorD.h"
#include "TCanvas.h"

//#include "calibration/THcPShTrack.h"
#include "calibration/CalorimeterCalibration.h"

#include <Eigen/Dense>

#define D_CALO_FP 292.64

// Whole calorimeter fid. limits
#define XMIN -60.
#define XMAX 60.
#define YMIN -58.
#define YMAX 58.

#define PR_ADC_THR 0
#define SH_ADC_THR 0


#define HMS_D_CALO_FP 338.69    //distance from FP to the calorimeter face

//Whole calorimeter
#define HMS_XMIN -65.4
#define HMS_XMAX  54.6
#define HMS_YMIN -30.
#define HMS_YMAX  30.

namespace hallc {
  namespace calibration {

    /** SHMS Calorimeter calibration class.
     */
    class ShowerCalibrator {

    protected:
      bool     _gui    = true;
      TCanvas* _canvas = nullptr;
      //std::string            input_rootfile_name;

    public:

      // Declaration of leaves types
      // Preshower and Shower ADC signals.
      // Calorimeter geometry constants.
      static const unsigned int fNrows_pr = CalorimeterCalibration::Config::N_rows_preshower;
      static const unsigned int fNcols_pr = CalorimeterCalibration::Config::N_cols_preshower;
      static const unsigned int fNrows_sh = CalorimeterCalibration::Config::N_rows_shower;
      static const unsigned int fNcols_sh = CalorimeterCalibration::Config::N_cols_shower;
      static const unsigned int fNpmts_pr = fNrows_pr * fNcols_pr;
      static const unsigned int fNpmts_sh = fNrows_sh * fNcols_sh;
      static const unsigned int fNpmts    = fNpmts_pr + fNrows_sh * fNcols_sh;

      struct MatrixQuantities {
        using Vector_t      = Eigen::VectorXd;
        using VectorCount_t = Eigen::VectorXi;
        using Matrix_t      = Eigen::MatrixXd;
        using LU_t          = Eigen::FullPivLU<typename MatrixQuantities::Matrix_t>;

        double        fe0       = 0.0;
        Vector_t      fqe       = Vector_t::Zero(fNpmts);
        Vector_t      fq0       = Vector_t::Zero(fNpmts);
        Vector_t      fQiq0     = Vector_t::Zero(fNpmts);
        Vector_t      falphaU   = Vector_t::Zero(fNpmts); // 'unconstrained' calib. constants
        Vector_t      falphaC   = Vector_t::Zero(fNpmts); // the sought calibration constants
        Vector_t      falpha0   = Vector_t::Zero(fNpmts); // initial gains
        Vector_t      falpha1   = Vector_t::Zero(fNpmts); // unit gains
        VectorCount_t fHitCount = VectorCount_t::Zero(fNpmts);
        Matrix_t      fQ        = Matrix_t::Zero(fNpmts, fNpmts);
      };

      std::string            input_calib_file;
      std::string            output_calib_file;

      CalorimeterCalibration _calibration;

      double fLoThr = 0.5; // Low and high thresholds on the normalized uncalibrated
      double fHiThr = 2.0; // energy deposition.
      double fMean  = 1.0;
      double fSigma = 0.1;

    public:
      ShowerCalibrator();
      ShowerCalibrator(std::string infile, std::string outfile);
      ShowerCalibrator(const CalorimeterCalibration& c);
      virtual ~ShowerCalibrator() = default;

      void LoadCalibration(int run_number){
        _calibration.LoadCalibration(run_number, input_calib_file);
      }
      void WriteCalibration(int run_number, std::string outfile){
        _calibration.WriteCalibration(run_number, outfile);
      }
      void UpdateCalibration(int run_number){
        _calibration.WriteCalibration(run_number, output_calib_file);
      }
      void Process(std::string rootfile);
      void UpdatePlots(std::string rootfile);

      void UseGui(bool v = true) { _gui = v; }
      void NoGui() { _gui = false; }

      void SavePlots(std::string plot_file) const {
        if (_canvas)
          _canvas->SaveAs(plot_file.c_str());
      }

      auto GetDataFrame(std::string rootfile);


      void Print() const {
        _calibration.Print();
      }

    };

  } // namespace calibration
} // namespace hallc

#endif
