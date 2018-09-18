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
#include "TMath.h"
#include "TMatrixD.h"
#include "TROOT.h"
#include "TTree.h"
#include "TVectorD.h"

#include "calibration/THcPShTrack.h"

#include "calibration/CalorimeterCalibration.h"

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
    public:
      CalorimeterCalibration _calibration;

    public:
      ShowerCalibrator();
      ShowerCalibrator(std::string, int, int);
      ShowerCalibrator(const CalorimeterCalibration& c);
      ~ShowerCalibrator();

      //void ReadThresholds();
      //void Init();
      bool ReadShRawTrack(THcPShTrack& trk, UInt_t ientry);
      void CalcThresholds();
      void ComposeVMs();
      void SolveAlphas();
      void FillHEcal();
      //void SaveAlphas(std::string output_fname = "pcal.param");
      //void SaveRawData();

      TH1F* hEunc;
      TH1F* hEuncSel;
      TH1F* hEcal;
      TH2F* hDPvsEcal;
      TH2F* hESHvsEPR;
      ////
      TH1F* hEPRunc;
      TH2F* hETOTvsEPR;
      TH2F* hETOTvsEPRunc;
      TH2F* hESHvsEPRunc;

      std::string  input_file_name;

      Double_t fLoThr; // Low and high thresholds on the normalized uncalibrated
      Double_t fHiThr; // energy deposition.
      UInt_t   fNev;   // Number of processed events.

      //Double_t fDeltaMin, fDeltaMax;     // Delta range, %.
      //Double_t fBetaMin, fBetaMax;       // Beta range
      //Double_t fHGCerMin;                // Threshold heavy gas Cerenkov signal, p.e.
      //Double_t fNGCerMin;                // Threshold noble gas Cerenkov signal, p.e.
      //UInt_t   fMinHitCount;             // Min. number of hits/chan. for calibration
      //Double_t fEuncLoLo, fEuncHiHi;     // Range of uncalibrated Edep histogram
      //UInt_t   fEuncNBin;                // Binning of uncalibrated Edep histogram
      //Double_t fEuncGFitLo, fEuncGFitHi; // Gaussian fit range of uncalib. Edep histo.

      UInt_t fNentries;
      UInt_t fNstart;
      UInt_t fNstop;
      Int_t  fNstopRequested;

      // Declaration of leaves types

      // Preshower and Shower ADC signals.

      // Calorimeter geometry constants.
      static const unsigned int fNrows_pr = 14; // Row number for Preshower
      static const unsigned int fNrows_sh = 16; // Row number for Shower
      static const unsigned int fNcols_pr = 2;  // 2 columns in Preshower
      static const unsigned int fNcols_sh = 14; // 14 columnsin Shower
      static const unsigned int fNpmts_pr = fNrows_pr * fNcols_pr;
      static const unsigned int fNpmts    = fNpmts_pr + fNrows_sh * fNcols_sh;

      Double_t P_pr_apos_p[THcPShTrack::fNrows_pr];
      Double_t P_pr_aneg_p[THcPShTrack::fNrows_pr];
      Double_t P_sh_a_p[THcPShTrack::fNcols_sh * THcPShTrack::fNrows_sh];

      // Track parameters.

      double   P_tr_n;
      Double_t P_tr_p;
      Double_t P_tr_x; // X FP
      Double_t P_tr_xp;
      Double_t P_tr_y; // Y FP
      Double_t P_tr_yp;

      Double_t P_tr_tg_dp;
      Double_t P_tr_tg_ph;
      Double_t P_tr_tg_th;
      Double_t P_tr_tg_y;

      Double_t P_hgcer_npe[4];
      Double_t P_ngcer_npe[4];
      Double_t P_tr_beta;

      Double_t P_cal_nclust;      // Preshower
      Double_t P_cal_ntracks;     // Preshower
      Double_t P_cal_fly_nclust;  // Shower
      Double_t P_cal_fly_ntracks; // Shower


      // Quantities for calculations of the calibration constants.
      double fe0;
      double fqe[fNpmts];
      double fq0[fNpmts];
      double fQ[fNpmts][fNpmts];
      double falphaU[fNpmts]; // 'unconstrained' calib. constants
      double falphaC[fNpmts]; // the sought calibration constants
      double falpha0[fNpmts]; // initial gains
      double falpha1[fNpmts]; // unit gains
      unsigned int fHitCount[fNpmts];

      void Print() const {
        _calibration.Print();
      }

    };

  } // namespace calibration
} // namespace hallc

#endif
