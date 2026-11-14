//
//  Commander.m
//  Amethyne
//
//  Created by AnshuDiya on 11/13/26.
//

#import "Commander_Header.h"
#include "Dispatcher.hpp"


@implementation MetalBundleLocator
@end

id<MTLDevice> device = nil;
id<MTLComputePipelineState> relationEstimation_CompPipeState = nil;
id<MTLComputePipelineState> syntheticsPreprocessing_CompPipeState = nil;
//add bernstein coeffs and ephemerality later...

id<MTLCommandQueue> commdQueue = nil;
id<MTLBuffer> SharedMetadataBuffer = nil;
id<MTLBuffer> OutputBuffer = nil;//private
    id<MTLBuffer> Output_STAGING_Buffer = nil;//shared
id<MTLBuffer> InputBuffer = nil;//private
    id<MTLBuffer> Input_STAGING_Buffer = nil;//shared
id<MTLBuffer> InputMetadataBuffer = nil;//private
    id<MTLBuffer> InputMetadata_STAGING_Buffer = nil;

//index starts at 0
void MetalCommander::instructionExecute(id<MTLCommandBuffer> commdBuffer, std::vector<subcommands>& instructions, int indexInInstructions){
    assert((SharedMetadataBuffer)&&(OutputBuffer)&&(InputBuffer)&&(InputMetadataBuffer));//none of them can be null, shouldve been initialized before this func.
    
    if(indexInInstructions < instructions.size()){
        
        if(std::holds_alternative<dataCopyOp>(instructions[indexInInstructions].typeOfsubcommand)){
            dataCopyOp OP = std::get<dataCopyOp>(instructions[indexInInstructions].typeOfsubcommand);
            id<MTLBlitCommandEncoder> blitter = [commdBuffer blitCommandEncoder];
            NSUInteger sizeOfDat = OP.dataSpanner.spanOfTheDat.size();//size of result in bytes
            
            switch (OP.correspondingBufferType) {
                case bufferTypes::PRIVATE_outputBuffer:{//need to copy results from private output buffer back
                    [blitter copyFromBuffer:OutputBuffer sourceOffset:0 toBuffer:Output_STAGING_Buffer destinationOffset:0 size:sizeOfDat];
                }
                    break;
                    
                case bufferTypes::PRIVATE_inputDataBuffer:{
                    [blitter copyFromBuffer:Input_STAGING_Buffer sourceOffset:0 toBuffer:InputBuffer destinationOffset:0 size:sizeOfDat];
                }
                    break;
                    
                case bufferTypes::PRIVATE_instructionBuffer:{//the metadata buffer
                    [blitter copyFromBuffer:InputMetadata_STAGING_Buffer sourceOffset:0 toBuffer:InputMetadataBuffer destinationOffset:0 size:sizeOfDat];
                }
                    break;
                    
                default://means shared metadata buffer copy instruction was a subcommand which doesnt make sense of enum val is corrupted
                    throw std::runtime_error("Illegal buffer copy operation command!");
                    break;
            }
            [blitter endEncoding];
            
            instructionExecute(commdBuffer, instructions, indexInInstructions+1);//+1 IS CRUCIAL FOR NO INFINI-LOOP!!!
            
        }
        else if(std::holds_alternative<kernelOp>(instructions[indexInInstructions].typeOfsubcommand)){
            id<MTLComputeCommandEncoder> commandEncoder = [commdBuffer computeCommandEncoder];
            [commandEncoder setBuffer:SharedMetadataBuffer offset:0 atIndex:0];//[[buffer(0)]]
            [commandEncoder setBuffer:OutputBuffer offset:0 atIndex:1];//[[buffer(1)]]
            [commandEncoder setBuffer:InputBuffer offset:0 atIndex:2];//[buffer(2)]]
            [commandEncoder setBuffer:InputMetadataBuffer offset:0 atIndex:3];//[[buffer(3)]]
            
            auto kernelInfo = std::get<kernelOp>(instructions[indexInInstructions].typeOfsubcommand);
            switch (kernelInfo.selectedKernel) {
                case kernels::synthetics_preprocessing:
                    [commandEncoder setComputePipelineState:syntheticsPreprocessing_CompPipeState];
                    break;
                case kernels::relationEstimation:
                    [commandEncoder setComputePipelineState:relationEstimation_CompPipeState];
                    break;
                case kernels::ephemerality_calculations:
                    //add implementation later...
                    break;
                case kernels::bernstein_coefficient_calculations:
                    //add implementation later...
                    break;
                    
                default://must be an illegal enum val
                    throw std::runtime_error("Illegal enum val provided for kernel Op to perform in metal!!!");
                    break;
            }
            
            [commandEncoder dispatchThreadgroups:MTLSizeMake((NSUInteger)kernelInfo.numThreadgroups, 1, 1) threadsPerThreadgroup:MTLSizeMake((NSUInteger)kernelInfo.threadsPerThreadgroup, 1, 1)];
            [commandEncoder endEncoding];
            
            instructionExecute(commdBuffer, instructions, indexInInstructions+1);//INDEX+1 IS CRUCIAL TO NO INFINI-LOOP!
            
        }
        
    } else {//runs when instruction index is one beyond the number of instructions a.k.a. all instructions have been encoded
        
            [commdBuffer addCompletedHandler:^(id<MTLCommandBuffer> cmdBfr) {
                assert(cmdBfr.status != MTLCommandBufferStatusError);//shouldnt be error. if error then abort.
                //wake worker threads...
            }];
        [commdBuffer commit];
    }
}


