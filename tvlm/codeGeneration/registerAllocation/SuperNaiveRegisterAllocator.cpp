#include "SuperNaiveRegisterAllocator.h"

#include "tvlm/tvlm/codeGeneration/FunctionalMacro.h"

namespace tvlm {


    void SuperNaiveRegisterAllocator::visit(Instruction *ins) {


    }

    void SuperNaiveRegisterAllocator::visit(Jump *ins) {
    }

    void SuperNaiveRegisterAllocator::visit(CondJump *ins) {

        auto virtRegs = getAllocatedVirtualRegisters(ins);
        writingPos_= 0;setupRegister(((*virtRegs)[0]), ins->condition(), ins);
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
//execute calling conventions before return TODO
        callingConvCalleeRestore(ins);


    }

    void SuperNaiveRegisterAllocator::visit(CallStatic *ins) {

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



//        spillCallReg(); //TODO implement calling convention -> spill caller saved
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

    void SuperNaiveRegisterAllocator::visit(Call *ins) {

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
                    addressDescriptor_[ins].emplace( LocationEntry( free, ins) );   //regMapping_.emplace( &reg,LocationEntry( free.index(), ins) );
                    registerDescriptor_[free].emplace(ins);
                    return free;
                    break;
            }
        }else{
            //notFound --> Allocate new
            VirtualRegister newReg = getReg(currentIns);
            addressDescriptor_[ins].emplace( LocationEntry( newReg, ins)); //regMapping_.emplace(&reg, LocationEntry( newReg.index(), ins));
            return newReg;
        }
        throw "error this shouldn't happen 123";
    }

    SuperNaiveRegisterAllocator::VirtualRegister SuperNaiveRegisterAllocator::getRegToSpill() {
        VirtualRegister res = regQueue_.front();
        regQueue_.erase(regQueue_.begin());
        return res;
    }


    std::set<LocationEntry>::iterator SuperNaiveRegisterAllocator::findLocation(std::set<LocationEntry> & set1, Location location) {
        for (auto it  = set1.begin(); it != set1.end();it++) {
            if(it->loc() == location){
                return it;
            }
        }
        return set1.end();
    }

    bool SuperNaiveRegisterAllocator::spill( const  SuperNaiveRegisterAllocator::VirtualRegister &regToSpill, const Instruction * currentIns) {
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
                    //TODO find if memory spill is available
//                                bool memAvailable = ;
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
                                newStackPlace = LocationEntry(targetProgram_.funcLocalAlloc_[currenFunction_]++,  regLoc->ins());
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
                //spill Code
                Register lReg = Register(lastReg.getNumber());
                targetProgram_.addF_insert(LMBS tiny::t86::MOV( lReg, tiny::t86::Bp())LMBE, currentIns, writingPos_++);
                targetProgram_.addF_insert(LMBS tiny::t86::SUB( lReg, (int64_t) newPosition)LMBE, currentIns, writingPos_++);
                targetProgram_.addF_insert(LMBS tiny::t86::MOV( tiny::t86::Mem(lReg), Register(regToSpill.getNumber()))LMBE, currentIns, writingPos_++);

                releaseRegister(lastReg);
            }else if (stackset){
//spill Code
                VirtualRegister lastReg = getLastRegister(currentIns);
                Register lReg = Register(lastReg.getNumber());
                targetProgram_.addF_insert(LMBS tiny::t86::MOV( lReg, tiny::t86::Bp())LMBE, currentIns, writingPos_++);
                targetProgram_.addF_insert(LMBS tiny::t86::SUB( lReg, (int64_t)newPosition)LMBE, currentIns, writingPos_++);
                targetProgram_.addF_insert(LMBS tiny::t86::MOV( tiny::t86::Mem(lReg), Register(regToSpill.getNumber()))LMBE, currentIns, writingPos_++);

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

    void SuperNaiveRegisterAllocator::updateJumpPatchPositions(const Instruction * ins) {
        for ( size_t pos : targetProgram_.jumpPos_[ins]) {
            targetProgram_.jump_patches_[pos].first.second = Label( targetProgram_.jump_patches_[pos].first.second.address() + writingPos_);
        }
    }
    void SuperNaiveRegisterAllocator::updateCallPatchPositions(const Instruction * ins) {
        for ( auto & pos : targetProgram_.callPos_[ins]) {
//            targetProgram_.call_patches_[pos].first.second = Label( targetProgram_.call_patches_[pos].first.second.address() + writingPos_);
            targetProgram_.unpatchedFCalls_[pos].first.second = Label( targetProgram_.unpatchedFCalls_[pos].first.second.address() + writingPos_);
        }
    }

    void SuperNaiveRegisterAllocator::spillAll( const Instruction * currentIns) {
        for ( auto it= registerDescriptor_.begin(); it != registerDescriptor_.end();it++) {
            spill(it->first, currentIns);
        }

    }

    void SuperNaiveRegisterAllocator::registerMemLocation(const Store *ins, const Instruction *currentIns) {

        ins->address(); //TODO
        auto it = targetProgram_.allocLmapping_.find(ins->address());
        if(it != targetProgram_.allocLmapping_.end()){

            addressDescriptor_[ins->value()].emplace( LocationEntry( ins->address(), ins->value(), it->second));
        }


    }

    void SuperNaiveRegisterAllocator::callingConvCallerSave(const Instruction *currIns) {
        spillAll(currIns);
    }

    void SuperNaiveRegisterAllocator::callingConvCalleeRestore(const Instruction *currIns) {

    }


}