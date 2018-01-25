#ifndef RCNP_DETECTOR_H
#define RCNP_DETECTOR_H
#include "Detector.h"

class RCNP_Detector : public Detector{
public:
  RCNP_Detector();
  ~RCNP_Detector() override {};
  
  int GetFieldID() const override {return 0;}
  void RegisterData(TTree& tree) override;
  void Process(const std::vector<UShort_t>& data) override;
  void Clear() override;
};

#endif