void MetalCommander::MetalCommanderSetup(){
    device = MTLCreateSystemDefaultDevice();
    commdQueue = [device newCommandQueue];
    NSError *error = nil;
    NSBundle *projectBundle =
    [NSBundle bundleForClass:[MetalBundleLocator class]];
    id<MTLLibrary> testLib =
    [device newDefaultLibraryWithBundle:projectBundle
                                  error:&error];
    id<MTLFunction> relationEstimation = [testLib newFunctionWithName:@"derivedDiffs"];
    id<MTLFunction> syntheticsPreprocessor = [testLib newFunctionWithName:@"syntheticsPreprocesser"];

    relationEstimation_CompPipeState = [device newComputePipelineStateWithFunction:relationEstimation error:&error];
    syntheticsPreprocessing_CompPipeState = [device newComputePipelineStateWithFunction:syntheticsPreprocessor error:&error];
    //add bernstein coeffs and ephemerality laer...
    
    if( (!relationEstimation_CompPipeState) || (!syntheticsPreprocessing_CompPipeState) ){
        NSLog(@"Unable to create pipeline state");
    }
    
    std::array<metalBufferInfo, numOfMetalBufferTypes> stagingBuffersInit;//to be filled with initialization data
    
    //50MB each
        stagingBuffersInit[0].sizeOfBuffer = 50000000;
    SharedMetadataBuffer = [device newBufferWithLength:stagingBuffersInit[0].sizeOfBuffer options:MTLResourceStorageModeShared];
        stagingBuffersInit[0].typeOfBuffer = bufferTypes::SHARED_initializerBuffer;
        stagingBuffersInit[0].ptrToMetalBuffer = [SharedMetadataBuffer contents];
    
        stagingBuffersInit[1].sizeOfBuffer = 50000000;
    OutputBuffer = [device newBufferWithLength:50000000 options:MTLResourceStorageModePrivate];
        Output_STAGING_Buffer = [device newBufferWithLength:50000000 options:MTLResourceStorageModeShared];
            stagingBuffersInit[1].typeOfBuffer = bufferTypes::PRIVATE_outputBuffer;
            stagingBuffersInit[1].ptrToMetalBuffer = [Output_STAGING_Buffer contents];//nullptr since not a shared buffer and no pointer to share to write to
    
        stagingBuffersInit[2].sizeOfBuffer = 50000000;
    InputBuffer = [device newBufferWithLength:50000000 options:MTLResourceStorageModePrivate];
        Input_STAGING_Buffer = [device newBufferWithLength:50000000 options:MTLResourceStorageModeShared];
            stagingBuffersInit[2].typeOfBuffer = bufferTypes::PRIVATE_inputDataBuffer;
            stagingBuffersInit[2].ptrToMetalBuffer = [Input_STAGING_Buffer contents];
    
        stagingBuffersInit[3].sizeOfBuffer = 50000000;
    InputMetadataBuffer = [device newBufferWithLength:50000000 options:MTLResourceStorageModePrivate];
        InputMetadata_STAGING_Buffer = [device newBufferWithLength:50000000 options:MTLResourceStorageModeShared];
            stagingBuffersInit[3].typeOfBuffer = bufferTypes::PRIVATE_instructionBuffer;
            stagingBuffersInit[3].ptrToMetalBuffer = [InputMetadata_STAGING_Buffer contents];
    
    if((!SharedMetadataBuffer)||(!OutputBuffer)||(!InputBuffer)||(!InputMetadataBuffer)){
        throw std::runtime_error("Buffers could not be initialized!");
    }
    
    metalBufferStaging::commanderPasskey initializationPasskey;
    metalBufferStaging::initializeStagingBuffers(initializationPasskey, stagingBuffersInit);
}

void MetalCommander::CommanderExecute(metalDispatchCommand commandToExecute){
    id<MTLCommandBuffer> commdBuffer = [commdQueue commandBuffer];
    id<MTLComputeCommandEncoder> cmdEncoder = [commdBuffer computeCommandEncoder];
    
    [cmdEncoder setBuffer:SharedMetadataBuffer offset:0 atIndex:0];//[[buffer(0)]]
    [cmdEncoder setBuffer:OutputBuffer offset:0 atIndex:1];//[[buffer(1)]]
    [cmdEncoder setBuffer:InputBuffer offset:0 atIndex:2];//[buffer(2)]]
    [cmdEncoder setBuffer:InputMetadataBuffer offset:0 atIndex:3];//[[buffer(3)]]
    
    instructionExecute(commdBuffer, commandToExecute.commands, 0);//recursive func that does logic for all the commands
}

