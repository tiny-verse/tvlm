#include "SuperNaiveRegisterAllocator.h"

#include "tvlm/tvlm/codeGeneration/FunctionalMacro.h"

namespace tvlm {


    void SuperNaiveRegisterAllocator::visit(Instruction *ins) {


    }

    void SuperNaiveRegisterAllocator::visit(Jump *ins) {
    }

    void SuperNaiveRegisterAllocator::visit(CondJump *ins) {

        auto virtRegs = getAllocatedVirtualRegisters(ins);
        writingPos_= 0;setupRegister(((*virtRegs)[0]), ins, ins);
        updateJumpPatchPositions(ins);

    }

    void SuperNaiveRegisterAllocator::visit(Return *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        if(ins->returnType()->registerType() == ResultType::StructAddress){

        } else if (ins->returnType()->registerType() == ResultType::Double){

        }else if (ins->returnType()->registerType() == ResultType::Integer){
            writingPos_= 0;setupRegister(((*virtRegs)[0]), ins->returnValue(),ins);

        }else{

        }
//execute calling conventions before return


    }

    void SuperNaiveRegisterAllocator::visit(CallStatic *ins) {

        writingPos_ = 0;

        updateCallPatchPositions(ins);



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

        writingPos_ =0;
        updateCallPatchPositions(ins);

    }

