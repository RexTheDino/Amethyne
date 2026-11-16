//
//  Dispatcher.hpp
//  Amethyne
//
//  Created by AnshuDiya on 11/13/26.
//

#pragma once
#include <span>
#include <vector>


enum class kernels{
    synthetics_preprocessing,
    relationEstimation,
    ephemerality_calculations,
    bernstein_coefficient_calculations
};

//all the other funcs queue up commands to this dispatcher, then a submit command is issued.

enum class bufferTypes{
    PRIVATE_instructionBuffer,
    PRIVATE_inputDataBuffer,
    PRIVATE_outputBuffer,
    SHARED_initializerBuffer,
    CLEAR_PRIVATE_instructionBuffer,//buffers shoudl be cleared in between command queue elements but this is for clearing within the same command.
    CLEAR_PRIVATE_inputDataBuffer,
    CLEAR_PRIVATE_outputBuffer,
    CLEAR_SHARED_initializerBuffer,
    count
};
    constexpr int numOfMetalBufferTypes = static_cast<int>(bufferTypes::count);


struct metalBufferInfo{
    bufferTypes typeOfBuffer;
    int sizeOfBuffer;
    void* ptrToMetalBuffer = nullptr;//to only be initialized to the staging buffers. if its shared buffer initialized to the buffer itself.
};

struct metalBufferStaging{
    enum class initializationStatus{
        initialized,
        uninitialized
    }; static initializationStatus metalBufferInitializationStatus;
    
    //contain pointers to staging grounds (the vectors below) so dispatcher can copy data to and from from these
    static std::span<std::byte> InputMetadataBufferPtr;//all of these are staging grounds the blit copies to and from!
    static std::span<std::byte> InputDataBufferPtr;
    static std::span<std::byte> outputBufferPtr;
        static metalBufferInfo SHARED_initializer_bufferInfo;//but this contains direct ptr to shared buffer!
    
    class commanderPasskey{friend class MetalCommander; commanderPasskey() = default;};
    //initialize the staging buffer vec sizes as well as the shared buffer pointer
    //the array conains the buffer sizes for each buffer
    static void initializeStagingBuffers(commanderPasskey derPasskey, std::array<metalBufferInfo, numOfMetalBufferTypes> &bufferMtdt);
    
    
private:
    static std::span<std::byte> InputMetadataBuffer;//to be initialized at with the begin and end pointer for the metal buffer
    static std::span<std::byte> InputDataBuffer;
    static std::span<std::byte> OutputBuffer;
    //no shared metadata buffer span CUZ POINTER POINTS TO THAT BUFFER!
    
    
};


class returnBuffer{//returns the span of the requested buffer
    friend class GPU_lieutenant;
    friend class MetalCommander;
    static std::span<std::byte>* returnRequestedBuffer(bufferTypes typeOfBufferToReturn);
};


template <typename intakeOutputDatClass>
std::span<std::byte> convertClassSpanToByteSpan(std::span<intakeOutputDatClass> givenSpan){
    std::byte* ptrToStart = reinterpret_cast<std::byte*>(givenSpan.begin());
    std::byte* ptrToEnd = reinterpret_cast<std::byte*>(givenSpan.end());
    
    std::span<std::byte> returnSpan(ptrToStart, ptrToEnd);
    return returnSpan;
}

struct spanOfIntakeOrOutputData{
    std::span<std::byte> spanOfTheDat;//very convenient since also gives size of input data
    spanOfIntakeOrOutputData(std::span<std::byte> theSpanThatBelongsToThis);
};


//DATA COPy OP IS FOR COPYING TO AND FRO FROM STAGING BUFFERS! ADD_COPY_COMMAND IS WHAT ACTUALLY COPIES BACK TO A C++ VEC!
struct dataCopyOp{
    friend struct subcommands;
    friend struct subcommandsHelper;
    spanOfIntakeOrOutputData dataSpanner;
    bufferTypes correspondingBufferType;
private:
        dataCopyOp(spanOfIntakeOrOutputData theSpanGiven, bufferTypes theTypeOfBuffer);
};
struct kernelOp{
    friend struct subcommands;
    friend struct subcommandsHelper;
    kernels selectedKernel;
    int numThreadgroups;
    int threadsPerThreadgroup;//set automatically through the constexpr struct by the constructor
private:
        kernelOp(kernels givenKernel, int numberOfThreadgroupsForThisKernel, int theThreadsToRunPerThreadgroupInThisKernel);
};

