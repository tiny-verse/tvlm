#include "ColoringAllocator.h"



namespace tvlm {


    Register ColoringAllocator::fillIntRegister(Instruction *ins) {
        return tvlm::Register(0);
    }

    FRegister ColoringAllocator::fillFloatRegister(Instruction *ins) {
        return tvlm::FRegister(0);
    }

    void ColoringAllocator::clearInt(Instruction *ins) {

    }

    void ColoringAllocator::clearFloat(Instruction *ins) {

    }

    void ColoringAllocator::spillCallReg() {

    }

    void ColoringAllocator::clearAllReg() {

    }

    void ColoringAllocator::spillAllReg() {

    }

    void ColoringAllocator::prepareReturnValue(size_t size, Instruction * ret) {

    }

    void ColoringAllocator::makeLocalAllocation(size_t size, const Register &reg, Instruction * ins) {

    }

    void ColoringAllocator::allocateStructArg(Type *type, Instruction *ins) {

    }

    void ColoringAllocator::resetAllocSize() {

    }

    void ColoringAllocator::correctStackAlloc(size_t patch) {

    }

    Register ColoringAllocator::getIntRegister(Instruction *ins) {
        return tvlm::Register(0);
    }

    FRegister ColoringAllocator::getFloatRegister(Instruction *ins) {
        return tvlm::FRegister(0);
    }

    Register ColoringAllocator::getFreeIntRegister() {
        return tvlm::Register(0);
    }

    FRegister ColoringAllocator::getFreeFloatRegister() {
        return tvlm::FRegister(0);
    }
}