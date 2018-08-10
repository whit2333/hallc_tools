#ifndef spectrometer_settings_hh
#define spectrometer_settings_hh

#include "clipp.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <range/v3/all.hpp>
#include <map>
#include <utility>
#include <unistd.h>
#include <stdio.h>


//auto build_range_with(bool use_json_input, std::string dbfile, std::vector<int> runlist );
//auto build_range_with_json(std::string dbfile, std::vector<int> runlist);

enum class RunMode { standard, build, find, help, print };
enum class FilterMode { angle, momentum  };
enum class FilterSpec { none, both, hms, shms  };

//using table_range_t = std::vector<std::pair<int, std::array<double, 4>>>;
using table_entry_t = std::map<string, std::map<std::string, double>>;
using table_range_t = std::vector<std::pair<int,table_entry_t>>;

table_range_t build_range_with_json(std::string dbfile, std::vector<int> runlist , bool all = false) ;
table_range_t build_range_with_DBASE(std::string dbfile, std::vector<int> runlist) ;

//template<typename T>
//class BuildRangeFrom  {
//  using json_t   = decltype( build_range_with_json(std::declval<std::string>(), std::declval<std::vector<int>>()));
//  using dbase_t  = decltype( build_range_with_DBASE(std::declval<std::string>(), std::declval<std::vector<int>>()));
//};

//template<typename T1,typename T2>
//auto build_range_with(bool use_json_input, std::string dbfile, std::vector<int> runlist ){
//  return use_json_input ? build_range_with_json<T1>(dbfile,runlist) : build_range_with_DBASE<T2>(dbfile,runlist);
//}

struct cli_settings {

  using string = std::string;

  static std::string GetFilterMode(FilterMode s)  {
    const std::map<FilterMode, std::string> spec_to_string = {{FilterMode::angle, "angle"},
                                                              {FilterMode::momentum, "momentum"}};
    auto search = spec_to_string.find(s);
    if (search == spec_to_string.end()) {
      return "angle";
    }
    return search->second;
  }

  static std::string GetSpecString(FilterSpec s)  {
    const std::map<FilterSpec,std::string> spec_to_string= {
      {FilterSpec::none , "none"}  ,
      {FilterSpec::both , "both"}  ,
      {FilterSpec::shms , "shms"}  ,
      {FilterSpec::hms  , "hms" }};
    auto search = spec_to_string.find(s);
    if (search == spec_to_string.end()) {
      return "both";
    }
    return search->second;
  }
  static FilterSpec GetSpec(const std::string& s) {
    const std::map<std::string,FilterSpec> str_to_spec= {
      {"none",FilterSpec::none},
      {"both",FilterSpec::both},
      {"shms",FilterSpec::shms},
      {"hms",FilterSpec::hms}};

    auto search = str_to_spec.find(s);
    if (search == str_to_spec.end()) {
      return FilterSpec::both;
    }
    return search->second;
  }

  RunMode             mode           = RunMode::print;
  bool                use_shms       = false;
  bool                use_hms        = false;
  int                 use_help       = false;
  bool                use_json_input = false;
  bool                filter_zero    = true;
  string              json_data_file = "settings.json";
  string              infile         = "DBASE";
  string              fmt            = "json";
  bool                use_all        = false;
  bool                use_unique     = false;
  int                 start_run      = 3900;
  int                 N_runs         = 100;
  int                 RunNumber      = 0;
  string              replay_dir     = ".";
  std::vector<int>    run_list       = {};
  int                 json_dump_format = -1;
  string              output_format  = "table";
  string              table_name     = "settings.json";
  bool                has_filter     = false;
  std::vector<FilterSpec> fspecs     = {};
  std::vector<FilterMode> fmodes     = {};
  std::vector<double> filter_values  = {};
  std::vector<double> filter_deltas  = {};

