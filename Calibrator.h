/* written by: Vadim Wagner <vwagner@ikp.tu-darmstadt.de> */

#ifndef CALIBRATOR_H
#define CALIBRATOR_H

#include "TString.h"

class TH1F;

class Calibrator{
public:    
    void Calibrate(const TH1F& hist);
private:
};

#endif