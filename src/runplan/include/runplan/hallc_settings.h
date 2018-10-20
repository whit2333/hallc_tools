#ifndef hallc_settings_HH
#define hallc_settings_HH 1

#include <array>
#include <iostream>
#include <map>
#include <tuple>
#include <vector>
#define _USE_MATH_DEFINES
#include <cmath> 

//#if !defined(__CLING__)
//#include "InSANE/PhysicalConstants.h"
//#include "InSANE/SystemOfUnits.h"
//using insane::units::degree;
//#else
//const double degree = M_PI/180.0;
//#endif

namespace hallc {


  /** hadron pids.
   * pi+     K+          p+
   * 221     321         2212
   * 0.13957 0.493677    0.938
   */
  const std::map<int, double> Hadron_masses = {{211, 0.13957},   {-211, 0.13957}, {321, 0.493677},
    {-321, 0.493677}, {2212, 0.938},   {-2212, 0.938}};
  /** SIDIS Kinematic.
   * 
   */
  struct SIDIS_Kinematic {
    // defining variables:
    double E0       = 10.6;
    double Q2       = 0.0;
    double x        = 0.0;
    double z        = 0.0;
    int    pid_had  = 211;
    double P_h_perp = 0.0;
    double phi_h    = 0.0;

    // computed variables:
    double nu   = 0.0;
    double W    = 0.0;
    double Wp   = 0.0;
    double th_e = 0.0;
    double th_q = 0.0;
    double Ee   = 0.0;
    double Ppi  = 0.0;
    double M_h  = 0.135;
    double E_h  = 0.0;
    double y    = 0.0;
    double q    = 0.0;

    void Compute(); 

    void Print(std::ostream& s = std::cout) const; 
    void PrintHeader(std::ostream& s = std::cout) const; 
  };
  using Kinematic = SIDIS_Kinematic;

  /** Inputs variables are: 
   *    x, Q2, z, E0.
   */
  //HCKinematic RecomputeKinematic_SIDIS(HCKinematic k1, double Q2);
  Kinematic RecomputeKinematic_SIDIS(Kinematic k1, double Q2);

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
    double HMS_theta  = 14.5*M_PI/180.0;//*degree;
    double HMS_p0     = 5.0; // GeV/c
    double HMS_phi    = M_PI;
    // SHMS detects hadron
    double SHMS_theta = 13.5*M_PI/180.0;//*degree;
    double SHMS_p0    = 3.0; // GeV/c
    double SHMS_phi   = 0.0; // SHMS sits on the +x side

    double SHMS_dtheta      = shms::SHMS_dtheta     ;
    double SHMS_dphi        = shms::SHMS_dphi       ;
    double SHMS_dP_low      = shms::SHMS_dP_low     ;
    double SHMS_dP_high     = shms::SHMS_dP_high    ;
    double SHMS_solid_angle = shms::SHMS_solid_angle;
    double HMS_dtheta       = hms::HMS_dtheta       ;
    double HMS_dphi         = hms::HMS_dphi         ;
    double HMS_dP_low       = hms::HMS_dP_low       ;
    double HMS_dP_high      = hms::HMS_dP_high      ;
    double HMS_solid_angle  = hms::HMS_solid_angle  ;


    double HMS_P_min()  const { return HMS_p0 * (1.0 - HMS_dP_low); }
    double HMS_P_max()  const { return HMS_p0 * (1.0 + HMS_dP_high); }
    double SHMS_P_min() const { return SHMS_p0 * (1.0 - SHMS_dP_low); }
    double SHMS_P_max() const { return SHMS_p0 * (1.0 + SHMS_dP_high); }

    double HMS_phi_min()  const { return HMS_phi -  HMS_dphi; }
    double HMS_phi_max()  const { return HMS_phi +  HMS_dphi; }
    double SHMS_phi_min() const { return SHMS_phi - SHMS_dphi; }
    double SHMS_phi_max() const { return SHMS_phi + SHMS_dphi; }

    double HMS_theta_min()  const { return HMS_theta -  HMS_dtheta; }
    double HMS_theta_max()  const { return HMS_theta +  HMS_dtheta; }
    double SHMS_theta_min() const { return SHMS_theta - SHMS_dtheta; }
    double SHMS_theta_max() const { return SHMS_theta + SHMS_dtheta; }

    void Print() const {
      std::cout << "HMS:  \n";
      std::cout << "   P0    = " << HMS_p0 << " GeV/c\n";
      std::cout << "   theta = " << HMS_theta * 180.0 / M_PI << " deg\n";
      std::cout << "   phi   = " << HMS_phi * 180.0 / M_PI << " deg\n";
      std::cout << "SHMS:  \n";
      std::cout << "   P0    = " << SHMS_p0 << " GeV/c\n";
      std::cout << "   theta = " << SHMS_theta * 180.0 / M_PI << " deg\n";
      std::cout << "   phi   = " << SHMS_phi * 180.0 / M_PI << " deg\n";
    }
  };

}

