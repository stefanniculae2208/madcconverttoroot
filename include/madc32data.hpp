#ifndef madc32data_hpp
#define madc32data_hpp

#include "TString.h"
#include "TTree.h"

#include "TreeData.h"

#include <iostream>

class madc32data {

public:
  madc32data();
  ~madc32data();

  madc32data(TString name);

  void addEvent(TreeData loc_data);

  void writeEvent(int evConter, int hoghBits);

  void writeTree();

  void newTree();

  TString filename;
  TTree *roottree;

  UChar_t Mod, Ch;
  ULong64_t TimeStamp = 0;
  Double_t FineTS = 0;
  UShort_t ChargeLong = 0;
  UShort_t ChargeShort = 0;
  UInt_t RecordLength = 2;
  UShort_t Signal[2]{0};

  std::vector<TreeData> data_vec;
};

#endif