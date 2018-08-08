#include "runplan/arrange_tables.h"

/** Arrange the tables into a run plan.
 *
 */
void arrange_tables() {

  using namespace ranges;
  using namespace date;
  using namespace std::chrono;

  constexpr auto csv_start   = 2018_y/October/29;
  constexpr auto csv_break   = 2018_y/November/15;
  constexpr auto csv_restart = 2019_y/February/21;
  constexpr auto csv_end     = 2019_y/March/10;

  auto run1_duration = std::chrono::duration_cast<days>( day_point(csv_break) - day_point(csv_start)).count();
  auto run2_duration = std::chrono::duration_cast<days>( day_point(csv_end) - day_point(csv_restart)).count();
  auto run_duration  = run1_duration + run2_duration;

  // open json files of LH2 and LD2 run tables
  // create a map (mirroring json)
  std::map<std::string, TableVector>  target_tables_map;
  json jtot;
  {
    // LD2 table
    json j; 
    std::ifstream infile("tables/LD2_run_plan_table.json");
    infile >> j;
    TableVector* alltables = nullptr; 
    std::string sj = j.dump();
    TBufferJSON::FromJSON( alltables, sj.c_str());

    jtot["LD2"] = j;
    target_tables_map["LD2"] = *alltables;

  }
  {
    // LH2 table
    json j; 
    std::ifstream infile("tables/LH2_run_plan_table.json");
    infile >> j;
    TableVector* alltables = nullptr; 
    std::string sj = j.dump();
    TBufferJSON::FromJSON( alltables, sj.c_str());

    jtot["LH2"] = j;
    target_tables_map["LH2"] = *alltables;

  }

  // ------------------------------------------------------------------------

  double run2_shms_min_theta = 11.6;
  double run2_hms_min_theta  = 14.1;

  TH1F* h_setting_time     = new TH1F("h1", "setting times; time [hr]", 24, 0, 12);
  TH1F* h_setting_time_LH2 = new TH1F("h1", "setting times; time [hr]", 24, 0, 12);
  TH1F* h_setting_time_LD2 = new TH1F("h1", "setting times; time [hr]", 24, 0, 12);

  TH2F* h_zx_settings0    = new TH2F("h_zx_settings", " z vs x ; x ; z", 20, 0, 1, 20, 0, 1);
  TH2F* h_ptheta_settings = new TH2F("h_zx_settings", " z vs x ; x ; z", 20, 0, 1, 20, 0, 1);


  std::vector<RunPlanTableEntry> all_LD2_settings;
  std::vector<RunPlanTableEntry> all_LH2_settings;
  std::vector<int> all_LD2_settings_num;
  std::vector<int> all_LH2_settings_num;

  std::vector<Q2Table> LD2_Q2_tables;
  std::vector<Q2Table> LH2_Q2_tables;

  auto LD2_tables = target_tables_map["LD2"];
  for (auto& atable : LD2_tables) {
    Q2Table& Q2_table = atable.second;
    // Swap every other entry so that the polarity changes only every other angle change. 
    auto n_swap = (Q2_table.size()-2)/4;
    for (auto i = Q2_table.begin()+2; i != Q2_table.begin()+2+4*n_swap+4; i += 4) {
      std::iter_swap(i, i+1);
    }
    LD2_Q2_tables.push_back(Q2_table);
  }

  auto LH2_tables = target_tables_map["LH2"];
  for (auto& atable : LH2_tables) {
    Q2Table& Q2_table = atable.second;
    // Swap every other entry so that the polarity changes only every other angle change. 
    auto n_swap = (Q2_table.size()-2)/4;
    for (auto i = Q2_table.begin()+2; i != Q2_table.begin()+2+4*n_swap+4; i += 4) {
      std::iter_swap(i, i+1);
    }
    // todo: set this correctly
    for(auto& en : Q2_table){
      en.A_target = 1;
    }
    LH2_Q2_tables.push_back(Q2_table);
  }

  
  auto LD2_rng1 = view::zip( view::ints(1)                        , LD2_Q2_tables[0]) | to_<std::vector>();
  auto LH2_rng1 = view::zip( view::ints(1)                        , LH2_Q2_tables[0]) | to_<std::vector>();
  auto LD2_rng2 = view::zip( view::ints((int) LD2_Q2_tables[0].size()+1), LD2_Q2_tables[1]) | to_<std::vector>();
  auto LH2_rng2 = view::zip( view::ints((int) LD2_Q2_tables[0].size()+1), LH2_Q2_tables[1]) | to_<std::vector>();
  auto LD2_rng3 = view::zip( view::ints((int) LD2_Q2_tables[1].size()+1), LD2_Q2_tables[2]) | to_<std::vector>();

  //auto allrng =
  //view::empty |
  //    action::push_back(view::zip(view::ints(1), LD2_Q2_tables[0])) |
  //    action::push_back(view::zip(view::ints(1), LH2_Q2_tables[0])) |
  //    action::push_back(view::zip(view::ints(LD2_Q2_tables[0].size() + 1), LD2_Q2_tables[1])) |
  //    action::push_back(view::zip(view::ints(LD2_Q2_tables[0].size() + 1), LH2_Q2_tables[1])) |
  //    action::push_back(view::zip(view::ints(LD2_Q2_tables[1].size() + 1), LD2_Q2_tables[2])); 

      // for (auto& a_table_vector : target_tables_map) {
      //  std::cout << a_table_vector.first << "\n";

      //  for (auto& a_table_targ : a_table_vector.second) {
      //    std::cout << "Q2 = " << a_table_targ.first << "\n";
      //    Q2Table& tab = a_table_targ.second;
      //    std::cout << tab.size() << "  settings \n";

      //    for (auto& entry : tab) {
      //      h_setting_time->Fill(entry.time);
      //      if (a_table_vector.first == std::string("LH2")) {
      //        h_setting_time_LH2->Fill(entry.time);
      //        entry.A_target = 1;
      //        entry.kinematic.Q2 = a_table_targ.first;
      //        all_LH2_settings.push_back(entry);
      //      } else if (a_table_vector.first == std::string("LD2")) {
      //        entry.kinematic.Q2 = a_table_targ.first;
      //        h_setting_time_LD2->Fill(entry.time);
      //        all_LD2_settings.push_back(entry);
      //        //RunPlanTableEntry::PrintHeader();
      //        //entry.Print();
      //      }
      //    }
      //  }
      //}

      // ------------------------------------------------------------------------
      std::vector<RunPlanTableEntry> all_settings;

  //auto n_swap = (all_LD2_settings.size()-2)/4;
  //for (auto i = all_LD2_settings.begin()+2; i != all_LD2_settings.begin()+2+4*n_swap+4; i += 4) {
  //  std::iter_swap(i, i+1);
  //}
  //n_swap = (all_LH2_settings.size()-2)/4;
  //for (auto i = all_LH2_settings.begin()+2; i != all_LH2_settings.begin()+2+4*n_swap+4; i += 4) {
  //  std::iter_swap(i, i+1);
  //}
  //auto LD2_rng = view::zip(all_LD2_settings, view::ints(1)) | to_<std::vector>();
  //auto LH2_rng = view::zip(all_LH2_settings, view::ints(1)) | to_<std::vector>();

  std::vector both_rng = {LD2_rng1,LH2_rng1,  LD2_rng2, LH2_rng2, LD2_rng3};
  //both_rng.push_back(LD2_rng1);
  //both_rng.push_back(LH2_rng1);
  //both_rng.push_back(LD2_rng2);
  //both_rng.push_back(LH2_rng2);
  //both_rng.push_back(LD2_rng3);

  auto all_rng = view::join(both_rng) | to_<std::vector>();

  //  action::stable_sort(
  //      action::stable_sort(all_rng, [](auto t1, auto t2) { return std::get<1>(t1) < std::get<1>(t2); }),
  //      [](auto t1, auto t2) { return std::get<0>(t1).kinematic.Q2 < std::get<0>(t2).kinematic.Q2; });

  all_rng =
      std::move(all_rng) |
      action::stable_sort([](auto t1, auto t2) { return std::get<0>(t1) < std::get<0>(t2); }) |
      action::stable_sort([](auto t1, auto t2) {
        return std::get<1>(t1).kinematic.Q2 < std::get<1>(t2).kinematic.Q2;
      });

  auto run_order = view::zip(
      view::ints(1), (all_rng | view::transform([](const auto& en) { return std::get<1>(en); })));

  auto run_order_vec = run_order | to_<std::vector>();
  for(const auto& entry : run_order_vec) {
    std::cout << std::get<0>(entry) << " ";
    std::get<1>(entry).Print();
  }

  // ------------------------------------------------------------------------

  auto times = run_order | view::transform( [](const auto& en){ return std::get<1>(en).time;});
  std::cout << times << "\n";

  double res = ranges::accumulate(times,0.0);
  std::cout <<  res << " hours \n";
  std::cout <<  res/24.0 << " days \n";

  auto shms_theta_p = all_LD2_settings | view::transform([](const RunPlanTableEntry& en) {
                        return std::make_pair(en.kinematic.th_q, en.polarity * en.kinematic.Ppi);
                      });
  auto shms_thetas = shms_theta_p | view::keys | to_<std::vector>();
  auto shms_ps     = shms_theta_p | view::values | to_<std::vector>();
  int irun = 0;
  auto LD2_numbered = all_LD2_settings | view::transform([&irun](const RunPlanTableEntry& en) {
                         int run_number = irun;
                         irun++;
                         return double(run_number);
                         //return std::make_pair(run_number, en.polarity * en.kinematic.Ppi);
                       })| to_<std::vector>();

  auto hms_theta_p = all_LD2_settings | view::transform([&irun](const RunPlanTableEntry& en) {
                       return std::make_pair(en.kinematic.th_e, en.kinematic.Ee);
                     });
  auto hms_thetas = hms_theta_p | view::keys | to_<std::vector>();
  auto hms_ps     = hms_theta_p | view::values | to_<std::vector>();


  // ------------------------------------------------------------------------
  auto ro_target_A = run_order | view::transform([](const auto& run_en) {
                    auto en = std::get<1>(run_en);
                    return (double)en.A_target;
                  }) | to_<std::vector>();
  auto ro_run_number = run_order | view::transform([](const auto& run_en) {
                    auto en = std::get<0>(run_en);
                    return (double)en;
                  }) | to_<std::vector>();
  auto ro_shms_pol = run_order | view::transform([](const auto& run_en) {
                    auto en = std::get<1>(run_en);
                    return (double)en.polarity;
                  }) | to_<std::vector>();
  auto ro_shms_theta = run_order | view::transform([](const auto& run_en) {
                    auto en = std::get<1>(run_en);
                    return (double)en.kinematic.th_q;
                  }) | to_<std::vector>();
  auto ro_hms_theta = run_order | view::transform([](const auto& run_en) {
                    auto en = std::get<1>(run_en);
                    return (double)en.kinematic.th_e;
                  }) | to_<std::vector>();
  auto ro_shms_p = run_order | view::transform([](const auto& run_en) {
                    auto en = std::get<1>(run_en);
                    return (double)en.kinematic.Ppi*en.polarity;
                  }) | to_<std::vector>();
  auto ro_hms_p = run_order | view::transform([](const auto& run_en) {
                    auto en = std::get<1>(run_en);
                    return (double)en.kinematic.Ee;
                  }) | to_<std::vector>();
  auto ro_times = run_order | view::transform([](const auto& run_en) {
                    auto en = std::get<1>(run_en);
                    return (double)en.time;
                  }) | to_<std::vector>();

  auto ro_by_targ = run_order | view::group_by([](const auto& t1, const auto& t2) {
                      return std::get<1>(t1).A_target == std::get<1>(t2).A_target;
                    }) |
                    view::transform([](auto r) { return r | to_<std::vector>(); }) |
                    to_<std::vector>();

  auto ro_by_Q2   = run_order | view::group_by([](const auto& t1, const auto& t2) {
                      return std::get<1>(t1).kinematic.Q2 == std::get<1>(t2).kinematic.Q2;
                    }) |
                    view::transform([](auto r) { return r | to_<std::vector>(); }) |
                    to_<std::vector>();

  auto ro_LD2_targ = run_order | view::filter([](const auto& run_en) {
                    return std::get<1>(run_en).A_target == 2;
  }) | to_<std::vector>();

  auto ro_LH2_targ = run_order | view::filter([](const auto& run_en) {
                    return std::get<1>(run_en).A_target == 1;
  }) | to_<std::vector>();

  for (const auto& atime : ro_times) {
    h_setting_time->Fill(atime);
  }

  for (const auto& en : ro_LD2_targ) {
    h_setting_time_LD2->Fill(std::get<1>(en).time);
    h_setting_time->Fill(std::get<1>(en).time*0.1);
  }
  for (const auto& en : ro_LH2_targ) {
    h_setting_time_LH2->Fill(std::get<1>(en).time);
  }


  {
    std::ofstream output_ro_by_Q2("tables/run_order_by_Q2.json",std::ios_base::trunc);
    output_ro_by_Q2 << TBufferJSON::ToJSON(&ro_by_Q2);
  }

  std::cout << " TYPE : \n";
  std::cout << typeid(ro_by_Q2).name() << '\n';
  debug_class<decltype(ro_by_Q2)>();
  debug_type<decltype(ro_by_Q2)>();


  TGraph* gr_run_number_VS_shms_pol  = new TGraph(ro_run_number.size(), ro_run_number.data(), ro_shms_pol.data());
  TGraph* gr_run_number_VS_target_A  = new TGraph(ro_run_number.size(), ro_run_number.data(), ro_target_A.data());
  TGraph* gr_run_number_VS_theta_hms  = new TGraph(ro_run_number.size(), ro_run_number.data(), ro_hms_theta.data());
  TGraph* gr_run_number_VS_theta_shms = new TGraph(ro_run_number.size(), ro_run_number.data(), ro_shms_theta.data());
  TGraph* gr_run_number_VS_p_hms   = new TGraph(ro_run_number.size(), ro_run_number.data(), ro_hms_p.data());
  TGraph* gr_run_number_VS_p_shms  = new TGraph(ro_run_number.size(), ro_run_number.data(), ro_shms_p.data());
  TGraph* gr_thetap_settings_shms = new TGraph(ro_run_number.size(), ro_shms_theta.data(), ro_shms_p.data());
  TGraph* gr_thetap_settings_hms  = new TGraph(ro_run_number.size(), ro_hms_theta.data(), ro_hms_p.data());

  gr_thetap_settings_shms->SetTitle(" ; #theta SHMS [deg] ; P SHMS [GeV/c]");
  gr_thetap_settings_hms->SetTitle(" ; #theta HMS [deg] ; P HMS [GeV/c]");


  // ------------------------------------------------------------------------
  //
  TCanvas* c = nullptr;

  c = new TCanvas();

  TMultiGraph* mg_thetap_settings = new TMultiGraph();

  THStack* hs = new THStack("h_setting_times","setting times; time [hr]");
  h_setting_time    ->SetFillColor(8);
  h_setting_time_LH2->SetFillColor(4);
  h_setting_time_LD2->SetFillColor(2);

  h_setting_time    ->SetMarkerColor(8);
  h_setting_time_LH2->SetMarkerColor(4);
  h_setting_time_LD2->SetMarkerColor(2);

  h_setting_time_LH2->SetMarkerStyle(21);
  h_setting_time_LD2->SetMarkerStyle(21);

  //hs->Add(h_setting_time);
  hs->Add(h_setting_time_LH2);
  hs->Add(h_setting_time_LD2);

  h_setting_time->Draw();
  hs->Draw("same");

  // -------------------------------------

  c  = new TCanvas();

  gr_thetap_settings_shms->SetMarkerStyle(20);
  mg_thetap_settings->Add(gr_thetap_settings_shms,"lp");

  gr_thetap_settings_hms->SetMarkerStyle(20);
  gr_thetap_settings_hms->SetMarkerColor(2);
  gr_thetap_settings_hms->SetLineColor(2);
  mg_thetap_settings->Add(gr_thetap_settings_hms,"lp");

  mg_thetap_settings->SetTitle(" ; #theta SHMS [deg] ; P SHMS [GeV/c]");
  mg_thetap_settings->Draw("a");

  // -------------------------------------

  c  = new TCanvas();

  TMultiGraph* mg = new TMultiGraph();

  gr_run_number_VS_shms_pol->SetTitle("SHMS polarity");
  gr_run_number_VS_shms_pol->SetMarkerStyle(20);
  mg->Add(gr_run_number_VS_shms_pol,"lp");

  gr_run_number_VS_target_A->SetTitle("target A");
  gr_run_number_VS_target_A->SetMarkerStyle(20);
  gr_run_number_VS_target_A->SetMarkerColor(2);
  gr_run_number_VS_target_A->SetLineColor(2);
  mg->Add(gr_run_number_VS_target_A,"lp");

  mg->SetTitle(" ; run number ");
  mg->Draw("a");
  c->BuildLegend();

  // -------------------------------------

  c  = new TCanvas();
  mg = new TMultiGraph();
  gr_run_number_VS_theta_shms->SetTitle("SHMS");
  gr_run_number_VS_theta_hms->SetTitle("HMS");

  gr_run_number_VS_theta_hms->SetMarkerStyle(20);
  mg->Add(gr_run_number_VS_theta_hms,"lp");

  gr_run_number_VS_theta_shms->SetMarkerStyle(20);
  gr_run_number_VS_theta_shms->SetMarkerColor(2);
  gr_run_number_VS_theta_shms->SetLineColor(2);
  mg->Add(gr_run_number_VS_theta_shms,"lp");

  mg->SetTitle(" ; run number ; #theta [deg] ");
  mg->Draw("a");
  c->BuildLegend();

  // -------------------------------------

  c  = new TCanvas();
  mg = new TMultiGraph();
  gr_run_number_VS_p_shms->SetTitle("SHMS");
  gr_run_number_VS_p_hms->SetTitle("HMS");

  gr_run_number_VS_p_hms->SetMarkerStyle(20);
  mg->Add(gr_run_number_VS_p_hms,"lp");

  gr_run_number_VS_p_shms->SetMarkerStyle(20);
  gr_run_number_VS_p_shms->SetMarkerColor(2);
  gr_run_number_VS_p_shms->SetLineColor(2);
  mg->Add(gr_run_number_VS_p_shms,"lp");

  mg->SetTitle(" ; run number ; p_{0} [GeV/c] ");
  mg->Draw("a");
  c->BuildLegend();
}
