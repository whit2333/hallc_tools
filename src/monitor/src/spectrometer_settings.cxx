#include "clipp.h"
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <stdlib.h>
#ifdef __cpp_lib_filesystem
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"
#include "TCanvas.h"
#include "nlohmann/json.hpp"

#include "THcParmList.h"

//R__LOAD_LIBRARY(libHallA.so)
//R__LOAD_LIBRARY(libdc.so)
//R__LOAD_LIBRARY(libHallC.so)

#include <range/v3/all.hpp>

#include "TGraph.h"

using namespace clipp;
using std::cout;
using std::string;


int main(int argc, char* argv[]) {

//all formatting options (with their default values)
auto clipp_format = doc_formatting{};
    //.start_column(8)                           //column where usage lines and documentation starts
    //.doc_column(20)                            //parameter docstring start col
    //.indent_size(4)                            //indent of documentation lines for children of a documented group
    //.line_spacing(0)                           //number of empty lines after single documentation lines
    //.paragraph_spacing(1)                      //number of empty lines before and after paragraphs
    //.flag_separator(", ")                      //between flags of the same parameter
    //.param_separator(" ")                      //between parameters 
    //.group_separator(" ")                      //between groups (in usage)
    //.alternative_param_separator("|")          //between alternative flags 
    //.alternative_group_separator(" | ")        //between alternative groups 
    //.surround_group("(", ")")                  //surround groups with these 
    //.surround_alternatives("(", ")")           //surround group of alternatives with these
    //.surround_alternative_flags("", "")        //surround alternative flags with these
    //.surround_joinable("(", ")")               //surround group of joinable flags with these
    //.surround_optional("[", "]")               //surround optional parameters with these
    //.surround_repeat("", "...")                //surround repeatable parameters with these
    ////.surround_value("<", ">")                  //surround values with these
    //.empty_label("")                           //used if parameter has no flags and no label
    //.max_alternative_flags_in_usage(1)         //max. # of flags per parameter in usage
    //.max_alternative_flags_in_doc(2)           //max. # of flags per parameter in detailed documentation
    //.split_alternatives(true)                  //split usage into several lines for large alternatives
    //.alternatives_min_split_size(3)            //min. # of parameters for separate usage line
    //.merge_alternative_flags_with_common_prefix(false)  //-ab(cdxy|xy) instead of -abcdxy|-abxy
    ////.merge_joinable_flags_with_common_prefix(true)     //-abc instead of -a -b -c
    //;

  bool   use_shms   = false;
  bool   use_hms    = false;
  bool   use_help   = false;
  string infile     = "DBASE";
  string fmt        = "json";
  int    start_run  = 3890;
  int    N_runs     = 300;
  int    RunNumber  = 3890;
  string replay_dir = ".";
  std::vector<int>  run_list;

  auto cli = ((option("-d", "--replay-dir") & value("dir", replay_dir)) % "Set the replay dir",
              (option("-N") & integer("N_runs", N_runs)) % "number of runs to process",
              (option("-P", "--shms").set(use_shms, true)) % "use SHMS settings",
              (option("-H", "--hms").set(use_hms, true)) % "use HMS settings",
              (repeatable(option("-r", "--runs") & integers("runs", run_list))) %
                  "Set indivindual runs to be used (instead of range)",
              option("-h", "--help").set(use_help, true) % "print help");
  // value("input file", infile),
  // option("-o")&value("output format", fmt),
  // option("-S","--start") & value("start_run", start_run),
  
  assert( cli.flags_are_prefix_free() );
  auto result = parse(argc, argv, cli);

  if (use_help) {
    //cout << make_man_page(cli, argv[0]);
    cout << "Usage:\n" << usage_lines(cli, "progname", clipp_format)
    << "\nOptions:\n" << documentation(cli, clipp_format) << '\n';
    std::exit(0);
  }


  if (!result) {
    auto doc_label = [](const parameter& p) {
      if(!p.flags().empty()) return p.flags().front();
      if(!p.label().empty()) return p.label();
      return doc_string{"<?>"};
    };

    cout << "args -> parameter mapping:\n";
    for(const auto& m : result) {
      cout << "#" << m.index() << " " << m.arg() << " -> ";
      auto p = m.param();
      if(p) {
        cout << doc_label(*p) << " \t";
        if(m.repeat() > 0) {
          cout << (m.bad_repeat() ? "[bad repeat " : "[repeat ")
          <<  m.repeat() << "]";
        }
        if(m.blocked())  cout << " [blocked]";
        if(m.conflict()) cout << " [conflict]";
        cout << '\n';
      }
      else {
        cout << " [unmapped]\n";
      }
    }
    cout << "missing parameters:\n";
    for(const auto& m : result.missing()) {
      auto p = m.param();
      if(p) {
        cout << doc_label(*p) << " \t";
        cout << " [missing after " << m.after_index() << "]\n";
      }
    }
    cout << make_man_page(cli, argv[0]);
    std::exit(EXIT_FAILURE);
  }

  // ---------------------------------
  // Check the input database exists
  std::string file_name           = replay_dir + "/DBASE/COIN/standard.database";
  std::string kinematics_filename = replay_dir + "/DBASE/COIN/standard.kinematics";
  //if( use_shms ) {
  //  file_name           = replay_dir + "/DBASE/SHMS/standard.database";
  //  kinematics_filename = replay_dir + "/DBASE/SHMS/standard.kinematics";
  //}
  fs::path    in_path   = file_name;
  if (!fs::exists(in_path)) {
    std::cerr << "File : " << file_name << " not found.\n";
    return -127;
  }
  std::string db_dir = replay_dir + "/DBASE";
  if (!fs::exists(replay_dir + "/DBASE")) {
    std::cerr << "Directory : " << replay_dir + "/DBASE"
              << " not found.\n";
    return -127;
  }
  //putenv((std::string("DB_DIR=") + db_dir));
  if (setenv("DB_DIR", db_dir.c_str(), 1)) {
    std::cout << "Failed to set env var DB_DIR\n";
    std::exit(EXIT_FAILURE);
  }
  //if(const char* env_p = std::getenv("DB_DIR"))
  //     std::cout << "Your DB_DIR is: " << env_p << '\n';

  //gErrorIgnoreLevel=1; // Ignore Info
  //gErrorIgnoreLevel=1001; // Ignore Info
  //gErrorIgnoreLevel=2001; // Ignore Warnings

  std::string run_list_json  = "DBASE/run_list.json";
  THcParmList* hc_parms = new THcParmList();

  hc_parms->AddString("g_ctp_database_filename", file_name.c_str());
  //hc_parms->Define("gen_run_number", "Run Number", RunNumber);
  //hc_parms->Load(file_name.c_str(), RunNumber);
  //hc_parms->Load(kinematics_filename.c_str(), RunNumber);
  //hc_parms->Load(hc_parms->GetString("g_ctp_parm_filename"));

  //std::cout << "g_ctp_database_filename   : " << hc_parms->GetString("g_ctp_database_filename") << "\n";
  //std::cout << "g_ctp_parm_filename       : " << hc_parms->GetString("g_ctp_parm_filename") << "\n";
  //std::cout << "g_ctp_kinematics_filename : " << hc_parms->GetString("g_ctp_kinematics_filename") << "\n";

  //auto j = nlohmann::json::parse(hc_parms->PrintJSON(RunNumber));
  //auto htheta_lab = hc_parms->Find("htheta_lab");
  //htheta_lab->Print();
  //auto ptheta_lab = hc_parms->Find("ptheta_lab");
  //ptheta_lab->Print();
  //auto hpcentral = hc_parms->Find("hpcentral");
  //hpcentral->Print();
  //auto ppcentral = hc_parms->Find("ppcentral");
  //ppcentral->Print();
  ////std::cout << j.dump()  << "\n";

  if( (!use_shms) && (!use_hms)) {
    use_shms = true;
    use_hms = true;
  }


  if(run_list.size() == 0){
    run_list = std::vector<int>(N_runs);
    std::iota(run_list.begin(), run_list.end(), start_run);
  }

  using namespace ranges;
  auto rng = run_list | view::transform([&](int irun){
    //hc_parms->Define("gen_run_number", "Run Number", irun);
    //hc_parms->Load(file_name.c_str(), irun);
    hc_parms->Load(kinematics_filename.c_str(), irun);
    return std::make_tuple(std::abs(hcana::json::FindVarValueOr(hc_parms,"htheta_lab",0.0)),
                           hcana::json::FindVarValueOr(hc_parms,"ptheta_lab",0.0),
                           hcana::json::FindVarValueOr(hc_parms,"hpcentral" ,0.0),
                           hcana::json::FindVarValueOr(hc_parms,"ppcentral" ,0.0)
                          );
  });
  auto run_shms_angles = view::zip(run_list,rng) 
  | view::remove_if([](auto t){
    return std::get<0>(std::get<1>(t)) == 0.0;
  }) | view::remove_if([](auto t){
    return std::get<1>(std::get<1>(t)) == 0.0;
  }) | view::remove_if([](auto t){
    return std::get<2>(std::get<1>(t)) == 0.0;
  }) | view::remove_if([](auto t){
    return std::get<3>(std::get<1>(t)) == 0.0;
  });

  for( auto en : (run_shms_angles | to_<std::vector>())) {
    std::cout << std::get<0>(en) <<  " :"; 
    if(use_hms) {
    std::cout << " HMS : ";
    std::cout << std::get<2>(std::get<1>(en)) << " GeV/c at ";
    std::cout << std::get<0>(std::get<1>(en)) << " deg  ";
    }
    if(use_shms) {
    std::cout << "SHMS : ";
    std::cout << std::get<3>(std::get<1>(en)) << " GeV/c at ";
    std::cout << std::get<1>(std::get<1>(en)) << " deg";
    }
    std::cout << "\n";
  }

}