    void SuperNaiveRegisterAllocator::visit(Copy *ins) {

        auto virtRegs = getAllocatedVirtualRegisters(ins);
        switch (ins->resultType()) {
            case ResultType::Integer: {
                writingPos_= 0;
                setupRegister(((*virtRegs)[0]), ins, ins);
                setupRegister(((*virtRegs)[1]), ins->src(), ins);
                break;
            }
            case ResultType::Double: {

                writingPos_= 0;
                setupFRegister(((*virtRegs)[0]), ins, ins);
                setupFRegister(((*virtRegs)[1]), ins->src(), ins);
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
        writingPos_= 0;
        setupFRegister(((*virtRegs)[0]), ins, ins);
        setupRegister(((*virtRegs)[1]), ins->src(), ins);
    }

    void SuperNaiveRegisterAllocator::visit(Truncate *ins) {

        auto virtRegs = getAllocatedVirtualRegisters(ins);
        writingPos_= 0;
        setupRegister(((*virtRegs)[0]), ins, ins);
        setupFRegister(((*virtRegs)[1]), ins->src(), ins);
    }

    void SuperNaiveRegisterAllocator::visit(BinOp *ins) {

        auto virtRegs = getAllocatedVirtualRegisters(ins);
        switch (ins->resultType()) {
            case ResultType::StructAddress:
            case ResultType::Integer: {
                writingPos_= 0;
                setupRegister(((*virtRegs)[0]), ins->lhs(), ins);
                setupRegister(((*virtRegs)[1]), ins->rhs(), ins);
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
                        writingPos_= 0;
                        setupFRegister(((*virtRegs)[0]), ins->lhs(), ins);
                        setupFRegister(((*virtRegs)[1]), ins->rhs(), ins);
                        replaceInRegister(ins->lhs(), ins);
                        break;

                    case BinOpType::NEQ:
                    case BinOpType::EQ:
                    case BinOpType::LTE:
                    case BinOpType::LT:
                    case BinOpType::GT:
                    case BinOpType::GTE:
                        writingPos_= 0;
                        setupFRegister(((*virtRegs)[0]), ins->lhs(), ins);
                        setupFRegister(((*virtRegs)[1]), ins->rhs(), ins);
                        setupRegister(((*virtRegs)[2]), ins, ins);
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
                        writingPos_= 0;
                        setupRegister(((*virtRegs)[0]), ins->operand(), ins);
                        setupRegister(((*virtRegs)[1]), ins, ins);
                        break;
                    }
                    case UnOpType::INC:
                    case UnOpType::NOT:
                    case UnOpType::DEC:
                        writingPos_= 0;
                        setupRegister(((*virtRegs)[0]), ins->operand(), ins);
                        replaceInRegister(ins->operand(), ins);

                        break;
                }
            }
            case ResultType::Double:
                setupFRegister(((*virtRegs)[0]), ins, ins);

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
                writingPos_= 0;
                setupRegister(((*virtRegs)[0]), ins, ins);
//                auto reg = getReg(ins, ins);
                break;
            }
            case ResultType::Double:{
                writingPos_= 0;
                setupFRegister(((*virtRegs)[0]), ins, ins);

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
        writingPos_= 0;
        setupRegister(((*virtRegs)[0]), ins, ins);

    }

    void SuperNaiveRegisterAllocator::visit(AllocG *ins) {

    }

    void SuperNaiveRegisterAllocator::visit(ArgAddr *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        writingPos_= 0;
        setupRegister(((*virtRegs)[0]), ins, ins);

    }

    void SuperNaiveRegisterAllocator::visit(PutChar *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        writingPos_= 0;
        setupRegister(((*virtRegs)[0]), ins->src(), ins);

    }

    void SuperNaiveRegisterAllocator::visit(GetChar *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        writingPos_= 0;
        setupRegister(((*virtRegs)[0]), ins, ins);

    }

    void SuperNaiveRegisterAllocator::visit(Load *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        writingPos_= 0;
        if(ins->type()->registerType() == ResultType::Double){
            setupFRegister(((*virtRegs)[0]), ins, ins);
            setupRegister(((*virtRegs)[1]), ins->address(), ins);
            return;
        }else if (ins->type()->registerType() == ResultType::Integer){

            auto it = targetProgram_.globalFind(ins->address());
            if(it != targetProgram_.globalEnd()) {
                setupRegister(((*virtRegs)[0]), ins, ins);
                return;
            }
//            if(dynamic_cast<Type::Array *>(ins->type())){
//                setupRegister(((*virtRegs)[0]), ins, 0);
//                setupRegister(((*virtRegs)[1]), ins->address(), 1);
//            }else{
                setupRegister(((*virtRegs)[0]), ins, ins);
                setupRegister(((*virtRegs)[1]), ins->address(), ins);
//            }
            return;
        }else if (ins->type()->registerType() == ResultType::StructAddress){
            replaceInRegister(ins->address(), ins);
            return;
        }
        throw "ERROR[RA] failed load";
    }

    void SuperNaiveRegisterAllocator::visit(Store *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        writingPos_= 0;
        setupRegister(((*virtRegs)[0]), ins->address(), ins);

        switch (ins->value()->resultType()) {
            case ResultType::Double:{
                setupFRegister(((*virtRegs)[1]), ins->value(), ins);
                return;
            }
            case ResultType::StructAddress:
            case ResultType::Integer:{
                setupRegister(((*virtRegs)[1]), ins->value(), ins);
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
        writingPos_= 0;
        writingPos_= 0;
        setupRegister(((*virtRegs)[0]), ins, ins);
        setupRegister(((*virtRegs)[1]), ins->base(), ins);
        setupRegister(((*virtRegs)[2]), ins->offset(), ins);

    }

    void SuperNaiveRegisterAllocator::visit(ElemAddrIndex *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        writingPos_= 0;
        setupRegister(((*virtRegs)[0]), ins, ins);
        setupRegister(((*virtRegs)[1]), ins->base(), ins);
        setupRegister(((*virtRegs)[2]), ins->index(), ins);

    }

    void SuperNaiveRegisterAllocator::visit(Halt *ins) { }

    void SuperNaiveRegisterAllocator::visit(StructAssign *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        writingPos_= 0;
        setupRegister(((*virtRegs)[0]), ins->srcVal(), ins);
        setupRegister(((*virtRegs)[1]), ins->dstAddr(), ins);
        setupRegister(((*virtRegs)[2]), ins, ins);
    }

    void SuperNaiveRegisterAllocator::visit(BasicBlock *bb) {
        enterNewBB(bb);

        for (auto * ins : getBBsInstructions(bb)) {
            visitChild(ins);
        }
        exitBB();
    }

    void SuperNaiveRegisterAllocator::visit(Function *fce) {
        currenFunction_ = fce;
        enterNewFce(fce);
        for (BasicBlock * bb : getFunctionBBs(fce)){
            visitChild(bb);
        }
        exitFce();

    }

    void SuperNaiveRegisterAllocator::visit(Program *p) {
        auto globals = getProgramsGlobals(p);
        enterNewBB(globals);


        for(const auto *ins : getBBsInstructions(globals)){
            if(const auto * i = dynamic_cast<const  LoadImm *>(ins)){
                if(i->resultType() == ResultType::Integer){

//                    auto reg = getReg(ins, ins);
                    auto virtRegs = getAllocatedVirtualRegisters(ins);
                    writingPos_= 0;
                    setupRegister(((*virtRegs)[0]), ins, ins);
                }else if (i->resultType() == ResultType::Double){
//                    auto freg=getFReg(ins, ins);
                    auto virtRegs = getAllocatedVirtualRegisters(ins);
                    writingPos_= 0;
                    setupRegister(((*virtRegs)[0]), ins, ins);
                }else{
                    throw "load imm with Res Type Structure or Void not implemented";
                }
            }else if(const auto * alloc = dynamic_cast< const AllocG *>(ins)){
                if(alloc->amount()){
                    throw "allocG with array not implemented"; //TODO global array
                }else{
//                    getReg(alloc, ins);
                    auto virtRegs = getAllocatedVirtualRegisters(ins);
                    writingPos_= 0;
                    setupRegister(((*virtRegs)[0]), ins, ins);
                }
            }else if( const auto  * store = dynamic_cast<const Store *>(ins)){
//                auto reg = getReg(ins, ins);
                auto virtRegs = getAllocatedVirtualRegisters(ins);
                writingPos_= 0;
                setupRegister(((*virtRegs)[0]), ins, ins);
            }else{
                throw tiny::ParserError("unknown global instruction", ins->ast()->location());
            }

        }
        exitBB();
        for (const auto & str : p->stringLiterals() ) {
//            auto reg = getReg(str.second, str.second);
            auto virtRegs = getAllocatedVirtualRegisters(str.second);
            writingPos_= 0;
            setupRegister(((*virtRegs)[0]), str.second, str.second);
        }

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
        for(size_t i = 1 ; i < regSize ; i++){
            freeReg_.emplace(VirtualRegisterPlaceholder(RegisterType::INTEGER, i));
        }
        size_t fregSize = tiny::t86::Cpu::Config::instance().floatRegisterCnt();
        for(size_t i = 1; i < fregSize ; i++){
            freeFReg_.emplace(VirtualRegisterPlaceholder(RegisterType::FLOAT, i));
        }


    }

    SuperNaiveRegisterAllocator::VirtualRegister
    SuperNaiveRegisterAllocator::getRegister(SuperNaiveRegisterAllocator::VirtualRegister &reg,const Instruction * ins,  const Instruction * currentIns) {
        currentWorkingReg_ = &reg;
        auto it = addressDescriptor_.find(ins); //auto it = regMapping_.find(&reg);
        if(it != addressDescriptor_.end() /*regMapping_.end()*/){
            switch (it->second.begin()->loc()){
                case Location::Register:
                    return it->second.begin()->regIndex();

                    break;
                case Location::Stack:
                case Location::Memory:

                    auto free = getReg(currentIns);
                    restore( free, * it->second.begin(), ins);
//                    addressDescriptor_.erase(it); //regMapping_.erase(it);
                    addressDescriptor_[ins].emplace( LocationEntry(Location::Register, free, ins) );   //regMapping_.emplace( &reg,LocationEntry(Location::Register, free.index(), ins) );
                    registerDescriptor_[free].emplace(ins);
                    return free;
                    break;
            }
        }else{
            //notFound --> Allocate new
            VirtualRegister newReg = getReg(currentIns);
            addressDescriptor_[ins].emplace( LocationEntry(Location::Register, newReg, ins)); //regMapping_.emplace(&reg, LocationEntry(Location::Register, newReg.index(), ins));
            return newReg;
        }
        throw "error this shouldn't happen 123";
    }

    SuperNaiveRegisterAllocator::VirtualRegister SuperNaiveRegisterAllocator::getRegToSpill() {
        VirtualRegister res = regQueue_.front();
        regQueue_.erase(regQueue_.begin());
        return res;
    }

    bool SuperNaiveRegisterAllocator::spill( SuperNaiveRegisterAllocator::VirtualRegister &regToSpill, const Instruction * currentIns) {
        int stackOffset;
        VirtualRegister & virtualRegister = regToSpill;
        auto regDesc = registerDescriptor_.find(regToSpill);
        auto newStackPosition = targetProgram_.funcLocalAlloc_[currenFunction_]++;
        LocationEntry newStackPlace = LocationEntry(Location::Stack, newStackPosition, nullptr);
        if(regDesc != registerDescriptor_.end()){
            for (auto * ins : regDesc->second) { //remove Register descriptor and add Stack Descriptor for each instruction in this register
                auto addr = addressDescriptor_.find(ins);
                if(addr == addressDescriptor_.end()){
                    throw "RA spill - spilling Instruction not registered with address - in consistency";
                }

                for (auto locIt = addr->second.begin() ; locIt != addr->second.end();) {//for each location of instruction equal to our register
                    switch (locIt->loc()){
                        case Location::Register:{
                            if(locIt->regIndex() == regToSpill){
                                newStackPlace = LocationEntry(Location::Stack, newStackPosition,  locIt->ins());
                                addr->second.insert(newStackPlace);
                                locIt = addr->second.erase(locIt);
                            }
                            break;}
                        case Location::Stack:
                        case Location::Memory:{
                            locIt++;
                            break;}
                    }



                }

            }
            //spill Code
            VirtualRegister lastReg = getLastRegister(currentIns);
            Register lReg = Register(lastReg.getNumber());
            targetProgram_.addF_insert(LMBS tiny::t86::MOV( lReg, tiny::t86::Bp())LMBE, currentIns, writingPos_++);
            targetProgram_.addF_insert(LMBS tiny::t86::SUB( lReg, (int64_t)newStackPosition)LMBE, currentIns, writingPos_++);
            targetProgram_.addF_insert(LMBS tiny::t86::MOV( tiny::t86::Mem(lReg), Register(regToSpill.getNumber()))LMBE, currentIns, writingPos_++);
            releaseRegister(lastReg);

    return true;
        }else{
            throw "[RA] cannot spill non existing Register";
        }

    }

    void SuperNaiveRegisterAllocator::restore(const SuperNaiveRegisterAllocator::VirtualRegister &whereTo,
                                              const LocationEntry &from, const Instruction * currentIns) {
        //TODO implement restore
        switch(whereTo.getType()){

            case RegisterType::INTEGER:
                switch (from.loc()) {
                    case Location::Register:
                        targetProgram_.addF_insert(LMBS tiny::t86::MOV( Register(whereTo.getNumber()), Register(from.regIndex().getNumber()) ) LMBE,currentIns, writingPos_++);

                        break;
                    case Location::Stack:
                        targetProgram_.addF_insert(LMBS tiny::t86::MOV( Register(whereTo.getNumber()), tiny::t86::Bp()) LMBE,currentIns, writingPos_++);
                        targetProgram_.addF_insert(LMBS tiny::t86::SUB( Register(whereTo.getNumber()), from.stackOffset()) LMBE,currentIns, writingPos_++);
                        targetProgram_.addF_insert(LMBS tiny::t86::MOV( Register(whereTo.getNumber()), tiny::t86::Mem( Register(whereTo.getNumber()))) LMBE,currentIns, writingPos_++);
                        break;
                    case Location::Memory:
                        targetProgram_.addF_insert(LMBS tiny::t86::MOV( Register(whereTo.getNumber()), tiny::t86::Mem(from.memAddress())) LMBE,currentIns, writingPos_);
                        break;
                }
                    case RegisterType::FLOAT:
                        switch (from.loc()) {
                            case Location::Register:
                                targetProgram_.addF_insert(LMBS tiny::t86::MOV( FRegister(whereTo.getNumber()), Register(from.regIndex().getNumber()) ) LMBE,currentIns, writingPos_++);

                                break;
                            case Location::Stack:{

                                VirtualRegister freeRegVirt = getReg(currentIns);
                                Register freeReg = freeRegVirt.getNumber();
                                targetProgram_.addF_insert(LMBS tiny::t86::MOV( freeReg, tiny::t86::Bp()) LMBE,currentIns, writingPos_++);
                                targetProgram_.addF_insert(LMBS tiny::t86::SUB( freeReg, from.stackOffset()) LMBE,currentIns, writingPos_++);
                                targetProgram_.addF_insert(LMBS tiny::t86::MOV( FRegister(whereTo.getNumber()), tiny::t86::Mem( Register(whereTo.getNumber()))) LMBE,currentIns, writingPos_++);

                                break;
                            }
                            case Location::Memory:
                                targetProgram_.addF_insert(LMBS tiny::t86::MOV( FRegister(whereTo.getNumber()), tiny::t86::Mem(from.memAddress())) LMBE,currentIns, writingPos_);
                                break;
                        }
        }
    }

    SuperNaiveRegisterAllocator::VirtualRegister SuperNaiveRegisterAllocator::getFreeFRegister(const Instruction * currentIns) {

    }

    SuperNaiveRegisterAllocator::VirtualRegister SuperNaiveRegisterAllocator::getReg( const Instruction * currentIns) {
        VirtualRegister res = VirtualRegister(RegisterType::INTEGER, 0);
        if(freeReg_.size() > 1){
            res = freeReg_.front();
            freeReg_.pop();

            regQueue_.push_back(res);//TODO
        }else {
            VirtualRegisterPlaceholder ress = getRegToSpill();
            spill( ress, currentIns);
            res = ress;
            regQueue_.push_back(res);
        }
        return res;
    }

    SuperNaiveRegisterAllocator::VirtualRegister SuperNaiveRegisterAllocator::getLastRegister(const Instruction * currentIns) {
        VirtualRegister res = VirtualRegister(RegisterType::INTEGER, 0);
        if(!freeReg_.empty()){
            res = freeReg_.front();
            freeReg_.pop();
        }else {
            throw "no free register";
        }
        return res;
    }
    void SuperNaiveRegisterAllocator::releaseRegister(const VirtualRegister & reg) {
        freeReg_.push(reg);
        auto it = std::find(regQueue_.begin(), regQueue_.end(),reg);
        if(it != regQueue_.end()){
            regQueue_.erase(it);
        }
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

    void SuperNaiveRegisterAllocator::setupRegister(VirtualRegisterPlaceholder & reg, const Instruction * ins,  const Instruction *currentIns) {
        auto location = addressDescriptor_.find(ins);
        if(location != addressDescriptor_.end()){
//            for (auto & loc : location->second) { //priority in ordered set will manage to do it
//                if(loc.loc() == Location::Register){
//                    reg->setNumber(loc.regIndex().getNumber());
//                    return;
//                }
//            }
            for (auto & loc : location->second) {
                switch(loc.loc()) {
                    case Location::Register:
                        reg.setNumber(loc.regIndex().getNumber());
                        return;
                    case Location::Memory: {
                        VirtualRegister freeVirtReg = getReg(currentIns);
                        Register freeReg = freeVirtReg.getNumber();
                        targetProgram_.addF_insert(
                                LMBS tiny::t86::MOV(freeReg, tiny::t86::Mem(loc.memAddress()))LMBE, currentIns,
                                writingPos_++);
                        addressDescriptor_[ins].emplace(LocationEntry(Location::Register, freeVirtReg, ins));
                        registerDescriptor_[freeVirtReg].emplace(ins);

                        reg.setNumber(freeReg.index());
                        return;
                    }
                    case Location::Stack: {


                        VirtualRegister freeVirtReg = getReg(currentIns);
                        Register freeReg = freeVirtReg.getNumber();


                        targetProgram_.addF_insert(LMBS tiny::t86::MOV(freeReg, tiny::t86::Bp())LMBE, currentIns, writingPos_++);
                        targetProgram_.addF_insert(
                                LMBS tiny::t86::SUB(freeReg, (int64_t) loc.stackOffset() )LMBE, currentIns,
                                writingPos_++);
                        targetProgram_.addF_insert(LMBS tiny::t86::MOV(freeReg, tiny::t86::Mem(freeReg))LMBE, currentIns, writingPos_++);
                        addressDescriptor_[currentIns].emplace(Location::Register, freeVirtReg, currentIns);
                        registerDescriptor_[freeVirtReg].emplace(currentIns);
                        reg.setNumber(freeReg.index());
                        return;
                    }
                }
            }
        }else{//not assigned
            auto regToAssign = getReg(currentIns);
            assert(regToAssign.getNumber() <= tiny::t86::Cpu::Config::instance().registerCnt());
            reg.setNumber(regToAssign.getNumber());
            addressDescriptor_[currentIns].insert(LocationEntry(Location::Register, regToAssign, currentIns));
            registerDescriptor_[regToAssign].emplace(currentIns);
            return;
        }
    }

    void
    SuperNaiveRegisterAllocator::setupFRegister(VirtualRegisterPlaceholder & reg, const Instruction *ins, const Instruction * currentIns) {

    }
    void SuperNaiveRegisterAllocator::removeFromRegisterDescriptor(const Instruction *ins) {
        throw "not Implemented [RA] removeFromRegisterDescriptor fnc ";
        auto address = addressDescriptor_.find(ins);
        if(address != addressDescriptor_.end()){
//            if (address->second.loc() == Location::Register){
//                registerDescriptor_[address->second.regIndex()].erase(ins);
//            }
        }

    }
    void SuperNaiveRegisterAllocator::replaceInRegister(const Instruction * replace, const Instruction * with) {
        auto address = addressDescriptor_.find(replace);
        if(address != addressDescriptor_.end()){
            for (auto  loc  = address->second.begin(); loc != address->second.end(); ) {
                if (loc->loc() == Location::Register){
                    registerDescriptor_[loc->regIndex()].erase(replace);
                    registerDescriptor_[loc->regIndex()].emplace(with);

                    for (auto  addr = addressDescriptor_[replace].begin(); addr != addressDescriptor_[replace].end();) { // remove descriptor with Register ( transfered)
                        if(addr->loc() == Location::Register){
                            addressDescriptor_[with].insert(*addr);
                            addr = addressDescriptor_[replace].erase(addr);
                            loc = address->second.begin();
                        }else{
                            addr ++;
                            loc++;
                        }
                    }
                }
            }
        }

    }

    void SuperNaiveRegisterAllocator::updateJumpPatchPositions(const Instruction * ins) {
        for ( size_t pos : targetProgram_.jumpPos_[ins]) {
            targetProgram_.jump_patches_[pos].first.second = Label( targetProgram_.jump_patches_[pos].first.second.address() + writingPos_);
        }
    }
    void SuperNaiveRegisterAllocator::updateCallPatchPositions(const Instruction * ins) {
        for ( size_t pos : targetProgram_.callPos_[ins]) {
            targetProgram_.call_patches_[pos].first.second = Label( targetProgram_.call_patches_[pos].first.second.address() + writingPos_);
        }
    }


}