#ifndef RCNPTREE_RAW_H
#define RCNPTREE_RAW_H

struct _FERA_RAW{
  UShort_t GR_FERA_ADC[64];
  UShort_t LAS_FERA_ADC[64];
  UShort_t GR_FERA_TDC[64];
  UShort_t LAS_FERA_TDC[64];
  
  void Clear(){
    memset(GR_FERA_ADC, 0, sizeof(GR_FERA_ADC));
    memset(LAS_FERA_ADC, 0, sizeof(LAS_FERA_ADC));
    memset(GR_FERA_TDC, 0, sizeof(GR_FERA_TDC));
    memset(LAS_FERA_TDC, 0, sizeof(LAS_FERA_TDC));
  }
};

struct _EVENT{
  Bool_t EVENT_GR;
  Bool_t EVENT_LAS;
  Bool_t EVENT_COIN;
};

struct _GR{
  UShort_t GR_ADC[6];
  UShort_t GR_MADC[3];
  UShort_t GR_TDC[6];
  UShort_t GR_RF[3];
  UShort_t GR_TLAS;
  UShort_t R_TDIFF[3];
};

struct _GF{
  UShort_t GF_ADC_XU[8];
  UShort_t GF_ADC_XD[8];
  UShort_t GF_ADC_YL[5];
  UShort_t GF_ADC_YR[5];
  UShort_t GF_MADC_X[8];
  UShort_t GF_MADC_Y[5];
  UShort_t GF_TDC_XU[8];
  UShort_t GF_TDC_XD[8];
  UShort_t GF_TDC_YL[5];
  UShort_t GF_TDC_YR[5];
  UShort_t GF_TDIFF_X[8];
  UShort_t GF_TDIFF_Y[5];
};

struct _LAS{
  UShort_t LAS_ADC[12];
  UShort_t LAS_MADC[6];
  UShort_t LAS_TDC[12];
  UShort_t LAS_RF[3];
  UShort_t LAS_TDIFF[6];
};

struct _V1190_RAW{
  Float_t TDC;
  UShort_t Geo;
  UShort_t Channel;
  Short_t Wire;
};

#endif