  auto GetCLI() {
    using namespace clipp;
    using std::cout;
    using std::string;
    using namespace ranges;

    auto first_args =
        ("Data source options" %
             ((option("-d", "--replay-dir") & value("dir", replay_dir)) %
                  "Set path of replay directory which should the contain directory DBASE. This is "
                  "the default data source with dir=." |
              (option("-j", "--json-data").set(use_json_input, true) &
               value("data", json_data_file)) %
                  "use json data as input instead of DBASE"),
         "Basic filtering options " %
             joinable(option("-u", "--unique").set(use_unique, true) % "filter unique (adjacent) entries",
              option("-a", "--all").set(use_all, true) %
                  "use all runs in supplied json file (only works with json input)",
              option("-z", "--show-zeros").set(filter_zero, false) %
                  "Turns of suppression of zero or null values"),
         "Data output options" %
             (option("-P", "--shms").set(use_shms, true) %
                  "Set to only the SHMS spectrometer for output. [default: both are used]",
              option("-H", "--hms").set(use_hms, true) %
                  "Set to only the HMS spectrometer for output. [default: both are used]",
              option("-J","--json-format")([&] { output_format = "json"; }) &
                  opt_integer("json_fmt", json_dump_format) %
                      "Set the printing format to json with an optional format spacing argument "
                      "[default:-1]"),
         option("-h", "--help").set(use_help, 1) % "print help",
         option("--man").set(use_help, 2) % "print man page");
    //auto last_args = ;
    //(option("-t", "--type") & value("type", output_format)) % "set the build type");

    auto standard_args = (repeatable(option("-r", "--runs") & integers("runs", run_list)) %
                              "Set indivindual runs to be used (instead of range)" |
                          ((option("-N", "--number-of-runs") & integer("N_runs", N_runs)) %
                               "number of runs to process",
                           (option("-S", "--start") & integer("start_run", start_run)) %
                               "Set the starting run for the output run sequence"));

    //auto build_cmd = (command("build").set(mode, RunMode::build) % "build mode",
    //                  value("table_name", table_name) % "Output file");
    //auto momentum_filter_type = 
    //    (command("momentum")([this] { this->fmodes.push_back(FilterMode::momentum); }) %
    //         "momentum filter type" &
    //     number("GeV/c", filter_values) % "momentum in degrees" &
    //     number("deltaP", filter_deltas) % "delta in degrees, used to search with the range of "
    //                                       "values [angle-delta,angle+delta]") % "Momentunm Filter types" ;
    //auto angle_filter_type = command("angle")([&] { fmodes.push_back(FilterMode::angle); }) &
    //                         number("deg", filter_values) % "angle in degrees" &
    //                         number("delta", filter_deltas) %
    //                             "delta in degrees, used to search with the range of "
    //                             "values [angle-delta,angle+delta]";
    //auto filter_opt = option("--filter").set(has_filter, true) % "Add filter" &
    //                  value("spectrometer")([this](const string& v) {
    //                    this->fspecs.push_back(GetSpec(v));
    //                  }) % "spectrometer to filter. Can be one of the following: hms,shms, both";
    //auto print_cmd = "print mode" % (command("print").set(mode, RunMode::print),
    //                                 option("--json-format")([&] { output_format = "json"; }) %
    //                                     "set the printing format [default:table]");
    auto filter_spec =
        (command("filter").set(mode, RunMode::print),
         value("spectrometer")([&](const string& v) { fspecs.push_back(GetSpec(v)); }));
    auto filter_angle = (command("angle")([this] { this->fmodes.push_back(FilterMode::angle); }),
                         (number("deg", filter_values) & number("delta", filter_deltas)));
    auto filter_momentum =
        (command("momentum")([this] { this->fmodes.push_back(FilterMode::momentum); }),
         (number("GeV/c", filter_values) & number("deltaP", filter_deltas)));
    auto print_cmd = (command("print").set(mode, RunMode::print));

    //return (     |
    //            (print_cmd, filter_opt & momentum_filter_type ) |
    //            (build_cmd, filter_opt & angle_filter_type ),
    //        first_args, standard_args, last_args);
    return (first_args, standard_args, (filter_spec, (filter_angle | filter_momentum)) | print_cmd);
  }

  auto GetFormat() {
    using namespace clipp;
    using std::cout;
    using std::string;

    // all formatting options (with their default values)
    auto clipp_format =
        doc_formatting{}.start_column(4) // column where usage lines and documentation starts
        //.doc_column(20)                            //parameter docstring start col
        //.indent_size(4)                            //indent of documentation lines for children of
        //a documented group .line_spacing(0)                           //number of empty lines
        //after single documentation lines .paragraph_spacing(1)                      //number of
        //empty lines before and after paragraphs .flag_separator(", ") //between flags of the same
        //parameter .param_separator(" ")                      //between parameters
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
        //.max_alternative_flags_in_doc(2)           //max. # of flags per parameter in detailed
        //documentation
        .split_alternatives(true)                  //split usage into several lines
        //for large alternatives
        .alternatives_min_split_size(2)            //min. # of parameters
        //for separate usage line .merge_alternative_flags_with_common_prefix(false)  //-ab(cdxy|xy)
        //instead of -abcdxy|-abxy
        ////.merge_joinable_flags_with_common_prefix(true)     //-abc instead of -a -b -c
        ;
    return clipp_format;
  }
};

#endif
