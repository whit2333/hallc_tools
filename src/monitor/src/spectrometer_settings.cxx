#include <cmath>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <array>
#include <stdlib.h>
#ifdef __cpp_lib_filesystem
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif
#if !defined(__CLING__)
#include <fmt/core.h>
#include <fmt/ostream.h>
#endif

#include "TCanvas.h"
#include "TGraph.h"

#include <iostream>
#include "THcParmList.h"

#include "spectrometer_settings.h"

using namespace clipp;
using std::cout;
using std::string;
using namespace ranges;

/**  Build output range with json database.
 *
 */
table_range_t build_range_with_json(std::string dbfile, std::vector<int> runlist, bool all ) {
  using namespace ranges;
  nlohmann::json  j_in;
  fs::path        in_path = dbfile;

  bool is_piped = false;
  if (!isatty(fileno(stdin))) {
    is_piped = true;
  }
  
  if(!is_piped) {
    if (!fs::exists(in_path)) {
      std::cerr << "File : " << dbfile << " not found.\n";
      std::exit(EXIT_FAILURE);
    }
  }

  std::ifstream jinput(dbfile);
  std::istream& in_stream = is_piped ? std::cin : jinput;
  in_stream >> j_in;

  if(!all) {
    auto rng = runlist | view::transform([&](int irun) {
      auto rn = std::to_string(irun);
      table_entry_t  res;
      if(j_in[rn].is_null()){
        res["hms"]["angle"] = 0.0;
        res["hms"]["momentum"] = 0.0;
        res["shms"]["angle"] = 0.0;
        res["shms"]["momentum"] = 0.0;
        return res;
      }
      if (j_in[rn].find("htheta_lab") != j_in[rn].end()) {
        res["hms"]["angle"]     = j_in[rn]["htheta_lab"].get<double>();
      }
      if (j_in[rn].find("hpcentral") != j_in[rn].end()) {
        res["hms"]["momentum"]  = j_in[rn]["hpcentral"].get<double>();
      }
      if (j_in[rn].find("ptheta_lab") != j_in[rn].end()) {
        res["shms"]["angle"]    = j_in[rn]["ptheta_lab"].get<double>();
      }
      if (j_in[rn].find("ppcentral") != j_in[rn].end()) {
        res["shms"]["momentum"] = j_in[rn]["ppcentral"].get<double>();
      }
      return res;
    });
    return view::zip(runlist, rng)| to_<std::vector>();
  }

  table_range_t res_tbl_range;
  for (nlohmann::json::iterator it = j_in.begin(); it != j_in.end(); ++it) {
      table_entry_t  res;
      if (it.value().find("htheta_lab") != it.value().end()) {
        res["hms"]["angle"]     = it.value()["htheta_lab"].get<double>();
        res["hms"]["momentum"]  = it.value()["hpcentral"].get<double>();
      } else {
        res["hms"]["angle"]     = 0.0;//it.value()["htheta_lab"].get<double>();
        res["hms"]["momentum"]  = 0.0;//it.value()["hpcentral"].get<double>();
      }

      if (it.value().find("ptheta_lab") != it.value().end()) {
        res["shms"]["angle"]    = it.value()["ptheta_lab"].get<double>();
        res["shms"]["momentum"] = it.value()["ppcentral"].get<double>();
      } else {
        res["shms"]["angle"]    = 0.0;//it.value()["ptheta_lab"].get<double>();
        res["shms"]["momentum"] = 0.0;//it.value()["ppcentral"].get<double>();
      }

      res_tbl_range.push_back(std::make_pair(std::stoi(it.key()),res));
  }
  return res_tbl_range;
}


/** Build output range using replay DBASE.
 *
 */
