//
//  Commander.m
//  Amethyne
//
//  Created by AnshuDiya on 11/13/26.
//

#import "Commander_Header.h"
#include "Dispatcher.hpp"
#import <Metal/Metal.h>
#import <Foundation/Foundation.h>

@interface MetalBundleLocator : NSObject
@end

@implementation MetalBundleLocator
@end

bool MetalCommander::hasBeenSetup = false;//initialized to not being set up

id<MTLDevice> device = nil;
id<MTLComputePipelineState> relationEstimation_CompPipeState = nil;
id<MTLComputePipelineState> syntheticsPreprocessing_CompPipeState = nil;
id<MTLComputePipelineState> bernsteinCalc_CompPipeState = nil;
id<MTLComputePipelineState> findMaxMinBernstenCoeffs_CompPipeState = nil;
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
//this is a global func
static void instructionExecute(id<MTLCommandBuffer> commdBuffer, std::vector<subcommands>& instructions, int indexInInstructions, void(*addCompletedHandlerFunc)()){
    assert((SharedMetadataBuffer)&&(OutputBuffer)&&(InputBuffer)&&(InputMetadataBuffer));//none of them can be null, shouldve been initialized before this func.
    
    if(indexInInstructions < instructions.size()){
        
        if(std::holds_alternative<dataCopyOp>(instructions[indexInInstructions].typeOfsubcommand)){
            dataCopyOp OP = std::get<dataCopyOp>(instructions[indexInInstructions].typeOfsubcommand);
            id<MTLBlitCommandEncoder> blitter = [commdBuffer blitCommandEncoder];
            NSUInteger sizeOfDat = OP.dataSpanner.spanOfTheDat.size();//size of result in bytes
            
            switch (OP.correspondingBufferType) {
                case bufferTypes::PRIVATE_outputBuffer:{//need to copy results from private output buffer back
                    OP.reverseCopy ? [blitter copyFromBuffer:Output_STAGING_Buffer sourceOffset:0 toBuffer:OutputBuffer destinationOffset:0 size:sizeOfDat] : [blitter copyFromBuffer:OutputBuffer sourceOffset:0 toBuffer:Output_STAGING_Buffer destinationOffset:0 size:sizeOfDat];
                }
                    break;
                    
                case bufferTypes::PRIVATE_inputDataBuffer:{
                    OP.reverseCopy ? [blitter copyFromBuffer:InputBuffer sourceOffset:0 toBuffer:Input_STAGING_Buffer destinationOffset:0 size:sizeOfDat] : [blitter copyFromBuffer:Input_STAGING_Buffer sourceOffset:0 toBuffer:InputBuffer destinationOffset:0 size:sizeOfDat];
                }
                    break;
                    
                case bufferTypes::PRIVATE_instructionBuffer:{//the metadata buffer
                    OP.reverseCopy ? [blitter copyFromBuffer:InputMetadataBuffer sourceOffset:0 toBuffer:InputMetadata_STAGING_Buffer destinationOffset:0 size:sizeOfDat] : [blitter copyFromBuffer:InputMetadata_STAGING_Buffer sourceOffset:0 toBuffer:InputMetadataBuffer destinationOffset:0 size:sizeOfDat];
                }
                    break;
                case bufferTypes::CLEAR_PRIVATE_outputBuffer:{
                    [blitter fillBuffer:OutputBuffer range:NSMakeRange(0, OutputBuffer.length) value:0];
                }
                    break;
                
                case bufferTypes::CLEAR_PRIVATE_inputDataBuffer:{
                    [blitter fillBuffer:InputBuffer range:NSMakeRange(0, InputBuffer.length) value:0];
                }
                    break;
                case bufferTypes::CLEAR_PRIVATE_instructionBuffer:{
                    [blitter fillBuffer:InputMetadataBuffer range:NSMakeRange(0, InputMetadataBuffer.length) value:0];
                }
                    break;
                case bufferTypes::CLEAR_SHARED_initializerBuffer:{
                    [blitter fillBuffer:SharedMetadataBuffer range:NSMakeRange(0, SharedMetadataBuffer.length) value:0];
                }
                    break;
                    
                default://means shared metadata buffer copy instruction was a subcommand which doesnt make sense of enum val that is corrupted or is a command to copy to a shared buffer which doesnt make sense sicne it should be handled by copyCommand
                    throw std::runtime_error("Illegal buffer copy operation command!");
                    break;
            }
            [blitter endEncoding];
            
            instructionExecute(commdBuffer, instructions, indexInInstructions+1, addCompletedHandlerFunc);//+1 IS CRUCIAL FOR NO INFINI-LOOP!!!
            
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
                    assert(syntheticsPreprocessing_CompPipeState != nil);
                    [commandEncoder setComputePipelineState:syntheticsPreprocessing_CompPipeState];
                    break;
                case kernels::relationEstimation:
                    assert(relationEstimation_CompPipeState != nil);
                    [commandEncoder setComputePipelineState:relationEstimation_CompPipeState];
                    break;
                case kernels::ephemerality_calculations:
                    //add implementation later...
                    break;
                case kernels::bernstein_coefficient_calculations:
                    [commandEncoder setComputePipelineState:bernsteinCalc_CompPipeState];
                    break;
                case kernels::bernstein_coeffs_max_min_finder:
                    [commandEncoder setComputePipelineState:findMaxMinBernstenCoeffs_CompPipeState];
                    break;
                    
                default://must be an illegal enum val
                    throw std::runtime_error("Illegal enum val provided for kernel Op to perform in metal!!!");
                    break;
            }
            
            [commandEncoder dispatchThreadgroups:MTLSizeMake((NSUInteger)kernelInfo.numThreadgroups, 1, 1) threadsPerThreadgroup:MTLSizeMake((NSUInteger)kernelInfo.threadsPerThreadgroup, 1, 1)];
            [commandEncoder endEncoding];
            
            instructionExecute(commdBuffer, instructions, indexInInstructions+1, addCompletedHandlerFunc);//INDEX+1 IS CRUCIAL TO NO INFINI-LOOP!
            
        }
        
    } else {//runs when instruction index is one beyond the number of instructions a.k.a. all instructions have been encoded
        
            [commdBuffer addCompletedHandler:^(id<MTLCommandBuffer> cmdBfr) {
                if(commdBuffer.status == MTLCommandBufferStatusError){
                    NSLog(@"Command buffer failed! Here's why... %@", commdBuffer.error);
                }
                assert(cmdBfr.status != MTLCommandBufferStatusError);//shouldnt be error. if error then abort.
                //perform requested func
                
                if(addCompletedHandlerFunc != nullptr){
                    (*addCompletedHandlerFunc)();
                }
                //wake worker threads...
            }];
        [commdBuffer commit];
        
        [commdBuffer waitUntilCompleted];//change later when needed, this is for testing purposes
    }
}


