#include <fstream>
#include <iostream>
#include <string.h>
#include <cstring>
#include <map>
#include <bitset>
#include <stdlib.h>

#include "TFile.h"
#include "TROOT.h"


#include "../include/madc32data.hpp"
#include "../include/TRealData.h"


typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;



struct listfile_v1
{
    static const int Version = 1;

    static const int FirstSectionOffset = 8;

    static const int SectionMaxWords  = 0xfffff;
    static const int SectionMaxSize   = SectionMaxWords * sizeof(u32);

    static const int SectionTypeMask  = 0xe0000000; // 3 bit section type
    static const int SectionTypeShift = 29;
    static const int SectionSizeMask  = 0x000fffff; // 20 bit section size in 32 bit words
    static const int SectionSizeShift = 0;
    static const int EventTypeMask    = 0x1e000000; // 4 bit event type
    static const int EventTypeShift   = 25;

    // Subevent containing module data
    static const int ModuleTypeMask  = 0xff000000;  // 8 bit module type
    static const int ModuleTypeShift = 24;

    static const int SubEventMaxWords  = 0xfffff;
    static const int SubEventMaxSize   = SubEventMaxWords * sizeof(u32);
    static const int SubEventSizeMask  = 0x000fffff; // 20 bit subevent size in 32 bit words
    static const int SubEventSizeShift = 0;
};

namespace listfile
{
    enum SectionType
    {
        /* The config section contains the mvmecfg as a json string padded with
         * spaces to the next 32 bit boundary. If the config data size exceeds
         * the maximum section size multiple config sections will be written at
         * the start of the file. */
        SectionType_Config      = 0,

        /* Readout data generated by one VME Event. Contains Subevent Headers
         * to split into VME Module data. */
        SectionType_Event       = 1,

        /* Last section written to a listfile before closing the file. Used for
         * verification purposes. */
        SectionType_End         = 2,

        /* Marker section written once at the start of a run and then once per
         * elapsed second. */
        SectionType_Timetick    = 3,

        /* Max section type possible. */
        SectionType_Max         = 7
    };

    enum VMEModuleType
    {
        Invalid         = 0,
        MADC32          = 1,
        MQDC32          = 2,
        MTDC32          = 3,
        MDPP16_SCP      = 4,
        MDPP32          = 5,
        MDI2            = 6,
        MDPP16_RCP      = 7,
        MDPP16_QDC      = 8,
        VMMR            = 9,

        MesytecCounter = 16,
        VHS4030p = 21,
    };

    static const std::map<VMEModuleType, const char *> VMEModuleTypeNames =
    {
        { VMEModuleType::MADC32,            "MADC-32" },
        { VMEModuleType::MQDC32,            "MQDC-32" },
        { VMEModuleType::MTDC32,            "MTDC-32" },
        { VMEModuleType::MDPP16_SCP,        "MDPP-16_SCP" },
        { VMEModuleType::MDPP32,            "MDPP-32" },
        { VMEModuleType::MDI2,              "MDI-2" },
        { VMEModuleType::MDPP16_RCP,        "MDPP-16_RCP" },
        { VMEModuleType::MDPP16_QDC,        "MDPP-16_QDC" },
        { VMEModuleType::VMMR,              "VMMR" },
        { VMEModuleType::VHS4030p,          "iseg VHS4030p" },
        { VMEModuleType::MesytecCounter,    "Mesytec Counter" },
    };

    const char *get_vme_module_name(VMEModuleType moduleType)
    {
        auto it = VMEModuleTypeNames.find(moduleType);
        if (it != VMEModuleTypeNames.end())
        {
            return it->second;
        }

        return "unknown";
    }

} // end namespace listfile
typedef listfile_v1 LF;

inline int bitExtractor(int word, int numbits, int position){
    return (((1 << numbits) - 1) & (word >> position)); 
}