table_range_t build_range_with_DBASE(std::string dbfile, std::vector<int> runlist, std::string spec_daq) {
  using namespace ranges;
  // build with hallc DBASE
  // ---------------------------------
  // Check the input database exists
  std::string file_name           = dbfile + "/DBASE/"+spec_daq+"/standard.database";
  std::string kinematics_filename = dbfile + "/DBASE/"+spec_daq+"/standard.kinematics";

  fs::path in_path = file_name;
  if (!fs::exists(in_path)) {
    std::cerr << "File : " << file_name << " not found.\n";
    std::exit(EXIT_FAILURE);
  }
  std::string db_dir = dbfile + "/DBASE";
  if (!fs::exists(dbfile + "/DBASE")) {
    std::cerr << "Directory : " << dbfile + "/DBASE"
              << " not found.\n";
    std::exit(EXIT_FAILURE);
  }
  if (setenv("DB_DIR", db_dir.c_str(), 1)) {
    std::cout << "Failed to set env var DB_DIR\n";
    std::exit(EXIT_FAILURE);
  }
  // if(const char* env_p = std::getenv("DB_DIR"))
  //     std::cout << "Your DB_DIR is: " << env_p << '\n';

  // gErrorIgnoreLevel=1; // Ignore Info
  // gErrorIgnoreLevel=1001; // Ignore Info
  gErrorIgnoreLevel = 2001; // Ignore Warnings

  //std::string run_list_json = "DBASE/run_list.json";

  THcParmList* hc_parms = new THcParmList();
  hc_parms->AddString("g_ctp_database_filename", file_name.c_str());

  // ----------------------------------------------------------
  //
  auto rng = runlist | view::transform([&](int irun) -> table_entry_t {
               hc_parms->Load(kinematics_filename.c_str(), irun);
               //return ({hcana::json::FindVarValueOr(hc_parms, "htheta_lab", 0.0),
               //         hcana::json::FindVarValueOr(hc_parms, "hpcentral", 0.0),
               //         hcana::json::FindVarValueOr(hc_parms, "ptheta_lab", 0.0),
               //         hcana::json::FindVarValueOr(hc_parms, "ppcentral", 0.0)});
               table_entry_t res;
               res["hms"]["angle"]     = std::abs(hcana::json::FindVarValueOr(hc_parms, "htheta_lab", 0.0));
               res["hms"]["momentum"]  = hcana::json::FindVarValueOr(hc_parms, "hpcentral", 0.0);
               res["shms"]["angle"]    = hcana::json::FindVarValueOr(hc_parms, "ptheta_lab", 0.0);
               res["shms"]["momentum"] = hcana::json::FindVarValueOr(hc_parms, "ppcentral", 0.0);
               return res;
             });

  return view::zip(runlist, rng);
}

/** MAIN
 */
