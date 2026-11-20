//
//  Dispatcher.cpp
//  Amethyne
//
//  Created by AnshuDiya on 11/13/26.
//

#include "Dispatcher.hpp"
#include "Commander_Header.h"

metalBufferStaging::initializationStatus metalBufferStaging::metalBufferInitializationStatus = metalBufferStaging::initializationStatus::uninitialized;

void metalBufferStaging::initializeStagingBuffers(commanderPasskey derPasskey, std::array<metalBufferInfo, numOfMetalBufferTypes> &bufferMtdt){
    
    for(int u = 0; u < 4; u++){
        if(bufferMtdt[u].ptrToMetalBuffer == nullptr){
            throw std::runtime_error("Encountered nullptr while attempting to initialize staging buffers!");
        }
        std::byte* modifiedPtrToBuffer = static_cast<std::byte*>(bufferMtdt[u].ptrToMetalBuffer);
        switch (bufferMtdt[u].typeOfBuffer) {
            case bufferTypes::SHARED_initializerBuffer:{
                std::span<std::byte> sharedBufferSpan(modifiedPtrToBuffer, bufferMtdt[u].sizeOfBuffer);
                SHARED_initializer_buffer = sharedBufferSpan;
            }
                break;
                
            case bufferTypes::PRIVATE_instructionBuffer:{
                std::span<std::byte> inputMtdtSpan(modifiedPtrToBuffer, bufferMtdt[u].sizeOfBuffer);
                InputMetadataBufferPtr = inputMtdtSpan;
            }
                break;
            
            case bufferTypes::PRIVATE_inputDataBuffer:{
                std::span<std::byte> inputDataSpan(modifiedPtrToBuffer, bufferMtdt[u].sizeOfBuffer);
                InputDataBufferPtr = inputDataSpan;
            }
                break;
                
            case bufferTypes::PRIVATE_outputBuffer:{
                std::span<std::byte> outputDataSpan(modifiedPtrToBuffer, bufferMtdt[u].sizeOfBuffer);
                outputBufferPtr = outputDataSpan;
            }
                break;
                
            default:
                throw std::runtime_error("Encountered illegal buffer type while attempting to initialize buffers!");
                break;
        }
    }
}


class std::span<std::byte>* returnBuffer::returnRequestedBuffer(bufferTypes typeOfBufferToReturn){
    switch (typeOfBufferToReturn) {
        case bufferTypes::SHARED_initializerBuffer:
            return &metalBufferStaging::SHARED_initializer_buffer;
            break;
                   
        case bufferTypes::PRIVATE_instructionBuffer:
            return &metalBufferStaging::InputMetadataBufferPtr;
            break;
        
        case bufferTypes::PRIVATE_inputDataBuffer:
            return &metalBufferStaging::InputDataBufferPtr;
            break;
        
        case bufferTypes::PRIVATE_outputBuffer:
            return &metalBufferStaging::outputBufferPtr;
            break;
            
        case bufferTypes::CLEAR_SHARED_initializerBuffer:
            return &metalBufferStaging::SHARED_initializer_buffer;
            break;
                   
        case bufferTypes::CLEAR_PRIVATE_instructionBuffer:
            return &metalBufferStaging::InputMetadataBufferPtr;
            break;
        
        case bufferTypes::CLEAR_PRIVATE_inputDataBuffer:
            return &metalBufferStaging::InputDataBufferPtr;
            break;
        
        case bufferTypes::CLEAR_PRIVATE_outputBuffer:
            return &metalBufferStaging::outputBufferPtr;
            break;
            
        default:
            throw std::invalid_argument("Illegal buffer return value requested!");
            break;
    }
}


/*template <typename intakeOutputDatClass>
std::span<std::byte> convertClassSpanToByteSpan(std::span<intakeOutputDatClass> givenSpan){
    std::byte* ptrToStart = reinterpret_cast<std::byte*>(givenSpan.begin());
    std::byte* ptrToEnd = reinterpret_cast<std::byte*>(givenSpan.end());
    
    std::span<std::byte> returnSpan(ptrToStart, ptrToEnd);
    return returnSpan;
}*/


