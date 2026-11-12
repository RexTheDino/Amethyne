//
//  Derived_Differences_Calculation.metal
//  Amethyne
//
//  Created by AnshuDiya on 11/2/26.
//

#include <metal_stdlib>
using namespace metal;

enum class relationships{
    linear = 5,
    quadratic = 6,
    cubic = 7
};

struct relationshipResult{
    uint16_t yTag;
    uint16_t xTag;
    relationships relationship;
    relationshipResult(uint16_t tagY, uint16_t tagX, relationships derRelation): yTag{tagY}, xTag{tagX}, relationship{derRelation}{}
};

//threadgroups of 64 means each threadgroup on a SIMD unit
//half configuration to save on byte loads
kernel void derivedDiffs(device relationshipResult* resultOutBuffer [[buffer(2)]], //to output in [tag1, tag2, relationship]
                         device const float* variableData [[buffer(0)]],
                         device const packed_half2* numberOfPointsAndVarTags [[buffer(1)]],//contains [tag1, tag2]. 0th index is pointsPerVar
                         uint16_t group_id [[threadgroup_position_in_grid]],
                         unsigned short simd_thread_id [[thread_index_in_simdgroup]]){
    
    //const uint16_t threadsPerGrp = 64;
    const float epsilon = 0.0005;
    const float cutoffThreshold = 1.1f;
    half2 varData;
    float pointsPerVar = numberOfPointsAndVarTags[0].x;//MUST BE FLOAT TO MATCH WITH OTHER DATA TYPES
    if(simd_thread_id == 5){varData = numberOfPointsAndVarTags[group_id+1];}//doing plus 1 cuz first element/first 2 indexes contains points per var
    varData = simd_broadcast(varData, 5);
    const uint32_t var1Start = varData.x * pointsPerVar;//tag1*pointsPerVar. this is *varY*
    const uint32_t var2Start = varData.y * pointsPerVar;//tag2*pointsPerVar. this is *varX*
    
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
        {float mean = simd_sum(fDiff)/(64-1);
            mean = (abs(mean) >= abs(epsilon)) ? mean : mean+epsilon;
            float stdDev = sqrt(simd_sum(pow(fDiff-mean, 2.0f))/(varData.x-1));//-1 to account for n-1 differences
            ratio = stdDev/abs(mean);}
        if((ratio < cutoffThreshold)&&(simd_thread_id == 5)){
            resultOutBuffer[group_id] = relationshipResult((uint16_t)varData.y, (uint16_t)pointsPerVar, static_cast<relationships>(5));
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
        float mean = simd_sum(sDiff)/(64-2);//to account for n-2 differences surviving
        mean = (abs(mean) >= abs(epsilon)) ? mean : mean+epsilon;
        float stdDev = sqrt(simd_sum(pow(sDiff-mean, 2.0f))/(varData.x-2));
        ratio = stdDev/abs(mean);
        if((ratio < cutoffThreshold)&&(simd_thread_id == 5)){
            resultOutBuffer[group_id] = relationshipResult((uint16_t)varData.y, (uint16_t)pointsPerVar, static_cast<relationships>(6));
            return;
        }else{
            sDiffFinal1 = sDiff;
        }
    }
    {float sDiffFinal2 = simd_shuffle(sDiffFinal1, clamp((uint16_t)simd_thread_id+1U, 1U, 61U));//n-2 survivors
        float tDiff = sDiffFinal2-sDiffFinal1;
        tDiff = select(tDiff, 0.0f, (simd_thread_id+1) > 61U);//zero stale data threads
        float mean = simd_sum(tDiff)/(64-3);
        mean = (abs(mean) >= abs(epsilon)) ? mean : mean+epsilon;
        float stdDev = sqrt(simd_sum(pow((tDiff-mean),2.0f))/(varData.x-3));
        ratio = stdDev/abs(mean);
        if((ratio < cutoffThreshold)&&(simd_thread_id == 5)){
            resultOutBuffer[group_id] = relationshipResult((uint16_t)varData.y, (uint16_t)pointsPerVar, static_cast<relationships>(7));
        }
    }
}
