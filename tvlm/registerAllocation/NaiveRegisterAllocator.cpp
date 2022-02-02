#include "NaiveRegisterAllocator.h"

tvlm::Register tvlm::NaiveRegisterAllocator::fillIntRegister(tvlm::Instruction *ins) {
    auto reg = getIntRegister(ins);
    alloc_regs_.resize( counter);
    alloc_regs_[reg.index()] = ins;
    return reg;
}

tvlm::FRegister tvlm::NaiveRegisterAllocator::fillFloatRegister(tvlm::Instruction *ins) {
    auto reg = getFloatRegister(ins);
    alloc_fregs_.resize( fcounter);
    alloc_fregs_[reg.index()] = ins;
    return reg;
}

void tvlm::NaiveRegisterAllocator::clearInt(tvlm::Instruction *ins) {
    auto it = std::find(alloc_regs_.begin(), alloc_regs_.end(), ins);
    if(it != alloc_regs_.end()){
        alloc_regs_.erase(it);
    }
}

void tvlm::NaiveRegisterAllocator::clearFloat(tvlm::Instruction *ins) {
    auto it = std::find(alloc_fregs_.begin(), alloc_fregs_.end(), ins);
    if(it != alloc_fregs_.end()){
        alloc_fregs_.erase(it);
    }
}


void tvlm::NaiveRegisterAllocator::clearAllReg() {
    for (int i = 0; i < tiny::t86::Cpu::Config::instance().registerCnt(); ++i) {
        alloc_regs_[i] = nullptr;
    }    for (int i = 0; i < tiny::t86::Cpu::Config::instance().floatRegisterCnt(); ++i) {
        alloc_fregs_[i] = nullptr;
    }
}

void tvlm::NaiveRegisterAllocator::spillAllReg() {
    for (int i = reserve_upbound; i < tiny::t86::Cpu::Config::instance().registerCnt(); ++i) {
        spillIntReg(tiny::t86::Reg(i));
    }
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

    auto reg = getFreeIntRegister();
    //restore spilled if not ... nothing
    auto const it = spilled_.find(ins);
    if (it != spilled_.end()) {
        //good guy compiler will obey:

        pb_->add(tiny::t86::MOV{reg, tiny::t86::Mem(tiny::t86::Bp() - it->second)});
        alloc_regs_[reg.index()] = ins;

        return reg; // restored
    }

    return reg; // empty
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


void tvlm::NaiveRegisterAllocator::clearTmpIntRegister(const tvlm::Register &reg) {

    RegisterAllocator::clearTmpIntRegister(reg);
}

tvlm::Register tvlm::NaiveRegisterAllocator::getFreeIntRegister() {
    return tiny::t86::Reg(counter++);
}

tvlm::FRegister tvlm::NaiveRegisterAllocator::getFreeFloatRegister() {
    return tiny::t86::FReg(fcounter++);
}

void tvlm::NaiveRegisterAllocator::copyStruct(tvlm::Register from, tvlm::Type *type, tvlm::Register to) {

    auto tmpReg = fillTmpIntRegister();
    Type::Struct * strct =  dynamic_cast<Type::Struct *>(type);
    for (int i = 0; i < strct->size(); ++i) {
        pb_->add(tiny::t86::MOV(tmpReg, tiny::t86::Mem(from + i )));
        pb_->add(tiny::t86::MOV(tiny::t86::Mem(to + i), tmpReg));
    }

    clearTmpIntRegister(tmpReg); tmpReg = -15654;
}

void tvlm::NaiveRegisterAllocator::allocateStructArg(Type * type, Instruction * ins) {
    functionLocalAllocSize += type->size();
    auto reg = fillIntRegister(ins); // // reg with a structure
    auto regTmp  = fillTmpIntRegister(); // Working reg
    pb_->add(tiny::t86::MOV(regTmp ,tiny::t86::Bp()));
    pb_->add(tiny::t86::SUB(regTmp, (int64_t)functionLocalAllocSize));


    copyStruct(reg, type, regTmp);

    pb_->add(tiny::t86::PUSH(regTmp));
    clearTmpIntRegister(regTmp);
}

void tvlm::NaiveRegisterAllocator::makeLocalAllocation(size_t size, const tvlm::Register &reg) {
    functionLocalAllocSize +=size;
    // already allocated, now just find addr for this allocation
    pb_->add(tiny::t86::MOV(reg ,tiny::t86::Bp()));
    pb_->add(tiny::t86::SUB(reg, (int64_t)functionLocalAllocSize));
}

void tvlm::NaiveRegisterAllocator::resetAllocSize() {
    functionLocalAllocSize = 0;
}

void tvlm::NaiveRegisterAllocator::correctStackAlloc(size_t patch) {
    replace(patch, tiny::t86::SUB(tiny::t86::Sp(), (int64_t)functionLocalAllocSize ));

}

void tvlm::NaiveRegisterAllocator::spillCallReg() {
    spillAllReg();
}
