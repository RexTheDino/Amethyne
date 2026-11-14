//
//  Derived_Differences_Calculation.metal
//  Amethyne
//
//  Created by AnshuDiya on 11/2/26.
//

#pragma once

#include <metal_stdlib>
using namespace metal;

enum class relationships: uint16_t{
    linear = 5,
    quadratic = 6,
    cubic = 7,
    null = 8
};

struct relationshipResult{
    uint16_t xTag;
    uint16_t yTag;
    relationships relationship;
    relationshipResult(uint16_t tagY, uint16_t tagX, relationships derRelation): yTag{tagY}, xTag{tagX}, relationship{derRelation}{}
};

struct numPtsAndVarTags{
    uint16_t tagX;
    uint16_t tagY;
};

//threadgroups of 64 means each threadgroup on a SIMD unit
//half configuration to save on byte loads
kernel void derivedDiffs(device relationshipResult* resultOutBuffer [[buffer(2)]], //to output in [tagX, tagY, relationship]
                         device const float* variableData [[buffer(0)]],
                         device const numPtsAndVarTags* numberOfPointsAndVarTags [[buffer(1)]],//contains [tagX, tagY]. 0th index is pointsPerVar
                         uint16_t group_id [[threadgroup_position_in_grid]],
                         unsigned short simd_thread_id [[thread_index_in_simdgroup]]){
    
    const float epsilon = 0.0005;
    const float cutoffThreshold = 1.1f;
    numPtsAndVarTags varData;
     uint16_t pointsPerVar = numberOfPointsAndVarTags[0].tagX;
    if(simd_thread_id == 5){varData = numberOfPointsAndVarTags[group_id+1];}//doing plus 1 cuz first element/first 2 indexes contains points per var
    varData.tagX = simd_broadcast(varData.tagX, 5); varData.tagY = simd_broadcast(varData.tagY, 5);
    const uint32_t var1Start = varData.tagX * pointsPerVar;//tagX*pointsPerVar. this is *varY*
    const uint32_t var2Start = varData.tagY * pointsPerVar;//tagY*pointsPerVar. this is *varX*
    
    //fix so it cant use dynamic point numbers through averaging later
    //y is var1, x is var 2
    float y1 = variableData[var1Start+simd_thread_id]; float x1 = variableData[var2Start+simd_thread_id];
    float y2 = simd_shuffle(y1, clamp((uint32_t)simd_thread_id+1U, 1U, 63U)); float x2 = simd_shuffle(x1, clamp((uint16_t)simd_thread_id+1U, 1U, 63U));
    //use these two to calculate the 128pts
    
    float ratio;
    float sDiffFinal1;
    float fDiffFinal1;
    {float fDiff;
        {float y2_y1 = y2 - y1;
            float x2_x1 = x2 - x1;
            fDiff = select(y2_y1/x2_x1, 0.0f, (simd_thread_id+1) > 63U);}
        {float range = (simd_max(fDiff)-simd_min(fDiff));float mean = simd_sum(fDiff)/(64-1);
            range = (abs(range) >= abs(epsilon)) ? range : range + epsilon; mean = (abs(mean) >= abs(epsilon)) ? mean : mean+epsilon;
            float stdDev = sqrt(simd_sum(pow(fDiff-mean, 2.0f))/(pointsPerVar-1));//-1 to account for n-1 differences
            ratio = stdDev/abs(range);} //stdDev/abs(mean);}
        if((ratio < cutoffThreshold)&&(simd_thread_id == 5)){
            resultOutBuffer[group_id] = relationshipResult(varData.tagX, varData.tagY, static_cast<relationships>(5));
            return;
        }else{
            fDiffFinal1 = fDiff;
        }
    }
    
    {float sDiff;
        //cant pull fDiff from last thread in SIMD which didnt have a y2 to use for fDiff and only contains 0
        float fDiffFinal2 = simd_shuffle(fDiffFinal1, clamp((uint32_t)(simd_thread_id+1U), 1U, 62U));
        sDiff = fDiffFinal2-fDiffFinal1;
        //if this thread is one that wouldve brought stale data in, zero the data
        sDiff = select(sDiff, 0.0f, (simd_thread_id+1) > 62U);
        float range = (simd_max(sDiff)-simd_min(sDiff));
        float mean = simd_sum(sDiff)/(64-2);//to account for n-2 differences surviving
        mean = (abs(mean) >= abs(epsilon)) ? mean : mean+epsilon;
        range = (abs(range) >= abs(epsilon)) ? range : range+epsilon;
        float stdDev = sqrt(simd_sum(pow(sDiff-mean, 2.0f))/(pointsPerVar-2));
        ratio = stdDev/abs(range);
        if((ratio < cutoffThreshold)&&(simd_thread_id == 5)){
            resultOutBuffer[group_id] = relationshipResult(varData.tagX, varData.tagY, static_cast<relationships>(6));
            return;
        }else{
            sDiffFinal1 = sDiff;
        }
    }
    {float sDiffFinal2 = simd_shuffle(sDiffFinal1, clamp((uint16_t)simd_thread_id+1U, 1U, 61U));//n-2 survivors
        float tDiff = sDiffFinal2-sDiffFinal1;
        tDiff = select(tDiff, 0.0f, (simd_thread_id+1) > 61U);//zero stale data threads
        float mean = simd_sum(tDiff)/(64-3);
        float range = (simd_max(tDiff)-simd_min(tDiff));
        mean = (abs(mean) >= abs(epsilon)) ? mean : mean+epsilon;
        range = (abs(range) >= abs(epsilon)) ? range : range+epsilon;
        float stdDev = sqrt(simd_sum(pow((tDiff-mean),2.0f))/(pointsPerVar-3));
        ratio = stdDev/abs(range);
        if((ratio < cutoffThreshold)&&(simd_thread_id == 5)){
            resultOutBuffer[group_id] = relationshipResult(varData.tagX, varData.tagY, static_cast<relationships>(7));
        }else{
            resultOutBuffer[group_id] = relationshipResult(varData.tagX, varData.tagY, static_cast<relationships>(8));//null relationship
        }
    }
}
