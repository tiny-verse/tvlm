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

    void Instruction::TerminatorN::print(tiny::ASTPrettyPrinter &p) const {
        Instruction::print(p);
        p << p.keyword << instrName_ << " " << p.identifier;
        printRegister(p, cond_);
        for(auto & t : targets_){
            p << p.identifier << t->name() << " ";
        }
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



    Instruction::DirectCallInstruction::DirectCallInstruction(Function *f, std::vector<std::pair< Instruction *, Type*>> &&args,
                                                              const ASTBase *ast, const std::string &instrName,
                                                              Instruction::Opcode opcode) :
            Instruction::CallInstruction{std::move(args), ast, instrName, opcode, f->getResultType()},
            f_{f}
    {

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
        }
        if(rhs_ == sub){
            rhs_ = toReplace;
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

    int Type::Char::size() const {
        return std::ceil(1/(double) Backend::MemoryCellSize);
    }

    int Type::Double::size() const {
        return std::ceil(8/ (double) Backend::MemoryCellSize);
    }

    int Type::Integer::size() const {
        return std::ceil(4 / (double) Backend::MemoryCellSize);
    }

    int Type::Pointer::size() const {
        return std::ceil(4 / (double) Backend::MemoryCellSize);
    }

    int Type::Array::size() const {
        //throw "unknown size"; TODO
        auto sz = dynamic_cast<LoadImm *>(size_);
        assert(sz && sz->resultType() == ResultType::Integer);
        if(sz){
            return base_->size() * sz->valueInt();
        }
    }
}

