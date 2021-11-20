#pragma once

#include <vector>

#include <memory>

#include "common/helpers.h"
#include "common/ast.h"
#include "tinyc/types.h"

namespace tvlm {

    using ASTBase = tiny::ASTBase;
    using Symbol = tiny::Symbol;
    using Type = tinyc::Type;

    class BasicBlock;
    class Function;



    /** Result type of an instruction. 
     */ 
    enum class ResultType {
        Integer,
        Double,
        Void,
    }; // tinyc::il::ResultType 
//
//    class Type{
//    public:
//        class Struct;
//        class Integer;
//        class Float;
//        class Pointer;
//        virtual ~Type() = default;
//
//        virtual int size() const  = 0;
//        std::string toString() const {
//            std::stringstream ss;
//            toStream(ss);
//            return ss.str();
//        }
//    private:
//        virtual void toStream(std::ostream & s) const = 0;
//    };
//
//    class Type::Integer : public Type{
//    public:
//        explicit Integer(){}
//        int size() const {
//            return 4;
//        }
//    private:
//        void toStream(std::ostream & s) const override {
//            s << "int";
//        }
//    };
//    class Type::Float : public Type{
//    public:
//        Float(){}
//        int size() const {
//            return 8;
//        }
//    private:
//
//        void toStream(std::ostream & s) const override {
//            s << "float";
//        }
//    };
//    class Type::Pointer : public Type{
//    public:
//        Pointer(Type * base) : base_{base}{}
//        int size()const{
//            return 4;
//        }
//    private:
//        void toStream(std::ostream & s) const override {
//            base_->toStream(s);
//            s << "*";
//        }
//        Type * base_;
//    };
//    class Type::Struct : public Type{
//    public:
//        Struct() {}
//        int size()const{
//            int size = 0;
//            for(auto & i : fields_){
//                size += i.second->size();
//            }
//            return size ? size : 4; //every struct has to have a memory footprint
//        }
//        void addField(Symbol name, Type * type) {
//            //hope duplicity is already checked
//            //            for (auto & i : fields_)
//            //                if (i.first == name)
//            //                    throw ParserError{STR("Field " << name.name() << " already defined "), ast->location()};
//            fields_.push_back(std::make_pair(name, type));
//        }
//
//        Type * getFieldType(Symbol name) const {
//            for (auto & i : fields_)
//                if (i.first == name)
//                    return i.second;
//            return nullptr;
//        }
//    private:
//
//        void toStream(std::ostream & s) const override {
//            s << "struct " << ast_->name.name();
//        }
//
//        std::vector<std::pair<Symbol, Type *>> fields_;
//
//    };

    /** Base class for intermediate language instructions. 
     */ 
    class Instruction {
    public:
        class ImmSize;
        class ImmIndex;
        class ImmValue;
        class BinaryOperator;
        class UnaryOperator;
        class LoadAddress;
        class StoreAddress;
        class Terminator;
        class Terminator0;
        class Returnator;
        class Terminator1;
        class TerminatorN;
        class SrcInstruction;
        class PhiInstruction;
        class VoidInstruction;
        class CallInstruction;
        class DirectCallInstruction;
        class IndirectCallInstruction;

        virtual ~Instruction() = default;

        /** Returns the result type of the instruction. 
         */
        ResultType resultType() const {
            return resultType_;
        }

        std::string const & name() const {
            return name_;
        }

        void setName(std::string const & value) {
            name_ = value;
        }

        virtual void print(tiny::ASTPrettyPrinter & p) const {
            p << name_ <<": " << instrName_;
        }


    protected:

        /** Creates the instruction with given return type and corresponding abstract syntax tree. 
         */
        Instruction(ResultType resultType, ASTBase const * ast, const std::string & instrName):
            resultType_{resultType},
            ast_{ast} ,
            instrName_{instrName}{
        }    

    private:


        ResultType resultType_;
        ASTBase const * ast_;
        std::string name_;
        std::string instrName_;

    }; // tinyc::il::Instruction

    class Instruction::ImmSize : public Instruction {
    public:

        size_t size() {
            return size_;   
        }

    protected:

        ImmSize(size_t size, ASTBase const * ast, const std::string & instrName):
            Instruction{ResultType::Integer, ast, instrName},
            size_{size} {
        }
        size_t size_;
    };

    class Instruction::ImmIndex : public Instruction {
    public:

        size_t index() {
            return index_;   
        }

    protected:

