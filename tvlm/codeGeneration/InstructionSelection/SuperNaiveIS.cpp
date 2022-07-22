#include "SuperNaiveIS.h"
#include "tvlm/tvlm/il/il_builder.h"

#define ZeroFlag 0x02
#define SignFlag 0x01

namespace tvlm {

    SuperNaiveIS::SuperNaiveIS():
    pb_(ProgramBuilder())
    , lastIns_(Label::empty())
    , program_(TargetProgram())
//    , globalTable_()
    , unpatchedCalls_()
//                        , regAllocator(new NaiveRegisterAllocator(&pb_))
    , regAssigner(new RegisterAssigner(&pb_, &program_))
    { }

    SuperNaiveIS::~SuperNaiveIS() =  default;

    void SuperNaiveIS::visit(Jump *ins) {
        tiny::t86::Label jmp = add(tiny::t86::JMP( tiny::t86::Label::empty()), ins);
//        future_patch_.emplace_back(jmp, ins->getTarget(1));
//        jump_patches_.emplace_back(std::make_pair(ins, jmp), ins->getTarget(1) );
        program_.patchJump(ins, jmp, ins->getTarget(1));

        lastIns_ = jmp;//        return jmp;
    }

    void SuperNaiveIS::visit(CondJump *ins) {
        auto ret = pb_.currentLabel();

        add(tiny::t86::CMP(getReg(ins->condition()), 0), ins);
//        clearIntReg(ins->condition());
        tiny::t86::Label condJump = add(tiny::t86::JZ(tiny::t86::Label::empty()) ,ins);
        tiny::t86::Label jumpToTrue = add(tiny::t86::JMP(tiny::t86::Label::empty()), ins);
//        future_patch_.emplace_back(jumpToTrue, ins->getTarget(1));
//        jump_patches_.emplace_back(std::make_pair(ins, jumpToTrue), ins->getTarget(1) );
        program_.patchJump(ins, jumpToTrue, ins->getTarget(1)  );
//        future_patch_.emplace_back(/*jumpToFalse*/condJump, ins->getTarget(0));
//        jump_patches_.emplace_back(std::make_pair(ins, condJump), ins->getTarget(0) );
        program_.patchJump(ins, condJump, ins->getTarget(0)  );
        lastIns_ = ret;//        return ret;
    }

    void SuperNaiveIS::visit(Return *ins) {

    }

    void SuperNaiveIS::visit(CallStatic *ins) {

    }

    void SuperNaiveIS::visit(Call *ins) {

    }

    void SuperNaiveIS::visit(Copy *ins) {
        auto ret = pb_.currentLabel();
        add(tiny::t86::MOV(getReg(ins), getReg(ins->src()) ), ins);

        lastIns_ = ret;//        return ret;
    }

    void SuperNaiveIS::visit(Extend *ins) {
        auto ret = pb_.currentLabel();
        add(tiny::t86::EXT(getFReg(ins), getReg(ins->src()) ), ins);
        //clearReg(ins->src());
        lastIns_ = ret;//        return ret;
    }

    void SuperNaiveIS::visit(Truncate *ins) {
        auto ret = pb_.currentLabel();
        add(tiny::t86::NRW(getReg(ins), getFReg(ins->src()) ), ins);
        //clearReg(ins->src());
        lastIns_ = ret;//        return ret;
    }

    void SuperNaiveIS::visit(BinOp *ins) {
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
                        add(tiny::t86::AND(lhsreg, 1), ins);
                        break;
                    case BinOpType::LT:
                        add(tiny::t86::SUB(lhsreg, rhsreg), ins);
                        add(tiny::t86::MOV(lhsreg, tiny::t86::Flags()), ins);
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
                        add(tiny::t86::AND(lhsreg, 1), ins);
                        break;
                };

                regAssigner->replaceIntReg(ins->lhs(), ins);
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
                        auto freeReg =regAssigner->getReg(ins);
                        add(tiny::t86::NRW(freeReg, lhsreg), ins);
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
                        add(tiny::t86::AND(tmpreg, 1), ins);

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
                        add(tiny::t86::AND(tmpreg, 1), ins);

                        //clearFloatReg(ins->rhs());
                        //clearFloatReg(ins->lhs());
                        lastIns_ = ret;//        return ret;
                        return;
                    }
                    default:
                        throw "not implemented or not supported";
                };

