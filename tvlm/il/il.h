#pragma once

#include <vector>

#include <memory>

#include "common/helpers.h"
#include "common/ast.h"
#include "tinyc/types.h"

namespace tvlm{
    class CfgBuilder;
}
namespace tvlm {

    using ASTBase = tiny::ASTBase;
    using Symbol = tiny::Symbol;
//    using Type = tinyc::Type;

    class BasicBlock;
    class Function;
    class ILVisitor;

    class Program;

#include "il_insns_predeclare.h"

    class IL{
    public:
        virtual void accept(ILVisitor * v) = 0;
    protected:
        friend class ILVisitor;
    };
    class Instruction;

    /** Result type of an instruction. 
     */ 
    enum class ResultType {
        Integer,
        Double,
        Void,
    }; // tinyc::il::ResultType

    class Type{
    public:
        class Struct;
        class Integer;
        class Double;
        class Pointer;
        class Array;
        class Char;
        class Void;
        class String;
        virtual ~Type() = default;

        virtual int size() const  = 0;
        std::string toString() const {
            std::stringstream ss;
            toStream(ss);
            return ss.str();
        }
        virtual ResultType registerType()const  =0;


    private:
        virtual void toStream(std::ostream & s) const = 0;
    };

    class Type::Integer : public Type{
    public:
        explicit Integer(){}
        int size() const {
            return 4;
        }

        ResultType registerType() const override {
            return ResultType::Integer;
        }

    private:
        void toStream(std::ostream & s) const override {
            s << "int";
        }
    };
    class Type::Char : public Type{
    public:
        explicit Char(){}
        int size() const {
            return 1;
        }

        ResultType registerType() const override {
            return ResultType::Integer;
        }

    private:
        void toStream(std::ostream & s) const override {
            s << "char";
        }
    };
    class Type::Void : public Type{
    public:
        explicit Void(){}
        int size() const {
            return 0;
        }

        ResultType registerType() const override {
            return ResultType::Integer;
        }

    private:
        void toStream(std::ostream & s) const override {
            s << "void";
        }
    };

    class Type::String : public Type{
    public:
        explicit String(size_t size_){}
        int size() const {
            return (int)size_;
        }

        ResultType registerType() const override {
            return ResultType::Integer; // Address of that string
        }

    private:
        void toStream(std::ostream & s) const override {
            s << "string (of size: " << size_ << ")";
        }
        size_t size_;
    };
    class Type::Double : public Type{
    public:
        Double(){}
        int size() const {
            return 8;
        }

        ResultType registerType() const override {
            return ResultType::Double;
        }

    private:

        void toStream(std::ostream & s) const override {
            s << "double";
        }
    };
    class Type::Pointer : public Type{
    public:
        Pointer(Type * base) : base_{base}{}
        int size()const{
            return 4;
        }

        ResultType registerType() const override {
            return ResultType::Integer;
        }

    private:
        void toStream(std::ostream & s) const override {
            base_->toStream(s);
            s << "*";
        }
        Type * base_;
    };

    class Type::Array : public Type{
    public:
        Array(Type * base, Instruction * size) : base_{base}, size_(size){}
        int size()const{
            throw "unknown size";
            return 4;
        }

        ResultType registerType() const override {
            return ResultType::Integer;
        }

    private:
        void toStream(std::ostream & s) const override {
            base_->toStream(s);
            s << "[" <<"]";
        }
        Type * base_;
        Instruction * size_;
    };
    class Type::Struct : public Type{
    public:
        Struct(const  Symbol &name, const std::vector<std::pair<Symbol, Type *>> & fields):name_(name) , fields_(fields){}
        int size()const{
            int size = 0;
            for(auto & i : fields_){
                size += i.second->size();
            }
            return size ? size : 1; //every struct has to have a memory footprint
        }

        Type * getFieldType(Symbol name) const {
            for (auto & i : fields_)
                if (i.first == name)
                    return i.second;
            return nullptr;
        }

        int getFieldOffset(Symbol name)const{
            int acc = 0;
            for (auto & i : fields_){
                if (i.first == name){
                    return acc;
                }else{
                    acc += i.second->size();
                }
            }
            return -1;
        }

        ResultType registerType() const override {
            return ResultType::Integer; // Address of that Struct
        }

    private:

        void toStream(std::ostream & s) const override {
            s << "struct " << name_.name();
        }

