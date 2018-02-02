//#define DEBUG

#include "Unpacker.h"

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
using std::flush;
using std::hex;
using std::dec;
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
    BLD1Header():__BlockNumber(0){};
    
    bool Read(gzFile& ifs){
        ReadFSInt(ifs, &HID);
        
        if(HID == _HID1)
          bigEndian = false;
        else if(HID == _HID2)
          bigEndian = true;
        else{
          cerr << "Error in " << "BLD1Header" << ": Wrong ID " << hex << HID << endl;
          return false;
        }
        
        ReadFSInt(ifs, &BlockNumber);
        ReadFSInt(ifs, &BSize);
        ReadFSInt(ifs, &HSize);
        ReadFSInt(ifs, &PrevPos1);
        ReadFSInt(ifs, &PrevPos2);
        ReadFSInt(ifs, &NextPos1);
        ReadFSInt(ifs, &NextPos2);
        
        if(gzeof(ifs))
          return false;
        
        if(HSize != _HSize*2){
            cerr << "Error in " << "BLD1Header" << ": Wrong size " << HSize << endl;
            return false;
        }
        if(BlockNumber != __BlockNumber++){
            cerr << "Error in " << "BLD1Header" << ": Wrong Blocknumber (" << BlockNumber << " != " << __BlockNumber -1 << ")!" << endl;
            return false;
        }
        
        return true;
    }

    bool IsBLD1Format(){
      if(HID == _HID1 || HID == _HID2)
        return true;
      return false;
    }
    
    UShort_t __BlockNumber;
    
    UInt_t HID;
    UInt_t BlockNumber;
    UInt_t BSize;
    UInt_t HSize;
    UInt_t PrevPos1;
    UInt_t PrevPos2;
    UInt_t NextPos1;
    UInt_t NextPos2;
    
    static constexpr UInt_t _HID1 = 0x424c4431;
    static constexpr UInt_t _HID2 = 0x31444c42;
    static constexpr UShort_t _HSize = 16;
};

struct BlockHeader{
    BlockHeader():__BlockNumber(0){};
    
    bool Read(gzFile& ifs){
        ReadFSShort(ifs, &HID);
        ReadFSShort(ifs, &HSize);
        ReadFSShort(ifs, &BID);
        ReadFSShort(ifs, &BSize);
        ReadFSShort(ifs, &BlockNumber);
        ReadFSShort(ifs, &EventNumber);
        UShort_t l, u;
        ReadFSShort(ifs, &l);
        ReadFSShort(ifs, &u);
        BSize32 = UInt_t(u) << 16 | l;
        #ifdef DEBUG
        cout << "BlockHeader   ID: " << hex << BID << "   HSize: " << hex << HSize << "   BSize: " << hex << BSize32 << "   #Events: " << dec << EventNumber << endl;
        #endif
        
        if(HID != _HID){
            cerr << "Error in " << "BlockHeader" << ": Wrong HID " << dec << HID << endl;
            return false;
        }
        if(HSize != _HSize){
            cerr << "Error in " << "BlockHeader" << ": Wrong size " << hex << HSize << endl;
            return false;
        }
        if(BlockNumber != __BlockNumber++){
            cerr << "Error in " << "BlockHeader" << ": Wrong Blocknumber (" << BlockNumber << " != " << __BlockNumber -1 << ")!" << endl;
            //return false;
        }
        
        return true;
    }
    
    UShort_t __BlockNumber;
    
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
    BlockTrailer(){};
    
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
    RunComment(){};
    
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
            if((temp & 0x00FF) == 0x0A)
                continue;
            comment[pos++] = temp & 0x00FF;
        }
        
        Comment = TString(comment);
        
        return true;
    }
    
    void Print(){
        cout << "'" << Comment << "'" << endl;
    }
    
    UShort_t version;
    UShort_t res1;
    UInt_t byte;
    UInt_t time;
    UShort_t run;
    UShort_t res2;
    Char_t comment[124];
    
    TString Comment;
    
    static constexpr UShort_t _HSize = 140;
};

struct EventHeader{
    EventHeader():__EventNumber(0){};
    
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
            cerr << "Error in " << "EventHeader" << ": Wrong Eventnumber! (" << EventNumber << " != " << __EventNumber-1 <<  ")" << endl;
            //return false;
        }
        
        return true;
    }
    
    UShort_t __EventNumber;
    
    UShort_t HID;
    UShort_t HSize;
    UShort_t EID;
    UShort_t ESize;
    UShort_t EventNumber;
    UShort_t FieldNumber;
    
    static constexpr UShort_t _HID = 0xFFDF;
    static constexpr UShort_t _HSize = 6;
};

