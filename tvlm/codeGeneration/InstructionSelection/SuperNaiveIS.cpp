#include "SuperNaiveIS.h"
#include "tvlm/tvlm/il/il_builder.h"

#define ZeroFlag 0x02
#define SignFlag 0x01

#include "tvlm/tvlm/codeGeneration/FunctionalMacro.h"

namespace tvlm {


//    SuperNaiveIS::SuperNaiveIS(std:: * prog):
////    pb_(ProgramBuilderOLD()),
////    lastIns_(Label::empty()),
//    program_(prog)
////    , globalTable_()
//    , unpatchedCalls_()
//    , analysis_(new ColoringLiveAnalysis<>(&program_))
//    , regAssigner_(new RegisterAssigner(&program_))
//    {
//        tiny::config.setDefaultIfMissing("-debug", "0");
//        hardDBG_ = std::stoul(tiny::config.get("-debug"));
//    }

    SuperNaiveIS::SuperNaiveIS(const std::shared_ptr<Program> & prog):
//    pb_(ProgramBuilderOLD()),
//    lastIns_(Label::empty()),
    program_(TargetProgram(prog))
//    , globalTable_()
    , unpatchedCalls_()
    , analysis_(new ColoringLiveAnalysis<>(&program_))
    , regAssigner_(new RegisterAssigner(&program_))
    {
        tiny::config.setDefaultIfMissing("-debug", "0");
        hardDBG_ = std::stoul(tiny::config.get("-debug"));
    }

    SuperNaiveIS::~SuperNaiveIS() = default;

    void SuperNaiveIS::visit(Jump *ins) {
        tiny::t86::Label jmp = addF(LMBJUMP, ins  );
//        future_patch_.emplace_back(jmp, ins->getTarget(1));
//        jump_patches_.emplace_back(std::make_pair(ins, jmp), ins->getTarget(1) );
        program_.patchJump(ins, jmp, ins->getTarget(1));

    }

    void SuperNaiveIS::visit(CondJump *ins) {
        auto condReg = getReg(ins->condition(), ins);
//        add(tiny::t86::CMP(condReg, 0), ins);
        addFsilent(LMB { return new tiny::t86::CMP(vR(condReg), 0);} , ins);
//        clearIntReg(ins->condition());
        tiny::t86::Label condJump = addFsilent(LMB {return new tiny::t86::JZ(tiny::t86::Label::empty());} ,ins);
        tiny::t86::Label jumpToTrue = addF(LMBJUMP, ins);
//        future_patch_.emplace_back(jumpToTrue, ins->getTarget(1));
//        jump_patches_.emplace_back(std::make_pair(ins, jumpToTrue), ins->getTarget(1) );
        program_.patchJump(ins, jumpToTrue, ins->getTarget(1)  );
//        future_patch_.emplace_back(/*jumpToFalse*/condJump, ins->getTarget(0));
//        jump_patches_.emplace_back(std::make_pair(ins, condJump), ins->getTarget(0) );
        program_.patchJump(ins, condJump, ins->getTarget(0)  );
    }

