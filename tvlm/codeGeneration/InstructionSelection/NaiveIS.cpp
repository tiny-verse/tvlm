#include "NaiveIS.h"
#include "t86/program/helpers.h"
#include "t86/instruction.h"
#include "tvlm/il/il.h"
#include "tvlm/il/il_builder.h"
#include "tvlm/il/il_insns.h"

#include "tvlm/analysis/liveness_analysis.h"

#define ZeroFlag 0x02
#define SignFlag 0x01
namespace tvlm{

    void NaiveIS::visit(Halt *ins) {
        lastIns_ = add(tiny::t86::HALT(), ins);
    }
    void NaiveIS::visit(StructAssign *ins) {
        auto ret = pb_.currentLabel();
        copyStruct(getReg(ins->srcVal()), ins->type(), getReg(ins->dstAddr()), ins);
        lastIns_ = ret;
    }

    void NaiveIS::visit(Jump *ins) {
        tiny::t86::Label jmp = add(tiny::t86::JMP( tiny::t86::Label::empty()), ins);
        future_patch_.emplace_back(jmp, ins->getTarget(1));
        lastIns_ = jmp;//        return jmp;
    }

    void NaiveIS::visit(CondJump *ins) {
        auto ret = pb_.currentLabel();

        add(tiny::t86::CMP(getReg(ins->condition()), 0), ins);
//        clearIntReg(ins->condition());
        tiny::t86::Label condJump = add(tiny::t86::JZ(tiny::t86::Label::empty()) ,ins);
        tiny::t86::Label jumpToTrue = add(tiny::t86::JMP(tiny::t86::Label::empty()), ins);
        future_patch_.emplace_back(jumpToTrue, ins->getTarget(1));
        future_patch_.emplace_back(/*jumpToFalse*/condJump, ins->getTarget(0));
        lastIns_ = ret;//        return ret;
    }
    
    void NaiveIS::visit(Return *ins) {
        auto ret = pb_.currentLabel();
        if(!ins->returnValue()){
             // void
            add(tiny::t86::MOV(tiny::t86::Sp(), tiny::t86::Bp()), ins);
            add(tiny::t86::POP(tiny::t86::Bp()), ins);
            add(tiny::t86::RET(), ins);
            return;
        }

        if(ins->returnType()->registerType() == ResultType::StructAddress){
            auto reg = getReg(ins);
            add(tiny::t86::MOV(reg,tiny::t86::Bp() ), ins);
            add(tiny::t86::ADD(reg, 2 ), ins);
            copyStruct(getReg(ins->returnValue()), ins->returnType(), tiny::t86::Reg(0), ins);

//            clearIntReg(ins->returnValue());
            add(tiny::t86::MOV(tiny::t86::Sp(), tiny::t86::Bp()), ins);
            add(tiny::t86::POP(tiny::t86::Bp()), ins);

            add(tiny::t86::RET(), ins);

        }else if (ins->returnType()->registerType() == ResultType::Double){
            add(tiny::t86::MOV(tiny::t86::FReg(0 /*or Somwhere on stack*/  ), getFReg(ins->returnValue())), ins);
            add(tiny::t86::MOV(tiny::t86::Sp(), tiny::t86::Bp()), ins);
            add(tiny::t86::POP(tiny::t86::Bp()), ins);


            add(tiny::t86::RET(), ins);
        }else if (ins->returnType()->registerType() == ResultType::Integer){

            add(tiny::t86::MOV(tiny::t86::Reg(0 /*or Somwhere on stack*/  ), getReg(ins->returnValue())), ins);
            add(tiny::t86::MOV(tiny::t86::Sp(), tiny::t86::Bp()), ins);
            add(tiny::t86::POP(tiny::t86::Bp()), ins);

            add(tiny::t86::RET(), ins);
        }else{ // void
            add(tiny::t86::MOV(tiny::t86::Sp(), tiny::t86::Bp()), ins);
            add(tiny::t86::POP(tiny::t86::Bp()), ins);
            add(tiny::t86::RET(), ins);

        }
//        regAllocator->clearAllReg();
        lastIns_= ret;
    }
    
