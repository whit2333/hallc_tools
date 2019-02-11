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
    using PlotDataMap   = std::map<int, display::PlotData*>;
    using SocketPlotMap = std::map<TSocket*, display::DisplayData*>;

    std::shared_ptr<THttpServer> _server        = nullptr; //!
    TServerSocket*               _ss            = nullptr; //! new TServerSocket(9090, kTRUE);
    TMonitor*                    _mon           = nullptr; //! new TMonitor;
    int                          _http_port     = 8888;
    std::string                  _host          = "127.0.0.1";
    int                          _sock_srv_port = 9090;
    PlotDataMap*                 _plot_map      = nullptr; /// Holds one "default" set of plots
    std::vector<TSocket*>        _running_sockets;         //!
    SocketPlotMap                _connected_clients;

  public:
    DisplayServer(int http_port = 8888, std::string host = "127.0.0.1" , int sock_srv_port = 9090 );
    virtual ~DisplayServer() {} 
     
    int StartSocketServer(int port = 9090);

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

    /** Run the server. Can be interrupted with ctrl-c .
     */
    void Run();

    ClassDef(DisplayServer,1)
  };


  /**
   *
   */
  struct EventDisplayServer : public DisplayServer {

    //SocketPlotMap _connected_replays;

    virtual void AddSocket(TSocket* s) {}
    //virtual void Update(TSocket* s, display::DisplayData*);
    //virtual void RemoveSocket(TSocket* s);
    ClassDef(EventDisplayServer,1)
  };


  /**
   *
   */
  struct MonitorDisplayServer : public DisplayServer {
    //SocketPlotMap _connected_replays;

    virtual void AddSocket(TSocket* s) {}
    //virtual void Update(TSocket* s, display::DisplayData*);
    //virtual void RemoveSocket(TSocket* s);
    ClassDef(MonitorDisplayServer,1)
  };

}

#endif

