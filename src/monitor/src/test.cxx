#include <iostream>
#include <cmath>
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"
#include "TCanvas.h"
//#include "ROOT/TCanvas.hxx"
#include "Math/Vector3D.h"
#include "Math/Vector4D.h"
#include "Math/VectorUtil.h"
#include "THStack.h"

#include "nlohmann/json.hpp"
#include "THcParmList.h"
R__LOAD_LIBRARY(libHallA.so)
R__LOAD_LIBRARY(libdc.so)
R__LOAD_LIBRARY(libHallC.so)

using Pvec3D   = ROOT::Math::XYZVector;
using Pvec4D   = ROOT::Math::PxPyPzMVector;
using inters = ROOT::VecOps::RVec<int>;
using doublers = ROOT::VecOps::RVec<double>;
using floaters = ROOT::VecOps::RVec<float>;
using shorters = ROOT::VecOps::RVec<short>;
using FVec     = std::vector<float>;

void test(int RunNumber = 3566) {

  std::string hallc_replay_dir = "/home/whit/projects/hall_AC_projects/new_hallc_replay/EXPERIMENT_replay/hallc_replay/";
  std::string rootfile         = std::string("rootfiles/coin_replay_production_") + std::to_string(RunNumber) + "_200000.root";
  std::string run_list_json  = "DBASE/run_list.json";

  THcParmList* hc_parms = new THcParmList();
  hc_parms->Define("gen_run_number", "Run Number", RunNumber);
  hc_parms->AddString("g_ctp_database_filename", (hallc_replay_dir+"DBASE/COIN/standard.database").c_str() );
  hc_parms->Load(hc_parms->GetString("g_ctp_database_filename"), RunNumber);
  hc_parms->Load(hc_parms->GetString("g_ctp_parm_filename"));
  hc_parms->Load(hc_parms->GetString("g_ctp_kinematics_filename"), RunNumber);

  auto j = nlohmann::json::parse(hc_parms->PrintJSON(RunNumber));
  auto htheta_lab = hc_parms->Find("htheta_lab");
  htheta_lab->Print();
  auto ptheta_lab = hc_parms->Find("ptheta_lab");
  ptheta_lab->Print();
  auto hpcentral = hc_parms->Find("hpcentral");
  hpcentral->Print();
  auto ppcentral = hc_parms->Find("ppcentral");
  ppcentral->Print();
  //std::cout << j.dump()  << "\n";

  std::cout << hcana::json::FindVarValueOr(hc_parms,"ppcentral",1.0) << std::endl;;

  nlohmann::json j2;
  {
    ifstream  in_run_file(run_list_json);
    in_run_file >> j2;
  }
  //if( j2.find(std::to_string(RunNumber)) == j2.end() ) {
  //}
  j2[std::to_string(RunNumber)] = j[std::to_string(RunNumber)];

  {
    //std::cout << j.dump(2) << "\n";
    // write prettified JSON to another file
    std::ofstream o(run_list_json);
    o << std::setw(4) << j2 << std::endl;
  }

  //std::cout << j2.dump() << "\n";

  ROOT::EnableImplicitMT(4);

  Pvec4D  Pbeam(0,0,10.6,0.000511);


  ROOT::RDataFrame d("T",rootfile);

  auto d0 = d
  .Define("hms_e_EoverP",
          [](doublers& EOverP) {
            return EOverP[EOverP > 0.8 && EOverP < 1.2];
          }, {"H.cal.etottracknorm"})
  .Define("hms_e_EoverP_nGood",
          [](doublers& EOverP) {
            return (int)EOverP.size();
          }, {"hms_e_EoverP"});

  auto d1 = d0.Filter(
    [](double npe) {
      if( npe >2 ) { return true; }
      return  false;
    },{"H.cer.npeSum"});
    
  auto d2 = d0.Filter(
    [](int n_good) {
        if( n_good >0 ) {
          return true;
        }
      return  false;
    },{"hms_e_EoverP_nGood"});

  auto d3 = d2.Filter(
    [](double npe) {
      if( npe >2 ) { return true; }
      return  false;
    },{"H.cer.npeSum"});
    


  auto h_EOverP_0       = d0.Histo1D<doublers>({"hms_e_EoverP_0","all; HMS E/P",100,0,1.8},"H.cal.etottracknorm");
  auto h_EOverP_1       = d0.Histo1D<doublers>({"hms_e_EoverP_1","E/P cut; HMS E/P",100,0,1.8},"hms_e_EoverP");
  auto h_EOverP_nGood_0 = d0.Histo1D({"h_EOverP_nGood_0","h_EOverP_nGood_0",10,0,10},"H.cal.ntracks");
  auto h_EOverP_nGood_1 = d0.Histo1D({"h_EOverP_nGood_1","h_EOverP_nGood_0",10,0,10},"hms_e_EoverP_nGood");

  auto h_cer_0          = d0.Histo1D({"hms_cer_0","all; HMS npe sum",100,0,20},"H.cer.npeSum");
  auto h_cer_2          = d2.Histo1D({"hms_cer_0","with E/P cut; HMS npe sum",100,0,20},"H.cer.npeSum");

  auto s_aero_npe_0          = d0.Histo1D({"s_aero_npe_0","all; SHMS aero npe sum",100,0,20},"P.aero.npeSum");
  auto s_aero_npe_2          = d2.Histo1D({"s_aero_npe_2","with E/P cut; SHMS aero npe sum",100,0,20},"P.aero.npeSum");

  auto s_hgc_npe_0          = d0.Histo1D({"s_hgc_npe_0","all; SHMS HGC npe sum",100,0,20},"P.hgcer.npeSum");
  auto s_hgc_npe_2          = d2.Histo1D({"s_hgc_npe_2","with HMS E/P cut; SHMS HGC npe sum",100,0,20},"P.hgcer.npeSum");
  auto s_hgc_npe_3          = d3.Histo1D({"s_hgc_npe_3","with HMS E/P and cer cut; SHMS HGC npe sum",100,0,20},"P.hgcer.npeSum");

  auto s_ngc_npe_0          = d0.Histo1D({"s_ngc_npe_0","all; SHMS ngc npe sum",100,0,50},"P.ngcer.npeSum");
  auto s_ngc_npe_2          = d2.Histo1D({"s_ngc_npe_2","with E/P cut; SHMS ngc npe sum",100,0,50},"P.ngcer.npeSum");
  auto s_ngc_npe_3          = d3.Histo1D({"s_ngc_npe_3","with HMS E/P  and cer cut; SHMS ngc npe sum",100,0,50},"P.ngcer.npeSum");

  auto s_EOverP_0       = d0.Histo1D<doublers>({"hms_e_EoverP_0","all; SHMS E/P",100,0,1.8},"P.cal.etottracknorm");
  auto s_EOverP_2       = d2.Histo1D<doublers>({"hms_e_EoverP_2","with HMS E/P cut ; SHMS E/P",100,0,1.8},"P.cal.etottracknorm");
  auto s_EOverP_3       = d3.Histo1D<doublers>({"hms_e_EoverP_3","with HMS E/P and cer cut ; SHMS E/P",100,0,1.8},"P.cal.etottracknorm");

  auto h_dc_chi2_0      = d0.Histo1D({"h_dc_chi2_0","with E/P cut; HMS dc chi2",100,0,20},"H.dc.chisq");
  auto h_dc_chi2_2      = d2.Histo1D({"h_dc_chi2_0","with E/P cut; HMS dc chi2",100,0,20},"H.dc.chisq");

  auto h_dc_x_fp_0      = d0.Histo1D({"h_dc_x_fp_0","with E/P cut; HMS dc x_fp",100,-50,50},"H.dc.x_fp");
  auto h_dc_x_fp_2      = d2.Histo1D({"h_dc_x_fp_2","with E/P cut; HMS dc x_fp",100,-50,50},"H.dc.x_fp");
  auto h_dc_y_fp_0      = d0.Histo1D({"h_dc_y_fp_0","with E/P cut; HMS dc y_fp",100,-50,50},"H.dc.y_fp");
  auto h_dc_y_fp_2      = d2.Histo1D({"h_dc_y_fp_2","with E/P cut; HMS dc y_fp",100,-50,50},"H.dc.y_fp");

  auto h_EOverP_2       = d1.Histo1D<doublers>({"hms_e_EoverP_2","all with cer cut; HMS E/P",100,0,1.8},"H.cal.etottracknorm");
  auto h_EOverP_3       = d1.Histo1D<doublers>({"hms_e_EoverP_3","with E/P and cer cut; HMS E/P",100,0,1.8},"hms_e_EoverP");
  auto h_EOverP_nGood_2 = d1.Histo1D({"h_EOverP_nGood_2","h_EOverP_nGood_0",10,0,10},"H.cal.ntracks");
  auto h_EOverP_nGood_3 = d1.Histo1D({"h_EOverP_nGood_3","h_EOverP_nGood_0",10,0,10},"hms_e_EoverP_nGood");


  //auto d1 = d.Filter([](const std::vector<float>& eOverP) {
  //  return 
  
  auto c = new TCanvas();
  c->Divide(2,2);
  c->cd(1);
  h_EOverP_0->DrawCopy();
  h_EOverP_1->SetLineColor(2);
  h_EOverP_1->DrawCopy("same");
  h_EOverP_2->SetLineColor(4);
  h_EOverP_2->DrawCopy("same");
  h_EOverP_3->SetLineColor(8);
  h_EOverP_3->DrawCopy("same");
  gPad->BuildLegend();
  c->cd(2);
  h_EOverP_nGood_0->DrawCopy();
  h_EOverP_nGood_1->SetLineColor(2);
  h_EOverP_nGood_1->DrawCopy("same");
  h_EOverP_nGood_2->SetLineColor(4);
  h_EOverP_nGood_2->DrawCopy("same");
  h_EOverP_nGood_3->SetLineColor(8);
  h_EOverP_nGood_3->DrawCopy("same");

  c->cd(3);
  auto hmax = h_cer_0->GetMaximum();
  h_cer_2->SetMaximum(hmax/20.0);
  h_cer_2->SetLineColor(2);
  h_cer_2->DrawCopy();
  h_cer_0->DrawCopy("same");

  c->cd(4);
  h_dc_x_fp_0->DrawCopy();
  h_dc_x_fp_2->SetLineColor(2);
  h_dc_x_fp_2->DrawCopy("same");
  h_dc_y_fp_0->DrawCopy("same");
  h_dc_y_fp_2->SetLineColor(2);
  h_dc_y_fp_2->DrawCopy("same");

  c = new TCanvas();
  c->Divide(2,2);
  c->cd(1);
  hmax = s_aero_npe_0->GetMaximum();
  s_aero_npe_0->SetMaximum(hmax/20.0);
  s_aero_npe_0->DrawCopy();
  s_aero_npe_2->SetLineColor(2);
  s_aero_npe_2->DrawCopy("same");

  c->cd(2);
  hmax = s_hgc_npe_0->GetMaximum();
  s_hgc_npe_0->SetMaximum(hmax/20.0);
  s_hgc_npe_0->DrawCopy();
  s_hgc_npe_2->SetLineColor(2);
  s_hgc_npe_2->DrawCopy("same");
  s_hgc_npe_3->SetLineColor(4);
  s_hgc_npe_3->DrawCopy("same");


  c->cd(3);
  hmax = s_ngc_npe_0->GetMaximum();
  s_ngc_npe_0->SetMaximum(hmax/100.0);
  s_ngc_npe_0->DrawCopy();
  s_ngc_npe_2->SetLineColor(2);
  s_ngc_npe_2->DrawCopy("same");
  s_ngc_npe_3->SetLineColor(4);
  s_ngc_npe_3->DrawCopy("same");

  c->cd(4);
  s_EOverP_0->DrawCopy();
  s_EOverP_2->SetLineColor(2);
  s_EOverP_2->DrawCopy("same");
  s_EOverP_3->SetLineColor(4);
  s_EOverP_3->DrawCopy("same");
  gPad->BuildLegend();

}