    void NaiveIS::visit(CallStatic *ins) {
        auto ret = pb_.currentLabel();
        //spill everything
//        regAllocator->spillAllReg();
        //args
        for( auto it = ins->args().crbegin() ; it != ins->args().crend();it++){
            if((*it).second->registerType() == ResultType::StructAddress) {
                allocateStructArg(it->second, it->first);
            }else if((*it).second->registerType() == ResultType::Integer){
                add(tiny::t86::PUSH(getReg(it->first)), ins);
//                clearIntReg(it->first);
            }else if ((*it).second->registerType() == ResultType::Double){
                add(tiny::t86::FPUSH(getFReg(it->first)), ins);
//                clearFloatReg(it->first);
            }
        }

        //prepare return Value
        ins->f()->getType()->registerType() == ResultType::StructAddress ?
            prepareReturnValue(ins->f()->getType()->size(), ins):
            prepareReturnValue(0, ins);

//        regAllocator->spillCallReg();
        //call
        tiny::t86::Label callLabel = add(tiny::t86::CALL{tiny::t86::Label::empty()}, ins);
        if(ins->resultType() == ResultType::Double){
            add(tiny::t86::MOV(getFReg(ins), tiny::t86::FReg(0)), ins);
        }else if (ins->resultType() == ResultType::Integer){
            add(tiny::t86::MOV(getReg(ins), tiny::t86::Reg(0)), ins);
        } else if(ins->resultType() == ResultType::Void){

        }

        //CountArgSize;
        int argSize  = 0;
        for (auto & a :ins->args()) {
            if(a.second->registerType() == ResultType:: Double){
                argSize +=1;
            }else{
                argSize ++;
            }
        }

        add(tiny::t86::ADD(tiny::t86::Sp(), argSize), ins);

        unpatchedCalls_.emplace_back(callLabel, ins->f()->name());
        lastIns_ = ret; //return ret;
    }

    void NaiveIS::visit(Call *ins) {
        auto ret = pb_.currentLabel();
        //spill everything
//        regAllocator->spillAllReg();

        //------------------args-------------------
        for( auto it = ins->args().crbegin() ; it != ins->args().crend();it++){
            if((*it).second->registerType() == ResultType::StructAddress) {
//                regAllocator->
                allocateStructArg(it->second, it->first);
            }else if((*it).second->registerType() == ResultType::Integer){
                add(tiny::t86::PUSH(getReg(it->first)), ins);
//                clearIntReg(it->first);
            }else if ((*it).second->registerType() == ResultType::Double){
                add(tiny::t86::FPUSH(getFReg(it->first)), ins);
//                clearFloatReg(it->first);
            }
        }
        //------------------args------------------

        //prepare return Value
        ins->retType()->registerType() == ResultType::StructAddress ?
//            regAllocator->
            prepareReturnValue(ins->retType()->size(), ins) :
//            regAllocator->
            prepareReturnValue(0, ins);

//        regAllocator->spillCallReg();
        //call
        tiny::t86::Label callLabel = add(tiny::t86::CALL{getReg(ins->f())}, ins);
        if(ins->resultType() == ResultType::Double){
            add(tiny::t86::MOV(getFReg(ins), tiny::t86::FReg(0)), ins);
        }else if (ins->resultType() == ResultType::Integer){
            add(tiny::t86::MOV(getReg(ins), tiny::t86::Reg(0)), ins);
        } else if(ins->resultType() == ResultType::Void){

        }


        int argSize  = 0;
        for (auto & a :ins->args()) {  //CountArgSize;
            if(a.second->registerType() == ResultType:: Double){
                argSize +=2;
            }else{
                argSize ++;
            }
        }

        add(tiny::t86::ADD(tiny::t86::Sp(), argSize), ins);

//        unpatchedCalls_.emplace_back(callLabel, ins->f()->name());
        lastIns_ = ret; //return ret;

    }

