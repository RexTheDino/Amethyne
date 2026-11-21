//
//  Bernstein_Coefficients.metal
//  Amethyne
//
//  Created by AnshuDiya on 11/8/26.
//

#include <metal_stdlib>
using namespace metal;
//KNOWN BELIEVED TO BE BUG-- the kernel needs to check all the bits in the bits provided for the degrees of the variables, but it supposedly stops when it reaches bits that read '00' failing to read the bits beyond that to find the variables whose degree-bits are seperated from others by 2 ' 0's '.

struct bernsteinMetadata{
    uint16_t numTerms;
};

//n >= r
struct nChooseR_values{//combinations of values for n choose r where (0 < n < 4) && (0 <= r < 4). to be filled
    //int ONE_choose_ZERO, ONE_choose_ONE;
    //int TWO_choose_ZERO, TWO_choose_ONE, TWO_choose_TWO;
    //int THREE_choose_ZERO, THREE_choose_ONE, THREE_choose_TWO, THREE_choose_THREE;
    const uint32_t values[12] = {1, 1, 0, 0, 1, 2, 1, 0, 1, 3, 3, 1};//arranged in order respective to the format above except it has padding so each row has 4 elements. needs to be filled still!
};

//first we need to know the dimensions of the matrice. since the max and min lenghts along any specific dimension range from (2,4) a.k.a. 3 possible values for each dimension that actually exists create a for loop that reads bytes from pvt buffer and stops when it reaches 00

//next it reads the degree of the variables for the term its calculating same bit-shifting strategy

//if we support 8 variables that means a max possible dimensions of 8 for the matrice as well as the variable degrees too.
//this corresponds to 4 bytes

struct InfoBytes{
    unsigned char one, two, three, four = 0;//one and two consist of dmensions of matrice, three and four the term degrees.
    float coefficient = 0.0f;
};
struct dimensionsReturn{
    int dimensions[8];
};
struct variableInfo{
    int numVars_inTerm;
    int degrees[8];
};
struct dimensionsAndVarInfo{
    dimensionsReturn matriceDimensions;
    dimensionsReturn coords;
    variableInfo varInfo;
};

struct InfoBytesAndN_Choose_R{
    InfoBytes info;
    nChooseR_values referral;
};

//returns dimensions of matrice
inline dimensionsReturn returnMatriceDimensions(InfoBytes rawBytes){
    dimensionsReturn returnArray;
    unsigned char maskByte = 0b00000011;
    //first dimension is stored in very first two bits. likewise last dimension stored in very last two bits.
    unsigned char dimensionOneThruFourBytes = rawBytes.one;
    unsigned char dimensionFourThruEightBytes = rawBytes.two;
    
    returnArray.dimensions[0] = int(dimensionOneThruFourBytes & maskByte)+1;//dimension PLUS ONE is being done since even at degree 0 there is a dimension length of 1!!!!!
    returnArray.dimensions[1] = int((dimensionOneThruFourBytes >> 2) & maskByte)+1;
    returnArray.dimensions[2] = int((dimensionOneThruFourBytes >> 4) & maskByte)+1;
    returnArray.dimensions[3] = int((dimensionOneThruFourBytes >> 6) & maskByte)+1;
        returnArray.dimensions[4] = int((dimensionFourThruEightBytes) & maskByte)+1;
        returnArray.dimensions[5] = int((dimensionFourThruEightBytes >> 2) & maskByte)+1;
        returnArray.dimensions[6] = int((dimensionFourThruEightBytes >> 4) & maskByte)+1;
        returnArray.dimensions[7] = int((dimensionFourThruEightBytes >> 6) & maskByte)+1;
    return returnArray;
}

