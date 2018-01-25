#include "RCNP_Detector.h"
using std::vector;
#include <iostream>
using std::cout;
using std::endl;
using std::hex;

extern Bool_t bigEndian;

vector<UShort_t> RegionIDs;

vector<UShort_t> IPR;

vector<UShort_t> GR_ADC_OLD;
vector<UInt_t> GR_V830;

vector<UShort_t> FERA_Type;
vector<UShort_t> FERA_Ch;
vector<UShort_t> FERA_Mid;
UShort_t GR_FERA_ADC[64];
UShort_t LAS_FERA_ADC[64];
UShort_t GR_FERA_TDC[64];
UShort_t LAS_FERA_TDC[64];

//static char *fera_name[4] = { "GR_FERA_ADC", "LAS_FERA_ADC", "GR_FERA_TDC", "LAS_FERA_TDC"}

vector<UInt_t> Scaler;
UInt_t Time;

UShort_t ChkSum;

struct Region{
  void Read(const vector< UShort_t >& data, UInt_t& pos){
    RID = (data[pos] & 0xF000) >> 12;
    RSize = data[pos] & 0x0FFF;
    
    pos++;
    __RegionNumber++;
  }
  
  UShort_t __RegionNumber = 0;
  
  UShort_t RID;
  UShort_t RSize;
  
  static constexpr UShort_t ID_Reserved   = 0x0;
  static constexpr UShort_t ID_V1190      = 0x1;
  static constexpr UShort_t ID_NimIn      = 0x2;
  static constexpr UShort_t ID_ADC        = 0x3;
  static constexpr UShort_t ID_MYRIAD     = 0x4;
  static constexpr UShort_t ID_EXT        = 0x5;
  static constexpr UShort_t ID_Scaler     = 0x6;
  static constexpr UShort_t ID_3377       = 0x7;
  static constexpr UShort_t ID_UNIX_TIME  = 0x8;
  static constexpr UShort_t ID_V830       = 0x9;
  static constexpr UShort_t ID_ADC_LAS    = 0xb;
  static constexpr UShort_t ID_TDC_LAS    = 0xc;
  static constexpr UShort_t ID_FERA_ADC   = 0xd;
  static constexpr UShort_t ID_FERA_TDC   = 0xe;
  static constexpr UShort_t ID_CHKSUM     = 0xf;
};

struct FeraHeader{
  void Read(UShort_t data){
    if(bigEndian){
      Header = data & 0x1;
      Count = data & 0x1E;
      Reserved = data & 0xE0;
      VStationNum = data & 0xFF00;
    }
    else{
      Header = (data & 0x8000) >> 15;
      Count = (data & 0x7000) >> 12;
      Reserved = data & 0xF00 >> 8;
      VStationNum = data & 0xFF;
    }
    
    Type = (VStationNum & 0xF0) >> 4;
    Mid = (VStationNum & 0xF);
  }
  
  Bool_t IsHeader(UShort_t data){
    if(bigEndian)
      return data & 0x1;
    else
      return (data & 0x8000) >> 15;
  }
  
  UShort_t Header;        /* Identifies header word  (=1) */
  UShort_t Count;         /* Word Count */
  UShort_t Reserved;      /* Not Used */
  UShort_t VStationNum;   /* Virtual Station Number */
  UShort_t Type;
  UShort_t Mid;
};

struct FeraData{
  void Read(UShort_t data){
    if(bigEndian){
      Header = data & 0x1;
      Channel = data & 0x1E;
      Data = data & 0xFFE0;
    }
    else{
      Header = (data & 0x8000) >> 15;
      Channel = (data & 0x7000) >> 12;
      Data = data & 0x7FF;
    }
    

  }
  
  Bool_t IsData(UShort_t data){
    if(bigEndian)
      return !(data & 0x1);
    else
      return !((data & 0x8000) >> 15);
  }
  
  UShort_t Header;        /* Identifies header word (=0) */ 
  UShort_t Channel;       /* Channel number */
  UShort_t Data;          /* Data */
};

RCNP_Detector::RCNP_Detector()
{
}

