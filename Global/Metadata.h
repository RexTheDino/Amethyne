//
//  Metadata.h
//  Amethyne
//
//  Created by AnshuDiya on 11/3/26.
//

#pragma once


//TEMPORARY scratchpad to hold data while architecture is being built!!!!
//TEMPORARY scratchpad to hold data while architecture is being built!!!!
//TEMPORARY scratchpad to hold data while architecture is being built!!!!
//TEMPORARY scratchpad to hold data while architecture is being built!!!!
//TEMPORARY scratchpad to hold data while architecture is being built!!!!

//TEMPORARY scratchpad to hold data while architecture is being built!!!!
struct varCreationMetadata{
    //TEMPORARY scratchpad to hold data while architecture is being built!!!!
    
    static const unsigned short synthChainLimit = 5;//controls the max chain length of complex vars
    static constexpr unsigned short synthVarBatchLimit = 1000;//the max num of synth vars created in one go
    static constexpr unsigned short number_of_synth_operators = 3;//add, subtract, multiply
    static constexpr unsigned short num_mod_ops = 2;
    
    //TEMPORARY scratchpad to hold data while architecture is being built!!!!
    static const unsigned short num_variables = 10;//tag is to correspond with variable number!!
    //TEMPORARY scratchpad to hold data while architecture is being built!!!!
};

struct RelationConstructionMetadata{
    //TEMPORARY scratchpad to hold data while architecture is being built!!!!
    static const unsigned short points_per_var = 1024;
    static const unsigned int points_per_var_eqn_construct_calculation_SGELSY = 128;
    //TEMPORARY scratchpad to hold data while architecture is being built!!!!
};
