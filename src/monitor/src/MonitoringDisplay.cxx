#include "monitor/MonitoringDisplay.h"
#include "cppitertools/zip.hpp"

#include <iostream>

namespace hallc {

  MonitoringDisplay::MonitoringDisplay()
      : _sock(new TSocket("localhost", 9090)), _data(-1, "unnamed") {
    char str[32];
    _sock->Recv(str, 32);
    // server tells us who we are
    int idx = !strcmp(str, "go") ? 0 : 1;
    _sock->SetCompressionLevel(1);
  }
  MonitoringDisplay::MonitoringDisplay(int num)
      : _sock(new TSocket("localhost", 9090)), _data(num) {
    char str[32];
    _sock->Recv(str, 32);
    // server tells us who we are
    int idx = !strcmp(str, "go") ? 0 : 1;
    _sock->SetCompressionLevel(1);
  }
  MonitoringDisplay::MonitoringDisplay(int num, std::string n)
      : _sock(new TSocket("localhost", 9090)), _data(num, n) {
    char str[32];
    _sock->Recv(str, 32);
    // server tells us who we are
    int idx = !strcmp(str, "go") ? 0 : 1;
    _sock->SetCompressionLevel(1);
  }
  MonitoringDisplay::MonitoringDisplay(std::string n)
      : _sock(new TSocket("localhost", 9090)), _data(-1, n) {
    char str[32];
    _sock->Recv(str, 32);
    // server tells us who we are
    int idx = !strcmp(str, "go") ? 0 : 1;
    _sock->SetCompressionLevel(1);
  }

  MonitoringDisplay::~MonitoringDisplay() {
      _sock->Send("Finished"); // tell server we are finished
      _sock->Close();
    }

  DisplayPlot* MonitoringDisplay::CreateDisplayPlot(std::string folder, 
                                                    std::string name, 
                                                    InitFunction_t&& f_init,
                                                    UpdateFunction_t&& f_update)
  {
    auto plt = new DisplayPlot(name, std::forward<InitFunction_t>(f_init), std::forward<UpdateFunction_t>(f_update));
    plt->_plot_data._id          = _plots.size();
    plt->_plot_data._name        = name;
    plt->_plot_data._folder_name = folder;
    //plt->_plot_data._canvas = new TCanvas(name.c_str(),name.c_str());
    RegisterPlot(plt);
    return plt;
  }

  DisplayPlot* MonitoringDisplay::CreateDisplayPlot(std::string name, 
                                                    InitFunction_t&& f_init,
                                                    UpdateFunction_t&& f_update)
  {
    return CreateDisplayPlot("/",name, std::forward<InitFunction_t>(f_init), std::forward<UpdateFunction_t>(f_update));
  }


  void MonitoringDisplay::RegisterPlot(DisplayPlot* p) {
    _plots[p->_plot_data._id]    = p;
    _data._plot_map[p->_plot_data._id] = &(p->_plot_data);
  }


  void MonitoringDisplay::InitAll() {
    for (auto& [i, plt] : _plots) {
      plt->Init();
      //if (plt->_plot_data._canvas) {
      //} else {
      //}
    }
  }

  void MonitoringDisplay::Process() {
    for (auto& [i, plt] : _plots) {
      plt->Update();
    }
  }

  void MonitoringDisplay::UpdateAll() {
    //_plot_map.clear();
    for (auto& [i, plt] : _plots) {
      plt->_plot_data._canvas->Modified();
      //_plot_map[i] = (hallc::display::PlotData*)plt;
    }
    for (auto& [i, plt] : _plots) {
      plt->_plot_data._canvas->Update();
    }
    UpdateServer();
    //TMessage::EnableSchemaEvolutionForAll(false);
    //TMessage mess(kMESS_OBJECT);
    //mess.Reset();                 // re-use TMessage object
    //mess.WriteObject(&_plot_map); // write object in message buffer
    //_sock->Send(mess);            // send message
    //std::cout << "message sent\n";
    //// messlen  += mess.Length();
    //// cmesslen += mess.CompLength();
  }

  void MonitoringDisplay::UpdateServer() {
    TMessage::EnableSchemaEvolutionForAll(false);
    TMessage mess(kMESS_OBJECT);
    mess.Reset();                 // re-use TMessage object
    mess.WriteObject(&_data); // write object in message buffer
    _sock->Send(mess);            // send message
    //std::cout << "message sent\n";
    // messlen  += mess.Length();
    // cmesslen += mess.CompLength();
  }


}

