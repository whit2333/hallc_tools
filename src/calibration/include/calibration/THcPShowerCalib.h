#ifndef ROOT_THcPShowerCalib
#define ROOT_THcPShowerCalib

#include "calibration/THcPShTrack.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TVectorD.h"
#include "TMatrixD.h"
#include "TDecompLU.h"
#include "TMath.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"

#include "TF1.h"

#include <time.h>

#include <vector>

#include <string>

#define D_CALO_FP 292.64

//Whole calorimeter fid. limits
#define XMIN -60.
#define XMAX  60.
#define YMIN -58.
#define YMAX  58.

#define PR_ADC_THR 0
#define SH_ADC_THR 0

using namespace std;

bool CollCut(double xptar , double ytar, double yptar, double delta);

//
// SHMS Calorimeter calibration class.
//

class THcPShowerCalib {

 public:

  THcPShowerCalib(string, int, int);
  THcPShowerCalib();
  ~THcPShowerCalib();

  void ReadThresholds();
  void Init();
  bool ReadShRawTrack(THcPShTrack &trk, UInt_t ientry);
  void CalcThresholds();
  void ComposeVMs();
  void SolveAlphas();
  void FillHEcal();
  void SaveAlphas();
  void SaveRawData();

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

 private:

  string fPrefix;
  Double_t fLoThr;     // Low and high thresholds on the normalized uncalibrated
  Double_t fHiThr;     // energy deposition.
  UInt_t fNev;         // Number of processed events.

  Double_t fDeltaMin, fDeltaMax;   // Delta range, %.
  Double_t fBetaMin, fBetaMax;     // Beta range
  Double_t fHGCerMin;              // Threshold heavy gas Cerenkov signal, p.e.
  Double_t fNGCerMin;              // Threshold noble gas Cerenkov signal, p.e.
  UInt_t fMinHitCount;             // Min. number of hits/chan. for calibration
  Double_t fEuncLoLo, fEuncHiHi;   // Range of uncalibrated Edep histogram
  UInt_t fEuncNBin;                // Binning of uncalibrated Edep histogram
  Double_t fEuncGFitLo,fEuncGFitHi;// Gaussian fit range of uncalib. Edep histo.

  TTree* fTree;
  UInt_t fNentries;
  UInt_t fNstart;
  UInt_t fNstop;
  Int_t  fNstopRequested;

  // Declaration of leaves types

  // Preshower and Shower ADC signals.

  Double_t        P_pr_apos_p[THcPShTrack::fNrows_pr];
  Double_t        P_pr_aneg_p[THcPShTrack::fNrows_pr];
  Double_t        P_sh_a_p[THcPShTrack::fNcols_sh*THcPShTrack::fNrows_sh];

  // Track parameters.

  double          P_tr_n;
  Double_t        P_tr_p;
  Double_t        P_tr_x;   //X FP
  Double_t        P_tr_xp;
  Double_t        P_tr_y;   //Y FP
  Double_t        P_tr_yp;

  Double_t        P_tr_tg_dp;
  Double_t        P_tr_tg_ph;
  Double_t        P_tr_tg_th;
  Double_t        P_tr_tg_y;

  Double_t        P_hgcer_npe[4];
  Double_t        P_ngcer_npe[4];
  Double_t        P_tr_beta;

  Double_t        P_cal_nclust;          //Preshower
  Double_t        P_cal_ntracks;         //Preshower
  Double_t        P_cal_fly_nclust;      //Shower
  Double_t        P_cal_fly_ntracks;     //Shower

  TBranch* b_P_tr_p;
  TBranch* b_P_pr_apos_p;
  TBranch* b_P_pr_aneg_p;
  TBranch* b_P_sh_a_p;
  TBranch* b_P_tr_n;
  TBranch* b_P_tr_x;
  TBranch* b_P_tr_y;
  TBranch* b_P_tr_xp;
  TBranch* b_P_tr_yp;
  TBranch* b_P_tr_tg_dp;
  TBranch* b_P_tr_tg_ph;
  TBranch* b_P_tr_tg_th;
  TBranch* b_P_tr_tg_y;
  TBranch* b_P_hgcer_npe;
  TBranch* b_P_ngcer_npe;
  TBranch* b_P_tr_beta;

  TBranch* b_P_cal_nclust;
  TBranch* b_P_cal_ntracks;
  TBranch* b_P_cal_fly_nclust;
  TBranch* b_P_cal_fly_ntracks;

  // Quantities for calculations of the calibration constants.

  Double_t fe0;
  Double_t fqe[THcPShTrack::fNpmts];
  Double_t fq0[THcPShTrack::fNpmts];
  Double_t fQ[THcPShTrack::fNpmts][THcPShTrack::fNpmts];
  Double_t falphaU[THcPShTrack::fNpmts];   // 'unconstrained' calib. constants
  Double_t falphaC[THcPShTrack::fNpmts];   // the sought calibration constants
  Double_t falpha0[THcPShTrack::fNpmts];   // initial gains
  Double_t falpha1[THcPShTrack::fNpmts];   // unit gains

  UInt_t fHitCount[THcPShTrack::fNpmts];

};


#endif
