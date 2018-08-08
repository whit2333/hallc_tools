#ifndef hallc_RunStatus_HH
#define hallc_RunStatus_HH

#include <array>
#include <map>
#include <vector>

#include "TH1F.h"
#include "THStack.h"


namespace hallc {

  /** RunStatus.
   *  The accumulated or replay status of a run.
   */
  class RunStatus {
  public:
    int _RunNumber       = 0;
    int _KinematicNumber = 0;
    int _DesiredCounts   = 1;
    int _CollectedCounts = 0;

    //ClassDef(RunStatus,1)
  };

  /** KinematicSet is a small set of runs at the same kinematics.
   *  Usually at different kinematics but it can also be used to break up
   *  long runs. 
   */
  struct KinematicSet {
    int                              _Number    = 0;
    double                           _TotalTime = 0.0;
    std::map<int, double>            _RunTimes  = {};
    std::vector<std::pair<int, int>> _ZATargets{};
  };

  /** RunSet is a group of runs having some similar configuration or setting.
   *  For example all the runs at the same Q2 (but different values of x)
   */
  struct RunSet {
    int                    _Number = 0;
    int                    _NRuns  = 0;
    double                 _Q2     = 0.0;
    double                 _x      = 0.0;
    std::vector<RunStatus> _Runs   = {};

    void Add(const RunStatus& rs ) { 
      _Runs.push_back(rs);
      _NRuns = _Runs.size();
    }
    //ClassDef(RunSet,1)
  };

  /** MonitoringHistograms.
   *  Contains all the histograms to be displayed for monitoring.
   */
  class MonitoringHistograms {
  private:
    std::vector<TH1F*> _Hists1;  // target histograms
    std::vector<TH1F*> _Hists2;  // status histograms
    std::vector<THStack*> _Stacks;

  protected:
    void AddHist1(TH1F* h1) { 
      _Hists1.push_back(h1);
    }
    void AddHist2(TH1F* h1) { 
      _Hists2.push_back(h1);
    }

  public:
    MonitoringHistograms(const hallc::RunSet& run_set){

      TH1F* h1 = new TH1F("h1","h1",run_set._NRuns,0,run_set._NRuns);
      TH1F* h2 = new TH1F("h2","h1",run_set._NRuns,0,run_set._NRuns);
      int ibin = 0;
      for(auto& arun : run_set._Runs) {
        ibin++;
        h1->SetBinContent(ibin,arun._DesiredCounts);
        h2->SetBinContent(ibin,arun._CollectedCounts);
      }
      h1->SetFillColor(8);
      //h1->SetFillStyle(3002);
      h2->SetFillColor(2);
      THStack* hs = new THStack("h_setting_times","totals ; run number ");
      hs->Add(h1);
      hs->Add(h2);
      AddHist1(h1);
      AddHist2(h2);
      _Stacks.push_back(hs);
    }

    THStack* GetStack(unsigned int i=0) {
      if(_Stacks.size() > i){
        return _Stacks[i];
      }
      return nullptr;
    }


  };

}

#if defined(__ROOTCLING__)
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ all typedef;

#pragma link C++ nestedclass;
#pragma link C++ nestedtypedef;

#pragma link C++ namespace hallc;

#pragma link C++ class hallc::RunStatus+;
#pragma link C++ class std::vector<hallc::RunStatus>+;
#pragma link C++ class std::map<int, hallc::RunStatus>+;

#pragma link C++ class hallc::RunSet+;
#pragma link C++ class std::vector<hallc::RunSet>+;

#pragma link C++ class hallc::MonitoringHistograms+;
#pragma link C++ class std::vector<hallc::MonitoringHistograms>+;

#endif

#endif
