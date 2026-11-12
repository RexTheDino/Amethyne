//
//  Ephemerality.metal
//  Amethyne
//
//  Created by AnshuDiya on 11/5/26.
//

#include <metal_stdlib>
using namespace metal;

struct momentum_classification{
    uint16_t tag;//tag of variable
    uint16_t momentum;//to be set to the momentum of the variable
    momentum_classification(uint16_t derTag, uint16_t derMomentum): tag{derTag}, momentum{derMomentum}{}
};
struct var_tag{
    uint16_t tag;//literally just contains tag for cleanliness
};
struct Metadata{
    uint16_t num_points_per_var_as_multiple_of_64;
    float timeStep;//what step in time is being taken between points
    float epsilon = 0.0005;
};

kernel void Ephemerality_Calculation(device const float* data [[buffer(0)]],
                                     device const var_tag* instructions [[buffer(1)]],
                                     device const Metadata* MetadataBffr [[buffer(2)]],
                                     device momentum_classification* results [[buffer(3)]],
                                     uint16_t group_id [[threadgroup_position_in_grid]],
                                     uint16_t simd_id [[thread_index_in_simdgroup]]){
    
    Metadata mtdt = MetadataBffr[0];
    mtdt.timeStep = select(mtdt.epsilon, mtdt.timeStep, mtdt.timeStep > mtdt.epsilon);
    float total = 0.0f;
    float previousVal = 0.0f;
    float averageSlopeVal;
    var_tag der_var = instructions[group_id];
    if(simd_id == 1){
        averageSlopeVal = (data[(der_var.tag-1)*64*mtdt.num_points_per_var_as_multiple_of_64]-data[der_var.tag*64*mtdt.num_points_per_var_as_multiple_of_64])/(mtdt.timeStep*mtdt.num_points_per_var_as_multiple_of_64*64);
    }
    averageSlopeVal = simd_broadcast(averageSlopeVal, 1);
    for(uint16_t b = 0; b < mtdt.num_points_per_var_as_multiple_of_64; b++){
        uint16_t indiceToDereference = (((der_var.tag-1)*64*mtdt.num_points_per_var_as_multiple_of_64) + (64 * b))+simd_id;
        float calculation = data[indiceToDereference];
        calculation -= simd_shuffle(calculation, clamp(simd_id-1U, 0U, 63U));//thread 1 will calculate 0.0f thats okay
        calculation = pow(calculation, 4.0f);
        calculation /= mtdt.timeStep;//get derivative
        calculation -= averageSlopeVal;//subtract derivative from avg slope
        calculation = simd_sum(calculation);
        total += calculation;
    }
    momentum_classification momentum(der_var.tag, round(log(total)));
    if(simd_id == 0){results[clamp(der_var.tag-1U, 0U, UINT_MAX)] = momentum;}
}


