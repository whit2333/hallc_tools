#include <iostream>
#include <chrono>
#include <map>
#ifdef __cpp_lib_filesystem
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

#include "clipp.h"
using namespace clipp;

#include <TCanvas.h>
#include <TF1.h>
#include <TH1.h>
#include <TStyle.h>
#include <TRint.h>

#include "calibration/THcPShowerCalib.h"
#include "calibration/ShowerCalibrator.h"
#include "calibration/CalorimeterCalibration.h"

using std::cout;
using std::endl;
using std::string;

//void shms_cal_calib(string Prefix, int nstop = -1, int nstart = 0);

int main(int argc, char* argv[]) {

  bool                     force_out        = false;
  bool                     verbose          = false;
  int                      N_events         = 0;
  int                      run_number       = 0;
  uint64_t                 start_event      = 0;
  bool                     help             = false;
  string                   infile           = "";
  string                   output_name      = "";
  string                   tree_name        = "hc_cal_calibration";

  auto cli =
      ("input file name" % (required("-i", "--input") & value("input file", infile)),
       "output file name" % (option("-o", "--output") & value("output file", output_name)),
       option("-f", "--force").set(force_out) % "force the output to overwrite existing files",
       "starting event - skips the first <evt> events " %
           (option("-S", "--start-event") & value("evt", start_event)),
       "calibration run number" %
           (option("-R", "--run-number") & value("run", run_number)),
       "Number of events to process from start event number" %
           (option("-N", "--Nevents") & value("N events", N_events)),
       "output tree name" % (option("-T", "--tree-name") & value("tree name", tree_name)),
       option("-h", "--help").set(help) % "print help",
       option("-v", "--verbose").set(verbose) % "turn on verbose output");

  if (!parse(argc, argv, cli)) {
    cout << make_man_page(cli, argv[0]);
    std::quick_exit(0);
  }
  if (help) {
    cout << make_man_page(cli, argv[0]);
    std::quick_exit(0);
  }


  int     dum_argc   = 1;
  char*   dum_argv[] = {"app"};
  TRint*  app    = new TRint("App", &dum_argc, dum_argv);

  // ------------------------------
  // file checks
  string outfile = output_name;

  fs::path in_path = infile;
  if (!fs::exists(in_path)) {
    std::cerr << "File : " << infile << " not found.\n";
    return -127;
  }

  if (in_path.extension() != fs::path(".root")) {
    std::cerr << "File : " << infile << " not a root file.\n";
    return -127;
  }

  fs::path in_path_prefix = in_path;
  in_path_prefix.replace_extension("");
  // fs::path out_path = outfile;
  // if (outfile.empty()) {
  //  out_path = in_path;
  //  out_path.replace_extension(".root");
  //  outfile = out_path.string();
  //}
  // if (out_path.extension() != fs::path(".root")) {
  //  out_path.replace_extension(".root");
  //  outfile = out_path.string();
  //}

  // if (fs::exists(out_path)) {
  //  if (force_out) {
  //    if(verbose) {
  //      std::cout << out_path << " will be overwritten\n";
  //    }
  //  } else {
  //    std::cerr << "error: file " << out_path << " exists (use -f to force overwrite).\n";
  //    return -127;
  //  }
  //}
  cout << "using    " << in_path << "\n";
  cout << "prefix   " << in_path_prefix << "\n";

  using namespace hallc::calibration;

  //hallc::calibration::CalorimeterCalibration cal_obj(run_number);
  //cal_obj.;

  //cal_obj.ReadLegacyCalibration();
  //std::cout << cal_obj.PrepareJson() << "\n";
  //cal_obj.BuildTester(1234);

  ShowerCalibrator theShowerCalib;
  theShowerCalib.input_file_name = in_path;
  theShowerCalib._calibration.LoadJsonCalibration("pcal_calib.json", run_number);
  theShowerCalib._calibration.Print();
  theShowerCalib.CalcThresholds();
  theShowerCalib._calibration.WriteCalibration(run_number);

  app->Run();

}

// ----------------------------------------------------------------------

