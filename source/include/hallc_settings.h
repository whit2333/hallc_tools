#ifndef hallc_settings_HH
#define hallc_settings_HH

#include <array>
#include <iostream>
#include <map>
#include <tuple>
#include <vector>

#include <fmt/core.h>
#include <fmt/ostream.h>

#include "InSANE/PhysicalConstants.h"
#include "InSANE/SystemOfUnits.h"


using insane::units::degree;

namespace hallc {

  struct HCKinematic {
    double x    = 0.0;
    double z    = 0.0;
    double nu   = 0.0;
    double W    = 0.0;
    double Wp   = 0.0;
    double th_e = 0.0;
    double th_q = 0.0;
    double Ee   = 0.0;
    double Ppi  = 0.0;
    double Q2   = 0.0;
  };
  using Kinematic = HCKinematic;


  namespace shms {
    const double SHMS_dtheta      = 0.024; // 24 mrad
    const double SHMS_dphi        = 0.040; // 40 mrad
    const double SHMS_dP_low      = 0.1;  // -10%
    const double SHMS_dP_high     = 0.22; //  22%
    const double SHMS_solid_angle = 4.0*SHMS_dtheta*SHMS_dphi; // 4 msr
  }
  namespace hms {
    const double HMS_dtheta       = 0.04; // 
    const double HMS_dphi         = 0.04; //
    const double HMS_dP_low       = 0.09; // -9%
    const double HMS_dP_high      = 0.09; //  9%
    const double HMS_solid_angle  = 4.0*HMS_dtheta*HMS_dphi; // 4 msr
  }

  /** Hall C Spectrometer Settings. 
   *
   * SHMS:
   *  Central Momentum:   Momentum Acceptance:     Momentum Resolution:
   *       2 to 11 GeV/c       -10% < δ < +22%          0.03%-0.08%
   *  Scattering Angle:  Horizontal acceptance:    Vertical acceptance:
   *       5.5 to 40°         +- 24mrad                 +- 40 mrad
   *  Solid angle acceptance: 
   *       4.0 msr            
   *  Vertex Length (90deg):  Vertex length res (Ytar):
   *       30cm                    0.1-0.3 cm 
   *  Horizontal resolution (YPtar): Vertical resolution (XPtar):
   *       0.5-1.2 mrad                   0.3-1.1 mrad
   *
   * HMS:
   * https://www.jlab.org/Hall-C/equipment/HMS/performance.html
   *
   *
   * Coordinate system used:
   * z = beam direction
   * x = SHMS (beam left)
   * y = up (towards hall ceiling)
   *  
   */
  struct HallCSetting {

    // HMS detects electron
    double HMS_theta  = 14.5*degree;
    double HMS_p0     = 5.0; // GeV/c
    double HMS_phi          = insane::units::pi;
    // SHMS detects hadron
    double SHMS_theta = 13.5*degree;
    double SHMS_p0    = 3.0; // GeV/c
    double SHMS_phi         = 0.0; // SHMS sits on the +x side

    double HMS_P_min()  const { return HMS_p0 * (1.0 - hms::HMS_dP_low); }
    double HMS_P_max()  const { return HMS_p0 * (1.0 + hms::HMS_dP_high); }
    double SHMS_P_min() const { return SHMS_p0 * (1.0 - shms::SHMS_dP_low); }
    double SHMS_P_max() const { return SHMS_p0 * (1.0 + shms::SHMS_dP_high); }

    double HMS_phi_min()  const { return HMS_phi - hms::HMS_dphi; }
    double HMS_phi_max()  const { return HMS_phi + hms::HMS_dphi; }
    double SHMS_phi_min() const { return SHMS_phi - shms::SHMS_dphi; }
    double SHMS_phi_max() const { return SHMS_phi + shms::SHMS_dphi; }

    double HMS_theta_min()  const { return HMS_theta - hms::HMS_dtheta; }
    double HMS_theta_max()  const { return HMS_theta + hms::HMS_dtheta; }
    double SHMS_theta_min() const { return SHMS_theta - shms::SHMS_dtheta; }
    double SHMS_theta_max() const { return SHMS_theta + shms::SHMS_dtheta; }
  };

}

/** Rate calculated on a given target
 */