    void NaiveIS::visit(BinOp *ins) {
        auto ret = pb_.currentLabel();

        switch (ins->resultType()) {
            case ResultType::StructAddress:
            case ResultType::Integer:{

            auto lhsreg = getReg(ins->lhs());
            auto rhsreg = getReg(ins->rhs());
            switch (ins->opType()) {
                case BinOpType::ADD:
                    add(tiny::t86::ADD(lhsreg, rhsreg), ins);
                    break;
                case BinOpType::SUB:
                    add(tiny::t86::SUB(lhsreg, rhsreg), ins);
                    break;
                case BinOpType::MOD:
                    add(tiny::t86::MOD(lhsreg, rhsreg), ins);
                    break;
                case BinOpType::MUL:
                    add(tiny::t86::MUL(lhsreg, rhsreg), ins);
                    break;
                case BinOpType::DIV:
                    add(tiny::t86::DIV(lhsreg, rhsreg), ins);
                    break;

                case BinOpType::AND:
                    add(tiny::t86::AND(lhsreg, rhsreg), ins);
                    break;
                case BinOpType::OR:
                    add(tiny::t86::OR(lhsreg, rhsreg), ins);
                    break;
                case BinOpType::XOR:
                    add(tiny::t86::XOR(lhsreg, rhsreg), ins);
                    break;

                case BinOpType::LSH:
                    add(tiny::t86::LSH(lhsreg, rhsreg), ins);
                    break;
                case BinOpType::RSH:
                    add(tiny::t86::RSH(lhsreg, rhsreg), ins);
                    break;

                case BinOpType::NEQ:
                    add(tiny::t86::SUB(lhsreg,rhsreg), ins);
                    break;
                case BinOpType::EQ:
                    add(tiny::t86::SUB(lhsreg,rhsreg), ins);
                    add(tiny::t86::MOV(lhsreg, tiny::t86::Flags()), ins);
                    add(tiny::t86::AND(lhsreg, ZeroFlag), ins);//ZeroFlag
                    break;
                case BinOpType::LTE:
                    add(tiny::t86::SUB(rhsreg, lhsreg), ins);
                    add(tiny::t86::MOV(lhsreg, tiny::t86::Flags()), ins);
                    add(tiny::t86::AND(lhsreg, SignFlag), ins);//SignFlag
                    add(tiny::t86::NOT(lhsreg), ins);
                    break;
                case BinOpType::LT:
                    add(tiny::t86::SUB(lhsreg, rhsreg), ins);
                    add(tiny::t86::MOV(lhsreg, tiny::t86::Flags()), ins), ins;
                    add(tiny::t86::AND(lhsreg, SignFlag), ins);//SignFlag
                    break;
                case BinOpType::GT:
                    add(tiny::t86::SUB(rhsreg, lhsreg), ins);
                    add(tiny::t86::MOV(lhsreg, tiny::t86::Flags()), ins);
                    add(tiny::t86::AND(lhsreg, SignFlag), ins);//SignFlag
                    break;
                case BinOpType::GTE:
                    add(tiny::t86::SUB(lhsreg, rhsreg), ins);
                    add(tiny::t86::MOV(lhsreg, tiny::t86::Flags()), ins);
                    add(tiny::t86::AND(lhsreg, SignFlag), ins);//SignFlag
                    add(tiny::t86::NOT(lhsreg), ins);
                    break;
            };

//            regAllocator->replaceInt(ins->lhs(), ins);
            replaceIntReg(ins->lhs(), ins);
            //clearIntReg(ins->rhs());
            lastIns_ = ret;//        return ret;
            return;
            }
            case ResultType::Double:{
                auto lhsreg = getFReg(ins->lhs());
                auto rhsreg = getFReg(ins->rhs());

                switch (ins->opType()) {
                    case BinOpType::ADD:
                        add(tiny::t86::FADD(lhsreg, rhsreg), ins);
                        break;
                    case BinOpType::SUB:
                        add(tiny::t86::FSUB(lhsreg, rhsreg), ins);
                        break;
                    case BinOpType::MUL:
                        add(tiny::t86::FMUL(lhsreg, rhsreg), ins);
                        break;
                    case BinOpType::DIV:
                        add(tiny::t86::FDIV(lhsreg, rhsreg), ins);
                        break;
                    case BinOpType::NEQ:{
                        add(tiny::t86::FSUB(lhsreg,rhsreg), ins);
                        //clearFloatReg(ins->lhs());
                        //clearFloatReg(ins->rhs());
                        lastIns_ = ret;//        return ret;
                        return;
                    }
                    case BinOpType::EQ:{
                        Register tmpreg = getReg(ins);
                        add(tiny::t86::FSUB(lhsreg,rhsreg), ins);
                        add(tiny::t86::MOV(tmpreg, tiny::t86::Flags()), ins);
                        add(tiny::t86::AND(tmpreg, ZeroFlag), ins);//ZeroFlag
                        //clearFloatReg(ins->rhs());
                        //clearFloatReg(ins->lhs());
                        lastIns_ = ret;//        return ret;
                        return;
                    }
                    case BinOpType::LTE:{
                        Register tmpreg = getReg(ins);

                        add(tiny::t86::FSUB(rhsreg, lhsreg), ins);
                        add(tiny::t86::MOV(tmpreg, tiny::t86::Flags()), ins);
                        add(tiny::t86::AND(tmpreg, SignFlag), ins);//SignFlag
                        add(tiny::t86::NOT(tmpreg), ins);

                        //clearFloatReg(ins->rhs());
                        //clearFloatReg(ins->lhs());
                        lastIns_ = ret;//        return ret;
                        return;
                    }
                    case BinOpType::LT:{
                        Register tmpreg = getReg(ins);
                        add(tiny::t86::FSUB(lhsreg, rhsreg), ins);
                        add(tiny::t86::MOV(tmpreg, tiny::t86::Flags()), ins);
                        add(tiny::t86::AND(tmpreg, SignFlag), ins);//SignFlag

                        //clearFloatReg(ins->rhs());
                        //clearFloatReg(ins->lhs());
                        lastIns_ = ret;//        return ret;
                        return;
                    }
                case BinOpType::GT:{
                        Register tmpreg = getReg(ins);
                        add(tiny::t86::FSUB(rhsreg, lhsreg), ins);
                        add(tiny::t86::MOV(tmpreg, tiny::t86::Flags()), ins);
                        add(tiny::t86::AND(tmpreg, SignFlag), ins);//SignFlag

                        //clearFloatReg(ins->rhs());
                        //clearFloatReg(ins->lhs());
                        lastIns_ = ret;//        return ret;
                        return;
                    }
                case BinOpType::GTE:{
                        Register tmpreg = getReg(ins);
                    add(tiny::t86::FSUB(lhsreg, rhsreg), ins);
                    add(tiny::t86::MOV(tmpreg, tiny::t86::Flags()), ins);
                    add(tiny::t86::AND(tmpreg, SignFlag), ins);//SignFlag
                    add(tiny::t86::NOT(tmpreg), ins);

                    //clearFloatReg(ins->rhs());
                    //clearFloatReg(ins->lhs());
                    lastIns_ = ret;//        return ret;
                    return;
                }
                default:
                    throw "not implemented or not supported";
                };

//                regAllocator->replaceFloat(ins->lhs(), ins);
                replaceFloatReg(ins->lhs(), ins);
                //clearFloatReg(ins->rhs());
                lastIns_ = ret;//        return ret;
                return;

                break;
            }
            case ResultType::Void:
                throw "not implemented";
                break;
        }

        add(tiny::t86::SUB(getReg(ins->lhs()), getReg(ins->rhs())), ins);

        add(tiny::t86::MOV(getReg(ins->rhs()), tiny::t86::Flags()), ins);
        add(tiny::t86::AND(getReg(ins->rhs()), SignFlag), ins);//SignFlag

//        regAllocator->replaceInt(ins->rhs(), ins);
        replaceIntReg(ins->rhs(), ins);
        //clearIntReg(ins->rhs());
        lastIns_ = ret;//        return ret;
    }
    