        ImmIndex(size_t index, ASTBase const * ast, const std::string & instrName):
            Instruction{ResultType::Integer, ast, instrName},
            index_{index} {
        }

        size_t index_;
    };

    class Instruction::ImmValue : public Instruction {
    public:

        int64_t value() {
            return value_;   
        }
        int64_t fvalue() {
            return fvalue_;
        }

    protected:

        ImmValue(int64_t value, ASTBase const * ast, const std::string & instrName):
            Instruction{ResultType::Integer, ast, instrName},
            value_{value}, fvalue_{0}{
        }

        ImmValue(double value, ASTBase const * ast, const std::string & instrName):
                Instruction{ResultType::Double, ast, instrName},
                value_{0}, fvalue_{value} {
        }

        int64_t value_;
        double fvalue_;
    };

    class Instruction::BinaryOperator : public Instruction {
    public:
        Instruction * lhs() const {
            return lhs_;
        }

        Instruction * rhs() const {
            return rhs_;
        }

    protected:
        BinaryOperator(Symbol op, Instruction * lhs, Instruction * rhs, ASTBase const * ast, const std::string & instrName):
            Instruction{GetResultType(lhs, rhs), ast, instrName},
            op_{op},
            lhs_{lhs},
            rhs_{rhs} {
        }        

        static ResultType GetResultType(Instruction * lhs, Instruction * rhs) {
            assert(lhs->resultType() != ResultType::Void && rhs->resultType() != ResultType::Void);
            if (lhs->resultType() == ResultType::Double || rhs->resultType() == ResultType::Double)
                return ResultType::Double;
            else
                return ResultType::Integer;
        }

    private:
        Symbol op_;
        Instruction * lhs_;
        Instruction * rhs_;    
    };

    class Instruction::UnaryOperator : public Instruction {
    public:
        Instruction * operand() const {
            return operand_;
        }
    protected:
        UnaryOperator(Symbol op, Instruction * operand, ASTBase const * ast, const std::string & instrName):
            Instruction{operand->resultType(), ast, instrName},
            op_{op},
            operand_{operand} {
        }

    private:
        Symbol op_;
        Instruction * operand_;

    }; 

    class Instruction::LoadAddress : public Instruction {
    public:

        Instruction * address() const { return address_; }

    protected:

        LoadAddress(Instruction * address,ResultType type, ASTBase const * ast, const std::string & instrName):
            Instruction{type, ast, instrName},
            address_{address} {
        }
    private:

        Instruction * address_;

    }; // Instruction::LoadAddress

    class Instruction::StoreAddress : public Instruction {
    public:

        Instruction * value() const { return value_; }

        Instruction * address() const { return address_; }

    protected:

        StoreAddress(Instruction * value, Instruction * address, ASTBase const * ast, const std::string & instrName):
            Instruction{ResultType::Integer, ast, instrName},
            value_{value},
            address_{address} {
        }
    private:

        Instruction * value_;
        Instruction * address_;

    }; // Instruction::StoreAddress

    class Instruction::Terminator : public Instruction {
    public:

        virtual size_t numTargets() const = 0;

        virtual BasicBlock * getTarget(size_t i) const = 0;        

    protected:
        Terminator(ASTBase const * ast, const std::string & instrName):
            Instruction{ResultType::Void, ast, instrName} {
        }
    }; // Instruction::Terminator

    class Instruction::Terminator0 : public Instruction::Terminator {
    public:
        size_t numTargets() const override { return 0; }

        BasicBlock * getTarget(size_t i) const override { return nullptr; }

    protected:

        Terminator0(ASTBase const * ast, const std::string & instrName):
            Terminator{ast, instrName} {
        }
    }; // Instruction::Terminator0

    class Instruction::Returnator : public Instruction::Terminator0 {
    public:
        Instruction * returnValue()const{
            return returnValue_;
        }
    protected:

        Returnator(Instruction * retValue, ASTBase const * ast, const std::string & instrName):
            Terminator0{ast, instrName}, returnValue_{retValue} {
        }
        Instruction * returnValue_;
    }; // Instruction::Returnator

    class Instruction::Terminator1 : public Instruction::Terminator {
    public:
        size_t numTargets() const override { return 1; }

        BasicBlock * getTarget(size_t i) const override { return i == 1 ? target_ : nullptr; }

    protected:

        Terminator1(BasicBlock * target, ASTBase const * ast, const std::string & instrName):
            Terminator{ast, instrName},
            target_{target} {
        }

