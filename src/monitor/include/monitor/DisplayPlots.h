#ifndef hallc_DisplayPlots_HH
#define hallc_DisplayPlots_HH

#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"

#include "podd2/Logger.h"

namespace hallc {

  namespace display {

    /** Plot data .
     *
     */
    struct PlotData {
      PlotData(){}
      PlotData(std::string name) : _name(name) {}

      int                _id         = 0;
      int                _run_number = 0;
      std::string        _name       = "";
      std::string        _folder_name= "/";
      TCanvas*           _canvas     = nullptr;
      std::vector<TH1F*> _hists1; /// 1D histograms
      std::vector<TH2F*> _hists2; /// 2D histograms
      std::vector<TH3F*> _hists3; /// 2D histograms

      void Merge(PlotData* d);

      /** Replace this data with d data.
       *
       */
      void Replace(PlotData* d);
      ClassDef(PlotData,1)
    };


    /** Display data. 
     *  This forms the message that is passed between the (display) server and the client (replay).
     *  How the server chooses to use the this data depends on the type of display server.
     *
     *  - An "event display server" might replace the existing plots for a given run.  
     *  - A "detector monitor server" might merge existing histograms together as more statistics are accumulated for each run
     *  - An "experment monitor server" has a group of runs it merges together to monitor the progress of data collection or look at online results.
     *
     */
    struct DisplayData {
      using PlotDataMap       = std::map<int, hallc::display::PlotData*>;

      int         _run_number = 1;
      std::string _name = "";
      PlotDataMap _plot_map;
      int         _id         = 1;

      DisplayData(int rn = 0, std::string n = "") : _run_number(rn), _name(n) {}

      ClassDef(DisplayData, 1)
    };

  } // namespace display

  /** Display Plot.
   *  The Display plot is used to construct a display::PlotData.
   *  The class display::DisplayData contains the collection of plots (canvases, histograms,etc).
   *  Each plot in the collection (each constructed by a DispalyPlot) has its data stored in a
   *  display::DisplayData collection. This collection is constructed in subsquent classes (see MonitoringDisplay).
   */
  class DisplayPlot  {
  public:
    using InitFunction_t   = std::function<int(DisplayPlot&)>;
    using UpdateFunction_t = std::function<int(DisplayPlot&)>;

    UpdateFunction_t  _update_lambda;
    InitFunction_t    _init_lambda;
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

    std::string GetName() const { return _plot_data._name; }
    std::string GetFolderName() const { return _plot_data._folder_name; }
    int GetRunNumber() const { return _plot_data._run_number; }
    int GetId() const { return _plot_data._id; }

    int Init() {
      _init_lambda(*this);
      return 0;
    }
    int Update() {
      _update_lambda(*this);
      return 0;
    }
    ClassDef(DisplayPlot, 1)
  };
}

#endif

