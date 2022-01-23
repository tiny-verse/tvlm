#include "NaiveRegisterAllocator.h"

tvlm::Register tvlm::NaiveRegisterAllocator::fillIntRegister(tvlm::Instruction *ins) {
    auto reg = getIntRegister(ins);
    alloc_regs_.resize( counter);
    alloc_regs_[reg.index()] = ins;
    return reg;
}

tvlm::FRegister tvlm::NaiveRegisterAllocator::fillFloatRegister(tvlm::Instruction *ins) {
    return tvlm::FRegister(fcounter++);
}

void tvlm::NaiveRegisterAllocator::clearInt(tvlm::Instruction *ins) {

}

void tvlm::NaiveRegisterAllocator::clearFloat(tvlm::Instruction *ins) {

}

void tvlm::NaiveRegisterAllocator::clearAllIntReg() {

}

void tvlm::NaiveRegisterAllocator::clearAllFloatReg() {

}

void tvlm::NaiveRegisterAllocator::clearAllReg() {
    RegisterAllocator::clearAllReg();
}

void tvlm::NaiveRegisterAllocator::spillAllReg() {

}

void tvlm::NaiveRegisterAllocator::registerPhi(tvlm::Phi *phi) {
    RegisterAllocator::registerPhi(phi);
}

tvlm::Register tvlm::NaiveRegisterAllocator::getIntRegister(tvlm::Instruction *ins) {
    auto f = findInIntRegs(ins);
    if(f.first){
        // register is already assigned to this  register
        return tiny::t86::Reg(f.second);
    }
    auto reg = tiny::t86::Reg(counter++);

    return reg;
}

tvlm::FRegister tvlm::NaiveRegisterAllocator::getFloatRegister(tvlm::Instruction *ins) {
    auto f = findInFloatRegs(ins);
    if(f.first){
        // register is already assigned to this  register
        return tiny::t86::FReg(f.second);
    }
    auto reg = tiny::t86::FReg(fcounter++);

    return reg;
}

std::pair<bool, int> tvlm::NaiveRegisterAllocator::findInIntRegs(tvlm::Instruction * ins) {
    auto f = std::find(alloc_regs_.begin(), alloc_regs_.end(), ins);
    if (f != alloc_regs_.end()) {
        return std::make_pair(true, f - alloc_regs_.begin());
    }else{
        return std::make_pair(false, 197);
    }
}
std::pair<bool, int> tvlm::NaiveRegisterAllocator::findInFloatRegs(tvlm::Instruction * ins) {
    auto f = std::find(alloc_fregs_.begin(), alloc_fregs_.end(), ins);
    if (f != alloc_fregs_.end()) {
        return std::make_pair(true, f - alloc_fregs_.begin());
    }else{
        return std::make_pair(false, 197);
    }
}
