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
#include "monitor/DetectorDisplay.h"

namespace hallc {

  struct DisplayServer  {
  public:
    std::shared_ptr<THttpServer> _server  = nullptr; //!
    TServerSocket*               _ss      = nullptr; //! new TServerSocket(9090, kTRUE);
    TMonitor*                    _mon     = nullptr; //! new TMonitor;
    std::vector<TSocket*>        _running_sockets;   //!

    std::map<int, hallc::display::PlotData*>* _plot_map = nullptr;

  public:
    DisplayServer();

    void Run();

    ClassDef(DisplayServer,1)
  };
}

#endif

