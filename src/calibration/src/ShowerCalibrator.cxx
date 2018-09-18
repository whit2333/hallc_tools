#include "calibration/THcPShTrack.h"
#include "calibration/ShowerCalibrator.h"

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

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"

using namespace std;

namespace hallc {
  namespace calibration {

    ShowerCalibrator::ShowerCalibrator(const CalorimeterCalibration& c) : _calibration(c) {}

    ShowerCalibrator::ShowerCalibrator() {}

    ShowerCalibrator::ShowerCalibrator(string fname, int nstart, int nstop)
        : input_file_name(fname), fNstart(nstart), fNstopRequested(nstop) {}

    ShowerCalibrator::~ShowerCalibrator() {}


    //------------------------------------------------------------------------------

    void ShowerCalibrator::CalcThresholds() {

      // Calculate +/-3 RMS thresholds on the uncalibrated total energy
      // depositions. These thresholds are used mainly to exclude potential
      // hadronic events due to the gas Cherenkov inefficiency.

      // Histogram uncalibrated energy depositions, get mean and RMS from the
      // histogram, establish +/-3 * RMS thresholds.

      // cout<< "ShowerCalibrator::CalcThresholds: FNentries = " << fNentries << endl;
      //  cout << "ShowerCalibrator::CalcThresholds: fNstart = " << fNstart << " "
      //       << "  fNstop = " << fNstop << endl;

      Int_t       nev = 0;

      using doublers = ROOT::VecOps::RVec<double>;

      auto collimator_cut = [&](double xptar, double ytar, double yptar, double delta) {
        // SHMS octagonal collimator cut.
        const double FullHeight = 25.; // cm
        const double FullWidth  = 17.;
        const double MidHeight  = 12.5;
        const double MidWidth   = 8.5;

        // Mark (04/16/18).
        double yc = -0.019 * (delta) + yptar * 253. - 40. * 0.00052 * (delta) + ytar;

        double xc = xptar * 253.;

        if (xc > FullHeight / 2.)
          return false;
        if (xc < -FullHeight / 2.)
          return false;
        if (yc > FullWidth / 2.)
          return false;
        if (yc < -FullWidth / 2.)
          return false;

        double slope = (FullHeight - MidHeight) / (FullWidth - MidWidth);
        if (xc > MidHeight / 2. + slope * (yc + FullWidth / 2.))
          return false;
        if (xc > FullHeight / 2. - slope * (yc - MidWidth / 2.))
          return false;
        if (xc < -FullHeight / 2. + slope * (yc - MidWidth / 2.))
          return false;
        if (xc < -MidHeight / 2. - slope * (yc + FullWidth / 2.))
          return false;

        return true;
      };

      ROOT::RDataFrame df("T", input_file_name.c_str(),
                          {"P.cal.pr.goodPosAdcPulseInt",
                           "P.cal.pr.goodNegAdcPulseInt",
                           "P.cal.fly.goodAdcPulseInt",
                           "P.tr.n",
                           "P.tr.x",
                           "P.tr.y",
                           "P.tr.th",
                           "P.tr.ph",
                           "P.tr.p",
                           "P.tr.tg_dp",
                           "P.tr.tg_ph",
                           "P.tr.tg_th",
                           "P.tr.tg_y",
                           "P.hgcer.npe",
                           "P.ngcer.npe",
                           "P.tr.beta",
                           "P.cal.nclust",
                           "P.cal.ntracks",
                           "P.cal.fly.nclust",
                           "P.cal.fly.ntracks"});


      //(P_tr_tg_th, P_tr_tg_y, P_tr_tg_ph, P_tr_tg_dp)
      // if (P_tr_n != 1)
      //  return 0;
      // bool good_trk = P_tr_tg_dp > fDeltaMin && P_tr_tg_dp < fDeltaMax;

      auto df_with_cuts = df.Filter([&](const double& n) { return int(n) == 1; }, {"P.tr.n"})
                              .Filter(
                                  [&](doublers& dp) {
                                    return (dp.at(0) > _calibration.fDeltaMin) &&
                                           (dp.at(0) < _calibration.fDeltaMax);
                                  },
                                  {"P.tr.tg_dp"})
                              .Filter(
                                  [&](doublers& th, doublers& y, doublers& yp, doublers& dp) {
                                    return collimator_cut(th.at(0), y.at(0), yp.at(0), dp.at(0));
                                  },
                                  {"P.tr.tg_th", "P.tr.tg_y", "P.tr.tg_ph", "P.tr.tg_dp"});

      // if (!good_trk)
      //  return 0;

      // good_trk = CollCut(P_tr_tg_th, P_tr_tg_y, P_tr_tg_ph, P_tr_tg_dp);

      // if (!good_trk)
      //  return 0;

      // good_trk = good_trk && P_tr_x + P_tr_xp * D_CALO_FP > XMIN &&
      //           P_tr_x + P_tr_xp * D_CALO_FP < XMAX && P_tr_y + P_tr_yp * D_CALO_FP > YMIN &&
      //           P_tr_y + P_tr_yp * D_CALO_FP < YMAX;
      // if (!good_trk)
      //  return 0;

      //////
      // if (P_cal_nclust != 1)
      //  return 0;
      // if (P_cal_fly_nclust != 1)
      //  return 0;

      //////
      // good_trk = P_tr_xp > -0.045 + 0.0025 * P_tr_x;
      // if (!good_trk)
      //  return 0;

      // bool good_ngcer = P_ngcer_npe[0] > fNGCerMin || P_ngcer_npe[1] > fNGCerMin ||
      //                  P_ngcer_npe[2] > fNGCerMin || P_ngcer_npe[3] > fNGCerMin;
      // if (!good_ngcer)
      //  return 0;

      // bool good_hgcer =
      //    P_hgcer_npe[0] + P_hgcer_npe[1] + P_hgcer_npe[2] + P_hgcer_npe[3] > fHGCerMin;
      // if (!good_hgcer)
      //  return 0;

      // bool good_beta = P_tr_beta > fBetaMin && P_tr_beta < fBetaMax;
      // if (!good_beta)
      //  return 0;

      auto d2 = df_with_cuts.Define("E_shower_cal",
                                    [&](doublers& pr_neg, doublers& pr_pos, doublers& shower) {
                                      THcPShTrack trk;
                                      UInt_t      nb = 0;
                                      for (UInt_t k = 0; k < THcPShTrack::fNcols_pr; k++) {
                                        for (UInt_t j = 0; j < THcPShTrack::fNrows_pr; j++) {
                                          nb++;
                                          Double_t adc;
                                          switch (k) {
                                          case 0:
                                            adc = pr_neg[j];
                                            break;
                                          case 1:
                                            adc = pr_pos[j];
                                            break;
                                          default:
                                            cout << "*** Wrong PreShower column! ***" << endl;
                                          }
                                          if (adc > PR_ADC_THR)
                                            trk.AddHit(adc, 0., nb);
                                        }
                                      }
                                      // Set Shower hits.
                                      for (UInt_t k = 0; k < THcPShTrack::fNcols_sh; k++) {
                                        for (UInt_t j = 0; j < THcPShTrack::fNrows_sh; j++) {
                                          nb++;
                                          Double_t adc = P_sh_a_p[k * THcPShTrack::fNrows_sh + j];
                                          if (adc > SH_ADC_THR) {
                                            trk.AddHit(adc, 0., nb);
                                          }
                                        }
                                      }
                                      return trk.Enorm();
                                    },
                                    {"P.cal.pr.goodPosAdcPulseInt", "P.cal.pr.goodNegAdcPulseInt",
                                     "P.cal.fly.goodAdcPulseInt"});

      auto hEunc = d2.Histo1D({"hEunc", "Edep/P uncalibrated", _calibration.fEuncNBin,
                                         _calibration.fEuncLoLo, _calibration.fEuncHiHi},
                                        "E_shower_cal");

      hEunc->DrawCopy();
      std::cout <<  " entries : " << *(d2.Count()) << "\n";

      // hEunc     = new TH1F("hEunc", "Edep/P uncalibrated", fEuncNBin, fEuncLoLo, fEuncHiHi);

      //// Set track coordinates and slopes at the face of Preshower.

      //trk.Reset(P_tr_p, P_tr_tg_dp, P_tr_x + D_CALO_FP * P_tr_xp, P_tr_xp,
      //          P_tr_y + D_CALO_FP * P_tr_yp, P_tr_yp);

      //// Set Preshower hits.

      //UInt_t nb = 0;
      //for (UInt_t k = 0; k < THcPShTrack::fNcols_pr; k++) {
      //  for (UInt_t j = 0; j < THcPShTrack::fNrows_pr; j++) {
      //    nb++;
      //    Double_t adc;
      //    switch (k) {
      //    case 0:
      //      adc = P_pr_aneg_p[j];
      //      break;
      //    case 1:
      //      adc = P_pr_apos_p[j];
      //      break;
      //    default:
      //      cout << "*** Wrong PreShower column! ***" << endl;
      //    }
      //    if (adc > PR_ADC_THR)
      //      trk.AddHit(adc, 0., nb);
      //  }
      //}

      //// Set Shower hits.

      //for (UInt_t k = 0; k < THcPShTrack::fNcols_sh; k++) {
      //  for (UInt_t j = 0; j < THcPShTrack::fNrows_sh; j++) {
      //    nb++;
      //    Double_t adc = P_sh_a_p[k * THcPShTrack::fNrows_sh + j];
      //    if (adc > SH_ADC_THR) {
      //      trk.AddHit(adc, 0., nb);
      //    }
      //  }
      //}
      //for (UInt_t ientry = fNstart; ientry < fNstop; ientry++) {

      //  if (ReadShRawTrack(trk, ientry)) {

      //    //    trk.Print(cout);
      //    //    getchar();

      //    trk.SetEs(falpha0); // Use initial gain constants here.
      //    Double_t Enorm = trk.Enorm();

      //    ////
      //    if (Enorm > 0.) {
      //      hEunc->Fill(Enorm);
      //      hETOTvsEPRunc->Fill(trk.EPRnorm(), Enorm);        ////
      //      hESHvsEPRunc->Fill(trk.EPRnorm(), trk.ESHnorm()); ////
      //      hEPRunc->Fill(trk.EPRnorm());                     ////
      //      nev++;
      //    }

      //    //    if (nev%100000 == 0)
      //    //      cout << "CalcThreshods: nev=" << nev << "  Enorm=" << Enorm << endl;
      //  }

      //  if (nev > 200000)
      //    break;
      //};

      //  hEunc->Fit("gaus","0","",fEuncGFitLo, fEuncGFitHi);    //fit, do not plot
      //hEunc->Fit("gaus", "", "", fEuncGFitLo, fEuncGFitHi);
      //hEunc->GetFunction("gaus")->SetLineColor(2);
      //hEunc->GetFunction("gaus")->SetLineWidth(1);
      //hEunc->GetFunction("gaus")->SetLineStyle(1);
      //TF1*     fit    = hEunc->GetFunction("gaus");
      //Double_t gmean  = fit->GetParameter(1);
      //Double_t gsigma = fit->GetParameter(2);
      //fLoThr          = gmean - 3. * gsigma;
      //fHiThr          = gmean + 3. * gsigma;
      //cout << "CalcThreshods: fLoThr=" << fLoThr << "  fHiThr=" << fHiThr << "  nev=" << nev
      //     << endl;

      //Int_t nbins = hEunc->GetNbinsX();
      //Int_t nlo   = hEunc->FindBin(fLoThr);
      //Int_t nhi   = hEunc->FindBin(fHiThr);

      //cout << "CalcThresholds: nlo=" << nlo << "  nhi=" << nhi << "  nbins=" << nbins << endl;

      //// Histogram of selected within the thresholds events.

      //hEuncSel = (TH1F*)hEunc->Clone("hEuncSel");

      //for (Int_t i = 0; i < nlo; i++)
      //  hEuncSel->SetBinContent(i, 0.);
      //for (Int_t i = nhi; i < nbins + 1; i++)
      //  hEuncSel->SetBinContent(i, 0.);
    };

