//
//  Tester_Setup.m
//  Amethyne
//
//  Created by AnshuDiya on 11/2/26.
//

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <AbsMin_Tester_Header.h>

@implementation MetalBundleLocator
@end

id<MTLDevice> device = nil;
id<MTLComputePipelineState> compPipeState = nil;
id<MTLCommandQueue> commdQueue = nil;
id<MTLBuffer> testInBuffer = nil;
id<MTLBuffer> testOutBuffer = nil;

void MetalSetup(){
    device = MTLCreateSystemDefaultDevice();
    commdQueue = [device newCommandQueue];
    NSError *error = nil;
    NSBundle *projectBundle =
        [NSBundle bundleForClass:[MetalBundleLocator class]];
    id<MTLLibrary> testLib =
        [device newDefaultLibraryWithBundle:projectBundle
                                      error:&error];
    id<MTLFunction> absMin = [testLib newFunctionWithName:@"absoluteMin"];
    compPipeState = [device newComputePipelineStateWithFunction:absMin error:&error];
    if(!compPipeState){
        NSLog(@"Unable to create pipeline state");
    }
    testInBuffer = [device newBufferWithLength:4000 options:MTLResourceStorageModeShared];
    testOutBuffer = [device newBufferWithLength:4000 options:MTLResourceStorageModePrivate];
}

void execute(){
    id<MTLCommandBuffer> commdBuffer = [commdQueue commandBuffer];
    id<MTLComputeCommandEncoder> cmdEncoder = [commdBuffer computeCommandEncoder];
        [cmdEncoder setComputePipelineState:compPipeState];
            [cmdEncoder setBuffer:testInBuffer offset:0 atIndex:0];//index 0
            [cmdEncoder setBuffer:testOutBuffer offset:0 atIndex:1];//index 1
    NSLog(@"\nMax threads per thread group: ");
    std::cout << compPipeState.maxTotalThreadsPerThreadgroup;
    NSLog(@"\nExecution Width: ");
    std::cout << compPipeState.threadExecutionWidth;
}
