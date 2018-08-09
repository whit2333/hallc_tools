#include "runplan/hallc_settings.h"

#if !defined(__CLING__)
#include <fmt/core.h>
#include <fmt/ostream.h>
#endif

void RunPlanTableEntry::PrintWikiHeader(std::ostream& s) {
  fmt::print(s,
             "{{|class=\"wikitable\"\n|{:^6}|| {:^5}|| {:^5}|| {:^6}|| {:^6}|| {:^6}|| {:^6}|| "
             "{:^3}|| {:^6}|| {:^9}|| {:^6}|| {:^6} |\n|- \n",
             "target", "x", "z", "th_e", "th_q", "Ee", "Ppi", "pol", "I", "sigma", "rate", "time");
}
void RunPlanTableEntry::PrintWikiFooter(std::ostream& s) {
  fmt::print(s,
             "|{:^6}|| {:^5}|| {:^5}|| {:^6}|| {:^6}|| {:^6}|| {:^6}|| {:^3}|| {:^6}|| {:^9}|| "
             "{:^6}|| {:^6} |\n|}}\n",
             "target", "x", "z", "th_e", "th_q", "Ee", "Ppi", "pol", "I", "sigma", "rate", "time");
}

void RunPlanTableEntry::PrintWiki(std::ostream& s) const {
  std::string target = "LD2";
  double      xs     = rates.LD2_XS;
  if (A_target == 1) {
    target    = "LH2";
    double xs = rates.LH2_XS;
  }
  fmt::print(s,
             "|{:^6}|| {:>5.3f}|| {:>5.3f}|| {:>6.2f}|| {:>6.2f}|| {:>6.3f}|| {:>6.3f}|| "
             "{:>+3d}|| {:>6.3f}|| {:>6.3e}|| {:>6.3f}|| {:>6.3f}|| {:>6.0f}\n|- \n",
             target, kinematic.x, kinematic.z, kinematic.th_e, kinematic.th_q, kinematic.Ee,
             kinematic.Ppi, polarity, Ibeam, xs, rates.total_rate, time, counts);
}
void RunPlanTableEntry::Print(std::ostream& s) const {
  std::string target = "LD2";
  double      xs     = rates.LD2_XS;
  if (A_target == 1) {
    target    = "LH2";
    double xs = rates.LH2_XS;
  }
  fmt::print(s,
             "{:^6}, {:>5.3f}, {:>5.3f}, {:>5.3f}, {:>6.2f}, {:>6.2f}, {:>6.3f}, {:>6.3f}, "
             "{:>+3d}, {:>6.3f}, {:>6.3e}, {:>6.3f}, {:>6.3f}, {:>6.0f}\n",
             target, kinematic.Q2, kinematic.x, kinematic.z, kinematic.th_e, kinematic.th_q,
             kinematic.Ee, kinematic.Ppi, polarity, Ibeam, xs, rates.total_rate, time, counts);
}
void RunPlanTableEntry::PrintHeader(std::ostream& s ) {
  fmt::print(s,
             "{:^6}, {:^5}, {:^5}, {:^5}, {:^6}, {:^6}, {:^6}, {:^6}, {:^3}, {:^6}, {:^9}, "
             "{:^6}, {:^6}\n",
             "target", "Q2", "x", "z", "th_e", "th_q", "Ee", "Ppi", "pol", "I", "sigma", "rate",
             "time");
}


