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

_FERA_RAW FERA_RAW;
_EVENT EVENT;
_GR GR;
_GF GF;
_LAS LAS;
UShort_t BLP_ADC[8];

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
  tree.Branch("GR_V830", &GR_V830);
  tree.Branch("IPR", &IPR);
  tree.Branch("Scaler", &Scaler);
  tree.Branch("Time", &Time);
  tree.Branch("ChkSum", &ChkSum);
  tree.Branch("FERA_Type", &FERA_Type);
  tree.Branch("FERA_Ch", &FERA_Ch);
  tree.Branch("FERA_Mid", &FERA_Mid);
  tree.Branch("EVENT", &EVENT, "EVENT_GR/O:EVENT_LAS:EVENT_COIN");
  tree.Branch("GR", &GR, "GR_ADC[6]/s:GR_MADC[3]:GR_TDC[6]:GR_RF[3]:GR_TLAS:GR_DIFF[3]");
  tree.Branch("GF", &GF, "GF_ADC_XU[8]/s:GF_ADC_XD[8]:GF_ADC_YL[5]:GF_ADC_YR[5]:GF_MADC_X[8]:GF_MADC_Y[5]:GF_TDC_XU[8]:GF_TDC_XD[8]:GF_TDC_YL[5]:GF_TDC_YR[5]:GF_TDIFF_X[8]:GF_TDIFF_Y[5]");
  tree.Branch("LAS", &LAS, "LAS_ADC[12]/s:LAS_MADC[6]:LAS_TDC[12]:LAS_RF[3]:LAS_TDIFF[6]");
  //tree.Branch("BLP", &BLP_ADC, "BLP_ADC[8]/s");
  
  Clear();
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
        GR_ADC_OLD.push_back(data[i]);
        //GR_ADC_OLD[i-pos] = (data[i]);
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
            FERA_RAW.GR_FERA_ADC[FD.Channel+16*FH.Mid] = FD.Data;
          else if(FH.Type == 1)
            FERA_RAW.LAS_FERA_ADC[FD.Channel+16*FH.Mid] = FD.Data;
          else if(FH.Type == 8)
            FERA_RAW.GR_FERA_TDC[FD.Channel+16*FH.Mid] = FD.Data;
          else if(FH.Type == 9)
            FERA_RAW.LAS_FERA_ADC[FD.Channel+16*FH.Mid] = FD.Data;
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
  
  EVENT.EVENT_COIN = IPR[0] & (1 << 8);
  EVENT.EVENT_GR = (IPR[0] & (1 << 5)) || EVENT.EVENT_COIN;
  EVENT.EVENT_LAS = (IPR[0] & (1 << 7)) || EVENT.EVENT_COIN;
  
  
  GR.GR_ADC[0] = FERA_RAW.GR_FERA_ADC[0x10];
  GR.GR_ADC[1] = FERA_RAW.GR_FERA_ADC[0x11];
  GR.GR_ADC[2] = FERA_RAW.GR_FERA_ADC[0x12];
  GR.GR_ADC[3] = FERA_RAW.GR_FERA_ADC[0x13];
  GR.GR_ADC[4] = FERA_RAW.GR_FERA_ADC[0x20];
  GR.GR_ADC[5] = FERA_RAW.GR_FERA_ADC[0x28];
  GR.GR_MADC[0] = sqrt(GR.GR_ADC[1]*GR.GR_ADC[0]);
  GR.GR_MADC[1] = sqrt(GR.GR_ADC[3]*GR.GR_ADC[2]);
  GR.GR_MADC[2] = sqrt(GR.GR_ADC[5]*GR.GR_ADC[4]);
  GR.GR_TDC[0] = FERA_RAW.GR_FERA_TDC[0x10];
  GR.GR_TDC[1] = FERA_RAW.GR_FERA_TDC[0x11];
  GR.GR_TDC[2] = FERA_RAW.GR_FERA_TDC[0x12];
  GR.GR_TDC[3] = FERA_RAW.GR_FERA_TDC[0x13];
  GR.GR_TDC[4] = FERA_RAW.GR_FERA_TDC[0x20];
  GR.GR_TDC[5] = FERA_RAW.GR_FERA_TDC[0x28];
  GR.GR_RF[0] = FERA_RAW.GR_FERA_TDC[0x18];
  GR.GR_RF[1] = FERA_RAW.GR_FERA_TDC[0x19];
  GR.GR_RF[2] = FERA_RAW.GR_FERA_TDC[0x1a];
  GR.GR_TLAS = FERA_RAW.GR_FERA_TDC[0x15];
  GR.R_TDIFF[0] = GR.GR_TDC[1]-GR.GR_TDC[0];
  GR.R_TDIFF[1] = GR.GR_TDC[3]-GR.GR_TDC[2];
  GR.R_TDIFF[2] = GR.GR_TDC[5]-GR.GR_TDC[4];

  LAS.LAS_ADC[0] = FERA_RAW.LAS_FERA_ADC[0x10];
  LAS.LAS_ADC[1] = FERA_RAW.LAS_FERA_ADC[0x11];
  LAS.LAS_ADC[2] = FERA_RAW.LAS_FERA_ADC[0x12];
  LAS.LAS_ADC[3] = FERA_RAW.LAS_FERA_ADC[0x13];
  LAS.LAS_ADC[4] = FERA_RAW.LAS_FERA_ADC[0x14];
  LAS.LAS_ADC[5] = FERA_RAW.LAS_FERA_ADC[0x15];
  LAS.LAS_ADC[6] = FERA_RAW.LAS_FERA_ADC[0x16];
  LAS.LAS_ADC[7] = FERA_RAW.LAS_FERA_ADC[0x17];
  LAS.LAS_ADC[8] = FERA_RAW.LAS_FERA_ADC[0x18];
  LAS.LAS_ADC[9] = FERA_RAW.LAS_FERA_ADC[0x19];
  LAS.LAS_ADC[10] = FERA_RAW.LAS_FERA_ADC[0x1a];
  LAS.LAS_ADC[11] = FERA_RAW.LAS_FERA_ADC[0x1b];
  LAS.LAS_MADC[0] = sqrt(LAS.LAS_ADC[0]*LAS.LAS_ADC[1]);
  LAS.LAS_MADC[1] = sqrt(LAS.LAS_ADC[2]*LAS.LAS_ADC[3]);
  LAS.LAS_MADC[2] = sqrt(LAS.LAS_ADC[4]*LAS.LAS_ADC[5]);
  LAS.LAS_MADC[3] = sqrt(LAS.LAS_ADC[6]*LAS.LAS_ADC[7]);
  LAS.LAS_MADC[4] = sqrt(LAS.LAS_ADC[8]*LAS.LAS_ADC[9]);
  LAS.LAS_MADC[5] = sqrt(LAS.LAS_ADC[10]*LAS.LAS_ADC[11]);
  LAS.LAS_TDC[0] = FERA_RAW.LAS_FERA_TDC[0x10];
  LAS.LAS_TDC[1] = FERA_RAW.LAS_FERA_TDC[0x12];
  LAS.LAS_TDC[2] = FERA_RAW.LAS_FERA_TDC[0x14];
  LAS.LAS_TDC[3] = FERA_RAW.LAS_FERA_TDC[0x16];
  LAS.LAS_TDC[4] = FERA_RAW.LAS_FERA_TDC[0x18];
  LAS.LAS_TDC[5] = FERA_RAW.LAS_FERA_TDC[0x1a];
  LAS.LAS_TDC[6] = FERA_RAW.LAS_FERA_TDC[0x1c];
  LAS.LAS_TDC[7] = FERA_RAW.LAS_FERA_TDC[0x1e];
  LAS.LAS_TDC[8] = FERA_RAW.LAS_FERA_TDC[0x20];
  LAS.LAS_TDC[9] = FERA_RAW.LAS_FERA_TDC[0x22];
  LAS.LAS_TDC[10] = FERA_RAW.LAS_FERA_TDC[0x24];
  LAS.LAS_TDC[11] = FERA_RAW.LAS_FERA_TDC[0x26];
  LAS.LAS_RF[0] = FERA_RAW.LAS_FERA_TDC[0x2a];
  LAS.LAS_RF[1] = FERA_RAW.LAS_FERA_TDC[0x2c];
  LAS.LAS_RF[2] = FERA_RAW.LAS_FERA_TDC[0x2e];
  LAS.LAS_TDIFF[0] = LAS.LAS_TDC[1]-LAS.LAS_TDC[0];
  LAS.LAS_TDIFF[1] = LAS.LAS_TDC[3]-LAS.LAS_TDC[2];
  LAS.LAS_TDIFF[2] = LAS.LAS_TDC[5]-LAS.LAS_TDC[4];
  LAS.LAS_TDIFF[3] = LAS.LAS_TDC[7]-LAS.LAS_TDC[6];
  LAS.LAS_TDIFF[4] = LAS.LAS_TDC[9]-LAS.LAS_TDC[8];
  LAS.LAS_TDIFF[5] = LAS.LAS_TDC[11]-LAS.LAS_TDC[10];

  GF.GF_ADC_XU[0] = FERA_RAW.GR_FERA_ADC[0x20];
  GF.GF_ADC_XU[1] = FERA_RAW.GR_FERA_ADC[0x21];
  GF.GF_ADC_XU[2] = FERA_RAW.GR_FERA_ADC[0x22];
  GF.GF_ADC_XU[3] = FERA_RAW.GR_FERA_ADC[0x23];
  GF.GF_ADC_XU[4] = FERA_RAW.GR_FERA_ADC[0x24];
  GF.GF_ADC_XU[5] = FERA_RAW.GR_FERA_ADC[0x25];
  GF.GF_ADC_XU[6] = FERA_RAW.GR_FERA_ADC[0x26];
  GF.GF_ADC_XU[7] = FERA_RAW.GR_FERA_ADC[0x27];
  GF.GF_ADC_XD[0] = FERA_RAW.GR_FERA_ADC[0x28];
  GF.GF_ADC_XD[1] = FERA_RAW.GR_FERA_ADC[0x29];
  GF.GF_ADC_XD[2] = FERA_RAW.GR_FERA_ADC[0x2a];
  GF.GF_ADC_XD[3] = FERA_RAW.GR_FERA_ADC[0x2b];
  GF.GF_ADC_XD[4] = FERA_RAW.GR_FERA_ADC[0x2c];
  GF.GF_ADC_XD[5] = FERA_RAW.GR_FERA_ADC[0x2d];
  GF.GF_ADC_XD[6] = FERA_RAW.GR_FERA_ADC[0x2e];
  GF.GF_ADC_XD[7] = FERA_RAW.GR_FERA_ADC[0x2f];
  GF.GF_ADC_YL[0] = FERA_RAW.GR_FERA_ADC[0x16];
  GF.GF_ADC_YL[1] = FERA_RAW.GR_FERA_ADC[0x17];
  GF.GF_ADC_YL[2] = FERA_RAW.GR_FERA_ADC[0x18];
  GF.GF_ADC_YL[3] = FERA_RAW.GR_FERA_ADC[0x19];
  GF.GF_ADC_YL[4] = FERA_RAW.GR_FERA_ADC[0x1a];
  GF.GF_ADC_YR[0] = FERA_RAW.GR_FERA_ADC[0x1b];
  GF.GF_ADC_YR[1] = FERA_RAW.GR_FERA_ADC[0x1c];
  GF.GF_ADC_YR[2] = FERA_RAW.GR_FERA_ADC[0x1d];
  GF.GF_ADC_YR[3] = FERA_RAW.GR_FERA_ADC[0x1e];
  GF.GF_ADC_YR[4] = FERA_RAW.GR_FERA_ADC[0x1f];
  GF.GF_MADC_X[0] = sqrt(GF.GF_ADC_XU[0]*GF.GF_ADC_XD[0]);
  GF.GF_MADC_X[1] = sqrt(GF.GF_ADC_XU[1]*GF.GF_ADC_XD[1]);
  GF.GF_MADC_X[2] = sqrt(GF.GF_ADC_XU[2]*GF.GF_ADC_XD[2]);
  GF.GF_MADC_X[3] = sqrt(GF.GF_ADC_XU[3]*GF.GF_ADC_XD[3]);
  GF.GF_MADC_X[4] = sqrt(GF.GF_ADC_XU[4]*GF.GF_ADC_XD[4]);
  GF.GF_MADC_X[5] = sqrt(GF.GF_ADC_XU[5]*GF.GF_ADC_XD[5]);
  GF.GF_MADC_X[6] = sqrt(GF.GF_ADC_XU[6]*GF.GF_ADC_XD[6]);
  GF.GF_MADC_X[7] = sqrt(GF.GF_ADC_XU[7]*GF.GF_ADC_XD[7]);
  GF.GF_MADC_Y[0] = sqrt(GF.GF_ADC_YL[0]*GF.GF_ADC_YR[0]);
  GF.GF_MADC_Y[1] = sqrt(GF.GF_ADC_YL[1]*GF.GF_ADC_YR[1]);
  GF.GF_MADC_Y[2] = sqrt(GF.GF_ADC_YL[2]*GF.GF_ADC_YR[2]);
  GF.GF_MADC_Y[3] = sqrt(GF.GF_ADC_YL[3]*GF.GF_ADC_YR[3]);
  GF.GF_MADC_Y[4] = sqrt(GF.GF_ADC_YL[4]*GF.GF_ADC_YR[4]);
  GF.GF_TDC_XU[0] = FERA_RAW.GR_FERA_TDC[0x20];
  GF.GF_TDC_XU[1] = FERA_RAW.GR_FERA_TDC[0x21];
  GF.GF_TDC_XU[2] = FERA_RAW.GR_FERA_TDC[0x22];
  GF.GF_TDC_XU[3] = FERA_RAW.GR_FERA_TDC[0x23];
  GF.GF_TDC_XU[4] = FERA_RAW.GR_FERA_TDC[0x24];
  GF.GF_TDC_XU[5] = FERA_RAW.GR_FERA_TDC[0x25];
  GF.GF_TDC_XU[6] = FERA_RAW.GR_FERA_TDC[0x26];
  GF.GF_TDC_XU[7] = FERA_RAW.GR_FERA_TDC[0x27];
  GF.GF_TDC_XD[0] = FERA_RAW.GR_FERA_TDC[0x28];
  GF.GF_TDC_XD[1] = FERA_RAW.GR_FERA_TDC[0x29];
  GF.GF_TDC_XD[2] = FERA_RAW.GR_FERA_TDC[0x2a];
  GF.GF_TDC_XD[3] = FERA_RAW.GR_FERA_TDC[0x2b];
  GF.GF_TDC_XD[4] = FERA_RAW.GR_FERA_TDC[0x2c];
  GF.GF_TDC_XD[5] = FERA_RAW.GR_FERA_TDC[0x2d];
  GF.GF_TDC_XD[6] = FERA_RAW.GR_FERA_TDC[0x2e];
  GF.GF_TDC_XD[7] = FERA_RAW.GR_FERA_TDC[0x2f];
  GF.GF_TDC_YL[0] = FERA_RAW.GR_FERA_TDC[0x16];
  GF.GF_TDC_YL[1] = FERA_RAW.GR_FERA_TDC[0x17];
  GF.GF_TDC_YL[2] = FERA_RAW.GR_FERA_TDC[0x18];
  GF.GF_TDC_YL[3] = FERA_RAW.GR_FERA_TDC[0x19];
  GF.GF_TDC_YL[4] = FERA_RAW.GR_FERA_TDC[0x1a];
  GF.GF_TDC_YR[0] = FERA_RAW.GR_FERA_TDC[0x1b];
  GF.GF_TDC_YR[1] = FERA_RAW.GR_FERA_TDC[0x1c];
  GF.GF_TDC_YR[2] = FERA_RAW.GR_FERA_TDC[0x1d];
  GF.GF_TDC_YR[3] = FERA_RAW.GR_FERA_TDC[0x1e];
  GF.GF_TDC_YR[4] = FERA_RAW.GR_FERA_TDC[0x1f];
  GF.GF_TDIFF_X[0] = GF.GF_TDC_XD[0]-GF.GF_TDC_XU[0];
  GF.GF_TDIFF_X[1] = GF.GF_TDC_XD[1]-GF.GF_TDC_XU[1];
  GF.GF_TDIFF_X[2] = GF.GF_TDC_XD[2]-GF.GF_TDC_XU[2];
  GF.GF_TDIFF_X[3] = GF.GF_TDC_XD[3]-GF.GF_TDC_XU[3];
  GF.GF_TDIFF_X[4] = GF.GF_TDC_XD[4]-GF.GF_TDC_XU[4];
  GF.GF_TDIFF_X[5] = GF.GF_TDC_XD[5]-GF.GF_TDC_XU[5];
  GF.GF_TDIFF_X[6] = GF.GF_TDC_XD[6]-GF.GF_TDC_XU[6];
  GF.GF_TDIFF_X[7] = GF.GF_TDC_XD[7]-GF.GF_TDC_XU[7];
  GF.GF_TDIFF_Y[0] = GF.GF_TDC_YR[0]-GF.GF_TDC_YL[0];
  GF.GF_TDIFF_Y[1] = GF.GF_TDC_YR[1]-GF.GF_TDC_YL[1];
  GF.GF_TDIFF_Y[2] = GF.GF_TDC_YR[2]-GF.GF_TDC_YL[2];
  GF.GF_TDIFF_Y[3] = GF.GF_TDC_YR[3]-GF.GF_TDC_YL[3];
  GF.GF_TDIFF_Y[4] = GF.GF_TDC_YR[4]-GF.GF_TDC_YL[4];
 
  return;
  BLP_ADC[0] = GR_ADC_OLD[0];
  BLP_ADC[1] = GR_ADC_OLD[1];
  BLP_ADC[2] = GR_ADC_OLD[2];
  BLP_ADC[3] = GR_ADC_OLD[3];
  BLP_ADC[4] = GR_ADC_OLD[4];
  BLP_ADC[5] = GR_ADC_OLD[5];
  BLP_ADC[6] = GR_ADC_OLD[6];
  BLP_ADC[7] = GR_ADC_OLD[7];
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
  FERA_RAW.Clear();
}