struct FieldHeader{
    FieldHeader(){};
    
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
    TFile fout(fileout, "RECREATE");
    if(!fout.IsOpen()) return ERR_CREATE;
    
    TTree tree(name, name);
    for(auto it = detectors.begin(); it != detectors.end(); ++it){
         Detector* d = it->second;
         d->RegisterData(tree);
    }
    
    vector<UShort_t> data;
    BLD1Header BLDH;
    RunComment RC;
    BlockHeader BH;
    BlockTrailer BT;
    UShort_t temp;
    
    UInt_t eventcounter = 0;
    
    while(!gzeof(ifs)){
        BLDH.Read(ifs);
        if(BLDH.IsBLD1Format()){
            #ifdef DEBUG
            cout << "Found BLD1 Format!" << endl;
            #endif
        }
        else{
            cerr << "Unknown Format: " << hex << BLDH.HID << endl;
            return ERR_UNKNOWN_FORMAT;
        }
        #ifdef DEBUG
        cout << "BLD1: HSize = " << hex << BLDH.HSize/2 << "   BSize = " << hex << BLDH.BSize/2 << endl;
        #endif
        if(gzeof(ifs)){
          fout.Write();
          return DONE;
        }
        if(!BH.Read(ifs)) return ERR_UNKNOWN_DATA_STRUCTURE;
        
        EventHeader EH;
        FieldHeader FH;
        
        while(BH.BSize32 > BT._HSize){
            #ifdef DEBUG
            cout << " Remaining Block Size " << hex << BH.BSize32 << hex << endl;
            #endif 
            if(BH.BID == BH._RunStartBlock){
              
                if(!RC.Read(ifs)) return ERR_UNKNOWN_DATA_STRUCTURE;
                RC.Print();
                Dump(ifs, BLDH.BSize/2 - RC._HSize - BH._HSize+6); //TODO wieso 6???
                BH.BSize32 = 0;
            }
            else if(BH.BID == BH._RunEndBlock){
                if(!RC.Read(ifs)) return ERR_UNKNOWN_DATA_STRUCTURE;
                RC.Print();
                BH.BSize32 -= RC._HSize;
            }
            else if((BH.BID & (~7)) == BH._DataBlock){
                #ifdef DEBUG
                cout << "Reading Block #" << dec << BH.BlockNumber << endl;
                #endif 
                for(UInt_t event = 0; event < BH.EventNumber; ++event){
                    if(gzeof(ifs)){
                      fout.Write();
                      return DONE;
                    }
                    if(!EH.Read(ifs)) return ERR_UNKNOWN_DATA_STRUCTURE;
                    BH.BSize32 -= EH._HSize + EH.ESize;
                    cout << "\r" << eventcounter << " Events processed!" << flush;
                    if(maxevents < eventcounter++){
                      fout.Write();
                      return DONE;
                    }
                    #ifdef DEBUG
                    cout << "  Reading Event #" << dec << EH.EventNumber  << " (" << hex << EH._HSize + EH.ESize << ")   (Bsize left: " << hex << BH.BSize32 << ")" << endl;
                    #endif
                    for(UInt_t field = 0; field < EH.FieldNumber; ++field){                     
                        if(!FH.Read(ifs)) return ERR_UNKNOWN_DATA_STRUCTURE;
                        EH.ESize -= FH._HSize + FH.FSize;
                        #ifdef DEBUG
                        cout << "    Reading Field #" << dec << field << endl;
                        #endif 
                        if(detectors.find(FH.FID) != detectors.end()){
                            for(UShort_t word = 0; word < FH.FSize; ++word){
                                ReadFSShort(ifs, &temp);
                                data.push_back(temp);
                            }
                            detectors[FH.FID]->Process(data);
                            data.clear();
                        }
                        else{ //Skipp
                            #ifdef DEBUG
                            cout << "    Skipping Detector with ID: " << hex << FH.FID << endl;
                            #endif
                            Dump(ifs, FH.FSize);
                        }
                        #ifdef DEBUG
                        cout << "   Remaining Words: " << hex << EH.ESize << endl;
                        #endif
                    }
                    tree.Fill();
                    for(auto it = detectors.begin(); it != detectors.end(); ++it){
                        Detector* d = it->second;
                        d->Clear();
                    }
                }
            }
            else if(BH.BID == BH._ScalerBlock){
                //TODO
              cerr <<"_ScalerBlock"<<endl;
            }
        }
        BT.Read(ifs);
        //return DONE;
    }
    
    fout.Write();
    return DONE;
}

#undef DEBUG