    //------------------------------------------------------------------------------

    bool ShowerCalibrator::ReadShRawTrack(THcPShTrack& trk, UInt_t ientry) {

      //
      // Set a Shower track event from ntuple ientry.
      //

      //fTree->GetEntry(ientry);

    //  if (ientry % 100000 == 0)
    //    cout << "   ReadShRawTrack: " << ientry << endl;

    //  // Request single electron track in calorimeter's fid. volume.
    //  //

    //  if (P_tr_n != 1)
    //    return 0;

    //  bool good_trk = P_tr_tg_dp > _calibration.fDeltaMin && P_tr_tg_dp < _calibration.fDeltaMax;

    //  if (!good_trk)
    //    return 0;

    //  good_trk = true;//CollCut(P_tr_tg_th, P_tr_tg_y, P_tr_tg_ph, P_tr_tg_dp);

    //  if (!good_trk)
    //    return 0;

    //  good_trk = good_trk && P_tr_x + P_tr_xp * D_CALO_FP > XMIN &&
    //             P_tr_x + P_tr_xp * D_CALO_FP < XMAX && P_tr_y + P_tr_yp * D_CALO_FP > YMIN &&
    //             P_tr_y + P_tr_yp * D_CALO_FP < YMAX;
    //  if (!good_trk)
    //    return 0;

    //  ////
    //  if (P_cal_nclust != 1)
    //    return 0;
    //  if (P_cal_fly_nclust != 1)
    //    return 0;

    //  ////
    //  good_trk = P_tr_xp > -0.045 + 0.0025 * P_tr_x;
    //  if (!good_trk)
    //    return 0;

    //  bool good_ngcer = P_ngcer_npe[0] > _calibration.fNGCerMin || P_ngcer_npe[1] > _calibration.fNGCerMin ||
    //                    P_ngcer_npe[2] > _calibration.fNGCerMin || P_ngcer_npe[3] > _calibration.fNGCerMin;
    //  if (!good_ngcer)
    //    return 0;

    //  bool good_hgcer =
    //      P_hgcer_npe[0] + P_hgcer_npe[1] + P_hgcer_npe[2] + P_hgcer_npe[3] > _calibration.fHGCerMin;
    //  if (!good_hgcer)
    //    return 0;

    //  bool good_beta = P_tr_beta > _calibration.fBetaMin && P_tr_beta < _calibration.fBetaMax;
    //  if (!good_beta)
    //    return 0;

    //  // Set track coordinates and slopes at the face of Preshower.

    //  trk.Reset(P_tr_p, P_tr_tg_dp, P_tr_x + D_CALO_FP * P_tr_xp, P_tr_xp,
    //            P_tr_y + D_CALO_FP * P_tr_yp, P_tr_yp);

    //  // Set Preshower hits.

    //  UInt_t nb = 0;

    //  for (UInt_t k = 0; k < THcPShTrack::fNcols_pr; k++) {
    //    for (UInt_t j = 0; j < THcPShTrack::fNrows_pr; j++) {
    //      nb++;
    //      Double_t adc;
    //      switch (k) {
    //      case 0:
    //        adc = P_pr_aneg_p[j];
    //        break;
    //      case 1:
    //        adc = P_pr_apos_p[j];
    //        break;
    //      default:
    //        cout << "*** Wrong PreShower column! ***" << endl;
    //      }
    //      if (adc > PR_ADC_THR)
    //        trk.AddHit(adc, 0., nb);
    //    }
    //  }

    //  // Set Shower hits.

    //  for (UInt_t k = 0; k < THcPShTrack::fNcols_sh; k++) {
    //    for (UInt_t j = 0; j < THcPShTrack::fNrows_sh; j++) {
    //      nb++;
    //      Double_t adc = P_sh_a_p[k * THcPShTrack::fNrows_sh + j];
    //      if (adc > SH_ADC_THR) {
    //        trk.AddHit(adc, 0., nb);
    //      }
    //    }
    //  }

      return 1;
    }

