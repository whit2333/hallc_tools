#include "monitor/DisplayPlots.h"

#include "cppitertools/zip.hpp"

namespace hallc {
  namespace display {
    void PlotData::Merge(PlotData* d) {
      for (auto&& [h1, h2] : iter::zip(_hists1, d->_hists1)) {
        (*h1) = (*h2);
      }
      for (auto&& [h1, h2] : iter::zip(_hists2, d->_hists2)) {
        (*h1) = (*h2);
      }
    }

    /** Replace this data with d data.
     *
     */
    void PlotData::Replace(PlotData* d) {
      for (auto&& [h1, h2] : iter::zip(_hists1, d->_hists1)) {
        (*h1) = (*h2);
      }
      for (auto&& [h1, h2] : iter::zip(_hists2, d->_hists2)) {
        (*h1) = (*h2);
      }
    }

  } // namespace display
} // namespace hallc
