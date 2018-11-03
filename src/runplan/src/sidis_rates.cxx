#include "sidis_rates.h"
#include "runplan/CSV_settings.h"
#include <iostream>

#include "Math/Vector3Dfwd.h"
#include "Math/Vector4Dfwd.h"
#include "Math/Transform3D.h"
#include "Math/Rotation3D.h"
#include "Math/RotationY.h"
#include "Math/RotationZ.h"
#include "ROOT/TFuture.hxx"

#include "TH1F.h"
#include "TFile.h"
#include "TTree.h"
#include "TBufferJSON.h"
#include "ROOT/RDataFrame.hxx"

// insane libraries
#include "InSANE/MaterialProperties.h"
#include "InSANE/Luminosity.h"
#include "InSANE/PhaseSpaceVariables.h"
#include "InSANE/PSSampler.h"
#include "InSANE/NFoldDifferential.h"
#include "InSANE/Jacobians.h"
#include "InSANE/DiffCrossSection.h"
#include "InSANE/FinalState.h"
#include "InSANE/Helpers.h"
#include "InSANE/Physics.h"
#include "InSANE/Kinematics.h"
#include "InSANE/Stat2015_UPDFs.h"
#include "InSANE/DSSFragmentationFunctions.h"

#include <fmt/core.h>
#include <fmt/ostream.h>
R__LOAD_LIBRARY(libfmt.so)

using namespace ROOT;
using namespace insane::physics;
using namespace insane::kinematics;
using namespace insane::units;
using namespace insane::helpers;
using namespace ROOT::Math;
using namespace ROOT::Experimental;

using PDFs           = insane::physics::Stat2015_UPDFs;
using FragFs         = insane::physics::DSSFragmentationFunctions;
using SIDIS_vars     = insane::kinematics::variables::SIDIS_x_y_z_phih_phie;
using SPEC_vars      = insane::kinematics::variables::SIDIS_eprime_ph;
using Q2_variable    = insane::kinematics::variables::MomentumTransfer;
using Theta_variable = insane::kinematics::variables::Theta;

namespace hallc {

  /** Calculates the table row entry.
   *
   *  Computes cross section, rate, and time  based on the desired statistics.
   */
  RunPlanTableEntry build_table_entry(const Kinematic& kine) {
    RunPlanTableEntry entry0;
    entry0.kinematic = kine;
    entry0.polarity  = 1;
    if(kine.pid_had < 0 ) {
      entry0.polarity  = -1;
    }
    //entry0.Ibeam     = 25.0;
    entry0.counts    = 30000;
    if (entry0.kinematic.z > 0.55) {
      entry0.counts = 20000;
    }
    sidis_config_xs_and_rates(entry0);
    double scale = 50.0 / entry0.Ibeam;
    entry0.time  = scale * (entry0.counts / entry0.rates.total_rate) / (60.0 * 60.0);
    RunPlanTableEntry::PrintHeader();
    entry0.Print();
    // if( use_LH2 ) {
    //  entry1.rate               = res_rate[3]*(entry1.Ibeam/50.0);
    //  entry1.sigma              = res_rate[4];
    //  entry1.time = entry1.counts/entry1.rate/(60.0*60.0);
    //}
    return entry0;
  }

