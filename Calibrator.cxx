#include "Calibrator.h"
#include "TH1F.h"
#include "TFile.h"

#include <iostream>

void Calibrator::Calibrate(const TH1F& hist)
{
    TFile file("test.root", "RECREATE");
    
    ULong64_t entries = 0;
    for(UInt_t i = 0; i < hist.GetNbinsX(); ++i){
      entries += hist.GetBinContent(i);
    }
    
    TH1F integral("Integral", "Integral", hist.GetNbinsX(), 0, hist.GetNbinsX());
    
    Double_t tot = 0;
    for(UInt_t i = 0; i < hist.GetNbinsX(); ++i){
      tot += hist.GetBinContent(i);
      integral.SetBinContent(i, tot/entries);
    }
    hist.Write();
    integral.Write();
}

