//
//  Synthetics_preprocessing.metal
//  Amethyne
//
//  Created by AnshuDiya on 11/3/26.
//

#include <metal_stdlib>
#include "Metadata.h"
using namespace metal;

//to be constructed from the start of the buffer
enum class morphOperations{
    add = 1,
    subtract = 2,
    multiply = 3
};
enum class modOperations{
    recp = 1,
    square = 2
};

struct operations{
    morphOperations morphOps;
    modOperations modOps;
};

struct MtdtStruct{
    uint16_t num_terms_per_synthetic;//the term count of this batch!
};

struct synthVarAddonCopy{//mirrors structure of synthVarAddon class
    operations ops;
    uint16_t var_tag;
};

float morphOpCalculator(morphOperations morphOp, float valToModify, float previousVal){
    switch(morphOp){
        case morphOperations::add:
            return valToModify + previousVal;
            break;
        case morphOperations::subtract:
            return valToModify-previousVal;
            break;
        case morphOperations::multiply:
            return valToModify*previousVal;
            break;
    }
}

float modOpCalculator(modOperations modOp, float valToModify){
    switch(modOp){
        case modOperations::recp:
            return metal::fast::divide(1.0f, valToModify);
            break;
        case modOperations::square:
            return (valToModify*valToModify);
            break;
    }
}

//if tag contains 0 dont use it obviously and exit
//also set bytes in metadata to format outlined above plz
//  !!!dispatched in threadgroups of 64 where each threadgroup handles one chain of instructions!!!
kernel void syntheticsPreprocesser(device const MtdtStruct* metadata [[buffer(0)]],
                                   device float* data[[buffer(1)]],//private buffer. write results back to this buffer!
                                   device const synthVarAddonCopy* instructions[[buffer(2)]],
                                   uint16_t group_id [[threadgroup_position_in_grid]],
                                   uint16_t simd_id [[thread_index_in_simdgroup]]){//private buffer! synthetic relaitonship processing occurs immediately afterwards
    MtdtStruct mtdt = *metadata;//equivaelnt to metadata[0]
    float calculatedVal;
    float previous_val;
    uint32_t indiceToDereference;
    int pointsPerVarAsMultipleOfSixtyFour = RelationConstructionMetadata::points_per_var/64;
    for(unsigned short h = 0; h < pointsPerVarAsMultipleOfSixtyFour; h++){
    for(unsigned short f = 0; f < mtdt.num_terms_per_synthetic; f++){
        //threadgroup one means 1st group of instructions. group 3 means 3rd group of synthetics.
        synthVarAddonCopy currentInstructions = *(instructions+(mtdt.num_terms_per_synthetic * group_id)+f);//instruction it grabs depends on what threadgroup it was
        //var tag will start from 0 and only be even nums since only even testing tags are provided for synth preprocessing
        indiceToDereference = (currentInstructions.var_tag * pointsPerVarAsMultipleOfSixtyFour * 64)+(64*h)+simd_id;//if at tag 3 & 128pts and simd_id 1, goes to indice 384 which is correct
        calculatedVal = data[indiceToDereference];
        calculatedVal = modOpCalculator(currentInstructions.ops.modOps, calculatedVal);
        if(f != 0){
            calculatedVal = morphOpCalculator(currentInstructions.ops.morphOps, calculatedVal, previous_val);
            previous_val = calculatedVal;
        }else{
            previous_val = calculatedVal;//do not execute morphing calculations for first val!
        }
        //add poitns corresponding to the set of instructions it actually processed
        //(group_id*2)+1 points to an odd slot corresponding to the respective position of the addons it gathered.
        //so if it grabbed the 5th (counting from 0, so in reality its 6th set of addons) set of addons being group_id == 5, and the number of points as a multile of 64 was 3, and it was simd_id 4, then the slot it writes back to would be ((5*2)+1) * (64*3) +4 == 2116
        //which is right. the slot it grabs is 11. odd slots are to be overwritten with synth processing results for subsequent relationship testing. 1, 3, 5, 7, 9, 11-- 11 is the SIXTH slot, which is correct, since counting up from 0.
        data[(((group_id*2)+1) *  64* pointsPerVarAsMultipleOfSixtyFour)+ simd_id + (64*h)] = calculatedVal;
    }
}
    
}


