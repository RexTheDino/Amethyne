//
//  Initialization_Data.cpp
//  Amethyne
//
//  Created by AnshuDiya on 11/1/26.
//

#include <stdio.h>
#include <vector>
#include "Initialization_Data_header.hpp"

struct MetalBufferPtrsStruct{
    private:
        std::vector<float*> MetalBufferPtrArray = std::vector<float*>(30, nullptr);//contains enough space for 29 buffers worth of pointers
    public:
        void setBufferPtr(uint16_t buffer, float* ptr);//sets the pointer for the specified buffer
        float* queryBufferPtr(uint16_t buffer);//returns pointer to requested buffer
}MetalBufferPtrs; //stores all the pointers to the specified metal buffers

void MetalBufferPtrsStruct::setBufferPtr(uint16_t buffer, float *ptr){
    MetalBufferPtrArray[buffer] = ptr;
}

float* MetalBufferPtrsStruct::queryBufferPtr(uint16_t buffer){
    float* returnPtr = MetalBufferPtrArray[buffer];
    if(returnPtr == nullptr){throw std::invalid_argument("Pointer to requested buffer has not been set.");}
    return returnPtr;//if ptr exists return it
}

float RelationsMetadata::cutoffThresholdForVariableRelation = 1.10f;
float RelationsMetadata::epsilon = 0.000001f;