  RunTable build_sidis_table(const csv::CSV_Settings& settings) {
    // The fortran code used for the fragmentation functions
    // doesn't allow multi threaded to run without modifying
    // the way it uses the common blocks
    ROOT::EnableImplicitMT(1);
    //{ std::ofstream json_ofile("tables/pion_rates.json", std::ios_base::trunc); }
   
    // Copy the settings
    auto csv_settings = settings;
    RunTable run_plan_table;

    for (auto& a_kinematic : csv_settings) {

      a_kinematic.Compute();

      auto plus_entry = ROOT::Experimental::Async([&a_kinematic]() {
        // pi plus
        RunPlanTableEntry entry0;
        entry0.kinematic = a_kinematic;
        entry0.polarity  = 1;
        entry0.Ibeam     = 25.0;
        entry0.counts    = 30000;
        if (entry0.kinematic.z > 0.55) {
          entry0.counts = 20000;
        }
        sidis_config_xs_and_rates(entry0);
        double scale = 50.0 / entry0.Ibeam;
        entry0.time  = scale * (entry0.counts / entry0.rates.total_rate) / (60.0 * 60.0);
        RunPlanTableEntry::PrintHeader();
        entry0.Print();
        return entry0;
      });

      auto minus_entry = ROOT::Experimental::Async([&a_kinematic]() {
        // pi minus
        RunPlanTableEntry entry0;
        entry0.kinematic = a_kinematic;
        entry0.polarity  = -1;
        entry0.Ibeam     = 50.0;
        entry0.counts    = 30000;
        if (entry0.kinematic.z > 0.55) {
          entry0.counts = 20000;
        }
        sidis_config_xs_and_rates(entry0);
        double scale = 50.0 / entry0.Ibeam;
        entry0.time  = scale * (entry0.counts / entry0.rates.total_rate) / (60.0 * 60.0);
        RunPlanTableEntry::PrintHeader();
        entry0.Print();
        // if( ! only_LH2 ) {
        //}
        // if( use_LH2 ) {
        //  RunPlanTableEntry  entry1 = entry0;
        //  entry1.use_LH2_target     = true;
        //  entry1.rate               = res_rate[3]*(entry1.Ibeam/50.0);
        //  entry1.sigma              = res_rate[4];
        //  entry1.time = entry1.counts/entry1.rate/(60.0*60.0);
        //  entry1.Print();
        //  resulting_entries.push_back( entry1 );
        //}
        return entry0;
      });

      run_plan_table.add(plus_entry.get());
      run_plan_table.add(minus_entry.get());
    }
    // all_tables.push_back(std::make_pair(setting_group.first, run_plan_table));
    // all_kaon_tables.push_back(std::make_pair(setting_group.first, run_plan_kaon_table));
    //}
    return run_plan_table;
  }

  void save_json_tables(const RunTable& table_rows, std::string table_name,
                   const std::ios_base::openmode& mode) {
    double        total_time = 0.0;
    std::ofstream ofs("tables/run_plan_table.txt", mode);
    // for(const auto& atable : all_tables){
    // ofs << "Q2 = " << atable.first << "\n";
    double Q2 = table_rows.GetVector().at(0).kinematic.Q2;
    double Q2group_total_time = 0.0;
    fmt::print(ofs, "{:^80}\n", fmt::format("Q2 = {:3.2f} GeV2", Q2));
    RunPlanTableEntry::PrintHeader(ofs);
    for (const auto& entry : table_rows.GetVector()) {
      entry.Print(ofs);
      total_time += entry.time;
      Q2group_total_time += entry.time;
    }
    fmt::print(ofs, "{:-<79s}\n", "-");
    ofs << fmt::format(" Time for Q2 = {:3.2f} GeV2 setting : ", Q2)
        << fmt::format("{:6.3f} hrs or {:6.3f} days\n", Q2group_total_time,
                       Q2group_total_time / 24.0);
    ofs << "\n";
    //}
    fmt::print(ofs, "{:=<79s}\n", "=");
    ofs << "      total time: " << total_time << " hours or " << total_time / 24.0 << " days\n";

    std::ofstream json_ofile("tables/run_plan_table.json", std::ios_base::trunc);
    //json_ofile << TBufferJSON::ToJSON(&all_tables);
    json_ofile.close();
  }

