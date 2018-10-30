#include "runplan/RunTable.h"
#include <fstream>
#include <sstream>

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

#include "nlohmann/json.hpp"

#include "TBufferJSON.h"

void RunPlanTableEntry::PrintWikiHeader2(std::ostream& s) {
  //s << R"foo(<span style="tr:nth-child(even){background-color: #f2f2f2}">)foo";
  s <<   "{|class=\"wikitable\" style=\"width: 85\%\"\n";
  fmt::print(s,
             "! colspan=\"3\" style=\"border: 1px solid black; padding: 5px; background: #ffdead;\" | Setting|| "
             "colspan=\"3\" style=\"border: 1px solid black; padding: 5px; background: #99ccff;\" | Kinematics || "
             "colspan=\"5\" style=\"border: 1px solid black; padding: 5px; background: #ffdead;\" | Spectrometer settings || "
             "colspan=\"2\"  style=\"border: 1px solid black; padding: 5px; background: #99ccff;\" | Charge Goals|| "
             "{:^9}|| {:^6} ||"
             "colspan=\"2\" style=\"border: 1px solid black; padding: 5px; background: #99ccff;\" | Rates \n",
             "est.", " ");
  fmt::print(s,
             "|- \n"
             "! {:^6} || {:^6} || {:^6} || {:^5} || {:^5} || {:^5} || "
             "{:^6} || {:^5} || {:^7} || {:^6} || {:^3} || "
             "{} || {} || {:^6} || {:^6} || {:^6} || {:^6}\n",
             "number", "target", "Ibeam", "Q2", "x", "z", 
             "th_HMS", "P HMS","th_SHMS",  "P SHMS", "pol", 
             "desired", "actual", "time", "# runs","coin","HMS");
}
void RunPlanTableEntry::PrintWikiFooter2(std::ostream& s) {
  s << "|- \n"
   << "|}\n";
}

void RunPlanTableEntry::PrintWiki2(std::ostream& s) const {
  std::string target = "LD2";
  double      xs     = rates.LD2_XS;
  if ((Z_target==1) && (A_target == 1)) {
    target    = "LH2";
    double xs = rates.LH2_XS;
  }
  if ((Z_target==13) && (A_target == 27)) {
    target    = "DUM";
    double xs = rates.window_XS;
  }
  std::string row = "|- style=\"text-align:center; background: #ffffff;\"\n";
  if( _number%2 == 0 ) {
    row = "|- style=\"text-align:center; background: #efeff5;\"\n";
  }
  s << row;
  fmt::print(s,
             "| {:>3d}-{:<2d} || {:^6} || {:>6.3f} || "
             "{:>4.3f} || {:>4.3f} || {:>4.3f} || "
             "{:>6.2f} || {:>6.2f} || {:>6.3f} || {:>6.3f} || {:>+3d} || "
             "{:^7.1f} || {:^6} || {:>6.3f} || {:^6} || {:^6} || {:^6}\n",
             _group, _number, target, Ibeam,  
             kinematic.Q2, kinematic.x, kinematic.z ,
             kinematic.th_e, kinematic.th_q, kinematic.Ee, polarity*kinematic.Ppi, polarity, 
             time*Ibeam*60.*60./1000.0, "xx", time, "NN", "x", "x");
}

void RunPlanTableEntry::PrintWikiHeader(std::ostream& s) {
  fmt::print(s,
             "{{|class=\"wikitable\"\n"
             "!{:^6}||{:^6}|| {:^5}|| {:^5}|| {:^5}|| {:^9}|| {:^9}|| {:^8}|| {:^9}|| {:^8}|| "
             "{:^6}|| {:^9}|| {:^6}|| {:^6}||{:^6}\n|- \n",
             "setting", "target", "Q2", "x", "z", "th_e(hms)", "th_q(shms)", "Pe(hms)", "Ph(shms)",
             "pol(shms)", "Ibeam", "sigma", "rate", "time", "charge");
}
void RunPlanTableEntry::PrintWikiFooter(std::ostream& s) {
  fmt::print(s,
             "|- \n"
             "!{:^6}||{:^6}|| {:^5}|| {:^5}|| {:^5}|| {:^9}|| {:^9}|| {:^8}|| {:^9}|| {:^8}|| "
             "{:^6}|| {:^9}|| {:^6}|| {:^6} ||{:^6}\n|}}\n",
             "setting", "target", "Q2", "x", "z", "th_e(hms)", "th_q(shms)", "Pe(hms)", "Ph(shms)",
             "pol(shms)", "Ibeam", "sigma", "rate", "time", "count");
  //s<< "</span>\n";
}

void RunPlanTableEntry::PrintWiki(std::ostream& s) const {
  std::string target = "LD2";
  double      xs     = rates.LD2_XS;
  if ((Z_target==1) && (A_target == 1)) {
    target    = "LH2";
    double xs = rates.LH2_XS;
  }
  if ((Z_target==13) && (A_target == 27)) {
    target    = "DUM";
    double xs = rates.window_XS;
  }
  fmt::print(s,
             "|{:>3d}|| {:^6}|| {:>5.3f}||{:>5.3f}|| {:>5.3f}|| {:>6.2f}|| {:>6.2f}|| {:>6.3f}|| {:>6.3f}|| "
             "{:>+3d}|| {:>6.3f}|| {:>6.3e}|| {:>6.3f}|| {:>6.3f}|| {:>6.0f}\n|- \n",
             _group,target, kinematic.Q2, kinematic.x, kinematic.z, kinematic.th_e, kinematic.th_q, kinematic.Ee,
             kinematic.Ppi, polarity, Ibeam, xs, rates.total_rate, time, counts);
}