void convert_file(std::ifstream &infile, TString filename, TString dir)
{

    using namespace listfile;


    TString rootfilename = dir + "rootfiles/" + filename + "_0.root";
    TFile *rootfile = new TFile(rootfilename, "RECREATE");


    madc32data madc_data(rootfilename);


    bool continueReading = true;
    int counter = 0;
    int sig = 0;

    int moduleId = 0;


    TreeData loc_data;



    while(continueReading){

        u32 sectionHeader;
        infile.read((char *)&sectionHeader, sizeof(u32));

        u32 sectionType   = (sectionHeader & LF::SectionTypeMask) >> LF::SectionTypeShift;
        u32 sectionSize   = (sectionHeader & LF::SectionSizeMask) >> LF::SectionSizeShift;


        switch (sectionType)
        {
            case SectionType_Config:
                {
                
                    //std::cout << "Config section of size " << sectionSize << std::endl;
                    infile.seekg(sectionSize * sizeof(u32), std::ifstream::cur);

                } break;






            case SectionType_Event:
                {

                    if((counter%1000000==0) && (counter != 0)){


                        madc_data.writeTree();
                        rootfile->Close();

                        delete(rootfile);

                        rootfilename = dir + "rootfiles/" + filename + "_" + std::to_string(counter/1000000) + ".root";

                        rootfile = new TFile(rootfilename, "RECREATE");

                        madc_data.newTree();




                    }


                    

                    if (counter%100000==0){
                        std::cout << '\r' << "Processing event " << counter<<std::endl;
                    }

                    //rootdata_SCP.initEvent();
                    //rootdata_QDC.initEvent();

                    sig = 0;

                    moduleId = 0;

                    //u32 eventType = (sectionHeader & LF::EventTypeMask) >> LF::EventTypeShift;


                    u32 wordsLeft = sectionSize;

                    while (wordsLeft > 1)
                    {
                        u32 subEventHeader;
                        infile.read((char *)&subEventHeader, sizeof(u32));
                        --wordsLeft;

                        //u32 moduleType = (subEventHeader & LF::ModuleTypeMask) >> LF::ModuleTypeShift;
                        u32 subEventSize = (subEventHeader & LF::SubEventSizeMask) >> LF::SubEventSizeShift;


/*                         std::cout<<std::endl<<"Module type "<<moduleType<<" size "<<subEventSize<<std::endl; */




                        for (u32 i=0; i<subEventSize; ++i)
                        {
                            u32 subEventData;
                            infile.read((char *)&subEventData, sizeof(u32));


                            if (subEventData == 0xffffffffffffffff){



                            }
                            else{

                                /* if((bitExtractor(subEventData, 2, 30) == 1) && (bitExtractor(subEventData, 6, 24) == 0));
                                    std::cout<<"Header"<<std::endl;

                                
                                if((bitExtractor(subEventData, 2, 30) == 0) && (bitExtractor(subEventData, 9, 21) == 32))
                                    std::cout<<"Sub event is "<<std::bitset<32>(subEventData)<<" at ch "<<bitExtractor(subEventData, 5, 16)
                                                <<" value "<<bitExtractor(subEventData, 16, 0)<<std::endl;

                                

                                if(bitExtractor(subEventData, 2, 30) == 2)
                                    std::cout<<"Unknown "<<std::bitset<32>(subEventData)<<std::endl;



                                if(bitExtractor(subEventData, 2, 30) == 3)
                                    std::cout<<"Footer"<<std::endl; */

                                sig = bitExtractor(subEventData, 2, 30);


                                if(sig == 1){//header
                                    moduleId = bitExtractor(subEventData, 8, 16);
                                }
                                if(sig == 0){//data
                                    loc_data.Ch = bitExtractor(subEventData, 5, 16);
                                    loc_data.Mod = moduleId;
                                    loc_data.ChargeLong = bitExtractor(subEventData, 13, 0);
                                    madc_data.addEvent(loc_data);
                                    

/*                                     std::cout<<"Sub event is "<<std::bitset<32>(subEventData)<<" at ch "<<bitExtractor(subEventData, 5, 16)<<" module "<<moduleId
                                                <<" value "<<bitExtractor(subEventData, 16, 0)<<std::endl; */
                                }

                                if(sig == 3){

                                    madc_data.writeEvent(bitExtractor(subEventData, 30, 0));

                                }


                            }


                            /* else{
                                //Check for MDPP-16 SCP/RCP
                                if (moduleType==0) moduleType=4;
                                if ((moduleType==4)||(moduleType==7)){
                                    SCPon = 1;
                                    sig = bitExtractor(subEventData, 4, 28);
                                    if (sig==4){ //header



                                    }
                                    else if (sig==1){//data
                                        //MDPP16 with SCP or RCP firmware
                                        chn = bitExtractor(subEventData, 6, 16);
                                        data = bitExtractor(subEventData, 16, 0);
                                        rootdata_SCP.setADC(chn, data);

                                        if (chn<16){
                                            pu = bitExtractor(subEventData, 1, 23);
                                            ov = bitExtractor(subEventData, 1, 22);
                                            rootdata_SCP.setPileup(chn, pu);
                                            rootdata_SCP.setOverflow(chn, ov);
                                        }


                                    }
                                    else if(sig==2){//extended time stamp
                                        extended = bitExtractor(subEventData, 16, 0);
                                        rootdata_SCP.setExtendedTime(extended);

                                    }
                                    else if(sig>=12){//end of event
                                        time = bitExtractor(subEventData, 30, 0);
                                        rootdata_SCP.setTime(time);

                                    }
                                }
                                //MDPP16 with QDC
                                if (moduleType==8){
                                    QDCon = 1;
                                    sig = bitExtractor(subEventData, 4, 28);
                                    if (sig==4){ //header

                                    }
                                    else if (sig==1){//data
                                        chn = bitExtractor(subEventData, 6, 16);
                                        data = bitExtractor(subEventData, 16, 0);
                                        rootdata_QDC.setADC(chn, data);

                                        if (chn<16){
                                            ov = bitExtractor(subEventData, 1, 22);
                                            rootdata_QDC.setOverflow(chn, ov);
                                        }
                                        //if ((chn==32)||(chn==33)){
                                        //    rootdata_QDC.setTrigger(chn%32, data);
                                        //}
                                    }
                                    if(sig==2){//extended time stamp
                                        extended = bitExtractor(subEventData, 16, 0);
                                        rootdata_QDC.setExtendedTime(extended);
                                    }
                                    else if(sig>=12){//end of event
                                        time = bitExtractor(subEventData, 30, 0);
                                        rootdata_QDC.setTime(time);
                                    }
                                }
                            } */
                        }
                        wordsLeft -= subEventSize;
                    }

                    u32 eventEndMarker;
                    infile.read((char *)&eventEndMarker, sizeof(u32));



                    //rootdata_QDC.writeEvent();
                    //rootdata_SCP.writeEvent();
                    counter++;
                } break;













            case SectionType_Timetick:
                {



                } break;

            case SectionType_End:
                {
                    printf("\nFound Listfile End section\n");
                    continueReading = false;

                    auto currentFilePos = infile.tellg();
                    infile.seekg(0, std::ifstream::end);
                    auto endFilePos = infile.tellg();

                    if (currentFilePos != endFilePos)
                    {
                        std::cout << "Warning: " << (endFilePos - currentFilePos)
                            << " bytes left after Listfile End Section" << std::endl;
                    }



                    break;
                }            





            default:
                {
                    printf("Warning: Unknown section type %u of size %u, skipping...\n",
                           sectionType, sectionSize);
                    infile.seekg(sectionSize * sizeof(u32), std::ifstream::cur);
                } break;



        }











    }

    std::cout << counter << " events total" << std::endl;

    madc_data.writeTree();
    rootfile->Close();






}





