#define DEBUG

#include "Unpacker.h"

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
using std::hex;
using std::dec;
#include <thread>
using std::thread;
using std::ref;
#include "TTree.h"
#include "TFile.h"

Bool_t bigEndian = true;

Unpacker::Unpacker() : name("Exp")
{
    int n = 1;

    if(*(char *)&n == 1)
        bigEndian = false;
}

void ReadFSByte(gzFile& f, void* adr, UInt_t len = 1){
    for(void* ptr = adr; ptr < adr + 1*len; ptr += 1)
        gzread(f, ptr, 1);
}

void ReadFSShort(gzFile& f, void* adr, UInt_t len = 1){
    for(void* ptr = adr; ptr < adr + 2*len; ptr += 2)
        gzread(f, ptr, 2);
    
    if(!bigEndian)
        for(void* ptr = adr; ptr < adr + 2*len; ptr += 2){
            UShort_t* val = (UShort_t*)ptr;
            *val = ((*val & 0xFF) << 8 | (*val & 0xFF00) >> 8);
        }
}

void ReadFSInt(gzFile& f, void* adr, UInt_t len = 1){
    for(void* ptr = adr; ptr < adr + 4*len; ptr += 4)
        gzread(f, ptr, 4);
    if(!bigEndian)
        for(void* ptr = adr; ptr < adr + 4*len; ptr += 4){
            UInt_t* val = (UInt_t*)ptr;
            *val = ((*val & 0xFF000000) >> 24 | (*val & 0xFF0000) >> 8 | (*val & 0xFF00) << 8 | (*val & 0xFF) << 24);
        }
}

void Dump(gzFile& f, UInt_t words){
    UShort_t temp;
    for (UInt_t i = 0; i < words; ++i){
        gzread(f, &temp, 2);
        //cout << hex << temp << " ";
    }
}

struct BLD1Header{
    bool Read(gzFile& ifs){
        ReadFSInt(ifs, &HID);
        ReadFSInt(ifs, &BlockNumber);
        ReadFSInt(ifs, &BSize);
        ReadFSInt(ifs, &HSize);
        ReadFSInt(ifs, &PrevPos1);
        ReadFSInt(ifs, &PrevPos2);
        ReadFSInt(ifs, &NextPos1);
        ReadFSInt(ifs, &NextPos2);
        
        if(HID != _HID){
            cerr << "Error in " << "BLD1Header" << ": Wrong ID " << hex << HID << endl;
            return false;
        }
        if(HSize != _HSize*2){
            cerr << "Error in " << "BLD1Header" << ": Wrong size " << HSize << endl;
            return false;
        }
        if(BlockNumber != __BlockNumber++){
            cerr << "Error in " << "BLD1Header" << ": Wrong Blocknumber!" << endl;
            return false;
        }
        
        return true;
    }
    
     UShort_t __BlockNumber = 0;
    
    UInt_t HID;
    UInt_t BlockNumber;
    UInt_t BSize;
    UInt_t HSize;
    UInt_t PrevPos1;
    UInt_t PrevPos2;
    UInt_t NextPos1;
    UInt_t NextPos2;
    
    static constexpr UInt_t _HID = 0x424c4431;
    static constexpr UShort_t _HSize = 16;
};

struct BlockHeader{
    bool Read(gzFile& ifs){
        ReadFSShort(ifs, &HID);
        ReadFSShort(ifs, &HSize);
        ReadFSShort(ifs, &BID);
        ReadFSShort(ifs, &BSize);
        ReadFSShort(ifs, &BlockNumber);
        ReadFSShort(ifs, &EventNumber);
        ReadFSInt(ifs, &BSize32);
        #ifdef DEBUG
        cout << "BlockHeader   ID: " << hex << BID << "   HSize: " << HSize << "   BSize: " << BSize << "   #Events: " << EventNumber << endl;
        #endif
        
        if(HID != _HID){
            cerr << "Error in " << "BlockHeader" << ": Wrong HID " << hex << HID << endl;
            return false;
        }
        if(HSize != _HSize){
            cerr << "Error in " << "BlockHeader" << ": Wrong size " << HSize << endl;
            return false;
        }
        if(BlockNumber != __BlockNumber++){
            cerr << "Error in " << "BlockHeader" << ": Wrong Blocknumber!" << endl;
            return false;
        }
        
        return true;
    }
    