    //------------------------------------------------------------------------------

    void ShowerCalibrator::ComposeVMs() {

      //
      // Fill in vectors and matrixes for the gain constant calculations.
      //

      fNev = 0;
      THcPShTrack trk;

      // Loop over the shower track events in the ntuples.

      //for (UInt_t ientry = fNstart; ientry < fNstop; ientry++) {

      //  if (ReadShRawTrack(trk, ientry)) {

      //    // Set energy depositions with default gains.
      //    // Calculate normalized to the track momentum total energy deposition,
      //    // check it against the thresholds.

      //    trk.SetEs(falpha0);
      //    Double_t Enorm = trk.Enorm();
      //    if (Enorm > fLoThr && Enorm < fHiThr) {

      //      trk.SetEs(falpha1); // Set energies with unit gains for now.
      //      // trk.Print(cout);

      //      fe0 += trk.GetP(); // Accumulate track momenta.

      //      vector<pmt_hit> pmt_hit_list; // Container to save PMT hits

      //      // Loop over hits.

      //      for (UInt_t i = 0; i < trk.GetNhits(); i++) {

      //        THcPShHit* hit = trk.GetHit(i);
      //        // hit->Print(cout);

      //        UInt_t nb = hit->GetBlkNumber();

      //        // Fill the qe and q0 vectors.

      //        fqe[nb - 1] += hit->GetEdep() * trk.GetP();
      //        fq0[nb - 1] += hit->GetEdep();

      //        // Save the PMT hit.

      //        pmt_hit_list.push_back(pmt_hit{hit->GetEdep(), nb});

      //        fHitCount[nb - 1]++; // Accrue the hit counter.

      //      } // over hits

      //      // Fill in the correlation matrix Q by retrieving the PMT hits.

      //      for (vector<pmt_hit>::iterator i = pmt_hit_list.begin(); i < pmt_hit_list.end(); i++) {

      //        UInt_t   ic = (*i).channel;
      //        Double_t is = (*i).signal;

      //        for (vector<pmt_hit>::iterator j = i; j < pmt_hit_list.end(); j++) {

      //          UInt_t   jc = (*j).channel;
      //          Double_t js = (*j).signal;

      //          fQ[ic - 1][jc - 1] += is * js;
      //          if (jc != ic)
      //            fQ[jc - 1][ic - 1] += is * js;
      //        }
      //      }

      //      fNev++;

      //    }; // if within enorm thresholds

      //  }; // success in reading

      //}; // over entries

      //// Take averages.

      //fe0 /= fNev;
      //for (UInt_t i = 0; i < THcPShTrack::fNpmts; i++) {
      //  fqe[i] /= fNev;
      //  fq0[i] /= fNev;
      //}

      //for (UInt_t i = 0; i < THcPShTrack::fNpmts; i++)
      //  for (UInt_t j = 0; j < THcPShTrack::fNpmts; j++)
      //    fQ[i][j] /= fNev;

      // Output vectors and matrixes, for debug purposes.
      /*
      ofstream q0out;
      q0out.open("q0.deb",ios::out);
      for (UInt_t i=0; i<THcPShTrack::fNpmts; i++)
        q0out << setprecision(20) << fq0[i] << " " << i << endl;
      q0out.close();

      ofstream qeout;
      qeout.open("qe.deb",ios::out);
      for (UInt_t i=0; i<THcPShTrack::fNpmts; i++)
        qeout << setprecision(20) << fqe[i] << " " << i << endl;
      qeout.close();

      ofstream Qout;
      Qout.open("Q.deb",ios::out);
      for (UInt_t i=0; i<THcPShTrack::fNpmts; i++)
        for (UInt_t j=0; j<THcPShTrack::fNpmts; j++)
          Qout << setprecision(20) << fQ[i][j] << " " << i << " " << j << endl;
      Qout.close();

      ofstream sout;
      sout.open("signal.deb",ios::out);
      for (UInt_t i=0; i<THcPShTrack::fNpmts; i++) {
        double sig_sum = fq0[i] * fNev;
        double sig2_sum = fQ[i][i] * fNev;
        int nhit = fHitCount[i];
        double sig = 0.;
        double err = 0.;
        if (nhit != 0) {
          sig = sig_sum/nhit;
          double rms2 = sig2_sum/nhit - (sig_sum/nhit)*(sig_sum/nhit);
          if (rms2 > 0.) {
            double rms = TMath::Sqrt(rms2);
            err = rms/TMath::Sqrt(double(nhit));
          }
        }

        sout << sig << " " << err << " " << nhit << " " << i << endl;
      }
      sout.close();
      */
    }

