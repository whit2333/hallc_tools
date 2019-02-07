#ifndef hallc_MonitoringDisplay_HH
#define hallc_MonitoringDisplay_HH 1

#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TMessage.h"
#include "TMonitor.h"
#include "TServerSocket.h"
#include "TSocket.h"
#include "TSystem.h"

#include <functional>
#include <map>
#include <memory>
#include <vector>

#include "cppitertools/zip.hpp"
#include "monitor/DisplayPlots.h"
#include "podd2/Logger.h"

namespace hallc {

  /** Monitoring Dispaly.
   *
   * Base class for DetectorDisplay and ExperimentMonitor.
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
  struct MonitoringDisplay {
  public:
    using InitFunction_t   = std::function<int(DisplayPlot&)>;
    using UpdateFunction_t = std::function<int(DisplayPlot&)>;

    TSocket*                                 _sock = nullptr; //!
    std::map<int, DisplayPlot*>              _plots;
    //std::map<int, hallc::display::PlotData*> _plot_map; // for passing over socket
    display::DisplayData                     _data;

    int _event_number = 0;

  public:
    MonitoringDisplay();
    MonitoringDisplay(int num);
    MonitoringDisplay(int num, std::string n);
    MonitoringDisplay(std::string n);

    virtual ~MonitoringDisplay();

    /** Constructs a DisplayPlot. This constructs display::PlotData which contains
     * everything but the init and update functions.
     */
    DisplayPlot* CreateDisplayPlot(std::string        folder, 
                                   std::string        name, 
                                   InitFunction_t&&   f_init,
                                   UpdateFunction_t&& f_update);
    DisplayPlot* CreateDisplayPlot(std::string        name, 
                                   InitFunction_t&&   f_init,
                                   UpdateFunction_t&& f_update);

    display::DisplayData* GetDisplayData() { return &_data; }

    /** Register plot DisplayData.  DisplayData is then sent to display server.
     */
    virtual void RegisterPlot(DisplayPlot* plot);

    /**
     */
    virtual void InitAll();

    /** Process  is called for each event that we would like to include in the display.
     *  It does not update the display.
     */
    virtual void Process();

    /** Update all the plots in the display with all the process calls since the last update.
     */
    virtual void UpdateAll();

    virtual void UpdateServer();

    ClassDef(MonitoringDisplay, 2)
  };

} // namespace hallc

#endif

