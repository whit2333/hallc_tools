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
  bool                     update           = false;
  bool                     verbose          = false;
  bool                     use_gui          = false;
  int                      N_events         = 0;
  int                      run_number       = 0;
  uint64_t                 start_event      = 0;
  bool                     help             = false;
  string                   dir              = "";
  string                   infile           = "pcal_calib.json";
  string                   output_name      = "";
  string                   tree_name        = "T";
  string                   rootfile         = "";

  auto cli =
      ("directory to find default calibration files" %
           (option("-d", "--dir") & value("directory", dir)),
       "input calibration file" % (option("-i", "--input") & value("input file", infile)),
       "output calibration file name" %
           (option("-o", "--output") & value("output file", output_name)),
       option("-f", "--force").set(force_out) % "force the output to overwrite existing files",
       option("-u", "--update").set(update) % "update the input calibration file",
       option("-g", "--gui").set(use_gui) % "update the input calibration file",
       //"starting event - skips the first <evt> events " %
       //    (option("-S", "--start-event") & value("evt", start_event)),
       "calibration run number" % (option("-R", "--run-number") & value("run", run_number)),
       "Number of events to process from start event number" %
           (option("-N", "--Nevents") & value("N events", N_events)),
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
  char*   dum_argv[] = {"app"};
  TRint*  app    = nullptr;

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

  fs::path in_path = dir + infile;
  if (!fs::exists(in_path)) {
    std::cerr << "File : " << dir + infile << " not found.\n";
    return -127;
  }
  infile = in_path.string();

  //fs::path in_path_prefix = in_path;
  //in_path_prefix.replace_extension("");
  fs::path out_path = dir + outfile;
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
  theShowerCalib.Process(rootfile);
  std::cout << "writing calibration to file : \n " << outfile << "\n";
  theShowerCalib.WriteCalibration(run_number, outfile);

  if (use_gui) {
    app->Run();
  }
}

