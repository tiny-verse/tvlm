#include "SuperNaiveRegisterAllocator.h"

#include "tvlm/tvlm/codeGeneration/FunctionalMacro.h"

namespace tvlm {


    void SuperNaiveRegisterAllocator::visit(Instruction *ins) {


    }

    void SuperNaiveRegisterAllocator::visit(Jump *ins) {
    }

    void SuperNaiveRegisterAllocator::visit(CondJump *ins) {

        auto virtRegs = getAllocatedVirtualRegisters(ins);
        setupRegister(&(virtRegs[0]), ins, 0);

    }

    void SuperNaiveRegisterAllocator::visit(Return *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        if(ins->returnType()->registerType() == ResultType::StructAddress){

        } else if (ins->returnType()->registerType() == ResultType::Double){

        }else if (ins->returnType()->registerType() == ResultType::Integer){
            setupRegister(&(virtRegs[0]), ins, 0);

        }else{

        }
//execute calling conventions before return


    }

    void SuperNaiveRegisterAllocator::visit(CallStatic *ins) {

//    auto ret = pb_.currentLabel();
//        //spill everything
//        regAllocator->spillAllReg();

//        //args /*-> prepare values


////        //prepare return Value //*-> preparation in RA -- memory and register in RA
//        ins->f()->getType()->registerType() == ResultType::StructAddress ?
//            prepareReturnValue(ins->f()->getType()->size(), ins):
//            prepareReturnValue(0, ins);

//        regAllocator->spillCallReg(); !!! TODO need
// execute calling conventions beforeCall


//        //call

//// move/collect result
//        if(ins->resultType() == ResultType::Double){
//            add(tiny::t86::MOV(getFReg(ins), tiny::t86::FReg(0)), ins);
//        }else if (ins->resultType() == ResultType::Integer){
//            add(tiny::t86::MOV(getReg(ins), tiny::t86::Reg(0)), ins);
//        } else if(ins->resultType() == ResultType::Void){
//
//        }

////      CountArgSize;
//        int argSize  = 0;
//        for (auto & a :ins->args()) {
//            if(a.second->registerType() == ResultType:: Double){
//                argSize +=1;
//            }else{
//                argSize ++;
//            }
//        }
//
//        add(tiny::t86::ADD(tiny::t86::Sp(), argSize), ins);



//        program_.registerCall(ins, callLabel, ins->f()->name());
//        lastIns_ = ret; //return ret;
    }

