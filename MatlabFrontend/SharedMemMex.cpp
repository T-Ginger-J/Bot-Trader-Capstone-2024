#include "mex.h"         
#include <windows.h>    
#include <mutex>         
#include <iostream>

struct SharedData {
    std::mutex mtx;      
    double input_value;  
    double output_value;
};

static HANDLE hMapFile = nullptr;
static SharedData* sharedData = nullptr;

void initializeSharedMemory() {
    if (!hMapFile) {
        hMapFile = OpenFileMappingW(  
            FILE_MAP_ALL_ACCESS,    
            FALSE,                  
            L"SharedMemoryExample"   
        );

        if (!hMapFile) {
            mexErrMsgIdAndTxt("SharedMemory:OpenFileMappingFailed",
                              "Could not open file mapping.");
        }

        sharedData = static_cast<SharedData*>(MapViewOfFile(
            hMapFile,
            FILE_MAP_ALL_ACCESS, 
            0,
            0,
            sizeof(SharedData)
        ));

        if (!sharedData) {
            CloseHandle(hMapFile);
            mexErrMsgIdAndTxt("SharedMemory:MapViewOfFileFailed",
                              "Could not map view of file.");
        }
    }
}

void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
    if (!sharedData) {
        initializeSharedMemory();
    }

    if (nrhs > 0) {
        double inputValue = mxGetScalar(prhs[0]);


        std::lock_guard<std::mutex> lock(sharedData->mtx);
        sharedData->input_value = inputValue;
    }

    double outputValue = 0.0;
    {
        std::lock_guard<std::mutex> lock(sharedData->mtx);
        outputValue = sharedData->output_value;
    }

    plhs[0] = mxCreateDoubleScalar(outputValue);
}