    private:
        BasicBlock * target_;
    }; // Instruction::Terminator1

    class Instruction::TerminatorN : public Instruction::Terminator {
    public:

        Instruction * condition() const { return cond_; }

        size_t numTargets() const override { return targets_.size(); }

        BasicBlock * getTarget(size_t i) const override { return targets_[i]; }

        void addTarget(BasicBlock * target) {
            targets_.push_back(target);
        }

    protected:

        TerminatorN(Instruction * cond, ASTBase const * ast, const std::string & instrName):
            Terminator{ast, instrName},
            cond_{cond} {
        }

    private:
        Instruction * cond_;
        std::vector<BasicBlock *> targets_;
    }; // Instruction::TerminatorN

    class Instruction::SrcInstruction : public Instruction{
    public:
        Instruction * src(){
            return src_;
        }
    protected:
        SrcInstruction(Instruction * src, ResultType type, ASTBase const * ast, const std::string & instrName):
                Instruction{type, ast, instrName},
                src_{src}{

        }
    private:
        Instruction * src_;
    }; // Instruction::SrcInstruction

    class Instruction::VoidInstruction : public Instruction{
    public:
    protected:
        VoidInstruction( ResultType type, ASTBase const * ast, const std::string & instrName):
                Instruction{type, ast, instrName}{

        }
    }; // Instruction::VoidInstruction

    class Instruction::PhiInstruction : public Instruction{
    public:
        void addIncomming( Instruction * src, BasicBlock * bb){
            contents_.emplace(bb, src);
        }

    protected:
        PhiInstruction( ResultType type, ASTBase const * ast, const std::string & instrName):
                Instruction{type, ast, instrName}{

        }
    private:
        std::unordered_map<BasicBlock*, Instruction *> contents_;
    }; // Instruction::PhiInstruction

    class Instruction::CallInstruction : public Instruction{
    public:
        Instruction * operator [] (size_t i) const {
            assert(i < args_.size());
            return args_[i];
        }


    protected:
        CallInstruction(std::vector<Instruction*> && args, ASTBase const * ast, const std::string & instrName ):
                Instruction{ResultType::Void, ast, instrName},
                args_{args}{

        }
        std::vector<Instruction *> args_;
    }; // Instruction::CallInstruction

    class Instruction::DirectCallInstruction : public CallInstruction{
    public:
        Function * f() const {
            return f_;
        }

    protected:
        DirectCallInstruction(Function * f, std::vector<Instruction*> && args,const ASTBase * ast , const std::string & instrName):
                CallInstruction{std::move(args), ast, instrName},
                f_{f}
                {

        }
        Function * f_;
    }; // Instruction::DirectCallInstruction

    class Instruction::IndirectCallInstruction : public CallInstruction{
    public:
        Instruction * f() const {
            return f_;
        }

    protected:
        IndirectCallInstruction(Instruction * f, std::vector<Instruction*> && args,const ASTBase * ast, const std::string & instrName ):
                CallInstruction{std::move(args), ast, instrName},
                f_{f}
                {

        }
        Instruction * f_;
    }; // Instruction::IndirectCallInstruction

#define INS(NAME, ENCODING) class NAME : public Instruction::ENCODING { \
    public: \
        ENCODING(NAME, ENCODING) \
};
#define ImmSize(NAME, ENCODING) NAME (size_t size, ASTBase const * ast) : Instruction::ENCODING{size, ast, #NAME} {}
#define ImmIndex(NAME, ENCODING) NAME (size_t index, ASTBase const * ast) : Instruction::ENCODING{index, ast, #NAME} {}
#define ImmValue(NAME, ENCODING) NAME (int64_t value, ASTBase const * ast) : Instruction::ENCODING{value, ast, #NAME} {} \
                                 NAME (double value, ASTBase const * ast) : Instruction::ENCODING{value, ast, #NAME} {}
#define BinaryOperator(NAME, ENCODING) NAME (Symbol op, Instruction * lhs, Instruction * rhs, ASTBase const * ast): Instruction::ENCODING{op, lhs, rhs, ast, #NAME} {}
#define UnaryOperator(NAME, ENCODING) NAME (Symbol op, Instruction * operand, ASTBase const * ast): Instruction::ENCODING{op, operand, ast, #NAME} {}
#define Terminator0(NAME, ENCODING) NAME (ASTBase const * ast): Instruction::ENCODING{ast, #NAME} {}
#define Returnator(NAME, ENCODING) NAME (Instruction * returnValue, ASTBase const * ast): Instruction::ENCODING{returnValue, ast, #NAME} {}
#define Terminator1(NAME, ENCODING) NAME (BasicBlock * target, ASTBase const * ast): Instruction::ENCODING{target, ast, #NAME} {}
#define TerminatorN(NAME, ENCODING) NAME (Instruction * condition, ASTBase const * ast): Instruction::ENCODING{condition, ast, #NAME} {}
#define LoadAddress(NAME, ENCODING) NAME (Instruction * address, ResultType type, ASTBase const * ast): Instruction::ENCODING{address, type, ast, #NAME} {}
#define StoreAddress(NAME, ENCODING) NAME (Instruction * value, Instruction * address, ASTBase const * ast): Instruction::ENCODING{value, address, ast, #NAME} {}
#define DirectCallInstruction(NAME, ENCODING) NAME (Function * f, std::vector<Instruction*> && args, ASTBase const * ast): Instruction::ENCODING{f, std::move(args), ast, #NAME} {}
#define IndirectCallInstruction(NAME, ENCODING) NAME (Instruction * f, std::vector<Instruction*> && args, ASTBase const * ast): Instruction::ENCODING{f, std::move(args), ast, #NAME} {}
#define PhiInstruction(NAME, ENCODING) NAME ( ResultType type, ASTBase const * ast): Instruction::ENCODING{type, ast, #NAME} {}


#define INSTYPE(NAME, ENCODING, TYPE) class NAME : public Instruction::ENCODING { \
    public: \
        ENCODING(NAME, ENCODING, TYPE) \
};

