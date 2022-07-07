#include "ColoringAllocator.h"



namespace tvlm {


    Register ColoringAllocator::getReg(const Instruction *ins) {
        return tvlm::Register(0);
    }

    FRegister ColoringAllocator::getFloatReg(const Instruction *ins) {
        return tvlm::FRegister(0);
    }

    void ColoringAllocator::clearInt(const Instruction *ins) {

    }

    void ColoringAllocator::clearFloat(const Instruction *ins) {

    }

    void ColoringAllocator::spillCallReg() {

    }

    void ColoringAllocator::clearAllReg() {

    }

    void ColoringAllocator::spillAllReg() {

    }

    void ColoringAllocator::prepareReturnValue(size_t size, const Instruction * ret) {

    }

    void ColoringAllocator::makeLocalAllocation(size_t size, const Register &reg, const Instruction * ins) {

    }

    void ColoringAllocator::allocateStructArg(Type *type, const Instruction *ins) {

    }

    void ColoringAllocator::resetAllocSize() {

    }

    void ColoringAllocator::correctStackAlloc(size_t patch) {

    }

    Register ColoringAllocator::getIntRegister(const Instruction *ins) {
        return tvlm::Register(0);
    }

    FRegister ColoringAllocator::getFloatRegister(const Instruction *ins) {
        return tvlm::FRegister(0);
    }

    Register ColoringAllocator::getFreeIntRegister() {
        return tvlm::Register(0);
    }

    FRegister ColoringAllocator::getFreeFloatRegister() {
        return tvlm::FRegister(0);
    }
}