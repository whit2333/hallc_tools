#ifndef hallc_DetectorDisplay_HH
#define hallc_DetectorDisplay_HH

#include "TH1F.h"
#include "TH2F.h"
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

namespace hallc {

  /**
   */
  class DisplayPlot {
  public:
    using InitFunction_t   = std::function<int(DisplayPlot&)>;
    using UpdateFunction_t = std::function<int(DisplayPlot&)>;

    int                _id = 0;
    TCanvas*           _canvas = nullptr;
    std::vector<TH1F*> _hists1; /// 1D histograms
    std::vector<TH2F*> _hists2; /// 2D histograms
    UpdateFunction_t    _update_lambda;
    InitFunction_t      _init_lambda;

  public:
    DisplayPlot(const DisplayPlot&) = default;
    DisplayPlot(InitFunction_t&& f_i, UpdateFunction_t&& f_u)
        : _init_lambda(std::forward<InitFunction_t>(f_i)),
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
    using InitFunction_t   = std::function<int(DisplayPlot&)>;
    using UpdateFunction_t = std::function<int(DisplayPlot&)>;

    std::shared_ptr<THttpServer>                 _server;
    std::map<int, std::shared_ptr<DisplayPlot>>  _plots;

    DetectorDisplay()
        : _server(std::make_shared<THttpServer>("http:8888?top=replay;rw")) {}

    void RegisterPlot(std::shared_ptr<DisplayPlot> plot) {
      _plots[plot->_id] = plot;
    }

    std::shared_ptr<DisplayPlot> CreateDisplayPlot(InitFunction_t&&   f_init,
                                                   UpdateFunction_t&& f_update) {
      auto plt = std::make_shared<DisplayPlot>(std::forward<InitFunction_t>(f_init),
                                               std::forward<UpdateFunction_t>(f_update));
      plt->_id = _plots.size()+1;
      RegisterPlot(plt);
      return plt;
    }

    void InitAll() {
      for(auto& [i, plt] : _plots) {
        plt->Init();
      }
    }

    void Process(){
      for(auto& [i, plt] : _plots) {
        plt->Update();
      }
    }
    void UpdateAll (){
      for(auto& [i, plt] : _plots) {
        plt->_canvas->Modified();
      }
      for(auto& [i, plt] : _plots) {
        plt->_canvas->Update();
      }
    }

    ClassDef(DetectorDisplay,2)
  };

}

#endif 
