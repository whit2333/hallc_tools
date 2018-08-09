#include "clipp.h"
#include <cmath>
#include <iostream>
#include <iomanip>
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

enum class RunMode { standard, build, filter, find, help };


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

  RunMode          mode       = RunMode::standard;
  bool             use_shms   = false;
  bool             use_hms    = false;
  bool             use_help   = false;
  bool             use_json_input   = false;
  string           json_data  = "settings.json";
  string           infile     = "DBASE";
  string           fmt        = "json";
  int              start_run  = 3890;
  int              N_runs     = 300;
  int              RunNumber  = 3890;
  string           replay_dir = ".";
  std::vector<int> run_list;
  string           build_type = "json";
  string           table_name = "settings.json";

  auto first_args =
      (((option("-d", "--replay-dir") & value("dir", replay_dir)) % "Set the replay dir" |
        (option("-j", "--json-data").set(use_json_input, true) & value("data", json_data)) %
            "use json data as input instead of DBASE"),
       (option("-P", "--shms").set(use_shms, true)) % "use SHMS settings",
       (option("-H", "--hms").set(use_hms, true)) % "use HMS settings",
       (option("-N", "--number-of-runs") & integer("N_runs", N_runs)) % "number of runs to process",
       (option("-S", "--start") & integer("start_run", start_run)) % "start run");
  auto last_args = (option("-h", "--help").set(use_help, true) % "print help");
  //(option("-t", "--type") & value("type", build_type)) % "set the build type");
  auto standard_args = (repeatable(option("-r", "--runs") & integers("runs", run_list)) %
                        "Set indivindual runs to be used (instead of range)");

  auto build_cmd  = (command("build").set(mode, RunMode::build) % "build mode" &
                    value("table_name", table_name) % "Output file");
  auto filter_cmd = (command("filter").set(mode, RunMode::filter) % "filter mode");
  auto print_cmd  = (command("print").set(mode, RunMode::standard) % "print mode");

  auto cli = ( first_args,
              standard_args,
              build_cmd | print_cmd | filter_cmd,
              last_args
             );

  // value("input file", infile),
  // option("-o")&value("output format", fmt),
  
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
  //
  nlohmann::json j_in;
  if(use_json_input) {
    fs::path    in_path   = json_data;
    if (!fs::exists(in_path)) {
      std::cerr << "File : " << json_data << " not found.\n";
      return -127;
    }
    std::ifstream jinput(json_data);
    jinput >> j_in;
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
  gErrorIgnoreLevel=2001; // Ignore Warnings

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

  if( (!use_shms) && (!use_hms)) {
    use_shms = true;
    use_hms = true;
  }

  if(run_list.size() == 0){
    run_list = std::vector<int>(N_runs);
    std::iota(run_list.begin(), run_list.end(), start_run);
  }

  // ----------------------------------------------------------
  //
  using namespace ranges;
  auto rng =
      run_list | view::transform([&](int irun) {
        hc_parms->Load(kinematics_filename.c_str(), irun);
        return std::make_tuple(hcana::json::FindVarValueOr(hc_parms, "htheta_lab", 0.0),
                               hcana::json::FindVarValueOr(hc_parms, "hpcentral", 0.0),
                               hcana::json::FindVarValueOr(hc_parms, "ptheta_lab", 0.0),
                               hcana::json::FindVarValueOr(hc_parms, "ppcentral", 0.0));
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

  auto output_settings = (run_shms_angles | to_<std::vector>());

  nlohmann::json j;
  for (auto en : output_settings) {
    int arun_num = std::get<0>(en);
    if(mode == RunMode::standard){
      std::cout << arun_num <<  " :"; 
    }
    if(use_hms) {
      if(mode == RunMode::standard){
        std::cout << " HMS : ";
        std::cout << std::get<1>(std::get<1>(en)) << " GeV/c at ";
        std::cout << std::get<0>(std::get<1>(en)) << " deg  ";
      }
      j[std::to_string(arun_num)]["hpcentral"]  = std::get<1>(std::get<1>(en));
      j[std::to_string(arun_num)]["htheta_lab"] = std::get<0>(std::get<1>(en));
    }
    if(use_shms) {
      if(mode == RunMode::standard){
        std::cout << "SHMS : ";
        std::cout << std::get<3>(std::get<1>(en)) << " GeV/c at ";
        std::cout << std::get<2>(std::get<1>(en)) << " deg";
      }
      j[std::to_string(arun_num)]["ppcentral"]  = std::get<3>(std::get<1>(en));
      j[std::to_string(arun_num)]["ptheta_lab"] = std::get<2>(std::get<1>(en));
    }
    if(mode == RunMode::standard){
      std::cout << "\n";
    }
  }

  if(mode == RunMode::build){
    ofstream out_file(table_name);
    out_file << std::setw(4) << j << std::endl;
  }
}
