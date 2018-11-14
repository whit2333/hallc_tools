#include "THttpServer.h"
#include <charconv>

#ifndef derp_HHH
#define  derp_HHH

void FinalizeRun(int run_number, std::string is_good) {
  int good_run = 0;
  std::from_chars(is_good.data(), is_good.data()+is_good.size(), good_run);
  std::cout << " Run : " <<  run_number << "\n";
  if( good_run) {
    std::cout << " is good!\n";
  } else {
    std::cout << " is NOT good!\n";
  }
}

class SimplePostProcess : public THaPostProcess {
public:
  using Function_t     = std::function<int(const THaEvData*)>;
  using InitFunction_t = std::function<int()>;
  Function_t     _event_lambda;
  InitFunction_t _init_lambda;

public:
  SimplePostProcess(Function_t&& f) : 
    _event_lambda(std::forward<Function_t>(f)), 
    _init_lambda([](){return 0;}) { }

  SimplePostProcess(InitFunction_t&& initf, Function_t&& f) : 
    _event_lambda(std::forward<Function_t>(f)), 
    _init_lambda(std::forward<InitFunction_t>(initf)) { }

  //SimplePostProcess()  { }
  virtual ~SimplePostProcess(){ }

  virtual Int_t Init(const TDatime& ) {

    //std::cout << derp;
    return _init_lambda();
  }
  virtual Int_t Process( const THaEvData* evt, const THaRunBase*, Int_t code ){ 
    _event_lambda(evt);
    if( evt->GetEvNum()%100 == 0) {
      gSystem->ProcessEvents();
    }
    return 0;
  }
  virtual Int_t Close(){ return 0; }
  ClassDef(SimplePostProcess,1)
};

#if defined(__ROOTCLING__)
#pragma link C++ class SimplePostProcess+;
#pragma link C++ class std::vector<SimplePostProcess*>+;
#endif

#endif

/** 
 */