void RunPlanTableEntry::Print(std::ostream& s) const {
  std::string target = "LD2";
  double      xs     = rates.LD2_XS;
  if ((Z_target==1) && (A_target == 1)) {
    target    = "LH2";
    double xs = rates.LH2_XS;
  }
  if ((Z_target==13) && (A_target == 27)) {
    target    = "DUM";
    double xs = rates.window_XS;
  }
  fmt::print(s,
             "{:>3d}, {:^6}, {:>5.3f}, {:>5.3f}, {:>5.3f}, {:>6.2f}, {:>6.2f}, {:>6.3f}, {:>6.3f}, "
             "{:>+3d}, {:>6.3f}, {:>6.3e}, {:>6.3f}, {:>6.3f}, {:>6.0f}\n",
             _group, target, kinematic.Q2, kinematic.x, kinematic.z, kinematic.th_e, kinematic.th_q,
             kinematic.Ee, kinematic.Ppi, polarity, Ibeam, xs, rates.total_rate, time, counts);
}

void RunPlanTableEntry::PrintHeader(std::ostream& s ) {
  fmt::print(s,
             "{:^6},{:^6}, {:^5}, {:^5}, {:^5}, {:^6}, {:^6}, {:^6}, {:^6}, {:^3}, {:^6}, {:^9}, "
             "{:^6}, {:^6}\n",
             "group", "target", "Q2", "x", "z", "th_e", "th_q", "Ee", "Ppi", "pol", "I", "sigma",
             "rate", "time");
}

void RunPlanTableEntry::SetTarget(int Z, int A) {
  // All rates are calculated with 50 uA beam.
  double scale = 50.0 / Ibeam;
  Z_target     = Z;
  A_target     = A;
  if (Z == 1) {
    if (A == 1) {
      rates.total_rate = rates.total_LH2_rate;
      time             = scale * (counts / rates.total_rate) / (60.0 * 60.0);
    } else if (A == 2) {
      rates.total_rate = rates.total_LD2_rate;
      time             = scale * (counts / rates.total_rate) / (60.0 * 60.0);
    }
  }
  if (Z == 13) {
    rates.total_rate = rates.window_rate;
    time             = scale * (counts / rates.total_rate) / (60.0 * 60.0);
  }
}

void RunTable::Number() {
  int n          = 0;
  int sn         = 0;
  int prev_group = 1;
  for (auto& row : _rows) {
    if (prev_group != row._group) {
      n          = 0;
      prev_group = row._group;
    }
    n++;
    sn++;
    row._number = n;
    row._set    = sn;
  }
}

void RunTable::Print(std::ostream& os) const {
  RunPlanTableEntry::PrintHeader(os);
  for(const auto& row : _rows) {
    row.Print(os);
  }
  std::cout << " total time : " << _total_time << " hrs (" << _total_time/24.0 << " days)\n";
}

void RunTable::PrintWiki(std::ostream& os) const {
  RunPlanTableEntry::PrintWikiHeader(os);
  for(const auto& row : _rows) {
    row.PrintWiki(os);
  }
  RunPlanTableEntry::PrintWikiFooter(os);
  os << " total time : " << _total_time << " hrs (" << _total_time/24.0 << " days)\n";
}

void RunTable::PrintWikiByGroup(std::ostream& os) const {
  double group_time = 0;
  int group = 0;
  RunPlanTableEntry::PrintWikiHeader2(os);
  for(const auto& row : _rows) {
    if(group != row._group){
      RunPlanTableEntry::PrintWikiFooter2(os);
      // print group summary
      os << "Kinematic group " << group << " total time : " << group_time << " hrs ("
         << group_time / 24.0 << " days)\n";
      // start new group 
      group = row._group;
      group_time = 0.0;
      os << "==== Kinematic Group " << group << ":  Q2 = " << row.kinematic.Q2 << " GeV2, x = " << row.kinematic.x << " ====\n";
      RunPlanTableEntry::PrintWikiHeader2(os);
    }
    group_time += row.time;
    row.PrintWiki2(os);
  }
  RunPlanTableEntry::PrintWikiFooter2(os);
  os << " total time : " << _total_time << " hrs (" << _total_time/24.0 << " days)\n";
}

void RunTable::PrintTable(std::string table_name, std::string ofilename) const {
  using namespace nlohmann;
  json j_table;
  json j;
  fs::path in_path = ofilename;
  if (fs::exists(in_path)) {
    std::cout << "Loading exiting file : " << ofilename << "\n";
    std::ifstream json_input_file(ofilename);
    json_input_file >> j;
  }
  //std::cout << TBufferJSON::ToJSON(this) << "\n";
  std::stringstream ss;
  ss << TBufferJSON::ToJSON(this).Data();
  std::string table_json = TBufferJSON::ToJSON(this).Data();
  std::cout << table_json << "\n";
  ss >> j_table;
  j[table_name]         = j_table;//json::parse(table_json);
  {
    std::ofstream json_out_file(ofilename);
    json_out_file << j.dump(0) << std::endl;
  }
}

RunTable* RunTable::LoadTable(std::string table_name, std::string ofilename) {
  fs::path in_path = ofilename;
  if (!fs::exists(in_path)) {
    std::cout << "error: file " << ofilename << " does not exist\n";
  }
  using namespace nlohmann;
  json          j;
  std::ifstream infile(ofilename);
  infile >> j;
  // find an entry
  RunTable* res = nullptr;
  if (j.find(table_name) != j.end()) {
    TBufferJSON::FromJSON(res, j[table_name].dump().c_str());
    //std::cout << it.key() << ", ";
    //runs.push_back(std::stoi(it.key()));
  }
  return res;
}
  void RunTable::add(const RunTable& tab) {
    for(const auto& entry : tab.GetVector() ) {
      add(entry);
    }
  }

