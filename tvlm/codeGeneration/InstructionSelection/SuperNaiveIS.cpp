#include "SuperNaiveIS.h"
#include "tvlm/tvlm/il/il_builder.h"

#define ZeroFlag 0x02
#define SignFlag 0x01

#include "tvlm/tvlm/codeGeneration/FunctionalMacro.h"

namespace tvlm {

    SuperNaiveIS::SuperNaiveIS():
//    pb_(ProgramBuilderOLD()),
//    lastIns_(Label::empty()),
     program_(TargetProgram())
//    , globalTable_()
    , unpatchedCalls_()
//                        , regAllocator(new NaiveRegisterAllocator(&pb_))
    , regAssigner(new RegisterAssigner( &program_))
    { }

    SuperNaiveIS::~SuperNaiveIS() =  default;

    void SuperNaiveIS::visit(Jump *ins) {
        tiny::t86::Label jmp = addF(LMBJUMP, ins  );
//        future_patch_.emplace_back(jmp, ins->getTarget(1));
//        jump_patches_.emplace_back(std::make_pair(ins, jmp), ins->getTarget(1) );
        program_.patchJump(ins, jmp, ins->getTarget(1));

    }

    void SuperNaiveIS::visit(CondJump *ins) {
        auto condReg = getReg(ins->condition(), ins);
//        add(tiny::t86::CMP(condReg, 0), ins);
        addF(LMB { return new tiny::t86::CMP(vR(condReg), 0);} , ins);
//        clearIntReg(ins->condition());
        tiny::t86::Label condJump = addF(LMB {return new tiny::t86::JZ(tiny::t86::Label::empty());} ,ins);
        tiny::t86::Label jumpToTrue = addF(LMBJUMP, ins);
//        future_patch_.emplace_back(jumpToTrue, ins->getTarget(1));
//        jump_patches_.emplace_back(std::make_pair(ins, jumpToTrue), ins->getTarget(1) );
        program_.patchJump(ins, jumpToTrue, ins->getTarget(1)  );
//        future_patch_.emplace_back(/*jumpToFalse*/condJump, ins->getTarget(0));
//        jump_patches_.emplace_back(std::make_pair(ins, condJump), ins->getTarget(0) );
        program_.patchJump(ins, condJump, ins->getTarget(0)  );
    }

    void SuperNaiveIS::visit(Return *ins) {
        auto returnReg = getReg(ins->returnValue(), ins);

    }

    void SuperNaiveIS::visit(CallStatic *ins) {

//    auto ret = pb_.currentLabel();
//        //spill everything
////        regAllocator->spillAllReg();

//        //args /*-> prepare values
        for (auto it = ins->args().crbegin() ; it != ins->args().crend();it++) {
            if((*it).second->registerType() == ResultType::StructAddress) {
//                allocateStructArg(it->second, it->first);
            }else if((*it).second->registerType() == ResultType::Integer){
                auto argReg = getReg(it->first, ins);
                addF(LMBS tiny::t86::PUSH(vR(argReg)) LMBE, ins);
//                clearIntReg(it->first);
            }else if ((*it).second->registerType() == ResultType::Double){
                auto argFReg = getFReg(it->first, ins);
                addF(LMBS tiny::t86::FPUSH(vFR(argFReg)) LMBE, ins);
//                clearFloatReg(it->first);
            }
        }
//
//        //prepare return Value //*-> preparation in RA -- memory and register in RA
        ins->f()->getType()->registerType() == ResultType::StructAddress ?
            prepareReturnValue(ins->f()->getType()->size(), ins):
            prepareReturnValue(0, ins);
//
////        regAllocator->spillCallReg();
//        //call
//        tiny::t86::Label callLabel = add(tiny::t86::CALL{tiny::t86::Label::empty()}, ins);
        tiny::t86::Label callLabel = addF(
                LMBS tiny::t86::CALL{tiny::t86::Label::empty()} LMBE
                , ins );
        if(ins->resultType() == ResultType::Double){
            auto freg = getFReg(ins, ins);
            addF( LMBS tiny::t86::MOV( vFR(freg), tiny::t86::FReg(0)) LMBE, ins);
        }else if (ins->resultType() == ResultType::Integer){
            auto reg = getReg(ins, ins);
            addF( LMBS tiny::t86::MOV( vR(reg), tiny::t86::Reg(0)) LMBE, ins);
        } else if(ins->resultType() == ResultType::Void){

        }
//
        //CountArgSize;
        int argSize  = 0;
        for (auto & a :ins->args()) {
            if(a.second->registerType() == ResultType:: Double){
                argSize +=1;
            }else{
                argSize ++;
            }
        }

        addF( LMBS tiny::t86::ADD(tiny::t86::Sp(), argSize) LMBE, ins);
//
        program_.registerCall(ins, callLabel, ins->f()->name());
//        lastIns_ = ret; //return ret;
    }

