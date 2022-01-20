#pragma once

#include <unordered_set>
#include <utility>
#include "il.h"
#include "t86/program/programbuilder.h"
#include "NaiveIS.h"



namespace tvlm {

    using Opcode = tvlm::Instruction::Opcode;


    class DAG;


    class Rule {
    public:
//        virtual bool matchesOpcode(Instruction * ins) const {
//            return this->matchesOpcode(ins->opcode_);
//        };
        virtual bool matchesOpcode(const Opcode & opcode) const =0;
        int num() const{
            return num_;
        }
        Rule(int cost) : Rule(cost, counter++){ }

        virtual Rule * makeCopy() const = 0;
        virtual bool operator== (const DAG * other)  = 0;
        virtual int countCost() = 0;
//        virtual const Rule * operator[] (size_t idx) const = 0;
    protected:
        Rule(int cost, int num ) : cost_(cost), num_(num), code_(){ }
        int cost_;
        int num_;
        static int counter;

        std::vector<tiny::t86::Instruction *> code_;
    };

    class DummyRule : public Rule{
    public:
        DummyRule():Rule(0){}
        bool matchesOpcode(const Opcode & ins) const override{
            return true;
        }
        void operator=(const DummyRule &) = delete;

        Rule * makeCopy() const override {
            return new DummyRule(*this);
        }
        bool operator== (const DAG * other)  override;
        int countCost () override;
    protected:
        DummyRule(const DummyRule & other) :Rule(other.cost_, other.num_), dag_(other.dag_){
        };
        DAG * dag_;
    };

    class VirtualRegisterPlaceholder{
        virtual VirtualRegisterPlaceholder * makeCopy() = 0;
    };

    class VirtualRegister : public VirtualRegisterPlaceholder{
    public:
            VirtualRegister(): reg_(tiny::t86::Register(-1)), set_(false){}
    private:
        VirtualRegisterPlaceholder *makeCopy() override {
            return new VirtualRegister(*this);
        }

    public:
        void setRegister(const tiny::t86::Register & reg){
            reg_ = reg;
            set_ = true;
        }
    private:
        tiny::t86::Register reg_;
        bool set_;
    };

    class VirtualFloatRegister : public VirtualRegisterPlaceholder{
    public:
        VirtualFloatRegister(): reg_(tiny::t86::FloatRegister(-1)), set_(false){}
        void setRegister(const tiny::t86::FloatRegister & reg){
            reg_ = reg;
            set_ = true;
        }

    private:
        VirtualRegisterPlaceholder *makeCopy() override {
            return new VirtualFloatRegister(*this);
        }

        tiny::t86::FloatRegister reg_;
        bool set_;
    };


    class DAG {
    public:
        friend class SpecializedRule;

        virtual bool matchesOpcode(Opcode opcode) const {
            return src_->opcode_  == opcode;
        }


        std::set<std::unique_ptr<Rule>> labels_;
        Rule * selectedRule_;


        explicit DAG( Instruction *src, const Instruction::Opcode & type, const std::vector<DAG*> & children = std::vector<DAG*>())
        :src_( src), children_(children), type_(type), reg(tiny::t86::Register(0)){
//            assert(src->opcode_ == type);
        }

        virtual void tile();

        const Instruction * ins()const{
            return src_;
        }
        tiny::t86::Register reg;

        DAG * operator[](size_t idx)const{
            return children_[idx];
        }

        bool operator==(const DAG & other)const{
            bool tmp = children_.size() == other.children_.size();
            auto it = children_.begin();
            auto ito = other.children_.begin();
            for ( ;it != children_.end();it++, ito++) {
                if(!tmp || !( tmp = (*it == *ito) ) )return false;
            }
            return true;
        }

    private:
        Opcode type_;
        std::vector<DAG*> children_;
        std::vector<Rule*> ruled_;

