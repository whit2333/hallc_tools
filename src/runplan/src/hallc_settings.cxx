#include "runplan/hallc_settings.h"

#if !defined(__CLING__)
#include <fmt/core.h>
#include <fmt/ostream.h>
#endif

namespace hallc {

  void Kinematic::Print(std::ostream& s) const {
    fmt::print(s,
               "{:>4.3f}, {:>4.3f}, {:>4.3f}, {:>4.3f}, {:>4.3f}, {:>4.3f}, {:>4.3f}, {:>4.3f}, {:>4.3f}, {:>4.3f}, {:>4.3f}\n",
               E0, Q2, x, z, th_e, th_q, Ee, Ppi, nu, W, Wp);
  }

  void Kinematic::PrintHeader(std::ostream& s) const {
  fmt::print(s,
             "|{:^6}|{:^6}|{:^6}|{:^6}|{:^6}|{:^6}|{:^6}|{:^6}|{:^6}|{:^6}|{:^6}|\n",
             "E", "Q2", "x", "z", "th_e", "th_q", "Ee", "Ppi","nu", "W" ,"Wp");
  }

  void Kinematic::Compute() {
    const double M = 0.938;
    M_h            = Hadron_masses.at(pid_had);
    Ee             = E0 - Q2 / (2.0 * M * x);
    y              = (E0 - Ee) / E0;
    nu             = y * E0;
    th_e           = 2.0 * std::asin(std::sqrt(M * x * y / (2.0 * E0 * (1.0 - y))));
    q              = std::sqrt(Q2 + nu * nu); // std::sqrt(E0 * E0 + Ee * Ee - 2.0 * E0 * Ee * std::cos(th_e));
    double Eh      = nu * z;
    E_h            = Eh;
    th_q           = std::asin(Ee * std::sin(th_e) / q);
    Ppi            = std::sqrt(Eh * Eh - M_h * M_h);
    W              = std::sqrt(M * M - Q2 + 2.0 * M * nu);
    Wp             = std::sqrt((M + nu - E_h) * (M + nu - E_h) - (Ppi - q) * (Ppi - q));
    th_e           = th_e * 180.0 / M_PI;
    th_q           = th_q * 180.0 / M_PI;
  }
  Kinematic RecomputeKinematic_SIDIS(Kinematic k2, double Q2) {
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
    return k2;
  }

  //void HCKinematic::Print(std::ostream& s) const {
  //  fmt::print(s,
  //             "{:>4.3f}, {:>4.3f}, {:>4.3f}, {:>4.3f}, {:>4.3f}, {:>4.3f}, {:>4.3f}, {:>4.3f}, {:>4.3f}, {:>4.3f}, {:>4.3f}\n",
  //             E0, Q2, x, z, th_e, th_q, Ee, Ppi, nu, W, Wp);
  //}

  //void HCKinematic::PrintHeader(std::ostream& s) const {
  //fmt::print(s,
  //           "|{:^6}|{:^6}|{:^6}|{:^6}|{:^6}|{:^6}|{:^6}|{:^6}|{:^6}|{:^6}|{:^6}|\n",
  //           "E", "Q2", "x", "z", "th_e", "th_q", "Ee", "Ppi","nu", "W" ,"Wp");
  //}

  //HCKinematic RecomputeKinematic_SIDIS(HCKinematic k2, double Q2) {
  //  const double M  = 0.938;
  //  k2.Q2           = Q2;
  //  k2.Ee           = k2.E0 - Q2 / (2.0 * M * k2.x);
  //  double y        = (k2.E0 - k2.Ee) / k2.E0;
  //  k2.nu           = y * k2.E0;
  //  k2.th_e         = 2.0*std::asin(std::sqrt(M * k2.x * y / (2.0 * k2.E0 * (1.0 - y))));
  //  double q  = std::sqrt(Q2 +k2.nu*k2.nu);//std::sqrt(k2.E0 * k2.E0 + k2.Ee * k2.Ee - 2.0 * k2.E0 * k2.Ee * std::cos(k2.th_e));
  //  double Eh = k2.nu * k2.z;
  //  k2.E_h    = Eh;
  //  k2.th_q   = std::asin(k2.Ee*std::sin(k2.th_e)/q);
  //  k2.Ppi    = std::sqrt(Eh * Eh - k2.M_h * k2.M_h);
  //  k2.W      = std::sqrt(M * M - Q2 + 2.0 * M * k2.nu);
  //  k2.Wp = std::sqrt((M + k2.nu - k2.E_h ) * (M + k2.nu - k2.E_h ) - (k2.Ppi - q) * (k2.Ppi - q));
  //  k2.th_e   = k2.th_e *180.0/M_PI;
  //  k2.th_q   = k2.th_q *180.0/M_PI;

  //  //k2.PrintHeader(std::cout);
  //  //k2.Print(std::cout);

  //  return k2;
  //}
}

