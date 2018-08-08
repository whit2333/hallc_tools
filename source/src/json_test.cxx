#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <thread>
#include <future>
#include <iostream>
#include <string>
#include <chrono>

#include <fmt/core.h>
#include <fmt/ostream.h>

#include "Math/Vector3Dfwd.h"
#include "Math/Vector4Dfwd.h"
#include "Math/Transform3D.h"
#include "Math/Rotation3D.h"
#include "Math/RotationY.h"
#include "Math/RotationZ.h"

#include "InSANE/MaterialProperties.h"
#include "InSANE/Luminosity.h"
#include "PhaseSpaceVariables.h"
#include "PSSampler.h"
#include "NFoldDifferential.h"
#include "Jacobians.h"
#include "DiffCrossSection.h"
#include "FinalState.h"
#include <typeinfo>


#include "InSANE/Helpers.h"
#include "InSANE/Physics.h"
#include "InSANE/Kinematics.h"

#include "InSANE/Stat2015_UPDFs.h"
#include "DSSFragmentationFunctions.h"

#include "TH1F.h"
#include "TFile.h"
#include "TTree.h"
#include "TBufferJSON.h"

#include "ROOT/RDataFrame.hxx"

#include "csv_settings.h"

using namespace ROOT;
using namespace insane::physics;
using namespace insane::kinematics;
using namespace insane::units;
using namespace insane::helpers;
using namespace ROOT::Math;

using PDFs           = insane::physics::Stat2015_UPDFs;
using FragFs         = insane::physics::DSSFragmentationFunctions;
using SIDIS_vars     = insane::kinematics::variables::SIDIS_x_y_z_phih_phie;
using SPEC_vars      = insane::kinematics::variables::SIDIS_eprime_ph;
using Q2_variable    = insane::kinematics::variables::MomentumTransfer;
using Theta_variable = insane::kinematics::variables::Theta;

using FullTable = std::vector<std::pair<double,std::vector<RunPlanTableEntry>>>;

void json_test() {
  std::ifstream t("run_plan_table.json");
  std::stringstream buffer;
  buffer << t.rdbuf();

  FullTable*    all_tables = nullptr;
  TBufferJSON::FromJSON(all_tables, buffer.str().c_str());
  std::cout << all_tables->size() << "\n";
  //std::cout << json << std::endl;
}

