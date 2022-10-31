#include "RegisterAllocator.h"

#include "tvlm/tvlm/codeGeneration/FunctionalMacro.h"

namespace tvlm{


    void RegisterAllocator::visit(Instruction *ins) {
    }

    void RegisterAllocator::visit(Jump *ins) {
    }

    void RegisterAllocator::visit(CondJump *ins) {

        auto virtRegs = getAllocatedVirtualRegisters(ins);
        writingPos_= 0;setupRegister(((*virtRegs)[0]), ins->condition(), ins);
        updateJumpPatchPositions(ins);

    }

    void RegisterAllocator::visit(Return *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        if(ins->returnType()->registerType() == ResultType::StructAddress){

        } else if (ins->returnType()->registerType() == ResultType::Double){

        }else if (ins->returnType()->registerType() == ResultType::Integer){
            writingPos_= 0;setupRegister(((*virtRegs)[0]), ins->returnValue(),ins);

        }else{

        }
        callingConvCalleeRestore(ins);


    }


    void RegisterAllocator::visit(CallStatic *ins) {

        writingPos_ = 0;
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        int regPos = 0;

        //        //args /*-> prepare values
        for (auto it = ins->args().crbegin() ; it != ins->args().crend();it++) {
            if((*it).second->registerType() == ResultType::StructAddress) {
//                allocateStructArg(it->second, it->first);
            }else if((*it).second->registerType() == ResultType::Integer){
//                auto argReg = getReg(it->first, ins);
                setupRegister(((*virtRegs)[regPos++]), it->first, ins);
//                clearIntReg(it->first);
            }else if ((*it).second->registerType() == ResultType::Double){
//                auto argFReg = getFReg(it->first, ins);

                setupFRegister(((*virtRegs)[regPos++]), it->first, ins);
//                addF(LMBS tiny::t86::FPUSH(vFR(argFReg)) LMBE, ins);
//                clearFloatReg(it->first);
            }
        }
        //        //prepare return Value //*-> preparation in RA -- memory and register in RA



//        spillCallReg(); //implement calling convention -> spill caller saved
        callingConvCallerSave(ins);

        updateCallPatchPositions(ins);
//        tiny::t86::Label callLabel = addF(
//                LMBS tiny::t86::CALL{tiny::t86::Label::empty()} LMBE
//                , ins );
        writingPos_++;




        size_t returnValueRegister = 0;
        if(ins->f()->getType()->registerType() == ResultType::StructAddress){
//            prepareReturnValue(ins->f()->getType()->size(), ins);
            setupRegister(((*virtRegs)[regPos++]), ins, ins);
        } else if (ins->f()->getType()->registerType() == ResultType::Double){
            setupFRegister(((*virtRegs)[regPos++]), ins, ins);

        }else if (ins->f()->getType()->registerType() == ResultType::Integer) {
//            returnValueRegister = getReg(ins, ins);
            setupRegister(((*virtRegs)[regPos++]), ins, ins);


        }else{//void

        }

        //implement restoring? no need -- needed will be restored - lazy


    }

