#include "monitor/DisplayServer.h"
#include "TClass.h"

#include <algorithm>
#include <csignal>
#include <cstring>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

volatile sig_atomic_t sig_caught = 0;

void handle_sig(int signum)
{
    /* in case we registered this handler for multiple signals */
    if (signum == SIGINT) {
        sig_caught = 1;
    }
    if (signum == SIGTERM) {
        sig_caught = 2;
    }
    if (signum == SIGABRT) {
        sig_caught = 3;
    }
}


namespace hallc {

  DisplayServer::DisplayServer()
      : _server(std::make_shared<THttpServer>("http:8888?top=replay&thrds=2;rw")) {
    _server->SetCors();
    _ss  = new TServerSocket(9090, kTRUE);
    _mon = new TMonitor;
    _mon->Add(_ss);
  }

  void DisplayServer::Run() {
    //
    TMessage* mess = nullptr;
    //
    void (*prev_handler)(int);
    prev_handler = signal(SIGINT, handle_sig);
    //
    while (1) {
      if(sig_caught) {
        break;
      }
      _mon->ResetInterrupt();
      TSocket*  s = _mon->Select(1000);

      //std::cout << " s = " << s << "\n";
      if(s == (TSocket*)-1)  { 
        //std::cout << " timeout\n";
        continue;
      }

      if (s->IsA() == TServerSocket::Class()) {
        std::cout << "socket connected!\n";
        TSocket* s0 = ((TServerSocket*)s)->Accept();
        _running_sockets.push_back(s0);
        _mon->Add(s0);
        s0->Send("go");
        continue;
      }

      auto status = s->Recv(mess);
      //std::cout << " mess recv done\n";

      if(status <= 0 ) {
        // The socket finished or disconnected somehow...
        //std::cout << " removing socket " << s << "\n";
        _mon->Remove(s);
        //
        RemoveSocket(s);
        continue;
      }

      if (mess->What() == kMESS_STRING) {
        std::cout << "message  string\n";
        char str[64];
        mess->ReadString(str, 64);
        auto res = std::find(std::begin(_running_sockets), std::end(_running_sockets), s);
        auto idx = std::distance(std::begin(_running_sockets), res);
        printf("Client %d: %s\n", idx, str);
      } else if (mess->What() == kMESS_OBJECT) {
        //
        //std::cout << "message  object\n";
        printf("got object of class: %s\n", mess->GetClass()->GetName());
        auto dd  = (display::DisplayData*) mess->ReadObject(mess->GetClass()) ;
        //
        Update(s, dd);
      } else {
        printf("*** Unexpected message ***\n");
      }
      delete mess;
    }

    signal (SIGINT, prev_handler);
    Shutdown();
    // delete mess;
  }

  void DisplayServer::RemoveSocket(TSocket* s) {

  }

  void DisplayServer::Update(TSocket* s,display::DisplayData* dd) {
    if( _connected_clients.count(s) == 0) {  
      _connected_clients[s] = dd;
      for (auto& [i, data] : dd->_plot_map) {
        std::string name = std::string("/") + std::to_string(dd->_run_number) + data->_folder_name;
        _server->Register(name.c_str(), data->_canvas);
      }
    } else {
      for (auto& [i, data] : dd->_plot_map) {
        _connected_clients[s]->_plot_map[i]->Merge((dd->_plot_map)[i]);
        //std::string name = std::string("/") + std::to_string(dd->_run_number) + "/";
        //_server->Register(name.c_str(), data->_canvas);
      }
    }
    //if (!_plot_map) {
    //  _plot_map = plts;
    //  for (auto& [i, data] : *_plot_map) {
    //    std::string name = std::string("/") + std::to_string(data->_run_number);
    //    _server->Register(name.c_str() , data->_canvas);
    //  }
    //} else {
    //  for (auto& [i, data] : *_plot_map) {
    //    (*_plot_map)[i]->Merge((*plts)[i]);
    //  }
    //}
  }

  //_______________________________________________________________________
    
  void EventDisplayServer::Update(TSocket* s, display::DisplayData* dd) {
    if( _connected_clients.count(s) == 0) {  
      _connected_clients[s] = dd;
      for (auto& [i, data] : dd->_plot_map) {
        std::string name = std::string("/") + std::to_string(dd->_run_number) + data->_folder_name;
        _server->Register(name.c_str(), data->_canvas);
      }
    } else {
      for (auto& [i, data] : dd->_plot_map) {
        _connected_clients[s]->_plot_map[i]->Replace((dd->_plot_map)[i]);
        //std::string name = std::string("/") + std::to_string(dd->_run_number) + "/";
        //_server->Register(name.c_str(), data->_canvas);
      }
    }
  }

} // namespace hallc
