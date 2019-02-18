#include "monitor/EventDisplays.h"

#include "THcHodoscope.h"
#include "THcHodoHit.h"

namespace hallc {
  namespace event_display {

    Int_t BasicEventDisplay::Init(const TDatime&) {
      auto plt2 = _det_display->CreateDisplayPlot("BasicEventDisplay_plot",
                                                  [&](hallc::DisplayPlot& plt) {
                                                    plt._plot_data._canvas = new TCanvas();
                                                    // initialize plots here
                                                    return 0;
                                                  },
                                                  [&](hallc::DisplayPlot& plt) {
                                                    // update histograms here
                                                    return 0;
                                                  });
      _det_display->InitAll();
      return 0;
    }

    Int_t BasicEventDisplay::Process(const THaEvData* evt, const THaRunBase*, Int_t code) {
      if ((evt->GetEvNum() > 1200) && (_counter > 3000)) {
        _det_display->_event_number  = evt->GetEvNum();
        _det_display->Process();
        // process is inside the if statment so only every 1000 events are viewed.
        _det_display->UpdateAll();
        gSystem->ProcessEvents();
        // WritePlots();
        _counter = 0;
      }
      _counter++;
      return 0;
    }

    Int_t BasicEventDisplay::Close() { return 0; }

    //___________________________________________________________________________

