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

#include "calibration/THcPShTrack.h"
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

namespace hallc {
  namespace calibration {

    //bool CollCut(double xptar, double ytar, double yptar, double delta);

    /** SHMS Calorimeter calibration class.
     */
    class ShowerCalibrator {

      bool   _gui = true;

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

      std::string            input_calib_file;
      std::string            output_calib_file;
      //std::string            input_rootfile_name;

      CalorimeterCalibration _calibration;


      double fLoThr; // Low and high thresholds on the normalized uncalibrated
      double fHiThr; // energy deposition.
      //UInt_t   fNev;   // Number of processed events.

      //UInt_t fNentries;
      //UInt_t fNstart;
      //UInt_t fNstop;
      //Int_t  fNstopRequested;

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

      void UseGui(bool v = true) { _gui = v; }
      void NoGui() { _gui = false; }

      //TH1F* hEunc;
      //TH1F* hEuncSel;
      //TH1F* hEcal;
      //TH2F* hDPvsEcal;
      //TH2F* hESHvsEPR;
      //////
      //TH1F* hEPRunc;
      //TH2F* hETOTvsEPR;
      //TH2F* hETOTvsEPRunc;
      //TH2F* hESHvsEPRunc;



      //std::array<double, fNrows_pr> P_pr_apos_p;
      //std::array<double, fNrows_pr> P_pr_aneg_p;
      //std::array<double, fNpmts_sh> P_sh_a_p;

      //// Track parameters.

      //double P_tr_n;
      //double P_tr_p;
      //double P_tr_x; // X FP
      //double P_tr_xp;
      //double P_tr_y; // Y FP
      //double P_tr_yp;

      //double P_tr_tg_dp;
      //double P_tr_tg_ph;
      //double P_tr_tg_th;
      //double P_tr_tg_y;

      //std::array<double,4> P_hgcer_npe;
      //std::array<double,4> P_ngcer_npe;
      //double P_tr_beta;

      //double P_cal_nclust;      // Preshower
      //double P_cal_ntracks;     // Preshower
      //double P_cal_fly_nclust;  // Shower
      //double P_cal_fly_ntracks; // Shower

      // Quantities for calculations of the calibration constants.
      //using Vector_t      = Eigen::Matrix<double, fNpmts, 1>;
      //using VectorCount_t = Eigen::Matrix<uint64_t, fNpmts, 1>;
      //using Matrix_t      = Eigen::Matrix<double, fNpmts, fNpmts>;

      //double        fe0;
      //Vector_t      fqe;
      //Vector_t      fq0;
      ////Matrix_t      fQ;
      //Vector_t      falphaU; // 'unconstrained' calib. constants
      //Vector_t      falphaC; // the sought calibration constants
      //Vector_t      falpha0; // initial gains
      //Vector_t      falpha1; // unit gains
      //VectorCount_t fHitCount;

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

      void Print() const {
        _calibration.Print();
      }

    };

  } // namespace calibration
} // namespace hallc

#endif
