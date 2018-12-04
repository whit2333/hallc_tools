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

//#include "calibration/THcPShowerCalib.h"
#include "calibration/ShowerCalibrator.h"
#include "calibration/CalorimeterCalibration.h"

using std::cout;
using std::endl;
using std::string;

int main(int argc, char* argv[]) {

  bool                     force_out        = false;
  bool                     update           = false;
  bool                     verbose          = false;
  bool                     use_gui          = false;
  bool                     save_plots       = false;
  int                      N_events         = 0;
  int                      run_number       = 0;
  uint64_t                 start_event      = 0;
  bool                     help             = false;
  string                   dir              = "db2/";
  string                   infile           = dir+"pcal_calib.json";
  string                   output_name      = dir+"pcal_calib_new.json";
  string                   tree_name        = "T";
  string                   rootfile         = "";
  string                   plot_file_name   = "shms_cal_calib.pdf";
  double                   weight           = 0.5;

  auto cli =
      ("directory to find default calibration files" %
           (option("-d", "--dir") & value("directory", dir)),
       "input calibration file" % (option("-i", "--input") & value("input file", infile)),
       "output calibration file name" %
           (option("-o", "--output") & value("output file", output_name)),
       option("-f", "--force").set(force_out) % "force the output to overwrite existing files",
       option("-u", "--update").set(update) % "update the input calibration file",
       option("-g", "--gui").set(use_gui) % "update the input calibration file",
       "saves plots to plot_file (Default: \"shms_cal_calib_plots.pdf\"" %
           (option("-s", "--save-plots").set(save_plots, true) &
            opt_value("plot_file", plot_file_name)),
       //"starting event - skips the first <evt> events " %
       //    (option("-S", "--start-event") & value("evt", start_event)),
       "calibration run number" % (option("-R", "--run-number") & value("run", run_number)),
       "Number of events to process from start event number" %
           (option("-N", "--Nevents") & value("N events", N_events)),
       "weight for updating new calibration [0-1]" %
           (option("-w", "--weight") & value("weight", weight)),
       "output tree name" % (option("-T", "--tree-name") & value("tree name", tree_name)),
       option("-h", "--help").set(help) % "print help",
       option("-v", "--verbose").set(verbose) % "turn on verbose output",
       "ROOT file " % required("-F", "--rootfile") & value("rootfile", rootfile));

  if (!parse(argc, argv, cli)) {
    cout << make_man_page(cli, argv[0]);
    std::quick_exit(0);
  }
  if (help) {
    cout << make_man_page(cli, argv[0]);
    std::quick_exit(0);
  }

  int     dum_argc   = 1;
  char*   dum_argv[] = {(char*)"app"};
  TRint*  app    = nullptr;

  gPrintViaErrorHandler = kTRUE;
  gErrorIgnoreLevel     = kError;
  if (use_gui) {
    app = new TRint("App", &dum_argc, dum_argv);
  }
  // ------------------------------
  // file checks
  string outfile = output_name;

  fs::path rootfile_path = rootfile;
  if (!fs::exists(rootfile_path)) {
    std::cerr << "ROOT File : " << rootfile << " not found.\n";
    return -127;
  }
  if (rootfile_path.extension() != fs::path(".root")) {
    std::cerr << "File : " << rootfile_path << " not a root file.\n";
    return -127;
  }

  fs::path in_path = infile;
  if (!fs::exists(in_path)) {
    std::cerr << "File : " << infile << " not found.\n";
    return -127;
  }
  infile = in_path.string();

  //fs::path in_path_prefix = in_path;
  //in_path_prefix.replace_extension("");
  fs::path out_path = outfile;
  if (outfile.empty()) {
    //std::cout << " empty \n";
    if (update) {
      // if forced output is set but not the output,
      // we assume the input file is the output file
      out_path = in_path;
    } else {
      out_path = dir + "pcal_calib_new.json";
    }
    outfile = out_path.string();
  }
  if (fs::exists(out_path)) {
    if (force_out || update) {
      if(verbose) {
        std::cout << out_path << " will be overwritten\n";
      }
      outfile = out_path.string();
    } else {
      std::cerr << "error: file " << out_path << " exists (use -f to force overwrite).\n";
      return -127;
    }
  }
  cout << "using in         " << infile << "\n";
  cout << "using out        " << outfile << "\n";
  cout << "using rootfile   " << rootfile << "\n";

  using namespace hallc::calibration;

  ShowerCalibrator theShowerCalib(infile, outfile);
  theShowerCalib.UseGui(use_gui);
  theShowerCalib.LoadCalibration(run_number);
  if (verbose) {
    theShowerCalib.Print();
  }
  // Get a copy of the initial calibration 
  CalorimeterCalibration cal_0(theShowerCalib._calibration);

  // Process data to get new calibration 
  theShowerCalib.Process(rootfile);
  if (verbose) {
    theShowerCalib.Print();
  }
  // Get the new calibraiton 
  CalorimeterCalibration cal_1(theShowerCalib._calibration);
  // merge 
  cal_0.Merge(cal_1,weight);
  theShowerCalib._calibration = cal_0;
  theShowerCalib.UpdatePlots(rootfile);

  std::cout << "writing calibration to file : \n " << outfile << "\n";
  theShowerCalib.WriteCalibration(run_number, outfile);
  if (save_plots) {
    fs::path plot_file_path = plot_file_name;
    std::vector<string> extns = {".pdf", ".png"};
    if (!std::count(extns.begin(), extns.end(), plot_file_path.extension().string())) {
      std::cerr << "error: file extension " << plot_file_path.extension() << " not supported\n";
      return -127;
    }
    theShowerCalib.SavePlots(plot_file_name);
  }
  if (use_gui) {
    app->Run();
  }
}