    Int_t SHMSEventDisplay::Init(const TDatime&) {
      // Addds
      int N_planes = _hod->GetNPlanes();

      // Hodoscope 2D
      auto plt1 = _det_display->CreateDisplayPlot(
          "shms/hod/", "SHMS_Hod_2D",
          [&, N_planes](hallc::DisplayPlot& plt) {
            plt._plot_data._canvas = new TCanvas(plt.GetName().c_str(), plt.GetName().c_str());
            for (int ip = 0; ip < N_planes; ip++) {
              auto n_paddles = _hod->GetNPaddles(ip) + 2;
              if (ip % 2 == 0) {
                plt._plot_data._hists2.push_back(new TH2F(fmt::format("SHMS_hod_plane_{}", ip).c_str(),
                                               fmt::format("SHMS Hodoscope plane {}", ip).c_str(),
                                               n_paddles, 0, n_paddles, 1, 0, 16));
              } else {
                plt._plot_data._hists2.push_back(new TH2F(fmt::format("SHMS_hod_plane_{}", ip).c_str(),
                                               fmt::format("SHMS Hodoscope plane {}", ip).c_str(),
                                               1, 0, 16, n_paddles, 0, n_paddles));
              }
            }
            //
            plt._plot_data._canvas->Divide(2, 1);
            plt._plot_data._canvas->cd(1);
            plt._plot_data._hists2[0]->SetLineColor(2);
            plt._plot_data._hists2[0]->SetFillColor(2);
            plt._plot_data._hists2[0]->Draw("box");
            plt._plot_data._hists2[1]->SetFillColor(4);
            plt._plot_data._hists2[1]->SetLineColor(4);
            plt._plot_data._hists2[1]->Draw("box same");
            plt._plot_data._canvas->cd(2);
            plt._plot_data._hists2[2]->SetLineColor(2);
            plt._plot_data._hists2[2]->SetFillColor(2);
            plt._plot_data._hists2[2]->Draw("box");
            plt._plot_data._hists2[3]->SetLineColor(4);
            plt._plot_data._hists2[3]->SetFillColor(4);
            plt._plot_data._hists2[3]->Draw("box same");
            return 0;
          },
          [&, N_planes](hallc::DisplayPlot& plt) {
            for (int ip = 0; ip < N_planes; ip++) {
              plt._plot_data._hists2[ip]->Reset();
              // std::cout << " plane " << ip << "\n";
              TClonesArray* hodoHits = _hod->GetPlane(ip)->GetHits();
              // hit_vecs[ip] = std::vector<THcHodoHit*>();
              for (Int_t iphit = 0; iphit < _hod->GetPlane(ip)->GetNScinHits(); iphit++) {
                THcHodoHit* hit = (THcHodoHit*)hodoHits->At(iphit);
                // keep only those with "two good times"
                if (hit->GetTwoGoodTimes()) {
                  auto i_paddle = hit->GetPaddleNumber();
                  plt._plot_data._hists2[ip]->Fill(i_paddle, i_paddle);
                }
              }
            }
            return 0;
          });

      // hodoscope 3D display
      auto plt2 = _det_display->CreateDisplayPlot(
          "shms/hod/", "SHMS_Hod_3D",
          [&, N_planes](hallc::DisplayPlot& plt) {
            plt._plot_data._canvas = new TCanvas(plt.GetName().c_str(), plt.GetName().c_str());
            // for (int ip = 0; ip < N_planes; ip++) {
            auto n_paddles = 16;
            // if (ip % 2 == 0) {
            //  plt._hists3.push_back(new TH3F(fmt::format("SHMS_hod_3D_{}", ip).c_str(),
            //                                 fmt::format("SHMS Hodoscope plane {}", ip).c_str(),
            //                                 n_paddles, 0, n_paddles, 1, 0, 16, 20, 0, 20));
            //} else {
            plt._plot_data._hists3.push_back(new TH3F(fmt::format("SHMS_hod_3D_{}", 0).c_str(),
                                           fmt::format("SHMS Hodoscope plane {}", 0).c_str(),
                                           n_paddles, 0, n_paddles, n_paddles, 0, n_paddles, 20, 0,
                                           20));
            //}
            //}
            //
            plt._plot_data._canvas->cd();
            plt._plot_data._hists3[0]->Draw("box2");
            return 0;
          },
          [&, N_planes](hallc::DisplayPlot& plt) {
            plt._plot_data._hists3[0]->Reset();
            for (int ip = 0; ip < N_planes; ip++) {
              // std::cout << " plane " << ip << "\n";
              TClonesArray* hodoHits = _hod->GetPlane(ip)->GetHits();
              // hit_vecs[ip] = std::vector<THcHodoHit*>();
              for (Int_t iphit = 0; iphit < _hod->GetPlane(ip)->GetNScinHits(); iphit++) {
                THcHodoHit* hit = (THcHodoHit*)hodoHits->At(iphit);
                // keep only those with "two good times"
                if (hit->GetTwoGoodTimes()) {
                  auto i_paddle = hit->GetPaddleNumber();

                  for (int ix = 0; ix < 16; ix++) {
                    if (ip % 2 == 0) {
                      plt._plot_data._hists3[0]->Fill(ix, i_paddle, ip + 15 * (ip > 1));
                    } else {
                      plt._plot_data._hists3[0]->Fill(i_paddle, ix, ip + 15 * (ip > 1));
                    }
                  }
                }
              }
            }
            return 0;
          });
      //
      _det_display->InitAll();
      return 0;
    }

    //___________________________________________________________________________