void coin_online_test(Int_t RunNumber = 0, Int_t MaxEvent = 0) {

  using namespace ROOT::Experimental;
  ROOT::EnableImplicitMT(4);

  ROOT::Experimental::TTaskGroup tg;

  //// future from a packaged_task
  //std::thread(std::move(task)).detach();              // launch on a thread
  //// future from an async()
  //TFuture<int> f2 = std::async(std::launch::async, []() { return 8; });
  //// future from a promise
  //std::promise<int> p;
  //TFuture<int> f3(p.get_future());
  //std::thread([&p] { p.set_value(9); }).detach();
  //std::cout << "Waiting..." << std::flush;
  //f1.wait();
  //f2.wait();
  //f3.wait();
  //std::cout << "Done!\nResults are: " << f1.get() << ' ' << f2.get() << ' ' << f3.get() << '\n';

  THttpServer* serv = new THttpServer(Form("http:8080?top=%s;rw", "replay"));
  //THttpServer* serv = new THttpServer("http:8080;ro;noglobal");

  TH1F* h1 = new TH1F("hgcerADChits","hgcer_ADC_hits",10, 0, 10);
  tg.Run([&]() {
    serv->Register("/h1/", h1);
    std::cout << " asdf\n";
  });


  SimplePostProcess* pp1 = new SimplePostProcess(
    [&](){
      auto phgcer_tot_npe = (TH1F*)gROOT->FindObject("phgcer_tot_npe");
      serv->Register("/shms_HGC", phgcer_tot_npe);
      return 0;
    },
    [](const THaEvData* evt){
      //h1->Fill(1000);
      return 0; 
    });

  serv->RegisterCommand("/title",  "/h1/hgcerADChits/->SetTitle(\"%arg1%\");");
  serv->RegisterCommand("/finalize",(std::string("FinalizeRun(")+std::to_string(RunNumber)+std::string(",\"%arg1%\")")).c_str(), "button;rootsys/icons/ed_execute.png");

  system("xdg-open \"http://localhost:8080?layout=grid2x2&items=[h1/hgcerADChits,shms_HGC/phgcer_tot_npe]&monitoring=1000\"");

  std::string hallc_replay_dir = "/home/whit/projects/hall_AC/EXPERIMENT_replay/hallc_replay/";

  // Get RunNumber and MaxEvent if not provided.
  if(RunNumber == 0) {
    cout << "Enter a Run Number (-1 to exit): ";
    cin >> RunNumber;
    if( RunNumber<=0 ) return;
  }
  if(MaxEvent == 0) {
    cout << "\nNumber of Events to analyze: ";
    cin >> MaxEvent;
    if(MaxEvent == 0) {
      cerr << "...Invalid entry\n";
      exit;
    }
  }

  // Create file name patterns.
  const char* RunFileNamePattern = "coin_all_%05d.dat";
  vector<TString> pathList;
  pathList.push_back(".");
  pathList.push_back("./raw");
  pathList.push_back("./raw/../raw.copiedtotape");
  pathList.push_back("./cache");

  //const char* RunFileNamePattern = "raw/coin_all_%05d.dat";
  const char* ROOTFileNamePattern = "ROOTfiles/coin_replay_production_%d_%d.root";
  
  // Load global parameters
  gHcParms->Define("gen_run_number", "Run Number", RunNumber);
  gHcParms->AddString("g_ctp_database_filename", "DBASE/COIN/standard.database");
  gHcParms->Load(gHcParms->GetString("g_ctp_database_filename"), RunNumber);
  gHcParms->Load(gHcParms->GetString("g_ctp_parm_filename"));
  gHcParms->Load(gHcParms->GetString("g_ctp_kinematics_filename"), RunNumber);
  // Load params for COIN trigger configuration
  gHcParms->Load("PARAM/TRIG/tcoin.param");
  // Load fadc debug parameters
  gHcParms->Load("PARAM/HMS/GEN/h_fadc_debug.param");
  gHcParms->Load("PARAM/SHMS/GEN/p_fadc_debug.param");

  // const char* CurrentFileNamePattern = "low_curr_bcm/bcmcurrent_%d.param";
  // gHcParms->Load(Form(CurrentFileNamePattern, RunNumber));

  // Load the Hall C detector map
  gHcDetectorMap = new THcDetectorMap();
  gHcDetectorMap->Load("MAPS/COIN/DETEC/coin.map");

  //=:=:=:=
  // SHMS 
  //=:=:=:=

  // Set up the equipment to be analyzed.
  THcHallCSpectrometer* SHMS = new THcHallCSpectrometer("P", "SHMS");
  SHMS->SetEvtType(1);
  SHMS->AddEvtType(4);
  SHMS->AddEvtType(5);
  SHMS->AddEvtType(6);
  SHMS->AddEvtType(7);
  gHaApps->Add(SHMS);

  // Add Noble Gas Cherenkov to SHMS apparatus
  THcCherenkov* pngcer = new THcCherenkov("ngcer", "Noble Gas Cherenkov");
  SHMS->AddDetector(pngcer);
  // Add drift chambers to SHMS apparatus
  THcDC* pdc = new THcDC("dc", "Drift Chambers");
  SHMS->AddDetector(pdc);
  // Add hodoscope to SHMS apparatus
  THcHodoscope* phod = new THcHodoscope("hod", "Hodoscope");
  SHMS->AddDetector(phod);
  // Add Heavy Gas Cherenkov to SHMS apparatus
  THcCherenkov* phgcer = new THcCherenkov("hgcer", "Heavy Gas Cherenkov");
  SHMS->AddDetector(phgcer);
  // Add Aerogel Cherenkov to SHMS apparatus
  THcAerogel* paero = new THcAerogel("aero", "Aerogel");
  SHMS->AddDetector(paero);
  // Add calorimeter to SHMS apparatus
  THcShower* pcal = new THcShower("cal", "Calorimeter");
  SHMS->AddDetector(pcal);

  // THcBCMCurrent* hbc = new THcBCMCurrent("H.bcm", "BCM current check");
  // gHaPhysics->Add(hbc);

  // Add rastered beam apparatus
  THaApparatus* pbeam = new THcRasteredBeam("P.rb", "Rastered Beamline");
  gHaApps->Add(pbeam);
  // Add physics modules
  // Calculate reaction point
  THaReactionPoint* prp = new THaReactionPoint("P.react", "SHMS reaction point", "P", "P.rb");
  gHaPhysics->Add(prp);
  // Calculate extended target corrections
  THcExtTarCor* pext = new THcExtTarCor("P.extcor", "HMS extended target corrections", "P", "P.react");
  gHaPhysics->Add(pext);
  // Calculate golden track quantites
  THaGoldenTrack* pgtr = new THaGoldenTrack("P.gtr", "SHMS Golden Track", "P");
  gHaPhysics->Add(pgtr);
  // Calculate the hodoscope efficiencies
  THcHodoEff* peff = new THcHodoEff("phodeff", "SHMS hodo efficiency", "P.hod");
  gHaPhysics->Add(peff);   

  // Add event handler for scaler events
  THcScalerEvtHandler* pscaler = new THcScalerEvtHandler("P", "Hall C scaler event type 1");
  pscaler->AddEvtType(1);
  pscaler->AddEvtType(4);
  pscaler->AddEvtType(5);
  pscaler->AddEvtType(6);
  pscaler->AddEvtType(7);
  pscaler->AddEvtType(129);
  pscaler->SetDelayedType(129);
  pscaler->SetUseFirstEvent(kTRUE);
  gHaEvtHandlers->Add(pscaler);

  //=:=:=
  // HMS 
  //=:=:=

  // Set up the equipment to be analyzed.
  THcHallCSpectrometer* HMS = new THcHallCSpectrometer("H", "HMS");
  HMS->SetEvtType(2);
  HMS->AddEvtType(4);
  HMS->AddEvtType(5);
  HMS->AddEvtType(6);
  HMS->AddEvtType(7);
  gHaApps->Add(HMS);
  // Add drift chambers to HMS apparatus
  THcDC* hdc = new THcDC("dc", "Drift Chambers");
  HMS->AddDetector(hdc);
  // Add hodoscope to HMS apparatus
  THcHodoscope* hhod = new THcHodoscope("hod", "Hodoscope");
  HMS->AddDetector(hhod);
  // Add Cherenkov to HMS apparatus
  THcCherenkov* hcer = new THcCherenkov("cer", "Heavy Gas Cherenkov");
  HMS->AddDetector(hcer);
  // Add Aerogel Cherenkov to HMS apparatus
  // THcAerogel* haero = new THcAerogel("aero", "Aerogel");
  // HMS->AddDetector(haero);
  // Add calorimeter to HMS apparatus
  THcShower* hcal = new THcShower("cal", "Calorimeter");
  HMS->AddDetector(hcal);

  // Add rastered beam apparatus
  THaApparatus* hbeam = new THcRasteredBeam("H.rb", "Rastered Beamline");
  gHaApps->Add(hbeam);  
  // Add physics modules
  // Calculate reaction point
  THaReactionPoint* hrp = new THaReactionPoint("H.react", "HMS reaction point", "H", "H.rb");
  gHaPhysics->Add(hrp);
  // Calculate extended target corrections
  THcExtTarCor* hext = new THcExtTarCor("H.extcor", "HMS extended target corrections", "H", "H.react");
  gHaPhysics->Add(hext);
  // Calculate golden track quantities
  THaGoldenTrack* hgtr = new THaGoldenTrack("H.gtr", "HMS Golden Track", "H");
  gHaPhysics->Add(hgtr);
  // Calculate the hodoscope efficiencies
  THcHodoEff* heff = new THcHodoEff("hhodeff", "HMS hodo efficiency", "H.hod");
  gHaPhysics->Add(heff);

  // Add event handler for scaler events
  THcScalerEvtHandler *hscaler = new THcScalerEvtHandler("H", "Hall C scaler event type 4");  
  hscaler->AddEvtType(2);
  hscaler->AddEvtType(4);
  hscaler->AddEvtType(5);
  hscaler->AddEvtType(6);
  hscaler->AddEvtType(7);
  hscaler->AddEvtType(129);
  hscaler->SetDelayedType(129);
  hscaler->SetUseFirstEvent(kTRUE);
  gHaEvtHandlers->Add(hscaler);

  //=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=
  // Kinematics Modules
  //=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=

  // Add Physics Module to calculate primary (scattered electrons) beam kinematics
  THcPrimaryKine* hkin_primary = new THcPrimaryKine("H.kin.primary", "HMS Single Arm Kinematics", "H", "H.rb");
  gHaPhysics->Add(hkin_primary);
  // Add Physics Module to calculate secondary (scattered hadrons) beam kinematics
  THcSecondaryKine* pkin_secondary = new THcSecondaryKine("P.kin.secondary", "SHMS Single Arm Kinematics", "P", "H.kin.primary");
  gHaPhysics->Add(pkin_secondary);
  
  //=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=
  // Global Objects & Event Handlers
  //=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=

  // Add trigger apparatus
  THaApparatus* TRG = new THcTrigApp("T", "TRG");
  gHaApps->Add(TRG);
  // Add trigger detector to trigger apparatus
  THcTrigDet* coin = new THcTrigDet("coin", "Coincidence Trigger Information");
  // Suppress missing reference time warnings for these event types
  coin->SetEvtType(1);
  coin->AddEvtType(2);
  TRG->AddDetector(coin); 

  
  //Add coin physics module THcCoinTime::THcCoinTime (const char *name, const char* description, const char* hadArmName, 
  // const char* elecArmName, const char* coinname) :
  THcCoinTime* coinTime = new THcCoinTime("CTime", "Coincidende Time Determination", "P", "H", "T.coin");
  gHaPhysics->Add(coinTime);

  // Add event handler for prestart event 125.
  THcConfigEvtHandler* ev125 = new THcConfigEvtHandler("HC", "Config Event type 125");
  gHaEvtHandlers->Add(ev125);
  // Add event handler for EPICS events
  THaEpicsEvtHandler* hcepics = new THaEpicsEvtHandler("epics", "HC EPICS event type 180");
  gHaEvtHandlers->Add(hcepics);
 
  // Set up the analyzer - we use the standard one,
  // but this could be an experiment-specific one as well.
  // The Analyzer controls the reading of the data, executes
  // tests/cuts, loops over Acpparatus's and PhysicsModules,
  // and executes the output routines.
  THcAnalyzer* analyzer = new THcAnalyzer;

  // A simple event class to be output to the resulting tree.
  // Creating your own descendant of THaEvent is one way of
  // defining and controlling the output.
  THaEvent* event = new THaEvent;

  // Define the run(s) that we want to analyze.
  // We just set up one, but this could be many.
  THcRun* run = new THcRun( pathList, Form(RunFileNamePattern, RunNumber) );

  // Set to read in Hall C run database parameters
  run->SetRunParamClass("THcRunParameters");
  
  // Eventually need to learn to skip over, or properly analyze the pedestal events
  run->SetEventRange(1, MaxEvent); // Physics Event number, does not include scaler or control events.
  run->SetNscan(1);
  run->SetDataRequired(0x7);
  run->Print();

  // Define the analysis parameters
  TString ROOTFileName = Form(ROOTFileNamePattern, RunNumber, MaxEvent);
  analyzer->SetCountMode(2);  // 0 = counter is # of physics triggers
                              // 1 = counter is # of all decode reads
                              // 2 = counter is event number

  auto pp2 = new SimplePostProcess(
    [&h1,phgcer](const THaEvData* evt){
      h1->Fill(phgcer->GetCerNPE());
      return 0; 
    });
  //analyzer->AddPostProcess(pp1);
  analyzer->AddPostProcess(pp2);

  analyzer->SetEvent(event);
  // Set EPICS event type
  analyzer->SetEpicsEvtType(180);
  // Define crate map
  analyzer->SetCrateMapFileName("MAPS/db_cratemap.dat");
  // Define output ROOT file
  analyzer->SetOutFile(ROOTFileName.Data());
  // Define DEF-file+
  analyzer->SetOdefFile("DEF-files/COIN/PRODUCTION/coin_production_hElec_pProt.def");
  // Define cuts file
  analyzer->SetCutFile("DEF-files/COIN/PRODUCTION/CUTS/coin_production_cuts.def");  // optional
  // File to record accounting information for cuts
  analyzer->SetSummaryFile(Form("REPORT_OUTPUT/COIN/PRODUCTION/summary_production_%d_%d.report", RunNumber, MaxEvent));  // optional
  // Start the actual analysis.
  analyzer->Process(run);
  // Create report file from template
  analyzer->PrintReport("TEMPLATES/COIN/PRODUCTION/coin_production.template",
  			Form("REPORT_OUTPUT/COIN/PRODUCTION/replay_coin_production_%d_%d.report", RunNumber, MaxEvent));  // optional


  tg.Wait();

  {
    std::cout << " Monitoring Histograms :\n";
   
    std::string monitor_dir = std::string("monitoring/")+std::to_string(RunNumber);
    gSystem->mkdir(monitor_dir.c_str(), true);

    ofstream   monitor_output(monitor_dir+"/hists.json");
    
    if( auto hkin_Q2 = (TH1F*)gROOT->FindObject("hkin_Q2") ){
      monitor_output  << TBufferJSON::ToJSON(hkin_Q2);
    }
    
  }

}
