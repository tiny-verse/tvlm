#include "NaiveIS.h"
#include "t86/program/helpers.h"
#include "t86/instruction.h"

#define ZeroFlag 0x02
#define SignFlag 0x01
namespace tvlm{

    void NaiveIS::visit(Instruction *ins) {}
    
    void NaiveIS::visit(Halt *ins) {
        lastIns_ = add(tiny::t86::HALT());
    }
    void NaiveIS::visit(StructAssign *ins) {
        //TODO
    }
    
    void NaiveIS::visit(Jump *ins) {
        tiny::t86::Label jmp = add(tiny::t86::JMP( tiny::t86::Label::empty()));
        future_patch_.emplace_back(jmp, ins->getTarget(1));
//        return jmp;
        lastIns_ = jmp;
    }
    
    void NaiveIS::visit(CondJump *ins) {
        auto ret = Label(lastIns_+1);

        add(tiny::t86::CMP(fillIntRegister(ins->condition()), 0));
        clearIntReg(ins->condition());
        tiny::t86::Label condJump = add(tiny::t86::JZ(tiny::t86::Label::empty()));
        tiny::t86::Label jumpToTrue = add(tiny::t86::JMP(tiny::t86::Label::empty()));
//        t86::Label jumpToFalse = add(t86::JMP(t86::Label::empty()));
//        patch(condJump, jumpToFalse);

        future_patch_.emplace_back(jumpToTrue, ins->getTarget(1));
        future_patch_.emplace_back(/*jumpToFalse*/condJump, ins->getTarget(0));

//        return ret;
        lastIns_ = ret;
    }
    
    void NaiveIS::visit(Return *ins) {
        auto ret = Label(lastIns_+1);

        add(tiny::t86::MOV(tiny::t86::Reg(0 /*or Somwhere on stack*/  ), fillIntRegister(ins->returnValue())));
        clearIntReg(ins->returnValue());
        add(tiny::t86::MOV(tiny::t86::Sp(), tiny::t86::Bp()));
        add(tiny::t86::POP(tiny::t86::Bp()));
        add(tiny::t86::RET());

        lastIns_= ret;
    }
    
    void NaiveIS::visit(CallStatic *ins) {

        auto ret = Label(lastIns_+1);

        //spill everything
        spillAllReg();
        //args
        for( auto it = ins->args().crbegin() ; it != ins->args().crend();it++){
            //TODO struct allocation, doubles
            add(tiny::t86::PUSH(fillIntRegister(*it)));
            clearIntReg(*it);

        }
        clearAllReg();
        //call
        tiny::t86::Label callLabel = add(tiny::t86::CALL{tiny::t86::Label::empty()});
        add(tiny::t86::MOV(fillIntRegister(ins),tiny::t86::Reg(0)));
        add(tiny::t86::ADD(tiny::t86::Sp(), ins->args().size()));//clear arguments; need?

        unpatchedCalls_.emplace_back(callLabel, ins->f()->name());
//        return ret;
        lastIns_ = ret;
    }

    void NaiveIS::visit(Call *ins) {
        auto ret = Label(lastIns_+1);

        //spilll everything
        spillAllReg();

        //parameters
        for( auto it = ins->args().crbegin() ; it != ins->args().crend();it++){
            add(tiny::t86::PUSH(fillIntRegister(*it)));
            clearIntReg(*it);

        }

        clearAllReg();
        //call itself
        add(tiny::t86::CALL(fillIntRegister(ins->f())));
        clearIntReg(ins->f());

        add(tiny::t86::MOV(fillIntRegister(ins),tiny::t86::Reg(0)));
        add(tiny::t86::ADD(tiny::t86::Sp(), ins->args().size()));//clear arguments; need?

//        return ret;
        lastIns_ = ret;
    }