#define SrcInstruction(NAME, ENCODING, TYPE) NAME (Instruction * src, ASTBase const * ast): Instruction::ENCODING{src, TYPE, ast, #NAME} {}
#define VoidInstruction(NAME, ENCODING, TYPE) NAME (ASTBase const * ast): Instruction::ENCODING{TYPE, ast, #NAME} {}

#include "il_insns.h"


    class BasicBlock {
    public:

        Instruction * add(Instruction * ins) {
            assert(! terminated());
            insns_.push_back(std::unique_ptr<Instruction>{ins});
            return ins;
        }

        bool terminated() const {
            if (insns_.empty())
                return false;
            return dynamic_cast<Instruction::Terminator*>(insns_.back().get()) != nullptr;
        }

        void print(tiny::ASTPrettyPrinter & p) const {
            p << p.identifier << name() << p.symbol << ":";
            p.indent();
            p.newline();
            for (auto & i : insns_) {
                // TODO add printing of instructions in some nice way
                i->print(p);
                p.newline();
            }
            p.dedent();
            p.newline();
        }

        std::string const & name() const {
            return name_;
        }

        BasicBlock * setName(std::string const & name) {
            name_ = name;
            return this;
        }

    private:
        std::string name_;
        std::vector<std::unique_ptr<Instruction>> insns_;
    }; // BasicBlock

    class Function {
    public:

        Function(ASTBase const * ast):
            ast_{ast} {
        }

        Symbol name() const {
            return name_;
        }

        Function * setName(std::string const & name) {
            name_ = name;
            return this;
        }

        bool contains(BasicBlock * b) const {
            for (auto const & i : bbs_)
                if (i.get() == b)
                    return true;
            return false;
        }

        void print(tiny::ASTPrettyPrinter & p) const {

            p << p.comment << "function: " ; //<< ast_->type()->toString();
            p.newline();
            p << p.identifier << name_ << p.symbol << ":";
            p.indent();
            p.newline();
            for (auto & i : bbs_)
                i->print(p);
            p.dedent();

        }

    private:
        friend class ILBuilder;

        ASTBase const * ast_;

        std::string name_;

        std::vector<std::unique_ptr<BasicBlock>> bbs_;

    };   // tvlm::Function

    class Program {
    public:
        Program( std::unordered_map<std::string, Instruction*> && stringLiterals,
                 std::unordered_map<Symbol, std::unique_ptr<Function>> && functions,
                 std::unique_ptr<BasicBlock> && globals
        ): stringLiterals_(std::move(stringLiterals)), functions_(std::move(functions)), globals_(std::move(globals)){}

    private:
        std::unordered_map<std::string, Instruction*> stringLiterals_;
        std::unordered_map<Symbol, std::unique_ptr<Function>> functions_;
        std::unique_ptr<BasicBlock> globals_;
    } ; // tvlm::Program

} // namespace tvlm
