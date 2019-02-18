#include "scandalizer/ScriptHelpers.h"
#include <iostream>
#include <cstdlib>

namespace hallc {
  namespace helper {

    bool running_with_hcana(){
      std::string process_name = "";
      if (const char* env_p = std::getenv("_")) {
        process_name = env_p ;
      }
      std::cout << "running program : " << process_name << "\n";
      auto last_part = process_name.substr(process_name.size()-5,5);
      std::cout << last_part << "\n";
      if( last_part == "hcana" ) {
        return true;
      }
      return false;
    }

    bool script_requires_hcana(){
      if(!running_with_hcana()){
        std::cout << " You are not running this script with hcana!\n";
        std::cout << " Use hcana!!!\n";
        std::quick_exit(1);
        return false;
      }
      return true;
    }

  }
}