    void NaiveIS::visit(UnOp *ins) {
        auto ret = pb_.currentLabel();
        switch (ins->resultType()) {

            case ResultType::Integer:
            case ResultType::StructAddress:{

                auto reg =
                        getReg(ins->operand());
                switch (ins->opType()) {
                    case UnOpType::NOT:
                        add(tiny::t86::NOT(reg), ins);
                        break;
                    case UnOpType::UNSUB:
                        add(tiny::t86::SUB(0 , reg), ins);
                        break;
                    case UnOpType::INC:
                        add(tiny::t86::INC( reg), ins);
                        break;
                    case UnOpType::DEC:
                        add(tiny::t86::DEC( reg), ins);
                        break;
                }
//                regAllocator->replaceInt(ins->operand(), ins);
                replaceIntReg(ins->operand(), ins);

                lastIns_ = ret;//        return ret;
                return;
            }
            case ResultType::Double:{
                auto reg = getFReg(ins->operand());
                switch (ins->opType()) {
                    case UnOpType::UNSUB:
                        add(tiny::t86::FSUB(0 , reg), ins);
                        break;
                    case UnOpType::INC:
                        add(tiny::t86::FADD( reg, 1), ins);
                        break;
                    case UnOpType::DEC:
                        add(tiny::t86::FSUB( reg, 1), ins);
                        break;
                    default:
                        throw "not implemented";
                }
//                regAllocator->replaceInt(ins->operand(), ins);
                replaceIntReg(ins->operand(), ins);
                lastIns_ = ret;//        return ret;
                return;
            }
            case ResultType::Void:
                throw "not implemented";
                break;
        }
    }
    