  void save_tables(const RunTable& table_rows, std::string table_name,
                   const std::ios_base::openmode& mode) {
    {
      double        total_time = 0.0;
      std::ofstream ofs(table_name + ".txt", std::ios_base::app);
      std::ofstream wiki_file(table_name + ".wiki", std::ios_base::app);
      // for (const auto& atable : all_tables) {
      // ofs << "Q2 = " << atable.first << "\n";
      double Q2group_total_time = 0.0;

      double Q2 = table_rows.GetVector().at(0).kinematic.Q2;

      fmt::print(ofs, "{:^80}\n", fmt::format("Q2 = {:3.2f} GeV2", Q2));
      fmt::print(wiki_file, "{:^80}\n", fmt::format("Q2 = {:3.2f} GeV2", Q2));

      RunPlanTableEntry::PrintHeader(ofs);
      RunPlanTableEntry::PrintWikiHeader(wiki_file);
      for (const auto& entry : table_rows.GetVector()) {
        if (std::abs(entry.polarity) >= 2) {
          continue;
        }
        entry.Print(ofs);
        entry.PrintWiki(wiki_file);
        total_time += entry.time;
        Q2group_total_time += entry.time;
      }
      RunPlanTableEntry::PrintWikiFooter(wiki_file);

      fmt::print(ofs, "{:-<79s}\n", "-");
      fmt::print(wiki_file, "{:-<79s}\n", "-");
      ofs << fmt::format(" Time for Q2 = {:3.2f} GeV2 setting : ", Q2)
          << fmt::format("{:6.3f} hrs or {:6.3f} days\n", Q2group_total_time,
                         Q2group_total_time / 24.0)
          << "\n";
      wiki_file << fmt::format(" Time for Q2 = {:3.2f} GeV2 setting : ", Q2)
                << fmt::format("{:6.3f} hrs or {:6.3f} days\n", Q2group_total_time,
                               Q2group_total_time / 24.0)
                << "\n";
      //}
      // fmt::print(ofs, "{:=<79s}\n", "=");
      // ofs << "      total time: " << total_time << " hours or " << total_time / 24.0 << "
      // days\n"; fmt::print(wiki_file, "{:=<79s}\n", "="); wiki_file << "      total time: " <<
      // total_time << " hours or " << total_time / 24.0
      //          << " days\n";

      // std::ofstream json_ofile("tables/LD2_run_plan_table.json", std::ios_base::trunc);
      // json_ofile << TBufferJSON::ToJSON(&all_tables);
      // json_ofile.close();
    }
    //{
    //  double        total_time = 0.0;
    //  std::ofstream ofs("tables/LD2_run_plan_kaon_table.txt", std::ios_base::trunc);
    //  std::ofstream wiki_file("tables/LD2_run_plan_kaon_table.wiki", std::ios_base::trunc);
    //  for (const auto& atable : all_kaon_tables) {
    //    // ofs << "Q2 = " << atable.first << "\n";
    //    double Q2group_total_time = 0.0;
    //    fmt::print(ofs, "{:^80}\n", fmt::format("Q2 = {:3.2f} GeV2", atable.first));
    //    fmt::print(wiki_file, "{:^80}\n", fmt::format("Q2 = {:3.2f} GeV2", atable.first));
    //    RunPlanTableEntry::PrintHeader(ofs);
    //    RunPlanTableEntry::PrintWikiHeader(wiki_file);
    //    for (const auto& entry : atable.second) {
    //      entry.Print(ofs);
    //      entry.PrintWiki(wiki_file);
    //      total_time += entry.time;
    //      Q2group_total_time += entry.time;
    //    }
    //    RunPlanTableEntry::PrintWikiFooter(wiki_file);
    //    fmt::print(ofs, "{:-<79s}\n", "-");
    //    ofs << fmt::format(" Time for Q2 = {:3.2f} GeV2 setting : ", atable.first)
    //        << fmt::format("{:6.3f} hrs or {:6.3f} days\n", Q2group_total_time,
    //                       Q2group_total_time / 24.0);
    //    ofs << "\n";
    //  }
    //  fmt::print(ofs, "{:=<79s}\n", "=");
    //  ofs << "      total time: " << total_time << " hours or " << total_time / 24.0 << " days\n";

    //  std::ofstream json_ofile("tables/LD2_run_plan_kaon_table.json", std::ios_base::trunc);
    //  json_ofile << TBufferJSON::ToJSON(&all_tables);
    //  json_ofile.close();
    //}
    //{
    //  double total_time = 0.0;
    //  std::ofstream ofs("tables/run_plan_kaon_table.txt",std::ios_base::trunc);
    //  for(const auto& atable : all_kaon_tables){
    //    //ofs << "Q2 = " << atable.first << "\n";
    //    double Q2group_total_time = 0.0;
    //    fmt::print(ofs, "{:^80}\n", fmt::format("Q2 = {:3.2f} GeV2",atable.first));
    //    RunPlanTableEntry::PrintHeader(ofs);
    //    for(const auto& entry : atable.second){
    //      entry.Print(ofs);
    //      total_time += entry.time;
    //      Q2group_total_time += entry.time;
    //    }
    //    fmt::print(ofs, "{:-<79s}\n", "-");
    //    ofs << fmt::format(" Time for Q2 = {:3.2f} GeV2 setting : ", atable.first)
    //    << fmt::format("{:6.3f} hrs or {:6.3f} days\n", Q2group_total_time,
    //                   Q2group_total_time / 24.0);
    //    ofs << "\n";
    //  }
    //  fmt::print(ofs, "{:=<79s}\n", "=");
    //  ofs << "      total time: " << total_time << " hours or " << total_time/24.0 << " days\n";

    //  std::ofstream json_ofile("tables/run_plan_kaon_table.json",std::ios_base::trunc);
    //  json_ofile << TBufferJSON::ToJSON(&all_tables);
    //  json_ofile.close();
    //}
  }

