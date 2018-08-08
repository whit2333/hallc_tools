#include "clipp.h"
#include <iostream>
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
  bool   rec         = false;
  bool   utf16       = false;
  string infile      = "";
  string fmt         = "csv";
  int    run_number  = 0;
  int    port        = 8081;
  string monitor_dir = "monitoring";

  auto cli = (value("input file", infile),
              option("-d", "--dir") & value("monitor_dir",monitor_dir) % "Set the monitoring directory. Default: \"monitoring\"",
              option("-r", "--run") & integer("run",run_number)        % "Set the run number",
              option("-f") & value("format", fmt)                      % "Not used. Only json is currently supported",
              option("-p", "--port") & integer("port",port)            % "Set port to bind http server");

  if (!parse(argc, argv, cli))
    cout << make_man_page(cli, argv[0]);


cout << "Usage:\n" << usage_lines(cli, "progname", clipp_format)
     << "\nOptions:\n" << documentation(cli, clipp_format) << '\n';

//or generate entire man page in one go
cout << make_man_page(cli, "progname", clipp_format)
        .prepend_section("DESCRIPTION", "This program lets you format text.")
        .append_section("LICENSE", "GPLv3");
}