void process_file(std::ifstream &infile, TString filename, TString dir)
{

    uint32_t fileVersion = 0;

    // Read the fourCC that's at the start of listfiles from version 1 and up.
    const size_t bytesToRead = 4;
    char fourCC[bytesToRead] = {};

    infile.read(fourCC, bytesToRead);
    static const char * const FourCC = "MVME";

    if (std::strncmp(fourCC, FourCC, bytesToRead) == 0)
    {
        infile.read(reinterpret_cast<char *>(&fileVersion), sizeof(fileVersion));
    }

    std::cout<<"File version "<<fileVersion<<std::endl;

    if(fileVersion != 1){
        std::cout<<"Error prorgram only works with file version 1!"<<std::endl;
        exit(EXIT_FAILURE);
    }

    auto firstSectionOffset = listfile_v1::FirstSectionOffset;

    infile.seekg(firstSectionOffset, std::ifstream::beg);



    convert_file(infile, filename, dir);







}





int main(int argc, char *argv[])
{

    //TString dirandfileName = "/media/gant/Expansion/converttoroot/teodora_2/elissa_073_220913_170058/elissa_073_220913_170058.mvmelst";

    //TString dirandfileName = "/media/gant/Expansion/converttoroot/elissa_073_220913_170058.zip";

    TString dirandfileName = argv[1];

    TString fileName = dirandfileName;

    TString dir = dirandfileName;
    int index = dir.Last('/');
    dir.Remove(index+1,dir.Sizeof());


    index = fileName.Last('/');//remove path




    //check if archive
    TString zipfilename = dirandfileName;
    TString zipcommand = ".! unzip -o " + zipfilename;
    if (dirandfileName.EndsWith(".zip")){

        fileName.Remove(0, index+1);
        fileName.Remove(fileName.Sizeof()-5, fileName.Sizeof());//remove .zip


        std::cout << "----- Unzipping file " << dirandfileName.Data() << " -----" << std::endl;
        if (dir.Sizeof()>1)
            zipcommand.Append(" -d " + dir + fileName);
        std::cout << zipcommand.Data() << std::endl;
        gROOT->ProcessLine(zipcommand.Data());

        //change filename to point to unzipped mvmelst file
        dirandfileName = dir + fileName + "/" + fileName + ".mvmelst";
        /* dirandfileName.Remove(dirandfileName.Sizeof()-4, dirandfileName.Sizeof());
        dirandfileName.Append("mvmelst"); */
        std::cout << "----- Unzip " << dirandfileName.Data() << " complete -----" << std::endl;
    }
    else{
        zipfilename = "";
        fileName.Remove(0, index+1);
        fileName.Remove(fileName.Sizeof()-9, fileName.Sizeof());//remove .mvmelst
    }










    TString mkdirCommand = ".! mkdir ";
    mkdirCommand.Append(dir);
    mkdirCommand.Append("rootfiles");

    gROOT->ProcessLine(mkdirCommand);




    std::ifstream infile(dirandfileName, std::ios::binary);

    if(!infile.is_open()){

        std::cout<<"Error opening file."<<std::endl;
        return 1;

    }

    infile.exceptions(std::ifstream::badbit | std::ifstream::failbit | std::ifstream::eofbit);





    process_file(infile, fileName, dir);



    return 0;

}