struct TargetRates  {
  //double total_XS    = 0.0;
  double window_XS   = 0.0;
  double LH2_XS      = 0.0;
  double LD2_XS      = 0.0;
  double p_XS        = 0.0;
  double n_XS        = 0.0;
  //double total_Lumi  = 0.0;
  double window_Lumi = 0.0;
  double LH2_Lumi    = 0.0;
  double LD2_Lumi    = 0.0;

  double window_rate = 0.0;
  double LH2_rate    = 0.0;
  double LD2_rate    = 0.0;
  double total_LD2_rate  = 0.0;
  double total_LH2_rate  = 0.0;
  double total_rate  = 0.0;
};

/** Run Plan Table Entry
 *
 */
struct RunPlanTableEntry {
public:
  hallc::HCKinematic  kinematic;
  hallc::HallCSetting hcSet;
  TargetRates         rates;

  double counts        = 0.0;
  double time          = 0.0;
  int    polarity      = 0.0;
  double Ibeam         = 0.0;
  double LD2_counts    = 0.0;
  double LH2_counts    = 0.0;
  double window_counts = 0.0;
  int    Z_target      = 1;
  int    A_target      = 2;

public:
  RunPlanTableEntry(){ }
  RunPlanTableEntry(const RunPlanTableEntry&) = default;
  RunPlanTableEntry(RunPlanTableEntry&&)      = default;
  RunPlanTableEntry& operator=(const RunPlanTableEntry&) = default;
  RunPlanTableEntry& operator=(RunPlanTableEntry&&) = default;

  static void PrintWikiHeader(std::ostream& s = std::cout) {
    fmt::print(s,
               "{{|class=\"wikitable\"\n|{:^6}|| {:^5}|| {:^5}|| {:^6}|| {:^6}|| {:^6}|| {:^6}|| "
               "{:^3}|| {:^6}|| {:^9}|| {:^6}|| {:^6} |\n|- \n",
               "target", "x", "z", "th_e", "th_q", "Ee", "Ppi", "pol", "I", "sigma", "rate",
               "time");
  }
  static void PrintWikiFooter(std::ostream& s = std::cout) {
    fmt::print(s,
               "|{:^6}|| {:^5}|| {:^5}|| {:^6}|| {:^6}|| {:^6}|| {:^6}|| {:^3}|| {:^6}|| {:^9}|| "
               "{:^6}|| {:^6} |\n|}}\n",
               "target", "x", "z", "th_e", "th_q", "Ee", "Ppi", "pol", "I", "sigma", "rate",
               "time");
  }

