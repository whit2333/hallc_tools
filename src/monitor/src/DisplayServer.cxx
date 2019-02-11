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

void handle_sig(int signum) {
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

  DisplayServer::DisplayServer(int http_port, std::string host, int sock_srv_port)
      : _http_port(http_port), _host(host), _sock_srv_port(sock_srv_port),
        _server(std::make_shared<THttpServer>((std::string("http:") + host + ":" +
                                               std::to_string(http_port) +
                                               std::string("?top=replay&thrds=2;rw"))
                                                  .c_str())) {
    _server->SetCors();
    _mon = new TMonitor;
  }

  int DisplayServer::StartSocketServer(int port) {
    _sock_srv_port = port;
    _ss            = new TServerSocket(_sock_srv_port, kTRUE);
    _mon->Add(_ss);
    return port;
  }

  void DisplayServer::Run() {
    //
    TMessage* mess = nullptr;
    //
    void (*prev_handler)(int);
    prev_handler = signal(SIGINT, handle_sig);
    //
    while (1) {
      if (sig_caught) {
        break;
      }
      _mon->ResetInterrupt();
      TSocket* s = _mon->Select(1000);

      if (s == (TSocket*)-1) {
        // std::cout << " timeout\n";
        continue;
      }

      if (s->IsA() == TServerSocket::Class()) {
        // std::cout << "socket connected!\n";
        TSocket* s0 = ((TServerSocket*)s)->Accept();
        // std::cout << " error code : " << s0->GetErrorCode() << "\n";
        _running_sockets.push_back(s0);
        _mon->Add(s0);
        s0->Send("go");
        continue;
      }

      auto status = s->Recv(mess);
      // std::cout << " mess recv done\n";

      if (status <= 0) {
        // The socket finished or disconnected somehow...
        // std::cout << " removing socket " << s << "\n";
        //
        RemoveSocket(s);
        _mon->Remove(s);
        continue;
      }

      if (mess->What() == kMESS_STRING) {
        // std::cout << "message  string\n";
        char str[64];
        mess->ReadString(str, 64);
        auto res = std::find(std::begin(_running_sockets), std::end(_running_sockets), s);
        auto idx = std::distance(std::begin(_running_sockets), res);
        printf("Client %d: %s\n", idx, str);
      } else if (mess->What() == kMESS_OBJECT) {
        //
        // std::cout << "message  object\n";
        // printf("got object of class: %s\n", mess->GetClass()->GetName());
        auto dd = (display::DisplayData*)mess->ReadObject(mess->GetClass());
        //
        Update(s, dd);
      } else {
        printf("*** Unexpected message ***\n");
      }
      delete mess;
    }

    signal(SIGINT, prev_handler);
    Shutdown();
    // delete mess;
  }

  void DisplayServer::Update(TSocket* s, display::DisplayData* dd) {
    if (_connected_clients.count(s) == 0) {
      _connected_clients[s] = dd;
      for (auto& [i, data] : dd->_plot_map) {
        std::string name = dd->GetFolder() + data->_folder_name;
        // std::cout << "hiding : " <<  name << "\n";
        _server->Register(name.c_str(), data->_canvas);
        //_server->Hide(name.c_str(), false);
      }
    } else {
      for (auto& [i, data] : dd->_plot_map) {
        _connected_clients[s]->_plot_map[i]->Merge((dd->_plot_map)[i]);
        // std::string name = std::string("/") + std::to_string(dd->_run_number) + "/";
        //_server->Register(name.c_str(), data->_canvas);
      }
    }
  }

  void DisplayServer::RemoveSocket(TSocket* s) {
    if (_connected_clients.count(s) != 0) {
      auto dd = _connected_clients[s];
      // std::string name = dd->GetFolder();// + data->_folder_name;
      // std::string name = std::string("/") + std::to_string(dd->_run_number);// +
      // data->_folder_name;
      for (auto& [i, data] : dd->_plot_map) {
        std::string name = dd->GetFolder() + data->_folder_name;
        _server->Unregister(data->_canvas);
        std::cout << "hiding : " << name << "\n";
        _server->Hide(name.c_str());
        auto iter = _connected_clients.find(s);
        if (iter != std::end(_connected_clients)) {
          std::cout << "erasing \n";
          _connected_clients.erase(iter);
        }
        for (auto& h1 : data->_hists1) {
          delete h1;
        }
        for (auto& h2 : data->_hists2) {
          delete h2;
        }
        for (auto& h3 : data->_hists3) {
          delete h3;
        }
        for (auto&& g1 : data->_graphs1) {
          delete g1;
        }
        // std::string name = std::string("/") + std::to_string(dd->_run_number) + "/";
        //_server->Register(name.c_str(), data->_canvas);
      }
    }
  }

  //_______________________________________________________________________

  // void EventDisplayServer::Update(TSocket* s, display::DisplayData* dd) {
  //  if( _connected_clients.count(s) == 0) {
  //    _connected_clients[s] = dd;
  //    for (auto& [i, data] : dd->_plot_map) {
  //      //std::string name = std::string("/") + std::to_string(dd->_run_number) +
  //      data->_folder_name; std::string name = dd->GetFolder() + data->_folder_name;
  //      _server->Register(name.c_str(), data->_canvas);
  //      std::cout << "unhiding : " <<  name << "\n";
  //      _server->Hide(name.c_str(), false);
  //    }
  //  } else {
  //    for (auto& [i, data] : dd->_plot_map) {
  //      _connected_clients[s]->_plot_map[i]->Replace((dd->_plot_map)[i]);
  //      //std::string name = std::string("/") + std::to_string(dd->_run_number) + "/";
  //      //_server->Register(name.c_str(), data->_canvas);
  //    }
  //  }
  //}

  // void EventDisplayServer::RemoveSocket(TSocket* s) {
  //  if( _connected_clients.count(s) != 0) {
  //    auto        dd   = _connected_clients[s];
  //    //std::string name = dd->GetFolder();
  //    // std::string name = std::string("/monitoring/") + std::to_string(dd->_run_number);// +
  //    // data->_folder_name;
  //    for (auto& [i, data] : dd->_plot_map) {
  //      std::string name = dd->GetFolder() + data->_folder_name;
  //      _server->Unregister(data->_canvas);
  //      std::cout << "hiding : " <<  name << "\n";
  //      _server->Hide(name.c_str());
  //      auto iter = _connected_clients.find(s);
  //      if(iter != std::end(_connected_clients) ){
  //        std::cout << "erasing \n";
  //        _connected_clients.erase(iter);
  //      }
  //      for(auto& h1 : data->_hists1 ) {
  //        delete h1;
  //      }
  //      for(auto& h2 : data->_hists2 ) {
  //        delete h2;
  //      }
  //      for(auto& h3 : data->_hists3 ) {
  //        delete h3;
  //      }
  //      //std::string name = std::string("/") + std::to_string(dd->_run_number) + "/";
  //      //_server->Register(name.c_str(), data->_canvas);
  //    }
  //  }
  //}

  ////_______________________________________________________________________
  //
  // void MonitorDisplayServer::Update(TSocket* s, display::DisplayData* dd) {
  //  if( _connected_clients.count(s) == 0) {
  //    _connected_clients[s] = dd;
  //    for (auto& [i, data] : dd->_plot_map) {
  //      std::string name = dd->GetFolder() + data->_folder_name;
  //      _server->Register(name.c_str(), data->_canvas);
  //      std::cout << "unhiding : " <<  name << "\n";
  //      _server->Hide(name.c_str(), false);
  //    }
  //  } else {
  //    for (auto& [i, data] : dd->_plot_map) {
  //      _connected_clients[s]->_plot_map[i]->Replace((dd->_plot_map)[i]);
  //    }
  //  }
  //}

  // void MonitorDisplayServer::RemoveSocket(TSocket* s) {
  //  if( _connected_clients.count(s) != 0) {
  //    auto        dd   = _connected_clients[s];
  //    //std::string name = dd->GetFolder();
  //    // std::string name = std::string("/monitoring/") + std::to_string(dd->_run_number);// +
  //    // data->_folder_name;
  //    for (auto& [i, data] : dd->_plot_map) {
  //      std::string name = dd->GetFolder() + data->_folder_name;
  //      _server->Unregister(data->_canvas);
  //      std::cout << "hiding : " <<  name << "\n";
  //      _server->Hide(name.c_str());
  //      auto iter = _connected_clients.find(s);
  //      if(iter != std::end(_connected_clients) ){
  //        std::cout << "erasing \n";
  //        _connected_clients.erase(iter);
  //      }
  //      for(auto& h1 : data->_hists1 ) {
  //        delete h1;
  //      }
  //      for(auto& h2 : data->_hists2 ) {
  //        delete h2;
  //      }
  //      for(auto& h3 : data->_hists3 ) {
  //        delete h3;
  //      }
  //      //std::string name = std::string("/") + std::to_string(dd->_run_number) + "/";
  //      //_server->Register(name.c_str(), data->_canvas);
  //    }
  //  }
  //}

} // namespace hallc