    protected:
        Instruction * src_;
        std::vector<tiny::t86::Instruction *> code_;
    };
//
//    class SpecializedRule : public Rule{
//    public:
//        // recursive comparison
//        explicit SpecializedRule( Instruction::Opcode opcode, int cost, std::vector<std::shared_ptr<Rule>>  children = std::vector<std::shared_ptr<Rule>>(), std::function<bool(const Rule * rule, const DAG * dag)> customMatcher = nullptr)
//                :Rule( cost), children_(std::move(children)), opcode_(opcode), customMatcher_(std::move(customMatcher)){
//        }
//        explicit SpecializedRule(Instruction::Opcode opcode, int cost, int size, std::function<bool(const Rule * rule, const DAG * dag)>  customMatcher = nullptr)
//                :Rule( cost), children_(std::move(std::vector<std::shared_ptr<Rule>> (size, std::shared_ptr<Rule>(DummyRule::getnew()))) ), customMatcher_(std::move(customMatcher)),
//                 opcode_(opcode){
//
//        }
//        SpecializedRule(const SpecializedRule & r) :Rule(r.cost_, r.num_),
//        children_(r.children_), opcode_(r.opcode_) {
//
//        }
//        virtual bool matchesOpcode(Instruction * ins) const {
//            return ins->opcode_ == opcode_;
//        }
//        virtual bool matchesOpcode(const Opcode & inscode) const {
//            return inscode == opcode_;
//        }
//
//        virtual Rule * makeCopy() const{
//            return new SpecializedRule(*this);
//        }
//
//        bool matchRuleToDAG(const DAG * dag)const ;
//        bool operator== (const DAG * other) const override;
//        virtual const Rule * operator[] (size_t idx) const {
//            return children_[idx].get();
//        }
//        const std::vector<std::shared_ptr<Rule>> children_;
//    private:
//        Instruction::Opcode opcode_;
//        std::function<bool (const Rule *, const tvlm::DAG* ) > customMatcher_;
//    };


            //
            //    template<typename T>
            //    class BinaryDAG : public DAG{
            //    public:
            //
            //        // recursive comparison
            //        BinaryDAG(int cost, T *src, DAG * lhs, DAG * rhs ):DAG(cost, src), lhs_(lhs), rhs_(rhs){
            //        }
            //        bool operator== (const DAG * other) const{
            //            auto ot = dynamic_cast<BinaryDAG<T>>(other);
            //            return ot && ot.lhs_ == this->operand_ && ot.rhs_;
            //        }
            //    private:
            //        DAG* lhs_;
            //        DAG* rhs_;
            //    };
            //
            //    template<typename T>
            //    class UnaryDAG : public DAG{
            //    public:
            //        UnaryDAG(int cost, T *src, DAG * operand ):DAG(cost, src), operand_(operand){
            //        }
            //
            //        // recursive comparison
            //        bool operator== (const DAG * other) const{
            //            auto ot = dynamic_cast<UnaryDAG<T>>(other);
            //            return ot && ot.operand_ == this->operand_;
            //        }
            //    private:
            //        DAG* operand_;
            //    };
            //
            //    template<typename T>
            //    class LeafDAG : public DAG{
            //    public:
            //        LeafDAG(int cost, T *src):DAG(cost, src){
            //        }
            //
            //        // recursive comparison
            //        bool operator== (const DAG * other) const{
            //            return  dynamic_cast<UnaryDAG<T>>(other);
            //        }
            //
            //    private:
            //
            //    };

            //
            //    class DAG{
            //    public:
            //        DAG(Instruction * ins, DAG * next = nullptr) :il_(ins), next_(next){}
            //        Instruction * il_;
            //        DAG * next_;
            //    };
            //    class DAGBuilder : public ILVisitor {
            //    public:
            //    protected:
            //
            //        void visit(Instruction::Terminator0 * ins) override;
            //        void visit(Instruction::Terminator1 * ins) override;
            //        void visit(Instruction::TerminatorN * ins) override;
            //        void visit(Instruction::Returnator * ins) override;
            //        void visit(Instruction::DirectCallInstruction * ins) override;
            //        void visit(Instruction::IndirectCallInstruction * ins) override;
            //        void visit(Instruction::SrcInstruction * ins) override;
            //        void visit(Instruction::BinaryOperator * ins) override;
            //        void visit(Instruction::UnaryOperator * ins) override;
            //        void visit(Instruction::ImmIndex * ins) override;
            //        void visit(Instruction::ImmSize * ins) override;
            //        void visit(Instruction::ImmValue * ins) override;
            //        void visit(Instruction::VoidInstruction * ins) override;
            //        void visit(Instruction::LoadAddress * ins) override;
            //        void visit(Instruction::StoreAddress * ins) override;
            //        void visit(Instruction::PhiInstruction * ins) override;
            //
            //        void visit(BasicBlock * bb) override;
            //        void visit(Function * fce) override;
            //        void visit(Program * p) override;
            //
            //        std::unordered_map<Instruction *, DAG*> compiled_;
            //        DAG * lastIns_;
            //    private:
            //        DAG*  visitChild(IL * il) {
            //            ILVisitor::visitChild(il);
            //            return lastIns_;
            //        }
            //
            //        template<typename T>
            //        DAG* visitChild(std::unique_ptr<T> const &ptr) {
            //            return visitChild(ptr.get());
            //        }
            //
            //
            //        DAG* add(Instruction * ins) {
            //            lastIns_ = new DAG(ins, nullptr);
            //            if(ins != nullptr ) compiled_.emplace(ins, lastIns_);
            //            return lastIns_;
            //        }
            //
            //        DAG* find(Instruction * ins){
            //            auto it = compiled_.find(ins);
            //            if(it == compiled_.end()){
            //                return nullptr;
            //            }
            //            return it->second;
            //        }
            //    };