  /** Compute the cross section and rate for a given setting
   *
   */
  std::vector<double> sidis_config_xs_and_rates(RunPlanTableEntry& run_setting) {

    hallc::Kinematic&    kine_set = run_setting.kinematic;
    hallc::HallCSetting& hcSet    = run_setting.hcSet;
    double               x_set    = kine_set.x;
    double               z_set    = kine_set.z;
    double               nu_set   = kine_set.nu;
    double               W_set    = kine_set.W;
    double               Wp_set   = kine_set.Wp;
    int                  polarity = run_setting.polarity;
    hcSet.HMS_theta               = kine_set.th_e * degree;
    hcSet.SHMS_theta              = kine_set.th_q * degree;
    hcSet.HMS_p0                  = kine_set.Ee;
    hcSet.SHMS_p0                 = kine_set.Ppi;

    // SHMS 4 msr dp -10 +20
    // HMS  6 msr dp +-8
    // -------------------------------------------------------
    // Phase space variables
    //hcSet.SHMS_dP_high = -0.05;
    IPSV lowdp_phi_SHMS_psv({hcSet.SHMS_phi_min(), hcSet.SHMS_phi_max()}, "phi_shms");
    IPSV lowdp_phi_HMS_psv({hcSet.HMS_phi_min(), hcSet.HMS_phi_max()}, "phi_hms");
    IPSV lowdp_theta_HMS_psv({hcSet.HMS_theta_min(), hcSet.HMS_theta_max()}, "theta_hms");
    IPSV lowdp_theta_SHMS_psv({hcSet.SHMS_theta_min(), hcSet.SHMS_theta_max()}, "theta_shms");
    IPSV lowdp_P_HMS_psv({hcSet.HMS_P_min(), hcSet.HMS_P_max()}, "P_hms");
    IPSV lowdp_P_SHMS_psv({hcSet.SHMS_P_min(), hcSet.SHMS_P_max()}, "P_shms");

    
    //hcSet.SHMS_dP_high = hallc::shms::SHMS_dP_high;
    //hcSet.SHMS_dP_low = -0.05;
    IPSV phi_SHMS_psv({hcSet.SHMS_phi_min(), hcSet.SHMS_phi_max()}, "phi_shms");
    IPSV phi_HMS_psv({hcSet.HMS_phi_min(), hcSet.HMS_phi_max()}, "phi_hms");
    IPSV theta_HMS_psv({hcSet.HMS_theta_min(), hcSet.HMS_theta_max()}, "theta_hms");
    IPSV theta_SHMS_psv({hcSet.SHMS_theta_min(), hcSet.SHMS_theta_max()}, "theta_shms");
    IPSV P_HMS_psv({hcSet.HMS_P_min(), hcSet.HMS_P_max()}, "P_hms");
    IPSV P_SHMS_psv({hcSet.SHMS_P_min(), hcSet.SHMS_P_max()}, "P_shms");

    hcSet.SHMS_dP_low = hallc::shms::SHMS_dP_low;

    auto diff_spectrometers =
        make_diff(P_HMS_psv, theta_HMS_psv, phi_HMS_psv, P_SHMS_psv, theta_SHMS_psv, phi_SHMS_psv);
    auto ps_spectrometers = make_phase_space(diff_spectrometers);

    auto diff_spectrometers_lowdp =
        make_diff(lowdp_P_HMS_psv, lowdp_theta_HMS_psv, lowdp_phi_HMS_psv, lowdp_P_SHMS_psv, lowdp_theta_SHMS_psv, lowdp_phi_SHMS_psv);
    auto ps_spectrometers_lowdp = make_phase_space(diff_spectrometers_lowdp);
    //diff_spectrometers.Print();
    //diff_spectrometers_lowdp.Print();

    // ------------------------------
    // Prepare the initial state
    double P1 = kine_set.E0;
    double m1 = 0.000511;
    double E1 = std::sqrt(P1 * P1 + m1 * m1);

    double P2 = 0.0;
    double m2 = M_p / GeV;
    double E2 = std::sqrt(P2 * P2 + m2 * m2);

    InitialState init_state(P1, P2, m2);

    int    xs_select = 1;
    PDFs   pdf;
    FragFs ffs;

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
    auto SIDIS_xs_func = [&](const InitialState& is, const std::array<double, 6>& x) {
      using namespace insane::units;
      double Mh = M_pion / GeV;

      SIDISKinematics kine(is.p1().E(), Mh, x, SPEC_vars());
      if (!(kine.isValid))
        return 0.0;

      // Gaussian k_perp and p_perp distributions for PT dependence
      double kperp2_mean = 0.28; // GeV^2
      double pperp2_mean = 0.25; // GeV^2
      double PT_mean     = kine.z * kine.z * kperp2_mean + pperp2_mean;
      double PT2         = kine.P_hPerp * kine.P_hPerp;
      auto   f_PT        = std::exp(-PT2 / PT_mean) / (PT_mean * pi);
      auto   t1          = kine.SIDIS_xs_term() * f_PT;
      double FF          = 0.0;
      auto   pdf_vals    = pdf.Calculate(kine.x, kine.Q2);

      if (polarity == 1) {
        // pi+
        if (xs_select == 1 || xs_select == 2) {
          // proton
          FF += PartonCharge2[q_id(Parton::u)] * ffs.D_u_piplus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::u)];
          FF += PartonCharge2[q_id(Parton::ubar)] * ffs.D_ubar_piplus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::ubar)];
          FF += PartonCharge2[q_id(Parton::d)] * ffs.D_d_piplus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::d)];
          FF += PartonCharge2[q_id(Parton::dbar)] * ffs.D_dbar_piplus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::dbar)];
          FF += PartonCharge2[q_id(Parton::s)] * ffs.D_s_piplus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::s)];
          FF += PartonCharge2[q_id(Parton::sbar)] * ffs.D_sbar_piplus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::sbar)];
        }
        if (xs_select == 0 || xs_select == 2) {
          // neutron
          FF += PartonCharge2[q_id(Parton::u)] * ffs.D_u_piplus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::d)];
          FF += PartonCharge2[q_id(Parton::ubar)] * ffs.D_ubar_piplus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::dbar)];
          FF += PartonCharge2[q_id(Parton::d)] * ffs.D_d_piplus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::u)];
          FF += PartonCharge2[q_id(Parton::dbar)] * ffs.D_dbar_piplus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::ubar)];
          FF += PartonCharge2[q_id(Parton::s)] * ffs.D_s_piplus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::s)];
          FF += PartonCharge2[q_id(Parton::sbar)] * ffs.D_sbar_piplus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::sbar)];
        }
      } else if (polarity == -1) {
        // pi-
        if (xs_select == 1 || xs_select == 2) {
          FF += PartonCharge2[q_id(Parton::u)] * ffs.D_u_piminus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::u)];
          FF += PartonCharge2[q_id(Parton::ubar)] * ffs.D_ubar_piminus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::ubar)];
          FF += PartonCharge2[q_id(Parton::d)] * ffs.D_d_piminus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::d)];
          FF += PartonCharge2[q_id(Parton::dbar)] * ffs.D_dbar_piminus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::dbar)];
          FF += PartonCharge2[q_id(Parton::s)] * ffs.D_s_piminus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::s)];
          FF += PartonCharge2[q_id(Parton::sbar)] * ffs.D_sbar_piminus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::sbar)];
        }
        if (xs_select == 0 || xs_select == 2) {
          // neutron
          FF += PartonCharge2[q_id(Parton::u)] * ffs.D_u_piminus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::d)];
          FF += PartonCharge2[q_id(Parton::ubar)] * ffs.D_ubar_piminus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::dbar)];
          FF += PartonCharge2[q_id(Parton::d)] * ffs.D_d_piminus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::u)];
          FF += PartonCharge2[q_id(Parton::dbar)] * ffs.D_dbar_piminus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::ubar)];
          FF += PartonCharge2[q_id(Parton::s)] * ffs.D_s_piminus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::s)];
          FF += PartonCharge2[q_id(Parton::sbar)] * ffs.D_sbar_piminus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::sbar)];
        }
      } else if (polarity == 2) {
        // pi+
        if (xs_select == 1 || xs_select == 2) {
          // proton
          FF += PartonCharge2[q_id(Parton::u)] * ffs.D_u_Kplus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::u)];
          FF += PartonCharge2[q_id(Parton::ubar)] * ffs.D_ubar_Kplus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::ubar)];
          FF += PartonCharge2[q_id(Parton::d)] * ffs.D_d_Kplus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::d)];
          FF += PartonCharge2[q_id(Parton::dbar)] * ffs.D_dbar_Kplus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::dbar)];
          FF += PartonCharge2[q_id(Parton::s)] * ffs.D_s_Kplus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::s)];
          FF += PartonCharge2[q_id(Parton::sbar)] * ffs.D_sbar_Kplus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::sbar)];
        }
        if (xs_select == 0 || xs_select == 2) {
          // neutron
          FF += PartonCharge2[q_id(Parton::u)] * ffs.D_u_Kplus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::d)];
          FF += PartonCharge2[q_id(Parton::ubar)] * ffs.D_ubar_Kplus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::dbar)];
          FF += PartonCharge2[q_id(Parton::d)] * ffs.D_d_Kplus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::u)];
          FF += PartonCharge2[q_id(Parton::dbar)] * ffs.D_dbar_Kplus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::ubar)];
          FF += PartonCharge2[q_id(Parton::s)] * ffs.D_s_Kplus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::s)];
          FF += PartonCharge2[q_id(Parton::sbar)] * ffs.D_sbar_Kplus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::sbar)];
        }
      } else if (polarity == -2) {
        // pi-
        if (xs_select == 1 || xs_select == 2) {
          FF += PartonCharge2[q_id(Parton::u)] * ffs.D_u_Kminus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::u)];
          FF += PartonCharge2[q_id(Parton::ubar)] * ffs.D_ubar_Kminus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::ubar)];
          FF += PartonCharge2[q_id(Parton::d)] * ffs.D_d_Kminus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::d)];
          FF += PartonCharge2[q_id(Parton::dbar)] * ffs.D_dbar_Kminus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::dbar)];
          FF += PartonCharge2[q_id(Parton::s)] * ffs.D_s_Kminus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::s)];
          FF += PartonCharge2[q_id(Parton::sbar)] * ffs.D_sbar_Kminus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::sbar)];
        }
        if (xs_select == 0 || xs_select == 2) {
          // neutron
          FF += PartonCharge2[q_id(Parton::u)] * ffs.D_u_Kminus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::d)];
          FF += PartonCharge2[q_id(Parton::ubar)] * ffs.D_ubar_Kminus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::dbar)];
          FF += PartonCharge2[q_id(Parton::d)] * ffs.D_d_Kminus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::u)];
          FF += PartonCharge2[q_id(Parton::dbar)] * ffs.D_dbar_Kminus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::ubar)];
          FF += PartonCharge2[q_id(Parton::s)] * ffs.D_s_Kminus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::s)];
          FF += PartonCharge2[q_id(Parton::sbar)] * ffs.D_sbar_Kminus(kine.z, kine.Q2) *
                pdf_vals[q_id(Parton::sbar)];
        }
      }
      FF *= kine.x;

      /// \f$ F_{UU,T} = x \sum e_a^2 f_1^a(x) D_1^a(z) \f$
      // double res = Elastic_XS({GE2, GM2}, kine);
      // res        = res * hbarc2_gev_nb * kine.sin_th;
      return (t1 * FF);
    };

    // -------------------------------------------------------
    //
    auto Eprime_and_Ph_func = [](const InitialState& is, const std::array<double, 6>& x) {
      using namespace insane::units;
      using namespace ROOT::Math;
      double          Mh = M_pion / GeV;
      SIDISKinematics kine(is.p1().E(), Mh, x, SPEC_vars());
      XYZVector       l2(Polar3D<double>(kine.P_l2, kine.theta_l2, kine.phi_l2));
      XYZVector       p_h(Polar3D<double>(kine.P_p2, kine.theta_p2, kine.phi_p2));
      auto            E_prime  = ROOT::Math::XYZTVector(l2.x(), l2.y(), l2.z(), kine.E_l2);
      auto            P_hadron = ROOT::Math::XYZTVector(p_h.x(), p_h.y(), p_h.z(), kine.E_p2);
      return std::array<ROOT::Math::XYZTVector, 2>({E_prime, P_hadron});
    };

    // returns SIDIS_vars using the SPEC_vars (measured by spectrometers)
    auto to_sidis_variables = [=](const InitialState& is, const std::array<double, 6>& x) {
      using namespace insane::units;
      using namespace ROOT::Math;
      double          Mh = M_pion / GeV;
      SIDISKinematics kine(is.p1().E(), Mh, x, SIDIS_vars());
      // kine.Print();
      std::array<double, 6> res = {kine.x, kine.y, kine.z, kine.P_hPerp, kine.phi_h, kine.phi_q};
      return res;
    };

    auto to_spectrometer_variables = [=](const InitialState& is, const std::array<double, 6>& x) {
      using namespace insane::units;
      using namespace ROOT::Math;
      double                Mh = M_pion / GeV;
      SIDISKinematics       kine(is.p1().E(), Mh, x, SPEC_vars());
      std::array<double, 6> res = {kine.x, kine.y, kine.z, kine.P_hPerp, kine.phi_h, kine.phi_q};
      return res;
    };

    auto diff_xs_sidis = make_diff_cross_section(ps_spectrometers, SIDIS_xs_func);
    auto diff_xs_sidis_lowdp = make_diff_cross_section(ps_spectrometers_lowdp, SIDIS_xs_func);

    auto fs_parts =
        make_final_state_particles({11, 211}, {0.000511, M_pion / GeV}, Eprime_and_Ph_func);

    auto fs_SIDIS = make_final_state(ps_spectrometers, fs_parts);

    auto p_integrated_XS = make_integrated_cross_section(init_state, diff_xs_sidis);
    auto n_integrated_XS = make_integrated_cross_section(init_state, diff_xs_sidis);

    //auto p_integrated_XS_lowdp = make_integrated_cross_section(init_state, diff_xs_sidis_lowdp);
    //auto n_integrated_XS_lowdp = make_integrated_cross_section(init_state, diff_xs_sidis_lowdp);
    // auto sampler       = make_ps_sampler(p_integrated_XS);
    // auto sampler2      = make_ps_sampler(n_integrated_XS);
    // sampler.SetFoamCells(500);
    // sampler.SetFoamSample(100);
    // sampler.SetFoamChat(0);
    // sampler2.SetFoamCells(500);
    // sampler2.SetFoamSample(100);
    // sampler2.SetFoamChat(0);

    // auto evgen    = make_event_generator(sampler, fs_SIDIS);
    xs_select       = 1;
    auto total_XS_p = p_integrated_XS.CalculateTotalXS();// + p_integrated_XS_lowdp.CalculateTotalXS();
    // auto total_XS_p = evgen.Init();

    // auto evgen2    = make_event_generator(sampler2, fs_SIDIS);
    xs_select       = 0;
    auto total_XS_n = n_integrated_XS.CalculateTotalXS();// + n_integrated_XS_lowdp.CalculateTotalXS();
    // auto total_XS_n = evgen2.Init();

    double total_XS = total_XS_n + total_XS_p;

    //// --------------------------
    //
    // std::cout << "Total cross section = " << total_XS  << " nb\n";
    int N_sim = 100000;

    using namespace insane;
    double Loop1_LD2_Lumi = materials::ComputeLuminosityPerN(
        {1, 2}, materials::LD2_density /*g/cm3*/, 10.0 /*cm*/, 50.0e-6);
    double Loop1_LH2_Lumi = materials::ComputeLuminosityPerN(
        {1, 1}, materials::LH2_density /*g/cm3*/, 10.0 /*cm*/, 50.0e-6);
    double Loop1_entrance_window_thickness = 0.0104;
    double Loop1_exit_window_thickness     = 0.0133;
    double Loop3_entrance_window_thickness = 0.0130;
    double Loop3_exit_window_thickness     = 0.0188;

    double Loop1_windows_Lumi = materials::ComputeLuminosityPerN(
        {13, 27}, materials::Al_density,
        Loop1_entrance_window_thickness + Loop1_exit_window_thickness, 50.0e-6);

    // double Lumi = Loop1_LD2_Lumi + Loop1_windows_Lumi;
    TargetRates targ_rates;

    targ_rates.LD2_Lumi    = Loop1_LD2_Lumi;
    targ_rates.LH2_Lumi    = Loop1_LH2_Lumi;
    targ_rates.window_Lumi = Loop1_windows_Lumi;

    // double total_cross_section = 2*total_XS_p * nanobarn;
    // double luminosity          = Lumi / cm2;
    targ_rates.p_XS      = total_XS_p;
    targ_rates.n_XS      = total_XS_n;
    targ_rates.LD2_XS    = total_XS_n + total_XS_p;
    targ_rates.LH2_XS    = total_XS_p;
    targ_rates.window_XS = (total_XS_n * 14 + total_XS_p * 13);

    targ_rates.LD2_rate       = targ_rates.LD2_XS * targ_rates.LD2_Lumi * (nanobarn / cm2);
    targ_rates.LH2_rate       = targ_rates.LH2_XS * targ_rates.LH2_Lumi * (nanobarn / cm2);
    targ_rates.window_rate    = targ_rates.window_XS * targ_rates.window_Lumi * (nanobarn / cm2);
    targ_rates.total_LD2_rate = targ_rates.window_rate + targ_rates.LD2_rate;
    targ_rates.total_LH2_rate = targ_rates.window_rate + targ_rates.LH2_rate;
    targ_rates.total_rate     = targ_rates.window_rate + targ_rates.LD2_rate;

    run_setting.rates = targ_rates;
    // run_setting.LD2_Luminosity    = Loop1_LD2_Lumi;
    // run_setting.window_Luminosity = Loop1_windows_Lumi;
    // run_setting.LD2_sigma         = total_xs_LD2;
    // run_setting.window_sigma      = total_xs_windows;
    std::vector<double> res = {targ_rates.LD2_XS,         targ_rates.LD2_rate,
                               targ_rates.total_LD2_rate, targ_rates.total_LH2_rate,
                               targ_rates.LH2_XS,         targ_rates.LH2_rate};
    return res;
  }

} // namespace hallc
