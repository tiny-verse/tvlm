#pragma once

#include <vector>
#include <list>
#include <map>

#include <memory>

#include "common/helpers.h"
#include "common/ast.h"
#include "tinyc/types.h"

namespace tvlm{
    template<class T>
    class CfgBuilder;

    using ASTBase = tiny::ASTBase;
    using Symbol = tiny::Symbol;
//    using Type = tinyc::Type;

    class BasicBlock;
    class Function;
    class ILVisitor;
    class ILBuilder;
    class Program;

#include "il_insns_predeclare.h"

    class IL{
    public:
        virtual ~IL() = default;
        virtual void accept(ILVisitor * v) = 0;
        virtual bool operator==(const IL * il) const = 0;
    protected:
        friend class ILVisitor;
        IL * parent_;
    };
    class Instruction;

    /** Result type of an instruction. 
     */ 
    enum class ResultType {
        Integer,
        Double,
        Void,
        StructAddress,
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
//        class String;
        virtual ~Type() = default;

        virtual size_t size() const  = 0;
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
        size_t size() const override;

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
        size_t size() const override;

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
        size_t size() const override {
            return 0;
        }

        ResultType registerType() const override {
            return ResultType::Void;
        }

    private:
        void toStream(std::ostream & s) const override {
            s << "void";
        }
    };

//    class Type::String : public Type{
//    public:
//        explicit String(size_t size):size_(size){}
//        int size() const {
//            return (int)size_;
//        }
//
//        ResultType registerType() const override {
//            return ResultType::Integer; // Address of that string
//        }
//
//    private:
//        void toStream(std::ostream & s) const override {
//            s << "string (of size: " << size_ << ")";
//        }
//        size_t size_;
//    };
    class Type::Double : public Type{
    public:
        Double(){}
        size_t size() const override;

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
        size_t size()const override;

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
        size_t size()const override;