        Symbol name_;
        std::vector<std::pair<Symbol, Type *>> fields_;

    };


    /** Base class for intermediate language instructions. 
     */ 
    class Instruction : public IL{
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
        class ElemInstruction;
        class ElemOffsetInstruction;
        class ElemIndexInstruction;
        class VoidInstruction;
        class CallInstruction;
        class DirectCallInstruction;
        class IndirectCallInstruction;
        class StructAssignInstruction;

        virtual ~Instruction() = default;

        /** Returns the result type of the instruction. 
         */
        ResultType resultType() const {
            return resultType_;
        }

        std::string const & name() const {
            return name_;
        }
        const ASTBase  * ast()const {
            return ast_;
        }

        void setName(std::string const & value) {
            name_ = value;
        }

        virtual void print(tiny::ASTPrettyPrinter & p) const {
            if (resultType_ != ResultType::Void) {
                printRegister(p, this);
                p << p.symbol << "= ";
            }
        }


        //enum opcode TODO
        enum class Opcode {
            ADD,
            SUB,
            UNSUB,
            MOD,
            MUL,
            DIV,
            AND,
            OR,
            XOR,
            NOT,
            LSH,
            RSH,
            INC,
            DEC,

            NEQ,
            EQ,
            LTE,
            LT,
            GT,
            GTE,


            BinOp,
            UnOp,
            LoadImm,
            AllocG,
            AllocL,
            Halt,

            Return,
            GetChar,
            PutChar,
            CondJump,
            Jump,
            Phi,
            Call,
            CallStatic,
            Extend,
            Truncate,
            ElemAddrIndex,
            ElemAddrOffset,
            Copy,
            Store,
            Load,
            ArgAddr,
            StructAssign,

        };

        Opcode opcode_;
    protected:

        friend class ILVisitor;
        friend class LivenessAnalysis;
        /** Creates the instruction with given return type and corresponding abstract syntax tree. 
         */
        Instruction(ResultType resultType, ASTBase const * ast, const std::string & instrName, Opcode opcode):
            resultType_{resultType},
            ast_{ast} ,
            instrName_{instrName},
            opcode_(opcode){
        }



        void printRegister(tiny::ASTPrettyPrinter & p, Instruction const * reg) const {
            p << p.identifier << reg->name() << p.symbol << ": " << p.keyword << (reg->resultType_ == ResultType::Integer ? "int " : "double ");
        }

        void printRegisterAddress(tiny::ASTPrettyPrinter & p, Instruction const * reg) const {
            p << p.symbol << "[" << p.identifier << reg->name() << p.symbol << "] ";
        }



        std::string instrName_;
        ASTBase const * ast_;
    private:


        ResultType resultType_;
        std::string name_;

    }; // tinyc::il::Instruction

    class Instruction::ImmSize : public Instruction {
    public:

        int size() const {
            return type_->size();
        }

        virtual void print(tiny::ASTPrettyPrinter & p) const override {

            Instruction::print(p);
            p << p.keyword << instrName_ << " "<< p.numberLiteral << size() << p.keyword << " (" << type_->toString() <<   ")";
            if(amount_){
                p << p.keyword << " x ";
                printRegister(p, amount_);
            }
        };
        Instruction * amount()const {
            return amount_;
        }
        virtual ~ImmSize(){}
    protected:
        // void accept(ILVisitor * v) override;

        ImmSize(Type * type,Instruction * amount, ASTBase const * ast, const std::string & instrName, Opcode opcode):
            Instruction{ResultType::Integer, ast, instrName, opcode},
            type_{type},
            amount_(amount){
            if(amount && amount->resultType() != ResultType::Integer ){
                throw tiny::ParserError(STR("vector size has to be of type Int"), ast->location());
            }
            assert(!amount || amount->resultType() == ResultType::Integer );
        }
        ImmSize(Type * type, ASTBase const * ast, const std::string & instrName, Opcode opcode ):
                ImmSize(type, nullptr, ast, instrName, opcode){}
        Instruction * amount_;
        Type * type_;
    };

    class Instruction::ImmIndex : public Instruction {
    public:

        size_t index() {
            return index_;   
        }
        virtual void print(tiny::ASTPrettyPrinter & p) const override {
            Instruction::print(p);
            p << p.keyword << instrName_ << " " << p.numberLiteral << index_;
        };

        virtual ~ImmIndex(){}
    protected:

