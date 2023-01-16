#include "il.h"
#include "il_builder.h"
#include "tvlm_backend"

#include <cmath>

namespace tvlm {
    using Backend = t86_Backend; //TODO Modular use macros

    void Instruction::Terminator1::print(tiny::ASTPrettyPrinter &p) const {
        Instruction::print(p);
        p << p.keyword << instrName_ << " " << p.identifier << target_->name();

    }

    void Instruction::Terminator1::printAlloc(tiny::ASTPrettyPrinter &p) const {
        Instruction::printAlloc(p);
        p << p.keyword << instrName_ << " " << p.identifier << target_->name();

    }

    Instruction::Terminator1::Terminator1(BasicBlock *target, const ASTBase *ast, const std::string &instrName,
                                          Opcode opcode) :
            Terminator{ast, instrName, opcode},
            target_{target} {

        target_->registerUsage(this);
    }

    bool Instruction::Terminator1::operator==(const IL *il) const {
        if( auto * other = dynamic_cast<const Instruction::Terminator1*>(il)){
            return target_->operator==(other->target_);
        }
        else return false;
    }

    void Instruction::Terminator2::print(tiny::ASTPrettyPrinter &p) const {
        Instruction::print(p);
        p << p.keyword << instrName_ << " " << p.identifier;
        printRegister(p, cond_);
        for(auto & t : targets_){
            p << p.identifier << t->name() << " ";
        }
    }

    void Instruction::Terminator2::printAlloc(tiny::ASTPrettyPrinter &p) const {
        Instruction::printAlloc(p);
        p << p.keyword << instrName_ << " " << p.identifier;
        printAllocRegister(p, cond_);
        for(auto & t : targets_){
            p << p.identifier << t->name() << " ";
        }
    }

Instruction::Terminator2::Terminator2(Instruction *cond, BasicBlock * trueTarget,
                                          BasicBlock * falseTarget,  const ASTBase *ast, const std::string &instrName,
                                          Opcode opcode) :
            Terminator{ast, instrName, opcode},
            cond_{cond},
            targets_{falseTarget, trueTarget}{
        cond_->registerUsage(this);
        for (auto * bb: targets_) {
            bb->registerUsage(this);
        }
    }

    bool Instruction::Terminator2::operator==(const IL *il) const{
        if( auto * other = dynamic_cast<const Instruction::Terminator2*>(il)){
            bool tmp = cond_->operator==(other->cond_) ;
            for (int i = 0 ; i < targets_.size(); i++) {
                tmp = tmp && targets_[i]->operator==(other->targets_[i]);
            }
            return tmp;
        }
        else return false;
    }

    void Instruction::DirectCallInstruction::print(tiny::ASTPrettyPrinter &p) const {
        Instruction::print(p);
        p << p.keyword << instrName_ << " " << p.identifier << f_->name().name();
        p << p.symbol << " (";
        for (auto & i : args_)
            printRegister(p, i.first);
        p << p.symbol << ")";
    }

    void Instruction::PhiInstruction::print(tiny::ASTPrettyPrinter &p) const {
        Instruction::print(p);
        p << p.keyword << instrName_ << " ";
        for(auto & i : contents_ ){
            printRegister(p, i.second);
            p <<  "<--" << i.first->name() << ", ";

        }
    };

    void Instruction::DirectCallInstruction::printAlloc(tiny::ASTPrettyPrinter &p) const {
        Instruction::printAlloc(p);
        p << p.keyword << instrName_ << " " << p.identifier << f_->name().name();
        p << p.symbol << " (";
        for (auto & i : args_)
            printAllocRegister(p, i.first);
        p << p.symbol << ")";
    }

    void Instruction::PhiInstruction::printAlloc(tiny::ASTPrettyPrinter &p) const {
        Instruction::printAlloc(p);
        p << p.keyword << instrName_ << " ";
        for(auto & i : contents_ ){
            printAllocRegister(p, i.second);
            p <<  "<--" << i.first->name() << ", ";

        }
    };



    Instruction::DirectCallInstruction::DirectCallInstruction(Function *f, std::vector<std::pair< Instruction *, Type*>> &&args,
                                                              const ASTBase *ast, const std::string &instrName,
                                                              Instruction::Opcode opcode) :
            Instruction::CallInstruction{std::move(args), ast, instrName, opcode, f->getResultType()},
            f_{f}
    {

    }

    bool Instruction::DirectCallInstruction::operator==(const IL *il) const {
        if( auto * other = dynamic_cast<const Instruction::DirectCallInstruction*>(il)){
            bool tmp = f_->operator==(other->f_);
            for (int i = 0; i < args_.size(); ++i) {
                tmp = tmp && args_[i].second == other->args_[i].second
                      && args_[i].first->operator==(other->args_[i].first);
            }
            return tmp;
        }
        else return false;
    }