  void PrintWiki(std::ostream& s = std::cout) const {
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
  void Print(std::ostream& s = std::cout) const {
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
  static void PrintHeader(std::ostream& s = std::cout) {
    fmt::print(s,
               "{:^6}, {:^5}, {:^5}, {:^5}, {:^6}, {:^6}, {:^6}, {:^6}, {:^3}, {:^6}, {:^9}, "
               "{:^6}, {:^6}\n",
               "target", "Q2", "x", "z", "th_e", "th_q", "Ee", "Ppi", "pol", "I", "sigma", "rate",
               "time");
  }
};

#if defined(__ROOTCLING__)
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ all typedef;

#pragma link C++ nestedclass;
#pragma link C++ nestedtypedef;

#pragma link C++ namespace hallc;

#pragma link C++ class hallc::HCKinematic+;
#pragma link C++ class std::vector<hallc::HCKinematic>+;
#pragma link C++ class std::vector<std::pair<double, hallc::HCKinematic>>+;

#pragma link C++ class hallc::HallCSetting+;
#pragma link C++ class TargetRates+;
#pragma link C++ class RunPlanTableEntry+;
#pragma link C++ class std::vector<RunPlanTableEntry>+;
#pragma link C++ class std::pair<double,std::vector<RunPlanTableEntry>>+;
#pragma link C++ class std::vector<std::pair<double,std::vector<RunPlanTableEntry>>>+;
#endif

namespace csv {

  using CSV_Settings = std::vector<hallc::HCKinematic>;

  double Q2_0 = 4.0;
  CSV_Settings kine0_settings =  {
    //x     z    nu    W      W'  th_e   th_q  Ee   Ppi
    {0.30,0.40,6.300,3.025,2.405,15.05,10.70,4.700,2.516},
    {0.30,0.50,6.300,3.025,2.227,15.05,10.70,4.700,3.147},
    {0.30,0.60,6.300,3.025,2.032,15.05,10.70,4.700,3.777},
    {0.30,0.70,6.300,3.025,1.816,15.05,10.70,4.700,4.408},
    {0.35,0.40,5.400,2.732,2.181,13.78,13.49,5.600,2.156},
    {0.35,0.50,5.400,2.732,2.025,13.78,13.49,5.600,2.696},
    {0.35,0.60,5.400,2.732,1.854,13.78,13.49,5.600,3.237},
    {0.35,0.70,5.400,2.732,1.666,13.78,13.49,5.600,3.777},
    {0.40,0.40,4.700,2.484,1.992,12.95,16.20,6.300,1.875},
    {0.40,0.50,4.700,2.484,1.854,12.95,16.20,6.300,2.346},
    {0.40,0.60,4.700,2.484,1.704,12.95,16.20,6.300,2.817},
    {0.40,0.70,4.700,2.484,1.538,12.95,16.20,6.300,3.287},
    {0.45,0.40,4.200,2.283,1.839,12.50,18.65,6.800,1.674},
    {0.45,0.50,4.200,2.283,1.715,12.50,18.65,6.800,2.095},
    {0.45,0.60,4.200,2.283,1.581,12.50,18.65,6.800,2.516},
    {0.45,0.70,4.200,2.283,1.434,12.50,18.65,6.800,2.937}
  };


  double Q2_1 = 5.0;
  CSV_Settings kine1_settings =  {
    //x,    z    nu    W      W'  th_e   th_q  Ee   Ppi
    {0.45,0.40,6.000,2.659,2.120,17.46,13.54,5.000,2.396},
    {0.45,0.50,6.000,2.659,1.967,17.46,13.54,5.000,2.997},
    {0.45,0.60,6.000,2.659,1.800,17.46,13.54,5.000,3.597},
    {0.45,0.70,6.000,2.659,1.616,17.46,13.54,5.000,4.198},
    {0.50,0.40,5.400,2.438,1.951,16.49,15.76,5.600,2.156},
    {0.50,0.50,5.400,2.438,1.813,16.49,15.76,5.600,2.696},
    {0.50,0.60,5.400,2.438,1.664,16.49,15.76,5.600,3.237},
    {0.50,0.70,5.400,2.438,1.499,16.49,15.76,5.600,3.777},
    {0.55,0.40,4.900,2.240,1.798,15.78,17.92,6.100,1.955},
    {0.55,0.50,4.900,2.240,1.675,15.78,17.92,6.100,2.446},
    {0.55,0.60,4.900,2.240,1.541,15.78,17.92,6.100,2.937},
    {0.55,0.70,4.900,2.240,1.393,15.78,17.92,6.100,3.427},
    {0.60,0.40,4.500,2.063,1.661,15.30,19.93,6.500,1.795},
    {0.60,0.50,4.500,2.063,1.550,15.30,19.93,6.500,2.246},
    {0.60,0.60,4.500,2.063,1.430,15.30,19.93,6.500,2.696},
    {0.60,0.70,4.500,2.063,1.298,15.30,19.93,6.500,3.147}
  };


  double Q2_2 = 6.1;
  CSV_Settings kine2_settings =  {
    //x     z    nu    W      W'  th_e   th_q  Ee   Ppi
    {0.50,0.40,6.500,2.641,2.103,20.21,12.92,4.500,2.596},
    {0.50,0.50,6.500,2.641,1.950,20.21,12.92,4.500,3.247},
    {0.50,0.60,6.500,2.641,1.783,20.21,12.92,4.500,3.898},
    {0.50,0.70,6.500,2.641,1.598,20.21,12.92,4.500,4.548},
    {0.55,0.40,5.900,2.421,1.933,18.96,15.02,5.100,2.356},
    {0.55,0.50,5.900,2.421,1.796,18.96,15.02,5.100,2.947},
    {0.55,0.60,5.900,2.421,1.646,18.96,15.02,5.100,3.537},
    {0.55,0.70,5.900,2.421,1.480,18.96,15.02,5.100,4.128},
    {0.60,0.40,5.400,2.221,1.778,18.07,17.02,5.600,2.156},
    {0.60,0.50,5.400,2.221,1.655,18.07,17.02,5.600,2.696},
    {0.60,0.60,5.400,2.221,1.521,18.07,17.02,5.600,3.237},
    {0.60,0.70,5.400,2.221,1.372,18.07,17.02,5.600,3.777},
    {0.65,0.40,5.000,2.040,1.638,17.48,18.86,6.000,1.995},
    {0.65,0.50,5.000,2.040,1.527,17.48,18.86,6.000,2.496},
    {0.65,0.60,5.000,2.040,1.406,17.48,18.86,6.000,2.997},
    {0.65,0.70,5.000,2.040,1.273,17.48,18.86,6.000,3.497}
  };

  std::vector<std::pair<double, CSV_Settings>> all_settings = {
    {Q2_0, kine0_settings}, {Q2_1, kine1_settings}, {Q2_2, kine2_settings}};

  CSV_Settings LH2_kine0_settings =  {
    //x     z    nu    W      W'  th_e   th_q  Ee   Ppi
    {0.30,0.40,6.300,3.025,2.405,15.05,10.70,4.700,2.516},
    {0.30,0.50,6.300,3.025,2.227,15.05,10.70,4.700,3.147},
    {0.30,0.60,6.300,3.025,2.032,15.05,10.70,4.700,3.777},
    {0.30,0.70,6.300,3.025,1.816,15.05,10.70,4.700,4.408},
    {0.35,0.40,5.400,2.732,2.181,13.78,13.49,5.600,2.156},
    {0.35,0.50,5.400,2.732,2.025,13.78,13.49,5.600,2.696},
    {0.35,0.60,5.400,2.732,1.854,13.78,13.49,5.600,3.237},
    {0.35,0.70,5.400,2.732,1.666,13.78,13.49,5.600,3.777},
    //{0.40,0.40,4.700,2.484,1.992,12.95,16.20,6.300,1.875},
    //{0.40,0.50,4.700,2.484,1.854,12.95,16.20,6.300,2.346},
    //{0.40,0.60,4.700,2.484,1.704,12.95,16.20,6.300,2.817},
    //{0.40,0.70,4.700,2.484,1.538,12.95,16.20,6.300,3.287},
    //{0.45,0.40,4.200,2.283,1.839,12.50,18.65,6.800,1.674},
    //{0.45,0.50,4.200,2.283,1.715,12.50,18.65,6.800,2.095},
    //{0.45,0.60,4.200,2.283,1.581,12.50,18.65,6.800,2.516},
    //{0.45,0.70,4.200,2.283,1.434,12.50,18.65,6.800,2.937}
  };

  CSV_Settings LH2_kine1_settings =  {
    //x,    z    nu    W      W'  th_e   th_q  Ee   Ppi
    {0.45,0.40,6.000,2.659,2.120,17.46,13.54,5.000,2.396},
    {0.45,0.50,6.000,2.659,1.967,17.46,13.54,5.000,2.997},
    {0.45,0.60,6.000,2.659,1.800,17.46,13.54,5.000,3.597},
    {0.45,0.70,6.000,2.659,1.616,17.46,13.54,5.000,4.198},
    {0.50,0.40,5.400,2.438,1.951,16.49,15.76,5.600,2.156},
    {0.50,0.50,5.400,2.438,1.813,16.49,15.76,5.600,2.696},
    {0.50,0.60,5.400,2.438,1.664,16.49,15.76,5.600,3.237},
    {0.50,0.70,5.400,2.438,1.499,16.49,15.76,5.600,3.777},
    //{0.55,0.40,4.900,2.240,1.798,15.78,17.92,6.100,1.955},
    //{0.55,0.50,4.900,2.240,1.675,15.78,17.92,6.100,2.446},
    //{0.55,0.60,4.900,2.240,1.541,15.78,17.92,6.100,2.937},
    //{0.55,0.70,4.900,2.240,1.393,15.78,17.92,6.100,3.427}
    //{0.60,0.40,4.500,2.063,1.661,15.30,19.93,6.500,1.795},
    //{0.60,0.50,4.500,2.063,1.550,15.30,19.93,6.500,2.246},
    //{0.60,0.60,4.500,2.063,1.430,15.30,19.93,6.500,2.696},
    //{0.60,0.70,4.500,2.063,1.298,15.30,19.93,6.500,3.147}
  };
  std::vector<std::pair<double, CSV_Settings>> LH2_settings = {
    {Q2_0, LH2_kine0_settings}, {Q2_1, LH2_kine1_settings}};

}

#endif