        // void accept(ILVisitor * v) override;
        ImmIndex(size_t index, ASTBase const * ast, const std::string & instrName, Opcode opcode):
            Instruction{ResultType::Integer, ast, instrName, opcode},
            index_{index} {
        }

        size_t index_;
    };

    class Instruction::ImmValue : public Instruction {
    public:

        int64_t valueInt() {
            return value_.i;
        }
        double valueFloat() {
            return value_.f;
        }
        virtual void print(tiny::ASTPrettyPrinter & p) const override {
            Instruction::print(p);
            p << p.keyword << instrName_ << " " << p.numberLiteral << (resultType() == ResultType::Integer ? value_.i : value_.f);
        };


        virtual ~ImmValue(){}
    protected:

        // void accept(ILVisitor * v) override;
        ImmValue(int64_t value, ASTBase const * ast, const std::string & instrName, Opcode opcode):
        Instruction{ResultType::Integer, ast, instrName, opcode}
        {
            value_.i = value;
        }

        ImmValue(double value, ASTBase const * ast, const std::string & instrName, Opcode opcode):
        Instruction{ResultType::Double, ast, instrName, opcode}
        {
             value_.f = value;
        }

        union {
            int64_t i;
            double f;
        } value_;
    };

    enum class BinOpType{

        ADD,
        SUB,

        MOD,
        MUL,

        DIV,
        AND,
        OR,
        XOR,
        LSH,
        RSH,

        NEQ,
        EQ,
        LTE,
        LT,
        GT,
        GTE,
    };
    class Instruction::BinaryOperator : public Instruction {
    public:

        Opcode op() const {
            return op_;
        }

        BinOpType opType() const {
            return operator_;
        }


        Instruction * lhs() const {
            return lhs_;
        }

        Instruction * rhs() const {
            return rhs_;
        }


        virtual void print(tiny::ASTPrettyPrinter & p) const override {
            Instruction::print(p);
            p << p.keyword << resolve_operator() << " ";
            printRegister(p, lhs_);
            printRegister(p, rhs_);
        };

        virtual ~BinaryOperator(){}
    protected:
        const char *  resolve_operator() const;
        // void accept(ILVisitor * v) override;

        BinaryOperator(Opcode op, BinOpType oper, Instruction * lhs, Instruction * rhs, ASTBase const * ast, const std::string & instrName):
            Instruction{GetResultType(lhs, rhs), ast, instrName, op},
            op_{op},
            operator_{oper},
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
        Opcode op_;
        BinOpType operator_;
        Instruction * lhs_;
        Instruction * rhs_;    
    };

    enum class UnOpType{
        UNSUB,
        NOT,
        INC,
        DEC,
    };
    class Instruction::UnaryOperator : public Instruction {
    public:
        Instruction * operand() const {
            return operand_;
        }
        UnOpType opType() const{
            return operator_;
        }
        virtual void print(tiny::ASTPrettyPrinter & p) const override {
            Instruction::print(p);
            p << p.keyword << resolve_operator() << " ";
            printRegister(p, operand_);
        };

        virtual ~UnaryOperator(){}
    protected:
        const char *  resolve_operator() const;
        // void accept(ILVisitor * v) override;

        UnaryOperator( Opcode op, UnOpType oper, Instruction * operand, ASTBase const * ast, const std::string & instrName):
            Instruction{operand->resultType(), ast, instrName, op},
            operator_{oper},
            operand_{operand} {
        }

    private:
        UnOpType operator_;
        Instruction * operand_;

    }; 

    class Instruction::LoadAddress : public Instruction {
    public:

        Instruction * address() const { return address_; }

        virtual void print(tiny::ASTPrettyPrinter & p) const override {
            Instruction::print(p);
            p << p.keyword << instrName_ << " ";
            printRegisterAddress(p, address_);
        };

        virtual ~LoadAddress(){}
    protected:
        // void accept(ILVisitor * v) override;

        LoadAddress(Instruction * address,ResultType type, ASTBase const * ast, const std::string & instrName, Opcode opcode):
            Instruction{type, ast, instrName, opcode},
            address_{address} {
        }
    private:

        Instruction * address_;

    }; // Instruction::LoadAddress

    class Instruction::StoreAddress : public Instruction {
    public:

        Instruction * value() const { return value_; }

        Instruction * address() const { return address_; }