    const char *Instruction::BinaryOperator::resolve_operator() const{
        switch (operator_) {
            case BinOpType::ADD:
                return "Add ";
            case BinOpType::SUB:
                return "Sub ";
            case BinOpType::MUL:
                return "Mul ";
            case BinOpType::DIV:
                return "Div ";
            case BinOpType::MOD:
                return "Mod ";
            case BinOpType::LSH:
                return "Lsh ";
            case BinOpType::RSH:
                return "Rsh ";
            case BinOpType::AND:
                return "And ";
            case BinOpType::OR:
                return "Or ";
            case BinOpType::XOR:
                return "Xor ";
            case BinOpType::EQ:
                return "Eq ";
            case BinOpType::NEQ:
                return "Neq ";
            case BinOpType::GT:
                return "Gt ";
            case BinOpType::LT:
                return "Lt ";
            case BinOpType::GTE:
                return "Gte ";
            case BinOpType::LTE:
                return "Lte ";
            default:
                throw "unknown opcode";
        }
    }

    void Instruction::BinaryOperator::replaceWith(Instruction *sub, Instruction *toReplace) {
        if(lhs_ == sub){
            lhs_ = toReplace;
            toReplace->registerUsage(this);
        }
        if(rhs_ == sub){
            rhs_ = toReplace;
            toReplace->registerUsage(this);
        }
    }

    const char *Instruction::UnaryOperator::resolve_operator() const {
        switch (operator_) {
            case UnOpType::UNSUB:
                return "Sub ";
            case UnOpType::INC:
                return "Inc ";
            case UnOpType::DEC:
                return "Dec ";
            case UnOpType::NOT:
                return "Not ";
            default:
                throw "unknown opcode";
        }
    }

    void Instruction::UnaryOperator::replaceWith(Instruction *sub, Instruction *toReplace) {
        if(operand_ == sub){
            operand_ = toReplace;
            toReplace->registerUsage(this);
        }
    }


    void Instruction::ElemIndexInstruction::print(tiny::ASTPrettyPrinter &p) const {
            Instruction::print(p);
            p << p.keyword << instrName_ << " " ;
            printRegister(p, base_);
            p << p.keyword<<  "+ ";
            printRegister(p, index_);
//            p << p.keyword<<  "x " << p.numberLiteral << offset_.size();
            p << p.keyword<<  "x " << p.numberLiteral;
            printRegister(p, offset_);
    }

    void Instruction::ElemOffsetInstruction::print(tiny::ASTPrettyPrinter &p) const {
        Instruction::print(p);
        p << p.keyword << instrName_ << " " ;
        printRegister(p, base_);
//        p << p.keyword<<  "+ " << p.numberLiteral << offset_.size();
        p << p.keyword<<  "+ "; printRegister(p, offset_);
    }

    void Instruction::StructAssignInstruction::print(tiny::ASTPrettyPrinter &p) const {
        Instruction::print(p);
        p << p.keyword << instrName_ << " " ;
        printRegister(p, dstAddr_);
        p << p.keyword << "= " ;
        printRegister(p, srcVal_);
        p << p.keyword << "( of size: " << p.numberLiteral << type_->size() << p.keyword << ")" ;
    }

    void Instruction::ElemIndexInstruction::printAlloc(tiny::ASTPrettyPrinter &p) const {
            Instruction::printAlloc(p);
            p << p.keyword << instrName_ << " " ;
            printAllocRegister(p, base_);
            p << p.keyword<<  "+ ";
            printAllocRegister(p, index_);
//            p << p.keyword<<  "x " << p.numberLiteral << offset_.size();
            p << p.keyword<<  "x " << p.numberLiteral;
            printAllocRegister(p, offset_);
    }

    void Instruction::ElemOffsetInstruction::printAlloc(tiny::ASTPrettyPrinter &p) const {
        Instruction::printAlloc(p);
        p << p.keyword << instrName_ << " " ;
        printAllocRegister(p, base_);
//        p << p.keyword<<  "+ " << p.numberLiteral << offset_.size();
        p << p.keyword<<  "+ "; printAllocRegister(p, offset_);
    }

    void Instruction::StructAssignInstruction::printAlloc(tiny::ASTPrettyPrinter &p) const {
        Instruction::printAlloc(p);
        p << p.keyword << instrName_ << " " ;
        printAllocRegister(p, dstAddr_);
        p << p.keyword << "= " ;
        printAllocRegister(p, srcVal_);
        p << p.keyword << "( of size: " << p.numberLiteral << type_->size() << p.keyword << ")" ;
    }

