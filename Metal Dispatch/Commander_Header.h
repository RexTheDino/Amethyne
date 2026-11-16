//
//  Commander_Header.h
//  Amethyne
//
//  Created by AnshuDiya on 11/13/26.
//

#pragma once


#include <iostream>

struct metalDispatchCommand;
struct subcommands;



class MetalCommander{
    friend class GPU_lieutenant;
private:
    static void MetalCommanderSetup();
    /*static void instructionExecute(id<MTLCommandBuffer> commdBuffer, std::vector<subcommands>& instructions, int indexInInstructions);*/
    static bool hasBeenSetup;
public:
    class LieutenantPasskey{friend class GPU_lieutenant; LieutenantPasskey() = default;};
    
    static void CommanderExecute(metalDispatchCommand commandToExecute, LieutenantPasskey passkey);
    
};
