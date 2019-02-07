#ifndef hallc_DisplayServer_HH
#define hallc_DisplayServer_HH

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
#include "monitor/DisplayPlots.h"
//#include "cppitertools/zip.hpp"

namespace hallc {

  struct DisplayServer  {
  public:
    std::shared_ptr<THttpServer> _server  = nullptr; //!
    TServerSocket*               _ss      = nullptr; //! new TServerSocket(9090, kTRUE);
    TMonitor*                    _mon     = nullptr; //! new TMonitor;
    std::vector<TSocket*>        _running_sockets;   //!
    using PlotDataMap =  std::map<int, display::PlotData*>;
    using SocketPlotMap = std::map<TSocket*, display::DisplayData*>;

    // Holds one "default" set of plot
    PlotDataMap* _plot_map = nullptr;

    SocketPlotMap _connected_clients;

  public:
    DisplayServer();
    virtual ~DisplayServer() {} 

    virtual void AddSocket(TSocket* s) {}

    /** Virtual method used when the socket is removed.
     * This should be used to update the status of the associated socket data,
     * such as update or finalize run plots.
     *
     */
    virtual void RemoveSocket(TSocket* s);

    /** Update plots. This determines the overall behavior.
     *  
     *  Event display -> over write old plots (run dependent)
     *  Exp. monitor -> Add to existing plots (all runs with same config)
     *  Detector monitor ->  Add or average plots over run.
     *
     */
    virtual void Update(TSocket* s, display::DisplayData*);

    virtual void Shutdown() {}

    void Run();

    ClassDef(DisplayServer,1)
  };


  /**
   *
   */
  struct EventDisplayServer : public DisplayServer {

    SocketPlotMap _connected_replays;

    virtual void AddSocket(TSocket* s) {
    }

    virtual void Update(TSocket* s, display::DisplayData*);
  };

}

#endif