    class ILTiler : public ILVisitor{
    public:
        using Label = tiny::t86::Label;
        using DataLabel = tiny::t86::DataLabel;

        static tiny::t86::Program translate(tvlm::Program & prog);
        static std::map<Opcode , std::set<Rule*>> allRules_ ;


    protected:
        ILTiler(): lastIns_(nullptr){

        }

        void visit(Instruction * ins) override {};
    public:
        void visit(Jump *ins) override;
        void visit(CondJump *ins) override;
        void visit(Return *ins) override;
        void visit(CallStatic *ins) override;
        void visit(Call *ins) override;
        void visit(Copy *ins) override;
        void visit(Extend *ins) override;
        void visit(Truncate *ins) override;
        void visit(BinOp *ins) override;
        void visit(UnOp *ins) override;
        void visit(LoadImm *ins) override;
        void visit(AllocL *ins) override;
        void visit(AllocG *ins) override;
        void visit(ArgAddr *ins) override;
        void visit(PutChar *ins) override;
        void visit(GetChar *ins) override;
        void visit(Load *ins) override;
        void visit(Store *ins) override;
        void visit(Phi *ins) override;
        void visit(ElemAddrOffset *ins) override;
        void visit(ElemAddrIndex *ins) override;
        void visit(Halt *ins) override;

    protected:

        void visit(BasicBlock * bb) override;
        void visit(Function * fce) override;
        void visit(Program * p) override;



    private:
        DAG * visitChild(IL * il) {
            Instruction * instr = dynamic_cast<Instruction * >(il);
            Function * fnc = nullptr;
            DAG* ret;

            if(instr && (ret = find(instr))){
                return ret; //already compiled
            }
            ILVisitor::visitChild(il);
            if(instr) { compiled_.emplace(instr, lastIns_); }else {fnc = dynamic_cast<Function *>(il);}
            if(fnc) { functionTable_.emplace(fnc->name(), lastIns_); }//else {fnc = dynamic_cast<Function *>(il);}

            return lastIns_;
        }

        template<typename T>
        DAG * visitChild(std::unique_ptr<T> const &ptr) {
            return visitChild(ptr.get());
        }

        DAG* find(Instruction * ins){
            auto it = compiled_.find(ins);
            if(it == compiled_.end()){
                return nullptr;
            }
            return it->second.get();
        }


        DAG * lastIns_;
        std::unordered_map<tiny::Symbol, std::unique_ptr<DAG>> functionTable_;
        std::unordered_map<Instruction *, std::unique_ptr<DAG>> compiled_;
        std::unique_ptr<DAG> globals_;

//        static std::unordered_map<Opcode , std::set<Rule*>> allRules_ ;
        static std::vector<std::unique_ptr<Rule>> AllRulesInit();

        std::unordered_set<Rule*> lastTiled_;
        std::unordered_map<Instruction*, std::unordered_set<Rule*>> tiled_;




    };




    class t86_Backend{
    public:
        using IL = tvlm::Program;
        using PB = tiny::t86::Program;
        PB compileToTarget( IL && il){
            //auto codeGenerator = CodeGenerator (il);

//            return tvlm::ILTiler::translate(il);
//            return tvlm::NaiveIS::translate(il);
            tiny::t86::ProgramBuilder pb;
            return std::move(pb.program());
        }
    };

} // namespace tvlm