    void SuperNaiveIS::visit(Return *ins) {

        if(ins->returnType()->registerType() == ResultType::StructAddress){
//            auto reg = getReg(ins);
            auto returnReg = getReg(ins->returnValue(), ins);
            addF( LMBS tiny::t86::MOV( vR(returnReg),tiny::t86::Bp() ) LMBE , ins);
            addF( LMBS tiny::t86::ADD( vR(returnReg), 1 ) LMBE , ins);
          copyStruct(returnReg, ins->returnType(), tiny::t86::Reg(0) , ins);
            // TODO fail to get info which is passing register Reg0 or stackplace or else

        }else if (ins->returnType()->registerType() == ResultType::Double){
            auto retFReg = getFReg(ins->returnValue(), ins);
            addF( LMBS tiny::t86::MOV(tiny::t86::FReg(0 /*or Somwhere on stack*/  ), vFR(retFReg) ) LMBE , ins);

        }else if (ins->returnType()->registerType() == ResultType::Integer){

            auto returnReg = getReg(ins->returnValue(), ins);
            addF( LMBS tiny::t86::MOV(tiny::t86::Reg(0 /*or Somwhere on stack*/  ), vR(returnReg)) LMBE , ins);

        }else{ // void
            withoutRegVariant(ins);
        }
        //here we want to respect calling convention and spilling everything alive
            addF( LMBS tiny::t86::MOV(tiny::t86::Sp(), tiny::t86::Bp()) LMBE , ins);
            addF( LMBS tiny::t86::POP(tiny::t86::Bp()) LMBE , ins);
            addF( LMBS tiny::t86::RET() LMBE , ins);
//        regAllocator->clearAllReg();
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

        size_t returnValueRegister = 0;
        if(ins->f()->getType()->registerType() == ResultType::StructAddress){
            prepareReturnValue(ins->f()->getType()->size(), ins);
        } else if (ins->f()->getType()->registerType() == ResultType::Double){
            returnValueRegister = getFReg(ins, ins);
        }else if (ins->f()->getType()->registerType() == ResultType::Integer) {
            returnValueRegister = getReg(ins, ins);

        }else{//void

        }
////        regAllocator->spillCallReg();
//        //call
//        tiny::t86::Label callLabel = add(tiny::t86::CALL{tiny::t86::Label::empty()}, ins);
        tiny::t86::Label callLabel = addF(
                LMBS tiny::t86::CALL{tiny::t86::Label::empty()} LMBE
                , ins );

        // -** manage return Value
        if(ins->f()->getType()->registerType() == ResultType::Double){
//            auto freg = getFReg(ins, ins);
            addF( LMBS tiny::t86::MOV( vFR(returnValueRegister), tiny::t86::FReg(0)) LMBE, ins);
        }else if (ins->f()->getType()->registerType() == ResultType::Integer){
//            auto reg = getReg(ins, ins);
            addF( LMBS tiny::t86::MOV( vR(returnValueRegister), tiny::t86::Reg(0)) LMBE, ins);
        } else if(ins->f()->getType()->registerType() == ResultType::StructAddress){

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

        addF( LMBS tiny::t86::ADD(tiny::t86::Sp(), argSize) LMBE, ins); // instead of popping function args
//
        program_.registerCall(ins, callLabel, ins->f()->name());
//        lastIns_ = ret; //return ret;
    }

    void SuperNaiveIS::visit(Call *ins) {
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

        size_t returnValueRegister = 0;
        if(ins->retType()->registerType()== ResultType::StructAddress){
            prepareReturnValue(ins->retType()->size(), ins);
        } else if (ins->retType()->registerType() == ResultType::Double){
            returnValueRegister = getFReg(ins, ins);
        }else if (ins->retType()->registerType() == ResultType::Integer) {
            returnValueRegister = getReg(ins, ins);

        }else{//void

        }
////        regAllocator->spillCallReg();
//        //call
//        tiny::t86::Label callLabel = add(tiny::t86::CALL{tiny::t86::Label::empty()}, ins);
        tiny::t86::Label callLabel = addF(
                LMBS tiny::t86::CALL{tiny::t86::Label::empty()} LMBE
                , ins );

        // -** manage return Value
        if(ins->retType()->registerType() == ResultType::Double){
//            auto freg = getFReg(ins, ins);
            addF( LMBS tiny::t86::MOV( vFR(returnValueRegister), tiny::t86::FReg(0)) LMBE, ins);
        }else if (ins->retType()->registerType() == ResultType::Integer){
//            auto reg = getReg(ins, ins);
            addF( LMBS tiny::t86::MOV( vR(returnValueRegister), tiny::t86::Reg(0)) LMBE, ins);
        } else if(ins->retType()->registerType() == ResultType::Void){

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

        addF( LMBS tiny::t86::ADD(tiny::t86::Sp(), argSize) LMBE, ins); // instead of popping function args
//
        program_.registerCall(ins, callLabel, ins->f()->name());
//        lastIns_ = ret; //return ret;

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
            default:
                throw "not implemented Copy";
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
                    default:
                        throw "not implemented BinOpType or not supported";
                        break;
                };

                regAssigner_->replaceIntReg(ins->lhs(), ins);
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
                        throw "not implemented BinOpType or not supported";
                        break;
                };

//                regAllocator->replaceFloat(ins->lhs(), ins);
                regAssigner_->replaceFloatReg(ins->lhs(), ins);
                //clearFloatReg(ins->rhs());
                return;

            }
//            case ResultType::Void:
            default:
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
//                        if(auto load = dynamic_cast<Load*>(ins->operand())){
//                            auto regAddr = getReg(load->address(), ins);
//                            //store incremented -- TODO make dirty reg
//                            addF(LMBS tiny::t86::MOV(tiny::t86::Mem(vR(regAddr)), vR(reg))LMBE, ins);
//                        }
                        break;
                    case UnOpType::DEC:
                        addF( LMBS tiny::t86::DEC( vR(reg)) LMBE, ins);
//                        if(auto load = dynamic_cast<Load*>(ins->operand())){
//                            auto regAddr = getReg(load->address(), ins);
//                            //store incremented -- TODO make dirty reg
//                            addF(LMBS tiny::t86::MOV(tiny::t86::Mem(vR(regAddr)), vR(reg))LMBE, ins);
//                        }
                        break;
                    default:
                        throw "not implemented UnOpType or not supported";
                        break;
                }