void MetalCommander::MetalCommanderSetup(){
    if(hasBeenSetup){return;}
    
    device = MTLCreateSystemDefaultDevice();
    commdQueue = [device newCommandQueue];
    NSError *error = nil;
    
    NSString *libraryPath =
        @"/Users/anshudiya/Library/Developer/Xcode/DerivedData/Amethyne-cpwpooyxjtgjhtfnwcbfozactvrf/Build/Products/Debug/default.metallib";

    NSURL *libraryURL = [NSURL fileURLWithPath:libraryPath];
    
    NSBundle *projectBundle = [NSBundle bundleForClass:[MetalBundleLocator class]];
    NSLog(@"Bundle path: %@", projectBundle.bundlePath);
    
    id<MTLLibrary> testLib = [device newLibraryWithURL:libraryURL error:&error];
    //[device newDefaultLibraryWithBundle:projectBundle error:&error];

        if (!testLib) {
            NSLog(@"Failed to load Metal library: %@", error);
            return;
        }

        NSLog(@"Metal library loaded: %@", testLib);

        NSLog(@"Available functions: %@", testLib.functionNames);
    
    id<MTLFunction> relationEstimation = [testLib newFunctionWithName:@"derivedDiffs"];
    id<MTLFunction> syntheticsPreprocessor = [testLib newFunctionWithName:@"syntheticsPreprocesser"];
    id<MTLFunction> bernsteinCoeffsCalc = [testLib newFunctionWithName:@"Bernstein_Coeffs_Filler"];
    id<MTLFunction> maxMinBernsteinCoeffsFinder = [testLib newFunctionWithName:@"findMaxAndMinCoeffInBuffer"];

    relationEstimation_CompPipeState = [device newComputePipelineStateWithFunction:relationEstimation error:&error];
    syntheticsPreprocessing_CompPipeState = [device newComputePipelineStateWithFunction:syntheticsPreprocessor error:&error];
    bernsteinCalc_CompPipeState = [device newComputePipelineStateWithFunction:bernsteinCoeffsCalc error:&error];
    findMaxMinBernstenCoeffs_CompPipeState = [device newComputePipelineStateWithFunction:maxMinBernsteinCoeffsFinder error:&error];
    //add ephemerality laer...
    
    
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
    
    hasBeenSetup = true;
}

void MetalCommander::CommanderExecute(metalDispatchCommand commandToExecute, LieutenantPasskey passkey, void(*addCompletedHandlerFunc)()){
    if(!hasBeenSetup){MetalCommanderSetup(); hasBeenSetup = true;}//only set up once
    
    MTLCaptureManager *captureManager = [MTLCaptureManager sharedCaptureManager];
    MTLCaptureDescriptor *captureDescriptor = [[MTLCaptureDescriptor alloc] init];
    captureDescriptor.captureObject = commdQueue;
    captureDescriptor.destination = MTLCaptureDestinationGPUTraceDocument;
    
    NSString *tracePath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"compute.gputrace"];
    [[NSFileManager defaultManager] removeItemAtPath:tracePath error:nil]; // Remove old capture if it is present...
    captureDescriptor.outputURL = [NSURL fileURLWithPath:tracePath];
    
    NSError *theError = nil;
    
    if([captureManager startCaptureWithDescriptor:captureDescriptor error:&theError]){
        
    id<MTLCommandBuffer> commdBuffer = [commdQueue commandBuffer];
    //id<MTLComputeCommandEncoder> cmdEncoder = [commdBuffer computeCommandEncoder];
    
    /*[cmdEncoder setBuffer:SharedMetadataBuffer offset:0 atIndex:0];//[[buffer(0)]]
    [cmdEncoder setBuffer:OutputBuffer offset:0 atIndex:1];//[[buffer(1)]]
    [cmdEncoder setBuffer:InputBuffer offset:0 atIndex:2];//[buffer(2)]]
    [cmdEncoder setBuffer:InputMetadataBuffer offset:0 atIndex:3];//[[buffer(3)]]*/
    
        instructionExecute(commdBuffer, commandToExecute.commands, 0, addCompletedHandlerFunc);//recursive func that does logic for all the commands
        
        [captureManager stopCapture];
        NSLog(@"Captured at: %@", tracePath);
    }else{
        NSLog(@"Failed to capture: %@", theError);
    }
    NSLog(@"Finished capture.");
}

