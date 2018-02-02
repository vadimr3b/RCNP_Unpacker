#include <iostream>

#include "Calibrator.h"

#include "TString.h"
#include "TChain.h"
#include "TH1F.h"
#include "TDirectory.h"

using std::cout;
using std::cerr;
using std::endl;

int main(const int narg, const char* argv[]){
  if(narg < 2){
    cout << "No targets. Will not produce a calib-file!" << endl;
    return -1;
  }
  
  TChain chain("Exp","Exp");
  
  TString tarDir ="./";
  
  for(int i = 1; i < narg; ++i){
    TString str = argv[i];
    if(str.EqualTo("-d"))
      tarDir = argv[++i];
    else
      chain.AddFile(argv[i]);
  }
  chain.Draw("GR_TDC_X1>>htemp(1000,0,1000)");
  
  TH1F hist = *((TH1F*) gDirectory->Get("htemp"));
  
  Calibrator calibrator;
  
  calibrator.Calibrate(hist);
  
  return 0;
}