#include "runplan/hallc_settings.h"

#if !defined(__CLING__)
#include <fmt/core.h>
#include <fmt/ostream.h>
#endif

namespace hallc {

  void HCKinematic::Print(std::ostream& s) const {
    fmt::print(s,
               "{:>4.3f}, {:>4.3f}, {:>4.3f}, {:>4.3f}, {:>4.3f}, {:>4.3f}, {:>4.3f}, {:>4.3f}, {:>4.3f}, {:>4.3f}, {:>4.3f}\n",
               E0, Q2, x, z, th_e, th_q, Ee, Ppi, nu, W, Wp);
  }

  void HCKinematic::PrintHeader(std::ostream& s) const {
  fmt::print(s,
             "|{:^6}|{:^6}|{:^6}|{:^6}|{:^6}|{:^6}|{:^6}|{:^6}|{:^6}|{:^6}|{:^6}|\n",
             "E", "Q2", "x", "z", "th_e", "th_q", "Ee", "Ppi","nu", "W" ,"Wp");
  }

  HCKinematic RecomputeKinematic_SIDIS(HCKinematic k2, double Q2) {
    const double M  = 0.938;
    k2.Q2           = Q2;
    k2.Ee           = k2.E0 - Q2 / (2.0 * M * k2.x);
    double y        = (k2.E0 - k2.Ee) / k2.E0;
    k2.nu           = y * k2.E0;
    k2.th_e         = 2.0*std::asin(std::sqrt(M * k2.x * y / (2.0 * k2.E0 * (1.0 - y))));
    double q  = std::sqrt(Q2 +k2.nu*k2.nu);//std::sqrt(k2.E0 * k2.E0 + k2.Ee * k2.Ee - 2.0 * k2.E0 * k2.Ee * std::cos(k2.th_e));
    double Eh = k2.nu * k2.z;
    k2.E_h    = Eh;
    k2.th_q   = std::asin(k2.Ee*std::sin(k2.th_e)/q);
    k2.Ppi    = std::sqrt(Eh * Eh - k2.M_h * k2.M_h);
    k2.W      = std::sqrt(M * M - Q2 + 2.0 * M * k2.nu);
    k2.Wp = std::sqrt((M + k2.nu - k2.E_h ) * (M + k2.nu - k2.E_h ) - (k2.Ppi - q) * (k2.Ppi - q));
    k2.th_e   = k2.th_e *180.0/M_PI;
    k2.th_q   = k2.th_q *180.0/M_PI;

    //k2.PrintHeader(std::cout);
    //k2.Print(std::cout);

    return k2;
  }
}

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


