#ifndef hallc_tools_runplan_RunTable_HH
#define hallc_tools_runplan_RunTable_HH

#include "runplan/hallc_settings.h"
#include <map>
#include <vector>

/** Rate calculated on a given target
 */
struct TargetRates {
  // double total_XS    = 0.0;
  double window_XS = 0.0;
  double LH2_XS    = 0.0;
  double LD2_XS    = 0.0;
  double p_XS      = 0.0;
  double n_XS      = 0.0;
  // double total_Lumi  = 0.0;
  double window_Lumi = 0.0;
  double LH2_Lumi    = 0.0;
  double LD2_Lumi    = 0.0;

  double window_rate    = 0.0;
  double LH2_rate       = 0.0;
  double LD2_rate       = 0.0;
  double total_LD2_rate = 0.0;
  double total_LH2_rate = 0.0;
  double total_rate     = 0.0;

  void Print(std::ostream& os = std::cout) const {
    os << "Cross sections : \n";
    os << " [p]          " << p_XS << " nb \n";
    os << " [n]          " << n_XS << " nb \n";
    os << " [LD2]        " << LD2_XS << " nb \n";
    os << " [LH2]        " << LH2_XS << " nb \n";
    os << " [windows]    " << window_XS << " nb \n";
    //os << "            : " << total_cross_section/microbarn  << " ub\n";
    //os << "            : " << total_cross_section/barn  << " b\n";
    os << "Luminosities : \n";
    os << " [LH2]        " << LH2_Lumi << " 1/cm2 s\n";
    os << " [LD2]        " << LD2_Lumi << " 1/cm2 s\n";
    os << " [windows]    " << window_Lumi << " 1/cm2 s\n";
    os << "Rates  : \n";
    os << " [LD2]        " << LD2_rate << " 1/s\n";
    os << " [LH2]        " << LH2_rate << " 1/s\n";
    os << " [windows]    " << window_rate << " 1/s\n";
    os << "Total Rates  : \n";
    os << " [LD2]        " << total_LD2_rate << " 1/s\n";
    os << " [LH2]        " << total_LH2_rate << " 1/s\n";
    os << "              " << total_rate << " 1/s\n";
  }
};

/** Run Plan Table Entry
 *
 */
struct RunPlanTableEntry {
public:
  hallc::Kinematic    kinematic;
  hallc::HallCSetting hcSet;
  TargetRates         rates;

  double counts        = 0.0;
  double time          = 0.0;
  int    polarity      = 0.0;
  double Ibeam         = 0.0;
  double LD2_counts    = 0.0;
  double LH2_counts    = 0.0;
  double window_counts = 0.0;
  int    Z_target      = 1;
  int    A_target      = 2;

  int    _group           = 0;
  int    _set             = 0;
  int    _number          = 0;
  double _charge_goal     = 0.0;
  double _charge_total    = 0.0;
  double _charge_this_run = 0.0;
  int    _count_goal      = 0;
  int    _count_total     = 0;
  int    _count_this_run  = 0;

public:
  RunPlanTableEntry() {}
  RunPlanTableEntry(const RunPlanTableEntry&) = default;
  RunPlanTableEntry(RunPlanTableEntry&&)      = default;
  RunPlanTableEntry& operator=(const RunPlanTableEntry&) = default;
  RunPlanTableEntry& operator=(RunPlanTableEntry&&) = default;
  ~RunPlanTableEntry()                              = default;

  void SetTarget(int Z, int A);

  static void PrintWikiHeader(std::ostream& s = std::cout);
  static void PrintWikiFooter(std::ostream& s = std::cout);

  static void PrintWikiHeader2(std::ostream& s = std::cout);
  void        PrintWiki2(std::ostream& s = std::cout) const;
  static void PrintWikiFooter2(std::ostream& s = std::cout);

  void        PrintWiki(std::ostream& s = std::cout) const;
  void        Print(std::ostream& s = std::cout) const;
  static void PrintHeader(std::ostream& s = std::cout);
};


/** Run table.
 */
struct RunTable {
  double                          _total_time = 0.0;
  std::vector<RunPlanTableEntry>  _rows;

  RunTable() {}
  RunTable(const RunTable&) = default;
  RunTable(RunTable&&)      = default;
  RunTable& operator=(const RunTable&) = default;
  RunTable& operator=(RunTable&&) = default;
  ~RunTable()                     = default;

  void add(const RunPlanTableEntry& entry) {
    _total_time += entry.time;
    _rows.push_back(entry);
  }
  void add(RunPlanTableEntry&& entry) {
    _total_time += entry.time;
    _rows.push_back(std::move(entry));
  }
  void add(const RunTable& tab) ;

  void RecomputeTime() {
    _total_time = 0;
    for (const auto& row : _rows) {
      _total_time += row.time;
    }
  }

  const std::vector<RunPlanTableEntry>& GetVector() const { return _rows; }
  std::vector<RunPlanTableEntry>& GetVector() { return _rows; }

  /** Sets the _number for each entry in the current order.
   */
  void             Number();

  void             Print(std::ostream& os = std::cout) const;
  void             PrintWiki(std::ostream& os) const;
  void             PrintWikiByGroup(std::ostream& os) const;
  void             PrintTable(std::string table_name, std::string ofile) const;
  static RunTable* LoadTable(std::string table_name, std::string ofilename);
};


#endif