        virtual void print(tiny::ASTPrettyPrinter & p) const override {
            Instruction::print(p);
            p << p.keyword << instrName_ << " ";
            printRegisterAddress(p, address_);
            printRegister(p, value_);
        };

        virtual ~StoreAddress(){}
    protected:
        // void accept(ILVisitor * v) override;

        StoreAddress(Instruction * value, Instruction * address, ASTBase const * ast, const std::string & instrName, Opcode opcode):
            Instruction{ResultType::Integer, ast, instrName, opcode},
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

        virtual ~Terminator(){}
    protected:
        Terminator(ASTBase const * ast, const std::string & instrName, Opcode opcode):
            Instruction{ResultType::Void, ast, instrName, opcode} {
        }
    }; // Instruction::Terminator

    class Instruction::Terminator0 : public Instruction::Terminator {
    public:
        size_t numTargets() const override { return 0; }

        BasicBlock * getTarget(size_t i) const override { return nullptr; }

        virtual void print(tiny::ASTPrettyPrinter & p) const override {
            Instruction::print(p);
            p << p.keyword << instrName_ << " ";
        }
    protected:
        // void accept(ILVisitor * v) override;

        Terminator0(ASTBase const * ast, const std::string & instrName, Opcode opcode):
            Terminator{ast, instrName, opcode} {
        }
    }; // Instruction::Terminator0

    class Instruction::Returnator : public Instruction::Terminator0 {
    public:
        Instruction * returnValue()const{
            return returnValue_;
        }

        virtual void print(tiny::ASTPrettyPrinter & p) const override {
            Instruction::print(p);
            p << p.keyword << instrName_ << " ";
            printRegister(p, returnValue_);
        }

        virtual ~Returnator(){}
    protected:
        // void accept(ILVisitor * v) override;

        Returnator(Instruction * retValue, ASTBase const * ast, const std::string & instrName, Opcode opcode):
            Terminator0{ast, instrName, opcode}, returnValue_{retValue} {
        }
        Instruction * returnValue_;
    }; // Instruction::Returnator

    class Instruction::Terminator1 : public Instruction::Terminator {
    public:
        size_t numTargets() const override { return 1; }

        BasicBlock * getTarget(size_t i) const override { return i == 1 ? target_ : nullptr; }

        virtual void print(tiny::ASTPrettyPrinter & p) const override ;

    protected:
        // void accept(ILVisitor * v) override;

        Terminator1(BasicBlock * target, ASTBase const * ast, const std::string & instrName, Opcode opcode):
            Terminator{ast, instrName, opcode},
            target_{target} {
        }

        virtual ~Terminator1(){}
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
        virtual void print(tiny::ASTPrettyPrinter & p) const override;

    protected:
        // void accept(ILVisitor * v) override;

        TerminatorN(Instruction * cond, ASTBase const * ast, const std::string & instrName, Opcode opcode):
            Terminator{ast, instrName, opcode},
            cond_{cond} {
        }

        virtual ~TerminatorN(){}
    private:
        Instruction * cond_;
        std::vector<BasicBlock *> targets_;
    }; // Instruction::TerminatorN

    class Instruction::SrcInstruction : public Instruction{
    public:
        Instruction * src(){
            return src_;
        }

        virtual void print(tiny::ASTPrettyPrinter & p) const override {
            Instruction::print(p);
            p << p.keyword << instrName_ << " ";
            printRegister(p, src_);
        }

        virtual ~SrcInstruction(){}
    protected:
        // void accept(ILVisitor * v) override;

        SrcInstruction(Instruction * src, ResultType type, ASTBase const * ast, const std::string & instrName, Opcode opcode):
                Instruction{type, ast, instrName, opcode},
                src_{src}{

        }
    private:
        Instruction * src_;
    }; // Instruction::SrcInstruction

    class Instruction::VoidInstruction : public Instruction{
    public:
        virtual void print(tinyc::ASTPrettyPrinter & p) const override{
            Instruction::print(p);
            p << p.keyword << instrName_ << " ";
        }

        virtual ~VoidInstruction(){}

    protected:
//        virtual void accept(ILVisitor * v) override;

        VoidInstruction( ResultType type, ASTBase const * ast, const std::string & instrName, Opcode opcode):
                Instruction{type, ast, instrName, opcode}{

        }
    }; // Instruction::VoidInstruction

