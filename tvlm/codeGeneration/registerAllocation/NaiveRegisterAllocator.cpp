#include "NaiveRegisterAllocator.h"


namespace tvlm {

    Register NaiveRegisterAllocator::getReg(const ILInstruction *ins) {
        auto reg = getIntRegister(ins);
        alloc_regs_.resize(counter);
        alloc_regs_[reg.index()] = ins;
        return reg;
    }

    FRegister NaiveRegisterAllocator::getFloatReg(const ILInstruction *ins) {
        auto reg = getFloatRegister(ins);
        alloc_fregs_.resize(fcounter);
        alloc_fregs_[reg.index()] = ins;
        return reg;
    }

    void NaiveRegisterAllocator::clearInt(const ILInstruction *ins) {
        auto it = std::find(alloc_regs_.begin(), alloc_regs_.end(), ins);
        if (it != alloc_regs_.end()) {
            alloc_regs_.erase(it);
        }
    }

    void NaiveRegisterAllocator::clearFloat(const ILInstruction *ins) {
        auto it = std::find(alloc_fregs_.begin(), alloc_fregs_.end(), ins);
        if (it != alloc_fregs_.end()) {
            alloc_fregs_.erase(it);
        }
    }


    void NaiveRegisterAllocator::clearAllReg() {
        for (int i = 0; i < tiny::t86::Cpu::Config::instance().registerCnt(); ++i) {
            alloc_regs_[i] = nullptr;
        }
        for (int i = 0; i < tiny::t86::Cpu::Config::instance().floatRegisterCnt(); ++i) {
            alloc_fregs_[i] = nullptr;
        }
    }

    void NaiveRegisterAllocator::spillAllReg() {
        for (int i = reserve_upbound; i < tiny::t86::Cpu::Config::instance().registerCnt(); ++i) {
            spillIntReg(Register(i), alloc_regs_[i]);
        }
        for (int i = reserve_upbound; i < tiny::t86::Cpu::Config::instance().floatRegisterCnt(); ++i) {
            spillFloatReg(tiny::t86::FReg(i), alloc_fregs_[i]);
        }
    }

    void NaiveRegisterAllocator::registerPhi(Phi *phi) {
        RegisterAllocator::registerPhi(phi);
    }

    Register NaiveRegisterAllocator::getIntRegister(const ILInstruction *ins) {
        auto f = findInIntRegs(ins);
        if (f.first) {
            // register is already assigned to this  register
            return Register(f.second);
        }

        auto reg = getFreeIntRegister();
        //restore spilled if not ... nothing
        auto const it = spilled_.find(ins);
        if (it != spilled_.end()) {
            //good guy compiler will obey:

            pb_->add(tiny::t86::MOV{reg, tiny::t86::Mem(tiny::t86::Bp() - it->second)}, ins);
            alloc_regs_[reg.index()] = ins;

            return reg; // restored
        }

        return reg; // empty
    }

    FRegister NaiveRegisterAllocator::getFloatRegister(const ILInstruction *ins) {
        auto f = findInFloatRegs(ins);
        if (f.first) {
            // register is already assigned to this  register
            return tiny::t86::FReg(f.second);
        }

        auto reg = getFreeFloatRegister();
        //restore spilled if not ... nothing
        auto const it = spilled_.find(ins);
        if (it != spilled_.end()) {
            //good guy compiler will obey:

            //TODO
//        pb_->add(tiny::t86::MOV{reg, tiny::t86::Mem(tiny::t86::Bp() - it->second)});
            auto tmp = RegisterAllocator::fillIntRegister();
            pb_->add(tiny::t86::MOV{tmp, tiny::t86::Mem(tiny::t86::Bp() - it->second)}, ins);
            pb_->add(tiny::t86::MOV{reg, tmp}, ins);
            clearIntRegister(tmp);

            alloc_regs_[reg.index()] = ins;

            return reg; // restored
        }

        return reg; // empty
    }

    std::pair<bool, int> NaiveRegisterAllocator::findInIntRegs(const ILInstruction *ins) {
        auto f = std::find(alloc_regs_.begin(), alloc_regs_.end(), ins);
        if (f != alloc_regs_.end()) {
            return std::make_pair(true, f - alloc_regs_.begin());
        } else {
            return std::make_pair(false, 197);
        }
    }

    std::pair<bool, int> NaiveRegisterAllocator::findInFloatRegs(const ILInstruction *ins) {
        auto f = std::find(alloc_fregs_.begin(), alloc_fregs_.end(), ins);
        if (f != alloc_fregs_.end()) {
            return std::make_pair(true, f - alloc_fregs_.begin());
        } else {
            return std::make_pair(false, 197);
        }
    }


    void NaiveRegisterAllocator::clearIntRegister(const Register &reg) {

        RegisterAllocator::clearIntRegister(reg);
    }

    Register NaiveRegisterAllocator::getFreeIntRegister() {
        return Register(counter++);
    }

    FRegister NaiveRegisterAllocator::getFreeFloatRegister() {
        return tiny::t86::FReg(fcounter++);
    }