    void SuperNaiveRegisterAllocator::visit(Call *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(Copy *ins) {

        auto virtRegs = getAllocatedVirtualRegisters(ins);
        switch (ins->resultType()) {
            case ResultType::Integer: {
                setupRegister(&(virtRegs[0]), ins, 0);
                setupRegister(&(virtRegs[1]), ins->src(), 1);
                break;
            }
            case ResultType::Double: {

                setupFRegister(&(virtRegs[0]), ins, 0);
                setupFRegister(&(virtRegs[1]), ins->src(), 1);
                break;
            }
            case ResultType::Void:
                throw "copy on Void not implemented";
                break;
            case ResultType::StructAddress:
                throw "copy on StructAddress not implemented";
                break;
        }
    }

    void SuperNaiveRegisterAllocator::visit(Extend *ins) {

        auto virtRegs = getAllocatedVirtualRegisters(ins);
        setupFRegister(&(virtRegs[0]), ins, 0);
        setupRegister(&(virtRegs[1]), ins->src(), 1);
    }

    void SuperNaiveRegisterAllocator::visit(Truncate *ins) {

        auto virtRegs = getAllocatedVirtualRegisters(ins);
        setupRegister(&(virtRegs[0]), ins, 0);
        setupFRegister(&(virtRegs[1]), ins->src(), 1);
    }

    void SuperNaiveRegisterAllocator::visit(BinOp *ins) {

        auto virtRegs = getAllocatedVirtualRegisters(ins);
        switch (ins->resultType()) {
            case ResultType::StructAddress:
            case ResultType::Integer: {
                setupRegister(&(virtRegs[0]), ins->lhs(), 0);
                setupRegister(&(virtRegs[1]), ins->rhs(), 1);
                break;
                }

            case ResultType::Double: {
                switch (ins->opType()) {
                    case BinOpType::ADD:
                    case BinOpType::SUB:
                    case BinOpType::MOD:
                    case BinOpType::MUL:
                    case BinOpType::DIV:
                    case BinOpType::AND:
                    case BinOpType::OR:
                    case BinOpType::XOR:
                    case BinOpType::LSH:
                    case BinOpType::RSH:
                        setupFRegister(&(virtRegs[0]), ins->lhs(), 0);
                        setupFRegister(&(virtRegs[1]), ins->rhs(), 1);
                        replaceInRegisterDescriptor(ins->lhs(), ins);
                        break;

                    case BinOpType::NEQ:
                    case BinOpType::EQ:
                    case BinOpType::LTE:
                    case BinOpType::LT:
                    case BinOpType::GT:
                    case BinOpType::GTE:
                        setupFRegister(&(virtRegs[0]), ins->lhs(), 0);
                        setupFRegister(&(virtRegs[1]), ins->rhs(), 1);
                        setupRegister(&(virtRegs[2]), ins, 2);
                        break;
                }
            }
            case ResultType::Void:
                throw "not implemented";
                break;
        }
    }

    void SuperNaiveRegisterAllocator::visit(UnOp *ins) {

        auto virtRegs = getAllocatedVirtualRegisters(ins);
        switch (ins->resultType()) {
            case ResultType::Integer:
            case ResultType::StructAddress:{
                switch (ins->opType()) {
                    case UnOpType::UNSUB:{
                        setupRegister(&(virtRegs[0]), ins->operand(), 0);
                        setupRegister(&(virtRegs[1]), ins, 1);
                        break;
                    }
                    case UnOpType::INC:
                    case UnOpType::NOT:
                    case UnOpType::DEC:
                        setupRegister(&(virtRegs[0]), ins->operand(), 0);
                        replaceInRegisterDescriptor(ins->operand(), ins);

                        break;
                }
            }
            case ResultType::Double:
                setupFRegister(&(virtRegs[0]), ins, 0);

                break;
            case ResultType::Void:
                throw "un op with void type not implemented";
                break;
        }
    }

    void SuperNaiveRegisterAllocator::visit(LoadImm *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);

        switch (ins->resultType()) {
            case ResultType::Integer:{

                        setupRegister(&(virtRegs[0]), ins, 0);
//                auto reg = getReg(ins, ins);
                break;
            }
            case ResultType::Double:{
                setupFRegister(&(virtRegs[0]), ins, 0);

//                auto freg = getFReg(ins, ins);
//TODO
                break;
            }
            case ResultType::StructAddress:
                throw "ERROR cant load StructAddress as value";
                break;
            case ResultType::Void:
                throw "ERROR cant load void as value";
                break;
        }

    }

    void SuperNaiveRegisterAllocator::visit(AllocL *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        setupRegister(&(virtRegs[0]), ins, 0);

    }

    void SuperNaiveRegisterAllocator::visit(AllocG *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(ArgAddr *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        setupRegister(&(virtRegs[0]), ins, 0);

    }

    void SuperNaiveRegisterAllocator::visit(PutChar *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        setupRegister(&(virtRegs[0]), ins->src(), 0);

    }

    void SuperNaiveRegisterAllocator::visit(GetChar *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        setupRegister(&(virtRegs[0]), ins, 0);

    }

    void SuperNaiveRegisterAllocator::visit(Load *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);

        if(ins->type()->registerType() == ResultType::Double){
            setupFRegister(&(virtRegs[0]), ins, 0);
            setupRegister(&(virtRegs[1]), ins->address(), 1);
            return;
        }else if (ins->type()->registerType() == ResultType::Integer){

            auto it = targetProgram_.globalFind(ins->address());
            if(it != targetProgram_.globalEnd()) {
                setupRegister(&(virtRegs[0]), ins, 0);
                return;
            }
//            if(dynamic_cast<Type::Array *>(ins->type())){
//                setupRegister(&(virtRegs[0]), ins, 0);
//                setupRegister(&(virtRegs[1]), ins->address(), 1);
//            }else{
                setupRegister(&(virtRegs[0]), ins, 0);
                setupRegister(&(virtRegs[1]), ins->address(), 1);
//            }
            return;
        }else if (ins->type()->registerType() == ResultType::StructAddress){
            replaceInRegisterDescriptor(ins->address(), ins);
            return;
        }
        throw "ERROR[RA] failed load";
    }