    void NaiveIS::visit(BinOp *ins) {
        auto ret = Label(lastIns_+1);

        switch (ins->resultType()) {
            case ResultType::Integer:{

            auto lhsreg = fillIntRegister(ins->lhs());
            auto rhsreg = fillIntRegister(ins->rhs());
            switch (ins->opType()) {
                case BinOpType::ADD:
                    add(tiny::t86::ADD(lhsreg, rhsreg));
                    //        add(MOV(fillIntRegister(instr), fillIntRegister(instr->lhs())));
                    break;
                case BinOpType::SUB:
                    add(tiny::t86::SUB(lhsreg, rhsreg));
                    break;
                case BinOpType::MOD:
                    add(tiny::t86::MOD(lhsreg, rhsreg));
                    break;
                case BinOpType::MUL:
                    add(tiny::t86::MUL(lhsreg, rhsreg));
                    break;
                case BinOpType::DIV:
                    add(tiny::t86::DIV(lhsreg, rhsreg));
                    break;

                case BinOpType::AND:
                    add(tiny::t86::AND(lhsreg, rhsreg));
                    break;
                case BinOpType::OR:
                    add(tiny::t86::OR(lhsreg, rhsreg));
                    break;
                case BinOpType::XOR:
                    add(tiny::t86::XOR(lhsreg, rhsreg));
                    break;

                case BinOpType::LSH:
                    add(tiny::t86::LSH(lhsreg, rhsreg));
                    break;
                case BinOpType::RSH:
                    add(tiny::t86::RSH(lhsreg, rhsreg));
                    break;

                case BinOpType::NEQ:
                    add(tiny::t86::SUB(lhsreg,rhsreg));
                    break;
                case BinOpType::EQ:
                    add(tiny::t86::SUB(lhsreg,rhsreg));
                    add(tiny::t86::MOV(lhsreg, tiny::t86::Flags()));
                    add(tiny::t86::AND(lhsreg, ZeroFlag));//ZeroFlag
                    break;
                case BinOpType::LTE:
                    add(tiny::t86::SUB(rhsreg, lhsreg));
                    add(tiny::t86::MOV(lhsreg, tiny::t86::Flags()));
                    add(tiny::t86::AND(lhsreg, SignFlag));//SignFlag
                    add(tiny::t86::NOT(lhsreg));
                    break;
                case BinOpType::LT:
                    add(tiny::t86::SUB(lhsreg, rhsreg));
                    add(tiny::t86::MOV(lhsreg, tiny::t86::Flags()));
                    add(tiny::t86::AND(lhsreg, SignFlag));//SignFlag
                    break;
                case BinOpType::GT:
                    add(tiny::t86::SUB(rhsreg, lhsreg));
                    add(tiny::t86::MOV(lhsreg, tiny::t86::Flags()));
                    add(tiny::t86::AND(lhsreg, SignFlag));//SignFlag
                    break;
                case BinOpType::GTE:
                    add(tiny::t86::SUB(lhsreg, rhsreg));
                    add(tiny::t86::MOV(lhsreg, tiny::t86::Flags()));
                    add(tiny::t86::AND(lhsreg, SignFlag));//SignFlag
                    add(tiny::t86::NOT(lhsreg));
                    break;
            };
//                regAllocator->alloc_regs_[getIntRegister(ins->lhs()).index()] = ins;
                regAllocator->replaceInt(ins->lhs(), ins);
                clearIntReg(ins->rhs());
                lastIns_ = ret;//        return ret;
                return;


                break;
            }
            case ResultType::Double:{

                throw "not implmented";
                break;
            }
            case ResultType::Void:
                throw "not implmented";
                break;
        }

        add(tiny::t86::SUB(fillIntRegister(ins->lhs()), fillIntRegister(ins->rhs())));

        add(tiny::t86::MOV(fillIntRegister(ins->rhs()), tiny::t86::Flags()));
        add(tiny::t86::AND(fillIntRegister(ins->rhs()),SignFlag));//SignFlag
//        regAllocator->alloc_regs_[getIntRegister(ins->rhs()).index()] = ins;
        regAllocator->replaceInt(ins->rhs(), ins);
        clearIntReg(ins->rhs());
        lastIns_ = ret;//        return ret;
    }
    
    void NaiveIS::visit(UnOp *ins) {
        auto ret = Label(lastIns_+1);
        auto reg =
        fillIntRegister(ins->operand());
        switch (ins->opType()) {
            case UnOpType::NOT:
                add(tiny::t86::NOT(reg));
                break;
            case UnOpType::UNSUB:
                add(tiny::t86::SUB(0 , reg));
                break;
            case UnOpType::INC:
                add(tiny::t86::INC( reg));
                break;
            case UnOpType::DEC:
                add(tiny::t86::DEC( reg));
                break;
        }
//        regAllocator->alloc_regs_[getIntRegister(ins->operand()).index()] = ins;
        regAllocator->replaceInt(ins->operand(), ins);

//        return ret;
        lastIns_ = ret;
    }
    