    //------------------------------------------------------------------------------

    void ShowerCalibrator::SolveAlphas() {

      //
      // Solve for the sought calibration constants, by use of the Root
      // matrix algebra package.
      //

      TMatrixD Q(THcPShTrack::fNpmts, THcPShTrack::fNpmts);
      TVectorD q0(THcPShTrack::fNpmts);
      TVectorD qe(THcPShTrack::fNpmts);
      TVectorD au(THcPShTrack::fNpmts);
      TVectorD ac(THcPShTrack::fNpmts);
      Bool_t   ok;

      cout << "Solving Alphas..." << endl;
      cout << endl;

      // Print out hit numbers.

      cout << "Hit counts:" << endl;
      UInt_t j = 0;

      for (UInt_t k = 0; k < THcPShTrack::fNcols_pr; k++) {
        k == 0 ? cout << "Preshower:" : cout << "        :";
        for (UInt_t i = 0; i < THcPShTrack::fNrows_pr; i++)
          cout << setw(6) << fHitCount[j++] << ",";
        cout << endl;
      }

      for (UInt_t k = 0; k < THcPShTrack::fNcols_sh; k++) {
        k == 0 ? cout << "Shower   :" : cout << "        :";
        for (UInt_t i = 0; i < THcPShTrack::fNrows_sh; i++)
          cout << setw(6) << fHitCount[j++] << ",";
        cout << endl;
      }

      // Initialize the vectors and the matrix of the Root algebra package.

      for (UInt_t i = 0; i < THcPShTrack::fNpmts; i++) {
        q0[i] = fq0[i];
        qe[i] = fqe[i];
        for (UInt_t k = 0; k < THcPShTrack::fNpmts; k++) {
          Q[i][k] = fQ[i][k];
        }
      }

      // Sanity check.

      for (UInt_t i = 0; i < THcPShTrack::fNpmts; i++) {

        // Check zero hit channels: the vector and matrix elements should be 0.

        if (fHitCount[i] == 0) {

          if (q0[i] != 0. || qe[i] != 0.) {

            cout << "*** Inconsistency in chanel " << i << ": # of hits  " << fHitCount[i]
                 << ", q0=" << q0[i] << ", qe=" << qe[i];

            for (UInt_t k = 0; k < THcPShTrack::fNpmts; k++) {
              if (Q[i][k] != 0. || Q[k][i] != 0.)
                cout << ", Q[" << i << "," << k << "]=" << Q[i][k] << ", Q[" << k << "," << i
                     << "]=" << Q[k][i];
            }

            cout << " ***" << endl;
          }
        }

        // The hit channels: the vector elements should be non zero.

        if ((fHitCount[i] != 0) && (q0[i] == 0. || qe[i] == 0.)) {
          cout << "*** Inconsistency in chanel " << i << ": # of hits  " << fHitCount[i]
               << ", q0=" << q0[i] << ", qe=" << qe[i] << " ***" << endl;
        }

      } // sanity check

      // Low hit number channels: exclude from calculation. Assign all the
      // correspondent elements 0, except self-correlation Q(i,i)=1.

      cout << endl;
      cout << "Channels with hit number less than " << _calibration.fMinHitCount << " will not be calibrated."
           << endl;
      cout << endl;

      for (UInt_t i = 0; i < THcPShTrack::fNpmts; i++) {

        if (fHitCount[i] < _calibration.fMinHitCount) {
          cout << "Channel " << i << ", " << fHitCount[i] << " hits, will not be calibrated."
               << endl;
          q0[i] = 0.;
          qe[i] = 0.;
          for (UInt_t k = 0; k < THcPShTrack::fNpmts; k++) {
            Q[i][k] = 0.;
            Q[k][i] = 0.;
          }
          Q[i][i] = 1.;
        }
      }

      // Declare LU decomposition method for the correlation matrix Q.

      TDecompLU lu(Q);
      Double_t  d1, d2;
      lu.Det(d1, d2);
      cout << "cond:" << lu.Condition() << endl;
      cout << "det :" << d1 * TMath::Power(2., d2) << endl;
      cout << "tol :" << lu.GetTol() << endl;

      // Solve equation Q x au = qe for the 'unconstrained' calibration (gain)
      // constants au.

      au = lu.Solve(qe, ok);
      cout << "au: ok=" << ok << endl;
      //  au.Print();

      // Find the sought 'constrained' calibration constants next.

      Double_t t1 = fe0 - au * q0; // temporary variable.
      //  cout << "t1 =" << t1 << endl;

      TVectorD Qiq0(THcPShTrack::fNpmts); // an intermittent result
      Qiq0 = lu.Solve(q0, ok);
      cout << "Qiq0: ok=" << ok << endl;
      //  Qiq0.Print();

      Double_t t2 = q0 * Qiq0; // another temporary variable
      //  cout << "t2 =" << t2 << endl;

      ac = (t1 / t2) * Qiq0 + au; // the sought gain constants
      // cout << "ac:" << endl;
      //  ac.Print();

      // Assign the gain arrays.

      for (UInt_t i = 0; i < THcPShTrack::fNpmts; i++) {
        falphaU[i] = au[i];
        falphaC[i] = ac[i];
      }
    }