inline variableInfo returnVariableInfo(InfoBytes rawBytes){
    variableInfo returnInfo;
    unsigned char maskByte = 0b00000011;
    returnInfo.numVars_inTerm = 0;//the number of variables present in this term
   //for first byte
    for(int v = 0; v < 4; v++){
        int currentVal = int((rawBytes.three >> (v*2)) & maskByte);
        if(currentVal != 0){
            returnInfo.degrees[v] = currentVal;
            returnInfo.numVars_inTerm++;
        }else{
            returnInfo.degrees[v] = 0;//set th e degree of this variable to 0 since it dont exist!!
            returnInfo.numVars_inTerm++;//every term has every variable inside it even if its of degree 0
            continue;//continue past 0 degree term
        }
    }
        for(int g = 0; g < 4; g++){
            int currentVal = int((rawBytes.four >> (g*2)) & maskByte);
            if(currentVal != 0){
                returnInfo.degrees[g+4] = currentVal;
                returnInfo.numVars_inTerm++;
            }else{
                returnInfo.degrees[g] = 0;
                returnInfo.numVars_inTerm++;//every term has every variable inside it even if its of degree 0
                continue;
            }
        }
    return returnInfo;
}

//dimension parameter corresponds to what dimension its in
//the fucn returns the product of all the matrice dimensions up to but not including the specified dimension
inline int matriceFactorReturn(dimensionsReturn derDimensions, int dimension){
    int returnVal = 1;
    for(int u = 0; u < 8; u++){
        if(u >= dimension){break;}//at dimension 7, goes all the way uptil before the last one. at dimension 0, doesnt even run
        returnVal *= derDimensions.dimensions[u];
    }
    return  returnVal;
}

inline dimensionsAndVarInfo returnIndexInDimensions(thread dimensionsAndVarInfo &providedStruct, InfoBytes rawBytes, uint32_t thread_ID){
    dimensionsReturn matriceDimensions = returnMatriceDimensions(rawBytes);
    providedStruct.varInfo = returnVariableInfo(rawBytes);
    int num_vars = providedStruct.varInfo.numVars_inTerm;
    dimensionsReturn returnCoords;
    int residual = thread_ID;
    int currentFactor;
    int truncatedResult;
    //runs for all iterations except h = 0. for h = 0 we do the mod thing.
    //num vars is always 8 anyways so doesnt matter, h effectivvely runs for all the dimensions of the grid
    for(int h = num_vars-1; h > 0; h--){//cycle down from highest dimension to lowest
        currentFactor = matriceFactorReturn(matriceDimensions, h);
        truncatedResult = int(residual/currentFactor);
        residual -= currentFactor * truncatedResult;
        returnCoords.dimensions[h] = truncatedResult;
    }
    returnCoords.dimensions[0] = thread_ID % matriceDimensions.dimensions[0];
    providedStruct.coords = returnCoords;
    providedStruct.matriceDimensions = matriceDimensions;
    
    return providedStruct;
}




//how to find the position in the matrice--thread knows its global thread-id across its dispatch first.t ake a 2x3x4x3 matrice. to find its position in the last dimension, thread divides its current position--lets say its 34--by all the dimensions before the last (2x3x4 == 24). Doing so gives a value between 1 and 2 but this is truncated to get 1. Now to get the dimension right before that, subtract (2x3x4) from the global_id to get 10 and divide that by the dimensions before the number representng the semi-last dimension--(2x3 == 6)--to get 10/6 == 1 [truncated]. Therefore the position in the second-to-last dimension is also 1. Subtract 2x3 again from the previous result to get 4. Now finally divide this byt he remaining dimension to get 4/2 == 2. The position in the second dimension is two. Do global_id % first dimension to get remainder for first dimension which is 0. The result is (0, 2, 1, 1)

//35-- 35/(2x3x4) == 1. 35-24 = 11. 11/(2x3) = 1. 11-(2x3) = 5. 5/(2) = 2. 35%2 = 1. result = (1,2,1,1)--correct!


