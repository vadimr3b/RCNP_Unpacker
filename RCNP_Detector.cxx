#include "RCNP_Detector.h"
using std::vector;
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
using std::hex;

extern Bool_t bigEndian;

vector<UShort_t> RegionIDs;

vector<UShort_t> IPR_vector;
UInt_t IPR;

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

vector<UShort_t> GR_WIRE_X1;
vector<UShort_t> GR_WIRE_U1;
vector<UShort_t> GR_WIRE_X2;
vector<UShort_t> GR_WIRE_U2;
vector<Float_t> GR_TDC_X1;
vector<Float_t> GR_TDC_U1;
vector<Float_t> GR_TDC_X2;
vector<Float_t> GR_TDC_U2;

vector<UShort_t> LAS_WIRE_X1;
vector<UShort_t> LAS_WIRE_U1;
vector<UShort_t> LAS_WIRE_V1;
vector<UShort_t> LAS_WIRE_X2;
vector<UShort_t> LAS_WIRE_U2;
vector<UShort_t> LAS_WIRE_V2;
vector<Float_t> LAS_TDC_X1;
vector<Float_t> LAS_TDC_U1;
vector<Float_t> LAS_TDC_V1;
vector<Float_t> LAS_TDC_X2;
vector<Float_t> LAS_TDC_U2;
vector<Float_t> LAS_TDC_V2;

Int_t V1190_QTC[16][2];

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
    Header = (data >> 15) & 0x1;
    Count = (data >> 11) & 0xF;
    Reserved = (data >> 8) & 07;
    VStationNum = (data >> 0) & 0xFF;
    
    Type = (VStationNum & 0xF0) >> 4;
    Mid = (VStationNum & 0xF);
  }
  
  Bool_t IsHeader(UShort_t data){
    return ((data >> 15) & 0x1);
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
    Header = (data >> 15) & 0x1;
    Channel = (data >> 11) & 0xF;
    Data = (data >> 0) & 0x7FF;
  }
  
  Bool_t IsData(UShort_t data){
    return !((data >> 15) & 0x1);
  }
  
  UShort_t Header;        /* Identifies header word (=0) */ 
  UShort_t Channel;       /* Channel number */
  UShort_t Data;          /* Data */
};

#define V1190_MAX_N_MODULES 32

constexpr Short_t V1190_Wire_Map[] = {
  0, 2, 1, 3, 4, 6, -2, -1, 5, 7, 8, 10, 9, 11, -1, -1,  // X1 plane
  1, 3, 2, 4, 5, 7,  0, -1, 6, 8, 9, 11,10, 12, -1, -1,  // U1 plane
  0, 2, 1, 3, 4, 6, -1, -1, 5, 7, 8, 10, 9, 11, -1, -1,  // X2 plane
  1, 3, 2, 4, 5, 7,  0, -1, 6, 8, 9, 11,10, 12, -1, -1   // U2 plane
};
  
struct V1190Data{
  void Read(UInt_t data){
      ID = (data >> 27) & 0x1F;
      switch(ID){
          case(ID_GlobalHeader):
              Geo = (data >> 0) & 0x1F;
              if(0 <= Geo && Geo < 8){
                IsGeoGR = kTRUE;
                IsGeoLAS = kFALSE;
              }
              else if(8 <= Geo && Geo < 24){
                IsGeoGR = kFALSE;
                IsGeoLAS = kTRUE;
              }
              else{
                IsGeoGR = kFALSE;
                IsGeoLAS = kFALSE;
              }
              EventCount = (data >> 5) & 0x3FFFFF;
              break;
          case(ID_TDCHeader):
              BunchID = (data >> 0) & 0xFFF;
              EventID = (data >> 12) & 0xFFF;
              TDC = (data >> 24) & 0x3;
              break;
          case(ID_TDCMeasurment):
              Measurement = (data >> 0) & 0x7FFFF;
              Channel = (data >> 19) & 0x7F;
              TrailingEdge = (data >> 26) & 0x1;
              break;
          case(ID_TDCTrailer):
              WordCount = (data >> 0) & 0xFFF;
              EventID = (data >> 12) & 0xFFF;
              TDC = (data >> 24) & 0x3;
              break;
          case(ID_TDCErrorr):
              ErrorFlags = (data >> 0) & 0x7FFF;
              TDC = (data >> 24) & 0x3;
              break;
          case(ID_ETTT):
              TimeTag = (data >> 0) & 0x3FFFFFF;
              break;
          case(ID_GlobalTrailer):
              Geo = (data >> 0) & 0x1F;
              WordCount = (data >> 5) & 0xFFFF;
              Status = (data >> 24) & 0x7;
              break;
          default:
            //cerr << "Unknown ID: " << ID << endl;
            break;
      }
  }
  
