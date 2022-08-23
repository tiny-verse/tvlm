#include "SuperNaiveRegisterAllocator.h"

#include "tvlm/tvlm/codeGeneration/FunctionalMacro.h"

namespace tvlm {


    void SuperNaiveRegisterAllocator::visit(Instruction *ins) {


    }

    void SuperNaiveRegisterAllocator::visit(Jump *ins) {
    }

    void SuperNaiveRegisterAllocator::visit(CondJump *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(Return *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(CallStatic *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(Call *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(Copy *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(Extend *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(Truncate *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(BinOp *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(UnOp *ins) {
        switch (ins->resultType()) {
            case ResultType::Integer:
            case ResultType::StructAddress:{
                auto & insReg = getSelectedRegisters(ins);
                for (auto & virtReg: insReg) {

                }
                break;
            }
            case ResultType::Double:

                break;
            case ResultType::Void:
                throw "un op with void type not implemented";
                break;
        }
    }

    void SuperNaiveRegisterAllocator::visit(LoadImm *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(AllocL *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(AllocG *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(ArgAddr *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(PutChar *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(GetChar *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(Load *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(Store *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(Phi *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(ElemAddrOffset *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(ElemAddrIndex *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(Halt *ins) { }

    void SuperNaiveRegisterAllocator::visit(StructAssign *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(BasicBlock *bb) {
        enterNewBB(bb);

        for (auto * ins : getBBsInstructions(bb)) {
            visitChild(ins);
        }
        exitBB();
    }

    void SuperNaiveRegisterAllocator::visit(Function *fce) {
        enterNewFce(fce);
        for (BasicBlock * bb : getFunctionBBs(fce)){
            visitChild(bb);
        }
        exitFce();

    }

    void SuperNaiveRegisterAllocator::visit(Program *p) {
        auto globals = getProgramsGlobals(p);
        enterNewBB(globals);
        for (auto * gl : getBBsInstructions(globals)) {
            visitChild(gl);
        }
        exitBB();

        for(auto & f : getProgramsFunctions(p)){
            visitChild(f.second);
        }

    }

    SuperNaiveRegisterAllocator::SuperNaiveRegisterAllocator(TargetProgram &tp):
            _mtWorkingReg_(RegisterType::INTEGER, -1),
            currentWorkingReg_( &_mtWorkingReg_),
            targetProgram_(tp)
    {
        size_t regSize = tiny::t86::Cpu::Config::instance().registerCnt();
        for(size_t i = 0 ; i < regSize ; i++){
            freeReg_.emplace_back(Register(i));
        }


    }

    SuperNaiveRegisterAllocator::Register
    SuperNaiveRegisterAllocator::getRegister(SuperNaiveRegisterAllocator::VirtualRegister &reg) {
        currentWorkingReg_ = &reg;
        auto it = regMapping_.find(&reg);
        if(it != regMapping_.end()){
            switch (it->second.loc()){
                case Location::Register:
                    return Register(it->second.regIndex());

                    break;
                case Location::Stack:
                case Location::Memory:

                    auto free = getFreeRegister();
                    restore( free, it->second);
                    regMapping_.erase(it);
                    regMapping_.emplace( &reg, LocationEntry(Location::Register, free.index()));
                    return free;
                    break;
            }
        }else{
            //notFound --> Allocate new
            Register newReg = getFreeRegister();
            regMapping_.emplace(&reg, LocationEntry(Location::Register, newReg.index()));
            return newReg;
        }
        throw "error this shouldn't happen 123";
    }

    SuperNaiveRegisterAllocator::Register SuperNaiveRegisterAllocator::getRegToSpill() {
        Register res = regQueue_.front();
        regQueue_.pop();
        return res;
    }

    bool SuperNaiveRegisterAllocator::spill( SuperNaiveRegisterAllocator::VirtualRegister &reg) {
        int stackOffset;
        VirtualRegister & virtualRegister = reg;
        auto accomodategReg = regMapping_.find(&reg);
        if(accomodategReg == regMapping_.end() && accomodategReg->second.loc() != Location::Register){
            return false;
        }
        Register regToSPill = Register(accomodategReg->second.regIndex());
        auto it = spillMapping_.find(&reg);
        if(it != spillMapping_.end()){
            switch (it->second.loc()){
                case Location::Register:{

                    throw "spilling from register to register not implemented";
                    break;
                }
                case Location::Stack:{

                    int stackOffset = it->second.stackOffset();
                    LMBS tiny::t86::MOV(tiny::t86::Mem(tiny::t86::Sp() + stackOffset),
                                        Register(reg.getNumber())) LMBE ; // TODO insert code

                    //update structures

                    regMapping_.erase(&virtualRegister);
                    spillMapping_.emplace(&virtualRegister, LocationEntry(Location::Stack, stackOffset));

                    break;
                }
                case Location::Memory:{

                    int memAddress = it->second.memAddress();
                    tiny::t86::MOV(tiny::t86::Mem(memAddress), Register(reg.getNumber())); // TODO insert code


                    //update structures

                    regMapping_.erase(&virtualRegister);
                    spillMapping_.emplace(&virtualRegister, LocationEntry(Location::Memory, memAddress));
                    throw "spilling to memory not implemented";
                    break;
                }
            }
        }else{
            if (0 /*determine if it has its address == variable, or it is tmp*/){
                //variable
//                    tiny::t86::MOV(tiny::t86::Mem(address), reg); // TODO insert code
            }else{
                //tmp
                //stackOffset = getNewStackOffset() //allocate next tmp local
                tiny::t86::MOV(tiny::t86::Mem(tiny::t86::Sp() + stackOffset), Register(reg.getNumber())); // TODO insert code
            }

        }


        //update structures

        regMapping_.erase(&virtualRegister);
        spillMapping_.emplace(&virtualRegister, LocationEntry(Location::Stack, stackOffset));

        return true;
    }

    void SuperNaiveRegisterAllocator::restore(const SuperNaiveRegisterAllocator::Register &whereTo,
                                              const LocationEntry &from) {
        //TODO implement restore

    }

    SuperNaiveRegisterAllocator::Register SuperNaiveRegisterAllocator::getFreeRegister() {
        Register res = Register(0);
        if(!freeReg_.empty()){
            res = freeReg_.front();
            freeReg_.erase(freeReg_.begin());

//            regQueue_.push(res);//TODO
        }else {
            VirtualRegisterPlaceholder ress = VirtualRegisterPlaceholder (RegisterType::INTEGER, getRegToSpill().index());
            spill( ress);
            res = ress.getNumber();
            regQueue_.push(res);
        }
        return res;
    }

    void SuperNaiveRegisterAllocator::exitFce() {

    }

    void SuperNaiveRegisterAllocator::enterNewFce(Function *bb) {

    }

    void SuperNaiveRegisterAllocator::enterNewBB(BasicBlock *bb) {

    }

    void SuperNaiveRegisterAllocator::exitBB() {

    }

    std::vector<TFInstruction> SuperNaiveRegisterAllocator::getSelectedInstr(const Instruction * ins) const {
        auto it = targetProgram_.selectedFInstrs_.find(ins);
        if(it == targetProgram_.selectedFInstrs_.end()){
            throw "finding instruction which is not compiled";
        }

        return it->second;
    }

    std::vector<VirtualRegisterPlaceholder> &
    SuperNaiveRegisterAllocator::getSelectedRegisters(const Instruction * ins) {
        auto it = targetProgram_.alocatedRegisters_.find(ins);
        if(it == targetProgram_.alocatedRegisters_.end()){
            throw "finding instruction which is not compiled";
        }

        return it->second;
    }


}