    void NaiveIS::visit(LoadImm *ins) {
        auto ret = pb_.currentLabel();

        switch (ins->resultType()) {
            case ResultType::StructAddress:
            case ResultType::Integer:{

                int64_t value = ins->valueInt();
                if(!instructionToEmplace.empty() ){
                    auto it = instructionToEmplace.find(ins);
                    if( it!= instructionToEmplace.end()){
                        auto * newIns = dynamic_cast<const LoadImm * >(it->second);
                        value = newIns->valueInt();
                    }
                }
                add(tiny::t86::MOV(getReg(ins), value ), ins);
                break;
            }
            case ResultType::Double:
                add(tiny::t86::MOV(getFReg(ins), ins->valueFloat() ), ins);
                break;
            case ResultType::Void:
                break;
        }

    }
    
    void NaiveIS::visit(ArgAddr *ins) {
        auto ret = pb_.currentLabel();
        auto reg = getReg(ins);
//        size_t offset = countArgOffset(ins->args(), ins->index());
        size_t offset =  ins->index();
        add(tiny::t86::MOV(reg, tiny::t86::Bp() ), ins);
        add(tiny::t86::ADD(reg, offset  + 3), ins);
        if( ins->type()->registerType() == ResultType::StructAddress){
            add(tiny::t86::MOV(reg, tiny::t86::Mem(reg)), ins);
        }

        lastIns_ = ret;//        return ret;
    }
    
    void NaiveIS::visit(AllocL *ins) {
        Label ret = pb_.currentLabel();
        Register reg = getReg(ins);
//        regAllocator->makeLocalAllocation(ins->size(), reg, ins);
        makeLocalAllocation(ins->size(), reg, ins);
        lastIns_ = ret;
    }

    void NaiveIS::visit(AllocG *ins) {
        throw "allocG?? done in globals";
        DataLabel glob = pb_.addData((int64_t)(ins->size()));
        int64_t addr = (int64_t)glob;
        globalPointer_ += 1;
        Label ret = add(tiny::t86::MOV(getReg(ins) , addr), ins);

        //std::cerr << "label ret val: " << ret.address() << "| and addr val:  " << addr << std::endl;
        lastIns_ = ret;//        return ret;
    }


    void NaiveIS::visit(Copy *ins) {
        auto ret = pb_.currentLabel();
        add(tiny::t86::MOV(getReg(ins), getReg(ins->src()) ), ins);

        lastIns_ = ret;//        return ret;
    }

    void NaiveIS::visit(Extend *ins) {
        auto ret = pb_.currentLabel();
        add(tiny::t86::EXT(getFReg(ins), getReg(ins->src()) ), ins);
        //clearReg(ins->src());
        lastIns_ = ret;//        return ret;
    }

    void NaiveIS::visit(Truncate *ins) {
        auto ret = pb_.currentLabel();
        add(tiny::t86::NRW(getReg(ins), getFReg(ins->src()) ), ins);
        //clearReg(ins->src());
        lastIns_ = ret;//        return ret;
    }


    void NaiveIS::visit(PutChar *ins) {
        auto ret = pb_.currentLabel();
        add(tiny::t86::PUTCHAR(getReg(ins->src()) ), ins);
        //clearIntReg(ins->src());
        lastIns_ = ret;
    }

    void NaiveIS::visit(GetChar *ins) {
        auto ret = pb_.currentLabel();
        add(tiny::t86::GETCHAR(getReg(ins) ), ins);
        lastIns_ = ret;
    }

