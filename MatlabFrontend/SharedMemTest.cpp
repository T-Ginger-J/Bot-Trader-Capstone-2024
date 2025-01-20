#define UNICODE 
#include <windows.h>
#include <iostream>
#include <string>
#include <mutex>
#include <thread>
#include <chrono>


struct SharedData {
    std::mutex mtx;            
    double input_value;        
    double output_value;       
};

int main() {
  
    HANDLE hMapFile = CreateFileMappingW(
        INVALID_HANDLE_VALUE,   
        NULL,                  
        PAGE_READWRITE,        
        0,                      
        sizeof(SharedData),    
        L"SharedMemoryExample"  
    );

    if (!hMapFile) {
        std::cerr << "Could not create file mapping object: " << GetLastError() << "\n";
        return 1;
    }

    SharedData* sharedData = static_cast<SharedData*>(MapViewOfFile(
        hMapFile,
        FILE_MAP_ALL_ACCESS,   
        0,
        0,
        sizeof(SharedData)
    ));

    if (!sharedData) {
        std::cerr << "Could not map view of file: " << GetLastError() << "\n";
        CloseHandle(hMapFile);
        return 1;
    }

    sharedData->input_value = 0.0;
    sharedData->output_value = 0.0;

    while (true) {
        {
            std::lock_guard<std::mutex> lock(sharedData->mtx); 
            std::cout << "MATLAB Input Value: " << sharedData->input_value << "\n";
            sharedData->output_value = sharedData->input_value * 2; 
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    UnmapViewOfFile(sharedData);
    CloseHandle(hMapFile);
    return 0;
}
