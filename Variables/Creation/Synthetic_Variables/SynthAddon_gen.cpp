//
//  SynthAddon_gen.cpp
//  Amethyne
//
//  Created by AnshuDiya on 11/11/26.
//

#include "SynthAddon_gen.hpp"
#include "Base_types.hpp"
#include "Initialization_Data_header.hpp"
#include "Variable_Gen_Header.hpp"
#include "Transfer_Manager.hpp"
#include "Nucleus.hpp"




synthVarGen::riggedSynthVarFuncReturn synthVarGen::createAndWrite_RIGGED_synthVars(uint16_t numVarsToCreate, std::vector<synthVarAddon> &resultVec, addonGenPasskey passkey){
    
    riggedSynthVarFuncReturn returnRigs;
    
    tagAssigner::testTagManager.createTestTags();//it clears existing test tags and creates new ones.
    
    uint16_t numAddons = 2;//rigged to only be 2 addon

if(resultVec.size() <= ((numVarsToCreate - 1) * numAddons) + (numAddons -1)){resultVec.resize(((numVarsToCreate -1) * numAddons) + numAddons);}

for(int b = 0; b < numVarsToCreate; b++){
    std::vector<uint16_t> tagsToPickFrom = Nucleus.coldStorage.returnRandTags(numAddons);
    assert(tagsToPickFrom.size() == numAddons);
    for(int j = 0; j < numAddons; j++){
        //the addon is assigned a tag from teh testing tag naming system. the tag is converted to the testing tag system before proceeding.
        uint16_t tag = tagAssigner::testTagManager.realTagToTestTag(tagsToPickFrom[j]);//tagAssigner::testTagManager.testSynthTagCounterpartOfRealTag(tagsToPickFrom[j]);
        
        uint16_t morphOpsCount = static_cast<uint16_t>(morphOps::count);
        assert(morphOpsCount > 1);
        
        std::bernoulli_distribution addOrMultiply(0.5);
        bool toAddOrToMultiply = addOrMultiply(Amethyne_rand);
        
        morphOps morphOp;
        if(toAddOrToMultiply){
            morphOp = morphOps::add;//rigged to be only add or multilply fo rnow
        }else{
            morphOp = morphOps::multiply;
        }
        
        synthVarAddon derAddon(morphOp, modOps::none, tag);//create the addon
        resultVec[(b * numAddons)+j] = derAddon;
        
        riggedSynthVar_Info resultStruct; resultStruct.baseTag = tag; resultStruct.theAddonApplied = derAddon;
        returnRigs.riggedAddons.push_back(resultStruct);
    }
}
numAddonsPerTestSynth numberOfAddons;
numberOfAddons.numAddons = numAddons;
currentAddonsPerTestSynth = numAddons;
    returnRigs.numAddons = numberOfAddons;
return returnRigs;
}



//passkey only constructable by the status updater of testing ground
//this funciton has a vector reference directly passed to it because there is no way beforehand to know how many slots are needed cuz random # of addons
numAddonsPerTestSynth synthVarGen::createAndWriteSynthVars(uint16_t numVarsToCreate, std::vector<synthVarAddon>& resultVec, addonGenPasskey passkey){
    
        tagAssigner::testTagManager.createTestTags();//it clears existing test tags and creates new ones.
    //decides the number of terms. 80% chance of 2-term, 13% chance of 3 term, and 7% chance of 4-8 terms
    std::discrete_distribution<uint16_t> addonCountDistr({80, 13, 7});
    //output ranges form [0, 2]
    uint16_t numAddons = addonCountDistr(Amethyne_rand)+2;//if it returns 0 numAddons is 2. return 1, numAddons is 3. return 2, numAddons is 4-8.
    if(numAddons == 4){//if 4+ terms decide how many!
        std::uniform_int_distribution<int> four_to_eight_termDistr(4, 8);
        numAddons = four_to_eight_termDistr(Amethyne_rand);
    }
    
    if(resultVec.size() <= ((numVarsToCreate - 1) * numAddons) + (numAddons -1)){resultVec.resize(((numVarsToCreate -1) * numAddons) + numAddons);}
    
    for(int b = 0; b < numVarsToCreate; b++){
        std::vector<uint16_t> tagsToPickFrom = Nucleus.coldStorage.returnRandTags(numAddons);
        assert(tagsToPickFrom.size() == numAddons);
        for(int j = 0; j < numAddons; j++){
            //the addon is assigned a tag from teh testing tag naming system. the tag is converted to the testing tag system before proceeding.
            uint16_t tag = tagAssigner::testTagManager.realTagToTestTag(tagsToPickFrom[j]);//tagAssigner::testTagManager.testSynthTagCounterpartOfRealTag(tagsToPickFrom[j]);
            
            uint16_t morphOpsCount = static_cast<uint16_t>(morphOps::count);
            uint16_t modOpsCount = static_cast<uint16_t>(modOps::count);
            assert((morphOpsCount > 1)&&(modOpsCount > 1));
            
            std::uniform_int_distribution<uint16_t> morphDistr(1, morphOpsCount-1);//needs to go up to but not including the count val
            morphOps morphOp = static_cast<morphOps>(morphDistr(Amethyne_rand));
            std::uniform_int_distribution<uint16_t> modDistr(1,modOpsCount-1);//needs to go up to but not including the count val
            modOps modOp = static_cast<modOps>(modDistr(Amethyne_rand));
            
            synthVarAddon derAddon(morphOp, modOp, tag);//create the addon
            resultVec[(b * numAddons)+j] = derAddon;
        }
    }
    numAddonsPerTestSynth returnVal;
    returnVal.numAddons = numAddons;
    currentAddonsPerTestSynth = returnVal.numAddons;
    return returnVal;
}
