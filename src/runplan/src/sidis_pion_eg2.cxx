#include <iostream>
#include <fstream>
#include <string>
#include <cmath>

#include "Math/Vector3Dfwd.h"
#include "Math/Vector4Dfwd.h"
#include "Math/Transform3D.h"
#include "Math/Rotation3D.h"
#include "Math/RotationY.h"
#include "Math/RotationZ.h"

#include "InSANE/MaterialProperties.h"
#include "InSANE/Luminosity.h"
#include "PhaseSpaceVariables.h"
#include "NFoldDifferential.h"
#include "Jacobians.h"
#include "DiffCrossSection.h"
#include "FinalState.h"
#include <typeinfo>

#include "PSSampler.h"

//#include "InSANE/DoubleSpinAsymmetries.h"
//#include "InSANE/AMTFormFactors.h"
//#include "InSANE/KellyFormFactors.h"
#include "InSANE/Helpers.h"
#include "InSANE/Physics.h"
#include "InSANE/Kinematics.h"

#include "InSANE/Stat2015_UPDFs.h"
#include "DSSFragmentationFunctions.h"

#include "TH1F.h"
#include "TFile.h"
#include "TTree.h"

#include "ROOT/RDataFrame.hxx"

#include "csv_settings.h"

using namespace ROOT; // TDataFrame's namespace

using namespace insane::physics;
using namespace insane::kinematics;
using namespace insane::units;
using namespace insane::helpers;
using namespace ROOT::Math;

using PDFs   = insane::physics::Stat2015_UPDFs;
using FragFs = insane::physics::DSSFragmentationFunctions;

using SIDIS_vars = insane::kinematics::variables::SIDIS_x_y_z_phih_phie;
using SPEC_vars = insane::kinematics::variables::SIDIS_eprime_ph;
using Q2_variable    = insane::kinematics::variables::MomentumTransfer;
using Theta_variable = insane::kinematics::variables::Theta;


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
  const double SHMS_dtheta      = 0.024; // 24 mrad
  const double SHMS_dphi        = 0.040; // 40 mrad
  const double SHMS_dP_low      = 0.1;  // -10%
  const double SHMS_dP_high     = 0.22; //  22%
  const double SHMS_solid_angle = 4.0*SHMS_dtheta*SHMS_dphi; // 4 msr
  const double SHMS_phi         = 0.0; // SHMS sits on the +x side
  const double HMS_dtheta       = 0.04; // 
  const double HMS_dphi         = 0.04; //
  const double HMS_dP_low       = 0.09; // -9%
  const double HMS_dP_high      = 0.09; //  9%
  const double HMS_solid_angle  = 4.0*HMS_dtheta*HMS_dphi; // 4 msr
  const double HMS_phi          = insane::units::pi-0.04;

  // HMS detects electron
  double HMS_theta  = 14.5*degree;
  double HMS_p0     = 5.0; // GeV/c
  // SHMS detects hadron
  double SHMS_theta = 13.5*degree;
  double SHMS_p0    = 3.0; // GeV/c

  double HMS_P_min()  const { return HMS_p0 * (1.0 - HMS_dP_low); }
  double HMS_P_max()  const { return HMS_p0 * (1.0 + HMS_dP_high); }
  double SHMS_P_min() const { return SHMS_p0 * (1.0 - SHMS_dP_low); }
  double SHMS_P_max() const { return SHMS_p0 * (1.0 + SHMS_dP_high); }

  double HMS_phi_min()  const { return HMS_phi - HMS_dphi; }
  double HMS_phi_max()  const { return HMS_phi + HMS_dphi; }
  double SHMS_phi_min() const { return SHMS_phi - SHMS_dphi; }
  double SHMS_phi_max() const { return SHMS_phi + SHMS_dphi; }

  double HMS_theta_min()  const { return HMS_theta - HMS_dtheta; }
  double HMS_theta_max()  const { return HMS_theta + HMS_dtheta; }
  double SHMS_theta_min() const { return SHMS_theta - SHMS_dtheta; }
  double SHMS_theta_max() const { return SHMS_theta + SHMS_dtheta; }
};