    void SuperNaiveRegisterAllocator::visit(Store *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        setupRegister(&(virtRegs[0]), ins->address(), 0);

        switch (ins->value()->resultType()) {
            case ResultType::Double:{
                setupFRegister(&(virtRegs[1]), ins->value(), 1);
                return;
            }
            case ResultType::StructAddress:
            case ResultType::Integer:{
                setupRegister(&(virtRegs[1]), ins->value(), 1);
                return;

            }
            case ResultType::Void:{
                throw "ERROR[IS] Store: failed to store void value";
            }
        }
        throw "ERROR[IS] Store: failed to resolve value type";
    }

    void SuperNaiveRegisterAllocator::visit(Phi *ins) {
        //TODO
    }

    void SuperNaiveRegisterAllocator::visit(ElemAddrOffset *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        setupRegister(&(virtRegs[0]), ins, 0);
        setupRegister(&(virtRegs[1]), ins->base(), 1);
        setupRegister(&(virtRegs[2]), ins->offset(), 2);

    }

    void SuperNaiveRegisterAllocator::visit(ElemAddrIndex *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        setupRegister(&(virtRegs[0]), ins, 0);
        setupRegister(&(virtRegs[1]), ins->base(), 1);
        setupRegister(&(virtRegs[2]), ins->index(), 2);

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

                    stackOffset = it->second.stackOffset();
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

            regQueue_.push(res);//TODO
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

    std::list<TFInstruction> SuperNaiveRegisterAllocator::getSelectedInstr(const Instruction * ins) const {
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

    void SuperNaiveRegisterAllocator::setupRegister(VirtualRegisterPlaceholder *reg, const Instruction *ins, size_t pos) {
        auto location = addressDescriptor_.find(ins);
        if(location != addressDescriptor_.end()){
            switch(location->second.loc()){
                case Location::Register:
                    reg->setNumber(location->second.regIndex());
                    break;
                case Location::Memory:{
                    Register freeReg = getReg(ins);
                    targetProgram_.addF_insert(LMBS tiny::t86::MOV( freeReg, tiny::t86::Mem(location->second.memAddress())) LMBE, ins, pos++);
                    break;
                }
                case Location::Stack:{

                    Register freeReg = getReg(ins);


                    targetProgram_.addF_insert(LMBS tiny::t86::MOV( freeReg, tiny::t86::Bp()) LMBE, ins, pos++);
                    targetProgram_.addF_insert(LMBS tiny::t86::SUB( freeReg, (int64_t) location->second.stackOffset() + 1 ) LMBE , ins, pos++);
                    break;
                }

            }
        }
    }

    void
    SuperNaiveRegisterAllocator::setupFRegister(VirtualRegisterPlaceholder *reg, const Instruction *ins, size_t pos) {

    }
    void SuperNaiveRegisterAllocator::removeFromRegisterDescriptor(const Instruction *ins) {
        auto address = addressDescriptor_.find(ins);
        if(address != addressDescriptor_.end()){
            if (address->second.loc() == Location::Register){
                registerDescriptor_[address->second.regIndex()].erase(ins);
            }
        }

    }
    void SuperNaiveRegisterAllocator::replaceInRegisterDescriptor(const Instruction * replace, const Instruction * with) {
        auto address = addressDescriptor_.find(replace);
        if(address != addressDescriptor_.end()){
            if (address->second.loc() == Location::Register){
                registerDescriptor_[address->second.regIndex()].erase(replace);
                registerDescriptor_[address->second.regIndex()].insert(with);
                addressDescriptor_.insert(std::make_pair(with, addressDescriptor_.at(replace)));
                //TODO what all should be in renaming changed?
            }
        }

    }

    SuperNaiveRegisterAllocator::Register SuperNaiveRegisterAllocator::getReg(const Instruction *ins) {

        return getFreeRegister();
    }



}