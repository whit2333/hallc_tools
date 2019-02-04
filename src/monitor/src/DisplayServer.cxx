#include "monitor/DisplayServer.h"
#include "TClass.h"

namespace hallc {

  DisplayServer::DisplayServer()
      : _server(std::make_shared<THttpServer>("http:8888?top=replay&thrds=2;rw")) {
    _server->SetCors();
    _ss  = new TServerSocket(9090, kTRUE);
    _mon = new TMonitor;
    _mon->Add(_ss);
  }

  void DisplayServer::Run() {
    TMessage* mess;

    while (1) {

      TSocket*  s = _mon->Select();

      if (s->IsA() == TServerSocket::Class()) {
        std::cout << "socket connected!\n";
        TSocket* s0 = ((TServerSocket*)s)->Accept();
        _running_sockets.push_back(s0);
        _mon->Add(s0);
        s0->Send("go");
        continue;
      }

      std::cout << " mess recv \n";
      s->Recv(mess);
      std::cout << " mess recv done\n";

      if (mess->What() == kMESS_STRING) {
        std::cout << "message  string\n";
        char str[64];
        mess->ReadString(str, 64);
        auto res = std::find(std::begin(_running_sockets), std::end(_running_sockets), s);
        auto idx = std::distance(std::begin(_running_sockets), res);
        printf("Client %d: %s\n", idx, str);
        //   mon->Remove(s);
        //   if (mon->GetActive() == 0) {
        //      printf("No more active clients... stopping\n");
        //      break;
        //   }
      } else if (mess->What() == kMESS_OBJECT) {
        printf("got object of class: %s\n", mess->GetClass()->GetName());
        auto plts  = (std::map<int, hallc::display::PlotData*>*)mess->ReadObject(mess->GetClass()) ;
        if(!_plot_map) {
          _plot_map = plts;
          for(auto& [i,data] : *_plot_map) {
            _server->Register("/plots",data->_canvas);
          }
        } else {
          for(auto& [i,data] : *_plot_map) {
            (*_plot_map)[i]->Merge((*plts)[i]);
          }
        }
        //   TH1 *h = (TH1 *)mess->ReadObject(mess->GetClass());
        //   if (h) {
        //      if (s == s0)
        //         pad1->cd();
        //      else
        //         pad2->cd();
        //      h->Print();
        //      h->DrawCopy();  //draw a copy of the histogram, not the histo itself
        //      c1->Modified();
        //      c1->Update();
        //      delete h;       // delete histogram
        //   }
      } else {
        printf("*** Unexpected message ***\n");
      }
    }
    // delete mess;
  }

} // namespace hallc