    void NaiveIS::visit(Load *ins) {
        auto ret = pb_.currentLabel();
        if(ins->type()->registerType() == ResultType::Double){
            add(tiny::t86::MOV(getFReg(ins), tiny::t86::Mem(getReg(ins->address()))), ins);
            lastIns_ = ret; //return ret;
            return;
        }else if (ins->type()->registerType() == ResultType::Integer){

            auto it = globalTable_.find(ins->address());
            if(it != globalTable_.end()){
                add(tiny::t86::MOV(getReg(ins), (int64_t)it->second), ins);
                lastIns_ = ret; //return ret;
                return;
            }
            if(dynamic_cast<Type::Array *>(ins->type())){
                add(tiny::t86::MOV(getReg(ins), getReg(ins->address())), ins);

            }else{
                add(tiny::t86::MOV(getReg(ins), tiny::t86::Mem(getReg(ins->address()))), ins);
            }
            lastIns_ = ret; //return ret;
            return;
        }else if (ins->type()->registerType() == ResultType::StructAddress){
//            regAllocator->replaceInt(ins->address(), ins);
            replaceIntReg(ins->address(), ins);
            lastIns_ = ret; //return ret;
            return;

        }
        throw "ouch?";
    }

    void NaiveIS::visit(Store *ins) {
        auto ret = pb_.currentLabel();
        if(ins->value()->resultType() == ResultType::Double){
            add(tiny::t86::MOV(Mem(getReg(ins->address())), getFReg(ins->value())), ins);
            //clearFloatReg(ins->value());

        }else{
            add(tiny::t86::MOV(Mem(getReg(ins->address())), getReg(ins->value())), ins);
            //clearIntReg(ins->value());
        }
        lastIns_ = ret;//        return ret;
    }

    void NaiveIS::visit(Phi *ins) {
//        regAllocator->
        registerPhi(ins);
//        return pb_.currentLabel();
        lastIns_ = pb_.currentLabel();
    }

    void NaiveIS::visit(ElemAddrOffset *ins) {
        auto ret = pb_.currentLabel();


        add(tiny::t86::MOV(getReg(ins),
                           getReg(ins->base())), ins);
        add(tiny::t86::ADD(getReg(ins),
                           getReg(ins->offset())), ins);

        //clearIntReg(ins->offset());
        lastIns_ = ret;//        return ret;
    }