void RCNP_Detector::RegisterData(TTree& tree)
{
  tree.Branch("RegionIDs", &RegionIDs);
  tree.Branch("IPR", &IPR);
  tree.Branch("GR_ADC_OLD", &GR_ADC_OLD);
  tree.Branch("GR_V830", &GR_V830);
  tree.Branch("Scaler", &Scaler);
  tree.Branch("Time", &Time);
  tree.Branch("ChkSum", &ChkSum);
  tree.Branch("FERA_Type", &FERA_Type);
  tree.Branch("FERA_Ch", &FERA_Ch);
  tree.Branch("FERA_Mid", &FERA_Mid);
  tree.Branch("GR_FERA_ADC", &GR_FERA_ADC, "GR_FERA_ADC[64]/S");
  tree.Branch("LAS_FERA_ADC", &LAS_FERA_ADC, "LAS_FERA_ADC[64]/S");
  tree.Branch("GR_FERA_TDC", &GR_FERA_TDC, "GR_FERA_TDC[64]/S");
  tree.Branch("LAS_FERA_TDC", &LAS_FERA_TDC, "LAS_FERA_TDC[64]/S");
}

void RCNP_Detector::Process(const vector< UShort_t >& data)
{
  Region R;
  UInt_t pos = 0;
  
  while(pos < data.size()){
    R.Read(data, pos);
    RegionIDs.push_back(R.RID);

    if(R.RID == R.ID_Reserved){
    }
    else if(R.RID == R.ID_V1190){
      //TODO
    }
    else if(R.RID == R.ID_NimIn){
      for(Int_t i = pos; i < pos+R.RSize; ++i)
        IPR.push_back(data[i]);
    }
    else if(R.RID == R.ID_ADC){
      for(Int_t i = pos; i < pos+R.RSize; ++i)
        GR_ADC_OLD .push_back(data[i]);
    }
    else if(R.RID == R.ID_Scaler){
      for(Int_t i = pos; i < pos+R.RSize; i+=2){
        if(bigEndian)
          Scaler.push_back(data[i] | (data[i+1] << 16));
        else
          Scaler.push_back(data[i+1] | (data[i] << 16));
      }
    }
    else if(R.RID == R.ID_UNIX_TIME){
      Time = (data[pos] | (data[pos+1] << 16));
    }
    else if(R.RID == R.ID_V830){
      for(Int_t i = pos; i < pos+R.RSize; i+=2)
        GR_V830.push_back(data[i] | (data[i+1] << 16));
      
      //TODO
    }
    else if(R.RID == R.ID_FERA_ADC || R.RID == R.ID_FERA_TDC){
      FeraHeader FH;
      FeraData FD;
      for(Int_t i = pos; i < pos+R.RSize; ++i){
        if(FH.IsHeader(data[i])){
          FH.Read(data[i]);
          FERA_Type.push_back(FH.Type);
          FERA_Mid.push_back(FH.Mid);
        }
        else{
          FD.Read(data[i]);
          FERA_Ch.push_back(FD.Channel);
          if(FH.Type == 0)
            GR_FERA_ADC[FD.Channel+16*FH.Mid] = FD.Data;
          else if(FH.Type == 1)
            LAS_FERA_ADC[FD.Channel+16*FH.Mid] = FD.Data;
          else if(FH.Type == 8)
            GR_FERA_TDC[FD.Channel+16*FH.Mid] = FD.Data;
          else if(FH.Type == 9)
            LAS_FERA_ADC[FD.Channel+16*FH.Mid] = FD.Data;
        }
      }
    }
    else if(R.RID == R.ID_CHKSUM){
      ChkSum = data[pos];
    }
    else{
      cout << "Unhandeld Region found: " << hex << R.RID << endl;
    }
    pos += R.RSize;
  }
}

void RCNP_Detector::Clear()
{
  RegionIDs.clear();
  IPR.clear();
  GR_ADC_OLD.clear();
  Scaler.clear();
  GR_V830 .clear();
  FERA_Type.clear();
  FERA_Ch.clear();
  FERA_Mid.clear();
  Time = 0;
  ChkSum = 0;
  memset(GR_FERA_ADC, 0, sizeof(GR_FERA_ADC));
  memset(LAS_FERA_ADC, 0, sizeof(LAS_FERA_ADC));
  memset(GR_FERA_TDC, 0, sizeof(GR_FERA_TDC));
  memset(LAS_FERA_TDC, 0, sizeof(LAS_FERA_TDC));
}
