//
//  Variable_Gen_Logic.cpp
//  Amethyne
//
//  Created by AnshuDiya on 11/3/26.
//

#include <stdio.h>
#include "Variable_Gen_Header.hpp"
#include "Var_Storage.hpp"
#include "Initialization_Data_header.hpp"

using namespace std;

bool synthVarGen::twoTermExhaustion::exhaustedTwoTermSynthetics = false;
uint16_t synthVarGen::twoTermExhaustion::opCounter = 1;
uint16_t synthVarGen::twoTermExhaustion::tag1Counter = 1;
uint16_t synthVarGen::twoTermExhaustion::tag2Counter = 1;
uint16_t synthVarGen::twoTermExhaustion::modOpsCounter = 1;

inline synthVarAddon::synthVarAddon(morphOps morphInstruction, modOps modInstruction, uint16_t assignedTag): morph{morphInstruction}, mod{modInstruction}, tag{assignedTag}{}

inline uint16_t synthVarAddon::returnTag(){return tag;}

inline morphOps synthVarAddon::returnMorphOp(){return morph;}

inline modOps synthVarAddon::returnModOp(){return mod;}



permanentSynthAddon::permanentSynthAddon(status derStatus, linkage derLinkage, synthVarAddon derAddon): STATUS{derStatus}, LINKAGE{derLinkage}, addon{derAddon}{}

inline permanentSynthAddon::status permanentSynthAddon::returnSTATUS(){return STATUS;}

inline permanentSynthAddon::linkage permanentSynthAddon::returnLINKAGE(){return LINKAGE;}

inline uint16_t permanentSynthAddon::returnAddonTag(){return addon.returnTag();}

inline morphOps permanentSynthAddon::returnAddonMorphOp(){return addon.returnMorphOp();}

inline modOps permanentSynthAddon::returnAddonModOp(){return addon.returnModOp();}



inline synthVar::synthVar(uint16_t derTag, uint16_t derAddonStartIndex, domain domainOfVar): tag{derTag}, synthAddonStartIndice{derAddonStartIndex}, derDomain{domainOfVar}{}

const uint16_t synthVar::returnTag(){return tag;}

uint16_t synthVar::returnStartIndice(){return synthAddonStartIndice;}

domain synthVar::returnDomain(){return derDomain;}

std::string synthVar::returnName(){return program_name;}



//void synthVarGen::createSynthVars(uint16_t numVarsToCreate, std::span<synthVarAddon> resultVec){
    


//needs to be revised to incoroporate grounded variables. grounded variables kept track of through relations header
/*
void synthVarGen::createAndStoreSynthVarsInTestingGround(){
    testingGround::morphModInstructions.clear();//clears previous synthetics
    int chainLength;
    if(!twoTermExhaustion::exhaustedTwoTermSynthetics){
        chainLength = 2;
    }else{
        chainLength = log(varCreationMetadata::synthVarBatchLimit * varCreationMetadata::number_of_synth_operators)/log(varCreationMetadata::num_variables * varCreationMetadata::number_of_synth_operators);
    }
    for (int g = 0; g < varCreationMetadata::synthVarBatchLimit; g++) {
        for (int j = 0; j < chainLength; j++) {
            if(twoTermExhaustion::exhaustedTwoTermSynthetics){
                uniform_int_distribution<uint16_t> tagDist(0, varCreationMetadata::num_variables-1);
                uint16_t tagAddonModifies = tagDist(Amethyne_rand);
                uniform_int_distribution<int> opDistr(1, varCreationMetadata::number_of_synth_operators);
                morphOps operation = static_cast<morphOps>(opDistr(Amethyne_rand));
                uniform_int_distribution<int> modDistr(1, varCreationMetadata::num_mod_ops);
                modOps modOp = static_cast<modOps>(modDistr(Amethyne_rand));//fix this static cast bull later
                synthVarAddon synthVar(operation, modOp, tagAddonModifies);
                testingGround::morphModInstructions.push_back(synthVar);
            } else {
                //if have not exhausted all two-term combos then continue. else break
                if(twoTermExhaustion::tag2Counter <= varCreationMetadata::num_variables-1){
                    morphOps operation = static_cast<morphOps>(twoTermExhaustion::opCounter);
                    uint16_t derTag = 0;
                    modOps modOp = static_cast<modOps>(twoTermExhaustion::modOpsCounter);//cheating for now but in future we gotta replace two term exhaustion once we no longer use the scratchpad cuz this SUCKS
                    switch (j) {
                        case 0:
                            derTag = twoTermExhaustion::tag1Counter;
                            break;
                        case 1:
                            derTag = twoTermExhaustion::tag2Counter;
                            break;
                    }
                    synthVarAddon synthVar(operation, modOp, derTag);
                    testingGround::morphModInstructions.push_back(synthVar);
                    //cycles through every two term combo possible. but PLEASE fix this trash once you dont need the global scratchpad
                    if(twoTermExhaustion::modOpsCounter <= varCreationMetadata::num_mod_ops){
                        twoTermExhaustion::modOpsCounter++;
                    }else{
                        twoTermExhaustion::modOpsCounter++;
                        
                        if(twoTermExhaustion::opCounter < varCreationMetadata::number_of_synth_operators){
                            twoTermExhaustion::opCounter++;
                        } else {
                            //all this happens when opCounter exceeds limits because thats wehn a certain (tag1, tag2) combo's been exhausted
                            twoTermExhaustion::tag1Counter++;
                            //if exceeded num variables then reset to 0. however if term TWO reaches max_variables we have exhausted all combos!
                            //when tag 1 resets tag 2 needs to increment!
                            if(twoTermExhaustion::tag1Counter > varCreationMetadata::num_variables-1){twoTermExhaustion::tag1Counter = 0; twoTermExhaustion::tag2Counter++;}
                            twoTermExhaustion::opCounter = 1;
                        }
                    }
                } else {
                    //when tag 2 counter reaches max, ALL combos with 2 variables have been EXHAUSTED! therefore, set bool to true
                    twoTermExhaustion::exhaustedTwoTermSynthetics = true;
                }
            }
        }
    }
}
*/
