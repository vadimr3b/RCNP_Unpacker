/* written by: Vadim Wagner <vwagner@ikp.tu-darmstadt.de> */

#ifndef UNPACKER_H
#define UNPACKER_H

#include "TString.h"
#include <vector>
using std::vector;
#include <map>
using std::map;
#include <zlib.h>

#include "Detector.h"

enum STATUS : int {DONE, ERR_OPEN, ERR_CREATE, ERR_UNKNOWN_FORMAT, ERR_UNEXPECTED_EOF, ERR_UNKNOWN_DATA_STRUCTURE,
                   ERR_UNIMPLEMENTED, ERR_EVENT_NUMBER_MISMATCH};
                   
class Unpacker{
public:
    
    Unpacker();
    
    void AddDetector(Detector* detector){
        detectors[detector->GetFieldID()] = detector;
    }
    
    void SetName(TString& name){
         this->name = name;
    }
    
    void SetName(const char* name){
         this->name = name;
    }
    
    STATUS Unpack(TString& filein, TString& fileout, uint32_t maxevents = -1);

private:
    TString name;
    map<UShort_t, Detector*> detectors;
};

#endif