  Short_t GetBaseTimeChannel(){
    if(IsGeoGR)
            return GR_BASE_TIME_CHANNEL;
    else if(IsGeoLAS){
            if((Geo & 1) == 0){
                    return LAS_EVEN_BASE_TIME_CHANNEL;
            }else{
                    return LAS_ODD_BASE_TIME_CHANNEL;
            }
    }else{
            cerr << "Unknown geo: " << Geo << endl;
            return -1;
    } 
  }
  
  Short_t GetWire(UShort_t geo, UShort_t ch){
   return (V1190_Wire_Map[(((geo) & 7) << 3) | (((ch) & 0x70) >> 4)] << 4)| ((ch) & 0x0f); 
  }
  
  Short_t GetGRVDCPlane(UShort_t geo, UShort_t ch){
     return (((geo) & 6) >> 1);
  }
  
  UShort_t GetLASVDCPlane(){
    if(Geo ==  8 || Geo ==  9 || (Geo == 11 && 96 <= Channel && Channel < 128)) return 1; //X1 plane
    if(Geo == 12 || Geo == 13) return 2; //U1 plane
    if(Geo == 14 || Geo == 15) return 3; //V1 plane
    if(Geo == 16 || Geo == 17 || (Geo == 11 && 32 <= Channel && Channel < 64)) return 4; //X2 plane
    if(Geo == 20 || Geo == 21) return 5; //U2 plane
    if(Geo == 22 || Geo == 23) return 6; //V2 plane
    return 0;
  }
  
  Short_t GetLASVDCWire(){
    if(Geo == 11 && 48 <= Channel && Channel < 64)
        return 16*3  + Channel - 48;
    else if(Geo == 16 && 48 <= Channel && Channel < 64)
        return 16*16 + Channel - 48;
    else if(Geo == 11){
        if(32 <= Channel && Channel < 64)
            return 16*15 + (Channel - 32);
        else if(96 <= Channel && Channel < 128)
            return 16*15 + (Channel - 96);
    }else{
        return (Geo%2)*128 + Channel;
    }
    cerr << "Bad Wire!" << endl;
    return -1;
  }
  
  static constexpr UShort_t GR_BASE_TIME_CHANNEL = 127;
  static constexpr UShort_t LAS_EVEN_BASE_TIME_CHANNEL = 0;
  static constexpr UShort_t LAS_ODD_BASE_TIME_CHANNEL = 127;
  
  /*Common*/
  UShort_t ID;
  UShort_t TDC;
  Short_t Geo = -1;
  Bool_t IsGeoGR;
  Bool_t IsGeoLAS;
  UShort_t EventID;
  
  /*Global Header (ID == 8)*/
  static constexpr UShort_t ID_GlobalHeader = 8;
  UInt_t EventCount;
  
  /*TDC Header (ID == 1)*/
  static constexpr UShort_t ID_TDCHeader = 1;
  UShort_t BunchID;
  