//                regAllocator->replaceInt(ins->operand(), ins);
                regAssigner_->replaceIntReg(ins->operand(), ins);

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
                regAssigner_->replaceIntReg(ins->operand(), ins);
                return;
            }
            case ResultType::Void:
                throw "not implemented";
                break;
            default:
                throw "not implemented UnOpType or not supported Result Type";
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
                return;
                break;
            }
            case ResultType::Double:{
                auto freg = getFReg(ins, ins);
                addF( LMBS tiny::t86::MOV( vFR(freg), ins->valueFloat() ) LMBE , ins);
                return;
                break;
            }
            case ResultType::StructAddress:
                throw "ERROR cant load StructAddress as value";
                break;
            case ResultType::Void:
                throw "ERROR cant load void as value";
                break;
        }
        throw "[SuperNaiveIs] escaped LoadImm switch";

    }

    void SuperNaiveIS::compileAlloc(Instruction::ImmSize * ins, int cpy){
        if(cpy == -1){
            cpy = regAssigner_->getAllocOffset(ins);
        }
        if(auto allocL = dynamic_cast<AllocL *>(ins)){
            auto reg = getReg(ins, ins);
            addF(LMBS tiny::t86::MOV( vR(reg), tiny::t86::Bp()) LMBE, ins);
            addF(LMBS tiny::t86::SUB( vR(reg), (int64_t) cpy ) LMBE , ins);
        }else if (auto allocG = dynamic_cast<AllocG*>(ins)){
            auto reg = getReg(ins, ins);
            addF(LMBS tiny::t86::MOV( vR(reg), (int64_t) cpy ) LMBE , ins);
        }else{
            throw CSTR("[SuperNaiveIS] cannot compile " << ins->name() << "as alloc instruction") ;
        }

}

    void SuperNaiveIS::visit(AllocL *ins) {
//        regAllocator->makeLocalAllocation(ins->size(), reg, ins);
       auto cpy = makeLocalAllocation(ins->size(), ins);
        if(ins->amount()){
            compileAlloc(ins, cpy);
        }
    }

    void SuperNaiveIS::visit(AllocG *ins) {
        //should be compiled insive globals

    }

    void SuperNaiveIS::visit(ArgAddr *ins) {
        size_t reg = getReg(ins, ins);
//        size_t offset = countArgOffset(ins->args(), ins->index());
        size_t offset =  ins->index();
        addF( LMBS tiny::t86::MOV( vR(reg), tiny::t86::Bp() ) LMBE, ins);
        addF( LMBS tiny::t86::ADD(vR(reg), offset  + 2) LMBE, ins);
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

    void SuperNaiveIS::visit(Load *load) {
        switch (load->resultType()){
            case ResultType::Double: {
                if (dynamic_cast<AllocL *>(load->address())) {
                    auto freg = getFReg(load, load);
//            auto regAddr = getReg(ins->address(), ins);
                    int64_t addrOffset = regAssigner_->getAllocOffset(load->address());
                    addF(LMBS tiny::t86::MOV(vFR(freg), tiny::t86::Mem(tiny::t86::Bp() - addrOffset))LMBE, load);
                }else if(dynamic_cast<AllocG *>(load->address())){
                    auto freg = getFReg(load, load);
//            auto regAddr = getReg(ins->address(), ins);
                    uint64_t addrOffset = regAssigner_->getAllocOffset(load->address());
                    addF(LMBS tiny::t86::MOV(vFR(freg), tiny::t86::Mem(addrOffset))LMBE, load);

                } else {
                    auto freg = getFReg(load, load);
                    auto regAddr = getReg(load->address(), load);
//                    int64_t addrOffset = regAssigner->getAllocOffset(ins->address());
                    addF(LMBS tiny::t86::MOV(vFR(freg), tiny::t86::Mem(vR(regAddr)))LMBE, load);

                }
                return;
                break;
            }case ResultType::Integer: {
//        }else if (ins->resultType() == ResultType::Integer){

//                auto it = program_.globalFindValue(ins->address()); TODO how with globals
//                if (it != program_.globalEndValue()) {
//                    auto reg = getReg(ins, ins);
//                    addF(LMBS tiny::t86::MOV(vR(reg), (int64_t) it->second)LMBE, ins);
//                    return;
//                }
//                it = program_.globalFindAddress(ins->address());
//                if (it != program_.globalEndAddress()){
//                    auto reg = getReg(ins, ins);
//                    addF(LMBS tiny::t86::MOV(vR(reg), (int64_t) it->second)LMBE, ins);
//                    return;
//                }
                if (dynamic_cast<Type::Array *>(load->type())) { // TODO
                    auto reg = getReg(load, load);
                    auto regAddr = getReg(load->address(), load);
                    addF(LMBS tiny::t86::MOV(vR(reg), vR(regAddr))LMBE, load);
                    return;

                } else {
                    if (dynamic_cast<AllocL *>(load->address())) {
                        auto reg = getReg(load, load);
                        //                auto regAddr = getReg(ins->address(), ins);
                        int64_t addrOffset = regAssigner_->getAllocOffset(load->address());
                        addF(LMBS tiny::t86::MOV(vR(reg), tiny::t86::Mem(tiny::t86::Bp() - addrOffset))LMBE, load);
                    }else if(dynamic_cast<AllocG *>(load->address())){
                        auto reg = getReg(load, load);
                        //                auto regAddr = getReg(ins->address(), ins);
                        uint64_t addrOffset = regAssigner_->getAllocOffset(load->address());
                        addF(LMBS tiny::t86::MOV(vR(reg), tiny::t86::Mem(addrOffset))LMBE, load);

                    } else {
                        auto reg = getReg(load, load);
                        auto regAddr = getReg(load->address(), load);
                        //                    int64_t addrOffset = regAssigner->getAllocOffset(ins->address());
                        addF(LMBS tiny::t86::MOV(vR(reg), tiny::t86::Mem(vR(regAddr)))LMBE, load);

                    }
                }
                return;
                break;
            }
            case ResultType::StructAddress: {

                //        }else if (ins->resultType() == ResultType::StructAddress){
                //            regAllocator->replaceInt(ins->address(), ins);
                regAssigner_->replaceIntReg(load->address(), load);
                return;
                break;
            }

            default:
                throw "not implemented Load ResultType or not supported";
                break;
        }
        throw "ERROR[IS] failed load";
    }

    void SuperNaiveIS::visit(Store *store) {
        switch (store->value()->resultType()) {
            case ResultType::Double: {
//                auto regAddr = getReg(ins->address(), ins);
//                int64_t addrOffset = regAssigner_->getAllocOffset(store->address());
//                auto regValue = getFReg(store->value(), store);
//                addF(LMBS tiny::t86::MOV(Mem(tiny::t86::Bp() - addrOffset), vFR(regValue))LMBE, store);
//
                    // Storing double cannot be address
//
                if (dynamic_cast<AllocL *>(store->address())) {

//                  auto regAddr = getReg(ins->address(), ins);
                    int64_t addrOffset = regAssigner_->getAllocOffset(store->address());
                    auto regValue = getFReg(store->value(), store);
                    addF(LMBS tiny::t86::MOV(Mem(tiny::t86::Bp() - addrOffset), vFR(regValue))LMBE, store);
                } else if (dynamic_cast<AllocG *>(store->address())) {
                    uint64_t addrOffset = regAssigner_->getAllocOffset(store->address());
                    auto regValue = getFReg(store->value(), store);
                    addF(LMBS tiny::t86::MOV(tiny::t86::Mem(addrOffset), vFR(regValue))LMBE, store);
                } else {
                    auto regAddr = getReg(store->address(), store);
                    auto regValue = getFReg(store->value(), store);
                    addF(LMBS tiny::t86::MOV(Mem(vR(regAddr)), vFR(regValue))LMBE, store);

                }
                return;
            }
            case ResultType::StructAddress:
            case ResultType::Integer: {
                if (dynamic_cast<AllocL *>(store->address())) {
                    if(auto allocLValue = dynamic_cast<AllocL*>(store->value())){

                        //AllocL as aa value needs to be compiled
                        if(!allocLValue->amount())compileAlloc(allocLValue);

                        int64_t addrOffset = regAssigner_->getAllocOffset(store->address());
                        auto regValue = getReg(store->value(), store);
                        int64_t valueOffset = regAssigner_->getAllocOffset(allocLValue);
                        addF(LMBS tiny::t86::MOV(vR(regValue), tiny::t86::Bp())LMBE, store);
                        addF(LMBS tiny::t86::SUB(vR(regValue), valueOffset)LMBE, store);
                        addF(LMBS tiny::t86::MOV(Mem(tiny::t86::Bp() - addrOffset), vR(regValue))LMBE, store);

                    }else if(auto allocGValue = dynamic_cast<AllocG*>(store->value())){

                        //allocG as a value needs to be in register
                        if(!allocGValue->amount())compileAlloc(allocGValue);
                        int64_t addrOffset = regAssigner_->getAllocOffset(store->address());
//                        auto regValue = getReg(store->value(), store);
                        int64_t valueOffset = regAssigner_->getAllocOffset(allocGValue);
                        addF(LMBS tiny::t86::MOV(Mem(tiny::t86::Bp() - addrOffset), valueOffset)LMBE, store);

                    }else{
                        int64_t addrOffset = regAssigner_->getAllocOffset(store->address());
                        auto regValue = getReg(store->value(), store);
                        addF(LMBS tiny::t86::MOV(Mem(tiny::t86::Bp() - addrOffset), vR(regValue))LMBE, store);
                    }
                } else if (dynamic_cast<AllocG *>(store->address())) {
                    if(auto allocLValue = dynamic_cast<AllocL*>(store->value())){

                        if(!allocLValue->amount())compileAlloc(allocLValue);
                        uint64_t addrOffset = regAssigner_->getAllocOffset(store->address());
                        auto regValue = getReg(store->value(), store);
                        int64_t valueOffset = regAssigner_->getAllocOffset(allocLValue);
                        addF(LMBS tiny::t86::MOV( vR(regValue), tiny::t86::Bp())LMBE, store);
                        addF(LMBS tiny::t86::SUB( vR(regValue), valueOffset)LMBE, store);
                        addF(LMBS tiny::t86::MOV(tiny::t86::Mem((uint64_t) addrOffset), vR(regValue))LMBE, store);

                    }else if(auto allocGValue = dynamic_cast<AllocG*>(store->value())){

                        if(!allocGValue->amount())compileAlloc(allocGValue);
                        uint64_t addrOffset = regAssigner_->getAllocOffset(store->address());
                        uint64_t addrValue = regAssigner_->getAllocOffset(store->value());
                        addF(LMBS tiny::t86::MOV(tiny::t86::Mem((uint64_t) addrOffset), addrValue)LMBE, store);

                    }else{
                        uint64_t addrOffset = regAssigner_->getAllocOffset(store->address());
                        auto regValue = getReg(store->value(), store);
                        addF(LMBS tiny::t86::MOV(tiny::t86::Mem((uint64_t) addrOffset), vR(regValue))LMBE, store);
                    }
                } else {
                    if(auto allocLValue = dynamic_cast<AllocL*>(store->value())){

                        if(!allocLValue->amount())compileAlloc(allocLValue);
                        auto regAddr = getReg(store->address(), store);
                        auto regValue = getReg(store->value(), store);
                        int64_t valueOffset = regAssigner_->getAllocOffset(allocLValue);
                        addF(LMBS tiny::t86::MOV(vR(regValue), tiny::t86::Bp())LMBE, store);
                        addF(LMBS tiny::t86::SUB(vR(regValue), valueOffset)LMBE, store);
                        addF(LMBS tiny::t86::MOV(Mem(vR(regAddr)), vR(regValue))LMBE, store);

                    }else if(auto allocGValue = dynamic_cast<AllocG*>(store->value())){

                        if(!allocGValue->amount())compileAlloc(allocGValue);
                        auto regAddr = getReg(store->address(), store);
//                        auto regValue = getReg(store->value(), store);
                        int64_t valueOffset = regAssigner_->getAllocOffset(allocGValue);
                        addF(LMBS tiny::t86::MOV(Mem(vR(regAddr)), valueOffset)LMBE, store);

                    }else{
                        auto regAddr = getReg(store->address(), store);
                        auto regValue = getReg(store->value(), store);
                        addF(LMBS tiny::t86::MOV(Mem(vR(regAddr)), vR(regValue))LMBE, store);
                    }

                }
                return;
            }
            default: {
                throw "ERROR[IS] Store: failed to store void (or other)value";
            }
        }
        throw "ERROR[IS] Store: failed to resolve value type";
    }

    void SuperNaiveIS::visit(Phi *ins) {
        regAssigner_->registerPhi(ins);
    }

    void SuperNaiveIS::visit(ElemAddrOffset *ins) {
        //access to structure
        if (dynamic_cast<AllocL *>(ins->base())) {
            auto reg = getReg(ins, ins);
//            auto regAddr = getReg(ins->address(), ins);
            int64_t baseOffset = regAssigner_->getAllocOffset(ins->base());
            auto regOffset = getReg(ins->offset(), ins);
//            addF(LMBS tiny::t86::MOV(vFR(freg), tiny::t86::Mem(tiny::t86::Bp() - addrOffset))LMBE, load);
            addF( LMBS tiny::t86::MOV(vR(reg),
                                      tiny::t86::Bp()  ) LMBE, ins);
            addF( LMBS tiny::t86::SUB(vR(reg), baseOffset ) LMBE, ins);
            addF( LMBS tiny::t86::ADD(vR(reg),
                                      vR(regOffset)) LMBE, ins);
        }else if(dynamic_cast<AllocG *>(ins->base())){
            auto reg = getReg(ins, ins);
//            auto regAddr = getReg(ins->address(), ins);
            int64_t baseOffset = regAssigner_->getAllocOffset(ins->base());
            auto regOffset = getReg(ins->offset(), ins);
//            addF(LMBS tiny::t86::MOV(vFR(reg), tiny::t86::Mem(addrOffset))LMBE, ins);
            addF( LMBS tiny::t86::MOV(vR(reg), baseOffset  ) LMBE, ins);
            addF( LMBS tiny::t86::ADD(vR(reg),// Add or Sub? need same -> ADD
                                      vR(regOffset)) LMBE, ins);

        }else{

            auto reg = getReg(ins, ins);
            auto regBase = getReg(ins->base(), ins);

            auto regOffset = getReg(ins->offset(), ins);
            addF( LMBS tiny::t86::MOV(vR(reg),
                                      vR(regBase)  ) LMBE, ins);
            addF( LMBS tiny::t86::ADD(vR(reg), // Add or Sub? need same -> ADD
                               vR(regOffset)) LMBE, ins);
        }

        //clearIntReg(ins->offset());
    }

    void SuperNaiveIS::visit(ElemAddrIndex *ins) {
        // access to array
        if (dynamic_cast<AllocL *>(ins->base())) {
            auto reg = getReg(ins, ins);
//            auto regAddr = getReg(ins->address(), ins);
            auto regIndex = getReg(ins->index(), ins);
            int64_t baseOffset = regAssigner_->getAllocOffset(ins->base());
            auto regOffset = getReg(ins->offset(), ins);
//        addF( LMBS tiny::t86::MOV(vR(reg),
//                           vR(regBase)) LMBE, ins);
//            //clearIntReg(ins->offset());
//            addF(LMBS tiny::t86::MUL(vR(regOffset),
//                                     vR(regIndex))LMBE, ins);
//            addF(LMBS tiny::t86::MOV(vR(reg),
//                                     tiny::t86::Bp())LMBE, ins);
//            addF(LMBS tiny::t86::SUB(vR(reg), baseOffset)LMBE, ins);
//            addF(LMBS tiny::t86::ADD(vR(reg), vR(regOffset))LMBE, ins);
//clearIntReg(ins->offset());
            addF(LMBS tiny::t86::MUL(vR(regOffset),
                                     vR(regIndex))LMBE, ins);
            addF(LMBS tiny::t86::MOV(vR(reg),
                                     tiny::t86::Mem(tiny::t86::Bp() - baseOffset))LMBE, ins);
//            addF(LMBS tiny::t86::SUB(vR(reg), baseOffset)LMBE, ins);
            addF(LMBS tiny::t86::ADD(vR(reg), vR(regOffset))LMBE, ins);

        }else if(dynamic_cast<AllocG *>(ins->base())){
            auto reg = getFReg(ins, ins);
            auto regIndex = getReg(ins->index(), ins);
            uint64_t baseOffset = regAssigner_->getAllocOffset(ins->base());
            auto regOffset = getReg(ins->offset(), ins);
////            addF(LMBS tiny::t86::MOV(vFR(reg), tiny::t86::Mem(addrOffset))LMBE, ins);
//            addF(LMBS tiny::t86::MUL(vR(regOffset),
//                                     vR(regIndex))LMBE, ins);
//            addF( LMBS tiny::t86::MOV(vR(reg), baseOffset  ) LMBE, ins);
//            addF( LMBS tiny::t86::ADD(vR(reg),
//                                      vR(regOffset)) LMBE, ins);
addF(LMBS tiny::t86::MUL(vR(regOffset),
                                     vR(regIndex))LMBE, ins);
            addF( LMBS tiny::t86::MOV(vR(reg), tiny::t86::Mem(baseOffset)  ) LMBE, ins);
            addF( LMBS tiny::t86::ADD(vR(reg),
                                      vR(regOffset)) LMBE, ins);

        }else {
            auto reg = getReg(ins, ins);
//        auto regBase = getReg(ins->base(), ins);
            auto regIndex = getReg(ins->index(), ins);
            auto regBaseOffset = getReg(ins->base(), ins);
//            int64_t baseOffset = regAssigner_->getAllocOffset(ins->base());

            auto regOffset = getReg(ins->offset(), ins);
//        addF( LMBS tiny::t86::MOV(vR(reg),
//                           vR(regBase)) LMBE, ins);
//            //clearIntReg(ins->offset());
//            addF(LMBS tiny::t86::MUL(vR(regOffset),
//                                     vR(regIndex))LMBE, ins);
//            addF(LMBS tiny::t86::MOV(vR(reg),
//                                     tiny::t86::Bp())LMBE, ins);
//            addF(LMBS tiny::t86::SUB(vR(reg), baseOffset)LMBE, ins);
//            addF(LMBS tiny::t86::ADD(vR(reg), regOffset)LMBE, ins);
//            //clearIntReg(ins->index());
//            // clearIntReg(ins->offset());
            addF(LMBS tiny::t86::MUL(vR(regOffset),
                                     vR(regIndex))LMBE, ins);
            addF(LMBS tiny::t86::MOV(vR(reg),
                                     vR(regBaseOffset) )LMBE, ins);
//            addF(LMBS tiny::t86::SUB(vR(reg), baseOffset)LMBE, ins);
            addF(LMBS tiny::t86::SUB(vR(reg), vR(regOffset))LMBE, ins);
            //clearIntReg(ins->index());
        }
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
    void SuperNaiveIS::copyStruct(size_t from, Type *type, Register to, const Instruction * ins) {

//        auto tmpReg = regAssigner->getFreeIntRegister();
        auto tmpReg = getExtraIntReg(ins);

        Type::Struct * strct =  dynamic_cast<Type::Struct *>(type);
        for (int i = 0; i < strct->size(); ++i) {
            addF( LMBS tiny::t86::MOV( vR(tmpReg), tiny::t86::Mem( vR(from) + i )) LMBE, ins);
            addF( LMBS tiny::t86::MOV(tiny::t86::Mem(to + i), tmpReg) LMBE, ins);
        }

//        clearTmpIntRegister(tmpReg); tmpReg = -15654;
    }

    void SuperNaiveIS::visit(StructAssign *ins) {
        auto regVal = getReg(ins->srcVal(), ins);
        auto dstVal = getReg(ins->dstAddr(), ins);
        copyStruct(regVal, ins->type(),dstVal , ins);
    }

    void SuperNaiveIS::visit(BasicBlock *bb) {
        auto insns = getBBsInstructions(bb);
        for(int64_t i = 0; i < insns.size() ;i++){
            visitChild(insns[i]);
        }
    }

    void SuperNaiveIS::visit(Function *fce) {
        regAssigner_->resetAllocSize();
        for (const auto & bb : getFunctionBBs(fce)) {
            visitChild(bb);
        }
        regAssigner_->exportAlloc(fce);

    }

    void SuperNaiveIS::visit(Program *p) {


        //===================================GLOBALS=====================================
//        makeGlobalTable(irb->globals_.get());
        auto globs = getProgramsGlobals(p);
        makeGlobalTable(globs);
//        for (const auto & str : p->stringLiterals() ) {
//            tiny::t86::DataLabel data = program_.addData(str.second);
//            auto reg = getReg(str.first, str.first);
//            addF(LMBS tiny::t86::MOV( vR(reg), data ) LMBE, str.first);
//        }

        //===================================FUNCTIONS=====================================
        for (auto & i : p->functions()){
            visitChild(i.second.get());
        }


        //        regAllocator->clearAllReg();
                compileGlobalTable(globs);

    }
    void SuperNaiveIS::makeGlobalTable(BasicBlock * globals) {

        for( auto *ins : getBBsInstructions(globals)){
            if(const auto * i = dynamic_cast<const  LoadImm *>(ins)){
                program_.globalEmplaceValue(ins, i->valueInt());
            }else if(const auto * alloc = dynamic_cast< const AllocG *>(ins)){
                auto cpy = regAssigner_->makeGlobalAllocation(alloc->size(), alloc);
                if(alloc->amount()){
                    auto strLit = program_.program_->stringLiterals().find(ins);
                    if(strLit != program_.program_->stringLiterals().end()) {
                        tiny::t86::DataLabel data = program_.addData(strLit->second);
                        program_.globalEmplaceAddress(alloc, data);
                    }else{
                        throw "[SuperNaiveIS]global array not implemented";
                    }
//                    auto reg = getReg(ins, ins);
//                    addF(LMBS tiny::t86::MOV( vR(reg), tiny::t86::Bp()) LMBE, ins);
//                    addF(LMBS tiny::t86::MOV( vR(reg), (int64_t) cpy ) LMBE , ins);
                }else{
                    switch(alloc->resultType()){
                        case ResultType::Double:{
                            tiny::t86::DataLabel label = program_.addData(0);
                            program_.globalEmplaceAddress(alloc, label);
                            throw "global Double compilation not implemented ";
                            break;
                        }
                        case ResultType::Integer:{
                            tiny::t86::DataLabel label = program_.addData(0);
                            program_.globalEmplaceAddress(alloc, label);
                            break;
                        }
                        case ResultType::StructAddress:
                            throw "global Struct compilation not implemented ";
                            break;
                        case ResultType::Void:
                            throw "global Void compilation not implemented ";
                            break;
                        default:
                            throw tiny::ParserError("invalid type of global allocation", ins->ast()->location());
                            break;
                    }
                }
            }else if(const auto * alloc = dynamic_cast< const AllocL *>(ins)/**TODO*/){
                regAssigner_->makeLocalAllocation(alloc->size(), alloc);
                switch(alloc->resultType()){
                    case ResultType::Double:{
                        tiny::t86::DataLabel label = program_.addData(0);
                        program_.globalEmplaceAddress(alloc, label);
                        throw "global Double compilation not implemented ";
                        break;
                    }
                    case ResultType::Integer:{
                        tiny::t86::DataLabel label = program_.addData(0);
                        program_.globalEmplaceAddress(alloc, label);
                        break;
                    }
                    case ResultType::StructAddress:
                        throw "global Struct compilation not implemented ";
                        break;
                    case ResultType::Void:
                        throw "global Void compilation not implemented ";
                        break;
                    default:
                        throw tiny::ParserError("invalid type of global allocation", ins->ast()->location());
                        break;
                }
            }else if(const auto * load = dynamic_cast< const Load *>(ins)){
                load->address();
                //TODO implement
                switch(load->resultType()){
                    case ResultType::Double: {
                        auto val = program_.globalFindAddress(load->address());
                        if (val == program_.globalEndAddress()) {
                            throw "uninitialized Global?";
                        }
                        program_.globalEmplaceValue(load, val->second);
                        break;
                    }
                    case ResultType::Integer:{
                                auto val = program_.globalFindAddress(load->address());
                                if (val == program_.globalEndAddress()){
                                    throw "uninitialized Global?";
                                }
                                program_.globalEmplaceValue(load, val->second);
                                break;
                        }
                    case ResultType::StructAddress:
                        throw "[SuperNaiveIS] - global make * load structAddress not implemented";
                        break;
                    default:
                        throw "[SuperNaiveIS] - global make * cannot load VoidType";
                        break;
                }

            }else if( const auto  * store = dynamic_cast<const Store *>(ins)){
                auto val = program_.globalFindValue(store->value());
                if(val == program_.globalEndValue()){
                    auto aval = program_.globalFindAddress(store->value());
                    if(aval == program_.globalEndAddress()){
                        throw "[SuperNaiveIS]uninitialized Global?";
                    }
                }
                program_.globalEmplaceAddress(store->address(),  val->second);
            }else{
                throw "[Super NaiveIS] unknown global instruction - make";
            }

        }

    }


    void SuperNaiveIS::compileGlobalTable(BasicBlock * globals) {

        for( auto *ins : getBBsInstructions(globals)){
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
                    size_t size = alloc->size();
                    if(auto imm  = dynamic_cast<LoadImm *>(alloc->amount())){
                        auto strLit = program_.program_->stringLiterals().find(ins);
                        if(strLit != program_.program_->stringLiterals().end()){
//                            tiny::t86::DataLabel data = program_.addData(strLit->second);
                            auto data = getGlobalTable(program_).find(alloc);
                            if(data != getGlobalTable(program_).end()){
                                auto reg = getReg(strLit->first, strLit->first);
                                int64_t dataValue = data->second;
                                addF(LMBS tiny::t86::MOV( vR(reg), dataValue ) LMBE, strLit->first);

                            }else{
                                throw "[SuperNaiveIS]allocG with global array as a string not prepared for global table";

                            }

                        }else{
                            throw "[SuperNaiveIS]allocG with global array -- not implemented";

                        }
                        //already done by str literals
                    }else{
                        throw "[SuperNaiveIS]allocG with array size of variable type not implemented"; //TODO global array

                    }

                }else{
//                    regAssigner_->makeGlobalAllocation(alloc->size(), alloc); -- already in make

                }
            }else if(const auto * alloc = dynamic_cast< const AllocL *>(ins)){
                if(alloc->amount()){
                    throw "[SuperNaiveIS]allocL in globals with array not possible";
                }else{
//                    regAssigner_->makeLocalAllocation(alloc->size(), alloc); //Already in make
                }
            }else if(const auto * load = dynamic_cast< const Load *>(ins)){
                switch (load->resultType()){
                    case ResultType::Double: {
                        if (dynamic_cast<AllocL *>(load->address())) {
                            auto freg = getFReg(load, load);
//            auto regAddr = getReg(ins->address(), ins);
                            int64_t addrOffset = regAssigner_->getAllocOffset(load->address());
                            addF(LMBS tiny::t86::MOV(vFR(freg), tiny::t86::Mem(tiny::t86::Bp() - addrOffset))LMBE, load);
                        }else if(dynamic_cast<AllocG *>(load->address())){
                            auto freg = getFReg(load, load);
//            auto regAddr = getReg(ins->address(), ins);
                            uint64_t addrOffset = regAssigner_->getAllocOffset(load->address());
                            addF(LMBS tiny::t86::MOV(vFR(freg), tiny::t86::Mem(addrOffset))LMBE, load);

                        } else {
                            auto freg = getFReg(load, load);
                            auto regAddr = getReg(load->address(), load);
//                    int64_t addrOffset = regAssigner->getAllocOffset(ins->address());
                            addF(LMBS tiny::t86::MOV(vFR(freg), tiny::t86::Mem(regAddr))LMBE, load);

                        }
                        return;
                        break;
                    }case ResultType::Integer: {
//        }else if (ins->resultType() == ResultType::Integer){

//                auto it = program_.globalFindValue(ins->address()); TODO how with globals
//                if (it != program_.globalEndValue()) {
//                    auto reg = getReg(ins, ins);
//                    addF(LMBS tiny::t86::MOV(vR(reg), (int64_t) it->second)LMBE, ins);
//                    return;
//                }
//                it = program_.globalFindAddress(ins->address());
//                if (it != program_.globalEndAddress()){
//                    auto reg = getReg(ins, ins);
//                    addF(LMBS tiny::t86::MOV(vR(reg), (int64_t) it->second)LMBE, ins);
//                    return;
//                }
                        if (dynamic_cast<Type::Array *>(load->type())) { // TODO
                            auto reg = getReg(load, load);
                            auto regAddr = getReg(load->address(), load);
                            addF(LMBS tiny::t86::MOV(vR(reg), vR(regAddr))LMBE, load);
                            return;

                        } else {
                            if (dynamic_cast<AllocL *>(load->address())) {
                                auto reg = getReg(load, load);
                                //                auto regAddr = getReg(ins->address(), ins);
                                int64_t addrOffset = regAssigner_->getAllocOffset(load->address());
                                addF(LMBS tiny::t86::MOV(vR(reg), tiny::t86::Mem(tiny::t86::Bp() - addrOffset))LMBE, load);
                            }else if(dynamic_cast<AllocG *>(load->address())){
                                auto reg = getReg(load, load);
                                //                auto regAddr = getReg(ins->address(), ins);
                                uint64_t addrOffset = regAssigner_->getAllocOffset(load->address());
                                addF(LMBS tiny::t86::MOV(vR(reg), tiny::t86::Mem(addrOffset))LMBE, load);

                            } else {
                                auto reg = getReg(load, load);
                                auto regAddr = getReg(load->address(), load);
                                //                    int64_t addrOffset = regAssigner->getAllocOffset(ins->address());
                                addF(LMBS tiny::t86::MOV(vR(reg), tiny::t86::Mem(vR(regAddr)))LMBE, load);

                            }
                        }
                        return;
                        break;
                    }
                    case ResultType::StructAddress: {

                        //        }else if (ins->resultType() == ResultType::StructAddress){
                        //            regAllocator->replaceInt(ins->address(), ins);
                        regAssigner_->replaceIntReg(load->address(), load);
                        return;
                        break;
                    }

                    default:
                        throw "ERROR[global IS] not implemented Load ResultType or not supported";
                        break;
                }

            }else if( const auto  * store = dynamic_cast<const Store *>(ins)){
                uint64_t value = program_.globalFindValue(store->value())->second;
                uint64_t address = program_.globalFindAddress(store->address())->second;
                switch (store->value()->resultType()) {
                    case ResultType::Double:{
////                auto regAddr = getReg(ins->address(), ins);
//                        int64_t addrOffset = regAssigner_->getAllocOffset(store->address());
//                        auto regValue = getFReg(store->value(), store);
//                        addF( LMBS tiny::t86::MOV(Mem(tiny::t86::Bp() - addrOffset), vFR(regValue)) LMBE, store);
                        if(dynamic_cast<AllocL*>(store->address())){

//                  auto regAddr = getReg(ins->address(), ins);
                            int64_t addrOffset = regAssigner_->getAllocOffset(store->address());
                            auto regValue = getFReg(store->value(), store);
                            addF( LMBS tiny::t86::MOV(Mem(tiny::t86::Bp() - addrOffset), vFR(regValue)) LMBE, store);
                        }else if(dynamic_cast<AllocG*>(store->address())){
                            uint64_t addrOffset = regAssigner_->getAllocOffset(store->address());
                            auto regValue = getFReg(store->value(), store);
                            addF( LMBS tiny::t86::MOV(tiny::t86::Mem( addrOffset), vFR(regValue)) LMBE, store);
                        }else{
                            auto regAddr = getReg(store->address(), store);
                            auto regValue = getReg(store->value(), store);
                            addF( LMBS tiny::t86::MOV(Mem(vR(regAddr)), vR(regValue)) LMBE, store);

                        }
                        continue;
                    }
                    case ResultType::StructAddress:
                    case ResultType::Integer:{
                        if(dynamic_cast<AllocL*>(store->address())){

//                  auto regAddr = getReg(ins->address(), ins);
                            int64_t addrOffset = regAssigner_->getAllocOffset(store->address());
                            auto regValue = getReg(store->value(), store);
                            addF( LMBS tiny::t86::MOV(Mem(tiny::t86::Bp() - addrOffset), vR(regValue)) LMBE, store);
                        }else if(dynamic_cast<AllocG*>(store->address())){
                            uint64_t addrOffset = regAssigner_->getAllocOffset(store->address());
                            auto regValue = getReg(store->value(), store);
                            addF( LMBS tiny::t86::MOV(tiny::t86::Mem((uint64_t)addrOffset), vR(regValue)) LMBE, store);
                        }else{
                            auto regAddr = getReg(store->address(), store);
                            auto regValue = getReg(store->value(), store);
                            addF( LMBS tiny::t86::MOV(Mem(vR(regAddr)), vR(regValue)) LMBE, store);

                        }
                        continue;
                    }
                    default:{
                        throw "ERROR[IS(global)] Store: failed to store void (or other)value";
                    }
                }
            }else{
                throw tiny::ParserError("unknown global instruction", ins->ast()->location());
            }

        } //end for :> Globals

    }


    TargetProgram SuperNaiveIS::translate(ILBuilder &ilb) {
        auto  prog = std::make_shared<Program>(ilb.finish());
        auto is = SuperNaiveIS(prog);
        is.run();
        return std::move(is.finalize());
//
//        return TargetProgram(is.program_);
    }
    TargetProgram  SuperNaiveIS::translate(TargetProgram && prog) {
        std::shared_ptr<Program> ILProg = prog.program_;
        auto is = SuperNaiveIS(ILProg); //NEED to reset everything ==> keep only ILprogram
//        is.program_ = std::move(prog);
//        is.program_.setProgram(prog.program_ );
//        is.analysisResult_ = is.analysis_->analyze();
//        prog->program_ = nullptr;
        is.run();
        return TargetProgram(std::move(is.finalize()));
//
//        return TargetProgram(is.program_);
    }

    void SuperNaiveIS::visit(Instruction *ins) {

    }

    TargetProgram SuperNaiveIS::finalize() {
//        TargetProgram res(program_);

        //registers are in allocated registers

        return TargetProgram(std::move(program_));
    }

    void SuperNaiveIS::run() {
        visit(getProgram(program_).get());
    }

    std::pair<const CfgNode<> *, std::set<CLiveRange *>>
    SuperNaiveIS::findAnalysisResult(const Instruction *ins) const {
        auto it = analysis_->instr_mapping().find(ins);
        if(it != analysis_->instr_mapping().end()){
               auto res = analysisResult_.find(it->second);
               if(res != analysisResult_.end()){
                   return *res;
               }
        }else{
            throw CSTR("[SuperNaiveIS.cpp] cannot find instruction " << ins->name() <<" in cfg");
        }
        return std::pair<const CfgNode<> *, std::set<CLiveRange *>>();
    }

}