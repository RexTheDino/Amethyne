//
//  Commander_Header.h
//  Amethyne
//
//  Created by AnshuDiya on 11/13/26.
//

#pragma once

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include <iostream>

struct metalDispatchCommand;
struct subcommands;

@interface MetalBundleLocator : NSObject
@end

class MetalCommander{
private:
    static void instructionExecute(id<MTLCommandBuffer> commdBuffer, std::vector<subcommands>& instructions, int indexInInstructions);
    
    static void MetalCommanderSetup();
    static void CommanderExecute(metalDispatchCommand commandToExecute);
};