    class Instruction::PhiInstruction : public Instruction{
    public:
        void addIncomming( Instruction * src, BasicBlock * bb){
            contents_.emplace(bb, src);
        }

        virtual void print(tiny::ASTPrettyPrinter & p) const override;
        std::unordered_map<BasicBlock*, Instruction *> contents()const{
            return contents_;
        }

        virtual ~PhiInstruction(){}

    protected:
//        virtual void accept(ILVisitor * v) override;

        PhiInstruction( ResultType type, ASTBase const * ast, const std::string & instrName, Opcode opcode):
                Instruction{type, ast, instrName, opcode}{

        }
    private:
        std::unordered_map<BasicBlock*, Instruction *> contents_;
    }; // Instruction::PhiInstruction

  class Instruction::StructAssignInstruction : public Instruction{
    public:
        void print(tiny::ASTPrettyPrinter & p) const override;

      virtual ~StructAssignInstruction(){}
    protected:
//      virtual void accept(ILVisitor * v) override;

        StructAssignInstruction( Instruction * dstAddr, Instruction * srcVal, Type::Struct * type, ASTBase const * ast, const std::string & instrName, Opcode opcode):
                Instruction{ResultType::Void, ast, instrName, opcode},
                srcVal_(srcVal),
                dstAddr_(dstAddr),
                type_(type)
                {

        }
    private:
        Instruction * srcVal_;
        Instruction * dstAddr_;
        Type::Struct * type_;
    }; // Instruction::StructAssignInstruction

    class Instruction::ElemInstruction : public Instruction{
    public:

        virtual ~ElemInstruction(){}
        Instruction * base()const {
            return base_;
        }
    protected:

        ElemInstruction( Instruction * base, ASTBase const * ast, const std::string & instrName, Opcode opcode):
                Instruction{ResultType::Integer, ast, instrName, opcode},
                base_(base){

        }

    protected:
        Instruction * base_;
    }; // Instruction::ElemInstruction

    class Instruction::ElemOffsetInstruction : public Instruction::ElemInstruction{
    public:

        virtual void print(tiny::ASTPrettyPrinter & p) const override;

        virtual ~ElemOffsetInstruction(){}

        Instruction * offset()const {
            return offset_;
        }
    protected:
//        virtual void accept(ILVisitor * v) override;

        ElemOffsetInstruction( Instruction * base, Instruction * offset,  ASTBase const * ast, const std::string & instrName, Opcode opcode):
                ElemInstruction{base, ast, instrName, opcode },
                offset_(offset){

        }
    private:
        Instruction * offset_;
    }; // Instruction::ElemOffsetInstruction

class Instruction::ElemIndexInstruction : public Instruction::ElemInstruction{
    public:

        virtual void print(tiny::ASTPrettyPrinter & p) const override;

    virtual ~ElemIndexInstruction(){}
        Instruction * offset()const {
            return offset_;
        }
        Instruction * index()const {
            return index_;
        }
    protected:
//    virtual void accept(ILVisitor * v) override;

        ElemIndexInstruction( Instruction * base,Instruction * offset, Instruction * index, ASTBase const * ast, const std::string & instrName, Opcode opcode):
                ElemInstruction{base, ast, instrName, opcode },
                index_(index),
                offset_(offset){

        }
    private:
        Instruction * index_;
        Instruction * offset_;
    }; // Instruction::ElemIndexInstruction

    class Instruction::CallInstruction : public Instruction{
    public:
        Instruction * operator [] (size_t i) const {
            assert(i < args_.size());
            return args_[i];
        }

        const std::vector<Instruction *> & args()const {
            return args_;
        }

        virtual ~CallInstruction(){}
    protected:

        CallInstruction(std::vector<Instruction*> && args, ASTBase const * ast, const std::string & instrName
                        , Opcode opcode , const ResultType & resultType):
                Instruction{resultType, ast, instrName, opcode},
                args_{args}{

        }
        std::vector<Instruction *> args_;
    }; // Instruction::CallInstruction

    class Instruction::DirectCallInstruction : public CallInstruction{
    public:
        Function * f() const {
            return f_;
        }

        virtual ~DirectCallInstruction(){}
        virtual void print(tiny::ASTPrettyPrinter & p) const override;
    protected:
//        virtual void accept(ILVisitor * v) override;

        DirectCallInstruction(Function * f, std::vector<Instruction*> && args,const ASTBase * ast , const std::string & instrName, Opcode opcode);
        Function * f_;
    }; // Instruction::DirectCallInstruction