    //------------------------------------------------------------------------------

    void ShowerCalibrator::FillHEcal() {

      //
      // Fill histogram of the normalized energy deposition, and 2-d histogram
      // of momentum deviation versus normalized energy deposition.
      // Output event by event energy depositions and momenta for debug purposes.
      //

      //  ofstream output;
      //  output.open("calibrated.deb",ios::out);

      Int_t nev = 0;

      THcPShTrack trk;

      for (UInt_t ientry = fNstart; ientry < fNstop; ientry++) {

        if (ReadShRawTrack(trk, ientry)) {
          //    trk.Print(cout);

          trk.SetEs(falphaC); // use the 'constrained' calibration constants
          Double_t P     = trk.GetP();
          Double_t delta = trk.GetDp();
          Double_t Enorm = trk.Enorm();

          ////
          if (Enorm > 0.) {
            hEcal->Fill(Enorm);
            hDPvsEcal->Fill(Enorm, delta, 1.);
            hESHvsEPR->Fill(trk.EPRnorm(), trk.ESHnorm());
            hETOTvsEPR->Fill(trk.EPRnorm(), trk.Enorm()); ////
            //      output << Enorm*P/1000. << " " << P/1000. << " " << delta << " "
            //	     << trk.GetX() << " " << trk.GetY() << endl;
            nev++;
          }
        }

        if (nev > 200000)
          break;
      };

      //  output.close();

      cout << "FillHEcal: " << nev << " events filled" << endl;
    };

