#ifdef __cpp_lib_filesystem
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

#include "nlohmann/json.hpp"

#include "TObject.h"


void json_load(){

  using nlohmann::json;
  json j;
  {
    std::ifstream json_input_file("run_list_1.json");
    json_input_file >> j;
  }

  std::cout << j.dump(2);

  std::cout << " runs : ";
  std::vector<int> runs;
  for (json::iterator it = j.begin(); it != j.end(); ++it) {
    std::cout << it.key() << "\n";
    runs.push_back(std::stoi(it.key()));
  }

}
