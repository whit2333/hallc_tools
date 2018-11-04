#include <iostream>
#include <cmath>
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"

#include "TCanvas.h"
#include "TLatex.h"
#include "TSystem.h"
#include "TStyle.h"
#include "Math/Vector3D.h"
#include "Math/Vector4D.h"
#include "Math/VectorUtil.h"
R__LOAD_LIBRARY(libMathMore.so)
R__LOAD_LIBRARY(libGenVector.so)

#include "THStack.h"
#include "TBufferJSON.h"

#include "nlohmann/json.hpp"

#include "THcParmList.h"
R__LOAD_LIBRARY(libPodd.so)
R__LOAD_LIBRARY(libHallA.so)
R__LOAD_LIBRARY(libdc.so)
R__LOAD_LIBRARY(libHallC.so)

// fmt - string formatting library
#include "fmt/core.h"
#include "fmt/ostream.h"
R__LOAD_LIBRARY(libfmt.so)

using Pvec3D   = ROOT::Math::XYZVector;
using Pvec4D   = ROOT::Math::PxPyPzMVector;

// VecOps::RVec is like std::vector with some extra bells and whistles 
using inters   = ROOT::VecOps::RVec<int>;
using doublers = ROOT::VecOps::RVec<double>;
using floaters = ROOT::VecOps::RVec<float>;
using shorters = ROOT::VecOps::RVec<short>;

void df_example(int RunNumber = 3547, const char* codatype = "COIN", int nevents = 50001) {

  std::string coda_type = codatype;
  //std::string coda_type = "SHMS";
  //std::string coda_type = "COIN";

  std::string hallc_replay_dir = "./";

  std::string rootfile    = std::string("ROOTfiles/coin_replay_production_");
  if (coda_type == "SHMS") {
    rootfile = std::string(
      "ROOTfiles/shms_replay_production_all_");
    // + std::to_string(RunNumber) + "_50000.root";
  }
  rootfile += std::to_string(RunNumber) + "_" + std::to_string(nevents) + ".root";


  std::string run_list_json  = "DBASE/run_list.json";

  //auto ptheta_lab = hc_parms->Find("ptheta_lab");
  //ptheta_lab->Print();
  //auto hpcentral = hc_parms->Find("hpcentral");
  //hpcentral->Print();
  //auto ppcentral = hc_parms->Find("ppcentral");
  //ppcentral->Print();
  //std::cout << j.dump()  << "\n";
  //double P0_SHMS = hcana::json::FindVarValueOr(hc_parms,"ppcentral",0.0) ;
  //std::cout <<  "SHMS P0 = " << P0_SHMS << "\n";
  //double THETA_SHMS = hcana::json::FindVarValueOr(hc_parms,"ptheta_lab",0.0) ;

  //nlohmann::json j2;
  //{
  //  ifstream  in_run_file(run_list_json);
  //  in_run_file >> j2;
  //}
  //if( j2.find(std::to_string(RunNumber)) == j2.end() ) {
  //}
  //j2[std::to_string(RunNumber)] = j[std::to_string(RunNumber)];

  //{
  //  //std::cout << j.dump(2) << "\n";
  //  // write prettified JSON to another file
  //  std::ofstream o(run_list_json);
  //  o << std::setw(4) << j2 << std::endl;
  //}

  //std::cout << j2.dump() << "\n";

  ROOT::EnableImplicitMT(4);

  Pvec4D  Pbeam(0,0,10.598,0.000511);

  ROOT::RDataFrame d("T",rootfile);

  auto d0 = d
  .Define("shms_e_EoverP",
          [](doublers& EOverP) {
            return EOverP[EOverP > 0.8 && EOverP < 1.8];
          }, {"P.cal.etottracknorm"})
  .Define("shms_e_EoverP_nGood",
          [](doublers& EOverP) {
            return (int)EOverP.size();
          }, {"shms_e_EoverP"})
  ;
    
  auto d2 = d0.Filter(
    [](int n_good) {
        if( n_good >0 ) {
          return true;
        }
      return  false;
    },{"shms_e_EoverP_nGood"});


  // Histograms lazy evaluated -> Nothing happens until they are used below (eg Draw() is called) 
  auto h_EOverP_0       = d0.Histo1D<doublers>({"hms_e_EoverP_0","all; HMS E/P",    100,0.05,1.8},"P.cal.etottracknorm");
  auto h_EOverP_1       = d0.Histo1D<doublers>({"hms_e_EoverP_1","E/P cut; HMS E/P",100,0.05,1.8},"shms_e_EoverP");
  auto h_EOverP_nGood_0 = d0.Histo1D({"h_EOverP_nGood_0","h_EOverP_nGood_0",10,0,10},"P.cal.ntracks");
  auto h_EOverP_nGood_1 = d0.Histo1D({"h_EOverP_nGood_1","h_EOverP_nGood_0",10,0,10},"shms_e_EoverP_nGood");
  
  auto s_dc_x_fp           = d2.Histo1D({"s_dc_x_fp ","xy fp; x",100,-50,50}, "P.dc.x_fp");
  auto s_dc_y_fp           = d2.Histo1D({"s_dc_y_fp ","xy fp; y",100,-50,50}, "P.dc.y_fp");
  auto s_dc_xp_fp          = d2.Histo1D({"s_dc_xp_fp","xy fp; xp",100,-50,50},"P.dc.xp_fp");
  auto s_dc_yp_fp          = d2.Histo1D({"s_dc_yp_fp","xy fp; xp",100,-50,50},"P.dc.yp_fp");
  
  // -----------------------------------------------------------
  //
  TCanvas* c = nullptr;
  int b1 = 0;
  int b2 = 0;
  double hmax = 0.0;
  TLatex latex;

  // ---------------------------------------------------------
  //
  c = new TCanvas();
  c->Divide(2,2);
  c->cd(1);

  // This call starts the loop over the data.
  // A DrawCopy is used so that the histogram is not deleted at the end of 
  // scope, and thus stays visible on the canvas.
  s_EOverP_0->DrawCopy();

  c->cd(2);

  c->cd(3);

  c->cd(4);

  gPad->BuildLegend();

  gSystem->mkdir("results/df_example", true);
  c->SaveAs((std::string("results/df_example/c1_")+std::to_string(RunNumber)+".pdf").c_str());
  c->SaveAs((std::string("results/df_example/c1_")+std::to_string(RunNumber)+".png").c_str());
  
  // ---------------------------------------------------------
  //
  c = new TCanvas();
  //c->Divide(2,2);
  //c->cd(1);
  s_EOverP_0->DrawCopy();
  s_EOverP_1->SetLineColor(2);
  s_EOverP_1->DrawCopy("same");

  gPad->BuildLegend();

  c->SaveAs((std::string("results/df_example/c2_")+std::to_string(RunNumber)+".pdf").c_str());
  c->SaveAs((std::string("results/df_example/c2_")+std::to_string(RunNumber)+".png").c_str());

}
