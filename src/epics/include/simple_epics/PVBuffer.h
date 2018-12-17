#ifndef epics_PVBuffer_H
#define epics_PVBuffer_H

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <map>

namespace hallc {

  /** PV Buffer.
   *
   *  Actuall the buffer is twice the size of fBufferMaxSize so that when 
   *  it gets full the buffer is rotated about the middle. This avoids 
   *  constantly moving memory around. Only half the buffer (index to fBufferMaxSize)
   *  should be considered valid at any moment.
   *
   */
  class PVBuffer {
  public:
    int                fBufferMaxSize = 1000;
    int                fArrayIndex    = 0;
    std::vector<float> fBuffer;
    std::vector<float> fBufferCopy;

    PVBuffer(float init_val = 0.0);

    int                 GetIndex() const { return fArrayIndex; }
    int                 GetMaxSize() const { return fBufferMaxSize; }
    void                Add(float val);
    int                 GetOffset() const;
    std::vector<float>& GetBufferCopy();

  private:
    class Impl;
    std::shared_ptr<Impl> m_impl;
  };

}

#endif