    Int_t HMSEventDisplay::Init(const TDatime&) {
      // Addds
      int N_planes = _hod->GetNPlanes();

      // Hodoscope 2D
      auto plt1 = _det_display->CreateDisplayPlot(
          "hms/hod/", 
          "HMS_Hod_2D",
          [&, N_planes](hallc::DisplayPlot& plt) {
            plt._plot_data._canvas = new TCanvas(plt.GetName().c_str(),plt.GetName().c_str());
            for (int ip = 0; ip < N_planes; ip++) {
              auto n_paddles = _hod->GetNPaddles(ip) + 2;
              if (ip % 2 == 0) {
                plt._plot_data._hists2.push_back(new TH2F(fmt::format("HMS_hod_plane_{}", ip).c_str(),
                                               fmt::format("HMS Hodoscope plane {}", ip).c_str(),
                                               n_paddles, 0, n_paddles, 1, 0, 16));
              } else {
                plt._plot_data._hists2.push_back(new TH2F(fmt::format("HMS_hod_plane_{}", ip).c_str(),
                                               fmt::format("HMS Hodoscope plane {}", ip).c_str(), 1,
                                               0, 16, n_paddles, 0, n_paddles));
              }
            }
            //
            plt._plot_data._canvas->Divide(2, 1);
            plt._plot_data._canvas->cd(1);
            plt._plot_data._hists2[0]->SetLineColor(2);
            plt._plot_data._hists2[0]->SetFillColor(2);
            plt._plot_data._hists2[0]->Draw("box");
            plt._plot_data._hists2[1]->SetFillColor(4);
            plt._plot_data._hists2[1]->SetLineColor(4);
            plt._plot_data._hists2[1]->Draw("box same");
            plt._plot_data._canvas->cd(2);
            plt._plot_data._hists2[2]->SetLineColor(2);
            plt._plot_data._hists2[2]->SetFillColor(2);
            plt._plot_data._hists2[2]->Draw("box");
            plt._plot_data._hists2[3]->SetLineColor(4);
            plt._plot_data._hists2[3]->SetFillColor(4);
            plt._plot_data._hists2[3]->Draw("box same");
            return 0;
          },
          [&, N_planes](hallc::DisplayPlot& plt) {
            // std::cout << "processing\n";
            //
            // std::cout << N_planes << " planes\n";
            for (int ip = 0; ip < N_planes; ip++) {
              plt._plot_data._hists2[ip]->Reset();
              // std::cout << " plane " << ip << "\n";
              TClonesArray* hodoHits = _hod->GetPlane(ip)->GetHits();
              // hit_vecs[ip] = std::vector<THcHodoHit*>();
              for (Int_t iphit = 0; iphit < _hod->GetPlane(ip)->GetNScinHits(); iphit++) {
                THcHodoHit* hit = (THcHodoHit*)hodoHits->At(iphit);
                // keep only those with "two good times"
                if (hit->GetTwoGoodTimes()) {
                  auto i_paddle = hit->GetPaddleNumber();
                  plt._plot_data._hists2[ip]->Fill(i_paddle, i_paddle);
                }
              }
            }
            return 0;
          });

      // hodoscope 3D display
      auto plt2 = _det_display->CreateDisplayPlot(
          "hms/hod/", 
          "HMS_Hod_3D",
          [&, N_planes](hallc::DisplayPlot& plt) {
            plt._plot_data._canvas = new TCanvas(plt.GetName().c_str(),plt.GetName().c_str());
            auto n_paddles = 16;
            plt._plot_data._hists3.push_back(new TH3F(fmt::format("HMS_hod_3D_{}", 0).c_str(),
                                           fmt::format("HMS Hodoscope plane {}", 0).c_str(),
                                           n_paddles, 0, n_paddles, n_paddles, 0, n_paddles, 20, 0,
                                           20));
            plt._plot_data._canvas->cd();
            plt._plot_data._hists3[0]->Draw("box2");
            return 0;
          },
          [&, N_planes](hallc::DisplayPlot& plt) {
            plt._plot_data._hists3[0]->Reset();
            for (int ip = 0; ip < N_planes; ip++) {
              TClonesArray* hodoHits = _hod->GetPlane(ip)->GetHits();
              for (Int_t iphit = 0; iphit < _hod->GetPlane(ip)->GetNScinHits(); iphit++) {
                THcHodoHit* hit = (THcHodoHit*)hodoHits->At(iphit);
                // keep only those with "two good times"
                if (hit->GetTwoGoodTimes()) {
                  auto i_paddle = hit->GetPaddleNumber();
                  for (int ix = 0; ix < 16; ix++) {
                    if (ip % 2 == 0) {
                      plt._plot_data._hists3[0]->Fill(ix, i_paddle, ip + 15 * (ip > 1));
                    } else {
                      plt._plot_data._hists3[0]->Fill(i_paddle, ix, ip + 15 * (ip > 1));
                    }
                  }
                }
              }
            }
            return 0;
          });
      //
      _det_display->InitAll();
      return 0;
    }

    //___________________________________________________________________________

  } // namespace event_display
} // namespace hallc