dataCopyOp::dataCopyOp(spanOfIntakeOrOutputData theSpanGiven, bufferTypes theTypeOfBuffer, bool toReverseCopyDataOrNot): dataSpanner{theSpanGiven}, correspondingBufferType{theTypeOfBuffer}, reverseCopy(toReverseCopyDataOrNot){}



kernelOp::kernelOp(kernels givenKernel, int numberOfThreadgroupsForThisKernel, int theThreadsToRunPerThreadgroupInThisKernel): selectedKernel{givenKernel}, numThreadgroups{numberOfThreadgroupsForThisKernel}, threadsPerThreadgroup{theThreadsToRunPerThreadgroupInThisKernel}{}

subcommands::subcommands(dataCopyOp appendDataCopyOp): typeOfsubcommand{appendDataCopyOp}{}
subcommands::subcommands(kernelOp appendKernelOp): typeOfsubcommand{appendKernelOp}{}

/*template <typename intakeOrOutputDatClass> void subcommandsHelper::appendDataCopyOp(std::span<intakeOrOutputDatClass> intakeOrOutputClassDatSpan, bufferTypes providedTypeOfBuffer, std::vector<subcommands>& vecToAppendTo){
    
    auto PureByteSpan = convertClassSpanToByteSpan<intakeOrOutputDatClass>(intakeOrOutputClassDatSpan);
    auto newSpanOfIntakeOutputDat = spanOfIntakeOrOutputData(PureByteSpan);
    
    auto newDataCopyOperation = dataCopyOp(newSpanOfIntakeOutputDat, providedTypeOfBuffer);
    vecToAppendTo.push_back(newDataCopyOperation);
}*/

spanOfIntakeOrOutputData::spanOfIntakeOrOutputData(std::span<std::byte> theSpanThatBelongsToThis): spanOfTheDat{theSpanThatBelongsToThis}{}

void subcommandsHelper::appendKernelOp(kernels givenKernel, int numberOfThreadgroupsForThisKernel, int theThreadsToRunPerThreadgroupInThisKernel, std::vector<subcommands>& vecToAppendTo){
    
    auto newKernelOp = kernelOp(givenKernel, numberOfThreadgroupsForThisKernel, theThreadsToRunPerThreadgroupInThisKernel);
    vecToAppendTo.push_back(newKernelOp);
}


metalDispatchCommand::metalDispatchCommand(std::vector<subcommands>& instructions, void (*theFunctionToExecuteWhenDone)()): commands{instructions}, funcToExecuteUponKernelComplete(theFunctionToExecuteWhenDone){}


copyCommand::copyCommand(std::span<std::byte> theSpanOfTheData, bufferTypes theTypeOfBuffer, bool reverseCopyFromInputOrToOutput, void (*theFunctionMeantToExecuteAfterCopy)()): spanOfDataInQuestion{theSpanOfTheData}, typeOfBuffer{theTypeOfBuffer}, reverseCopy{reverseCopyFromInputOrToOutput}, funcToExecuteAfterCopy(theFunctionMeantToExecuteAfterCopy){}