    void NaiveRegisterAllocator::copyStruct(Register from,const Type *type, Register to,const ILInstruction *ins) {

        auto tmpReg = RegisterAllocator::fillIntRegister();
        const Type::Struct *strct = dynamic_cast<const Type::Struct *>(type);
        for (int i = 0; i < strct->size(); ++i) {
            pb_->add(tiny::t86::MOV(tmpReg, tiny::t86::Mem(from + i)), ins);
            pb_->add(tiny::t86::MOV(tiny::t86::Mem(to + i), tmpReg), ins);
        }

        clearIntRegister(tmpReg);
        tmpReg = -15654;
    }

    void NaiveRegisterAllocator::allocateStructArg(const Type *type, const ILInstruction *ins) {
        functionLocalAllocSize += type->size();
        auto reg = NaiveRegisterAllocator::getReg(ins); // // reg with a structure
        auto regTmp = RegisterAllocator::fillIntRegister(); // Working reg
        pb_->add(tiny::t86::MOV(regTmp, tiny::t86::Bp()), ins);
        pb_->add(tiny::t86::SUB(regTmp, (int64_t) functionLocalAllocSize), ins);


        copyStruct(reg, type, regTmp, ins);

        pb_->add(tiny::t86::PUSH(regTmp), ins);
        clearIntRegister(regTmp);
    }

    void NaiveRegisterAllocator::makeLocalAllocation(size_t size, const Register &reg, const ILInstruction *ins) {
        functionLocalAllocSize += size;
        // already allocated, now just find addr for this allocation
        pb_->add(tiny::t86::MOV(reg, tiny::t86::Bp()), ins);
        pb_->add(tiny::t86::SUB(reg, (int64_t) functionLocalAllocSize), ins);
    }

    void NaiveRegisterAllocator::resetAllocSize() {
        functionLocalAllocSize = 0;
    }

    void NaiveRegisterAllocator::correctStackAlloc(size_t patch) {
        replace(patch, tiny::t86::SUB(tiny::t86::Sp(), (int64_t) functionLocalAllocSize));

    }

    void NaiveRegisterAllocator::spillCallReg() {
        spillAllReg();
    }

    bool NaiveRegisterAllocator::spillIntReg(const Register &reg, const Instruction *ins) {


        //if empty Reg - do nothing
        if (!alloc_regs_[reg.index()]) {
            return false;
        }

        const Instruction *ins_to_spill = alloc_regs_[reg.index()];

        int64_t mem_offset = std::numeric_limits<int64_t>::max();
        auto it = spilled_.find(ins_to_spill);
        if(it == spilled_.end()){
            functionLocalAllocSize+=1;
            mem_offset =  (int64_t)functionLocalAllocSize;
            //pb_.add(t86::SUB(t86::Sp(), 1)); // aggregating allocation - no need anymore
            spilled_.emplace(ins_to_spill, mem_offset);

        }else{
            mem_offset = it->second;
        }


        pb_->add(tiny::t86::MOV(tiny::t86::Mem(tiny::t86::Bp() - mem_offset), reg), ins);

        alloc_regs_[reg.index()] = nullptr;
        return true;
    }

    bool NaiveRegisterAllocator::spillFloatReg(const FRegister &reg,const Instruction *ins) {


        //if empty Reg - do nothing
        if (!alloc_fregs_[reg.index()]) {
            return false;
        }

        const Instruction *ins_to_spill = alloc_fregs_[reg.index()];

        int64_t mem_offset = std::numeric_limits<int64_t>::max();
        auto it = spilled_.find(ins_to_spill);
        if(it == spilled_.end()){
            functionLocalAllocSize+=1;
            mem_offset =  (int64_t)functionLocalAllocSize;
            //pb_.add(t86::SUB(t86::Sp(), 1)); // aggregating allocation - no need anymore
            spilled_.emplace(ins_to_spill, mem_offset);

        }else{
            mem_offset = it->second;
        }


        pb_->add(tiny::t86::MOV(tiny::t86::Mem(tiny::t86::Bp() - mem_offset), reg), ins);

        alloc_fregs_[reg.index()] = nullptr;
        return true;
    }

    void NaiveRegisterAllocator::prepareReturnValue(size_t size,const Instruction *ret) {
        if(size == 0){
            auto regTmp  = NaiveRegisterAllocator::getReg(ret);  // tmpAddress prepared for return Value
            pb_->add(tiny::t86::PUSH(regTmp),ret );
            clearIntRegister(regTmp);
        }else{
            functionLocalAllocSize += size;
            auto regTmp  = NaiveRegisterAllocator::getReg(ret);  // tmpAddress prepared for return Value
            pb_->add(tiny::t86::MOV(regTmp ,tiny::t86::Bp()), ret);
            pb_->add(tiny::t86::SUB(regTmp, (int64_t)functionLocalAllocSize), ret);
            pb_->add(tiny::t86::PUSH(regTmp), ret);
            clearIntRegister(regTmp);
        }
    }

}