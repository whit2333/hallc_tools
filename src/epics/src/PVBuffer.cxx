#include "PVBuffer.h"

#include <algorithm>
#include <sstream>
#include <thread>

namespace hallc {

  class PVBuffer::Impl {
  public:
    std::mutex m;
  };
  //______________________________________________________________________________

  PVBuffer::PVBuffer(float init_val)
      : fBuffer(2 * 1000, init_val), m_impl(std::make_shared<Impl>()) {
    fArrayIndex = fBufferMaxSize;
  }
  //______________________________________________________________________________

  void PVBuffer::Add(float val) {
    std::lock_guard<std::mutex> lockGuard(m_impl->m);
    if (fArrayIndex + 1 == fBufferMaxSize * 2) {
      // Rotate about the middle
      std::rotate(fBuffer.begin(), fBuffer.begin() + fBufferMaxSize, fBuffer.end());
      fArrayIndex = fBufferMaxSize - 1;
    }
    fArrayIndex++;
    fBuffer[fArrayIndex] = val;
  }
  //______________________________________________________________________________

  int PVBuffer::GetOffset() const {
    if (fArrayIndex > fBufferMaxSize) {
      return fArrayIndex - fBufferMaxSize;
    }
    return 0;
  }
  //______________________________________________________________________________

  std::vector<float>& PVBuffer::GetBufferCopy() {
    std::lock_guard<std::mutex> lockGuard(m_impl->m);
    fBufferCopy.clear();
    std::copy(fBuffer.begin() + GetOffset(), fBuffer.begin() + GetOffset() + fBufferMaxSize,
              std::back_inserter(fBufferCopy));
    return fBufferCopy;
  }
  //______________________________________________________________________________

} // namespace hallc
