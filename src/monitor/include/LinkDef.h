#if defined(__ROOTCLING__)
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ all typedef;

#pragma link C++ nestedclass;
#pragma link C++ nestedtypedef;

#pragma link C++ namespace hallc;
#pragma link C++ namespace hallc::scandalizer;
#pragma link C++ namespace hallc::event_display;
#pragma link C++ namespace hallc::display;

#pragma link C++ class hallc::scandalizer::SimplePostProcess+;
#pragma link C++ class hallc::scandalizer::SpectrometerMonitor+;
#pragma link C++ class hallc::scandalizer::TrackingEfficiencyMonitor+;

#pragma link C++ class hallc::display::PlotData+;
#pragma link C++ class hallc::DisplayPlot+;
//#pragma link C++ class std::shared_ptr<hallc::DisplayPlot>+;

#pragma link C++ class std::map<int, hallc::DisplayPlot*>+;
#pragma link C++ class std::map<int, hallc::display::PlotData*>+;

#pragma link C++ class hallc::DisplayServer+;

#pragma link C++ class hallc::DetectorDisplay+;
#pragma link C++ class hallc::DisplayPostProcess+;

#pragma link C++ class hallc::event_display::BasicEventDisplay+;
#pragma link C++ class hallc::event_display::HMSEventDisplay+;
#pragma link C++ class hallc::event_display::SHMSEventDisplay+;

//#pragma link C++ class hallc::HCKinematic+;
//#pragma link C++ class std::vector<hallc::HCKinematic>+;
//#pragma link C++ class std::vector<std::pair<double, hallc::HCKinematic>>+;
//
//#pragma link C++ class hallc::HallCSetting+;
//#pragma link C++ class TargetRates+;
//#pragma link C++ class RunPlanTableEntry+;
//
//#pragma link C++ class std::vector<RunPlanTableEntry>+;
//#pragma link C++ class std::pair<double,std::vector<RunPlanTableEntry>>+;
//#pragma link C++ class std::vector<std::pair<double,std::vector<RunPlanTableEntry>>>+;

#endif
