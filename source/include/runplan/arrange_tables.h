#ifndef arrange_tables_hh
#define arrange_tables_hh

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <thread>
#include <future>
#include <iostream>
#include <string>
#include <chrono>
#include <typeinfo>

#include "TH1F.h"
#include "TFile.h"
#include "TTree.h"
#include "TBufferJSON.h"
#include "TCanvas.h"
#include "TMultiGraph.h"
#include "THStack.h"

#include "Math/Vector3Dfwd.h"
#include "Math/Vector4Dfwd.h"
#include "Math/Transform3D.h"
#include "Math/Rotation3D.h"
#include "Math/RotationY.h"
#include "Math/RotationZ.h"

#include "ROOT/TFuture.hxx"
#include "ROOT/RDataFrame.hxx"

// range-v3 library (header only)
#include "range/v3/all.hpp"
#include "range/v3/numeric/accumulate.hpp"

// nlohmann json library (header only)
#include "nlohmann/json.hpp"

// fmt - string formatting library
#include "fmt/core.h"
#include "fmt/ostream.h"
R__LOAD_LIBRARY(libfmt.so)

// date library (while waiting on c++20) https://github.com/HowardHinnant/date
#include "date/date.h"

#include "hallc_settings.h"

using json        = nlohmann::json;
using Q2Table     = std::vector<RunPlanTableEntry>;
using TableVector = std::vector<std::pair<double, Q2Table>>;
using day_point   = std::chrono::time_point<std::chrono::system_clock, date::days>;


    // ---------------------------------------------------------
    // template debugging
    // to print a warning do something like:
    // debug_type<second_arg_type<decltype(f2)>>()
    //
    template <typename T>                
    inline void debug_type(const T&) __attribute__((deprecated));        

    template <typename T>                                          
    inline void debug_type(const T&) { }                           

    template <typename T>                                        
    inline void debug_type() __attribute__((deprecated));        

    template <typename T>                                            
    inline void debug_type() { 
        std::cout << __PRETTY_FUNCTION__ << "\n";
    } 

    template <typename T>                                            
    struct debug_class {
      T t;
      debug_class(){
        std::cout << __PRETTY_FUNCTION__ << "\n";
      }
    };

    //__________________________________________________________

void arrange_tables();
void arrange_tables2();



#endif