    void NaiveIS::visit(LoadImm *ins) {
        auto ret = lastIns_ +1;

        switch (ins->resultType()) {
            case ResultType::Integer:{

                int64_t value = ins->valueInt();
                if(!instructionToEmplace.empty() ){
                    auto it = instructionToEmplace.find(ins);
                    if( it!= instructionToEmplace.end()){
                        auto * ins = dynamic_cast<LoadImm * >(it->second);
                        value = ins->valueInt();
                        instructionToEmplace.erase(it);
                        delete ins;
                    }
                }
                add( tiny::t86::MOV(fillIntRegister(ins), value ));
                break;
            }
            case ResultType::Double:
                add(tiny::t86::MOV(fillFloatRegister(ins), ins->valueFloat() ));
                break;
            case ResultType::Void:
                break;
        }

    }
    
    void NaiveIS::visit(ArgAddr *ins) {
        //TODO double args ? struct args?
        auto ret = Label(lastIns_+1);
        auto reg = fillIntRegister(ins);
        add(tiny::t86::MOV(reg, tiny::t86::Bp() ));
        add(tiny::t86::ADD(reg, (int64_t)ins->index()+2 ));

//        return ret;
        lastIns_ = ret;
    }
    
    void NaiveIS::visit(AllocL *ins) {
        Label ret = lastIns_ +1;
        Register reg = fillIntRegister(ins);
        functionLocalAllocSize +=ins->size()/4;//TODO make size decomposition according target memory laout
        // already allocated, now just find addr for this allocation
                add(tiny::t86::MOV(reg ,tiny::t86::Bp()));
        add(tiny::t86::SUB(reg, (int64_t)functionLocalAllocSize));
//        return ret;
        lastIns_ = ret;
    }

    void NaiveIS::visit(AllocG *ins) {
        throw "allocG?? done in globals";
        DataLabel glob = pb_.addData((int64_t)(ins->size()/4));
        int64_t addr = (int64_t)glob;
        globalPointer_ += 1;
        Label ret = add( tiny::t86::MOV(fillIntRegister(ins) , addr));

        std::cerr << "label ret val: " << ret.address() << "| and addr val:  " << addr << std::endl;
//        return ret;
        lastIns_ = ret;
    }


    void NaiveIS::visit(Copy *ins) {
        auto ret = Label(lastIns_+1);
        add(tiny::t86::MOV(fillIntRegister(ins), fillIntRegister(ins->src()) ));

//        return ret;
        lastIns_ = ret;
    }

    void NaiveIS::visit(Extend *ins) {
        auto ret = Label(lastIns_+1);
        add(tiny::t86::EXT(fillFloatRegister(ins), fillIntRegister(ins->src()) ));
//        return ret;
        clearReg(ins->src());
        lastIns_ = ret;
    }

    void NaiveIS::visit(Truncate *ins) {
        auto ret = Label(lastIns_+1);
        add(tiny::t86::NRW(fillIntRegister(ins), fillFloatRegister(ins->src()) ));
//        return ret;
        clearReg(ins->src());
        lastIns_ = ret;
    }


    void NaiveIS::visit(PutChar *ins) {
        auto ret = Label(lastIns_+1);
        add(tiny::t86::PUTCHAR(fillIntRegister(ins->src()) ));
        clearIntReg(ins->src());
        lastIns_ = ret;
    }

    void NaiveIS::visit(GetChar *ins) {
        auto ret = Label(lastIns_+1);
        add(tiny::t86::GETCHAR( fillIntRegister(ins) ));
        lastIns_ = ret;
    }

    void NaiveIS::visit(Load *ins) {
        auto ret = Label(lastIns_+1);

        auto it = globalTable_.find(ins->address());
        if(it != globalTable_.end()){
            add(tiny::t86::MOV(fillIntRegister(ins), (int64_t)it->second));
            lastIns_ = ret; //return ret;
            return;
        }

        add(tiny::t86::MOV(fillIntRegister(ins), tiny::t86::Mem(fillIntRegister(ins->address()))));
        lastIns_ = ret; //return ret;
        return;
    }

