#pragma once

#include <unordered_set>
#include <utility>
#include "tvlm/tvlm/il/il.h"
#include "tvlm/tvlm/il/il_builder.h"
#include "t86/program/programbuilder.h"
#include "tvlm/tvlm/codeGeneration/InstructionSelection/NaiveIS.h"



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

//    class DummyRule : public Rule{
//    public:
//        DummyRule():Rule(0){}
//        bool matchesOpcode(const Opcode & ins) const override{
//            return true;
//        }
//        void operator=(const DummyRule &) = delete;
//
//        Rule * makeCopy() const override {
//            return new DummyRule(*this);
//        }
//        bool operator== (const DAG * other)  override;
//        int countCost () override;
//    protected:
//        DummyRule(const DummyRule & other) :Rule(other.cost_, other.num_), dag_(other.dag_){
//        };
//        DAG * dag_;
//    };


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


    class DAGNode {
    public:
        DAGNode(const Instruction * ins )
                :instruction_(ins)
                ,children_()
                ,parent_()
                ,common_ILInstructions_()
                ,assigned_code_()
        {}

        bool sameInstr(const Instruction * ins) const {
            return (*instruction_) == ins;
        }

        const Instruction * ins()const {
            return instruction_;
        }
        std::vector<DAGNode*> children()const {
            return children_;
        }
        std::vector<DAGNode*> parents()const {
            return parent_;
        }
        std::vector<tiny::t86::Instruction *> moveCode() {

            return std::move(assigned_code_);
        }
        std::vector<const Instruction *> common()const {
            return common_ILInstructions_;
        }
        void addChildren(DAGNode * child){
            children_.push_back(child);
        }
        void addParent(DAGNode * parent){
            parent_.push_back(parent);
        }
        void addCommon(const Instruction * common){
            common_ILInstructions_.push_back(common);
        }
        void addCode(std::vector<tiny::t86::Instruction *> && code){
            for (auto * i : assigned_code_) {
                delete i;
            }
            assigned_code_.clear();
            assigned_code_ = std::move(code);
        }
    private:
        const Instruction * instruction_;
        std::vector<DAGNode*> children_;
        std::vector<DAGNode*> parent_;


        std::vector<tiny::t86::Instruction *> assigned_code_;
        std::vector<const Instruction *> common_ILInstructions_;

    };

    class ConvertToDAGNode : public ILVisitor{
    public:
        ~ConvertToDAGNode() override = default;

        void visit(Instruction *ins) override;
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
        void visit(StructAssign *ins) override;

        void visit(BasicBlock *bb) override;
        void visit(Function *fce) override;
        void visit(Program *p) override;
    private:
        std::map<Instruction* , DAGNode * > seen_;
//        std::unordered_map<Function *, DAGNode * > fncDag_;
        std::unordered_map<BasicBlock *, DAGNode *> bbDag_;
        DAGNode * last_dag_;
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
            //        void visit(Instruction::Terminator2 * ins) override;
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
        void visit(StructAssign *ins) override;

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
        static int MemoryCellSize;

        static  void tests();

        using IL = tvlm::Program;
        using TargetProgram = tiny::t86::Program;
        TargetProgram compileToTarget( IL && il);
    };
} // namespace tvlm

