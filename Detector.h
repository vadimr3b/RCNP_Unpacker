#ifndef DETECTOR_H
#define DETECTOR_H

#include "TString.h"
#include "TTree.h"
#include <vector>

class Detector{
public:
  virtual int GetFieldID() const = 0;
  virtual void RegisterData(TTree& tree) = 0;
  virtual void Process(const std::vector<UShort_t>& data) = 0;
  virtual void Clear() = 0;
  
  virtual ~Detector(){};
};

#endif