    void SuperNaiveIS::visit(Call *ins) {

    }

    void SuperNaiveIS::visit(Copy *ins) {
        switch (ins->resultType()) {
            case ResultType::Integer:{

                auto lhs = getReg(ins, ins);
                auto rhs = getReg(ins->src(), ins);

                addF(LMB {
                    return new tiny::t86::MOV(
                            vR(lhs),
                            vR(rhs));
                }, ins);

                break;
            }
            case ResultType::Double:{

                auto lhs = getFReg(ins, ins);
                auto rhs = getFReg(ins->src(), ins);

                addF(LMB{
                    return new tiny::t86::MOV(
                            vFR(lhs),
                            vFR(rhs) );
                } , ins);

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

    void SuperNaiveIS::visit(Extend *ins) {
        auto lhs = getFReg(ins, ins);
        auto rhs = getReg(ins->src(), ins);
//        add(tiny::t86::EXT(lhs , rhs ), ins);
        addF(LMB { return new tiny::t86::EXT(
                vFR(lhs) ,
                vR(rhs));
            }, ins);
        //clearReg(ins->src());
    }

    void SuperNaiveIS::visit(Truncate *ins) {
        auto lhs = getReg(ins, ins);
        auto rhs = getFReg(ins->src(), ins);
//        add(tiny::t86::NRW(lhs, getFReg(ins->src()) ), ins);
        addF(LMB { return new tiny::t86::NRW(vR(lhs), vFR(rhs) );}, ins);
        //clearReg(ins->src());
    }

    void SuperNaiveIS::visit(BinOp *ins) {

        switch (ins->resultType()) {
            case ResultType::StructAddress:
            case ResultType::Integer:{

                auto lhsreg = getReg(ins->lhs(), ins);
                auto rhsreg = getReg(ins->rhs(), ins);
                switch (ins->opType()) {
                    case BinOpType::ADD:
                        addF(LMBS tiny::t86::ADD(vR(lhsreg), vR(rhsreg)) LMBE, ins);
                        break;
                    case BinOpType::SUB:
                        addF(LMBS tiny::t86::SUB(vR(lhsreg), vR(rhsreg)) LMBE, ins);
                        break;
                    case BinOpType::MOD:
                        addF(LMBS tiny::t86::MOD(vR(lhsreg), vR(rhsreg)) LMBE, ins);
                        break;
                    case BinOpType::MUL:
                        addF(LMBS tiny::t86::MUL(vR(lhsreg), vR(rhsreg)) LMBE, ins);
                        break;
                    case BinOpType::DIV:
                        addF(LMBS tiny::t86::DIV(vR(lhsreg), vR(rhsreg)) LMBE, ins);
                        break;

                    case BinOpType::AND:
                        addF(LMBS tiny::t86::AND(vR(lhsreg), vR(rhsreg)) LMBE, ins);
                        break;
                    case BinOpType::OR:
                        addF(LMBS tiny::t86::OR(vR(lhsreg), vR(rhsreg)) LMBE, ins);
                        break;
                    case BinOpType::XOR:
                        addF(LMBS tiny::t86::XOR(vR(lhsreg), vR(rhsreg)) LMBE, ins);
                        break;

                    case BinOpType::LSH:
                        addF(LMBS tiny::t86::LSH(vR(lhsreg), vR(rhsreg)) LMBE, ins);
                        break;
                    case BinOpType::RSH:
                        addF(LMBS tiny::t86::RSH(vR(lhsreg), vR(rhsreg)) LMBE, ins);
                        break;

                    case BinOpType::NEQ:
                        addF(LMBS tiny::t86::SUB(vR(lhsreg), vR(rhsreg)) LMBE, ins);
                        break;
                    case BinOpType::EQ:
                        addF(LMBS tiny::t86::SUB(vR(lhsreg), vR(rhsreg)) LMBE, ins);
                        addF(LMBS tiny::t86::MOV(vR(lhsreg), tiny::t86::Flags()) LMBE, ins);
                        addF(LMBS tiny::t86::AND(vR(lhsreg), ZeroFlag) LMBE, ins);//ZeroFlag
                        break;
                    case BinOpType::LTE:
                        addF(LMBS tiny::t86::SUB(vR(rhsreg), vR(lhsreg))  LMBE, ins);
                        addF(LMBS tiny::t86::MOV(vR(lhsreg), tiny::t86::Flags())  LMBE, ins);
                        addF(LMBS tiny::t86::AND(vR(lhsreg), SignFlag)  LMBE, ins);//SignFlag
                        addF(LMBS tiny::t86::NOT(vR(lhsreg))  LMBE, ins);
                        addF(LMBS tiny::t86::AND(vR(lhsreg), 1)  LMBE, ins);
                        break;
                    case BinOpType::LT:
                        addF(LMBS tiny::t86::SUB(vR(lhsreg), vR(rhsreg)) LMBE, ins);
                        addF(LMBS tiny::t86::MOV(vR(lhsreg), tiny::t86::Flags()) LMBE, ins);
                        addF(LMBS tiny::t86::AND(vR(lhsreg), SignFlag) LMBE, ins);//SignFlag
                        break;
                    case BinOpType::GT:
                        addF(LMBS tiny::t86::SUB(vR(rhsreg), vR(lhsreg)) LMBE, ins);
                        addF(LMBS tiny::t86::MOV(vR(lhsreg), tiny::t86::Flags())LMBE, ins);
                        addF(LMBS tiny::t86::AND(vR(lhsreg), SignFlag) LMBE, ins);//SignFlag
                        break;
                    case BinOpType::GTE:
                        addF(LMBS tiny::t86::SUB(vR(lhsreg), vR(rhsreg)) LMBE, ins);
                        addF(LMBS tiny::t86::MOV(vR(lhsreg), tiny::t86::Flags()) LMBE, ins);
                        addF(LMBS tiny::t86::AND(vR(lhsreg), SignFlag) LMBE , ins);//SignFlag
                        addF(LMBS tiny::t86::NOT(vR(lhsreg)) LMBE, ins);
                        addF(LMBS tiny::t86::AND(vR(lhsreg), 1) LMBE, ins);
                        break;
                };

                regAssigner->replaceIntReg(ins->lhs(), ins);
                //clearIntReg(ins->rhs());
                return;
            }
            case ResultType::Double:{
                auto lhsreg = getFReg(ins->lhs(), ins);
                auto rhsreg = getFReg(ins->rhs(), ins);

                switch (ins->opType()) {
                    case BinOpType::ADD:
                        addF(LMBS tiny::t86::FADD(vFR(lhsreg), vFR(rhsreg)) LMBE, ins);
                        break;
                    case BinOpType::SUB:
                        addF(LMBS tiny::t86::FSUB(vFR(lhsreg), vFR(rhsreg)) LMBE, ins);
                        break;
                    case BinOpType::MUL:
                        addF(LMBS tiny::t86::FMUL(vFR(lhsreg), vFR(rhsreg)) LMBE, ins);
                        break;
                    case BinOpType::DIV:
                        addF(LMBS tiny::t86::FDIV(vFR(lhsreg), vFR(rhsreg)) LMBE, ins);
                        break;
                    case BinOpType::NEQ:{
                        addF(LMBS tiny::t86::FSUB(vFR(lhsreg),vFR(rhsreg)) LMBE, ins);
                        auto freeReg  = getReg(ins, ins);
                        addF( LMBS tiny::t86::NRW(vR(freeReg), vFR(lhsreg)) LMBE, ins);
                        //clearFloatReg(ins->lhs());
                        //clearFloatReg(ins->rhs());
                        return;
                    }
                    case BinOpType::EQ:{
                        auto tmpreg = getReg(ins, ins);
                        addF( LMBS tiny::t86::FSUB(vFR(lhsreg),vFR(rhsreg)) LMBE, ins);
                        addF( LMBS tiny::t86::MOV(vR(tmpreg), tiny::t86::Flags()) LMBE, ins);
                        addF( LMBS tiny::t86::AND(vR(tmpreg), ZeroFlag) LMBE, ins);//ZeroFlag
                        //clearFloatReg(ins->rhs());
                        //clearFloatReg(ins->lhs());
                        return;
                    }
                    case BinOpType::LTE:{
                        auto tmpreg = getReg(ins, ins);

                        addF( LMBS tiny::t86::FSUB(vFR(rhsreg), vFR(lhsreg)) LMBE, ins);
                        addF( LMBS tiny::t86::MOV(vR(tmpreg), tiny::t86::Flags()) LMBE, ins);
                        addF( LMBS tiny::t86::AND(vR(tmpreg), SignFlag) LMBE,  ins);//SignFlag
                        addF( LMBS tiny::t86::NOT(vR(tmpreg)) LMBE, ins);
                        addF( LMBS tiny::t86::AND(vR(tmpreg), 1) LMBE, ins);

                        //clearFloatReg(ins->rhs());
                        //clearFloatReg(ins->lhs());
                        return;
                    }
                    case BinOpType::LT:{
                        auto tmpreg = getReg(ins, ins);
                        addF( LMBS tiny::t86::FSUB(vFR(lhsreg), vFR(rhsreg)) LMBE, ins);
                        addF( LMBS tiny::t86::MOV(vR(tmpreg), tiny::t86::Flags()) LMBE, ins);
                        addF( LMBS tiny::t86::AND(vR(tmpreg), SignFlag) LMBE, ins);//SignFlag

                        //clearFloatReg(ins->rhs());
                        //clearFloatReg(ins->lhs());
                        return;
                    }
                    case BinOpType::GT:{
                        auto tmpreg = getReg(ins, ins);
                        addF( LMBS tiny::t86::FSUB(vFR(rhsreg), vFR(lhsreg)) LMBE, ins);
                        addF( LMBS tiny::t86::MOV(vR(tmpreg), tiny::t86::Flags()) LMBE, ins);
                        addF( LMBS tiny::t86::AND(vR(tmpreg), SignFlag) LMBE,  ins);//SignFlag

                        //clearFloatReg(ins->rhs());
                        //clearFloatReg(ins->lhs());
                        return;
                    }
                    case BinOpType::GTE:{
                        auto tmpreg = getReg(ins, ins);
                        addF( LMBS tiny::t86::FSUB(vFR(lhsreg), vFR(rhsreg)) LMBE , ins);
                        addF( LMBS tiny::t86::MOV(vR(tmpreg), tiny::t86::Flags()) LMBE , ins);
                        addF( LMBS tiny::t86::AND(vR(tmpreg), SignFlag) LMBE, ins );//SignFlag
                        addF( LMBS tiny::t86::NOT(vR(tmpreg)) LMBE , ins);
                        addF( LMBS tiny::t86::AND(vR(tmpreg), 1) LMBE , ins);

                        //clearFloatReg(ins->rhs());
                        //clearFloatReg(ins->lhs());
                        return;
                    }
                    default:
                        throw "not implemented or not supported";
                };

//                regAllocator->replaceFloat(ins->lhs(), ins);
                regAssigner->replaceFloatReg(ins->lhs(), ins);
                //clearFloatReg(ins->rhs());
                return;

            }
            case ResultType::Void:
                throw "not implemented";
                break;
        }

        throw "ERROR escaped Bin Op Switch";
    }

    void SuperNaiveIS::visit(UnOp *ins) {
        switch (ins->resultType()) {

            case ResultType::Integer:
            case ResultType::StructAddress:{

                auto reg =
                        getReg(ins->operand(), ins);
                switch (ins->opType()) {
                    case UnOpType::NOT:
                        addF( LMBS tiny::t86::NOT(vR(reg)) LMBE, ins);
                        break;
                    case UnOpType::UNSUB:{
                        auto tmpReg = getReg(ins, ins);
                        addF( LMBS tiny::t86::MOV( vR(tmpReg) , (int64_t) 0 ) LMBE, ins);
                        addF( LMBS tiny::t86::SUB( vR(tmpReg) , vR(reg)) LMBE, ins);
                        return;
                    }
                    case UnOpType::INC:
                        addF(LMBS tiny::t86::INC( vR(reg)) LMBE, ins);
                        break;
                    case UnOpType::DEC:
                        addF( LMBS tiny::t86::DEC( vR(reg)) LMBE, ins);
                        break;
                }
//                regAllocator->replaceInt(ins->operand(), ins);
                regAssigner->replaceIntReg(ins->operand(), ins);

                return;
            }
            case ResultType::Double:{
                auto reg = getFReg(ins->operand(), ins);
                switch (ins->opType()) {
                    case UnOpType::UNSUB:
                        addF( LMBS tiny::t86::FSUB(0 , vFR(reg)) LMBE, ins);
                        break;
                    case UnOpType::INC:
                        addF( LMBS tiny::t86::FADD( vFR(reg), 1) LMBE, ins);
                        break;
                    case UnOpType::DEC:
                        addF( LMBS tiny::t86::FSUB( vFR(reg), 1) LMBE, ins);
                        break;
                    default:
                        throw "not implemented";
                }
//                regAllocator->replaceInt(ins->operand(), ins);
                regAssigner->replaceIntReg(ins->operand(), ins);
                return;
            }
            case ResultType::Void:
                throw "not implemented";
                break;
        }
    }

    void SuperNaiveIS::visit(LoadImm *ins) {
        switch (ins->resultType()) {
            case ResultType::Integer:{

                int64_t value = ins->valueInt();
//                if(!instructionToEmplace.empty() ){
//                    auto it = instructionToEmplace.find(ins);
//                    if( it!= instructionToEmplace.end()){
//                        auto * newIns = dynamic_cast<const LoadImm * >(it->second);
//                        value = newIns->valueInt();
//                    }
//                }
                auto reg = getReg(ins, ins);
                addF(LMBS tiny::t86::MOV(vR(reg), value ) LMBE , ins);
                break;
            }
            case ResultType::Double:{
                auto freg = getFReg(ins, ins);
                addF( LMBS tiny::t86::MOV( vFR(freg), ins->valueFloat() ) LMBE , ins);
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

    void SuperNaiveIS::visit(AllocL *ins) {
        auto reg = getReg(ins, ins);
//        regAllocator->makeLocalAllocation(ins->size(), reg, ins);
        makeLocalAllocation(ins->size(), reg, ins); //TODO !!!! TODO TODO TODO functional local Allocation
    }

    void SuperNaiveIS::visit(AllocG *ins) {

    }

    void SuperNaiveIS::visit(ArgAddr *ins) {
        size_t reg = getReg(ins, ins);
//        size_t offset = countArgOffset(ins->args(), ins->index());
        size_t offset =  ins->index();
        addF( LMBS tiny::t86::MOV( vR(reg), tiny::t86::Bp() ) LMBE, ins);
        addF( LMBS tiny::t86::ADD(vR(reg), offset  + 3) LMBE, ins);
        if( ins->type()->registerType() == ResultType::StructAddress){
            addF( LMBS tiny::t86::MOV( vR(reg), tiny::t86::Mem(vR(reg))) LMBE, ins);
        }

    }
    void SuperNaiveIS::visit(PutChar *ins) {
        auto reg = getReg(ins->src(), ins);
        addF( LMBS tiny::t86::PUTCHAR( vR(reg)) LMBE,  ins);
        //clearIntReg(ins->src());
    }

    void SuperNaiveIS::visit(GetChar *ins) {
        auto reg = getReg(ins, ins);
        addF( LMBS tiny::t86::GETCHAR( vR(reg)) LMBE, ins);
    }

    void SuperNaiveIS::visit(Load *ins) {
        if(ins->type()->registerType() == ResultType::Double){
            auto freg =getFReg(ins, ins);
            auto regAddr = getReg(ins->address(), ins);
            addF( LMBS tiny::t86::MOV( vFR(freg), tiny::t86::Mem( vR(regAddr))) LMBE, ins);
            return;
        }else if (ins->type()->registerType() == ResultType::Integer){

            auto it = program_.globalFind(ins->address());
            if(it != program_.globalEnd()){
                auto reg = getReg(ins, ins);
                addF( LMBS tiny::t86::MOV(vR(reg), (int64_t)it->second) LMBE, ins);
                return;
            }
            if(dynamic_cast<Type::Array *>(ins->type())){
                auto reg = getReg(ins, ins);
                auto regAddr = getReg(ins->address(), ins);
                addF( LMBS tiny::t86::MOV( vR(reg), vR(regAddr)) LMBE, ins);

            }else{
                auto reg = getReg(ins, ins);
                auto regAddr = getReg(ins->address(), ins);
                addF( LMBS tiny::t86::MOV( vR(reg), tiny::t86::Mem(vR(regAddr))) LMBE, ins);
            }
            return;
        }else if (ins->type()->registerType() == ResultType::StructAddress){
//            regAllocator->replaceInt(ins->address(), ins);
            regAssigner->replaceIntReg(ins->address(), ins);
            return;

        }
        throw "ERROR[IS] failed load";
    }

    void SuperNaiveIS::visit(Store *ins) {
        switch (ins->value()->resultType()) {
            case ResultType::Double:{
                auto regAddr = getReg(ins->address(), ins);
                auto regValue = getFReg(ins->value(), ins);
                addF( LMBS tiny::t86::MOV(Mem(vR(regAddr)), vFR(regValue)) LMBE, ins);
                return;
            }
            case ResultType::StructAddress:
            case ResultType::Integer:{
                auto regAddr = getReg(ins->address(), ins);
                auto regValue = getReg(ins->value(), ins);
                addF( LMBS tiny::t86::MOV(Mem(vR(regAddr)), vR(regValue)) LMBE, ins);
                return;
            }
            case ResultType::Void:{
                throw "ERROR[IS] Store: failed to store void value";
            }
        }
        throw "ERROR[IS] Store: failed to resolve value type";
    }

    void SuperNaiveIS::visit(Phi *ins) {
        regAssigner->registerPhi(ins);
    }

    void SuperNaiveIS::visit(ElemAddrOffset *ins) {

        auto reg = getReg(ins, ins);
        auto regBase = getReg(ins->base(), ins);
        auto regOffset = getReg(ins->offset(), ins);
        addF( LMBS tiny::t86::MOV(vR(reg),
                          vR(regBase) ) LMBE, ins);
        addF( LMBS tiny::t86::ADD(vR(reg),
                           vR(regOffset)) LMBE, ins);

        //clearIntReg(ins->offset());
    }

    void SuperNaiveIS::visit(ElemAddrIndex *ins) {

        auto reg= getReg(ins, ins);
        auto regBase = getReg(ins->base(), ins);
        auto regIndex = getReg(ins->index(), ins);
        addF( LMBS tiny::t86::MOV( vR(reg),
                           vR(regBase)) LMBE, ins);
        addF( LMBS tiny::t86::MOV(vR(reg),
                           tiny::t86::Mem(vR(regBase))) LMBE, ins);
        //clearIntReg(ins->offset());
        addF( LMBS tiny::t86::ADD(vR(reg),
                           vR(regIndex)) LMBE, ins);
        //clearIntReg(ins->index());
    }

    void SuperNaiveIS::visit(Halt *ins) {
        addF( LMBS tiny::t86::HALT() LMBE, ins);
    }


    void SuperNaiveIS::copyStruct(size_t from, Type *type, size_t to, const Instruction * ins) {

//        auto tmpReg = regAssigner->getFreeIntRegister();
        auto tmpReg = getExtraIntReg(ins);

        Type::Struct * strct =  dynamic_cast<Type::Struct *>(type);
        for (int i = 0; i < strct->size(); ++i) {
            addF( LMBS tiny::t86::MOV( vR(tmpReg), tiny::t86::Mem( vR(from) + i )) LMBE, ins);
            addF( LMBS tiny::t86::MOV(tiny::t86::Mem(vR(to) + i), tmpReg) LMBE, ins);
        }

//        clearTmpIntRegister(tmpReg); tmpReg = -15654;
    }

    void SuperNaiveIS::visit(StructAssign *ins) {
        copyStruct(getReg(ins->srcVal(), ins), ins->type(), getReg(ins->dstAddr(), ins), ins);
    }

    void SuperNaiveIS::visit(BasicBlock *bb) {
        auto insns = getBBsInstructions(bb);
        for(int64_t i = 0; i < insns.size() ;i++){
            visitChild(insns[i]);
        }
    }

    void SuperNaiveIS::visit(Function *fce) {
        regAssigner->resetAllocSize();
        for (const auto & bb : getFunctionBBs(fce)) {
            visitChild(bb);
        }
        regAssigner->exportAlloc(fce);

    }

    void SuperNaiveIS::visit(Program *p) {


        //===================================GLOBALS=====================================
//        makeGlobalTable(irb->globals_.get());
        auto globs = getProgramsGlobals(p);
        makeGlobalTable(globs);
        for (const auto & str : p->stringLiterals() ) {
            tiny::t86::DataLabel data = program_.addData(str.first);
            auto reg = getReg(str.second, str.second);
            addF(LMBS tiny::t86::MOV( vR(reg), data ) LMBE, str.second);
        }

        //===================================FUNCTIONS=====================================
        for (auto & i : p->functions()){
            visitChild(i.second.get());
        }


        //        regAllocator->clearAllReg();
                compileGlobalTable(globs);

    }
    void SuperNaiveIS::makeGlobalTable(BasicBlock * globals) {

        for(const auto *ins : getBBsInstructions(globals)){
            if(const auto * i = dynamic_cast<const  LoadImm *>(ins)){
                program_.globalEmplace(ins, i->valueInt());
            }else if(const auto * alloc = dynamic_cast< const AllocG *>(ins)){
                if(alloc->resultType() == ResultType::Double){
                    tiny::t86::DataLabel label = program_.addData(0);
                    program_.globalEmplace(alloc, label);
                    throw "global Double compilation not implemented ";
                }else if(alloc->resultType() == ResultType::Integer){
                    tiny::t86::DataLabel label = program_.addData(0);
                    program_.globalEmplace(alloc, label);

                } else if (alloc->resultType() == ResultType::StructAddress){
                    throw "global Struct compilation not implemented ";
                }else if (alloc->resultType() == ResultType::Void){
                    throw "global Void compilation not implemented ";
                }else{
                    throw tiny::ParserError("invalid type of global allocation", ins->ast()->location());
                }
            }else if( const auto  * store = dynamic_cast<const Store *>(ins)){
                auto val = program_.globalFind(store->value());
                if(val == program_.globalEnd()){
                    throw "uninitialized Global?";
                }
                program_.globalEmplace(store->address(),  val->second);
            }else{
                throw "unknown global instruction - make";
            }

        }

    }


    void SuperNaiveIS::compileGlobalTable(BasicBlock * globals) {

        for(const auto *ins : getBBsInstructions(globals)){
            if(const auto * i = dynamic_cast<const  LoadImm *>(ins)){
                if(i->resultType() == ResultType::Integer){
                    auto reg = getReg(ins, ins);
                    addF( LMBS tiny::t86::MOV( vR(reg), i->valueInt()) LMBE, ins);
                }else if (i->resultType() == ResultType::Double){
                    auto freg=getFReg(ins, ins);
                    addF( LMBS tiny::t86::MOV( vFR(freg), i->valueFloat()) LMBE, ins);
                }else{
                    throw "load imm with Res Type Structure or Void not implemented";
                }
            }else if(const auto * alloc = dynamic_cast< const AllocG *>(ins)){
                if(alloc->amount()){
                    throw "allocG with array not implemented"; //TODO global array
                }else{
                    regAssigner->makeGlobalAllocation(alloc->size(), getReg(alloc, ins), alloc);
                }
            }else if( const auto  * store = dynamic_cast<const Store *>(ins)){
                uint64_t value = program_.globalFind(store->value())->second;
                uint64_t address = program_.globalFind(store->address())->second;
                auto reg = getReg(ins, ins);
                addF(LMBS tiny::t86::MOV( vR(reg), (int64_t)address)LMBE , ins);
                addF(LMBS tiny::t86::MOV(Mem(vR(reg)), (int64_t)value) LMBE, ins);
            }else{
                throw tiny::ParserError("unknown global instruction", ins->ast()->location());
            }

        }

    }


    TargetProgram SuperNaiveIS::translate(ILBuilder &ilb) {
        auto is = SuperNaiveIS();
        Program *  prog = new Program(ilb.finish());
        is.program_.setProgram(prog );
        is.visit(prog);
        return is.finalize();
//TODO
        return TargetProgram(is.program_);
    }

    void SuperNaiveIS::visit(Instruction *ins) {

    }

    TargetProgram SuperNaiveIS::finalize() {
        auto res = TargetProgram(program_);

        //registers are in allocated registers

        return res;
    }

}