  /*TDC Measurement (ID == 0)*/
  static constexpr UShort_t ID_TDCMeasurment = 0;
  Int_t Measurement;
  UShort_t Channel;
  UShort_t TrailingEdge; /*0 = leading, 1 = trailing*/
  
  /*TDC Trailer (ID == 3)*/
  static constexpr UShort_t ID_TDCTrailer= 3;
  
  /*TDC Error (ID == 4)*/
  static constexpr UShort_t ID_TDCErrorr = 4;
  UShort_t ErrorFlags;
  
  /*Extended Trigger Time Tag (ID == 17)*/
  static constexpr UShort_t ID_ETTT = 17;
  UInt_t TimeTag;
  
  /*Global Trailer (ID == 16)*/
  static constexpr UShort_t ID_GlobalTrailer = 16;
  UShort_t Status;
  UShort_t WordCount;
    
};

RCNP_Detector::RCNP_Detector()
{
}

void RCNP_Detector::RegisterData(TTree& tree)
{
  //tree.Branch("RegionIDs", &RegionIDs);
  //tree.Branch("GR_V830", &GR_V830);
  tree.Branch("IPR", &IPR);
  tree.Branch("Scaler", &Scaler);
  //tree.Branch("Time", &Time);
  //tree.Branch("ChkSum", &ChkSum);
  //tree.Branch("FERA_Type", &FERA_Type);
  //tree.Branch("FERA_Ch", &FERA_Ch);
  //tree.Branch("FERA_Mid", &FERA_Mid);
  tree.Branch("EVENT", &EVENT, "EVENT_GR/O:EVENT_LAS:EVENT_COIN");
  tree.Branch("GR", &GR, "GR_ADC[6]/s:GR_MADC[3]:GR_TDC[6]:GR_RF[3]:GR_TLAS:GR_DIFF[3]");
  //tree.Branch("GF", &GF, "GF_ADC_XU[8]/s:GF_ADC_XD[8]:GF_ADC_YL[5]:GF_ADC_YR[5]:GF_MADC_X[8]:GF_MADC_Y[5]:GF_TDC_XU[8]:GF_TDC_XD[8]:GF_TDC_YL[5]:GF_TDC_YR[5]:GF_TDIFF_X[8]:GF_TDIFF_Y[5]");
  tree.Branch("LAS", &LAS, "LAS_ADC[12]/s:LAS_MADC[6]:LAS_TDC[12]:LAS_RF[3]:LAS_TDIFF[6]");
  
  tree.Branch("GR_WIRE_X1", &GR_WIRE_X1);
  tree.Branch("GR_WIRE_U1", &GR_WIRE_U1);
  tree.Branch("GR_WIRE_X2", &GR_WIRE_X2);
  tree.Branch("GR_WIRE_U2", &GR_WIRE_U2);
  
  tree.Branch("GR_TDC_X1", &GR_TDC_X1);
  tree.Branch("GR_TDC_U1", &GR_TDC_U1);
  tree.Branch("GR_TDC_X2", &GR_TDC_X2);
  tree.Branch("GR_TDC_U2", &GR_TDC_U2);
  
  tree.Branch("LAS_WIRE_X1", &LAS_WIRE_X1);
  tree.Branch("LAS_WIRE_U1", &LAS_WIRE_U1);
  tree.Branch("LAS_WIRE_V1", &LAS_WIRE_V1);
  tree.Branch("LAS_WIRE_X2", &LAS_WIRE_X2);
  tree.Branch("LAS_WIRE_U2", &LAS_WIRE_U2);
  tree.Branch("LAS_WIRE_V2", &LAS_WIRE_V2);
  
  tree.Branch("LAS_TDC_X1", &LAS_TDC_X1);
  tree.Branch("LAS_TDC_U1", &LAS_TDC_U1);
  tree.Branch("LAS_TDC_V1", &LAS_TDC_V1);
  tree.Branch("LAS_TDC_X2", &LAS_TDC_X2);
  tree.Branch("LAS_TDC_U2", &LAS_TDC_U2);
  tree.Branch("LAS_TDC_V2", &LAS_TDC_V2);
  
  tree.Branch("V1190_QTC", &V1190_QTC, "V1190_QTC[16][2]/I");
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
    
    switch(R.RID){
        case(R.ID_Reserved):
          break;
        
        case(R.ID_V1190):{
            UInt_t temp;
            UInt_t localpos = pos;
            Bool_t insideTDCLoop, insideGlobalLoop;
            V1190Data V1190D, V1190Check;
            Int_t BaseTime[V1190_MAX_N_MODULES];
            for(UInt_t i = 0; i < V1190_MAX_N_MODULES; ++i)
              BaseTime[i] = -10000;

            while(localpos < pos + R.RSize){
                temp = (UInt_t(data[localpos+1]) << 16) | (UInt_t(data[localpos]));
                localpos+=2;
                V1190D.Read(temp);
                if(V1190D.ID != V1190D.ID_GlobalHeader){
                  cerr << "Expected V1190D Global-Header! " << V1190D.ID << endl;
                  return;
                }
                insideGlobalLoop = true;
                
                while(insideGlobalLoop){
                    temp = (UInt_t(data[localpos+1]) << 16) | (UInt_t(data[localpos]));
                    localpos += 2;
                    V1190D.Read(temp);
                    if(V1190D.ID != V1190D.ID_TDCHeader){
                      //cerr << "Expected V1190D TDC-Header! " << V1190D.ID << endl;
                      if(V1190D.ID != V1190D.ID_GlobalHeader)
                        return;
                      temp = (UInt_t(data[localpos+1]) << 16) | (UInt_t(data[localpos]));
                      localpos += 2;
                      V1190D.Read(temp);
                      if(V1190D.ID != V1190D.ID_TDCHeader){ //BUG?
                        cerr << "Data broken... skipping field!" << endl;
                        return;
                      }
                    }
                    
                    insideTDCLoop = true;
                    while(insideTDCLoop){
                      temp = (UInt_t(data[localpos+1]) << 16) | (UInt_t(data[localpos]));
                      localpos += 2;
                      V1190D.Read(temp);
                      
                      switch(V1190D.ID){
                        case(V1190D.ID_TDCMeasurment):
                          if(!V1190D.TrailingEdge && V1190D.Geo >= 0 && V1190D.Geo < V1190_MAX_N_MODULES){
                            if(V1190D.IsGeoGR || V1190D.IsGeoLAS)
                              if(V1190D.Channel == V1190D.GetBaseTimeChannel())
                                BaseTime[V1190D.Geo] = V1190D.Measurement;
                          }
                        break;
                        
                        case(V1190D.ID_TDCErrorr):
                          cerr << "V1190D: TDC-Error!" << endl;
                        break;
                        
                        case(V1190D.ID_TDCTrailer):
                          insideTDCLoop = false;
                        break;
                        
                        default:
                          cerr << "Unexpected V1190D Block: " << V1190D.ID << endl;
                          return;
                      }
                    }
                    
                    temp = (UInt_t(data[localpos+1]) << 16) | (UInt_t(data[localpos]));
                    V1190Check.Read(temp);
                    if(V1190Check.ID == V1190Check.ID_GlobalTrailer){
                      V1190D.Read(temp);
                      localpos += 2;
                      insideGlobalLoop = false;
                    }
                }
            }
            localpos = pos;
            _V1190_RAW raw;
            while(localpos < pos + R.RSize){
                temp = (UInt_t(data[localpos+1]) << 16) | (UInt_t(data[localpos]));
                localpos+=2;
                V1190D.Read(temp);
                if(V1190D.ID != V1190D.ID_GlobalHeader){
                  cerr << "Expected V1190D Global-Header! " << V1190D.ID << endl;
                  return;
                }
                insideGlobalLoop = true;
                
                while(insideGlobalLoop){
                    temp = (UInt_t(data[localpos+1]) << 16) | (UInt_t(data[localpos]));
                    localpos += 2;
                    V1190D.Read(temp);
                    if(V1190D.ID != V1190D.ID_TDCHeader){
                      //cerr << "Expected V1190D TDC-Header! " << V1190D.ID << endl;
                      if(V1190D.ID != V1190D.ID_GlobalHeader)
                        return;
                      temp = (UInt_t(data[localpos+1]) << 16) | (UInt_t(data[localpos]));
                      localpos += 2;
                      V1190D.Read(temp);
                      if(V1190D.ID != V1190D.ID_TDCHeader){
                        cerr << "Data broken... skipping field!" << endl; //BUG in DAQ?
                        return;
                      }
                    }
                    raw.Geo= V1190D.Geo;
                    
                    insideTDCLoop = true;
                    while(insideTDCLoop){
                      temp = (UInt_t(data[localpos+1]) << 16) | (UInt_t(data[localpos]));
                      localpos += 2;
                      V1190D.Read(temp);
                      
                      switch(V1190D.ID){
                        case(V1190D.ID_TDCMeasurment):
                            raw.Channel = V1190D.Channel;
                            raw.Wire = V1190D.GetWire(raw.Geo, raw.Channel);
                            if(BaseTime[raw.Geo] == -10000 && raw.Geo != 10)
                              cerr << "BaseTime Error!" << endl;
                            else{
                              raw.TDC = (V1190D.Measurement - BaseTime[raw.Geo])/10.0;
                              if(V1190D.Channel > 0)
                              if(raw.Geo == 0 && (96 <= raw.Channel && raw.Channel < 112)){
                                if(V1190D.TrailingEdge)
                                  V1190_QTC[raw.Channel - 96][1] = V1190D.Measurement - BaseTime[raw.Geo];
                                else
                                  V1190_QTC[raw.Channel - 96][0] = V1190D.Measurement - BaseTime[raw.Geo];
                              }
                              
                              if(!V1190D.TrailingEdge){
                                if(V1190D.IsGeoGR){
                                  if(raw.Wire >= 0)
                                    switch(V1190D.GetGRVDCPlane(raw.Geo, raw.Channel)){
                                        case 0: // X1 Plane
                                          GR_TDC_X1.push_back(-raw.TDC);
                                          GR_WIRE_X1.push_back(raw.Wire);
                                          break;
                                        case 1: // U1 Plane
                                          GR_TDC_U1.push_back(-raw.TDC);
                                          GR_WIRE_U1.push_back(raw.Wire);
                                          break;
                                        case 2: // X2 Plane
                                          GR_TDC_X2.push_back(-raw.TDC);
                                          GR_WIRE_X2.push_back(raw.Wire);
                                          break;
                                        case 3: // U2 Plane
                                          GR_TDC_U2.push_back(-raw.TDC);
                                          GR_WIRE_U2.push_back(raw.Wire);
                                          break;
                                    }
                                }
                                else if(V1190D.IsGeoLAS){
                                  if(raw.Channel == V1190D.GetBaseTimeChannel())
                                    break;
                                  if(V1190D.GetLASVDCWire() >= 0){
                                    const Short_t TDC_Offset = 750;
                                    switch(V1190D.GetLASVDCPlane()){
                                        case 1://X1 plane 
                                          LAS_TDC_X1.push_back(-raw.TDC - TDC_Offset);
                                          LAS_WIRE_X1.push_back(V1190D.GetLASVDCWire());
                                          break;
                                        case 2://U1 plane 
                                          LAS_TDC_U1.push_back(-raw.TDC - TDC_Offset);
                                          LAS_WIRE_U1.push_back(V1190D.GetLASVDCWire());
                                          break;
                                        case 3://V1 plane 
                                          LAS_TDC_V1.push_back(-raw.TDC - TDC_Offset);
                                          LAS_WIRE_V1.push_back(V1190D.GetLASVDCWire());
                                          break;
                                        case 4://X2 plane 
                                          LAS_TDC_X2.push_back(-raw.TDC - TDC_Offset);
                                          LAS_WIRE_X2.push_back(V1190D.GetLASVDCWire());
                                          break;
                                        case 5://U2 plane 
                                          LAS_TDC_U2.push_back(-raw.TDC - TDC_Offset);
                                          LAS_WIRE_U2.push_back(V1190D.GetLASVDCWire());
                                          break;
                                        case 6://V2 plane 
                                          LAS_TDC_V2.push_back(-raw.TDC - TDC_Offset);
                                          LAS_WIRE_V2.push_back(V1190D.GetLASVDCWire());
                                    }
                                  }
                                }
                              }
                          break;
                          
                          case(V1190D.ID_TDCErrorr):
                            cerr << "V1190D: TDC-Error!" << endl;
                          break;
                          
                          case(V1190D.ID_TDCTrailer):
                            insideTDCLoop = false;
                          break;
                          
                          default:
                            cerr << "Unexpected V1190D Block: " << V1190D.ID << endl;
                            return;
                        }
                      }
                    }
                    
                    temp = (UInt_t(data[localpos+1]) << 16) | (UInt_t(data[localpos]));
                    V1190Check.Read(temp);
                    if(V1190Check.ID == V1190Check.ID_GlobalTrailer){
                      localpos += 2;
                      insideGlobalLoop = false;
                    }
                }
            }
        }
        break;
        
        case(R.ID_NimIn):
            for(UInt_t i = pos; i < pos+R.RSize; ++i)
                IPR_vector.push_back(data[i]);
        break;
            
        case(R.ID_ADC):
            for(UInt_t i = pos; i < pos+R.RSize; ++i)
                GR_ADC_OLD.push_back(data[i]);
            //GR_ADC_OLD[i-pos] = (data[i]);
        break;
            
        case(R.ID_Scaler):
            for(UInt_t i = pos; i < pos+R.RSize; i+=2){
                if(bigEndian)
                  Scaler.push_back(data[i] | (data[i+1] << 16));
                else
                  Scaler.push_back(data[i+1] | (data[i] << 16));
            }
        break;
      
        case(R.ID_UNIX_TIME):
            Time = (data[pos] | (data[pos+1] << 16));
        break;
        
        case(R.ID_V830):
            for(UInt_t i = pos; i < pos+R.RSize; i+=2)
                GR_V830.push_back(data[i] | (data[i+1] << 16));
            //NOTE Only Debug purpose?
        break;

        case(R.ID_FERA_ADC):
        case(R.ID_FERA_TDC):{
            FeraHeader FH;
            FeraData FD;
            UInt_t header_left = 3;
            for(UInt_t i = pos; i < pos+R.RSize && header_left > 0; --header_left){
                if(!FH.IsHeader(data[i]) && R.RSize < 17){
                    cerr << "Expected a Fera-Header!" << endl;
                    return;
                }
                FH.Read(data[i++]);
                if(R.RSize == 17){ //BUG in the DAQ?
                  FH.Header = 1;
                  FH.Count = 16;
                }

                FERA_Type.push_back(FH.Type);
                FERA_Mid.push_back(FH.Mid);
                for(UInt_t j = FH.Count; j > 0; --j){
                    FD.Read(data[i++]);
                    FERA_Ch.push_back(FD.Channel);
                    if(FH.Type == 0)
                        FERA_RAW.GR_FERA_ADC[FD.Channel+16*FH.Mid] = FD.Data;
                    else if(FH.Type == 1)
                        FERA_RAW.LAS_FERA_ADC[FD.Channel+16*FH.Mid] = FD.Data;
                    else if(FH.Type == 8)
                        FERA_RAW.GR_FERA_TDC[FD.Channel+16*FH.Mid] = FD.Data;
                    else if(FH.Type == 9)
                        FERA_RAW.LAS_FERA_TDC[FD.Channel+16*FH.Mid] = FD.Data;
                }
            }
        }
        break;
            
        case(R.ID_CHKSUM):
          ChkSum = data[pos];
        break;
            
        default:
            cout << "Unhandeld Region found: " << hex << R.RID << endl;
    }
    pos += R.RSize;
  }
  if(IPR_vector.size() == 2)
    IPR = (UInt_t(IPR_vector[1]) << 16) | IPR_vector[0];
  else
    IPR = IPR_vector[0];
  
  EVENT.EVENT_COIN = IPR_vector[0] & (1 << 8);
  EVENT.EVENT_GR = (IPR_vector[0] & (1 << 5)) || EVENT.EVENT_COIN;
  EVENT.EVENT_LAS = (IPR_vector[0] & (1 << 7)) || EVENT.EVENT_COIN;
  
  GR.GR_ADC[0] = FERA_RAW.GR_FERA_ADC[16];
  GR.GR_ADC[1] = FERA_RAW.GR_FERA_ADC[17];
  GR.GR_ADC[2] = FERA_RAW.GR_FERA_ADC[18];
  GR.GR_ADC[3] = FERA_RAW.GR_FERA_ADC[19];
  GR.GR_ADC[4] = FERA_RAW.GR_FERA_ADC[32];
  GR.GR_ADC[5] = FERA_RAW.GR_FERA_ADC[40];
  GR.GR_MADC[0] = sqrt(GR.GR_ADC[1]*GR.GR_ADC[0]);
  GR.GR_MADC[1] = sqrt(GR.GR_ADC[3]*GR.GR_ADC[2]);
  GR.GR_MADC[2] = sqrt(GR.GR_ADC[5]*GR.GR_ADC[4]);
  GR.GR_TDC[0] = FERA_RAW.GR_FERA_TDC[16];
  GR.GR_TDC[1] = FERA_RAW.GR_FERA_TDC[17];
  GR.GR_TDC[2] = FERA_RAW.GR_FERA_TDC[18];
  GR.GR_TDC[3] = FERA_RAW.GR_FERA_TDC[19];
  GR.GR_TDC[4] = FERA_RAW.GR_FERA_TDC[32];
  GR.GR_TDC[5] = FERA_RAW.GR_FERA_TDC[40];
  GR.GR_RF[0] = FERA_RAW.GR_FERA_TDC[24];
  GR.GR_RF[1] = FERA_RAW.GR_FERA_TDC[25];
  GR.GR_RF[2] = FERA_RAW.GR_FERA_TDC[26];
  GR.GR_TLAS = FERA_RAW.GR_FERA_TDC[21];
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
  IPR_vector.clear();
  GR_ADC_OLD.clear();
  Scaler.clear();
  GR_V830.clear();
  FERA_Type.clear();
  FERA_Ch.clear();
  FERA_Mid.clear();
  Time = 0;
  ChkSum = 0;
  FERA_RAW.Clear();
  
  GR_WIRE_X1.clear();
  GR_WIRE_U1.clear();
  GR_WIRE_X2.clear();
  GR_WIRE_U2.clear();
  GR_TDC_X1.clear();
  GR_TDC_U1.clear();
  GR_TDC_X2.clear();
  GR_TDC_U2.clear();
  
  LAS_WIRE_X1.clear();
  LAS_WIRE_U1.clear();
  LAS_WIRE_V1.clear();
  LAS_WIRE_X2.clear();
  LAS_WIRE_U2.clear();
  LAS_WIRE_V2.clear();
  LAS_TDC_X1.clear();
  LAS_TDC_U1.clear();
  LAS_TDC_V1.clear();
  LAS_TDC_X2.clear();
  LAS_TDC_U2.clear();
  LAS_TDC_V2.clear();
  
  memset(&V1190_QTC, 0, sizeof(V1190_QTC));
}
