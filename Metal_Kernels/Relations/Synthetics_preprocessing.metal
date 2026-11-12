//
//  Synthetics_preprocessing.metal
//  Amethyne
//
//  Created by AnshuDiya on 11/3/26.
//

#include <metal_stdlib>
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
    uint16_t pointsPerVar_as_multiple_of_64;//*as a multiple of 64! change derived_differences to also look for num_points and as a multiple of 64*
    uint16_t num_terms_per_synthetic;//the term count of this batch!
    uint16_t write_back_indice;//what starting indice to write back points to
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
//dispatched in threadgroups of 64 where each threadgroup handles one chain of instructions
kernel void syntheticsPreprocesser(device const MtdtStruct* metadata [[buffer(0)]],
                                   device float* data[[buffer(1)]],//private buffer. write results back to this buffer!
                                   device const synthVarAddonCopy* instructions[[buffer(2)]],
                                   uint16_t group_id [[threadgroup_position_in_grid]],
                                   uint16_t simd_id [[thread_index_in_simdgroup]]){//private buffer! synthetic relaitonship processing occurs immediately afterwards
    MtdtStruct mtdt = *metadata;
    float calculatedVal;
    float previous_val;
    uint32_t indiceToDereference;
    for(unsigned short h = 0; h < mtdt.pointsPerVar_as_multiple_of_64; h++){
    for(unsigned short f = 0; f < mtdt.num_terms_per_synthetic; f++){
        synthVarAddonCopy currentInstructions = *(instructions+(mtdt.num_terms_per_synthetic * group_id)+f);//instruction it grabs depends on what threadgroup it was
        indiceToDereference = (currentInstructions.var_tag * mtdt.pointsPerVar_as_multiple_of_64 * 64)+(64*h)+simd_id-1;//if at tag 3 & 128pts and simd_id 1, goes to indice 384 which is correct
        calculatedVal = data[indiceToDereference];
        calculatedVal = modOpCalculator(currentInstructions.ops.modOps, calculatedVal);
        if(f != 0){
            calculatedVal = morphOpCalculator(currentInstructions.ops.morphOps, calculatedVal, previous_val);
            previous_val = calculatedVal;
        }else{
            previous_val = calculatedVal;//do not execute morphing calculations for first val!
        }
    }
        //add poitns corresponding to the set of instructions it actually processed
        data[mtdt.write_back_indice+(group_id*64*mtdt.pointsPerVar_as_multiple_of_64)+simd_id] = calculatedVal;
}
    
}