    class Instruction::IndirectCallInstruction : public CallInstruction{
    public:
        Instruction * f() const {
            return f_;
        }

        virtual void print(tiny::ASTPrettyPrinter & p) const override {
            Instruction::print(p);
            p << p.keyword << instrName_ << " ";
            printRegisterAddress(p, f_);
            p << p.symbol << "(";
            for (auto & i : args_)
                printRegister(p, i);
            p << p.symbol << ")";
        };

        virtual ~IndirectCallInstruction(){}
    protected:
//        virtual void accept(ILVisitor * v) override;

        IndirectCallInstruction(Instruction * f, std::vector<Instruction*> && args,const ASTBase * ast, const std::string & instrName, Opcode opcode ):
                CallInstruction{std::move(args), ast, instrName, opcode, f->resultType()},
                f_{f}
                {

        }
        Instruction * f_;
    }; // Instruction::IndirectCallInstruction




    class ILVisitor {
    public:

        virtual void visit(Instruction * ins) = 0;
        virtual void visit(Jump * ins) = 0;
        virtual void visit(CondJump * ins) = 0;
        virtual void visit(Return * ins) = 0;
        virtual void visit(CallStatic * ins) = 0;
        virtual void visit(Call * ins) = 0;
        virtual void visit(Copy * ins) = 0;
        virtual void visit(Extend * ins) = 0;
        virtual void visit(Truncate * ins) = 0;
//        virtual void visit(Instruction::SrcInstruction * ins) = 0;
        virtual void visit(BinOp * ins) = 0;
        virtual void visit(UnOp * ins) = 0;
        virtual void visit(LoadImm * ins) = 0;
        virtual void visit(AllocL * ins) = 0;
        virtual void visit(AllocG * ins) = 0;
        virtual void visit(ArgAddr * ins) = 0;
        virtual void visit(PutChar * ins) = 0;
        virtual void visit(GetChar * ins) = 0;
        virtual void visit(Load * ins) = 0;
        virtual void visit(Store * ins) = 0;
        virtual void visit(Phi * ins) = 0;
        virtual void visit(ElemAddrOffset * ins) = 0;
        virtual void visit(ElemAddrIndex * ins) = 0;
        virtual void visit(Halt * ins) = 0;
        virtual void visit(StructAssign * ins) = 0;


        virtual void visit(BasicBlock * bb) = 0;
        virtual void visit(Function * fce) = 0;
        virtual void visit(Program * p) = 0;

    protected:

        void visitChild(IL * child) {
            child->accept(this);
        }

        static std::vector<BasicBlock*> getFunctionBBs(Function * f);
        static std::vector<Instruction*> getBBsInstructions(BasicBlock * bb);
        static std::vector<std::pair<Symbol ,Function*>> getProgramsFunctions(Program * p) ;
        static BasicBlock*  getProgramsGlobals(Program * p) ;

    }; // tinyc::ASTVisitor






#define INS(NAME, ENCODING) class NAME : public Instruction::ENCODING { \
    public: \
        ENCODING(NAME, ENCODING)                                        \
        virtual ~NAME(){}                                           \
        inline virtual void accept(ILVisitor * v)     \
        { v->visit(this); } \
};