    std::vector<BasicBlock *> ILVisitor::getFunctionBBs(Function *f)  {
        std::vector<BasicBlock*> tmp;
        for(const auto & bb : f->bbs_){
            tmp.emplace_back(bb.get());
        }
        return tmp;
    }

    std::vector<Instruction *> ILVisitor::getBBsInstructions(BasicBlock *bb)  {
        std::vector<Instruction*> tmp;
        for(const auto & ins : bb->insns_){
            tmp.emplace_back(ins.get());
        }
        return tmp;
    }

    std::vector<std::pair<Symbol, Function *>> ILVisitor::getProgramsFunctions(Program *p) {
        std::vector<std::pair<Symbol, Function*>> tmp;
        for(const auto & f : p->functions_){
            tmp.emplace_back(f.first, f.second.get());
        }
        return tmp;
    }

    BasicBlock *ILVisitor::getProgramsGlobals(Program *p) {
        return p->globals_.get();
    }

    BasicBlock *ILVisitor::getProgramsGlobals(ILBuilder &p) {
        return p.globals_.get();
    }
    Instruction*  ILVisitor::getVariableAddress(ILBuilder &p, const Symbol & name) {
        return p.getVariableAddress(name);
    }

    size_t Type::Char::size() const {
        return std::ceil(1/(double) Backend::MemoryCellSize);
    }

    size_t Type::Double::size() const {
        return std::ceil(4/ (double) Backend::MemoryCellSize);//Just Float
    }

    size_t Type::Integer::size() const {
        return std::ceil(4 / (double) Backend::MemoryCellSize);
    }

    size_t Type::Pointer::size() const {
        return std::ceil(4 / (double) Backend::MemoryCellSize);
    }

    size_t Type::Array::size() const {
        //throw "unknown size"; TODO
        size_t result = resolveStatic(size_);
//        auto sz = dynamic_cast<LoadImm *>(size_);
//        assert(sz && sz->resultType() == ResultType::Integer);
//        if(result){
            return base_->size() * result;
//        }
    }

    size_t Type::Array::resolveStatic(Instruction *instr) const {
        if(auto binop = dynamic_cast<BinOp*>(instr)){
            size_t lhs;
            size_t rhs;
            if(binop->lhs()->resultType() == ResultType::Integer){
                lhs = resolveStatic(binop->lhs());
            }else if (binop->lhs()->resultType() == ResultType::Double){
                lhs = (int)resolveStaticDouble(binop->lhs());
            }
            if(binop->rhs()->resultType() == ResultType::Integer){
                rhs = resolveStatic(binop->rhs());
            }else if (binop->rhs()->resultType() == ResultType::Double){
                rhs = (int)resolveStaticDouble(binop->rhs());
            }
            size_t res = 0;
            switch (binop->opType()) {
                case BinOpType::ADD:
                    res = lhs + rhs;
                    break;
                case BinOpType::SUB:
                    res = lhs - rhs;
                    break;
                case BinOpType::MOD:
                    res = lhs % rhs;
                    break;
                case BinOpType::MUL:
                    res = lhs * rhs;
                    break;
                case BinOpType::DIV:
                    res = lhs / rhs;
                    break;
                case BinOpType::AND:
                    res = lhs && rhs;
                    break;
                case BinOpType::OR:
                    res = lhs || rhs;
                    break;
                case BinOpType::XOR:
                    res = lhs ^ rhs;
                    break;
                case BinOpType::LSH:
                    res = lhs << rhs;
                    break;
                case BinOpType::RSH:
                    res = lhs >> rhs;
                    break;
                case BinOpType::NEQ:
                    res = lhs != rhs;
                    break;
                case BinOpType::EQ:
                    res = lhs == rhs;
                    break;
                case BinOpType::LTE:
                    res = lhs <= rhs;
                    break;
                case BinOpType::LT:
                    res = lhs < rhs;
                    break;
                case BinOpType::GT:
                    res = lhs > rhs;
                    break;
                case BinOpType::GTE:
                    res = lhs >= rhs;
                    break;
            }
            return res;
        }else if(auto unop = dynamic_cast<UnOp*>(instr)){
            size_t operand;
            if(unop->operand()->resultType() == ResultType::Integer){
                operand = resolveStatic(unop->operand());
            }else if (unop->operand()->resultType() == ResultType::Double){
                operand = (int)resolveStaticDouble(unop->operand());
            }
            switch (unop->opType()) {

                case UnOpType::UNSUB:
                    return -operand;
                    break;
                case UnOpType::NOT:
                    return !operand;
                    break;
                case UnOpType::INC:
                    return operand +1;
                    break;
                case UnOpType::DEC:
                    return operand -1;
                    break;
            }

        }else if(auto trunc = dynamic_cast<Truncate*>(instr)){
            return (int)resolveStaticDouble(trunc->src());
        }else if(auto extend = dynamic_cast<Extend*>(instr)){
            return (int)resolveStaticDouble(extend);
        }else if(auto loadImm = dynamic_cast<LoadImm*>(instr)){
            if( loadImm->resultType() == ResultType::Integer){
                return loadImm->valueInt();
            }else{
                return (int)resolveStaticDouble(instr);
            }
        }else{
            throw new tiny::ParserError("cannot resolve statically", instr->ast()->location());
        }


        return 0;
    }