    void NaiveIS::visit(Store *ins) {
        auto ret = Label(lastIns_+1);
        add(tiny::t86::MOV(Mem(fillIntRegister(ins->address())), fillIntRegister(ins->value())));
        clearIntReg(ins->value());
//        return ret;
        lastIns_ = ret;
    }

    void NaiveIS::visit(Phi *ins) {
        regAllocator->registerPhi(ins);
//        return Label(lastIns_ + 1);
        lastIns_ = lastIns_ + 1;
    }

    void NaiveIS::visit(ElemAddrOffset *ins) {
        auto ret = Label(lastIns_+1);

        add(tiny::t86::ADD(fillIntRegister(ins->base()),
                fillIntRegister(ins->offset())));
//        regAllocator->alloc_regs_[getIntRegister(ins->base()).index()] = ins;
        regAllocator->replaceInt(ins->base(), ins);
        clearIntReg(ins->offset());
//        return ret;
        lastIns_ = ret;
    }

    void NaiveIS::visit(ElemAddrIndex *ins) {
        auto ret = Label(lastIns_+1);

        add(tiny::t86::ADD(fillIntRegister(ins->index()),
                           fillIntRegister(ins->offset())));
        clearIntReg(ins->offset());
        add(tiny::t86::ADD(fillIntRegister(ins->base()),
                           fillIntRegister(ins->index())));
//        regAllocator->alloc_regs_[getIntRegister(ins->base()).index()] = ins;
        regAllocator->replaceInt(ins->base(), ins);
        clearIntReg(ins->index());
//        return ret;
        lastIns_ = ret;
    }
    
    void NaiveIS::visit(BasicBlock * bb) {
        tiny::t86::Label ret = pb_.currentLabel(); //next from last Instruction Label
        compiledBB_.emplace(bb, ret);
        auto insns = getBBsInstructions(bb);
        int64_t len = (int64_t) insns.size()-1;
        if(len == -1){
            lastIns_ = ret; // return ret; // empty BB;
            return;
        }
        for(int64_t i = 0; i < len ;i++){
            visit(insns[i]);
            tiny::t86::Label tmp = lastIns_;
            compiled_.emplace(insns[i], tmp);
        }
        auto & last = insns[len];
        spillAllReg();
        visit(last);
        clearAllReg();
        lastIns_ = ret; // return ret;
    }
    
    void NaiveIS::visit(Function * fnc) {
        functionLocalAllocSize = 0;
        tiny::t86::Label ret =
                add(tiny::t86::PUSH(tiny::t86::Bp()));
        add(tiny::t86::MOV(tiny::t86::Bp(), tiny::t86::Sp()));
        Label toPatchStack = add(tiny::t86::SUB(tiny::t86::Sp(), 0));
        for (const auto & bb : getFunctionBBs(fnc)) {
            visit(bb);
        }
        // TODO replace(toPatchStack, tiny::t86::SUB(tiny::t86::Sp(), (int64_t)functionLocalAllocSize ));

        lastIns_ = ret; //return ret;
    }
    
