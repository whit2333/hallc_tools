#if defined(__ROOTCLING__)
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ all typedef;

#pragma link C++ nestedclass;
#pragma link C++ nestedtypedef;

#pragma link C++ namespace hallc;
#pragma link C++ namespace calibration;

#pragma link C++ class std::vector<Double_t>+;
#pragma link C++ class hallc::calibration::THcPShHit+;
#pragma link C++ class hallc::calibration::THcPShTrack+;
#pragma link C++ class hallc::calibration::pmt_hit+;
#pragma link C++ class hallc::calibration::CalHit+;
#pragma link C++ class hallc::calibration::ShowerTrack+;
#pragma link C++ class hallc::calibration::ShowerTrackInfo+;

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
//
//#pragma link C++ class hallc::MonitoringHistograms+;
//#pragma link C++ class std::vector<hallc::MonitoringHistograms>+;


#endif