    double Type::Array::resolveStaticDouble(Instruction *instr) const {
        if(auto binop = dynamic_cast<BinOp*>(instr)){
            double lhs;
            double rhs;
            if(binop->lhs()->resultType() == ResultType::Integer){
                lhs = (double)resolveStatic(binop->lhs());
            }else if (binop->lhs()->resultType() == ResultType::Double){
                lhs = resolveStaticDouble(binop->lhs());
            }
            if(binop->rhs()->resultType() == ResultType::Integer){
                rhs = (double)resolveStatic(binop->rhs());
            }else if (binop->rhs()->resultType() == ResultType::Double){
                rhs = resolveStaticDouble(binop->rhs());
            }
            size_t res = 0;
            switch (binop->opType()) {
                case BinOpType::ADD:
                    res = lhs + rhs;
                    break;
                case BinOpType::SUB:
                    res = lhs - rhs;
                    break;
                case BinOpType::MOD:
                    throw tiny::ParserError("cannot use mod on double operand", instr->ast()->location());
                    break;
                case BinOpType::MUL:
                    res = lhs * rhs;
                    break;
                case BinOpType::DIV:
                    res = lhs / rhs;
                    break;
                case BinOpType::AND:
                    res = lhs && rhs;
                    break;
                case BinOpType::OR:
                    res = lhs || rhs;
                    break;
                case BinOpType::XOR:
//                    res = lhs ^ rhs;
                    throw tiny::ParserError("cannot use xor on double operand", instr->ast()->location());

                    break;
                case BinOpType::LSH:
//                    res = lhs << rhs;
                    throw tiny::ParserError("cannot use << on double operand", instr->ast()->location());
                    break;
                case BinOpType::RSH:
//                    res = lhs >> rhs;
                    throw tiny::ParserError("cannot use >> on double operand", instr->ast()->location());

                    break;
                case BinOpType::NEQ:
                    res = lhs != rhs;
                    break;
                case BinOpType::EQ:
                    res = lhs == rhs;
                    break;
                case BinOpType::LTE:
                    res = lhs <= rhs;
                    break;
                case BinOpType::LT:
                    res = lhs < rhs;
                    break;
                case BinOpType::GT:
                    res = lhs > rhs;
                    break;
                case BinOpType::GTE:
                    res = lhs >= rhs;
                    break;
            }
            return res;
        }else if(auto unop = dynamic_cast<UnOp*>(instr)){
            size_t operand;
            if(unop->operand()->resultType() == ResultType::Integer){
                operand = resolveStatic(unop->operand());
            }else if (unop->operand()->resultType() == ResultType::Double){
                operand = (int)resolveStaticDouble(unop->operand());
            }
            switch (unop->opType()) {

                case UnOpType::UNSUB:
                    return -operand;
                    break;
                case UnOpType::NOT:
                    return !operand;
                    break;
                case UnOpType::INC:
                    return operand +1;
                    break;
                case UnOpType::DEC:
                    return operand -1;
                    break;
            }

        }else if(auto trunc = dynamic_cast<Truncate*>(instr)){
            return (int)resolveStaticDouble(trunc->src());
        }else if(auto extend = dynamic_cast<Extend*>(instr)){
            return (int)resolveStaticDouble(extend);
        }else if(auto loadImm = dynamic_cast<LoadImm*>(instr)){
            if( loadImm->resultType() == ResultType::Integer){
                return loadImm->valueInt();
            }else{
                return (int)resolveStaticDouble(instr);
            }
        }else{
            throw new tiny::ParserError("cannot resolve statically", instr->ast()->location());
        }

        return 0;
    }

    bool Instruction::Terminator0::operator==(const IL *il) const {
        if( dynamic_cast<const Halt*>(il) && dynamic_cast<const Halt *>(this)){
            return true;
        }
        else return false;
    }
}