//fills buffer with bernstein coefficients
//num threads dispatched is equal to the product of all the dimensions of the matrice.
kernel void Bernstein_Coeffs_Filler(device const InfoBytes* inputDat[[buffer(2)]], device const bernsteinMetadata* metadatForKernel[[buffer(0)]],
                                    device float* outBuffer[[buffer(1)]],
                                    uint32_t thread_ID [[thread_position_in_grid]]){
    
    bernsteinMetadata metadat = metadatForKernel[0];//contains the number of terms and therefore the number of infobytes
    for(int y = 0; y < metadat.numTerms; y++){
        
        if(metadat.numTerms > 9999){break;}//probably inifnite loop
        
        InfoBytes rawBytes = inputDat[y];//bytes containing matrice dimensions, term degree, and coefficient value. //the infobyte struct it picks depends on the current counter of the loop
        nChooseR_values reference;
        float coefficient = rawBytes.coefficient;
        
        dimensionsAndVarInfo matriceAndTermInfo;//the .dimensions here contain i,j,k,etc... values
        returnIndexInDimensions(matriceAndTermInfo, rawBytes, thread_ID);//fill with dimensions and degree data
        float result = coefficient;//initialized to the coefficient, to be multiplied by the fractions
        for(int v = 0; v < matriceAndTermInfo.varInfo.numVars_inTerm; v++){//numvars in term is redundant and always constant at 8
            float multiplier;//(i choose p) where i is coord and p is term degree
            float divider;//(n choose p) where n is max degree of entire polynomial and p is term degree
            int termDegree;
            int maxDegree;
            termDegree = matriceAndTermInfo.varInfo.degrees[v];
            
            if(termDegree != 0){
                maxDegree = matriceAndTermInfo.matriceDimensions.dimensions[v]-1;//max degree is dimension size -1
                int dimensionCoord = matriceAndTermInfo.coords.dimensions[v];//coordinate in the matrix itself
                if(dimensionCoord < termDegree){continue;}//continue to next iteration when i < p since i needs to be >= p
                
                //dimensions and maxDegree start from 1 so need to do -1
                multiplier = reference.values[((dimensionCoord-1)*4)+termDegree];//since 4 terms per n_choose_X.
                divider = reference.values[((maxDegree-1)*4)+termDegree];//-1 so it starts from 0
                
                result *= multiplier/divider;//multiply by each fraction
            }else{
                result *= 1.0f;//0 choose 0 is just 1
            }
        }
        if(y == 0){//if calculating for very first term set the value equal to the result. else add result a.k.a. the per term contribution to the current existing value
            outBuffer[thread_ID] = result;
        }else{
            outBuffer[thread_ID] += result;
        }
    }
}


//rewrites the very first element with the maximum value found in all the bernstein coefficients
//numpts[0] == matrice_volume!!!
kernel void findMaxAndMinCoeffInBuffer(device const uint32_t* numPts[[buffer(0)]], device float* outBuffer[[buffer(1)]],
                                 uint32_t simd_id [[thread_index_in_simdgroup]]){
    //int num_points_in_outBuffer = outBuffer[0];
    int num_points_to_search = numPts[0]/64;
    uint32_t remainderPts = numPts[0] % 64;
    
    float maxCoeffFound = 0.0f;
    float minCoeffFound = 0.0f;
    int indexToReference = (simd_id * num_points_to_search);//what indice this thread will start searching from
    for(int d = 0; d < num_points_to_search; d++){
        maxCoeffFound = max(outBuffer[indexToReference+d+1], maxCoeffFound);//dont check [0] cuz it contains volume of matrice
        minCoeffFound = min(outBuffer[indexToReference+d+1], minCoeffFound);//dont check [0] cuz it contains volume of matrice
    }
    if(remainderPts > 0){//if the number of pts output wasnt a clean 64 have each thread compare remaining pts
        indexToReference = 1+(num_points_to_search*63);//start at one plus the index where the threads mustve left off
        maxCoeffFound = max(outBuffer[indexToReference+clamp(simd_id, 0U, remainderPts)], maxCoeffFound);
        minCoeffFound = min(outBuffer[indexToReference+clamp(simd_id, 0U, remainderPts)], minCoeffFound);
    }
    maxCoeffFound = simd_max(maxCoeffFound);//get the max all threads found
    minCoeffFound = simd_min(minCoeffFound);//get min all the threads found
        outBuffer[1] = maxCoeffFound;//fill second element of out buffer with the max coefficient found
        outBuffer[0] = minCoeffFound;//filled in format min,max
}
