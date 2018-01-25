#include "Unpacker.h"
#include <iostream>
#include "TString.h"
#include "RCNP_Detector.h"

using std::cout;
using std::cerr;
using std::endl;

int main(const int narg, const char* argv[]){
  if(narg < 2){
    cout << "No target to unpack..." << endl;
    return -1;
  }
  
  std::vector<TString> targets;
  
  uint32_t maxblocks = -1;
  TString tarDir ="";
  
  for(int i = 1; i < narg; ++i){
    TString str = argv[i];
    if(str.EqualTo("-n"))
        maxblocks = std::atoi(argv[++i]);
    else if(str.EqualTo("-d"))
        tarDir = argv[++i];
    else
        targets.push_back(str);
  }
  
  for(uint i = 0; i < targets.size(); ++i){
    TString fdir;
    TString fname(targets[i]);
    
    if(fname.Contains('/')){
        fdir = fname(0, fname.Last('/')+1);
        fname = fname(fname.Last('/')+1, fname.Length());
    }
    else
        fdir = "./";
    
    if(tarDir.Length() > 0)
        fdir = tarDir;
    
    cout << "Unpacking '" << targets[i] << "'" << endl;
    TString fout = fdir + fname.ReplaceAll(".bld", ".root");
    Unpacker u;
    
    RCNP_Detector rcnp;
    
    u.AddDetector(&rcnp);
    
    STATUS s = u.Unpack(targets[i], fout, maxblocks);
    cout << endl;
    switch(s){
      case STATUS::ERR_OPEN                             : cout << "Error: Could not open '" << fname << "'"                << endl; break;
      case STATUS::ERR_CREATE                           : cout << "Error: Could not create outputfile"                     << endl; break;
      case STATUS::ERR_UNEXPECTED_EOF                   : cout << "Error: Unexpected end of file"                          << endl; break;
      case STATUS::ERR_UNKNOWN_DATA_STRUCTURE           : cout << "Error: Unknown data structure"                          << endl; break;
      case STATUS::ERR_EVENT_NUMBER_MISMATCH            : cout << "Error: Eventnumber and counter do not match!"           << endl; break;
      case STATUS::DONE                                 : cout << "unpacked '" << fname << "'"                             << endl; break;
      default                                           : cout << "Error: Unknown status code: " << static_cast<int>(s)    << endl; break;
    }
  }
  
  return 0;
}