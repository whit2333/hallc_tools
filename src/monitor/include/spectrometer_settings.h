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
table_range_t build_range_with_DBASE(std::string dbfile, std::vector<int> runlist, std::string spec_daq ="COIN") ;

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
  bool                use_first_unique = false;
  int                 start_run      = 3900;
  int                 end_run        = 0;
  int                 N_runs         = 0;
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
  string              daq_spec_type  = "COIN";

  auto GetCLI() {
    using namespace clipp;
    using std::cout;
    using std::string;
    using namespace ranges;

    auto standard_args = (repeatable(option("-r", "--runs") & integers("runs", run_list)) %
                              "Set indivindual runs to be used instead of range." |
                          ((option("-N", "--number-of-runs") & integer("N_runs", N_runs)) %
                               "Number of runs in the sequence starting at <start_run>",
                           (option("-S", "--start") & integer("start_run", start_run)) %
                               "Set the starting run for the output run sequence") |
                          ((option("-R", "--run-range") & integer("start_run", start_run) &
                           integer("end_run", end_run)) % "Set the range of runs to use"));
    auto first_args =
        ("Data source options" %
             ((option("-d", "--replay-dir") & value("dir", replay_dir)) %
                  "Set path of replay directory which should the contain directory DBASE. This is "
                  "the default data source with dir=." |
              (option("-j", "--json-data").set(use_json_input, true) &
               value("data", json_data_file)) %
                  "use json data as input instead of DBASE"),
         "Basic filtering options " %
             joinable( (option("-u", "--unique").set(use_unique, true) %
                          "filter unique (adjacent) entries returning the latest run" |
                          option("-U", "--first-unique").set(use_first_unique, true) %
                          "filter unique (adjacent) entries returning the first run in the same group of settings"),
                      option("-a", "--all").set(use_all, true) %
                          "use all runs in supplied json file (only works with json input)",
                      option("-z", "--show-zeros").set(filter_zero, false) %
                          "Turns of suppression of zero or null values"),
         "Run range options" % standard_args,
         "Data output options" %
             ("Single spectrometer output [default: both]. H->HMS, P->SHMS (note in 6 GeV era "
              "S->SOS). Selects wither to use DBASE/{COIN,SHMS<HMS} when using DBASE as input." %
                  joinable(option("-P", "--shms").set(use_shms, true) % "SHMS" ,
                           option("-H", "--hms").set(use_hms, true) % "HMS" ),
              (option("-J", "--json-format")([&] { output_format = "json"; }) &
               opt_integer("json_style", json_dump_format)) %
                  "Use json output format. Optionally setting the printing style number "
                  "[default:-1] which defines json indentation spacing."),
         option("-h", "--help").set(use_help, 1) % "print help",
         option("-m", "--man").set(use_help, 2) % "print man page");
    //auto last_args = ;
    //(option("-t", "--type") & value("type", output_format)) % "set the build type");

    auto filter_spec =
        "Set to: hms,shms, or both. Specifies spectrometer to use with filter." % (command("filter").set(mode, RunMode::print),
         value("spec")([&](const string& v) { fspecs.push_back(GetSpec(v)); }));
    auto filter_angle = "Constructs a filter for angles in the range [deg-delta,deg+delta] degrees." %
                        (command("angle")([this] { this->fmodes.push_back(FilterMode::angle); }),
                         (number("deg", filter_values) & number("delta", filter_deltas)));
    auto filter_momentum =
        "Constructs a filter for momenta in the range [P0-deltaP,P0+deltaP] GeV/c." %
        (command("momentum")([this] { this->fmodes.push_back(FilterMode::momentum); }),
         (number("P0", filter_values) & number("deltaP", filter_deltas)));
    auto print_cmd = (command("print").set(mode, RunMode::print));

    //return (     |
    //            (print_cmd, filter_opt & momentum_filter_type ) |
    //            (build_cmd, filter_opt & angle_filter_type ),
    //        first_args, standard_args, last_args);
    return (first_args, "Filters" % (filter_spec, "filter by " % (filter_angle | filter_momentum)) |
                            "Print spectrometer" % print_cmd);
  }

  auto GetFormat() {
    using namespace clipp;
    using std::cout;
    using std::string;

    // all formatting options (with their default values)
    auto clipp_format =
        doc_formatting{}.start_column(4) // column where usage lines and documentation starts
        .doc_column(28)                            //parameter docstring start col
        .indent_size(3)                            //indent of documentation lines for children of
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
        //for separate usage line 
        //.merge_alternative_flags_with_common_prefix(false)  //-ab(cdxy|xy)
        //instead of -abcdxy|-abxy
        ////.merge_joinable_flags_with_common_prefix(true)     //-abc instead of -a -b -c
        ;
    return clipp_format;
  }
};

#endif