//void shms_cal_calib(string Prefix, int nstop, int nstart) {
//  //
//  // A steering Root script for the SHMS calorimeter calibration.
//  //
//
//
//  //chrono::steady_clock sc;               // create an object of `steady_clock` class
//  //auto                 start = sc.now(); // start timer
//  //// do stuff....
//
//  //auto end = sc.now(); // end timer (starting & ending is done by measuring the time at the moment
//  //                     // the process started & ended respectively)
//  //auto time_span =
//  //    static_cast<chrono::duration<double>>(end - start); // measure time span between start & end
//  //cout << "Operation took: " << time_span.count() << " seconds !!!";
//  //return 0;
//
//  // Initialize the analysis clock
//  clock_t t = clock();
//
//  cout << "Calibrating file " << Prefix << ".root, events " << nstart << " -- " << nstop << endl;
//
//  THcPShowerCalib theShowerCalib(Prefix, nstart, nstop);
//
//  std::cout << " ReadThresholds\n";
//  theShowerCalib.ReadThresholds(); // Read in threshold param-s and intial gains
//  std::cout << " Init\n";
//  theShowerCalib.Init();           // Initialize constants and variables
//  std::cout << " CalcThresholds\n";
//  theShowerCalib.CalcThresholds(); // Thresholds on the uncalibrated Edep/P
//  std::cout << " ComposeVMs\n";
//  theShowerCalib.ComposeVMs();     // Compute vectors amd matrices for calib.
//  std::cout << " SolveAlphas\n";
//  theShowerCalib.SolveAlphas();    // Solve for the calibration constants
//  std::cout << " SaveAlphas\n";
//  theShowerCalib.SaveAlphas();     // Save the constants
//  // theShowerCalib.SaveRawData();   // Save raw data into file for debuging
//  std::cout << " FillHEcal\n";
//  theShowerCalib.FillHEcal(); // Fill histograms
//
//  // Plot histograms
//
//  TCanvas* Canvas = new TCanvas("Canvas", "PHMS Shower Counter calibration", 1000, 667);
//  Canvas->Divide(2, 2);
//
//  Canvas->cd(1);
//
//  // Normalized uncalibrated energy deposition.
//
//  theShowerCalib.hEunc->DrawCopy();
//
//  theShowerCalib.hEuncSel->SetFillColor(kGreen);
//  theShowerCalib.hEuncSel->DrawCopy("same");
//
//  Canvas->cd(2);
//  theShowerCalib.hESHvsEPR->Draw("colz");
//
//  // Normalized energy deposition after calibration.
//
//  Canvas->cd(3);
//  gStyle->SetOptFit();
//
//  theShowerCalib.hEcal->Fit("gaus", "O", "", 0.5, 1.5);
//  TF1*     fit    = theShowerCalib.hEcal->GetFunction("gaus");
//  Double_t gmean  = fit->GetParameter(1);
//  Double_t gsigma = fit->GetParameter(2);
//  double   gLoThr = gmean - 2. * gsigma;
//  double   gHiThr = gmean + 2. * gsigma;
//  cout << "gLoThr=" << gLoThr << "  gHiThr=" << gHiThr << endl;
//  theShowerCalib.hEcal->Fit("gaus", "", "", gLoThr, gHiThr);
//
//  theShowerCalib.hEcal->GetFunction("gaus")->SetLineColor(2);
//  theShowerCalib.hEcal->GetFunction("gaus")->SetLineWidth(1);
//  theShowerCalib.hEcal->GetFunction("gaus")->SetLineStyle(1);
//
//  // SHMS delta(P) versus the calibrated energy deposition.
//
//  Canvas->cd(4);
//  theShowerCalib.hDPvsEcal->Draw("colz");
//
//  // Save canvas in a pdf format.
//  Canvas->Print(Form("%s_%d_%d.pdf", Prefix.c_str(), nstart, nstop));
//
//  // Save histograms in root file.
//
//  TFile* froot = new TFile(Form("%s_%d_%d.root", Prefix.c_str(), nstart, nstop), "RECREATE");
//  theShowerCalib.hEunc->Write();
//  theShowerCalib.hEuncSel->Write();
//  theShowerCalib.hESHvsEPR->Write();
//  theShowerCalib.hEcal->Write();
//  theShowerCalib.hDPvsEcal->Write();
//  theShowerCalib.hETOTvsEPR->Write();    ////
//  theShowerCalib.hEPRunc->Write();       ////
//  theShowerCalib.hETOTvsEPRunc->Write(); ////
//  theShowerCalib.hESHvsEPRunc->Write();  ////
//  froot->Close();
//
//  // Calculate the analysis rate
//  t = clock() - t;
//  printf("The analysis took %.1f seconds \n", ((float)t) / CLOCKS_PER_SEC);
//}