        ResultType registerType() const override {
            return ResultType::StructAddress;
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
        size_t size()const override{
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
            return ResultType::StructAddress; // Address of that Struct
        }
        const std::vector<std::pair<Symbol, Type *>> & fields() const {
            return fields_;
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
        class Terminator2;
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

        ~Instruction() override = default;

        bool operator==(const IL *il) const override = 0;
        virtual std::vector<Instruction *> children() const = 0;

        virtual void replaceWith(Instruction * sub, Instruction * toReplace ) = 0;
        void replaceMe(Instruction * with ) {
            for (auto  * ins: used_) {
                replaceWith(this, with);
            }
            with->used_ = used_;
        }
        //read as this is used in instruction [usage]
        virtual void registerUsage(Instruction * usage){
            used_.push_back(usage);
        };

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

        void setParentBB(BasicBlock * parent ){
            parentBB_ = parent;
        }

        BasicBlock * getParentBB()const{
            return parentBB_;
        }


        const std::vector<Instruction *> &  usages()const {
            return used_;
        }

        std::vector<Instruction *> & usages() {
            return used_;
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


            //BinOp,
            //UnOp,
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
        template<typename T>
        friend class LivenessAnalysis;
        /** Creates the instruction with given return type and corresponding abstract syntax tree. 
         */
        Instruction(ResultType resultType, ASTBase const * ast, const std::string & instrName, Opcode opcode):
            resultType_{resultType},
            ast_{ast} ,
            instrName_{instrName},
            opcode_(opcode){
        }

        static void printResultType(tiny::ASTPrettyPrinter & p,const ResultType & res) {
            switch (res) {
                case ResultType::Integer:
                    p << "int ";
                    break;
                case ResultType::Double:
                    p << "double ";
                    break;
                case ResultType::Void:
//                    p << "void ";
//                    p << "";
                    break;
                case ResultType::StructAddress:
                    p << "addr ";
                    break;
            }

        }

        void printRegister(tiny::ASTPrettyPrinter & p, Instruction const * reg) const {
            p << p.identifier << reg->name() << p.symbol << ": " << p.keyword;
            printResultType(p, reg->resultType_);
        }

        static void printRegisterAddress(tiny::ASTPrettyPrinter & p, Instruction const * reg) {
            p << p.symbol << "[" << p.identifier << reg->name() << p.symbol << "] ";
        }

        std::vector<Instruction *> used_;
        std::string instrName_;
        ASTBase const * ast_;
    private:


        ResultType resultType_;
        std::string name_;
        BasicBlock * parentBB_;

    }; // tinyc::il::Instruction

    class Instruction::ImmSize : public Instruction {
    public:
        std::vector<Instruction*> children() const override{
            return {amount_};
        }

        bool operator==(const IL *il) const override {
            if( auto * other = dynamic_cast<const Instruction::ImmSize*>(il)){
                return size_ == other->size_ && amount_->operator==(other->amount_);
            }
            else return false;
        }

        int size() const {
            return size_;
//            return type_->size();
        }

        void replaceWith(Instruction *sub, Instruction *toReplace) override {
            if(amount_ == sub){
                amount_ = toReplace;
                toReplace->registerUsage(this);
            }
        }

        void print(tiny::ASTPrettyPrinter & p) const override {

            Instruction::print(p);
            p << p.keyword << instrName_ << " "<< p.numberLiteral << size() << p.keyword << " (size: " << size_ <<   ")";
            if(amount_){
                p << p.keyword << " x ";
                printRegister(p, amount_);
            }
        };
        Instruction * amount()const {
            return amount_;
        }
        ~ImmSize() override{}
    protected:
        // void accept(ILVisitor * v) override;

        ImmSize(Type * type,Instruction * amount, ASTBase const * ast, const std::string & instrName, Opcode opcode):
            Instruction{ResultType::Integer, ast, instrName, opcode},
            size_{type->size()},
            amount_(amount){
            if(amount && amount->resultType() != ResultType::Integer ){
                throw tiny::ParserError(STR("vector size has to be of type Int"), ast->location());
            }
            assert(!amount || amount->resultType() == ResultType::Integer );

            if(amount_) amount_->registerUsage(this);
        }
        ImmSize(size_t size, ASTBase const * ast, const std::string & instrName, Opcode opcode):
            Instruction{ResultType::Integer, ast, instrName, opcode},
            size_{size},
            amount_(nullptr){
        }
        ImmSize(Type * type, ASTBase const * ast, const std::string & instrName, Opcode opcode ):
                ImmSize(type, nullptr, ast, instrName, opcode){}
        Instruction * amount_;
        size_t size_;
//        Type * type_;
    };

    class Instruction::ImmIndex : public Instruction {
    public:
        std::vector<Instruction*> children() const override{
            return {};
        }
        bool operator==(const IL *il) const override {
            if( auto * other = dynamic_cast<const Instruction::ImmIndex*>(il)){
                return type_ == other->type_ && index_ == other->index_;
            }
            else return false;
        }

        Type * type()const{
            return type_;
        }
        size_t index() const{
            return index_;   
        }
//        std::vector<Instruction *> args()const{
//            return args_;
//        }

        void replaceWith(Instruction *sub, Instruction *toReplace) override {
            //nothing to do
        }

        void print(tiny::ASTPrettyPrinter & p) const override {
            Instruction::print(p);
            p << p.keyword << instrName_ << " " << p.numberLiteral << index_;
        };

        ~ImmIndex() override = default;
    protected:

        // void accept(ILVisitor * v) override;
        ImmIndex(size_t index, Type * type, ASTBase const * ast, const std::string & instrName, Opcode opcode):
            Instruction{type->registerType(), ast, instrName, opcode},
            index_{index}, type_(type) {

        }
        size_t index_;
        Type * type_;
    };

    class Instruction::ImmValue : public Instruction {
    public:
        std::vector<Instruction*> children() const override{
            return {};
        }

        bool operator==(const IL *il) const override {
            if( auto * other = dynamic_cast<const Instruction::ImmValue *>(il)){
                return
                resultType() == other->resultType() &&
                resultType() == ResultType::Integer ? (value_.i == other->value_.i) :  (value_.f == other -> value_.f);
            }
            else return false;
        }

        int64_t valueInt() const {
            return value_.i;
        }
        double valueFloat() const {
            return value_.f;
        }
        void print(tiny::ASTPrettyPrinter & p) const override {
            Instruction::print(p);
            p << p.keyword << instrName_ << " " << p.numberLiteral << (resultType() == ResultType::Integer ? value_.i : value_.f);
        };

        void replaceWith(Instruction *sub, Instruction *toReplace) override {
            //nothing to do
        }

        ~ImmValue() override= default;
    protected:

        // void accept(ILVisitor * v) override;
        ImmValue(int64_t value, ASTBase const * ast, const std::string & instrName, Opcode opcode):
        Instruction{ResultType::Integer, ast, instrName, opcode}, value_()
        {
            value_.i = value;
        }

        ImmValue(double value, ASTBase const * ast, const std::string & instrName, Opcode opcode):
        Instruction{ResultType::Double, ast, instrName, opcode}, value_()
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
        std::vector<Instruction*> children() const override{
            return {lhs_, rhs_};
        }

        bool operator==(const IL *il) const override {
            if( auto * other = dynamic_cast<const Instruction::BinaryOperator*>(il)){
                return
                op_ == other->op_&& operator_ == other->operator_ &&
                lhs_->operator==(other->lhs_) && rhs_->operator==(other->rhs_);
            }
            else return false;
        }
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


        void print(tiny::ASTPrettyPrinter & p) const override {
            Instruction::print(p);
            p << p.keyword << resolve_operator();
            printRegister(p, lhs_);
            printRegister(p, rhs_);
        };

        ~BinaryOperator() override= default;

        void replaceWith(Instruction *sub, Instruction *toReplace) override;

    protected:
        const char *  resolve_operator() const;
        // void accept(ILVisitor * v) override;

        BinaryOperator(Opcode op, BinOpType oper, Instruction * lhs, Instruction * rhs, ASTBase const * ast, const std::string & instrName):
            Instruction{GetResultType(lhs, rhs, oper), ast, instrName, op},
            op_{op},
            operator_{oper},
            lhs_{lhs},
            rhs_{rhs} {
            assert(lhs->resultType() == rhs->resultType());
            lhs->registerUsage(this);
            rhs->registerUsage(this);
        }
        static bool convertsToBool(const BinOpType & op) {
            switch (op){
                case BinOpType::LT:
                case BinOpType::LTE:
                case BinOpType::GT:
                case BinOpType::GTE:
                case BinOpType::EQ:
                case BinOpType::NEQ:
//                case BinOpType::AND:
//                case BinOpType::OR:
//                case BinOpType::XOR:
                return true;
            default:
                return false;
            }
        }

        static ResultType GetResultType(Instruction * lhs, Instruction * rhs, const BinOpType & op) {
            assert(lhs->resultType() != ResultType::Void && rhs->resultType() != ResultType::Void);
            if(lhs->resultType() == rhs->resultType() && lhs->resultType() == ResultType::Double && convertsToBool(op)){
                return ResultType::Integer;
            }else if (lhs->resultType() == ResultType::Double || rhs->resultType() == ResultType::Double)
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
        std::vector<Instruction*> children() const override{
            return {operand_};
        }

        bool operator==(const IL *il) const override {
            if( auto * other = dynamic_cast<const Instruction::UnaryOperator*>(il)){
                return
                operator_ == other->operator_ &&
                operand_->operator==( other->operand_ )
                ;
            }
            else return false;
        }
        Instruction * operand() const {
            return operand_;
        }
        UnOpType opType() const{
            return operator_;
        }
        void print(tiny::ASTPrettyPrinter & p) const override {
            Instruction::print(p);
            p << p.keyword << resolve_operator() << " ";
            printRegister(p, operand_);
        };

        ~UnaryOperator() override{}

        void replaceWith(Instruction *sub, Instruction *toReplace) override;

    protected:
        const char *  resolve_operator() const;
        // void accept(ILVisitor * v) override;

        UnaryOperator( Opcode op, UnOpType oper, Instruction * operand, ASTBase const * ast, const std::string & instrName):
            Instruction{operand->resultType(), ast, instrName, op},
            operator_{oper},
            operand_{operand} {
            operand->registerUsage(this);
        }

    private:
        UnOpType operator_;
        Instruction * operand_;

    }; 

    class Instruction::LoadAddress : public Instruction {
    public:
        std::vector<Instruction*> children() const override{
            return {address_};
        }

        bool operator==(const IL *il) const override {
            if( auto * other = dynamic_cast<const Instruction::LoadAddress*>(il)){
                return resultType_ == other->resultType_ && address_->operator==(other->address_);
            }
            else return false;
        }
        Instruction * address() const { return address_; }
        Type * type() const { return type_; }

        void print(tiny::ASTPrettyPrinter & p) const override {
            Instruction::print(p);
            p << p.keyword << instrName_ << " ";
            printRegisterAddress(p, address_);
        };

        ~LoadAddress() override= default;

        void replaceWith(Instruction *sub, Instruction *toReplace) override {
            if(address_ == sub){
                address_ = toReplace;
                toReplace->registerUsage(this);
            }
        }

    protected:
        // void accept(ILVisitor * v) override;

        LoadAddress(Instruction * address,Type * type, ASTBase const * ast, const std::string & instrName, Opcode opcode):
            Instruction{type->registerType(), ast, instrName, opcode},
            address_{address}, type_(type) {

            address_->registerUsage(this);
        }
        LoadAddress(Instruction * address,const ResultType & type, ASTBase const * ast, const std::string & instrName, Opcode opcode):
            Instruction{type, ast, instrName, opcode},
            address_{address}, type_(nullptr) {

            address_->registerUsage(this);
        }
    private:

        Instruction * address_;
        Type * type_;

    }; // Instruction::LoadAddress

    class Instruction::StoreAddress : public Instruction {
    public:
        std::vector<Instruction*> children() const override{
            return {address_, value_};
        }
        bool operator==(const IL *il) const override {
            if( auto * other = dynamic_cast<const Instruction::StoreAddress*>(il)){
                return address_->operator==(other->address_) && value_->operator==(other->value_);
            }
            else return false;
        }
        Instruction * value() const { return value_; }

        Instruction * address() const { return address_; }

        void print(tiny::ASTPrettyPrinter & p) const override {
            Instruction::print(p);
            p << p.keyword << instrName_ << " ";
            printRegisterAddress(p, address_);
            printRegister(p, value_);
        };
        void replaceWith(Instruction *sub, Instruction *toReplace) override {
            if(address_ == sub){
                address_ = toReplace;
                toReplace->registerUsage(this);
            }
            if(value_ == sub){
                value_ = toReplace;
                toReplace->registerUsage(this);
            }
        }
        ~StoreAddress() override= default;
    protected:
        // void accept(ILVisitor * v) override;

        StoreAddress(Instruction * value, Instruction * address, ASTBase const * ast, const std::string & instrName, Opcode opcode):
            Instruction{ResultType::Void, ast, instrName, opcode},
            value_{value},
            address_{address} {

            address_->registerUsage(this);
            value_->registerUsage(this);
        }
    private:

        Instruction * value_;
        Instruction * address_;

    }; // Instruction::StoreAddress

    class Instruction::Terminator : public Instruction {
    public:

        virtual size_t numTargets() const = 0;

        virtual BasicBlock * getTarget(size_t i) const = 0;

        ~Terminator() override= default;
    protected:
        Terminator(ASTBase const * ast, const std::string & instrName, Opcode opcode):
            Instruction{ResultType::Void, ast, instrName, opcode} {
        }
    }; // Instruction::Terminator

    class Instruction::Terminator0 : public Instruction::Terminator {
    public:
        virtual bool operator==(const IL *il) const override;
        size_t numTargets() const override { return 0; }

        BasicBlock * getTarget(size_t i) const override { return nullptr; }

        void print(tiny::ASTPrettyPrinter & p) const override {
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
        std::vector<Instruction*> children() const override{
            return {returnValue_};
        }

        bool operator==(const IL *il) const override {
            if( auto * other = dynamic_cast<const Instruction::Returnator*>(il)){
                return type_ == other->type_ && returnValue_->operator==(other->returnValue_);
            }
            else return false;
        }

        Instruction * returnValue()const{
            return returnValue_;
        }

        Type * returnType()const{
            return type_;
        }

        void replaceWith(Instruction *sub, Instruction *toReplace) override {
            if(returnValue_ == sub){
                returnValue_ = toReplace;
                toReplace->registerUsage(this);
            }
        }

        void print(tiny::ASTPrettyPrinter & p) const override {
            tvlm::Instruction::Terminator0::print(p);
            p << p.keyword << instrName_ << " ";
            if(returnValue_) printRegister(p, returnValue_);
        }

        ~Returnator() override= default;
    protected:
        // void accept(ILVisitor * v) override;

        Returnator(Instruction * retValue, Type * type, ASTBase const * ast, const std::string & instrName, Opcode opcode):
            Terminator0{ast, instrName, opcode}, returnValue_{retValue}, type_(type) {

            if(returnValue_) returnValue_->registerUsage(this);
        }
        Instruction * returnValue_;
        Type * type_;
    }; // Instruction::Returnator

    class Instruction::Terminator1 : public Instruction::Terminator {
    public:
        std::vector<Instruction*> children() const override{
            return {};
        }

        bool operator==(const IL *il) const override;

        size_t numTargets() const override { return 1; }

        BasicBlock * getTarget(size_t i) const override { return i == 1 ? target_ : nullptr; }

        void print(tiny::ASTPrettyPrinter & p) const override ;

    protected:
    public:
        void replaceWith(Instruction *sub, Instruction *toReplace) override{

    }

    protected:
        // void accept(ILVisitor * v) override;

        Terminator1(BasicBlock * target, ASTBase const * ast, const std::string & instrName, Opcode opcode);

        ~Terminator1() override = default;
    private:
        BasicBlock * target_;
    }; // Instruction::Terminator1

    class Instruction::Terminator2 : public Instruction::Terminator {
    public:
        std::vector<Instruction*> children() const override{
            return {cond_};
        }

        bool operator==(const IL *il) const override;
        Instruction * condition() const { return cond_; }

        size_t numTargets() const override { return targets_.size(); }

        BasicBlock * getTarget(size_t i) const override { return targets_[i]; }

//        void addTarget(BasicBlock * target) {
//            targets_.push_back(target);
//        }
        void print(tiny::ASTPrettyPrinter & p) const override;
        void replaceWith(Instruction *sub, Instruction *toReplace) override{
            if(cond_ == sub){
                cond_ = toReplace;
                toReplace->registerUsage(this);
            }
        }
    protected:
        // void accept(ILVisitor * v) override;

        Terminator2(Instruction * cond,BasicBlock * trueTarget, BasicBlock * falseTarget, ASTBase const * ast, const std::string & instrName, Opcode opcode);

        ~Terminator2() override = default;
    private:
        Instruction * cond_;
        std::vector<BasicBlock *> targets_;
    }; // Instruction::Terminator2

    class Instruction::SrcInstruction : public Instruction{
    public:
        std::vector<Instruction*> children() const override{
            return {src_};
        }

        bool operator==(const IL *il) const override{
            if( auto * other = dynamic_cast<const Instruction::SrcInstruction*>(il)){
                return opcode_ == other->opcode_ && src_->operator==(other->src_);
            }
            else return false;
        }

        Instruction * src(){
            return src_;
        }

        void print(tiny::ASTPrettyPrinter & p) const override {
            Instruction::print(p);
            p << p.keyword << instrName_ << " ";
            printRegister(p, src_);
        }
        void replaceWith(Instruction *sub, Instruction *toReplace) override{
            if(src_ == sub){
                src_ = toReplace;
                toReplace->registerUsage(this);
            }
        }
        ~SrcInstruction() override = default;
    protected:
        // void accept(ILVisitor * v) override;

        SrcInstruction(Instruction * src, ResultType type, ASTBase const * ast, const std::string & instrName, Opcode opcode):
                Instruction{type, ast, instrName, opcode},
                src_{src}{
            src_->registerUsage(this);
        }
    private:
        Instruction * src_;
    }; // Instruction::SrcInstruction

    class Instruction::VoidInstruction : public Instruction{
    public:
        std::vector<Instruction*> children() const override{
            return {};
        }

        bool operator==(const IL *il) const override{
            if( auto * other = dynamic_cast<const Instruction::VoidInstruction*>(il)){
                return opcode_ == other->opcode_;
            }
            else return false;
        }


        void print(tinyc::ASTPrettyPrinter & p) const override{
            Instruction::print(p);
            p << p.keyword << instrName_ << " ";
        }

        ~VoidInstruction() override = default;

        void replaceWith(Instruction *sub, Instruction *toReplace) override {

        }

    protected:
//        virtual void accept(ILVisitor * v) override;

        VoidInstruction( ResultType type, ASTBase const * ast, const std::string & instrName, Opcode opcode):
                Instruction{type, ast, instrName, opcode}{

        }
    }; // Instruction::VoidInstruction

    class Instruction::PhiInstruction : public Instruction{
    public:
        std::vector<Instruction*> children() const override{
//            throw "not implemented children on Phi Instruction";
            std::vector<Instruction*> successors;
            for (auto & c: contents_) {
                successors.push_back(c.second);
            }
            return std::move(successors);
        }
        bool operator==(const IL *il) const override{
            if( auto * other = dynamic_cast<const Instruction::PhiInstruction*>(il)){
                return contents_ ==  other->contents_;
            }
            else return false;
        }


        void addIncomming( Instruction * src, BasicBlock * bb){
            contents_.emplace(bb, src);
        }

        void print(tiny::ASTPrettyPrinter & p) const override;
        std::unordered_map<BasicBlock*, Instruction *> contents()const{
            return contents_;
        }
        void replaceWith(Instruction *sub, Instruction *toReplace) override{
            for (auto & i : contents_) {
                if(i.second == sub)
                    i.second = toReplace;
                toReplace->registerUsage(this);
            }
        }
        ~PhiInstruction() override = default;

    protected:
//        virtual void accept(ILVisitor * v) override;

        PhiInstruction( ResultType type, ASTBase const * ast, const std::string & instrName, Opcode opcode):
                Instruction{type, ast, instrName, opcode}{
            for (auto & p : contents_) {
                p.second->registerUsage(this);
            }
        }
    private:
        std::unordered_map<BasicBlock*, Instruction *> contents_;
    }; // Instruction::PhiInstruction

  class Instruction::StructAssignInstruction : public Instruction{
    public:
      std::vector<Instruction*> children() const override{
          return {srcVal_, dstAddr_};
      }
      bool operator==(const IL *il) const override{
          if( auto * other = dynamic_cast<const Instruction::StructAssignInstruction*>(il)){
              return type_ == other->type_ &&
                    srcVal_->operator==(other->srcVal_) &&
                    dstAddr_->operator==(other->dstAddr_) ;
          }
          else return false;
      }


      void print(tiny::ASTPrettyPrinter & p) const override;

      ~StructAssignInstruction() override = default;

      Instruction * srcVal()const {
          return srcVal_;
      }

      Instruction * dstAddr()const {
          return dstAddr_;
      }

      Type::Struct * type()const {
          return type_;
      }
      void replaceWith(Instruction *sub, Instruction *toReplace) override{
        if(srcVal_ == sub){
            srcVal_ = toReplace;
            toReplace->registerUsage(this);
        }
        if(dstAddr_ == sub){
            dstAddr_ = toReplace;
            toReplace->registerUsage(this);
        }
      }

    protected:
//      virtual void accept(ILVisitor * v) override;

        StructAssignInstruction( Instruction * srcVal, Instruction * dstAddr, Type::Struct * type, ASTBase const * ast, const std::string & instrName, Opcode opcode):
                Instruction{ResultType::Void, ast, instrName, opcode},
                srcVal_(srcVal),
                dstAddr_(dstAddr),
                type_(type)
                {
                srcVal_->registerUsage(this);
                dstAddr_->registerUsage(this);
        }
    private:
        Instruction * srcVal_;
        Instruction * dstAddr_;
        Type::Struct * type_;
    }; // Instruction::StructAssignInstruction

    class Instruction::ElemInstruction : public Instruction{
    public:
        std::vector<Instruction*> children() const override{
            return {base_};
        }
        bool operator==(const IL *il) const override{
            if( auto * other = dynamic_cast<const Instruction::ElemInstruction*>(il)){
                return base_->operator==(other->base_);
            }
            else return false;
        }

        ~ElemInstruction() override = default;
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
        std::vector<Instruction*> children() const override{
            return {base_, offset_};
        }
        bool operator==(const IL *il) const override{
            if( auto * other = dynamic_cast<const Instruction::ElemOffsetInstruction*>(il)){
                return base_->operator==(other->base_) && offset_->operator==(other->offset_);
            }
            else return false;
        }

        void print(tiny::ASTPrettyPrinter & p) const override;

        ~ElemOffsetInstruction() override = default;

        Instruction * offset()const {
            return offset_;
        }
        void replaceWith(Instruction *sub, Instruction *toReplace) override{
            if(base_ == sub) {
                base_ = toReplace;
                toReplace->registerUsage(this);
            }
            if(offset_ == sub){
                offset_ = toReplace;
                toReplace->registerUsage(this);
            }

        }
    protected:
//        virtual void accept(ILVisitor * v) override;

        ElemOffsetInstruction( Instruction * base, Instruction * offset,  ASTBase const * ast, const std::string & instrName, Opcode opcode):
                ElemInstruction{base, ast, instrName, opcode },
                offset_(offset){

            base_->registerUsage(this);
            offset_->registerUsage(this);
        }
    private:
        Instruction * offset_;
    }; // Instruction::ElemOffsetInstruction

class Instruction::ElemIndexInstruction : public Instruction::ElemInstruction{
    public:
        std::vector<Instruction*> children() const override{
            return {base_, offset_, index_};
        }
        bool operator==(const IL *il) const override{
            if( auto * other = dynamic_cast<const Instruction::ElemIndexInstruction*>(il)){
                return base_->operator==(other->base_)
                    && offset_->operator==(other->offset_)
                    && index_->operator==(other->index_);
            }
            else return false;
        }

    void print(tiny::ASTPrettyPrinter & p) const override;

    ~ElemIndexInstruction() override = default;
        Instruction * offset()const {
            return offset_;
        }
        Instruction * index()const {
            return index_;
        }
    void replaceWith(Instruction *sub, Instruction *toReplace) override{
        if(base_ == sub){
            base_ = toReplace;
            toReplace->registerUsage(this);
        }
        if(offset_ == sub){
            offset_ = toReplace;
            toReplace->registerUsage(this);
        }
        if(index_ == sub){
            index_ = toReplace;
            toReplace->registerUsage(this);
        }
    }
    protected:
//    virtual void accept(ILVisitor * v) override;

        ElemIndexInstruction( Instruction * base,Instruction * offset, Instruction * index, ASTBase const * ast, const std::string & instrName, Opcode opcode):
                ElemInstruction{base, ast, instrName, opcode },
                index_(index),
                offset_(offset){

            base_->registerUsage(this);
            index_->registerUsage(this);
            offset_->registerUsage(this);
        }
    private:
        Instruction * index_;
        Instruction * offset_;
    }; // Instruction::ElemIndexInstruction

    class Instruction::CallInstruction : public Instruction{
    public:

        std::vector<Instruction*> children() const override{
            std::vector<Instruction*> predecessors;
            for (auto & arg : args_) {
                predecessors.push_back(arg.first);
            }
            return std::move(predecessors);
        }

        Instruction * operator [] (size_t i) const {
            assert(i < args_.size());
            return args_[i].first;
        }

        const std::vector<std::pair< Instruction *, Type*>> & args()const {
            return args_;
        }

        ~CallInstruction() override =default;



    protected:

        CallInstruction(std::vector<std::pair< Instruction *, Type*>> && args,
                        ASTBase const * ast, const std::string & instrName
                        , Opcode opcode , const ResultType & resultType):
                Instruction{resultType, ast, instrName, opcode},
                args_{args}{
            for (auto & a :args_ ) {
                a.first->registerUsage(this);

            }
        }
        std::vector<std::pair< Instruction *, Type*>> args_;
    }; // Instruction::CallInstruction

    class Instruction::DirectCallInstruction : public CallInstruction{
    public:


        bool operator==(const IL *il) const override;


        Function * f() const {
            return f_;
        }

        ~DirectCallInstruction() override = default;
        void print(tiny::ASTPrettyPrinter & p) const override;
        void replaceWith(Instruction *sub, Instruction *toReplace) override {
            for (auto & a : args_) {
                if(a.first == sub){
                    a.first = toReplace;
                    toReplace->registerUsage(this);
                }
            }
        }
    protected:
//        virtual void accept(ILVisitor * v) override;

        DirectCallInstruction(Function * f, std::vector<std::pair< Instruction *, Type*>> && args,
                              const ASTBase * ast , const std::string & instrName, Opcode opcode);
        Function * f_;
    }; // Instruction::DirectCallInstruction

    class Instruction::IndirectCallInstruction : public CallInstruction{
    public:
        std::vector<Instruction*> children() const override{
            std::vector<Instruction*> predecessors{f_};
            for (auto & arg : args_) {
                predecessors.push_back(arg.first);
            }
            return std::move(predecessors);
        }
        virtual bool operator==(const IL *il) const override {
            if( auto * other = dynamic_cast<const Instruction::IndirectCallInstruction*>(il)){
                bool tmp = f_->operator==(other->f_) && retType_ == other->retType_ ;
                for (int i = 0; i < args_.size(); ++i) {
                    tmp = tmp && args_[i].second == other->args_[i].second
                          && args_[i].first->operator==(other->args_[i].first);
                }
                return tmp;
            }
            else return false;
        }

        Instruction * f() const {
            return f_;
        }
        Type * retType() const {
            return retType_;
        }
        void replaceWith(Instruction *sub, Instruction *toReplace) override {
            if(f_ == sub){
                f_ = toReplace;
                toReplace->registerUsage(this);
            }
            for (auto & a : args_) {
                if(a.first == sub){
                    a.first = toReplace;
                    toReplace->registerUsage(this);
                }
            }
        }
        void print(tiny::ASTPrettyPrinter & p) const override {
            Instruction::print(p);
            p << p.keyword << instrName_ << " ";
            printRegisterAddress(p, f_);
            p << p.symbol << "(";
            for (auto & i : args_)
                printRegister(p, i.first);
            p << p.symbol << ")";
        };

        ~IndirectCallInstruction() override = default;
    protected:
//        virtual void accept(ILVisitor * v) override;

        IndirectCallInstruction(Instruction * f,Type * retType, std::vector<std::pair< Instruction *, Type*>> && args,const ASTBase * ast, const std::string & instrName, Opcode opcode ):
                CallInstruction{std::move(args), ast, instrName, opcode, f->resultType()},
                f_{f}, retType_(retType)
                {
                for (auto & a :args_ ) {
                    a.first->registerUsage(this);

                }
                f->registerUsage(this);
        }
        Instruction * f_;
        Type * retType_;
    }; // Instruction::IndirectCallInstruction






    class ILVisitor {
    public:
        virtual ~ILVisitor() = default;
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
        static BasicBlock*  getProgramsGlobals(ILBuilder & p) ;
        static Instruction*  getVariableAddress(ILBuilder &p, const Symbol & name);

    };

    // tinyc::ASTVisitor






#define INS(NAME, ENCODING) class NAME : public Instruction::ENCODING { \
    public: \
        ENCODING(NAME, ENCODING)                                        \
        virtual ~NAME(){}                                           \
        virtual void accept(ILVisitor * v)     \
        { v->visit(this); } \
};

#define ImmSize(NAME, ENCODING) NAME (Type * type,Instruction * amount, ASTBase const * ast) : Instruction::ENCODING{type, amount, ast, #NAME, Instruction::Opcode::NAME} {} \
                                NAME (size_t size, ASTBase const * ast) : Instruction::ENCODING{size, ast, #NAME, Instruction::Opcode::NAME} {}
#define ImmIndex(NAME, ENCODING) NAME (size_t index, Type * type,  ASTBase const * ast) : Instruction::ENCODING{index,type, ast, #NAME, Instruction::Opcode::NAME} {}
#define ImmValue(NAME, ENCODING) NAME (int64_t value, ASTBase const * ast) : Instruction::ENCODING{value, ast, #NAME, Instruction::Opcode::NAME} {} \
                                 NAME (double value, ASTBase const * ast) : Instruction::ENCODING{value, ast, #NAME, Instruction::Opcode::NAME} {}
#define BinaryOperator(NAME, ENCODING) NAME (BinOpType oper, Opcode op, Instruction * lhs, Instruction * rhs, ASTBase const * ast): Instruction::ENCODING{op, oper, lhs, rhs, ast, #NAME} {}
#define UnaryOperator(NAME, ENCODING) NAME (UnOpType oper, Opcode op, Instruction * operand, ASTBase const * ast): Instruction::ENCODING{op, oper, operand, ast, #NAME} {}
#define Terminator0(NAME, ENCODING) NAME (ASTBase const * ast): Instruction::ENCODING{ast, #NAME, Instruction::Opcode::NAME} {}
#define Returnator(NAME, ENCODING) NAME (Instruction * returnValue, Type * type, ASTBase const * ast): Instruction::ENCODING{returnValue, type, ast, #NAME, Instruction::Opcode::NAME} {}
#define Terminator1(NAME, ENCODING) NAME (BasicBlock * target, ASTBase const * ast): Instruction::ENCODING{target, ast, #NAME, Instruction::Opcode::NAME} {}
#define Terminator2(NAME, ENCODING) NAME (Instruction * condition,BasicBlock * trueTarget, BasicBlock * falseTarget,  ASTBase const * ast): Instruction::ENCODING{condition, trueTarget, falseTarget, ast, #NAME, Instruction::Opcode::NAME} {}
#define LoadAddress(NAME, ENCODING) NAME (Instruction * address, Type * type, ASTBase const * ast): Instruction::ENCODING{address, type, ast, #NAME, Instruction::Opcode::NAME} {} \
                                    NAME (Instruction * address, const ResultType & type, ASTBase const * ast): Instruction::ENCODING{address, type, ast, #NAME, Instruction::Opcode::NAME} {}
#define StoreAddress(NAME, ENCODING) NAME (Instruction * value, Instruction * address, ASTBase const * ast): Instruction::ENCODING{value, address, ast, #NAME, Instruction::Opcode::NAME} {}
#define DirectCallInstruction(NAME, ENCODING) NAME (Function * f, std::vector<std::pair< Instruction *, Type*>> && args, ASTBase const * ast): Instruction::ENCODING{f, std::move(args), ast, #NAME, Instruction::Opcode::NAME} {}
#define IndirectCallInstruction(NAME, ENCODING) NAME (Instruction * f, Type * retType, std::vector<std::pair< Instruction *, Type*>> && args, ASTBase const * ast): Instruction::ENCODING{f, retType, std::move(args), ast, #NAME, Instruction::Opcode::NAME} {}
#define PhiInstruction(NAME, ENCODING) NAME ( ResultType type, ASTBase const * ast): Instruction::ENCODING{type, ast, #NAME, Instruction::Opcode::NAME} {}
#define ElemIndexInstruction(NAME, ENCODING) NAME (  Instruction * base, Instruction * offset, Instruction * index, ASTBase const * ast): Instruction::ENCODING{base,offset, index, ast, #NAME, Instruction::Opcode::NAME} {}
#define ElemOffsetInstruction(NAME, ENCODING) NAME (  Instruction * base, Instruction * offset, ASTBase const * ast): Instruction::ENCODING{base, offset, ast, #NAME, Instruction::Opcode::NAME} {}
#define StructAssignInstruction(NAME, ENCODING) NAME ( Instruction * srcVal, Instruction * dstAddr, Type::Struct * type, ASTBase const * ast): Instruction::ENCODING{srcVal, dstAddr, type, ast, #NAME, Instruction::Opcode::NAME} {}

#define INSTYPE(NAME, ENCODING, TYPE) class NAME : public Instruction::ENCODING { \
    public: \
        ENCODING(NAME, ENCODING, TYPE)                                            \
        virtual ~NAME(){}                                                     \
        virtual void accept(ILVisitor * v)     \
        { v->visit(this); } \
};

#define SrcInstruction(NAME, ENCODING, TYPE) NAME (Instruction * src, ASTBase const * ast): Instruction::ENCODING{src, TYPE, ast, #NAME, Instruction::Opcode::NAME} {}
#define VoidInstruction(NAME, ENCODING, TYPE) NAME (ASTBase const * ast): Instruction::ENCODING{TYPE, ast, #NAME, Instruction::Opcode::NAME} {}

#include "il_insns.h"


    class BasicBlock : public IL {
    public:

        bool operator==(const IL *il) const override {
            if( auto * other = dynamic_cast<const BasicBlock*>(il)){
                return this == other;
            }
            else
                return false;
        }

        Instruction * add(Instruction * ins) {
            assert(! terminated());
            ins->setParentBB(this);
            insns_.push_back(std::unique_ptr<Instruction>{ins});
            return ins;
        }

        Instruction * inject(Instruction * ins, size_t pos = 0) {
            ins->setParentBB(this);
            insns_.insert( insns_.begin() + pos, std::unique_ptr<Instruction>{ins});
            return ins;
        }
        Instruction * injectAfter(Instruction * ins, const Instruction * pos ) {
            auto it = insns_.begin();
            for (;it != insns_.end() && it->get() != pos ;it++ );
            if(it == insns_.end()){
                return nullptr;
            }
            it++;
            ins->setParentBB(this);
            ins->setName(STR("injectA " << pos->name()) );
            insns_.insert( it, std::unique_ptr<Instruction>{ins});
            return ins;
        }
        Instruction * injectBefore(Instruction * ins, const Instruction * pos ) {
            auto it = insns_.begin();
            for (;it != insns_.end() && it->get() != pos ;it++ );
            if(it == insns_.end()){
                return nullptr;
            }
            ins->setParentBB(this);
            ins->setName(STR("injectB " << pos->name()) );
            insns_.insert( it, std::unique_ptr<Instruction>{ins});
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

        void registerUsage(Instruction * ins){
            usages_.emplace_back(ins);
        }

        void addSucc(BasicBlock * bl){
            successor_.push_back(bl);
        }
        void addPred(BasicBlock * bl){
            predecessor_.push_back(bl);
        }
        void replaceWith(BasicBlock *sub, BasicBlock *toReplace) {
            //TODO
            throw "not implemented replacing of BasicBLocks";



        }
    protected:
        void accept(ILVisitor * v) override{ v->visit(this); };

        friend class ILVisitor;
        template<class T>
        friend class tvlm::CfgBuilder;
    private:
        std::string name_;
        std::vector<std::unique_ptr<Instruction>> insns_;
        std::vector<Instruction*> usages_;
        std::vector<BasicBlock*>predecessor_;   //only inside function
        std::vector<BasicBlock*>successor_;     //only inside function
    }; // BasicBlock

    class Function : public IL{
    public:
        bool operator==(const IL *il) const override {
            if( auto * other = dynamic_cast<const Function *>(il)){
                return this == other;
            }
            else return false;
        }
        explicit Function(ASTBase const * ast):
            ast_{ast}
            ,type_()
            ,resultType_(ResultType::Void){
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
        void setType(Type * r){
            type_ = r;
        }
        Type * getType()const{
            return type_;
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
        template<class T>
        friend class CfgBuilder;
    protected:

         void accept(ILVisitor * v) override{ v->visit(this); };

    private:
        friend class ILBuilder;
        friend class ILVisitor;

        ASTBase const * ast_;

        std::string name_;

        std::vector<std::unique_ptr<BasicBlock>> bbs_;

        ResultType resultType_;

        Type * type_;

    };   // tvlm::Function

    class Program : public IL{
    public:
        bool operator==(const IL *il) const override {
            if( auto * other = dynamic_cast<const Program*>(il)){
                return this == other;
            }
            else return false;
        }
        Program()= default;
        Program( std::map< Instruction*,std::string> && stringLiterals,
                 std::vector<std::pair<Symbol, std::unique_ptr<Function>>> && functions,
                 std::unique_ptr<BasicBlock> && globals,
                 std::vector<std::unique_ptr<tvlm::Type>> && allocated_types,
                 std::unordered_map<Symbol, Instruction*> && globalNames
        ): stringLiterals_(std::move(stringLiterals)), functions_(std::move(functions)),
        globals_(std::move(globals)), allocated_types_(std::move(allocated_types)), globalNames_(std::move(globalNames)){}

        Program(Program && p) noexcept :
        stringLiterals_(std::move(p.stringLiterals_)),
        functions_(std::move(p.functions_)),
        globals_(std::move(p.globals_)),
        allocated_types_(std::move(p.allocated_types_)),
        globalNames_(p.globalNames_){
        }
        Program(const Program & p) = delete;

        const std::vector<std::pair<Symbol, std::unique_ptr<Function>>> & functions() const {
            return functions_;
        }
        const BasicBlock * globals() const {
            return globals_.get();
        }
        const std::map< Instruction*,std::string> & stringLiterals() const{
            return stringLiterals_;
        }
        const Instruction * getGlobalVariableAddress(const Symbol & name)const {
            auto i = globalNames_.find(name);
            if (i != globalNames_.end())
                return i->second;
            else
                return nullptr;
        }

        void print(tiny::ASTPrettyPrinter & p) const {
            globals_->print(p);
            for (auto & i : functions_)
                i.second->print(p);
        }
    protected:
        friend class ILVisitor;
        friend class ILBuilder;
        template<class T>
        friend class CfgBuilder;
        inline  void accept(ILVisitor * v) override{ v->visit(this); };
    private:
        std::map< Instruction*,std::string> stringLiterals_;
        std::vector<std::pair<Symbol, std::unique_ptr<Function>>> functions_;
        std::unique_ptr<BasicBlock> globals_;
        std::vector<std::unique_ptr<tvlm::Type>> allocated_types_;
        std::unordered_map<Symbol, Instruction*> globalNames_;
    } ; // tvlm::Program


//    inline void Instruction::Terminator0::accept(ILVisitor * v) { v->visit(this); }
//    inline void Instruction::Terminator1::accept(ILVisitor * v) { v->visit(this); }
//    inline void Instruction::Terminator2::accept(ILVisitor * v) { v->visit(this); }
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



} // namespace tvlm
