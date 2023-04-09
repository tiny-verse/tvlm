#include "RegisterAllocator.h"

#include "tvlm/tvlm/codeGeneration/FunctionalMacro.h"

namespace tvlm{

    void RegisterAllocator::visit(Instruction *ins) {
    }

    void RegisterAllocator::visit(Jump *ins) {
        writingPos_ = 0;
        spillAll(ins);

        updateJumpPatchPositions(ins);
    }

    void RegisterAllocator::visit(CondJump *ins) {

        auto virtRegs = getAllocatedVirtualRegisters(ins);
//        writingPos_= 0; .. preceding spill cannot decrease
        writingPos_ = 0;
        setupRegister(((*virtRegs)[0]), ins->condition(), ins);
        spillAll(ins);

        updateJumpPatchPositions(ins);

    }

    void RegisterAllocator::visit(Return *ins) {
        writingPos_ = 0;
        if(ins->returnValue()){
        auto virtRegs = getAllocatedVirtualRegisters(ins);
                if (ins->returnType()->registerType() == ResultType::Double){
//                writingPos_= 0;.. preceding spill cannot decrease
                setupFRegister(((*virtRegs)[0]), ins->returnValue(),ins);
            }else if (ins->returnType()->registerType() == ResultType::Integer){
//                writingPos_= 0;.. preceding spill cannot decrease
                setupRegister(((*virtRegs)[0]), ins->returnValue(),ins);
            }
        }
        forgetAllRegs(ins);
        callingConvCalleeRestore(ins);

    }


    void RegisterAllocator::visit(CallStatic *ins) {

        writingPos_ = 0;
        size_t offsetPos = 0;

        size_t tmp = 0;
        std::vector<VirtualRegisterPlaceholder> * virtRegs = nullptr;
        int regPos = 0;

        //        //args /*-> prepare values
        for (auto it = ins->args().crbegin() ; it != ins->args().crend();it++) {
            if( (*it).second->registerType() == ResultType::Integer){
                if(virtRegs == nullptr){
                    virtRegs = getAllocatedVirtualRegisters(ins);
                }
                tmp = writingPos_;
                setupRegister(((*virtRegs)[regPos++]), it->first, ins);
                offsetPos += writingPos_ -tmp;
                allocAddrRegwritePos((*it).first);
                writingPos_++;
            }else if ((*it).second->registerType() == ResultType::Double){
                if(virtRegs == nullptr){
                    virtRegs = getAllocatedVirtualRegisters(ins);
                }
                tmp = writingPos_;
                setupFRegister(((*virtRegs)[regPos++]), it->first, ins);
                offsetPos += writingPos_ -tmp;
                writingPos_++;
            }
        }
        //        //prepare return Value //*-> preparation in RA -- memory and register in RA

        tmp = writingPos_;
        callingConvCallerSave(ins);
        offsetPos += writingPos_ -tmp;

        tmp = writingPos_;
        writingPos_ = offsetPos;
        updateCallPatchPositions(ins); //writing pos needs to be only those newly added
//        setCallPatchPositions(ins);
        writingPos_ = tmp + 1;
//        tiny::t86::Label callLabel = ...

        if (ins->f()->getType()->registerType() == ResultType::Double){
            if(virtRegs == nullptr){
                virtRegs = getAllocatedVirtualRegisters(ins);
            }
            setupFRegister(((*virtRegs)[regPos++]), ins, ins);

        }else if (ins->f()->getType()->registerType() == ResultType::Integer) {
            if(virtRegs == nullptr){
                 virtRegs = getAllocatedVirtualRegisters(ins);
            }
            setupRegister(((*virtRegs)[regPos++]), ins, ins);

        }else{//void
        }
        //implement restoring? no need -- needed will be restored - lazy

    }