//                regAllocator->replaceFloat(ins->lhs(), ins);
                regAssigner->replaceFloatReg(ins->lhs(), ins);
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
        regAssigner->replaceIntReg(ins->rhs(), ins);
        //clearIntReg(ins->rhs());
        lastIns_ = ret;//        return ret;
    }

    void SuperNaiveIS::visit(UnOp *ins) {
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
                regAssigner->replaceIntReg(ins->operand(), ins);

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
                regAssigner->replaceIntReg(ins->operand(), ins);
                lastIns_ = ret;//        return ret;
                return;
            }
            case ResultType::Void:
                throw "not implemented";
                break;
        }
    }

    void SuperNaiveIS::visit(LoadImm *ins) {
        auto ret = pb_.currentLabel();

        switch (ins->resultType()) {
            case ResultType::StructAddress:
            case ResultType::Integer:{

                int64_t value = ins->valueInt();
//                if(!instructionToEmplace.empty() ){
//                    auto it = instructionToEmplace.find(ins);
//                    if( it!= instructionToEmplace.end()){
//                        auto * newIns = dynamic_cast<const LoadImm * >(it->second);
//                        value = newIns->valueInt();
//                    }
//                }
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

    void SuperNaiveIS::visit(AllocL *ins) {
        Label ret = pb_.currentLabel();
        Register reg = getReg(ins);
//        regAllocator->makeLocalAllocation(ins->size(), reg, ins);
        makeLocalAllocation(ins->size(), reg, ins);
        lastIns_ = ret;
    }

    void SuperNaiveIS::visit(AllocG *ins) {

    }

    void SuperNaiveIS::visit(ArgAddr *ins) {
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
    void SuperNaiveIS::visit(PutChar *ins) {
        auto ret = pb_.currentLabel();
        add(tiny::t86::PUTCHAR(getReg(ins->src()) ), ins);
        //clearIntReg(ins->src());
        lastIns_ = ret;
    }

    void SuperNaiveIS::visit(GetChar *ins) {
        auto ret = pb_.currentLabel();
        add(tiny::t86::GETCHAR(getReg(ins) ), ins);
        lastIns_ = ret;
    }

    void SuperNaiveIS::visit(Load *ins) {
        auto ret = pb_.currentLabel();
        if(ins->type()->registerType() == ResultType::Double){
            add(tiny::t86::MOV(getFReg(ins), tiny::t86::Mem(getReg(ins->address()))), ins);
            lastIns_ = ret; //return ret;
            return;
        }else if (ins->type()->registerType() == ResultType::Integer){

            auto it = program_.globalFind(ins->address());
            if(it != program_.globalEnd()){
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
            regAssigner->replaceIntReg(ins->address(), ins);
            lastIns_ = ret; //return ret;
            return;

        }
        throw "ouch?";
    }

    void SuperNaiveIS::visit(Store *ins) {
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

    void SuperNaiveIS::visit(Phi *ins) {
        regAssigner->registerPhi(ins);
    }

    void SuperNaiveIS::visit(ElemAddrOffset *ins) {
        auto ret = pb_.currentLabel();


        add(tiny::t86::MOV(getReg(ins),
                           getReg(ins->base())), ins);
        add(tiny::t86::ADD(getReg(ins),
                           getReg(ins->offset())), ins);

        //clearIntReg(ins->offset());
        lastIns_ = ret;//        return ret;
    }

    void SuperNaiveIS::visit(ElemAddrIndex *ins) {
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

    void SuperNaiveIS::visit(Halt *ins) {
        lastIns_ = add(tiny::t86::HALT(), ins);
    }


    void SuperNaiveIS::copyStruct(const Register &from, Type *type, const Register &to, const Instruction * ins) {

        auto tmpReg = regAssigner->getFreeIntRegister();
        Type::Struct * strct =  dynamic_cast<Type::Struct *>(type);
        for (int i = 0; i < strct->size(); ++i) {
            add(tiny::t86::MOV(tmpReg, tiny::t86::Mem(from + i )), ins);
            add(tiny::t86::MOV(tiny::t86::Mem(to + i), tmpReg), ins);
        }

//        clearTmpIntRegister(tmpReg); tmpReg = -15654;
    }

    void SuperNaiveIS::visit(StructAssign *ins) {
        auto ret = pb_.currentLabel();
        copyStruct(getReg(ins->srcVal()), ins->type(), getReg(ins->dstAddr()), ins);
        lastIns_ = ret;
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
            tiny::t86::DataLabel data = pb_.addData(str.first);
            add(tiny::t86::MOV(getReg(str.second), data ), str.second);
        }

        //===================================FUNCTIONS=====================================
        for (auto & i : p->functions()){
            visitChild(i.second.get());
            Label fncLabel = lastIns_;

        }


        //        regAllocator->clearAllReg();
//        tiny::t86::Label prolog = //t86::Label(lastInstruction_index +1);
                compileGlobalTable(globs);

    }
    void SuperNaiveIS::makeGlobalTable(BasicBlock * globals) {

        for(const auto *ins : getBBsInstructions(globals)){
            if(const auto * i = dynamic_cast<const  LoadImm *>(ins)){
                program_.globalEmplace(ins, i->valueInt());
            }else if(const auto * alloc = dynamic_cast< const AllocG *>(ins)){
                if(alloc->resultType() == ResultType::Double){
                    tiny::t86::DataLabel label = pb_.addData(0);
                    program_.globalEmplace(alloc, label);
                    throw "global Double compilation not implemented ";
                }else if(alloc->resultType() == ResultType::Integer){
                    tiny::t86::DataLabel label = pb_.addData(0);
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
                    add(tiny::t86::MOV( getReg(ins), i->valueInt()), ins);
                }else if (i->resultType() == ResultType::Double){
                    add(tiny::t86::MOV( getFReg(ins), i->valueFloat()), ins);
                }else{
                    throw "load imm with Res Type Structure or Void not implemented";
                }
            }else if(const auto * alloc = dynamic_cast< const AllocG *>(ins)){
                if(alloc->amount()){
                    throw "allocG with array not implemented"; //TODO global array
                }else{
                    regAssigner->makeGlobalAllocation(alloc->size(), getReg(alloc), alloc);
                }
            }else if( const auto  * store = dynamic_cast<const Store *>(ins)){
                uint64_t value = program_.globalFind(store->value())->second;
                uint64_t address = program_.globalFind(store->address())->second;
                add(tiny::t86::MOV( tiny::t86::Reg(0), (int64_t)address), ins);
                add(tiny::t86::MOV(Mem(tiny::t86::Reg(0)), (int64_t)value), ins);
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
//TODO
        return TargetProgram(is.program_);
    }

    void SuperNaiveIS::visit(Instruction *ins) {

    }

}