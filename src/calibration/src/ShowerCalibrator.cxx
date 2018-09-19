#include "calibration/THcPShTrack.h"
#include "calibration/ShowerCalibrator.h"
#include "calibration/ShowerTrack.h"

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
#include "TH1D.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TFitResult.h"
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

    void ShowerCalibrator::CalcThresholds() {
      // Calculate +/-3 RMS thresholds on the uncalibrated total energy
      // depositions. These thresholds are used mainly to exclude potential
      // hadronic events due to the gas Cherenkov inefficiency.

      // Histogram uncalibrated energy depositions, get mean and RMS from the
      // histogram, establish +/-3 * RMS thresholds.

      // cout<< "ShowerCalibrator::CalcThresholds: FNentries = " << fNentries << endl;
      //  cout << "ShowerCalibrator::CalcThresholds: fNstart = " << fNstart << " "
      //       << "  fNstop = " << fNstop << endl;

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

      auto df_with_cuts =
          df.Filter([&](const double& n) { return int(n) == 1; }, {"P.tr.n"})
              .Filter(
                  [&](doublers& dp) {
                    return (dp.at(0) > _calibration.fDeltaMin) &&
                           (dp.at(0) < _calibration.fDeltaMax);
                  },
                  {"P.tr.tg_dp"})
              .Filter(
                  [&](doublers& npe) {
                    return (npe[0] + npe[1] + npe[2] + npe[3]) > _calibration.fHGCerMin;
                  },
                  {"P.hgcer.npe"})
              .Filter(
                  [&](doublers& npe) {
                    return (npe[0] + npe[1] + npe[2] + npe[3]) > _calibration.fNGCerMin;
                  },
                  {"P.ngcer.npe"})
              .Filter(
                  [&](doublers& beta) {
                    return (beta.at(0) > _calibration.fBetaMin) &&
                           (beta.at(0) < _calibration.fBetaMax);
                  },
                  {"P.tr.beta"})
              .Filter(
                  [&](doublers& th, doublers& y, doublers& yp, doublers& dp) {
                    return collimator_cut(th.at(0), y.at(0), yp.at(0), dp.at(0));
                  },
                  {"P.tr.tg_th", "P.tr.tg_y", "P.tr.tg_ph", "P.tr.tg_dp"})
              .Filter("P.cal.nclust==1")
              .Filter("P.cal.fly.nclust==1")
              .Define("shower_track",
                      [&](doublers& x, doublers& y, doublers& xp, doublers& yp, doublers& p,
                          doublers& dp) {
                        return ShowerTrackInfo{p.at(0),dp.at(0),x.at(0),xp.at(0),y.at(0),yp.at(0)};
                      },
                      {"P.tr.x", "P.tr.y", "P.tr.th", "P.tr.ph", "P.tr.p", "P.tr.tg_dp"});
      ;

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

      auto d2 =
          df_with_cuts
              .Define("trk",
                      [&](const ShowerTrackInfo& st, const doublers& pr_neg, const doublers& pr_pos,
                          const doublers& shower) {
                        ShowerTrack trk(st, &_calibration);
                        uint64_t    nb      = 0;
                        double      total_E = 0.0;
                        for (const auto& val : pr_neg) {
                          if (val > 0.0) {
                            trk.AddHit(nb, val);
                          }
                          nb++;
                        }
                        for (const auto& val : pr_pos) {
                          if (val > 0.0) {
                            trk.AddHit(nb, val);
                          }
                          nb++;
                        }
                        // Set Shower hits.
                        for (const auto& val : shower) {
                          if (val > 0.0) {
                            trk.AddHit(nb, val);
                          }
                          nb++;
                        }
                        // std::cout << trk.Enorm() << "\n";
                        // std::cout << trk.GetP() << "\n";
                        // trk.Print();
                        return trk;
                      },
                      {"shower_track", "P.cal.pr.goodNegAdcPulseInt", "P.cal.pr.goodPosAdcPulseInt",
                       "P.cal.fly.goodAdcPulseInt"})
              .Define("E_shower_cal0", [&](const ShowerTrack& trk) { return trk.Enorm(); }, {"trk"})
              .Define("E_times_p",
                      [&](const ShowerTrackInfo& st, double Etot) { return (st._P) * Etot; },
                      {"shower_track", "E_shower_cal0"})
              .Define("E_tot2", [](double Etot) { return Etot * Etot; }, {"E_shower_cal0"})
              ;

      TCanvas* c = new TCanvas();
      c->Divide(2,2);
      c->cd(1);

      std::cout << " Please ignore the clang errors above and below...\n" ;
      auto h_Euncalib =
          d2.Histo1D({"h_Euncalib", "; E/p total", 100, 0.8, 1.8}, "E_shower_cal0");
      TH1D* hEunc = (TH1D*)h_Euncalib->Clone("hEunc");
      std::cout <<  " entries : " << *(d2.Count()) << "\n";

      TFitResultPtr r = hEunc->Fit("gaus", "S", "", 0.8,1.8);//_calibration.fEuncGFitLo, _calibration.fEuncGFitHi);
      //hEunc->Fit("gaus", "0", "", _calibration.fEuncGFitLo, _calibration.fEuncGFitHi);
      //hEunc->Fit("gaus", "", "", _calibration.fEuncGFitLo, _calibration.fEuncGFitHi);
      hEunc->GetFunction("gaus")->SetLineColor(2);
      hEunc->GetFunction("gaus")->SetLineWidth(2);
      hEunc->GetFunction("gaus")->SetLineStyle(1);
      TF1*     fit    = hEunc->GetFunction("gaus");
      std::cout << r << std::endl;
      Double_t gmean  = r.Get()->Parameter(1);
      Double_t gsigma = r.Get()->Parameter(2);
      double fLoThr          = gmean - 3. * gsigma;
      double fHiThr          = gmean + 3. * gsigma;
      cout << "CalcThreshods: fLoThr   = " << fLoThr << "\n";
      cout << "               fHiThr   = " << fHiThr << "\n";

      auto disp0 = d2.Display({"E_shower_cal0", "P.cal.pr.goodNegAdcPulseInt",
                               "P.cal.pr.goodPosAdcPulseInt", "P.cal.fly.goodAdcPulseInt"});
      disp0->Print();

      auto d3 =
          d2.Filter([&](double Enorm) { return (Enorm > fLoThr) && (Enorm < fHiThr); },
                    {"E_shower_cal0"});

      MatrixQuantities mq;
      d3.Foreach(
          [&mq](const ShowerTrack& trk) {
            auto new_trk = trk.UpdatedTrack(nullptr);
            mq.fe0 += new_trk.GetP();
            for (const auto& ahit : new_trk.GetHits()) {
              mq.fq0[ahit._channel] += ahit._energy;
              mq.fqe[ahit._channel] += ahit._energy * new_trk.GetP();
              mq.fHitCount[ahit._channel]++;
              for (const auto& bhit : new_trk.GetHits()) {
                mq.fQ(ahit._channel,bhit._channel) += ahit._energy*bhit._energy;
                if (ahit._channel != bhit._channel) {
                  mq.fQ(ahit._channel,bhit._channel) += ahit._energy * bhit._energy;
                }
              }
            }
          },
          {"trk"});
      auto   d3_Nev = d3.Count();
      double Nev    = *d3_Nev;
      std::cout << " Nev " << Nev << "\n";

      mq.fq0 *= (1.0 / Nev);
      mq.fqe *= (1.0 / Nev);
      mq.fQ  *= (1.0 / Nev);
      mq.fe0 *= (1.0 / Nev);

      for (uint64_t i = 0; i < fNpmts; i++) {
        if (mq.fHitCount(i) < 20){//_calibration.fMinHitCount) {
          //cout << "Channel " << i << ", " << mq.fHitCount(i) << " hits, will not be calibrated." << endl;
          mq.fq0(i) = 0.;
          mq.fqe(i) = 0.;
          for (uint64_t k = 0; k < fNpmts; k++) {
            mq.fQ(i,k) = 0.;
            mq.fQ(k,i) = 0.;
          }
          mq.fQ(i,i) = 1.0;
        }
      }

      MatrixQuantities::LU_t lu(mq.fQ);
      //mq.falphaU = mq.fQ.householderQr().solve(fqe);
      //mq.falphaU = mq.fQ.fullPivLu().solve(fqe);
      //mq.falphaU = mq.fQ.fullPivHouseholderQr().solve(fqe);
      mq.falphaU = lu.solve(mq.fqe);
      //std::cout << " alphaU " << mq.falphaU.transpose() << "\n";
      //std::cout << " fqe " << mq.fqe.transpose() << "\n";
      //std::cout << " hitcount  " << mq.fHitCount.transpose() << "\n";
      //std::cout << " diag Q " << mq.fQ.diagonal().transpose() << "\n";
      //std::cout << "fQ \n";
      //std::cout << mq.fQ.transpose() << "\n";

      double t1 = mq.fe0 - mq.falphaU.dot(mq.fq0); // temporary variable.
      //std::cout << " t " << t1 << "\n";
      //std::cout << " |Q| " << mq.fQ.determinant() << "\n";
      //fqe[nb - 1] += hit->GetEdep() * trk.GetP();
      //fq0[nb - 1] += hit->GetEdep();

      //std::cout << " fq0 " << mq.fq0 << "\n";
      //std::cout << " fqe " << mq.fqe << "\n";
      //std::cout << " fe0 " << mq.fe0 << "\n";
      //for(const auto& v : mq.fq0) {
      //  std::cout << v ", ";
      //} std::cout << "\n";
      //mq.fQi = lu.Solve(q0, ok);
      //cout << "Qiq0: ok=" << ok << endl;
      //  Qiq0.Print();
      
      mq.fQiq0 = mq.fQ.fullPivLu().solve(mq.fq0);
      double t2 = mq.fq0.dot(mq.fQiq0); // another temporary variable
      //  cout << "t2 =" << t2 << endl;

      mq.falphaC = (t1 / t2) * mq.fQiq0 + mq.falphaU; // the sought gain constants
      //std::cout << " falphaC " << mq.falphaC.transpose() << "\n";

      std::array<double,fNpmts> temp;
      Eigen::Map<typename MatrixQuantities::Vector_t>(temp.data(),fNpmts) = mq.falphaC;

      _calibration.SetGainCoeffs(temp);

      auto d4 = d2.Define("trk_2",
                          [&](const ShowerTrackInfo& st, const doublers& pr_neg,
                              const doublers& pr_pos, const doublers& shower) {
                            ShowerTrack trk(st, &_calibration);
                            uint64_t    nb = 0;
                            for (const auto& val : pr_neg) {
                              if (val > 0.0) {
                                trk.AddHit(nb, val);
                              }
                              nb++;
                            }
                            for (const auto& val : pr_pos) {
                              if (val > 0.0) {
                                trk.AddHit(nb, val);
                              }
                              nb++;
                            }
                            // Set Shower hits.
                            for (const auto& val : shower) {
                              if (val > 0.0) {
                                trk.AddHit(nb, val);
                              }
                              nb++;
                            }
                            return trk;
                          },
                          {"shower_track", "P.cal.pr.goodNegAdcPulseInt",
                           "P.cal.pr.goodPosAdcPulseInt", "P.cal.fly.goodAdcPulseInt"})
                    .Define("E_shower_cal2", [&](const ShowerTrack& trk) { return trk.Enorm(); },
                            {"trk_2"});


      auto h_Euncalib2 = d4.Histo1D({"h_Euncalib2", "; E/p total", 100, 0.8, 1.8}, "E_shower_cal2");

      h_Euncalib2->SetLineColor(4);
      h_Euncalib2->SetLineWidth(2);
      h_Euncalib2->DrawCopy("same");
      //std::cout << "derp3\n";
      //TH1D* hEunc2 = (TH1D*)h_Euncalib2->Clone("hEunc2");
      //hEunc2->SetLineColor(2);
      //hEunc2->Draw("same");

      //c->cd(2);
      //hqe->Draw();
      //c->cd(4);
      //hq0->Draw();
    }

    //------------------------------------------------------------------------------



  } // namespace calibration
} // namespace hallc