    void NaiveIS::visit(Program * p) {
    
//        Label globals = visitChild(getProgramsGlobals(p));
        makeGlobalTable(getProgramsGlobals(p));
        Label callMain = add(nullptr, tiny::t86::CALL{Label::empty()});
        for ( auto & f : getProgramsFunctions(p)) {
            Label fncLabel = visitChild(f.second);
            functionTable_.emplace(f.first, fncLabel);
        }

        Label main = functionTable_.find(Symbol("main"))->second;
        pb_.patch(callMain, main);
        add(nullptr, tiny::t86::HALT{});



//        regAllocator ->alloc_regs_.resize(t86::Cpu::Config::instance().registerCnt());

        //===================================GLOBALS=====================================
        BasicBlock functionGlobals;
//        makeGlobalTable(irb->globals_.get());
        auto globs = getProgramsGlobals(p);
        makeGlobalTable(globs);
        for (const auto & str : p->stringLiterals() ) {
            tiny::t86::DataLabel data = pb_.addData(str.first);
            add(tiny::t86::MOV(fillIntRegister(str.second), data ));
        }

        tiny::t86::Label start = add(tiny::t86::JMP(tiny::t86::Label::empty()));

        //===================================FUNCTIONS=====================================
        for (auto & i : p->functions()){
            visit(i.second.get());
            Label fncLabel = lastIns_;
            addFunction(i.first, fncLabel);
        }

        //===================================MEM INTERCEPT=====================================
        for(auto & f : functionTable_){
            //TODO Instruction * fnc_addr = irb->env_->getVariableAddress(f.first);
            // instructionToEmplace.emplace(fnc_addr, new LoadImm((int64_t)f.second.address(), nullptr));
        }

        clearAllReg();
        tiny::t86::Label prolog = //t86::Label(lastInstruction_index +1);
                compileGlobalTable(globs);

        add(tiny::t86::CALL(functionAddr("main"))) ;

        //===================================END of MEM INTERCEPT=====================================
        add(tiny::t86::HALT());
        pb_.patch(start, prolog);
        //===================================PATCHING JUMPS=====================================

        for (const auto & toPatch: future_patch_) {
            auto it = compiledBB_.find(toPatch.second);
            if(it == compiledBB_.end()){
                throw "wat now? - I can jump on invalid BB....";
            }
            pb_.patch(toPatch.first, it->second );

        }
        //===================================PATCHING Calls=====================================
        for( auto & toPatch : unpatchedCalls_){
            auto it = functionTable_.find(toPatch.second);
            if(it == functionTable_.end()){
                throw "WTF failed patching calls";
            }
            pb_.patch(toPatch.first,it->second);
            std::cerr << "patching call at " << toPatch.first << " with " << it->second << std::endl;
        }


    }

    tiny::t86::Program NaiveIS::translate(Program &prog) {
        //TODO
        NaiveIS v;
        v.visit( &prog);
        tiny::t86::Program rawProg = v.pb_.program();
        std::vector<tiny::t86::Instruction*> instrs = rawProg.moveInstructions();
        int line = 0;
        for(const tiny::t86::Instruction * i : instrs){
            std::cerr << tiny::color::blue << line++ << ": " << tiny::color::green << i->toString() << std::endl;
        }


        return {std::move(instrs), rawProg.data()};
//            return v.pb_.program();
    }

    void NaiveIS::makeGlobalTable(BasicBlock * globals) {

        for(auto & ins : getBBsInstructions(globals)){
            if(auto i = dynamic_cast<LoadImm *>(ins)){
                globalTable_.emplace(ins, i->valueInt());
            }else if(dynamic_cast<AllocG *>(ins)){
                tiny::t86::DataLabel label = pb_.addData(0);
                globalTable_.emplace(ins, label);
            }else if( auto store = dynamic_cast<Store *>(ins)){
                auto val = globalTable_.find(store->value());
                if(val == globalTable_.end()){
                    throw "uninitialized Global?";
                }
                globalTable_[store->address()] = val->second;
            }else{
                throw "unknown global instruction - make";
            }

        }

    }

    NaiveIS::Label NaiveIS::compileGlobalTable(BasicBlock *globals) {
        auto ret = Label(lastIns_+1);
        for(auto & ins : getBBsInstructions(globals)){
            if(auto loadImm= dynamic_cast<LoadImm *>(ins)){
                globalTable_.emplace(ins, loadImm->valueInt());
            }else if(dynamic_cast<AllocG *>(ins)){
                DataLabel label = pb_.addData(0);
                globalTable_.emplace(ins, label);
            }else if(auto store = dynamic_cast<Store *>(ins)){
                uint64_t value = globalTable_.find(store->value())->second;
                uint64_t address = globalTable_.find(store->address())->second;
                add(tiny::t86::MOV( tiny::t86::Reg(0), (int64_t)address));
                add(tiny::t86::MOV(Mem(tiny::t86::Reg(0)), (int64_t)value));
            }else{
                throw "unknown global instruction - compile";
            }

        }
        return ret;
    }

    NaiveIS::NaiveIS(): pb_(tiny::t86::ProgramBuilder()), lastIns_(Label::empty()),
                        regAllocator(new NaiveRegisterAllocator()){

    }

  uint64_t NaiveIS::functionAddr(const std::string & name) const{

        return functionTable_.find(Symbol(name))->second.address();
    }

    void NaiveIS::addFunction(Symbol name, NaiveIS::Label instr) {

        functionTable_.emplace(name, instr.address());
    }


}