std::vector<double> sidis_pion_eg2(csv::CSV_kinematic kine_set){

  HallCSetting hcSet;
  double       x_set  = kine_set[0];
  double       z_set  = kine_set[1];
  double       nu_set = kine_set[2];
  double       W_set  = kine_set[3];
  double       Wp_set = kine_set[4];
  hcSet.HMS_theta     = kine_set[5];
  hcSet.SHMS_theta    = kine_set[6];
  hcSet.HMS_p0        = kine_set[7];
  hcSet.SHMS_p0       = kine_set[8];

  //SHMS 4 msr dp -10 +20
  //HMS  6 msr dp +-8

  // -------------------------------------------------------
  // Phase space variables
  IPSV phi_SHMS_psv({hcSet.SHMS_phi_min(), hcSet.SHMS_phi_max()}, "phi_shms");
  IPSV phi_HMS_psv({hcSet.HMS_phi_min(), hcSet.HMS_phi_max()}, "phi_hms");
  IPSV theta_HMS_psv({hcSet.HMS_theta_min(), hcSet.HMS_theta_max()}, "theta_hms");
  IPSV theta_SHMS_psv({hcSet.SHMS_theta_min(), hcSet.SHMS_theta_max()}, "theta_shms");
  IPSV P_HMS_psv({hcSet.HMS_P_min(), hcSet.HMS_P_max()}, "P_hms");
  IPSV P_SHMS_psv({hcSet.SHMS_P_min(), hcSet.SHMS_P_max()}, "P_shms");

  auto diff_spectrometers  = make_diff(P_HMS_psv,  theta_HMS_psv,  phi_HMS_psv, 
                                       P_SHMS_psv, theta_SHMS_psv, phi_SHMS_psv);
  auto ps_spectrometers    = make_phase_space(diff_spectrometers);
  diff_spectrometers.Print();

  // ------------------------------
  // Prepare the initial state
  double P1 = 11.0;
  double m1 = 0.000511;
  double E1 = std::sqrt(P1*P1+m1*m1);

  double P2 = 0.0;
  double m2 = M_p/GeV;
  double E2 = std::sqrt(P2*P2+m2*m2);

  InitialState init_state(P1, P2, m2);

  /** SIDIS diferential cross section.
   *
   *            d(sigma)
   *   --------------------------
   *   d(x)d(y)d(z)d(phi)d(phi_e)
   *   
   *   Note: "phi" is the angle between the leptonic and hadronic planes. 
   *         See http://inspirehep.net/record/677636 for details.
   *         "phi_e" is the scattered electron's azimuthal angle.
   *
   */
  auto SIDIS_xs_func = [&](const InitialState& is, const std::array<double,6>& x){
    using namespace insane::units;
    static PDFs   pdf;
    static FragFs ffs;
    static double Mh = M_pion/GeV;

    SIDISKinematics kine(is.p1().E(), Mh, x, SPEC_vars());
    if(!(kine.isValid)) return 0.0;
    //return 1.0;

    // Gaussian k_perp and p_perp distributions for PT dependence
    double kperp2_mean = 0.28; // GeV^2
    double pperp2_mean = 0.25; // GeV^2
    double PT_mean     = kine.z * kine.z * kperp2_mean + pperp2_mean;
    double PT2         = kine.P_hPerp * kine.P_hPerp;
    auto   f_PT        = std::exp(-PT2 / PT_mean) / (PT_mean * pi);
    auto   t1          = kine.SIDIS_xs_term() * f_PT;
    double FF          = 0.0;
    auto   pdf_vals    = pdf.Calculate(kine.x, kine.Q2);

    FF += PartonCharge2[q_id(Parton::u)] * ffs.D_u_piplus(kine.z, kine.Q2) *
          pdf_vals[q_id(Parton::u)];
    FF += PartonCharge2[q_id(Parton::ubar)] * ffs.D_ubar_piplus(kine.z, kine.Q2) *
          pdf_vals[q_id(Parton::ubar)];
    FF += PartonCharge2[q_id(Parton::d)] * ffs.D_d_piplus(kine.z, kine.Q2) *
          pdf_vals[q_id(Parton::d)];
    FF += PartonCharge2[q_id(Parton::dbar)] * ffs.D_dbar_piplus(kine.z, kine.Q2) *
          pdf_vals[q_id(Parton::dbar)];
    FF *= kine.x;

    /// \f$ F_{UU,T} = x \sum e_a^2 f_1^a(x) D_1^a(z) \f$
    //double res = Elastic_XS({GE2, GM2}, kine);
    //res        = res * hbarc2_gev_nb * kine.sin_th;
    return(t1*FF);
  };

  // -------------------------------------------------------
  //
  auto Eprime_and_Ph_func = [](const InitialState& is, const std::array<double, 6>& x)
  {
    using namespace insane::units;
    using namespace ROOT::Math;
    double Mh = M_pion/GeV;
    //SIDISKinematics kine(is.p1().E(), Mh, x, SIDIS_vars());
    SIDISKinematics kine(is.p1().E(), Mh, x, SPEC_vars());

    XYZVector l2(Polar3D<double>(kine.P_l2, kine.theta_l2, kine.phi_l2));
    XYZVector p_h(Polar3D<double>(kine.P_p2, kine.theta_p2, kine.phi_p2));
    //XYZVector p_h_lab(r1*r2*p_h);

    auto E_prime  = ROOT::Math::XYZTVector(l2.x(), l2.y(), l2.z(), kine.E_l2);
    auto P_hadron = ROOT::Math::XYZTVector(p_h.x(), p_h.y(), p_h.z(), kine.E_p2);
    return std::array<ROOT::Math::XYZTVector,2>({E_prime,P_hadron});
  };

  // returns SIDIS_vars using the SPEC_vars (measured by spectrometers)
  auto to_sidis_variables = [=](const InitialState& is, const std::array<double,6>& x){
    using namespace insane::units;
    using namespace ROOT::Math;
    double Mh = M_pion/GeV;
    SIDISKinematics kine(is.p1().E(), Mh, x, SIDIS_vars());
    //kine.Print();
    std::array<double,6> res = {kine.x, kine.y, kine.z, kine.P_hPerp, kine.phi_h, kine.phi_q};
    return res;
  };

  auto to_spectrometer_variables = [=](const InitialState& is, const std::array<double,6>& x){
    using namespace insane::units;
    using namespace ROOT::Math;
    double Mh = M_pion/GeV;
    SIDISKinematics kine(is.p1().E(), Mh, x, SPEC_vars());
    std::array<double,6> res = {kine.x, kine.y, kine.z, kine.P_hPerp, kine.phi_h, kine.phi_q};
    return res;
  };
  
  //auto to_sidis_tup = std::make_tuple(
  //        [&](const InitialState& is, const std::array<double,6>& x){ return std::get<0>(to_sidis_variables(is,x));},
  //        [&](const InitialState& is, const std::array<double,6>& x){ return std::get<1>(to_sidis_variables(is,x));},
  //        [&](const InitialState& is, const std::array<double,6>& x){ return std::get<2>(to_sidis_variables(is,x));},
  //        [&](const InitialState& is, const std::array<double,6>& x){ return std::get<3>(to_sidis_variables(is,x));},
  //        [&](const InitialState& is, const std::array<double,6>& x){ return std::get<4>(to_sidis_variables(is,x));},
  //        [&](const InitialState& is, const std::array<double,6>& x){ return std::get<5>(to_sidis_variables(is,x));}
  //                                   );

   
  //IPSV phi_SHMS_psv({hc_setting.SHMS_phi_min(), hc_setting.SHMS_phi_max()}, "phi_shms");
  //IPSV theta_HMS_psv( {hc_setting.HMS_theta_min(),  hc_setting.HMS_theta_max()}, "theta_hms");
  //IPSV theta_SHMS_psv({hc_setting.SHMS_theta_min(), hc_setting.SHMS_theta_max()}, "theta_shms");
  //IPSV P_HMS_psv( {hc_setting.HMS_P_min(),  hc_setting.HMS_P_max()},  "P_hms");
  //IPSV P_SHMS_psv({hc_setting.SHMS_P_min(), hc_setting.SHMS_P_max()}, "P_shms");
  // ---------------------------------------------------
  // cross section differential in x y z PT phi_h phi_q 
  //auto ps_spectrometer    = make_phase_space( diff_spectrometer );//,
      //make_PSV("P_hms", {-pi, pi},//hc_setting.HMS_P_min(), hc_setting.HMS_P_max()},
      //         [&](const InitialState& is, const std::array<double, 6>& x) {
      //           return std::get<0>(to_spectrometer_variables(is, x));
      //         }),
      //make_PSV("theta_hms", {hc_setting.HMS_theta_min(), hc_setting.HMS_theta_max()},
      //         [&](const InitialState& is, const std::array<double, 6>& x) {
      //           return std::get<1>(to_spectrometer_variables(is, x));
      //         }));
      //make_PSV("phi_hms", {hc_setting.HMS_phi_min(), hc_setting.HMS_phi_max()},
      //         [&](const InitialState& is, const std::array<double, 6>& x) {
      //           return std::get<2>(to_spectrometer_variables(is, x));
      //         }),
      //make_PSV("P_shms", {hc_setting.SHMS_P_min(), hc_setting.SHMS_P_max()},
      //         [&](const InitialState& is, const std::array<double, 6>& x) {
      //           return std::get<3>(to_spectrometer_variables(is, x));
      //         }),
      //make_PSV("theta_shms", {hc_setting.SHMS_theta_min(), hc_setting.SHMS_theta_max()},
      //         [&](const InitialState& is, const std::array<double, 6>& x) {
      //           return std::get<4>(to_spectrometer_variables(is, x));
      //         }),
      //make_PSV("phi_shms", {hc_setting.SHMS_phi_min(), hc_setting.SHMS_phi_max()},
      //         [&](const InitialState& is, const std::array<double, 6>& x) {
      //           return std::get<5>(to_spectrometer_variables(is, x));
      //         }));
  //ps_dxyzPTphiphie.Print();
  //
  auto diff_xs_sidis       = make_diff_cross_section(ps_spectrometers, SIDIS_xs_func);

  // Transform to a function of the 2 measured momenta
  //auto jm_spec_dxyzPTphiphie = make_jacobian(diff_dxyzPTphiphie, 
  //                                           diff_spectrometers,
  //                                           to_sidis_tup);

  //auto xs_spectrometers = make_jacobian_transformed_xs(diff_xs_sidis, 
  //                                                     init_state,
  //                                                     jm_spec_dxyzPTphiphie, 
  //                                                     ps_spectrometers);

  //// ----------------------------
  //
  auto sampler       = make_ps_sampler(make_integrated_cross_section(init_state, diff_xs_sidis));

  auto fs_parts = make_final_state_particles({11, 211}, {0.000511, M_pion / GeV},
                                             Eprime_and_Ph_func);

  auto fs_SIDIS = make_final_state(ps_spectrometers, fs_parts);
  //auto fs_SIDIS = make_final_state(ps_spectrometers, fs_parts);

  auto evgen    = make_event_generator(sampler, fs_SIDIS);
  auto total_XS = evgen.Init();

  //// --------------------------
  //
  std::cout << "Total cross section = " << total_XS  << " nb\n";
  int    N_sim      = 100000;

  using namespace insane;
  double Loop1_LD2_Lumi = materials::ComputeLuminosityPerN({1, 2}, 
                                                 materials::LD2_density /*g/cm3*/,
                                                 10.0 /*cm*/, 
                                                 50.0e-6);
  double Loop1_entrance_window_thickness = 0.104;
  double Loop1_exit_window_thickness     = 0.133;
  double Loop3_entrance_window_thickness = 0.130;
  double Loop3_exit_window_thickness     = 0.188;

  double Loop1_windows_Lumi = materials::ComputeLuminosityPerN({13, 27}, 
                                                               materials::Al_density ,
                                                               Loop1_entrance_window_thickness  +
                                                               Loop1_exit_window_thickness     ,
                                                               50.0e-6);

  double Lumi = Loop1_LD2_Lumi + Loop1_windows_Lumi;

  double total_cross_section = total_XS * nanobarn;
  double luminosity          = Lumi / cm2;
  double rate                = total_cross_section * luminosity;
  std::cout << "Total XS    : " << total_XS  << " nb\n";
  std::cout << "            : " << total_cross_section/microbarn  << " ub\n";
  std::cout << "            : " << total_cross_section/barn  << " b\n";
  std::cout << "Luminosity  : " << Lumi               << " 1/cm2 s\n";
  std::cout << " [LD2]        " << Loop1_LD2_Lumi     << " 1/cm2 s\n";
  std::cout << " [windows]    " << Loop1_windows_Lumi << " 1/cm2 s\n";
  std::cout << "Rate        : " << rate << " 1/s\n";

  std::vector<double> res   = {total_XS, rate};

  return res;

  // --------------------------
  //
  //TFile* f  = new TFile("eg_output_sidis_pion.root","RECREATE");
  //RDataFrame d1(N_sim);

  //auto gen_func = [&](){ 
  //  return evgen();
  //};
  //using gen_func_t = insane::helpers::result_t<decltype(gen_func)>;

  //auto gen_func2 = [&](gen_func_t data){ 
  //  auto [vars, parts, pdgs] = data;
  //  auto res = to_vector(parts);
  //  //std::transform(res.begin(), res.end(), res.begin(),[&](auto v){return boost_to_lab(v);});
  //  return res;
  //};
  //using gen_func2_t = typename insane::helpers::result_t<decltype(gen_func2)>;

  //auto gen_func_nucleon_rest = [&](gen_func_t data){ 
  //  auto [vars, parts, pdgs] = data;
  //  auto res = to_vector(parts);
  //  return res;
  //};
  //using gen_func_nucleon_rest_t = typename insane::helpers::result_t<decltype(gen_func_nucleon_rest)>;

  //auto gen_func3 = [&](gen_func_t data){ 
  //  auto [vars, parts, pdgs] = data;
  //  return to_vector(vars);
  //};
  //using gen_func3_t = typename insane::helpers::result_t<decltype(gen_func3)>;

  //// Generate the event returning the tuple containing 
  //// [thrown variables, final_state, pdg codes]
  //auto d2  = d1.Define("EvGen", gen_func , {})
  //             .Define("Pgen",        gen_func2 , {"EvGen"});
  //             //.Define("Prest_frame", gen_func_nucleon_rest , {"EvGen"});

  ////// Create leaf for thrown variables
  ////auto d4  = d3.Define("vars", gen_func3 , {"EvGen"})
  ////.Define("Q2",  [](const gen_func3_t& vars){ return vars[0];}, {"vars"})
  ////.Define("phi",  [](const gen_func3_t& vars){ return vars[1];}, {"vars"})
  ////.Define("Q20",
  ////        [&](const gen_func2_t& parts ){ 
  ////          auto   q      = init_state_EIC.p1()-parts.at(0);
  ////          double Q2_ev  = -1.0*(q.Dot(q));
  ////          return Q2_ev ;
  ////        },
  ////        {"Pgen"})
  ////.Define("Delta",
  ////        [&](const gen_func2_t& parts ){ 
  ////          auto   Delta = init_state_EIC.p2()-parts.at(1);
  ////          return Delta ;
  ////        }, {"Pgen"})
  ////.Define("u",
  ////        [&](const gen_func2_t& parts ){ 
  ////          auto   delta = init_state_EIC.p1()-parts.at(1);
  ////          return delta.Dot(delta) ;
  ////        }, {"Pgen"})
  ////.Define("Delta_Long",
  ////        [&](const gen_func2_t& parts ){ 
  ////          auto   Delta = init_state_EIC.p2()-parts.at(1);
  ////          return Delta.pz()/(init_state_EIC.p2().pz());
  ////        }, {"Pgen"})
  ////.Define("Delta_p_over_p",
  ////        [&](const gen_func2_t& parts ){ 
  ////          auto   Delta = init_state_EIC.p2()-parts.at(1);
  ////          return Delta.P()/(init_state_EIC.p2().P());
  ////        }, {"Pgen"})
  ////.Define("t",
  ////        [&](const gen_func2_t& parts ){ 
  ////          auto   Delta = init_state_EIC.p2()-parts.at(1);
  ////          double t_ev  = (Delta.Dot(Delta));
  ////          return t_ev ;
  ////        }, {"Pgen"})
  ////.Define("theta_e", 
  ////        [&](const gen_func2_t& parts ){return parts.at(0).Theta()/degree; }, 
  ////        {"Pgen"})
  ////.Define("P_e", 
  ////        [&](const gen_func2_t& parts ){return parts.at(0).P(); }, 
  ////        {"Pgen"})
  ////.Define("theta_p", 
  ////        [&](const gen_func2_t& parts ){return parts.at(1).Theta()/degree; }, 
  ////        {"Pgen"})
  ////.Define("P_p", 
  ////        [&](const gen_func2_t& parts ){return parts.at(1).P(); }, 
  ////        {"Pgen"})
  ////.Define("tau", [&](const double& Qsq ){return Qsq/(4.0*0.938*0.938); }, {"Q2"})
  ////.Define("theta_e_Rest",
  ////        [&](const gen_func_nucleon_rest_t& P_rest_frame ){
  ////          double theta_e = pi - P_rest_frame.at(0).Theta();
  ////          return theta_e/degree; },
  ////          {"Prest_frame"})
  ////.Define("epsilon",
  ////        [&](const double& tau, const gen_func_nucleon_rest_t& P_rest_frame ){
  ////          double theta_e = pi - P_rest_frame.at(0).Theta();
  ////          double den     = 1.0+2.0*(1.0+tau)*std::pow(std::tan(theta_e/2.0),2) ;
  ////          return 1.0/den;
  ////        }, {"tau","Prest_frame"})
  ////.Define("MinusEpsilon",
  ////        [&](const double& tau, const gen_func_nucleon_rest_t& P_rest_frame ){
  ////          double theta_e = pi - P_rest_frame.at(0).Theta();
  ////          double den     = 1.0+2.0*(1.0+tau)*std::pow(std::tan(theta_e/2.0),2) ;
  ////          return 1.0-1.0/den;
  ////        }, {"tau","Prest_frame"})
  ////.Define("Aperp", 
  ////        [&](const double& tau, const double& theta_e, const double& eps){
  ////          double GEoverGM = (1.0/2.79);
  ////          double num = -2.0*std::sqrt(tau*(1.0+tau))*std::tan(theta_e*degree/2.0)*GEoverGM;
  ////          double den = GEoverGM*GEoverGM + tau/eps;
  ////          return num/den;
  ////        },
  ////        {"tau","theta_e_Rest","epsilon"})
  ////.Define("Asym", 
  ////        [&](const gen_func2_t& parts, const double& th_e, const double& Q2){ 
  ////          static insane::physics::AMTFormFactors ffs;
  ////          XYZVector Ptarg{1,0,0};
  ////          auto   n_0   = init_state_REST.p1().Vect().Cross(parts.at(0).Vect()).Unit();
  ////          auto   q     = init_state_REST.p1()-parts.at(0);
  ////          auto   q3    = q.Vect();
  ////          auto   z_q   = q.Vect().Unit();
  ////          auto   u_x   = n_0.Cross(z_q).Unit();
  ////          auto   Pxy   = Ptarg - (Ptarg.Dot(z_q))*z_q;
  ////          auto   th_star  = VectorUtil::Angle(Ptarg, z_q);
  ////          auto   phi_star = VectorUtil::Angle(Pxy, u_x);
  ////          return insane::physics::A_elastic(ffs, Q2, th_e*degree, th_star, phi_star);
  ////        }, {"Prest_frame","theta_e_Rest","Q2"})
  ////.Define("r_p",
  ////        [&](const double& th_p ){return z_detector*std::sin(th_p*degree);},
  ////        {"theta_p"});

  ////auto d5 = d4.Filter(
  ////  [](double A){
  ////    if ( std::abs(A) >0.1 ){
  ////      return true;
  ////    }
  ////    return false;
  ////  },{"Asym"});

  ////auto d6 = d4.Filter(
  ////  [](double pr){
  ////    if ( std::abs(pr - 0.01) < 0.005 ){
  ////      return true;
  ////    }
  ////    return false;
  ////  }, {"r_p"});

  ////// --------------------------
  ////d4.Snapshot("elastic_ep", "eg_output_elastic_ep.root");
  //d2.Snapshot("pion_sidis", "eg_output_sidis_pion.root");


  //////auto   tot_count_wCut     = d6.Count();
  ////auto   tot_count_wCut     = d5.Count();
  ////double total_detected     = *tot_count_wCut;
  ////double day_integrated_lum = 100.0/116.0; // 1/fb per day
  ////double count_per_day      = total_XS*day_integrated_lum;
  ////double count_per_second   = count_per_day/(24.0*60.0*60.0);
  ////double simulated_time     = double(N_sim)/count_per_second;
  ////double meas_rate_in_bin   = total_detected/simulated_time;

  ////std::cout << " count_per_second = " << count_per_second << " Hz\n";
  ////std::cout << " total_detected   = " << total_detected   << "\n";
  ////std::cout << " stat_unc for 1s  = " << std::sqrt(count_per_second)/count_per_second << " \%\n";
  ////std::cout << " simulated_time   = " << simulated_time   << " s\n";
  ////std::cout << " meas_rate_in_bin = " << meas_rate_in_bin << " Hz\n";
  ////std::cout << " meas_unc for 1s  = " << std::sqrt(meas_rate_in_bin)/meas_rate_in_bin << " \%\n";
  ////std::cout << " meas_unc for 1m  = " << std::sqrt(60.0*meas_rate_in_bin)/(60.0*meas_rate_in_bin) << " \%\n";
  ////std::cout << " z_detector       = " << z_detector << " m\n";

  //f->Close();
}

