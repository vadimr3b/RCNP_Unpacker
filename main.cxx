#include "Unpacker.h"
#include <iostream>
#include "TString.h"



int main(const int narg, const char* argv[]){
  if(narg < 2){
    std::cout << "No target to unpack..." << std::endl;
    return -1;
  }
  
  std::vector<TString> targets;
  
  uint32_t maxevents = -1;
  TString tarDir ="";
  
  for(int i = 1; i < narg; ++i){
    TString str = argv[i];
    if(str.EqualTo("-n"))
        maxevents = std::atoi(argv[++i]);
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
    
    std::cout << "Unpacking '" << targets[i] << "'" << std::endl;
    TString fout = fdir + fname.ReplaceAll(".bld", ".root");
    Unpacker u;
    u.SetName("test");
    
    STATUS s = u.Unpack(targets[i], fout, maxevents);
    switch(s){
      case STATUS::ERR_OPEN                             : std::cout << "Error: Could not open '" << fname << "'"                << std::endl; break;
      case STATUS::ERR_CREATE                           : std::cout << "Error: Could not create outputfile"                     << std::endl; break;
      case STATUS::ERR_UNEXPECTED_EOF                   : std::cout << "Error: Unexpected end of file"                          << std::endl; break;
      case STATUS::ERR_UNKNOWN_DATA_STRUCTURE           : std::cout << "Error: Unknown data structure"                          << std::endl; break;
      case STATUS::ERR_EVENT_NUMBER_MISMATCH            : std::cout << "Error: Eventnumber and counter do not match!"           << std::endl; break;
      case STATUS::DONE                                 : std::cout << "unpacked '" << fname << "'"                             << std::endl; break;
      default                                           : std::cout << "Error: Unknown status code: " << static_cast<int>(s)    << std::endl; break;
    }
  }
  
  return 0;
}