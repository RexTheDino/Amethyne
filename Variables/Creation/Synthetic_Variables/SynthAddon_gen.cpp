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



//passkey only constructable by the status updater of testing ground
numAddonsPerTestSynth synthVarGen::createAndWriteSynthVars(uint16_t numVarsToCreate, std::span<synthVarAddon> resultVec, addonGenPasskey passkey){
        tagAssigner::testTagManager.createTestTags();//it clears existing test tags and creates new ones.
    //decides the number of terms. 80% chance of 2-term, 13% chance of 3 term, and 7% chance of 4-8 terms
    std::discrete_distribution<uint16_t> addonCountDistr({80, 13, 7});
    uint16_t numAddons = addonCountDistr(Amethyne_rand);
    if(numAddons == 2){//if 4+ terms decide how many!
        std::uniform_int_distribution<int> four_to_eight_termDistr(4, 8);
        numAddons = four_to_eight_termDistr(Amethyne_rand);
    }
    for(int b = 0; b < numVarsToCreate; b++){
        for(int j = 0; j < numAddons; j++){
            //the addon is assigned a tag from teh testing tag naming system
                uint16_t tag = tagAssigner::testTagManager.testSynthTagCounterpartOfRealTag(coldStorage::returnGroundedTag());
            std::uniform_int_distribution<uint16_t> morphDistr(1, static_cast<uint16_t>(morphOps::count));
            morphOps morphOp = static_cast<morphOps>(morphDistr(Amethyne_rand));
            std::uniform_int_distribution<uint16_t> modDistr(1, static_cast<uint16_t>(modOps::count));
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