#define ImmSize(NAME, ENCODING) NAME (Type * type,Instruction * amount, ASTBase const * ast) : Instruction::ENCODING{type, amount, ast, #NAME, Instruction::Opcode::NAME} {}
//#define ImmSize(NAME, ENCODING) NAME (Type * type, ASTBase const * ast) : Instruction::ENCODING{type, ast, #NAME, Instruction::Opcode::NAME} {}
#define ImmIndex(NAME, ENCODING) NAME (size_t index, ASTBase const * ast) : Instruction::ENCODING{index, ast, #NAME, Instruction::Opcode::NAME} {}
#define ImmValue(NAME, ENCODING) NAME (int64_t value, ASTBase const * ast) : Instruction::ENCODING{value, ast, #NAME, Instruction::Opcode::NAME} {} \
                                 NAME (double value, ASTBase const * ast) : Instruction::ENCODING{value, ast, #NAME, Instruction::Opcode::NAME} {}
#define BinaryOperator(NAME, ENCODING) NAME (BinOpType oper, Opcode op, Instruction * lhs, Instruction * rhs, ASTBase const * ast): Instruction::ENCODING{op, oper, lhs, rhs, ast, #NAME} {}
#define UnaryOperator(NAME, ENCODING) NAME (UnOpType oper, Opcode op, Instruction * operand, ASTBase const * ast): Instruction::ENCODING{op, oper, operand, ast, #NAME} {}
#define Terminator0(NAME, ENCODING) NAME (ASTBase const * ast): Instruction::ENCODING{ast, #NAME, Instruction::Opcode::NAME} {}
#define Returnator(NAME, ENCODING) NAME (Instruction * returnValue, ASTBase const * ast): Instruction::ENCODING{returnValue, ast, #NAME, Instruction::Opcode::NAME} {}
#define Terminator1(NAME, ENCODING) NAME (BasicBlock * target, ASTBase const * ast): Instruction::ENCODING{target, ast, #NAME, Instruction::Opcode::NAME} {}
#define TerminatorN(NAME, ENCODING) NAME (Instruction * condition, ASTBase const * ast): Instruction::ENCODING{condition, ast, #NAME, Instruction::Opcode::NAME} {}
#define LoadAddress(NAME, ENCODING) NAME (Instruction * address, ResultType type, ASTBase const * ast): Instruction::ENCODING{address, type, ast, #NAME, Instruction::Opcode::NAME} {}
#define StoreAddress(NAME, ENCODING) NAME (Instruction * value, Instruction * address, ASTBase const * ast): Instruction::ENCODING{value, address, ast, #NAME, Instruction::Opcode::NAME} {}
#define DirectCallInstruction(NAME, ENCODING) NAME (Function * f, std::vector<Instruction*> && args, ASTBase const * ast): Instruction::ENCODING{f, std::move(args), ast, #NAME, Instruction::Opcode::NAME} {}
#define IndirectCallInstruction(NAME, ENCODING) NAME (Instruction * f, std::vector<Instruction*> && args, ASTBase const * ast): Instruction::ENCODING{f, std::move(args), ast, #NAME, Instruction::Opcode::NAME} {}
#define PhiInstruction(NAME, ENCODING) NAME ( ResultType type, ASTBase const * ast): Instruction::ENCODING{type, ast, #NAME, Instruction::Opcode::NAME} {}
#define ElemIndexInstruction(NAME, ENCODING) NAME (  Instruction * base, Instruction * offset, Instruction * index, ASTBase const * ast): Instruction::ENCODING{base,offset, index, ast, #NAME, Instruction::Opcode::NAME} {}
#define ElemOffsetInstruction(NAME, ENCODING) NAME (  Instruction * base, Instruction * offset, ASTBase const * ast): Instruction::ENCODING{base, offset, ast, #NAME, Instruction::Opcode::NAME} {}
#define StructAssignInstruction(NAME, ENCODING) NAME ( Instruction * srcVal, Instruction * dstAddr, Type::Struct * type, ASTBase const * ast): Instruction::ENCODING{srcVal, dstAddr, type, ast, #NAME, Instruction::Opcode::NAME} {}

#define INSTYPE(NAME, ENCODING, TYPE) class NAME : public Instruction::ENCODING { \
    public: \
        ENCODING(NAME, ENCODING, TYPE)                                            \
        virtual ~NAME(){}                                                     \
        inline virtual void accept(ILVisitor * v)     \
        { v->visit(this); } \
};

#define SrcInstruction(NAME, ENCODING, TYPE) NAME (Instruction * src, ASTBase const * ast): Instruction::ENCODING{src, TYPE, ast, #NAME, Instruction::Opcode::NAME} {}
#define VoidInstruction(NAME, ENCODING, TYPE) NAME (ASTBase const * ast): Instruction::ENCODING{TYPE, ast, #NAME, Instruction::Opcode::NAME} {}

