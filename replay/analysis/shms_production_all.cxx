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
 *
 */
void shms_production_all(Int_t RunNumber = 0, Int_t MaxEvent = 0) {

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

  std::string hallc_replay_dir = "/home/whit/projects/hall_AC_projects/new_hallc_replay/EXPERIMENT_replay/hallc_replay/";

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
  const char* RunFileNamePattern = "shms_all_%05d.dat";
  vector<TString> pathList;
  pathList.push_back(".");
  pathList.push_back("./raw");
  pathList.push_back("./raw/../raw.copiedtotape");
  pathList.push_back("./cache");

  const char* ROOTFileNamePattern = "rootfiles/shms_replay_production_all_%d_%d.root";

  // Load global parameters
  gHcParms->Define("gen_run_number", "Run Number", RunNumber);
  gHcParms->AddString("g_ctp_database_filename", (hallc_replay_dir+"DBASE/SHMS/standard.database").c_str() );
  gHcParms->Load(gHcParms->GetString("g_ctp_database_filename"), RunNumber);
  gHcParms->Load(gHcParms->GetString("g_ctp_parm_filename"));
  gHcParms->Load(gHcParms->GetString("g_ctp_kinematics_filename"), RunNumber);
  // Load parameters for SHMS trigger configuration
  gHcParms->Load((hallc_replay_dir+"PARAM/TRIG/tshms.param").c_str());
  // Load fadc debug parameters
  gHcParms->Load((hallc_replay_dir+"PARAM/SHMS/GEN/p_fadc_debug.param").c_str());

  // Load the Hall C detector map
  gHcDetectorMap = new THcDetectorMap();
  gHcDetectorMap->Load((hallc_replay_dir+"MAPS/SHMS/DETEC/STACK/shms_stack.map").c_str());

  // Add trigger apparatus
  THaApparatus* TRG = new THcTrigApp("T", "TRG");
  gHaApps->Add(TRG);
  // Add trigger detector to trigger apparatus
  THcTrigDet* shms = new THcTrigDet("shms", "SHMS Trigger Information");
  TRG->AddDetector(shms);

  // Set up the equipment to be analyzed
  THcHallCSpectrometer* SHMS = new THcHallCSpectrometer("P", "SHMS");
  gHaApps->Add(SHMS);
  // Add Noble Gas Cherenkov to SHMS apparatus
  THcCherenkov* ngcer = new THcCherenkov("ngcer", "Noble Gas Cherenkov");
  SHMS->AddDetector(ngcer);
  // Add drift chambers to SHMS apparatus
  THcDC* dc = new THcDC("dc", "Drift Chambers");
  SHMS->AddDetector(dc);
  // Add hodoscope to SHMS apparatus
  THcHodoscope* hod = new THcHodoscope("hod", "Hodoscope");
  SHMS->AddDetector(hod);
  // Add Heavy Gas Cherenkov to SHMS apparatus
  THcCherenkov* hgcer = new THcCherenkov("hgcer", "Heavy Gas Cherenkov");
  SHMS->AddDetector(hgcer);
  // Add Aerogel Cherenkov to SHMS apparatus
  THcAerogel* aero = new THcAerogel("aero", "Aerogel");
  SHMS->AddDetector(aero);
  // Add calorimeter to SHMS apparatus
  THcShower* cal = new THcShower("cal", "Calorimeter");
  SHMS->AddDetector(cal);

  auto pp2 = new SimplePostProcess(
    [&h1,hgcer](const THaEvData* evt){
      h1->Fill(hgcer->GetCerNPE());
      return 0; 
    });


  // Add rastered beam apparatus
  THaApparatus* beam = new THcRasteredBeam("P.rb", "Rastered Beamline");
  gHaApps->Add(beam);
  // Add physics modules
  // Calculate reaction point
  THaReactionPoint* prp = new THaReactionPoint("P.react", "SHMS reaction point", "P", "P.rb");
  gHaPhysics->Add(prp);
  // Calculate extended target corrections
  THcExtTarCor* pext = new THcExtTarCor("P.extcor", "HMS extended target corrections", "P", "P.react");
  gHaPhysics->Add(pext);
  // Calculate golden track quantites
  THaGoldenTrack* gtr = new THaGoldenTrack("P.gtr", "SHMS Golden Track", "P");
  gHaPhysics->Add(gtr);
  // Calculate primary (scattered beam - usually electrons) kinematics
  THcPrimaryKine* kin = new THcPrimaryKine("P.kin", "SHMS Single Arm Kinematics", "P", "P.rb");
  gHaPhysics->Add(kin);
  // Calculate the hodoscope efficiencies
  THcHodoEff* peff = new THcHodoEff("phodeff", "SHMS hodo efficiency", "P.hod");
  gHaPhysics->Add(peff);

  // Add event handler for prestart event 125.
  THcConfigEvtHandler* ev125 = new THcConfigEvtHandler("HC", "Config Event type 125");
  gHaEvtHandlers->Add(ev125);
  // Add event handler for EPICS events
  THaEpicsEvtHandler* hcepics = new THaEpicsEvtHandler("epics", "HC EPICS event type 180");
  gHaEvtHandlers->Add(hcepics);
  // Add event handler for scaler events
  THcScalerEvtHandler* pscaler = new THcScalerEvtHandler("P", "Hall C scaler event type 1");
  pscaler->AddEvtType(1);
  pscaler->AddEvtType(129);
  pscaler->SetDelayedType(129);
  pscaler->SetUseFirstEvent(kTRUE);
  gHaEvtHandlers->Add(pscaler);
  // Add event handler for DAQ configuration event
  THcConfigEvtHandler *pconfig = new THcConfigEvtHandler("pconfig", "Hall C configuration event handler");
  gHaEvtHandlers->Add(pconfig);

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

  analyzer->AddPostProcess(pp1);
  analyzer->AddPostProcess(pp2);
  analyzer->SetEvent(event);
  // Set EPICS event type
  analyzer->SetEpicsEvtType(180);
  // Define crate map
  analyzer->SetCrateMapFileName((hallc_replay_dir+"MAPS/db_cratemap.dat").c_str());
  // Define output ROOT file
  analyzer->SetOutFile(ROOTFileName.Data());
  // Define DEF-file
  analyzer->SetOdefFile((hallc_replay_dir+"DEF-files/SHMS/PRODUCTION/pstackana_production_all.def").c_str());
  // Define cuts file
  analyzer->SetCutFile((hallc_replay_dir+"DEF-files/SHMS/PRODUCTION/CUTS/pstackana_production_cuts.def").c_str());  // optional


  // File to record accounting information for cuts
  analyzer->SetSummaryFile((hallc_replay_dir+Form("REPORT_OUTPUT/SHMS/PRODUCTION/summary_all_production_%d_%d.report", RunNumber, MaxEvent)).c_str());  // optional
  // Start the actual analysis.
  analyzer->Process(run);
  // Create report file from template
  //analyzer->PrintReport((hallc_replay_dir+"TEMPLATES/SHMS/PRODUCTION/pstackana_production.template").c_str(),
  //                      (hallc_replay_dir+Form("REPORT_OUTPUT/SHMS/PRODUCTION/replay_shms_all_production_%d_%d.report", RunNumber, MaxEvent)).c_str());  // optional


  tg.Wait();
}