    void RegisterAllocator::visit(Call *ins) {

        writingPos_ = 0;
        auto virtRegs = getAllocatedVirtualRegisters(ins);

        int regPos = 0;

        //        //args /*-> prepare values
        for (auto it = ins->args().crbegin() ; it != ins->args().crend();it++) {
            if((*it).second->registerType() == ResultType::StructAddress) {
//                allocateStructArg(it->second, it->first);
            }else if((*it).second->registerType() == ResultType::Integer){
//                auto argReg = getReg(it->first, ins);
                setupRegister(((*virtRegs)[regPos++]), it->first, ins);
//                clearIntReg(it->first);
            }else if ((*it).second->registerType() == ResultType::Double){
//                auto argFReg = getFReg(it->first, ins);

                setupFRegister(((*virtRegs)[regPos++]), it->first, ins);
//                addF(LMBS tiny::t86::FPUSH(vFR(argFReg)) LMBE, ins);
//                clearFloatReg(it->first);
            }
        }
        //        //prepare return Value //*-> preparation in RA -- memory and register in RA


//        spillCallReg(); //TODO implement calling convention
        callingConvCallerSave(ins);

        updateCallPatchPositions(ins);
//        tiny::t86::Label callLabel = addF(
//                LMBS tiny::t86::CALL{tiny::t86::Label::empty()} LMBE
//                , ins );
        writingPos_++;

        // implement restoring? - no

        size_t returnValueRegister = 0;
        if(ins->retType()->registerType() == ResultType::StructAddress){
//            prepareReturnValue(ins->f()->getType()->size(), ins);
            setupRegister(((*virtRegs)[regPos++]), ins, ins);
        } else if (ins->retType()->registerType() == ResultType::Double){
            setupFRegister(((*virtRegs)[regPos++]), ins, ins);

        }else if (ins->retType()->registerType() == ResultType::Integer) {
//            returnValueRegister = getReg(ins, ins);
            setupRegister(((*virtRegs)[regPos++]), ins, ins);


        }else{//void

        }

    }

