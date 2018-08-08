#include "clipp.h"
#include <iostream>
#include <cmath>

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"
#include "TCanvas.h"
#include "nlohmann/json.hpp"

#include "THcParmList.h"

R__LOAD_LIBRARY(libHallA.so)
R__LOAD_LIBRARY(libdc.so)
R__LOAD_LIBRARY(libHallC.so)

#include <range/v3/all.hpp>

#include "TGraph.h"

using namespace clipp;
using std::cout;
using std::string;

//all formatting options (with their default values)
auto clipp_format = doc_formatting{}
    .start_column(8)                           //column where usage lines and documentation starts
    .doc_column(20)                            //parameter docstring start col
    .indent_size(4)                            //indent of documentation lines for children of a documented group
    .line_spacing(0)                           //number of empty lines after single documentation lines
    .paragraph_spacing(1)                      //number of empty lines before and after paragraphs
    .flag_separator(", ")                      //between flags of the same parameter
    .param_separator(" ")                      //between parameters 
    .group_separator(" ")                      //between groups (in usage)
    .alternative_param_separator("|")          //between alternative flags 
    .alternative_group_separator(" | ")        //between alternative groups 
    .surround_group("(", ")")                  //surround groups with these 
    .surround_alternatives("(", ")")           //surround group of alternatives with these
    .surround_alternative_flags("", "")        //surround alternative flags with these
    .surround_joinable("(", ")")               //surround group of joinable flags with these
    .surround_optional("[", "]")               //surround optional parameters with these
    .surround_repeat("", "...")                //surround repeatable parameters with these
    //.surround_value("<", ">")                  //surround values with these
    .empty_label("")                           //used if parameter has no flags and no label
    //.max_alternative_flags_in_usage(1)         //max. # of flags per parameter in usage
    //.max_alternative_flags_in_doc(2)           //max. # of flags per parameter in detailed documentation
    .split_alternatives(true)                  //split usage into several lines for large alternatives
    .alternatives_min_split_size(3)            //min. # of parameters for separate usage line
    .merge_alternative_flags_with_common_prefix(false)  //-ab(cdxy|xy) instead of -abcdxy|-abxy
    //.merge_joinable_flags_with_common_prefix(true)     //-abc instead of -a -b -c
    ;



int main(int argc, char* argv[]) {
  bool   use_shms   = false;
  bool   use_hms    = false;
  bool   utf16      = false;
  string infile     = "DBASE";
  string fmt        = "json";
  int    start_run  = 3890;
  int    N_runs     = 300;
  int    RunNumber  = 3890;

  auto cli = (value("input file", infile),
              option("-r", "--run") & integer("run",RunNumber)        % "Set the run number",
              option("-p","-s","--shms").set(use_shms,true) % "use SHMS settings",
              option("-h", "--hms").set(use_hms,true)       % "use HMS settings",
              option("-o") & value("output format", fmt),
              option("-N") & number("N_runs", N_runs)       % "number of runs to process",
              option("-S","--start") & value("start_run", start_run),
              option("-utf16").set(utf16).doc("use UTF-16 encoding"));

  if (!parse(argc, argv, cli))
    cout << make_man_page(cli, argv[0]);

  std::string hallc_replay_dir = "./";
  //std::string hallc_replay_dir = "/home/whit/projects/hall_AC_projects/new_hallc_replay/EXPERIMENT_replay/hallc_replay/";
  std::string run_list_json  = "DBASE/run_list.json";

  THcParmList* hc_parms = new THcParmList();
  hc_parms->AddString("g_ctp_database_filename", (hallc_replay_dir+"DBASE/COIN/standard.database").c_str() );
  hc_parms->Define("gen_run_number", "Run Number", RunNumber);
  hc_parms->Load(hc_parms->GetString("g_ctp_database_filename"), RunNumber);
  //hc_parms->Load(hc_parms->GetString("g_ctp_parm_filename"));
  hc_parms->Load(hc_parms->GetString("g_ctp_kinematics_filename"), RunNumber);

  std::cout << "g_ctp_database_filename   : " << hc_parms->GetString("g_ctp_database_filename") << "\n";
  std::cout << "g_ctp_parm_filename       : " << hc_parms->GetString("g_ctp_parm_filename") << "\n";
  std::cout << "g_ctp_kinematics_filename : " << hc_parms->GetString("g_ctp_kinematics_filename") << "\n";

  auto j = nlohmann::json::parse(hc_parms->PrintJSON(RunNumber));
  auto htheta_lab = hc_parms->Find("htheta_lab");
  htheta_lab->Print();
  auto ptheta_lab = hc_parms->Find("ptheta_lab");
  ptheta_lab->Print();
  auto hpcentral = hc_parms->Find("hpcentral");
  hpcentral->Print();
  auto ppcentral = hc_parms->Find("ppcentral");
  ppcentral->Print();
  //std::cout << j.dump()  << "\n";


  std::vector<int> runs(N_runs);
  std::iota(runs.begin(), runs.end(), start_run);

  using namespace ranges;
  auto rng = runs | view::transform([&](int irun){
    hc_parms->Define("gen_run_number", "Run Number", irun);
    hc_parms->Load(hc_parms->GetString("g_ctp_database_filename"), irun);
    hc_parms->Load(hc_parms->GetString("g_ctp_kinematics_filename"), irun);
    return std::make_tuple(std::abs(hcana::json::FindVarValueOr(hc_parms,"htheta_lab",0.0)),
                           hcana::json::FindVarValueOr(hc_parms,"ptheta_lab",0.0),
                           hcana::json::FindVarValueOr(hc_parms,"hpcentral" ,0.0),
                           hcana::json::FindVarValueOr(hc_parms,"ppcentral" ,0.0)
                          );
  });
  auto run_shms_angles = view::zip(runs,rng) 
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
    std::cout << std::get<0>(en) <<  " : \n"; 
    std::cout << " HMS : ";
    std::cout << std::get<2>(std::get<1>(en)) << " GeV/c at ";
    std::cout << std::get<0>(std::get<1>(en)) << " deg, ";
    std::cout << "SHMS : ";
    std::cout << std::get<3>(std::get<1>(en)) << " GeV/c at ";
    std::cout << std::get<1>(std::get<1>(en)) << " deg.\n";
  }


}