    void NaiveIS::visit(ElemAddrIndex *ins) {
        auto ret = pb_.currentLabel();

        add(tiny::t86::MOV(getReg(ins),
                           getReg(ins->base())), ins);
        add(tiny::t86::MOV(getReg(ins),
                           tiny::t86::Mem(getReg(ins->base()))), ins);
        //clearIntReg(ins->offset());
        add(tiny::t86::ADD(getReg(ins),
                           getReg(ins->index())), ins);
        //clearIntReg(ins->index());
        lastIns_ = ret;//        return ret;
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
            visitChild(insns[i]);
            tiny::t86::Label tmp = lastIns_;
            compiled_.emplace(insns[i], tmp);
        }
        auto last = insns[len];
//        regAllocator->spillAllReg();
        visitChild(last);
//        regAllocator->clearAllReg();
        lastIns_ = ret; // return ret;
    }
    
    void NaiveIS::visit(Function * fnc) {
//        regAllocator->resetAllocSize();//        functionLocalAllocSize = 0;
        resetAllocSize();//        functionLocalAllocSize = 0;
        tiny::t86::Label ret =
                add(tiny::t86::PUSH(tiny::t86::Bp()), nullptr);//TODO
        add(tiny::t86::MOV(tiny::t86::Bp(), tiny::t86::Sp()), nullptr);
        Label toPatchStack = add(tiny::t86::SUB(tiny::t86::Sp(), 0), nullptr); //TODO
        for (const auto & bb : getFunctionBBs(fnc)) {
            visitChild(bb);
        }
//        regAllocator->correctStackAlloc(toPatchStack); //        replace(toPatchStack, tiny::t86::SUB(tiny::t86::Sp(), (int64_t)functionLocalAllocSize ));
        correctStackAlloc(toPatchStack); //        replace(toPatchStack, tiny::t86::SUB(tiny::t86::Sp(), (int64_t)functionLocalAllocSize ));
//        regAllocator->clearAllReg();
        lastIns_ = ret; //return ret;
    }
    
    void NaiveIS::visit(ILBuilder & irb) {

        //===================================GLOBALS=====================================
        BasicBlock functionGlobals;
//        makeGlobalTable(irb->globals_.get());
        auto globs = getProgramsGlobals(irb);
        makeGlobalTable(globs);
        for (const auto & str : irb.stringLiterals() ) {
            tiny::t86::DataLabel data = pb_.addData(str.first);
            add(tiny::t86::MOV(getReg(str.second), data ), str.second);
        }

        tiny::t86::Label start = add(tiny::t86::JMP(tiny::t86::Label::empty()), nullptr); //TODO

        //===================================FUNCTIONS=====================================
        for (auto & i : irb.functions()){
            visitChild(i.second.get());
            Label fncLabel = lastIns_;
            addFunction(i.first, fncLabel);
        }

        //===================================MEM INTERCEPT=====================================
        for(auto & f : functionTable_){
             Instruction * fnc_addr = getVariableAddress(irb, f.first);
             instructionToEmplace.emplace(fnc_addr, new LoadImm((int64_t)f.second.address(), nullptr));
        }

//        regAllocator->clearAllReg();
        tiny::t86::Label prolog = //t86::Label(lastInstruction_index +1);
                compileGlobalTable(globs);

        add(tiny::t86::CALL(functionAddr("main")), nullptr) ;//TODO

        //===================================END of MEM INTERCEPT=====================================
        add(tiny::t86::HALT(), nullptr); //TODO?
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
    void NaiveIS::visit(Program * prog) {

        //===================================GLOBALS=====================================
        BasicBlock functionGlobals;
//        makeGlobalTable(irb->globals_.get());
        auto globs = getProgramsGlobals(prog);
        makeGlobalTable(globs);
        for (const auto & str : prog->stringLiterals() ) {
            tiny::t86::DataLabel data = pb_.addData(str.first);
            add(tiny::t86::MOV(getReg(str.second), data ), str.second);
        }

        tiny::t86::Label start = add(tiny::t86::JMP(tiny::t86::Label::empty()), nullptr); //TODO

        //===================================FUNCTIONS=====================================
        for (auto & i : prog->functions()){
            visitChild(i.second.get());
            Label fncLabel = lastIns_;
            addFunction(i.first, fncLabel);
        }

        //===================================MEM INTERCEPT=====================================
        for(auto & f : functionTable_){
             const Instruction * fnc_addr = prog->getGlobalVariableAddress(f.first);
             instructionToEmplace.emplace(fnc_addr, new LoadImm((int64_t)f.second.address(), nullptr));
        }

//        regAllocator->clearAllReg();
        tiny::t86::Label prolog = //t86::Label(lastInstruction_index +1);
                compileGlobalTable(globs);

        add(tiny::t86::CALL(functionAddr("main")), nullptr) ;//TODO

        //===================================END of MEM INTERCEPT=====================================
        add(tiny::t86::HALT(), nullptr); //TODO?
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

    tiny::t86::Program NaiveIS::translate(ILBuilder &ilb) {
        Program program = ilb.finish();
        //auto la = std::make_unique<LivenessAnalysis<TileInfo>>(&program);
       // auto analysis = la->analyze();
        std::cerr << "huh" << std::endl;
        auto v = new NaiveIS();
        v->visit(&program);


        tvlm::ProgramBuilder rawProgB = v->pb_;
        //delete v; //TODO unordered_map failing - double free()
        tiny::t86::Program rawProg = rawProgB.program();
        std::vector<tiny::t86::Instruction*> instrs = rawProg.moveInstructions();
        int line = 0;
        for(const tiny::t86::Instruction * i : instrs){
            std::cerr << tiny::color::blue << line++ << ": " << tiny::color::green << i->toString() << std::endl;
        }

        return {std::move(instrs), rawProg.data()};
    }

    void NaiveIS::makeGlobalTable(BasicBlock * globals) {

        for(const auto *ins : getBBsInstructions(globals)){
            if(const auto * i = dynamic_cast<const  LoadImm *>(ins)){
                globalTable_.emplace(ins, i->valueInt());
            }else if(const auto * alloc = dynamic_cast< const AllocG *>(ins)){
                tiny::t86::DataLabel label = pb_.addData(0);
                globalTable_.emplace(alloc, label);
            }else if( const auto  * store = dynamic_cast<const Store *>(ins)){
                auto val = globalTable_.find(store->value());
                if(val == globalTable_.end()){
                    throw "uninitialized Global?";
                }
                globalTable_.emplace(store->address(),  val->second);
            }else{
                throw "unknown global instruction - make";
            }

        }

    }

    NaiveIS::Label NaiveIS::compileGlobalTable(BasicBlock *globals) {
        auto ret = pb_.currentLabel();
        for(auto & ins : getBBsInstructions(globals)){
            const auto * inss = ins;
            if(const auto * loadImm= dynamic_cast<const  LoadImm *>(inss)){
                globalTable_.emplace(loadImm, loadImm->valueInt());
            }else if(const auto * allocG = dynamic_cast<const AllocG *>(inss)){
                DataLabel label = pb_.addData(0);
                globalTable_.emplace(allocG, label);
            }else if(const auto * store = dynamic_cast<const Store *>(inss)){
                uint64_t value = globalTable_.find(store->value())->second;
                uint64_t address = globalTable_.find(store->address())->second;
                add(tiny::t86::MOV( tiny::t86::Reg(0), (int64_t)address), ins);
                add(tiny::t86::MOV(Mem(tiny::t86::Reg(0)), (int64_t)value), ins);
            }else{
                throw tiny::ParserError("unknown global instruction", ins->ast()->location());
            }

        }
        return ret;
    }

    NaiveIS::NaiveIS(): pb_(ProgramBuilder()), lastIns_(Label::empty())
                        , functionTable_()
                        , globalTable_()
                        , compiled_()
                        , compiledBB_()
                        , future_patch_()
                        , unpatchedCalls_()
                        , globalPointer_(0)
                        , instructionToEmplace()
//                        , regAllocator(new NaiveRegisterAllocator(&pb_))
                        , regAssigner(new RegisterAssigner(&pb_))
                        , regIntCounter_(1)
                        , regFloatCounter_(1)
                        { }

  uint64_t NaiveIS::functionAddr(const std::string & name) const{

        return functionTable_.find(Symbol(name))->second.address();
    }

    void NaiveIS::addFunction(Symbol name, NaiveIS::Label instr) {

        functionTable_.emplace(name, instr.address());
    }


    size_t NaiveIS::countArgOffset(std::vector<const Instruction *>& args, size_t index) {
        size_t acc=  0;
        size_t tmp= 0;

        for (auto & a: args) {
            tmp++;
            acc += (a->resultType() == ResultType::Double) ? 2 : 1;
        }
        assert(tmp == index);
        return acc;
    }

    template<typename T>
    void NaiveIS::replace(NaiveIS::Label label, const T & instruction) {
//        if(label >= pb_.currentLabel()){
//            //ERROR
//            std::cerr << "replace unable to continue" << std::endl;
//            return;
//        }
//        auto p = pb_.program();
//        auto instrs = p.moveInstructions();
//        delete instrs[label];
//        instrs[label] = new T(instruction);
//
//        auto resProg = tiny::t86::Program(instrs, p.data());
//        pb_ = ProgramBuilder(std::move(resProg));

        pb_.replace(label, instruction);
    }

    void NaiveIS::copyStruct(const Register &from, Type *type, const Register &to, const Instruction * ins) {

        auto tmpReg = getFreeIntRegister();
        Type::Struct * strct =  dynamic_cast<Type::Struct *>(type);
        for (int i = 0; i < strct->size(); ++i) {
            add(tiny::t86::MOV(tmpReg, tiny::t86::Mem(from + i )), ins);
            add(tiny::t86::MOV(tiny::t86::Mem(to + i), tmpReg), ins);
        }

//        clearTmpIntRegister(tmpReg); tmpReg = -15654;
    }

    NaiveIS::~NaiveIS(){
//            for (auto & i:instructionToEmplace) {
//                delete i.second;
//            }
        tiny::ASTPrettyPrinter p(std::cout);
        for (auto it = globalTable_.begin(); it != globalTable_.end(); it=globalTable_.begin()) {
            std::cout << "deleting" << it->second << std::endl;
            it->first->print(p);
            std::cout << std::endl;
            globalTable_.erase(it);
        }
        std::cout << "deleted " << std::endl;

        instructionToEmplace.~unordered_map();
        functionTable_.~unordered_map();
        globalTable_.~unordered_map(); //!!!
        compiled_.~unordered_map();
        compiledBB_.~unordered_map();
//        regAllocator.reset(nullptr);
        std::cout << "calling ~NaiveIS" << std::endl;
    }


}