    void RegisterAllocator::visit(Call *ins) {
        size_t tmp = 0;
        size_t offsetPos = 0;
        writingPos_ = 0;
        auto virtRegs = getAllocatedVirtualRegisters(ins);

        int regPos = 0;

        //        //args /*-> prepare values
        for (auto it = ins->args().crbegin() ; it != ins->args().crend();it++) {
                if((*it).second->registerType() == ResultType::Integer){

                    tmp = writingPos_;
                    setupRegister(((*virtRegs)[regPos++]), it->first, ins);
                    offsetPos += writingPos_ -tmp;
                    allocAddrRegwritePos((*it).first);
                    writingPos_++;
                }else if ((*it).second->registerType() == ResultType::Double){
                    tmp = writingPos_;
                    setupFRegister(((*virtRegs)[regPos++]), it->first, ins);
                    offsetPos += writingPos_ -tmp;
                    writingPos_++;
                }
        }
        //prepare return Value //*-> preparation in RA -- memory and register in RA

        tmp = writingPos_;
        callingConvCallerSave(ins);
        offsetPos += writingPos_ -tmp;

        tmp = writingPos_;
        if (ins->retType()->registerType() == ResultType::Double){
            setupFRegister(((*virtRegs)[regPos++]), ins, ins);
        }else if (ins->retType()->registerType() == ResultType::Integer) {
            setupRegister(((*virtRegs)[regPos++]), ins, ins);
        }else{//void
        }
        setupRegister(((*virtRegs)[regPos++]), ins->f(), ins);
        offsetPos += writingPos_ -tmp;

        tmp = writingPos_;
        writingPos_ = offsetPos;
        updateCallPatchPositions(ins);
//        setCallPatchPositions(ins);

        writingPos_ = tmp + 1;

        // implement restoring? - no

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
        switch (ins->lhs()->resultType()) {
//            case ResultType::StructAddress:
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
                    default:
                        throw "[RegisterAllocator] in double BinOp is unknown operator" ;
                }
                break;
            }
            case ResultType::Void:
                throw "not implemented";
                break;
        }
    }

    void RegisterAllocator::visit(UnOp *ins) {

        auto virtRegs = getAllocatedVirtualRegisters(ins);
        switch (ins->resultType()) {

            case ResultType::Integer:{
                switch (ins->opType()) {
                    case UnOpType::UNSUB:{
                        writingPos_= 0;
                        setupRegister(((*virtRegs)[0]), ins->operand(), ins);
                        setupRegister(((*virtRegs)[1]), ins, ins);
                        return;
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
                break;
            }
            case ResultType::Double:{
                writingPos_= 0;
                setupFRegister(((*virtRegs)[0]), ins, ins);
                break;
            }
            case ResultType::Void:
                throw "ERROR cant load void as value";
                break;
        }

    }

    void RegisterAllocator::visit(AllocL *ins) {
        if(ins->amount()){
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        writingPos_= 0;
        setupRegister(((*virtRegs)[0]), ins, ins);
        }

    }

    void RegisterAllocator::visit(AllocG *ins) {
        if(ins->amount()){
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        writingPos_= 0;
        setupRegister(((*virtRegs)[0]), ins, ins);
        }
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
        std::vector<VirtualRegisterPlaceholder>* virtRegs = nullptr;
        writingPos_= 0;
        if(ins->resultType() == ResultType::Double){
            virtRegs =  getAllocatedVirtualRegisters(ins);
            setupFRegister(((*virtRegs)[0]), ins, ins);
            if(!dynamic_cast<AllocL *>(ins->address()) && !dynamic_cast<AllocG *>(ins->address())){
                setupRegister(((*virtRegs)[1]), ins->address(), ins);
            }
            return;
        }else if (ins->resultType() == ResultType::Integer ){
            virtRegs =  getAllocatedVirtualRegisters(ins);
            if (dynamic_cast<Type::Array *>(ins->type())) {
                setupRegister(((*virtRegs)[0]), ins, ins);
                setupRegister(((*virtRegs)[1]), ins->address(), ins);
                return;

            } else {
                if (dynamic_cast<AllocL *>(ins->address())) {
                    setupRegister(((*virtRegs)[0]), ins, ins);
                }else if(dynamic_cast<AllocG *>(ins->address())){
                    setupRegister(((*virtRegs)[0]), ins, ins);
                } else {
                    setupRegister(((*virtRegs)[0]), ins, ins);
                    setupRegister(((*virtRegs)[1]), ins->address(), ins);
                }
            }
            return;
        }
        throw "ERROR[RA] failed load";
    }

    void RegisterAllocator::visit(Store *ins) {
        writingPos_= 0;

        switch (ins->value()->resultType()) {
            case ResultType::Double:{
                if(dynamic_cast<AllocL*>(ins->address()) || dynamic_cast<AllocG*>(ins->address())){
                    auto *virtRegs = getAllocatedVirtualRegisters(ins);
                    setupFRegister(((*virtRegs)[0]), ins->value(), ins);
                    return;
                }else {
                    auto *virtRegs = getAllocatedVirtualRegisters(ins);
                    setupRegister(((*virtRegs)[0]), ins->address(), ins);
                    setupFRegister(((*virtRegs)[1]), ins->value(), ins);
                    return;

                }
                return;
            }

            case ResultType::Integer:{
                if(dynamic_cast<AllocL*>(ins->address()) || dynamic_cast<AllocG*>(ins->address())){
                    if (!dynamic_cast<AllocG*>(ins->value())){
                        auto *virtRegs = getAllocatedVirtualRegisters(ins);
                        setupRegister(((*virtRegs)[0]), ins->value(), ins);
                    }
                    return;
                }else {
                    auto *virtRegs = getAllocatedVirtualRegisters(ins);
                    setupRegister(((*virtRegs)[0]), ins->address(), ins);
                    if (!dynamic_cast<AllocG*>(ins->value())){
                        setupRegister(((*virtRegs)[1]), ins->value(), ins);
                    }
                    return;

                }
                return;

            }
            case ResultType::Void:{
                throw "ERROR[IS] Store: failed to store void value";
            }
        }
        throw "ERROR[IS] Store: failed to resolve value type";
    }


    void RegisterAllocator::visit(Phi *ins) {

    }

    void RegisterAllocator::visit(ElemAddrOffset *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        writingPos_= 0;
        int regpos = 0;

        if (dynamic_cast<AllocL *>(ins->base())) {
            setupRegister(((*virtRegs)[regpos++]), ins, ins);
            writingPos_++;
            writingPos_++;
            setupRegister(((*virtRegs)[regpos++]), ins->offset(), ins);

        }else if(dynamic_cast<AllocG *>(ins->base())){
            setupRegister(((*virtRegs)[regpos++]), ins, ins);
            writingPos_++;
            setupRegister(((*virtRegs)[regpos++]), ins->offset(), ins);

        }else {
            setupRegister(((*virtRegs)[regpos++]), ins, ins);
            setupRegister(((*virtRegs)[regpos++]), ins->base(), ins);
            writingPos_++;
            setupRegister(((*virtRegs)[regpos++]), ins->offset(), ins);

        }
    }

    void RegisterAllocator::visit(ElemAddrIndex *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        writingPos_= 0;
        int regpos = 0;

        if (dynamic_cast<AllocL *>(ins->base())) {
            setupRegister(((*virtRegs)[regpos++]), ins->index(), ins);
            setupRegister(((*virtRegs)[regpos++]), ins->offset(), ins);
            writingPos_++;
            setupRegister(((*virtRegs)[regpos++]), ins, ins);

        }else if(dynamic_cast<AllocG *>(ins->base())){
            setupRegister(((*virtRegs)[regpos++]), ins->index(), ins);
            setupRegister(((*virtRegs)[regpos++]), ins->offset(), ins);
            writingPos_++;
            setupRegister(((*virtRegs)[regpos++]), ins, ins);

        }else {
            setupRegister(((*virtRegs)[regpos++]), ins->index(), ins);
            setupRegister(((*virtRegs)[regpos++]), ins->offset(), ins);
            writingPos_++;
            setupRegister(((*virtRegs)[regpos++]), ins, ins);
            setupRegister(((*virtRegs)[regpos++]), ins->base(), ins);

        }

    }

    void RegisterAllocator::visit(Halt *ins) { }

    void RegisterAllocator::visit(StructAssign *ins) {
        auto virtRegs = getAllocatedVirtualRegisters(ins);
        writingPos_= 0;
        setupRegister(((*virtRegs)[0]), ins->srcVal(), ins);
        allocAddrRegwritePos(ins->srcVal());
        setupRegister(((*virtRegs)[1]), ins->dstAddr(), ins);
        allocAddrRegwritePos(ins->dstAddr());
        setupRegister(((*virtRegs)[2]), ins, ins);
        releaseRegister((*virtRegs)[2]); // don't need it anymore
    }

    void RegisterAllocator::visit(BasicBlock *bb) {

        for (auto * ins : getBBsInstructions(bb)) {
            visitChild(ins);
        }

    }

//    void RegisterAllocator::visit(BasicBlock *bb) {
//
//        auto insns =getBBsInstructions(bb);
//        int i = 0;
//        for(;i <insns.size()-1;i++ ){
//            writingPos_ = 0;
//            visitChild(insns[i]);
//        }
//        writingPos_ = 0;
//        spillAll(insns[i]);
//        visitChild(insns[i]);
//    }

    void RegisterAllocator::visit(Function *fce) {
        currenFunction_ = fce;

        for (BasicBlock * bb : getFunctionBBs(fce)){
            visitChild(bb);
        }

        //update RegAllocator state
        resetFreeRegs();

    }

    void RegisterAllocator::visit(Program *p) {
        auto globals = getProgramsGlobals(p);

        for (const auto & str : p->stringLiterals() ) {
            auto virtRegs = getAllocatedVirtualRegisters(str.first);
            writingPos_= 0;
            setupRegister(((*virtRegs)[0]), str.first, str.first);
        }

        for(auto *ins : getBBsInstructions(globals)){
            if(auto * i = dynamic_cast<const  LoadImm *>(ins)){
                if(i->resultType() == ResultType::Integer){

                    auto virtRegs = getAllocatedVirtualRegisters(ins);
                    writingPos_= 0;
                    setupRegister(((*virtRegs)[0]), ins, ins);
                }else if (i->resultType() == ResultType::Double){

                    auto virtRegs = getAllocatedVirtualRegisters(ins);
                    writingPos_= 0;
                    setupRegister(((*virtRegs)[0]), ins, ins);
                }else{
                    throw "load imm with Res Type Structure or Void not implemented";
                }
            }else if(const auto * alloc = dynamic_cast< const AllocG *>(ins)){
                continue;
            }else if(const auto * allocL = dynamic_cast< const AllocL *>(ins)){

            }else if( const auto  * store = dynamic_cast<const Store *>(ins)){

                auto virtRegs = getAllocatedVirtualRegisters(ins);
                writingPos_= 0;
                setupRegister(((*virtRegs)[0]), store->value(), ins);
            }else if( const auto  * load = dynamic_cast<const Load *>(ins)){

                auto virtRegs = getAllocatedVirtualRegisters(ins);
                writingPos_= 0;

                switch (load->resultType()) {
                    case ResultType::Double:{

                        setupFRegister(((*virtRegs)[0]), ins, ins);

                        break;
                    }
                    case ResultType::Integer:{

                        auto it = targetProgram_.globalFindAddress(load->address());
                        if(it != targetProgram_.globalEndAddress()){
                            setupRegister(((*virtRegs)[0]), ins, ins);

                            break;
                        }
                        if(dynamic_cast<Type::Array *>(load->type())){
                            setupRegister(((*virtRegs)[0]), ins, ins);
                            setupRegister(((*virtRegs)[1]), load->address(), ins);

                        }else{
                            setupRegister(((*virtRegs)[0]), ins, ins);

                        }

                        break;
                    }

                    default:
                        throw "ERROR[IS] failed load";


                }

            }else{
                throw tiny::ParserError("unknown global instruction", ins->ast()->location());
            }

        }


        for(auto & f : getProgramsFunctions(p)){
            visitChild(f.second);
        }

    }


    RegisterAllocator::RegisterAllocator(TargetProgram && tp):
            targetProgram_(std::move(tp)),
            writingPos_(0),
            currenFunction_(nullptr)
    {

    }


    std::set<LocationEntry>::iterator RegisterAllocator::findLocation(std::set<LocationEntry> & set1, Location location) {
        for (auto it  = set1.begin(); it != set1.end();it++) {
            if(it->loc() == location){
                return it;
            }
        }
        return set1.end();
    }

    bool RegisterAllocator::spill( const  RegisterAllocator::VirtualRegister &regToSpill,  Instruction * currentIns) {
        int stackOffset;
        const VirtualRegister & virtualRegister = regToSpill;
        auto regDesc = registerDescriptor_.find(regToSpill);
        size_t newPosition = 0;
        LocationEntry newStackPlace = LocationEntry(newPosition, nullptr);

        bool memset = false, stackset = false;
        if(regDesc != registerDescriptor_.end() && ! regDesc->second.empty()){ //only if in reg is something
            for (auto itRegDesc = regDesc->second.begin();itRegDesc != regDesc->second.end();) {
                //remove Register descriptor and find Stack Descriptor for each instruction in this register
                //insert data to address desc for each change
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
                            if(stackLoc == addr->second.end()) { //no old stack loc
                                 // create new stack loc

                                newStackPlace = LocationEntry(getFuncLocalAlloc(&targetProgram_)[currenFunction_]++,  regLoc->ins());
                                newPosition = newStackPlace.stackOffset();
                                assert(newStackPlace.ins() && newStackPlace.stackOffset() > 0); //is set
                                addr->second.insert(newStackPlace);

                            }
                            newPosition = newStackPlace.stackOffset();
                        }
//                        if(stackLoc == addr->second.end()) { //use old stack loc
//                            addressDescriptor_[ins].emplace(newStackPlace);
//                        }

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

                    //TODO wrong cannot spill here need to spill after definition
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
                    releaseRegister(lastReg)
                    // endof TODO cannot spill+
                    ;
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
                        if(whereTo.getNumber() != from.regIndex().getNumber()){
                            targetProgram_.addF_insert(LMBS tiny::t86::MOV( Register(whereTo.getNumber()), Register(from.regIndex().getNumber()) ) LMBE,currentIns, writingPos_++);
                        }

                        break;
                    case Location::Stack:
                        targetProgram_.addF_insert(LMBS tiny::t86::MOV( Register(whereTo.getNumber()), tiny::t86::Bp()) LMBE,currentIns, writingPos_++);
                        targetProgram_.addF_insert(LMBS tiny::t86::SUB( Register(whereTo.getNumber()), from.stackOffset()) LMBE,currentIns, writingPos_++);
                        targetProgram_.addF_insert(LMBS tiny::t86::MOV( Register(whereTo.getNumber()), tiny::t86::Mem( Register(whereTo.getNumber()))) LMBE,currentIns, writingPos_++);
                        break;
                    case Location::Memory:
                        targetProgram_.addF_insert(LMBS tiny::t86::MOV( Register(whereTo.getNumber()), tiny::t86::Mem(from.memAddress())) LMBE,currentIns, writingPos_++);
                        break;
                }
                break;
            case RegisterType::FLOAT:
                switch (from.loc()) {
                    case Location::Register:
                        if(whereTo.getNumber() != from.regIndex().getNumber()){
                            targetProgram_.addF_insert(LMBS tiny::t86::MOV( FRegister(whereTo.getNumber()), FRegister(from.regIndex().getNumber()) ) LMBE,currentIns, writingPos_++);
                        }
                        break;
                    case Location::Stack:{

                        VirtualRegister freeRegVirt = VirtualRegisterPlaceholder(RegisterType::INTEGER, 0);
                        Register freeReg = freeRegVirt.getNumber();
                        targetProgram_.addF_insert(LMBS tiny::t86::MOV( freeReg, tiny::t86::Bp()) LMBE,currentIns, writingPos_++);
                        targetProgram_.addF_insert(LMBS tiny::t86::SUB( freeReg, from.stackOffset()) LMBE,currentIns, writingPos_++);
                        targetProgram_.addF_insert(LMBS tiny::t86::MOV( FRegister(whereTo.getNumber()), tiny::t86::Mem( freeReg )) LMBE,currentIns, writingPos_++);

                        break;
                    }
                    case Location::Memory:
                        targetProgram_.addF_insert(LMBS tiny::t86::MOV( FRegister(whereTo.getNumber()), tiny::t86::Mem(from.memAddress())) LMBE,currentIns, writingPos_++);
                        break;
                }
                break;
        }
    }


    void RegisterAllocator::updateStructures(const VirtualRegisterPlaceholder & regToAssign, const Instruction * ins){
        //remove from structures the old data
        unsubscribeRegister(regToAssign);
        registerDescriptor_[regToAssign].emplace(ins);
        //add to structures
        addressDescriptor_[ins].insert(LocationEntry(regToAssign, ins));
    }

    void RegisterAllocator::unsubscribeRegister(const VirtualRegister &reg) {
        //remove from structures the old data

        for (auto  itOldIns = registerDescriptor_[reg].begin();
                itOldIns != registerDescriptor_[reg].end();
            )

        {
            auto oldIns = *itOldIns;
            auto & locations = addressDescriptor_[oldIns];
            for (auto itLoc = locations.begin(); itLoc != locations.end();) {
                auto & loc = *itLoc;
                if(loc.loc() == Location::Register && loc.regIndex() == reg){
                    //erase address descriptor pointing to Register
                    // no longer viable
                    itLoc = locations.erase(itLoc);
                }else{
                    itLoc ++;
                }
            }

            //erase instruction from register descriptor
            //no longer viable
            itOldIns =
            registerDescriptor_[reg].erase(itOldIns);
            //add to structures
        }

    }

    void RegisterAllocator::setupRegister(VirtualRegisterPlaceholder & reg, Instruction * ins,   Instruction *currentIns) {
        auto location = addressDescriptor_.find(ins);
        if(location != addressDescriptor_.end() && !location->second.empty()){
            for (auto & loc : location->second) {
                switch(loc.loc()) {
                    case Location::Register:{

                        reg.setNumber(loc.regIndex().getNumber());


                        bool global = false;
                        if(*ins->name().begin() == 'g'){global = true;}
                        ins->setAllocName(generateInstrName(reg, global));
                        return;
                    }
                    case Location::Memory: {
                        VirtualRegister freeVirtReg = getReg(ins, currentIns);
                        Register freeReg = freeVirtReg.getNumber();
                        restore(freeVirtReg, loc, currentIns);
                        updateStructures(freeVirtReg, ins);

                        reg.setNumber(freeReg.index());
                        bool global = false;
                        if(*ins->name().begin() == 'g'){global = true;}
                        ins->setAllocName(generateInstrName(reg, global));
                        return;
                    }
                    case Location::Stack: {


                        VirtualRegister freeVirtReg = getReg(ins, currentIns);
                        Register freeReg = freeVirtReg.getNumber();

                        restore(freeVirtReg, loc, currentIns);

                        updateStructures(freeVirtReg, ins);
                        reg.setNumber(freeReg.index());
                        bool global = false;
                        if(*ins->name().begin() == 'g'){global = true;}
                        ins->setAllocName(generateInstrName(reg, global));
                        return;
                    }
                }
            }
        }else{//not assigned
            auto regToAssign = getReg(ins, currentIns);
            assert(regToAssign.getNumber() <= tiny::t86::Cpu::Config::instance().registerCnt());
            reg.setNumber(regToAssign.getNumber());

            //replace in structures

            updateStructures(regToAssign, ins);

            return;
        }
    }

    void
    RegisterAllocator::setupFRegister(VirtualRegisterPlaceholder & reg, Instruction *ins, Instruction * currentIns) {
        auto location = addressDescriptor_.find(ins);
        if(location != addressDescriptor_.end()){

            for (auto & loc : location->second) {
                switch(loc.loc()) {
                    case Location::Register:
                        reg.setNumber(loc.regIndex().getNumber());
                        return;
                    case Location::Memory: {
                        VirtualRegister freeVirtReg = getFReg(ins, currentIns);
                        Register freeReg = freeVirtReg.getNumber();
                        restore(freeVirtReg, loc, currentIns);
                        addressDescriptor_[ins].emplace(freeVirtReg, ins);
                        registerDescriptor_[freeVirtReg].emplace(ins);

                        reg.setNumber(freeReg.index());
                        return;
                    }
                    case Location::Stack: {


                        VirtualRegister freeVirtReg = getFReg(ins, currentIns);
                        Register freeReg = freeVirtReg.getNumber();

                        restore(freeVirtReg, loc, currentIns);addressDescriptor_[currentIns].emplace( freeVirtReg, currentIns);
                        registerDescriptor_[freeVirtReg].emplace(currentIns);
                        reg.setNumber(freeReg.index());
                        return;
                    }
                }
            }
        }else{//not assigned
            auto regToAssign = getFReg(ins, currentIns);
            assert(regToAssign.getNumber() <= tiny::t86::Cpu::Config::instance().floatRegisterCnt());
            reg.setNumber(regToAssign.getNumber());
            addressDescriptor_[currentIns].insert(LocationEntry(regToAssign, currentIns));
            registerDescriptor_[regToAssign].emplace(currentIns);
            return;
        }
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

                            loc = address->second.begin();
                        }
                    }

                }
                loc++;
            }
        }

    }

    void RegisterAllocator::updateJumpPatchPositions(const Instruction * ins) {
        for ( size_t pos : getJumpPos(&targetProgram_)[ins]) {
            getJump_patches(&targetProgram_)[pos].first.second = Label( getJump_patches(&targetProgram_)[pos].first.second.address() + writingPos_);
        }
    }
    void RegisterAllocator::updateCallPatchPositions(const Instruction * ins) {
        for ( auto & pos : getCallPos(&targetProgram_)[ins]) {
            getUnpatchedFCalls(&targetProgram_)[pos].first.second = Label( getUnpatchedFCalls(&targetProgram_)[pos].first.second.address() + writingPos_);
        }
    }
    void RegisterAllocator::setCallPatchPositions(const Instruction * ins) {
        for ( auto & pos : getCallPos(&targetProgram_)[ins]) {
            getUnpatchedFCalls(&targetProgram_)[pos].first.second = Label(writingPos_);
        }
    }

    void RegisterAllocator::spillAll(  Instruction * currentIns) {
        for ( auto it= registerDescriptor_.begin(); it != registerDescriptor_.end();it++) {
            spill(it->first, currentIns);
        }

    }
    void RegisterAllocator::forgetAllRegs(Instruction * currentIns) {
        for ( auto it= registerDescriptor_.begin(); it != registerDescriptor_.end();it++) {
            unsubscribeRegister(it->first);
        }

    }
    void RegisterAllocator::resetFreeRegs(const Instruction * except){

        for ( auto it= registerDescriptor_.begin(); it != registerDescriptor_.end();it++) {
            releaseRegister(it->first);
        }
    }

    void RegisterAllocator::callingConvCallerSave( Instruction *currIns) {
        spillAll(currIns);
    }

    void RegisterAllocator::callingConvCalleeRestore( Instruction *currIns) {

        //----------------------------------
    }


    std::vector<VirtualRegisterPlaceholder> *
    RegisterAllocator::getAllocatedVirtualRegisters(const Instruction *ins) {
        auto it = getAllocatedRegisters(&targetProgram_).find(ins);
        if ( it != getAllocatedRegisters(&targetProgram_).end()){
            return  &(it->second);
        }else{
            throw "trying to find allocated registers for ins that was not compiled";
            if(getSelectedFInstrs(&targetProgram_).find(ins) != getSelectedFInstrs(&targetProgram_).end()){
                return  &(getAllocatedRegisters(&targetProgram_)[ins]);
            }else{
            }
        }
    }

}