struct subcommands{
    std::variant<dataCopyOp, kernelOp> typeOfsubcommand;
    
    subcommands(dataCopyOp appendDataCopyOp);
    subcommands(kernelOp appendKernelOp);
};

class subcommandsHelper{
public:
    //constructor should carry out the above template functions to make everything easy. this constructor is meant to construct a subcommand w// a data Op
    template <typename intakeOrOutputDatClass> static void appendDataCopyOp(std::span<intakeOrOutputDatClass> intakeOrOutputClassDatSpan, bufferTypes providedTypeOfBuffer, std::vector<subcommands>& vecToAppendTo);
    
    static void appendKernelOp(kernels givenKernel, int numberOfThreadgroupsForThisKernel, int theThreadsToRunPerThreadgroupInThisKernel, std::vector<subcommands>& vecToAppendTo);
};


/*template <typename intakeOrOutputDataClass>
subcommands convertIntakeDatPtrToMetalBufferCommand(std::span<intakeOrOutputDataClass> intakeOrOutputClassDatSpan, bufferTypes providedTypeOfBuffer){
    std::byte* ptrToStart = reinterpret_cast<std::byte*>(intakeOrOutputClassDatSpan.begin());
    std::byte* ptrToEnd = reinterpret_cast<std::byte*>(intakeOrOutputClassDatSpan.end());
    
        std::span<std::byte> constructorSpan(ptrToStart, ptrToEnd);
        spanOfIntakeOrOutputData returnSpan(constructorSpan);
    
    subcommands returnCommand(returnSpan, providedTypeOfBuffer);
}*/




//contains all the commands required to carry out a succesful dispatch from start to end
struct metalDispatchCommand{
    std::vector<subcommands> commands;
    metalDispatchCommand(std::vector<subcommands>& instructions);//threadgroups depends on how many kernel is designed to execute with. either way not something this class needs to know, thats the job of the thing calling this function.
};

//THIS IS THE COMMAND THAT EXECUTES A COPY BACK TO THE REQUESTED RESULT VEC!
struct copyCommand{
    friend class GPU_lieutenant;
    
    std::span<std::byte> spanOfDataInQuestion;
    bufferTypes typeOfBuffer;
private:
    copyCommand(std::span<std::byte> theSpanOfTheData, bufferTypes theTypeOfBuffer);//type of buffer is the kind of buffer to copy back from.
};

//the funcs that want to dispatch a metal command create a vector of type subcommands, sequentially add their instructions to it, then send it over to the lieutenant which packages it into a dispatch command object and adds it to teh command queue. further down the line the metal objc++ class queries the dispatch command everytime it finishes up with a command to see if theres another command it should then execute. this way the instructions can be added squentially as they are determined instead of needing to maintain a local master metadata storage while different functions execute.
struct commandQueue{
    friend class GPU_lieutenant;
private:
    static std::vector<std::variant<copyCommand, metalDispatchCommand>> commands;
};

struct threadsPerGroup{
    static constexpr int synthetic_preprocessing = 64;
    static constexpr int relation_estimation = 64;
};

class GPU_lieutenant{
    static void executeCopyCommand(copyCommand commandToExecute);
public:
    //when copying to shared buffer do NOT use dataCopyOp. instead use COPY COMMAND.
    static void addMetalCommand(std::vector<subcommands>& instructionsToAdd);
    
    //THIS is whats to be used for ocpying to staging buffer AND copying to SHARED buffer(s)!
    template <typename resultVecType> static void addCopyCommand(std::span<resultVecType> spanOfDataInQuestion, bufferTypes typeOfBuffer);//typeOfBuffer decides wether to copy to or from
    static void clearCommands();
    static void FIRE_AWAY();
};
