#include "../include/madc32data.hpp"



madc32data::madc32data(){};

madc32data::~madc32data(){};



madc32data::madc32data(TString name)
{

    filename = name;

    roottree = new TTree("MADC32_tree", "Data from MADC32");

        
    roottree->Branch("Mod", &Mod, "Mod/b");
    roottree->Branch("Ch", &Ch, "Ch/b");
    roottree->Branch("TimeStamp", &TimeStamp, "TimeStamp/l");
    roottree->Branch("FineTS", &FineTS, "Finets/D");
    roottree->Branch("ChargeLong", &ChargeLong, "ChargeLong/s");
    roottree->Branch("ChargeShort", &ChargeShort, "ChargeShort/s");
    roottree->Branch("RecordLength", &RecordLength, "RecordLength/i");
    roottree->Branch("Signal", Signal, "Signal[RecordLength]/s");
    
}

void madc32data::newTree()
{

    roottree = new TTree("MADC32_tree", "Data from MADC32");

        
    roottree->Branch("Mod", &Mod, "Mod/b");
    roottree->Branch("Ch", &Ch, "Ch/b");
    roottree->Branch("TimeStamp", &TimeStamp, "TimeStamp/l");
    roottree->Branch("FineTS", &FineTS, "Finets/D");
    roottree->Branch("ChargeLong", &ChargeLong, "ChargeLong/s");
    roottree->Branch("ChargeShort", &ChargeShort, "ChargeShort/s");
    roottree->Branch("RecordLength", &RecordLength, "RecordLength/i");
    roottree->Branch("Signal", Signal, "Signal[RecordLength]/s");

}


void madc32data::addEvent(TreeData loc_data)
{

    data_vec.push_back(loc_data);

}




void madc32data::writeEvent(int evCounter)
{


    for(auto i = 0; i < data_vec.size(); i++){

        Mod = data_vec.at(i).Mod;
        Ch = data_vec.at(i).Ch;
        ChargeLong = data_vec.at(i).ChargeLong;
        TimeStamp = evCounter;
        roottree->Fill();

    }


    std::vector<TreeData>().swap(data_vec);




/*     Mod = loc_data.Mod;
    Ch = loc_data.Ch;
    ChargeLong = loc_data.ChargeLong;

    roottree->Fill(); */
    

    



}


void madc32data::writeTree()
{

    roottree->Write();

}