namespace csv {

  using CSV_Settings = std::vector<hallc::Kinematic>;

  const double Q2_0 = 4.0;
  const CSV_Settings kine0_settings =  {

    //E0   Q2    x     z     pid_had P_h_perp   phi_h    
    //{10.6, Q2_0, 0.30, 0.40, 211,    0.0,       0.0},
    //{10.6, Q2_0, 0.30, 0.50, 211,    0.0,       0.0},
    //{10.6, Q2_0, 0.30, 0.60, 211,    0.0,       0.0},
    //{10.6, Q2_0, 0.30, 0.70, 211,    0.0,       0.0},
    {10.6, Q2_0, 0.35, 0.40, 211,    0.0,       0.0},
    {10.6, Q2_0, 0.35, 0.50, 211,    0.0,       0.0},
    {10.6, Q2_0, 0.35, 0.60, 211,    0.0,       0.0},
    {10.6, Q2_0, 0.35, 0.70, 211,    0.0,       0.0},
    {10.6, Q2_0, 0.40, 0.40, 211,    0.0,       0.0},
    {10.6, Q2_0, 0.40, 0.50, 211,    0.0,       0.0},
    {10.6, Q2_0, 0.40, 0.60, 211,    0.0,       0.0},
    {10.6, Q2_0, 0.40, 0.70, 211,    0.0,       0.0},
    {10.6, Q2_0, 0.45, 0.40, 211,    0.0,       0.0},
    {10.6, Q2_0, 0.45, 0.50, 211,    0.0,       0.0},
    {10.6, Q2_0, 0.45, 0.60, 211,    0.0,       0.0},
    {10.6, Q2_0, 0.45, 0.70, 211,    0.0,       0.0},
    {10.6, Q2_0, 0.5, 0.40, 211,    0.0,       0.0},
    {10.6, Q2_0, 0.5, 0.50, 211,    0.0,       0.0},
    {10.6, Q2_0, 0.5, 0.60, 211,    0.0,       0.0},
    {10.6, Q2_0, 0.5, 0.70, 211,    0.0,       0.0}
  };


  const double Q2_1 = 4.75;
  const CSV_Settings kine1_settings =  {
    //E0   Q2    x     z     pid_had P_h_perp   phi_h    
    {10.6, Q2_1, 0.45, 0.40},
    {10.6, Q2_1, 0.45, 0.50},
    {10.6, Q2_1, 0.45, 0.60},
    {10.6, Q2_1, 0.45, 0.70},
    {10.6, Q2_1, 0.50, 0.40},
    {10.6, Q2_1, 0.50, 0.50},
    {10.6, Q2_1, 0.50, 0.60},
    {10.6, Q2_1, 0.50, 0.70},
    {10.6, Q2_1, 0.55, 0.40},
    {10.6, Q2_1, 0.55, 0.50},
    {10.6, Q2_1, 0.55, 0.60},
    {10.6, Q2_1, 0.55, 0.70},
    {10.6, Q2_1, 0.60, 0.40},
    {10.6, Q2_1, 0.60, 0.50},
    {10.6, Q2_1, 0.60, 0.60},
    {10.6, Q2_1, 0.60, 0.70}
  };

  const double Q2_1_v2 = 4.5;
  const CSV_Settings kine1_settings_v2 =  {
    //E0   Q2    x     z     pid_had P_h_perp   phi_h    
    {10.6, Q2_1_v2, 0.45, 0.40},
    {10.6, Q2_1_v2, 0.45, 0.50},
    {10.6, Q2_1_v2, 0.45, 0.60},
    {10.6, Q2_1_v2, 0.45, 0.70},
    {10.6, Q2_1_v2, 0.50, 0.40},
    {10.6, Q2_1_v2, 0.50, 0.50},
    {10.6, Q2_1_v2, 0.50, 0.60},
    {10.6, Q2_1_v2, 0.50, 0.70},
    {10.6, Q2_1_v2, 0.55, 0.40},
    {10.6, Q2_1_v2, 0.55, 0.50},
    {10.6, Q2_1_v2, 0.55, 0.60},
    {10.6, Q2_1_v2, 0.55, 0.70},
    {10.6, Q2_1_v2, 0.60, 0.40},
    {10.6, Q2_1_v2, 0.60, 0.50},
    {10.6, Q2_1_v2, 0.60, 0.60},
    {10.6, Q2_1_v2, 0.60, 0.70}
  };