    void RegisterAllocator::visit(Copy *ins) {

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

    void RegisterAllocator::visit(Extend *ins) {

        auto virtRegs = getAllocatedVirtualRegisters(ins);
        writingPos_= 0;
        setupFRegister(((*virtRegs)[0]), ins, ins);
        setupRegister(((*virtRegs)[1]), ins->src(), ins);
    }

    void RegisterAllocator::visit(Truncate *ins) {

        auto virtRegs = getAllocatedVirtualRegisters(ins);
        writingPos_= 0;
        setupRegister(((*virtRegs)[0]), ins, ins);
        setupFRegister(((*virtRegs)[1]), ins->src(), ins);
    }

    void RegisterAllocator::visit(BinOp *ins) {

        auto virtRegs = getAllocatedVirtualRegisters(ins);
        switch (ins->resultType()) {
            case ResultType::StructAddress:
            case ResultType::Integer: {
                writingPos_= 0;
                setupRegister(((*virtRegs)[0]), ins->lhs(), ins);
                setupRegister(((*virtRegs)[1]), ins->rhs(), ins);
                replaceInRegister(ins->lhs(), ins);
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

    void RegisterAllocator::visit(UnOp *ins) {

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
                    case UnOpType::NOT:
                        writingPos_= 0;
                        setupRegister(((*virtRegs)[0]), ins->operand(), ins);
                        replaceInRegister(ins->operand(), ins);
                        break;
                    case UnOpType::INC:
                    case UnOpType::DEC:
                        writingPos_= 0;
                        setupRegister(((*virtRegs)[0]), ins->operand(), ins);
//                        if(auto load = dynamic_cast<Load*>(ins->operand())){
//                            setupRegister(((*virtRegs)[1]), load->address(), ins);
//                            //store incremented -- TODO make dirty reg
//                        }
                        //is it necessary to save store location?
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

    void RegisterAllocator::visit(LoadImm *ins) {
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

    void RegisterAllocator::visit(AllocL *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        writingPos_= 0;
        setupRegister(((*virtRegs)[0]), ins, ins);

    }

    void RegisterAllocator::visit(AllocG *ins) {

    }

    void RegisterAllocator::visit(ArgAddr *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        writingPos_= 0;
        setupRegister(((*virtRegs)[0]), ins, ins);

    }

    void RegisterAllocator::visit(PutChar *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        writingPos_= 0;
        setupRegister(((*virtRegs)[0]), ins->src(), ins);

    }

    void RegisterAllocator::visit(GetChar *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        writingPos_= 0;
        setupRegister(((*virtRegs)[0]), ins, ins);

    }

    void RegisterAllocator::visit(Load *ins) {
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

    void RegisterAllocator::visit(Store *ins) {
        auto *virtRegs = getAllocatedVirtualRegisters(ins);
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


    void RegisterAllocator::visit(Phi *ins) {
        //TODO phi function to inform registers
    }

    void RegisterAllocator::visit(ElemAddrOffset *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        writingPos_= 0;
        writingPos_= 0;
        setupRegister(((*virtRegs)[0]), ins, ins);
        setupRegister(((*virtRegs)[1]), ins->base(), ins);
        setupRegister(((*virtRegs)[2]), ins->offset(), ins);

    }

    void RegisterAllocator::visit(ElemAddrIndex *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        writingPos_= 0;
        setupRegister(((*virtRegs)[0]), ins, ins);
        setupRegister(((*virtRegs)[1]), ins->base(), ins);
        setupRegister(((*virtRegs)[2]), ins->index(), ins);

    }

    void RegisterAllocator::visit(Halt *ins) { }

    void RegisterAllocator::visit(StructAssign *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        writingPos_= 0;
        setupRegister(((*virtRegs)[0]), ins->srcVal(), ins);
        setupRegister(((*virtRegs)[1]), ins->dstAddr(), ins);
        setupRegister(((*virtRegs)[2]), ins, ins);
    }

    void RegisterAllocator::visit(BasicBlock *bb) {
//        enterNewBB(bb);

        for (auto * ins : getBBsInstructions(bb)) {
            visitChild(ins);
        }
//        exitBB();
    }

    void RegisterAllocator::visit(Function *fce) {
        currenFunction_ = fce;
//        enterNewFce(fce);
        for (BasicBlock * bb : getFunctionBBs(fce)){
            visitChild(bb);
        }
//        exitFce();

    }

    void RegisterAllocator::visit(Program *p) {
        auto globals = getProgramsGlobals(p);
//        enterNewBB(globals);


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
//        exitBB();
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


    RegisterAllocator::RegisterAllocator(TargetProgram &tp):
            targetProgram_(tp),
            writingPos_(0),
            currenFunction_(nullptr)
    {
//        size_t regSize = tiny::t86::Cpu::Config::instance().registerCnt();
//        for(size_t i = 1 ; i < regSize ; i++){
//            freeReg_.emplace(VirtualRegisterPlaceholder(RegisterType::INTEGER, i));
//        }
//        size_t fregSize = tiny::t86::Cpu::Config::instance().floatRegisterCnt();
//        for(size_t i = 1; i < fregSize ; i++){
//            freeFReg_.emplace(VirtualRegisterPlaceholder(RegisterType::FLOAT, i));
//        }


    }


    std::set<LocationEntry>::iterator RegisterAllocator::findLocation(std::set<LocationEntry> & set1, Location location) {
        for (auto it  = set1.begin(); it != set1.end();it++) {
            if(it->loc() == location){
                return it;
            }
        }
        return set1.end();
    }

    bool RegisterAllocator::spill( const  RegisterAllocator::VirtualRegister &regToSpill, const Instruction * currentIns) {
        int stackOffset;
        const VirtualRegister & virtualRegister = regToSpill;
        auto regDesc = registerDescriptor_.find(regToSpill);
        size_t newPosition = 0;
        LocationEntry newStackPlace = LocationEntry(newPosition, nullptr);

        bool memset = false, stackset = false;
        if(regDesc != registerDescriptor_.end() && ! regDesc->second.empty()){ //only if in reg is something
            for (auto itRegDesc = regDesc->second.begin();itRegDesc != regDesc->second.end();) {    //remove Register descriptor and find Stack Descriptor for each instruction in this register
                // if not found ADD it
                auto * ins = *itRegDesc;
                auto addr = addressDescriptor_.find(ins);
                if(addr == addressDescriptor_.end()){
                    throw "RA spill - spilling Instruction not registered with address - breaks consistency";
                }
                auto regLoc = findLocation(addr->second, Location::Register);
                if(regLoc != addr->second.end() && regLoc->regIndex() == regToSpill){
                    auto memLoc = findLocation(addr->second, Location::Memory);
                    auto stackLoc = findLocation(addr->second, Location::Stack);
                    if(memLoc != addr->second.end()){ //spill to memory (save)
                        if(!memset){
                            newPosition = memLoc->memAddress();
                            memset = true;
                        }
                    }else {
                        if(!stackset){
                            stackset = true;
                            if(stackLoc != addr->second.end()){ //use old stack loc
                                newPosition = stackLoc->stackOffset();
                            }else{ // create new stack loc
                                newStackPlace = LocationEntry(getFuncLocalAlloc(targetProgram_)[currenFunction_]++,  regLoc->ins());
                                newPosition = newStackPlace.stackOffset();
                                addr->second.insert(newStackPlace);
                            }
                        }
                    }

                    regLoc = findLocation(addr->second, Location::Register);
                    do {
                        addr->second.erase(regLoc); //remove addr-descriptor that says: it is in Register
                        regLoc = findLocation(addr->second, Location::Register);
                    } while (regLoc!=addr->second.end());
                }


                itRegDesc = regDesc->second.erase(itRegDesc); // remove reg-descriptor from this register
            }

            if(memset){
                VirtualRegister lastReg = getLastRegister(currentIns);
                //spill Code IntReg
                Register lReg = Register(lastReg.getNumber());
                targetProgram_.addF_insert(LMBS tiny::t86::MOV( lReg, tiny::t86::Bp())LMBE, currentIns, writingPos_++);
                targetProgram_.addF_insert(LMBS tiny::t86::SUB( lReg, (int64_t) newPosition)LMBE, currentIns, writingPos_++);
                if(regToSpill.getType() == RegisterType::INTEGER){
                    targetProgram_.addF_insert(LMBS tiny::t86::MOV( tiny::t86::Mem(lReg), Register(regToSpill.getNumber()))LMBE, currentIns, writingPos_++);
                }else if (regToSpill.getType() == RegisterType::FLOAT){
                    targetProgram_.addF_insert(LMBS tiny::t86::MOV( tiny::t86::Mem(lReg), FRegister(regToSpill.getNumber()))LMBE, currentIns, writingPos_++);
                }else{
                    throw "[RA] spill: register type not handled (in memory spill)";
                }
                releaseRegister(lastReg);
            }else if (stackset){
//spill Code
                VirtualRegister lastReg = getLastRegister(currentIns);
                Register lReg = Register(lastReg.getNumber());
                targetProgram_.addF_insert(LMBS tiny::t86::MOV( lReg, tiny::t86::Bp())LMBE, currentIns, writingPos_++);
                targetProgram_.addF_insert(LMBS tiny::t86::SUB( lReg, (int64_t)newPosition)LMBE, currentIns, writingPos_++);
                if(regToSpill.getType() == RegisterType::INTEGER){
                    targetProgram_.addF_insert(LMBS tiny::t86::MOV( tiny::t86::Mem(lReg), Register(regToSpill.getNumber()))LMBE, currentIns, writingPos_++);
                }else if (regToSpill.getType() == RegisterType::FLOAT){
                    targetProgram_.addF_insert(LMBS tiny::t86::MOV( tiny::t86::Mem(lReg), FRegister(regToSpill.getNumber()))LMBE, currentIns, writingPos_++);
                }else{
                    throw "[RA] spill: register type not handled (in stack spill)";
                }
                releaseRegister(lastReg);
            }else{
                throw "[RA] error - couldn't pick mem nor stack place to spill";
            }

            return true;
        }else if ( regDesc->second.empty()) {
            return false;
        }else {
            throw "[RA] cannot spill non existing Register";

        }

    }

    void RegisterAllocator::restore(const RegisterAllocator::VirtualRegister &whereTo,
                                              const LocationEntry &from, const Instruction * currentIns) {
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



    void RegisterAllocator::setupRegister(VirtualRegisterPlaceholder & reg, const Instruction * ins,  const Instruction *currentIns) {
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
                        addressDescriptor_[ins].emplace(LocationEntry(freeVirtReg, ins));
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
                        addressDescriptor_[currentIns].emplace( freeVirtReg, currentIns);
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
            addressDescriptor_[currentIns].insert(LocationEntry(regToAssign, currentIns));
            registerDescriptor_[regToAssign].emplace(currentIns);
            return;
        }
    }

    void
    RegisterAllocator::setupFRegister(VirtualRegisterPlaceholder & reg, const Instruction *ins, const Instruction * currentIns) {

    }

    void RegisterAllocator::replaceInRegister(const Instruction * replace, const Instruction * with) {
        //we do not replace because copy is enough ( we do not need to erase)
        auto address = addressDescriptor_.find(replace);
        if(address != addressDescriptor_.end()){
            for (auto loc  = address->second.begin(); loc != address->second.end(); ) { //for each address
                if (loc->loc() == Location::Register){ //that is in register
                    registerDescriptor_[loc->regIndex()].emplace(with);  // replace with

                    for (auto  addr = addressDescriptor_[replace].begin(); addr != addressDescriptor_[replace].end();addr++) { // remove descriptor with Register ( transfered)
                        if(addr->loc() == Location::Register){
                            addressDescriptor_[with].insert(*addr);
//                            addr = addressDescriptor_[replace].erase(addr);
                            loc = address->second.begin();
                        }
                    }
//                    registerDescriptor_[loc->regIndex()].erase(replace); // remove it
                }
                loc++;
            }
        }

    }

    void RegisterAllocator::updateJumpPatchPositions(const Instruction * ins) {
        for ( size_t pos : getJumpPos(targetProgram_)[ins]) {
            getJump_patches(targetProgram_)[pos].first.second = Label( getJump_patches(targetProgram_)[pos].first.second.address() + writingPos_);
        }
    }
    void RegisterAllocator::updateCallPatchPositions(const Instruction * ins) {
        for ( auto & pos : getCallPos(targetProgram_)[ins]) {
//            targetProgram_.call_patches_[pos].first.second = Label( targetProgram_.call_patches_[pos].first.second.address() + writingPos_);
            getUnpatchedFCalls(targetProgram_)[pos].first.second = Label( getUnpatchedFCalls(targetProgram_)[pos].first.second.address() + writingPos_);
        }
    }

    void RegisterAllocator::spillAll( const Instruction * currentIns) {
        for ( auto it= registerDescriptor_.begin(); it != registerDescriptor_.end();it++) {
            spill(it->first, currentIns);
        }

    }
//    void GeneralRegisterAllocator::registerMemLocation(const Store *ins, const Instruction *currentIns) {
//
//        ins->address(); //TODO
//        auto it = targetProgram_.allocLmapping_.find(ins->address());
//        if(it != targetProgram_.allocLmapping_.end()){
//
//            addressDescriptor_[ins->value()].emplace( LocationEntry( ins->address(), ins->value(), it->second));
//        }
//
//
//    }

    void RegisterAllocator::callingConvCallerSave(const Instruction *currIns) {
        spillAll(currIns);
    }

    void RegisterAllocator::callingConvCalleeRestore(const Instruction *currIns) {

    }


    std::vector<VirtualRegisterPlaceholder> *
    RegisterAllocator::getAllocatedVirtualRegisters(const Instruction *ins) {
//            return targetProgram_.alocatedRegisters_[ins];
        auto it = getAllocatedRegisters(targetProgram_).find(ins);
        if ( it != getAllocatedRegisters(targetProgram_).end()){
            return  &(it->second);
        }else{
            throw "trying to find allocated registers for ins that was not compiled";
            if(getSelectedFInstrs(targetProgram_).find(ins) != getSelectedFInstrs(targetProgram_).end()){
                return  &(getAllocatedRegisters(targetProgram_)[ins]);
            }else{
            }
        }
    }
}