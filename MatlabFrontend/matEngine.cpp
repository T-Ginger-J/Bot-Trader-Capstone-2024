
#include "mex.hpp"
#include "mexAdapter.hpp"
#include <thread>
#include <memory>

using namespace matlab::data;
using namespace matlab::mex;
using namespace matlab::engine;

// using matlab::mex::ArgumentList;
// using matlab::data::ArrayFactory;
// using matlab::data::ArrayType;

class MexFunction : public matlab::mex::Function {
public:    
    //gateway function
    void operator()(ArgumentList outputs, ArgumentList inputs) {
        checkArguments(outputs, inputs);
        // std::thread myThread(&MexFunction::add, inputs[0][0], inputs[1][0]);
        // myThread.detach();
        printFunction(getEngine());
        outputs[0] = factory->createScalar(0);
    }

    void checkArguments(ArgumentList outputs, ArgumentList inputs) {
        // Get pointer to engine
        std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr = getEngine();

        // Check offset argument: First input must be scalar double
        if (inputs[0].getType() != ArrayType::DOUBLE ||
            inputs[0].getNumberOfElements() != 1)
        {
            matlabPtr->feval(u"error",
                0,
                std::vector<Array>({ factory->createScalar("First input must be scalar double") }));
        }

        // Check array argument: Second input must be double array
        if (inputs[1].getType() != ArrayType::DOUBLE)
        {
            matlabPtr->feval(u"error",
                0,
                std::vector<Array>({ factory->createScalar("Input must be double array") }));
        }

        // Check number of outputs
        if (outputs.size() > 1) 
        {
            matlabPtr->feval(u"error",
                0,
                std::vector<Array>({ factory->createScalar("Only one output is returned") }));
        }
    }

    static void printFunction(std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr) {
        matlabPtr->feval(u"fprintf", 0, std::vector<Array>({ factory->createScalar("Hello from C++ thread!\n") }));
    }
    // static double add(double x, double y) {
    //     return x + y;
    // }

private:
    static ArrayFactory *factory;
};

ArrayFactory* MexFunction::factory = new ArrayFactory();