#include "il_insns.h"


    class BasicBlock : public IL {
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

    protected:
        void accept(ILVisitor * v) override;

        friend class ILVisitor;
        friend class tvlm::CfgBuilder;
    private:
        std::string name_;
        std::vector<std::unique_ptr<Instruction>> insns_;
    }; // BasicBlock

    class Function : public IL{
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

        void setResultType(const ResultType & r){
            resultType_ = r;
        }
        ResultType getResultType()const{
            return resultType_;
        }

        bool contains(BasicBlock * b) const {
            for (auto const & i : bbs_)
                if (i.get() == b)
                    return true;
            return false;
        }

        void print(tiny::ASTPrettyPrinter & p) const {

            p << p.comment << "function: " ; //<< il_->type()->toString();
            p.newline();
            p << p.identifier << name_ << p.symbol << ":";
            p.indent();
            p.newline();
            for (auto & i : bbs_)
                i->print(p);
            p.dedent();

        }

        const ASTBase * ast(){
            return ast_;
        }

        friend class CfgBuilder;
    protected:

         void accept(ILVisitor * v) override;

    private:
        friend class ILBuilder;
        friend class ILVisitor;

        ASTBase const * ast_;

        std::string name_;

        std::vector<std::unique_ptr<BasicBlock>> bbs_;

        ResultType resultType_;

    };   // tvlm::Function

    class Program : public IL{
    public:
        Program( std::unordered_map<std::string, Instruction*> && stringLiterals,
                 std::vector<std::pair<Symbol, std::unique_ptr<Function>>> && functions,
                 std::unique_ptr<BasicBlock> && globals,
                 std::vector<std::unique_ptr<tvlm::Type>> && allocated_types
        ): stringLiterals_(std::move(stringLiterals)), functions_(std::move(functions)),
        globals_(std::move(globals)), allocated_types_(std::move(allocated_types)){}

        Program(Program && p):
        stringLiterals_(std::move(p.stringLiterals_)),
        functions_(std::move(p.functions_)),
        globals_(std::move(p.globals_)),
        allocated_types_(std::move(p.allocated_types_)){
        }
        Program(const Program & p) = delete;

        const std::vector<std::pair<Symbol, std::unique_ptr<Function>>> & functions() const {
            return functions_;
        }
        const std::unordered_map<std::string, Instruction*> & stringLiterals() const{
            return stringLiterals_;
        }
    protected:
        friend class ILVisitor;
         void accept(ILVisitor * v) override;
    private:
        std::unordered_map<std::string, Instruction*> stringLiterals_;
        std::vector<std::pair<Symbol, std::unique_ptr<Function>>> functions_;
        std::unique_ptr<BasicBlock> globals_;
        std::vector<std::unique_ptr<tvlm::Type>> allocated_types_;
    } ; // tvlm::Program


//    inline void Instruction::Terminator0::accept(ILVisitor * v) { v->visit(this); }
//    inline void Instruction::Terminator1::accept(ILVisitor * v) { v->visit(this); }
//    inline void Instruction::TerminatorN::accept(ILVisitor * v) { v->visit(this); }
//    inline void Instruction::Returnator::accept(ILVisitor * v) { v->visit(this); }
//    inline void Instruction::BinaryOperator::accept(ILVisitor * v) { v->visit(this); }
//    inline void Instruction::UnaryOperator::accept(ILVisitor * v) { v->visit(this); }
//    inline void Instruction::LoadAddress::accept(ILVisitor * v) { v->visit(this); }
//    inline void Instruction::StoreAddress::accept(ILVisitor * v) { v->visit(this); }
//    inline void Instruction::SrcInstruction::accept(ILVisitor * v) { v->visit(this); }
//    inline void Instruction::VoidInstruction::accept(ILVisitor * v) { v->visit(this); }
//    inline void Instruction::PhiInstruction::accept(ILVisitor * v) { v->visit(this); }
//    inline void Instruction::ElemIndexInstruction::accept(ILVisitor * v) { v->visit(this); }
//    inline void Instruction::ElemOffsetInstruction::accept(ILVisitor * v) { v->visit(this); }
//    inline void Instruction::StructAssignInstruction::accept(ILVisitor *v) {v->visit(this); }
//    inline void Instruction::IndirectCallInstruction::accept(ILVisitor * v) { v->visit(this); }
//    inline void Instruction::DirectCallInstruction::accept(ILVisitor * v) { v->visit(this); }
//
//
//
//    inline void Instruction::ImmValue::accept(ILVisitor * v) { v->visit(this); }
//    inline void Instruction::ImmSize::accept(ILVisitor * v) { v->visit(this); }
//    inline void Instruction::ImmIndex::accept(ILVisitor * v) { v->visit(this); }

    inline void BasicBlock::accept(ILVisitor * v) { v->visit(this); }
    inline void Function::accept(ILVisitor * v) { v->visit(this); }
    inline void Program::accept(ILVisitor * v) { v->visit(this); }


} // namespace tvlm