    UShort_t __BlockNumber = 0;
    
    UShort_t HID;
    UShort_t HSize;
    UShort_t BID;
    UShort_t BSize;
    UShort_t BlockNumber;
    UShort_t EventNumber;
    UInt_t BSize32;
    
    static constexpr UShort_t _RunStartBlock = 0x0F01;
    static constexpr UShort_t _RunEndBlock = 0x0F02;
    static constexpr UShort_t _RunMiddleBlock = 0x0F03;
    static constexpr UShort_t _DataBlock = 0x0000;
    static constexpr UShort_t _ScalerBlock = 0x0E00;
    static constexpr UShort_t _EventBuiltBlock = 0x0F01;
    
    static constexpr UShort_t _HID = 0xFFFF;
    static constexpr UShort_t _HSize = 8;
};

struct BlockTrailer{
    bool Read(gzFile& ifs){
        ReadFSShort(ifs, &HID);
        ReadFSShort(ifs, &HSize);
        
        if(HID != _HID){
            cerr << "Error in " << "BlockTrailer" << ": Wrong HID " << hex << HID << endl;
            return false;
        }
        if(HSize != _HSize){
            cerr << "Error in " << "BlockTrailer" << ": Wrong size " << HSize << endl;
            return false;
        }
        
        return true;
    }
    
    UShort_t HID;
    UShort_t HSize;
    
    static constexpr UShort_t _HID = 0xFFEF;
    static constexpr UShort_t _HSize = 2;
};

struct RunComment{
    bool Read(gzFile& ifs){
        ReadFSShort(ifs, &version);
        ReadFSShort(ifs, &res1);
        ReadFSInt(ifs, &byte);
        ReadFSInt(ifs, &time);
        ReadFSShort(ifs, &run);
        ReadFSShort(ifs, &res2);
        
        UShort_t temp;
        Byte_t pos = 0;
        for(Int_t i = 0; i < 124; ++i){
            ReadFSShort(ifs, &temp);
            if(temp & 0x00FF == 0x0A)
                continue;
            comment[pos++] = temp & 0x00FF;
        }
        
        return true;
    }
    
    void Print(){
        Printf("%s", comment);        
    }
    
    UShort_t version;
    UShort_t res1;
    UInt_t byte;
    UInt_t time;
    UShort_t run;
    UShort_t res2;
    UChar_t comment[124];
    
    static constexpr UShort_t _HSize = 140;
};

struct EventHeader{
    bool Read(gzFile& ifs){
        ReadFSShort(ifs, &HID);
        ReadFSShort(ifs, &HSize);
        ReadFSShort(ifs, &EID);
        ReadFSShort(ifs, &ESize);
        ReadFSShort(ifs, &EventNumber);
        ReadFSShort(ifs, &FieldNumber);
        
        if(HID != _HID){
            cerr << "Error in " << "EventHeader" << ": Wrong HID " << hex << HID << endl;
            return false;
        }
        if(HSize != _HSize){
            cerr << "Error in " << "EventHeader" << ": Wrong size " << HSize << endl;
            return false;
        }
        if(EventNumber != __EventNumber++){
            cerr << "Error in " << "EventHeader" << ": Wrong Eventnumber!" << endl;
            return false;
        }
        
        return true;
    }
    
    UShort_t __EventNumber = 0;
    
    UShort_t HID;
    UShort_t HSize;
    UShort_t EID;
    UShort_t ESize;
    UShort_t EventNumber;
    UShort_t FieldNumber;
    
    static constexpr UShort_t _HID = 0xFFDF;
    static constexpr UShort_t _HSize = 6;
};

struct FieldHedaer{
    bool Read(gzFile& ifs){
        ReadFSShort(ifs, &HID);
        ReadFSShort(ifs, &HSize);
        ReadFSShort(ifs, &FID);
        ReadFSShort(ifs, &FSize);

        if(HID != _HID){
            cerr << "Error in " << "FieldHeader" << ": Wrong HID " << hex << HID << endl;
            return false;
        }
        
        if(HSize != _HSize){
            cerr << "Error in " << "FieldHeader" << ": Wrong size " << HSize << endl;
            return false;
        }
        
        return true;
    }
    
