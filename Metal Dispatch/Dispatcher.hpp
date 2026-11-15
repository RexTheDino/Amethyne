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
    static std::span<std::byte*> returnRequestedBuffer(bufferTypes typeOfBufferToReturn);
};

struct spanOfIntakeOrOutputData{
    std::span<std::byte> spanOfTheDat;//very convenient since also gives size of input data
    spanOfIntakeOrOutputData(std::span<std::byte> theSpanThatBelongsToThis);
};

struct dataCopyOp{
    spanOfIntakeOrOutputData dataSpanner;
    bufferTypes correspondingBufferType;
};
struct kernelOp{
    kernels selectedKernel;
    int numThreadgroups;
    int threadsPerThreadgroup;//set automatically through the constexpr struct by the constructor
};

template <typename intakeOutputDatClass>
std::span<std::byte> convertClassSpanToByteSpan(std::span<intakeOutputDatClass> givenSpan){
    std::byte* ptrToStart = reinterpret_cast<std::byte*>(givenSpan.begin());
    std::byte* ptrToEnd = reinterpret_cast<std::byte*>(givenSpan.end());
    
    std::span<std::byte> returnSpan(ptrToStart, ptrToEnd);
    return returnSpan;
}

struct subcommands{
    std::variant<dataCopyOp, kernelOp> typeOfsubcommand;
    subcommands(kernels selectedKernelOp, int numberOfThreadgroups);
    template <typename intakeOrOutputDatClass> subcommands(std::span<intakeOrOutputDatClass> intakeOrOutputClassDatSpan, bufferTypes providedTypeOfBuffer);
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
    int numThreadgroups;
    kernels selectedKernel;//selected kernel is initialized in constructor as the first kernel-type subcommand seen in instructions.
    metalDispatchCommand(std::vector<subcommands> instructions, int numberOfThreadgroups);//threadgroups depends on how many kernel is designed to execute with. either way not something this class needs to know, thats the job of the thing calling this function.
};

struct copyCommand{
    std::span<std::byte> spanOfDataInQuestion;
    bufferTypes typeOfBuffer;
    copyCommand(std::span<std::byte> theSpanOfTheData, bufferTypes theTypeOfBuffer);
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
public:
    static void addMetalCommand(std::vector<subcommands>& instructionsToAdd, int numberOfThreadgroups);
    static void addCopyCommand(std::span<std::byte> spanOfDataInQuestion, bufferTypes typeOfBuffer);//typeOfBuffer decides wether to copy to or from
    static void clearCommands();
    static void FIRE_AWAY();
};