int main(int argc, char* argv[]) {

  cli_settings opts;

  bool is_piped = false;
  if (!isatty(fileno(stdin))) {
    is_piped = true;
  }

  auto cli          = opts.GetCLI();
  auto clipp_format = opts.GetFormat();

  assert( cli.flags_are_prefix_free() );
  auto result = parse(argc, argv, cli);

  auto doc_filter = param_filter{}.prefix("--");

  if (opts.use_help == 1) {
    cout << "\033[1mspectrometer_settings\033[0m\n";
    cout << "Usage:\n" << usage_lines(cli, "hcspec", clipp_format)
       << "\nOptions:\n" << documentation(cli, clipp_format,doc_filter) << '\n';
    std::exit(0);
  } else if (opts.use_help == 2) {
    cout << make_man_page(cli, argv[0])
                .prepend_section("DESCRIPTION" , "              things don't have to be difficult.")
                .prepend_section("NAME"        , "              \033[1mhcspec : hallc spectrometer_settings\033[0m")
                .append_section("LICENSE"      , "              GPL3")
                .append_section("EXAMPLES"      , 
                                "\n"
                                "    hcspec -S 6000 -N 1000 -u print \n"
                                "\n"
                                "    hcspec -S 6000 -N 1000 -u  print | hcspec  filter hms angle 14 1\n"
                                "\n"
                                " Dump runs to JSON then use json file (which is much faster)\n"
                                "    hcspec -S 0 -N 7000  -J print > all.json\n"
                                "    hcspec -a -j all.json print  | hcspec filter hms angle 14 0.25 -u\n"
                                "\n"
                                "    hcspec -a -j all.json print  | hcspec filter hms angle 14 0.25 -u | jq \n"
                                "\n"
                                );
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

  // -----------------------------------------------------
  // If result is piped then the input should only be json 
  //
  if(is_piped) {
    // we assume only json is piped as input
    opts.use_json_input = true;
    // All runs of the piped data are used
    opts.use_all = true;
  }

  // -----------------------------------------------------
  // If the output is being piped then only ouput json
  bool is_piped_out = false;
  if (!isatty(fileno(stdout))) {
    is_piped_out = true;
    opts.output_format = "json";
    opts.mode = RunMode::print;
  }

  // If neither are given, use both
  if( (!opts.use_shms) && (!opts.use_hms)) {
    opts.use_shms = true;
    opts.use_hms = true;
  } else if(opts.use_shms) {
    opts.daq_spec_type = "SHMS";
  } else if(opts.use_hms) {
    opts.daq_spec_type = "HMS";
  }

  // ---------------------------------
  // define the run list to use
  std::cout << opts.start_run << " - ";
  std::cout << opts.end_run << "\n";
  if ((opts.end_run > 0) && (opts.end_run > opts.start_run)) {
    opts.N_runs = opts.end_run - opts.start_run;
  }
  auto& run_list = opts.run_list;
  if (run_list.size() == 0) {
    if (opts.N_runs <= 0) {
      opts.N_runs = 100;
      if(opts.use_json_input ) { 
        opts.use_all = true;
      }
    }
    run_list = std::vector<int>(opts.N_runs);
    std::iota(run_list.begin(), run_list.end(), opts.start_run);
  }

  // ---------------------------------
  //
  std::string dbfile = opts.replay_dir;
  if(opts.use_json_input){
    dbfile = opts.json_data_file;
  }

  table_range_t run_shms_angles;
  if(opts.use_json_input) {
    run_shms_angles = build_range_with_json( dbfile, run_list, opts.use_all);
  } else {
    run_shms_angles = build_range_with_DBASE( dbfile, run_list,opts.daq_spec_type);
  }

  auto output_settings = run_shms_angles;

  if(opts.filter_zero) {
    if (opts.use_hms) {
      output_settings = output_settings | view::remove_if([](auto t){
        return std::get<1>(t)["hms"]["angle"] == 0.0;
      }) | view::remove_if([](auto t){
        return std::get<1>(t)["hms"]["momentum"] == 0.0;
      }) | to_<std::vector>();
    }

    if (opts.use_shms) {
      output_settings = output_settings| view::remove_if([](auto t){
        return std::get<1>(t)["shms"]["angle"] == 0.0;
      }) | view::remove_if([](auto t){
        return std::get<1>(t)["shms"]["momentum"] == 0.0;
      }) | to_<std::vector>();
    }
  }

  for (auto& [spm, mo, val, del] :
       view::zip(opts.fspecs, opts.fmodes, opts.filter_values, opts.filter_deltas) |
           to_<std::vector>()) {
    std::string spec = cli_settings::GetSpecString(spm);
    std::string mode = cli_settings::GetFilterMode(mo);
    //std::cout << " ----- \n";
    //std::cout << spec << "\n";
    //std::cout << mode << "\n";
    //std::cout << val << "\n";
    //std::cout << del << "\n";
    output_settings = output_settings | view::filter([&](auto t) {
                        if (spec == "none") {
                          return false;
                        }
                        if (spec == "both") {
                          return (std::abs(val - std::get<1>(t)["hms"][mode]) < del) ||
                                 (std::abs(val - std::get<1>(t)["shms"][mode]) < del);
                        }
                        return (std::abs(val - std::get<1>(t)[spec][mode]) < del);
                      }) |
                      to_<std::vector>();
  }
  if(opts.use_unique){
    output_settings =
        output_settings | view::adjacent_remove_if([&](auto t1, auto t2) {
          return ((std::get<1>(t1)["hms"]["angle"] == std::get<1>(t2)["hms"]["angle"]) &&
                  (std::get<1>(t1)["hms"]["momentum"] == std::get<1>(t2)["hms"]["momentum"]) &&
                  (std::get<1>(t1)["shms"]["angle"] == std::get<1>(t2)["shms"]["angle"]) &&
                  (std::get<1>(t1)["shms"]["momentum"] == std::get<1>(t2)["shms"]["momentum"]));
        }) |
        to_<std::vector>();
  } 
  if(opts.use_first_unique){
    output_settings =
        output_settings | view::reverse | view::adjacent_remove_if([&](auto t1, auto t2) {
          return ((std::get<1>(t1)["hms"]["angle"] == std::get<1>(t2)["hms"]["angle"]) &&
                  (std::get<1>(t1)["hms"]["momentum"] == std::get<1>(t2)["hms"]["momentum"]) &&
                  (std::get<1>(t1)["shms"]["angle"] == std::get<1>(t2)["shms"]["angle"]) &&
                  (std::get<1>(t1)["shms"]["momentum"] == std::get<1>(t2)["shms"]["momentum"]));
        }) | view::reverse|
        to_<std::vector>();
  }

  nlohmann::json j_output;
  for (auto en : output_settings) {
    int arun_num = std::get<0>(en);
    if (opts.use_hms) {
      j_output[std::to_string(arun_num)]["hpcentral"]  = std::get<1>(en)["hms"]["momentum"];
      j_output[std::to_string(arun_num)]["htheta_lab"] = std::get<1>(en)["hms"]["angle"];
    }
    if (opts.use_shms) {
      j_output[std::to_string(arun_num)]["ppcentral"]  = std::get<1>(en)["shms"]["momentum"];
      j_output[std::to_string(arun_num)]["ptheta_lab"] = std::get<1>(en)["shms"]["angle"];
    }
  }

  if( is_piped_out ) {
    opts.output_format = "json";
    opts.mode = RunMode::print;
  }


  switch (opts.mode) {

  case RunMode::print:

    if (opts.output_format == "json") {
      if(opts.json_dump_format>=0) {
        std::cout << j_output.dump(opts.json_dump_format) << std::endl;
      } else {
        std::cout << j_output << std::endl;
      }
    } else {

      for (auto en : output_settings) {
        int arun_num = std::get<0>(en);

        fmt::print("{:<9} ", arun_num);
        //std::cout << arun_num << " :";
        if (opts.use_hms) {
          fmt::print("HMS: {:>7.3f} GeV/c at {:<7.3f} deg    ", std::get<1>(en)["hms"]["momentum"],
                     std::get<1>(en)["hms"]["angle"]);
          //std::cout << " HMS : ";
          //std::cout << std::get<1>(en)["hms"]["momentum"] << " GeV/c at ";
          //std::cout << std::get<1>(en)["hms"]["angle"] << " deg  ";
        }
        if (opts.use_shms) {
          fmt::print("SHMS: {:>7.3f} GeV/c at {:<7.3f} deg  ", std::get<1>(en)["shms"]["momentum"],
                     std::get<1>(en)["shms"]["angle"]);
          //std::cout << "SHMS : ";
          //std::cout << std::get<1>(en)["shms"]["momentum"] << " GeV/c at ";
          //std::cout << std::get<1>(en)["shms"]["angle"] << " deg";
        }
        std::cout << "\n";
      }
    }
    break;

  case RunMode::build:

    ofstream out_file(opts.table_name);
    out_file << std::setw(2) << j_output << std::endl;
    break;

  }

}
