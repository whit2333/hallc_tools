#ifndef hallc_DetectorDisplay_HH
#define hallc_DetectorDisplay_HH

#include "TSystem.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TCanvas.h"
#include "THttpServer.h"
#include <vector>
#include <map>
#include <memory>
#include <functional>

#include "THcParmList.h"
#include "CodaDecoder.h"
#include "THaEvData.h"
#include "THaPostProcess.h"

#include "TMessage.h"
#include "TMonitor.h"
#include "TServerSocket.h"
#include "TSocket.h"

#include "podd2/Logger.h"

#include "cppitertools/zip.hpp"

namespace hallc {

  namespace display {

    /**
     */
    struct PlotData {
      PlotData(){}
      PlotData(std::string name) : _name(name) {}

      int                _id     = 0;
      TCanvas*           _canvas = nullptr;
      std::vector<TH1F*> _hists1; /// 1D histograms
      std::vector<TH2F*> _hists2; /// 2D histograms
      std::vector<TH3F*> _hists3; /// 2D histograms
      std::string        _name = "";

      void Merge(PlotData* d){
        for(auto&& [h1,h2] : iter::zip(_hists1, d->_hists1)) {
          (*h1) = (*h2);
        }
        for(auto&& [h1,h2] : iter::zip(_hists2, d->_hists2)) {
          (*h1) = (*h2);
        }
      }

      ClassDef(PlotData,1)
    };

  } // namespace display


  /** Display Plot.
   *
   */
  class DisplayPlot  {
  public:
    using InitFunction_t   = std::function<int(DisplayPlot&)>;
    using UpdateFunction_t = std::function<int(DisplayPlot&)>;

    UpdateFunction_t   _update_lambda;
    InitFunction_t     _init_lambda;

    display::PlotData _plot_data;

  public:
    DisplayPlot() : _plot_data("empty") {}
    DisplayPlot(const DisplayPlot&) = default;
    DisplayPlot(std::string name, InitFunction_t&& f_i, UpdateFunction_t&& f_u)
        : _plot_data(name), _init_lambda(std::forward<InitFunction_t>(f_i)),
          _update_lambda(std::forward<UpdateFunction_t>(f_u)) {}
    DisplayPlot(InitFunction_t&& f_i, UpdateFunction_t&& f_u)
        : _plot_data("unnamed"), _init_lambda(std::forward<InitFunction_t>(f_i)),
          _update_lambda(std::forward<UpdateFunction_t>(f_u)) {}

    int Init() {
      _init_lambda(*this);
      return 0;
    }
    int Update() {
      _update_lambda(*this);
      return 0;
    }
    ClassDef(DisplayPlot,1)
  };


  /** Detector Displays.
   *
   * \code{.cpp}
   * auto plt2 = ddisplay->CreateDisplayPlot(
   *     [&](hallc::DisplayPlot& plt) {
   *       plt._canvas = new TCanvas();
   *       // initialize plots here
   *       return 0;
   *     },
   *     [&](hallc::DisplayPlot& plt) {
   *       // update histograms here
   *       return 0;
   *     });
   * \endcode
   *
   */
  struct DetectorDisplay  {
  public:
    using InitFunction_t   = std::function<int(DisplayPlot&)>;
    using UpdateFunction_t = std::function<int(DisplayPlot&)>;

    TSocket*                                 _sock = nullptr; //!
    std::map<int, DisplayPlot*>              _plots;
    std::map<int, hallc::display::PlotData*> _plot_map; // for passing over socket
    //std::shared_ptr<THttpServer>                 _server = nullptr;

  public:
    DetectorDisplay();
    virtual ~DetectorDisplay(){
      _sock->Send("Finished");          // tell server we are finished
      _sock->Close();
    }

    DisplayPlot* CreateDisplayPlot(std::string name, InitFunction_t&& f_init,
                                   UpdateFunction_t&& f_update);

    void                         RegisterPlot(DisplayPlot* plot);
    void                         InitAll();
    void                         Process();
    void                         UpdateAll();

    ClassDef(DetectorDisplay,2)
  };


  /** Simple  PostProcess implementation with some lambda hooks.
   *
   */
  class DisplayPostProcess : public podd2::AnalysisLogging<THaPostProcess> {
  private:
    int _counter      = 0;
    int _N_ped_skip   = 1200;
    int _N_event_skip = 1000;

  public:
    int              _run_number  = 0;
    DetectorDisplay* _det_display = nullptr;

  public:
    DisplayPostProcess(DetectorDisplay* d)
        : podd2::AnalysisLogging<THaPostProcess>(), _det_display(d) {}

    DisplayPostProcess() : podd2::AnalysisLogging<THaPostProcess>() {}
    virtual ~DisplayPostProcess() {}

    virtual Int_t Init(const TDatime&);
    virtual Int_t Process(const THaEvData* evt, const THaRunBase*, Int_t code);
    virtual Int_t Close();

    Int_t WritePlots();

    ClassDef(DisplayPostProcess, 1)
  };
}

#endif 
