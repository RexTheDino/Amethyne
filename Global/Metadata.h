//
//  Metadata.h
//  Amethyne
//
//  Created by AnshuDiya on 11/3/26.
//

#pragma once

#ifndef ShaderTypes_h
#define ShaderTypes_h

#ifdef __METAL_VERSION__
    #include <metal_stdlib>
    #define SHARED_CONST constant constexpr
    typedef uint32_t UInt32;
#else
    #include <cstdint>
    #define SHARED_CONST inline constexpr
    typedef uint32_t UInt32;
#endif


//TEMPORARY scratchpad to hold data while architecture is being built!!!!
//TEMPORARY scratchpad to hold data while architecture is being built!!!!
//TEMPORARY scratchpad to hold data while architecture is being built!!!!
//TEMPORARY scratchpad to hold data while architecture is being built!!!!
//TEMPORARY scratchpad to hold data while architecture is being built!!!!

//TEMPORARY scratchpad to hold data while architecture is being built!!!!
struct varCreationMetadata{
    //TEMPORARY scratchpad to hold data while architecture is being built!!!!
    
    static SHARED_CONST unsigned short synthChainLimit = 5;//controls the max chain length of complex vars
    static SHARED_CONST unsigned short synthVarBatchLimit = 30000;//the max num of synth vars created in one go
    static SHARED_CONST unsigned short number_of_synth_operators = 3;//add, subtract, multiply
    static SHARED_CONST unsigned short num_mod_ops = 2;
    
    //TEMPORARY scratchpad to hold data while architecture is being built!!!!
    static SHARED_CONST unsigned short num_variables = 10;//tag is to correspond with variable number!!
    //TEMPORARY scratchpad to hold data while architecture is being built!!!!
};

struct RelationConstructionMetadata{
    //TEMPORARY scratchpad to hold data while architecture is being built!!!!
    //MUST BE A MULTIPLE OF 64!
    static SHARED_CONST unsigned short points_per_var = 128;
    static SHARED_CONST unsigned int points_per_var_eqn_construct_calculation_SGELSY = 128;
    //TEMPORARY scratchpad to hold data while architecture is being built!!!!
};

#endif