    UShort_t HID;
    UShort_t HSize;
    UShort_t FID;
    UShort_t FSize;
    
    static constexpr UShort_t _HID = 0xFFCF;
    static constexpr UShort_t _HSize = 4;
};

STATUS Unpacker::Unpack(TString& filein, TString& fileout, uint32_t maxevents)
{
    
    gzFile ifs(gzopen(filein, "rb"));
    if(ifs == Z_NULL) return ERR_OPEN;
    TFile ofs(fileout, "RECREATE");
    if(!ofs.IsOpen()) return ERR_CREATE;
    
    TTree tree(name, name);
    UInt_t eventnr = 0;         tree.Branch("eventnr", & eventnr);
    for(auto it = detectors.begin(); it != detectors.end(); ++it){
         Detector* d = it->second;
         d->RegisterData(tree);
    }
    
    vector<vector<UShort_t>> data(detectors.size(), vector<UShort_t>(1024*1024));
    vector<thread> threads(detectors.size());
    BLD1Header BLDH;
    BlockHeader BH;
    BlockTrailer BT;
    RunComment RC;
    EventHeader EH;
    FieldHedaer FH;
    UShort_t temp;
    UInt_t detnumber;
    
    while(eventnr != maxevents && !gzeof(ifs)){
        BLDH.Read(ifs);
        if(BLDH.HID == 0x424c4431)
            cout << "Found BLD1 Format!" << endl;
        else{
            cerr << "Unknown Format: " << hex << BLDH.HID << endl;
            return ERR_UNKNOWN_FORMAT;
        }
        cout << "BLD1: HSize [Word] = " << hex << BLDH.HSize/2 << "   BSize [Word] = " << BLDH.BSize/2 << endl;
        
        if(!BH.Read(ifs)) return ERR_UNKNOWN_DATA_STRUCTURE;
        while(BH.BSize > 0){
            if(BH.BID == BH._RunStartBlock){
                if(!RC.Read(ifs)) return ERR_UNKNOWN_DATA_STRUCTURE;
                RC.Print();
                Dump(ifs, BLDH.BSize/2 - RC._HSize - BH._HSize+6);
                BH.BSize = 0;
            }
            else if(BH.BID == BH._RunEndBlock){
                if(!RC.Read(ifs)) return ERR_UNKNOWN_DATA_STRUCTURE;
                RC.Print();
                BH.BSize -= RC._HSize;
            }
            else if((BH.BID & (~7)) == BH._DataBlock){
                for(UInt_t event = 0; event < BH.EventNumber; ++event){
                    if(!EH.Read(ifs)) return ERR_UNKNOWN_DATA_STRUCTURE;
                    BH.BSize -= EH._HSize + EH.ESize;
                    detnumber = 0;
                    for(UInt_t field = 0; field < EH.FieldNumber; ++field){
                        if(!FH.Read(ifs)) return ERR_UNKNOWN_DATA_STRUCTURE;
                        EH.ESize -= FH._HSize + FH.FSize;
                        if(detectors.find(FH.FID) != detectors.end()){
                            for(UShort_t word = 0; word < FH.FSize; ++word){
                                ReadFSShort(ifs, &temp);
                                data[detnumber].push_back(temp);
                            }
                            threads.push_back(thread(&Detector::Process, detectors[FH.FID], ref(data[detnumber++])));
                        }
                        else{ //Skipp
                            #ifdef DEBUG
                            cout << "Skipping Detector: " << hex << FH.FID << endl;
                            #endif
                            Dump(ifs, FH.FSize);
                            threads.push_back(thread());
                        }
                    }
                    for(UShort_t det = 0; det < detnumber; ++det){
                        threads[det].join();
                        data[det].clear();
                    }
                    threads.clear();
                    tree.Fill();
                    for(auto it = detectors.begin(); it != detectors.end(); ++it){
                        Detector* d = it->second;
                        d->Clear();
                    }
                }
            }
            else if(BH.BID == BH._ScalerBlock){
                //TODO
            }
        }
        BT.Read(ifs);
        //return DONE;
    }
    
    ofs.Write();
    return DONE;
}

#undef DEBUG