  const double Q2_2 = 5.75;
  const CSV_Settings kine2_settings =  {
    //E0   Q2    x     z     pid_had P_h_perp   phi_h    
    {10.6, Q2_2, 0.50, 0.40},
    {10.6, Q2_2, 0.50, 0.50},
    {10.6, Q2_2, 0.50, 0.60},
    {10.6, Q2_2, 0.50, 0.70},
    {10.6, Q2_2, 0.55, 0.40},
    {10.6, Q2_2, 0.55, 0.50},
    {10.6, Q2_2, 0.55, 0.60},
    {10.6, Q2_2, 0.55, 0.70},
    {10.6, Q2_2, 0.60, 0.40},
    {10.6, Q2_2, 0.60, 0.50},
    {10.6, Q2_2, 0.60, 0.60},
    {10.6, Q2_2, 0.60, 0.70},
    {10.6, Q2_2, 0.65, 0.40},
    {10.6, Q2_2, 0.65, 0.50},
    {10.6, Q2_2, 0.65, 0.60},
    {10.6, Q2_2, 0.65, 0.70}
  };

  const double Q2_2_v2 = 5.5;
  const CSV_Settings kine2_settings_v2 =  {
    //E0   Q2    x     z     pid_had P_h_perp   phi_h    
    {10.6, Q2_2_v2, 0.50, 0.40},
    {10.6, Q2_2_v2, 0.50, 0.50},
    {10.6, Q2_2_v2, 0.50, 0.60},
    {10.6, Q2_2_v2, 0.50, 0.70},
    {10.6, Q2_2_v2, 0.55, 0.40},
    {10.6, Q2_2_v2, 0.55, 0.50},
    {10.6, Q2_2_v2, 0.55, 0.60},
    {10.6, Q2_2_v2, 0.55, 0.70},
    {10.6, Q2_2_v2, 0.60, 0.40},
    {10.6, Q2_2_v2, 0.60, 0.50},
    {10.6, Q2_2_v2, 0.60, 0.60},
    {10.6, Q2_2_v2, 0.60, 0.70},
    {10.6, Q2_2_v2, 0.65, 0.40},
    {10.6, Q2_2_v2, 0.65, 0.50},
    {10.6, Q2_2_v2, 0.65, 0.60},
    {10.6, Q2_2_v2, 0.65, 0.70}
  };

  const std::vector<std::pair<double, CSV_Settings>> all_settings = {
    {Q2_0, kine0_settings}, {Q2_1, kine1_settings}, {Q2_2, kine2_settings}};

  const CSV_Settings LH2_kine0_settings =  {
    //E0   Q2    x     z     pid_had P_h_perp   phi_h    
    //{10.6, Q2_0, 0.30, 0.40},
    //{10.6, Q2_0, 0.30, 0.50},
    //{10.6, Q2_0, 0.30, 0.60},
    //{10.6, Q2_0, 0.30, 0.70},
    {10.6, Q2_0, 0.35, 0.40},
    {10.6, Q2_0, 0.35, 0.50},
    {10.6, Q2_0, 0.35, 0.60},
    {10.6, Q2_0, 0.35, 0.70},
    //{10.6, Q2_0, 0.40, 0.40},
    //{10.6, Q2_0, 0.40, 0.50},
    //{10.6, Q2_0, 0.40, 0.60},
    //{10.6, Q2_0, 0.40, 0.70},
    {10.6, Q2_0, 0.45, 0.40},
    {10.6, Q2_0, 0.45, 0.50},
    {10.6, Q2_0, 0.45, 0.60},
    {10.6, Q2_0, 0.45, 0.70}
  };

  const CSV_Settings LH2_kine1_settings =  {
    //E0   Q2    x     z     pid_had P_h_perp   phi_h    
    {10.6, Q2_1, 0.45, 0.40},
    {10.6, Q2_1, 0.45, 0.50},
    {10.6, Q2_1, 0.45, 0.60},
    {10.6, Q2_1, 0.45, 0.70},
    //{10.6, Q2_1, 0.50, 0.40},
    //{10.6, Q2_1, 0.50, 0.50},
    //{10.6, Q2_1, 0.50, 0.60},
    //{10.6, Q2_1, 0.50, 0.70},
    {10.6, Q2_1, 0.55, 0.40},
    {10.6, Q2_1, 0.55, 0.50},
    {10.6, Q2_1, 0.55, 0.60},
    {10.6, Q2_1, 0.55, 0.70},
    //{10.6, Q2_1, 0.60, 0.40},
    //{10.6, Q2_1, 0.60, 0.50},
    //{10.6, Q2_1, 0.60, 0.60},
    //{10.6, Q2_1, 0.60, 0.70}
  };
  const std::vector<std::pair<double, CSV_Settings>> LH2_settings = {
    {Q2_0, LH2_kine0_settings}, {Q2_1, LH2_kine1_settings}};

  const CSV_Settings test_kine_settings =  {
    //E0   Q2    x     z     pid_had P_h_perp   phi_h    
    {10.6, Q2_1, 0.45, 0.40},
    {10.6, Q2_1, 0.60, 0.50},
  };

}

#endif