void GPU_lieutenant::executeCopyCommand(copyCommand commandToExecute){
    auto& spanOfDat = commandToExecute.spanOfDataInQuestion;
    std::span<std::byte>* stagingBuffer = returnBuffer::returnRequestedBuffer(commandToExecute.typeOfBuffer);
    //the data to write to or read from staging buffer cannot be larger than staging buffer
    assert(spanOfDat.size() <= stagingBuffer->size());
    switch (commandToExecute.typeOfBuffer) {
        //copy provided data to shared buffer
        case bufferTypes::SHARED_initializerBuffer:
            //copy over requetsed data
            commandToExecute.reverseCopy ? std::copy(stagingBuffer->begin(), stagingBuffer->begin() + spanOfDat.size(), spanOfDat.begin()) : std::copy(spanOfDat.begin(), spanOfDat.end(), stagingBuffer->begin());
            break;
        
        case bufferTypes::PRIVATE_instructionBuffer:
            commandToExecute.reverseCopy ? std::copy(stagingBuffer->begin(), stagingBuffer->begin() + spanOfDat.size(), spanOfDat.begin()) : std::copy(spanOfDat.begin(), spanOfDat.end(), stagingBuffer->begin());
            break;
            
        case bufferTypes::PRIVATE_inputDataBuffer://reverse copy rules if true. i.e. copy from input to the requested vec
            commandToExecute.reverseCopy ? std::copy(stagingBuffer->begin(), stagingBuffer->begin() + spanOfDat.size(), spanOfDat.begin()) : std::copy(spanOfDat.begin(), spanOfDat.end(), stagingBuffer->begin());
            break;
        
        case bufferTypes::PRIVATE_outputBuffer://reverse copy rules if true. i.e. copy from requested vec to the output
            commandToExecute.reverseCopy ? std::copy(spanOfDat.begin(), spanOfDat.end(), stagingBuffer->begin()) : std::copy(stagingBuffer->begin(), stagingBuffer->begin() + spanOfDat.size(), spanOfDat.begin());
            break;

        case bufferTypes::CLEAR_PRIVATE_instructionBuffer:
            std::fill(stagingBuffer->begin(), stagingBuffer->end(), std::byte{0});
            break;
        case bufferTypes::CLEAR_PRIVATE_inputDataBuffer:
            std::fill(stagingBuffer->begin(), stagingBuffer->end(), std::byte{0});
            break;
        case bufferTypes::CLEAR_PRIVATE_outputBuffer:
            std::fill(stagingBuffer->begin(), stagingBuffer->end(), std::byte{0});
            break;
        case bufferTypes::CLEAR_SHARED_initializerBuffer:
            std::fill(stagingBuffer->begin(), stagingBuffer->end(), std::byte{0});
            break;
        case bufferTypes::count:
            throw std::invalid_argument("Buffer count value provided as a type of buffer for a copy command!");
            break;
        default:
            throw std::invalid_argument("Illegal buffer type value!");
    }
    if(commandToExecute.funcToExecuteAfterCopy != nullptr){
        (*commandToExecute.funcToExecuteAfterCopy)();//execute func to execute after copy command, if set
    }
}

void GPU_lieutenant::addMetalCommand(std::vector<subcommands> &instructionsToAdd, void(*funcToExecuteAfterBufferComitt)()){commandQueue::commands.push_back(metalDispatchCommand(instructionsToAdd, funcToExecuteAfterBufferComitt));}

/*template <typename resultVecType> void GPU_lieutenant::addCopyCommand(std::span<resultVecType> spanOfDataInQuestion, bufferTypes typeOfBuffer){
    
    auto pureByteSpan = convertClassSpanToByteSpan<resultVecType>(spanOfDataInQuestion);
    commandQueue::commands.push_back(copyCommand(pureByteSpan, typeOfBuffer));
}*/

void GPU_lieutenant::clearCommands(){commandQueue::commands.clear();}

void GPU_lieutenant::FIRE_AWAY(){
    MetalCommander::MetalCommanderSetup();
    
    for(int y = 0; y < commandQueue::commands.size(); y++){
        if(std::holds_alternative<copyCommand>(commandQueue::commands[y])){
            auto theCopyCommand = std::get<copyCommand>(commandQueue::commands[y]);
            executeCopyCommand(theCopyCommand);
        }else if(std::holds_alternative<metalDispatchCommand>(commandQueue::commands[y])){
            auto theDispatchCommand = std::get<metalDispatchCommand>(commandQueue::commands[y]);
            MetalCommander::LieutenantPasskey passkey;
            //func does not execute if its null, thats a check inside instructionExecute()
            MetalCommander::CommanderExecute(theDispatchCommand, passkey, theDispatchCommand.funcToExecuteUponKernelComplete);
        }else{
            throw std::runtime_error("Command queue does not contain a copy or dispatch command!");
        }
    }
    clearCommands();//clear after running commands
}
