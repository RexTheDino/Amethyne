//
//  Metadata.h
//  Amethyne
//
//  Created by AnshuDiya on 11/3/26.
//

#pragma once

#ifndef Metadata_h
#define Metadata_h


#endif /* Metadata_h */

//TEMPORARY scratchpad to hold data while architecture is being built!!!!
//TEMPORARY scratchpad to hold data while architecture is being built!!!!
//TEMPORARY scratchpad to hold data while architecture is being built!!!!
//TEMPORARY scratchpad to hold data while architecture is being built!!!!
//TEMPORARY scratchpad to hold data while architecture is being built!!!!

//TEMPORARY scratchpad to hold data while architecture is being built!!!!
struct varCreationMetadata{
    //TEMPORARY scratchpad to hold data while architecture is being built!!!!
    
    static constexpr unsigned short synthChainLimit = 5;//controls the max chain length of complex vars
    static constexpr unsigned short synthVarBatchLimit = 1000;//the max num of synth vars created in one go
    static constexpr uint16_t number_of_synth_operators = 3;//add, subtract, multiply
    static constexpr uint16_t num_mod_ops = 2;
    
    //TEMPORARY scratchpad to hold data while architecture is being built!!!!
    static const uint16_t num_variables = 10;//tag is to correspond with variable number!!
    //TEMPORARY scratchpad to hold data while architecture is being built!!!!
};

struct RelationConstructionMetadata{
    //TEMPORARY scratchpad to hold data while architecture is being built!!!!
    static const uint16_t points_per_var = 1024;
    //TEMPORARY scratchpad to hold data while architecture is being built!!!!
};