    //------------------------------------------------------------------------------

    //void ShowerCalibrator::SaveAlphas(std::string output_fname) {

    //  //
    //  // Output the gain constants in a format suitable for inclusion in the
    //  // pcal.param file to be used in the analysis.
    //  //

    //  ofstream output;
    //  // char*    fname = Form("pcal.param.%s_%d_%d", "asdf", fNstart, fNstopRequested);
    //  // cout << "SaveAlphas: fname=" << fname << endl;

    //  output.open(output_fname, ios::out);

    //  output << "; Calibration constants for file " << input_file_name << ", " << fNev
    //         << " events processed" << endl;
    //  output << endl;

    //  UInt_t j = 0;

    //  for (UInt_t k = 0; k < THcPShTrack::fNcols_pr; k++) {
    //    k == 0 ? output << "pcal_neg_gain_cor =" : output << "pcal_pos_gain_cor =";
    //    for (UInt_t i = 0; i < THcPShTrack::fNrows_pr; i++)
    //      output << fixed << setw(6) << setprecision(2) << falphaC[j++] << ",";
    //    output << endl;
    //  }

    //  for (UInt_t k = 0; k < THcPShTrack::fNcols_sh; k++) {
    //    k == 0 ? output << "pcal_arr_gain_cor =" : output << "                   ";
    //    for (UInt_t i = 0; i < THcPShTrack::fNrows_sh; i++)
    //      output << fixed << setw(6) << setprecision(2) << falphaC[j++] << ",";
    //    output << endl;
    //  }

    //  output.close();
    //}

    //-----------------------------------------------------------------------------


  } // namespace calibration
} // namespace hallc
