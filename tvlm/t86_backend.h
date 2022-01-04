#pragma once

#include <unordered_set>
#include <utility>
#include "il.h"
#include "t86/program/programbuilder.h"


namespace tvlm {

    using Opcode = tvlm::Instruction::Opcode;


    class DAG;


    class Rule {
    public:
        virtual bool matchesOpcode(Instruction * ins) const =0;
        int num() const{
            return num_;
        }
        Rule(int cost, int num = counter++) : cost_(cost), num_(num){ }

        virtual Rule * makeCopy() const = 0;
        virtual bool operator== (const DAG * other) const = 0;
        virtual const Rule * operator[] (size_t idx) const = 0;
    protected:
        int cost_;
        int num_;
        static int counter;

        std::vector<tiny::t86::Instruction *> code_;
    };

    class DummyRule : public Rule{
    public:
        virtual bool matchesOpcode(Instruction * ins) const {
            return false;
        }
        void operator=(const DummyRule &) = delete;

        virtual Rule * makeCopy() const {
            return new DummyRule(*this);
        }
        static DummyRule *get();
        virtual bool operator== (const DAG * other) const override;

        virtual const Rule * operator[] (size_t idx) const override{
            return nullptr;
        }


    protected:
        DummyRule(const DummyRule & other) = default;
        DummyRule():Rule(0){}

        virtual bool matchesOpcode(Instruction::Opcode ins) const {
            return false;
        }

    private:
        static DummyRule * dummy;
    };



    class DAG {
    public:
        friend class SpecializedRule;

        virtual bool matchesOpcode(Opcode opcode) const {
            return src_->opcode_  == opcode;
        }


        std::set<std::unique_ptr<Rule>> labels_;
        Rule * selectedRule_;


        explicit DAG( Instruction *src, Instruction::Opcode type, const std::vector<DAG*> & children = std::vector<DAG*>())
        :src_( src), children_(children), type_(type){
        }

        virtual void tile();
    private:
        Opcode type_;
        std::vector<DAG*> children_;

    protected:
        Instruction * src_;
        std::vector<tiny::t86::Instruction *> code_;
    };

    class SpecializedRule : public Rule{
    public:
        // recursive comparison
        explicit SpecializedRule( Instruction::Opcode opcode, int cost, std::vector<const Rule*>  children = std::vector<const Rule*>(), std::function<bool(const Rule * rule, const DAG * dag)> customMatcher = nullptr)
                :Rule( cost), children_(std::move(children)), opcode_(opcode), customMatcher_(std::move(customMatcher)){
        }
        explicit SpecializedRule(Instruction::Opcode opcode, int cost, int size, std::function<bool(const Rule * rule, const DAG * dag)>  customMatcher = nullptr)
                :Rule( cost), children_(std::move(std::vector<const Rule*> (size, DummyRule::get())) ), customMatcher_(std::move(customMatcher)),
                 opcode_(opcode){

        }
        SpecializedRule(const SpecializedRule & r) :Rule(r.cost_, r.num_),
        children_(r.children_), opcode_(r.opcode_) {

        }
        virtual bool matchesOpcode(Instruction * ins) const {
            return ins->opcode_ == opcode_;
        }

        virtual Rule * makeCopy() const{
            return new SpecializedRule(*this);
        }

        bool matchRuleToDAG(const DAG * dag)const ;
        bool operator== (const DAG * other) const override;
        virtual const Rule * operator[] (size_t idx) const override{
            return children_[idx];
        }
        const std::vector<const Rule*> children_;
    private:
        Instruction::Opcode opcode_;
        std::function<bool (const Rule *, const tvlm::DAG* ) > customMatcher_;
    };


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

        static tiny::t86::Program translate(tvlm::Program & prog){
            //TODO
            ILTiler v;
            v.visit( &prog);
            std::cerr << "huh" << std::endl;
            v.functionTable_.begin()->second->tile();

            auto & globLabels = v.functionTable_.begin()->second->labels_;

            std::cerr << "huh" << globLabels.size() << std::endl;
            tiny::t86::ProgramBuilder pb;
            return std::move(pb.program());
        }
        static  std::unordered_map<Opcode, std::set<std::unique_ptr<tvlm::Rule>>> allRules_ ;


    protected:
        ILTiler(): lastIns_(nullptr){

        }

        void visit(Instruction * ins) override {};
        void visit(Instruction::Terminator0 * ins) override;
        void visit(Instruction::Terminator1 * ins) override;
        void visit(Instruction::TerminatorN * ins) override;
        void visit(Instruction::Returnator * ins) override;
        void visit(Instruction::DirectCallInstruction * ins) override;
        void visit(Instruction::IndirectCallInstruction * ins) override;
        void visit(Instruction::SrcInstruction * ins) override;
        void visit(Instruction::BinaryOperator * ins) override;
        void visit(Instruction::UnaryOperator * ins) override;
        void visit(Instruction::ImmIndex * ins) override;
        void visit(Instruction::ImmSize * ins) override;
        void visit(Instruction::ImmValue * ins) override;
        void visit(Instruction::VoidInstruction * ins) override;
        void visit(Instruction::LoadAddress * ins) override;
        void visit(Instruction::StoreAddress * ins) override;
        void visit(Instruction::PhiInstruction * ins) override;
        void visit(Instruction::ElemInstruction * ins) override;

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
            return it->second;
        }


        DAG * lastIns_;
        std::unordered_map<tiny::Symbol, DAG*> functionTable_;
        std::unordered_map<Instruction *, DAG*> compiled_;
        DAG *  globals_;

//        static std::unordered_map<Opcode , std::set<std::unique_ptr<Rule>>> allRules_ ;
        static std::unordered_map<Opcode, std::set<std::unique_ptr<tvlm::Rule>>> AllRulesInit();

        std::unordered_set<Rule*> lastTiled_;
        std::unordered_map<Instruction*, std::unordered_set<Rule*>> tiled_;




    };

    class ILtoISNaive : public ILVisitor{
    public:
        using Label = tiny::t86::Label;
        using DataLabel = tiny::t86::DataLabel;

        static tiny::t86::Program translate(tvlm::Program & prog){
            //TODO
            ILtoISNaive v;
            v.visit( &prog);
            tiny::t86::Program rawProg = v.pb_.program();
            std::vector<tiny::t86::Instruction*> instrs = rawProg.moveInstructions();
            int line = 0;
            for(const tiny::t86::Instruction * i : instrs){
                std::cerr << tiny::color::blue << line++ << ": " << tiny::color::green << i->toString() << std::endl;
            }


            return {instrs, rawProg.data()};
//            return v.pb_.program();
        }

    protected:
        ILtoISNaive(): pb_(tiny::t86::ProgramBuilder()), lastIns_(Label::empty()){

        }
        void visit(Instruction * ins) override {};
        void visit(Instruction::Terminator0 * ins) override;
        void visit(Instruction::Terminator1 * ins) override;
        void visit(Instruction::TerminatorN * ins) override;
        void visit(Instruction::Returnator * ins) override;
        void visit(Instruction::DirectCallInstruction * ins) override;
        void visit(Instruction::IndirectCallInstruction * ins) override;
        void visit(Instruction::SrcInstruction * ins) override;
        void visit(Instruction::BinaryOperator * ins) override;
        void visit(Instruction::UnaryOperator * ins) override;
        void visit(Instruction::ImmIndex * ins) override;
        void visit(Instruction::ImmSize * ins) override;
        void visit(Instruction::ImmValue * ins) override;
        void visit(Instruction::VoidInstruction * ins) override;
        void visit(Instruction::LoadAddress * ins) override;
        void visit(Instruction::StoreAddress * ins) override;
        void visit(Instruction::PhiInstruction * ins) override;
        void visit(Instruction::ElemInstruction * ins) override;

        void visit(BasicBlock * bb) override;
        void visit(Function * fce) override;
        void visit(Program * p) override;

    private:
        Label visitChild(IL * il) {
            ILVisitor::visitChild(il);
            return lastIns_;
        }

        template<typename T>
        Label visitChild(std::unique_ptr<T> const &ptr) {
            return visitChild(ptr.get());
        }

        template<typename T>
        Label add(Instruction * ins, const T& instruction) {
            lastIns_ = pb_.add( instruction);
            if(ins != nullptr ) compiled_.emplace(ins, lastIns_);
            return lastIns_;
        }

        Label find(Instruction * ins){
            auto it = compiled_.find(ins);
            if(it == compiled_.end()){
                return Label::empty();
            }
            return it->second;
        }

        tiny::t86::ProgramBuilder pb_;
        Label lastIns_;
        std::unordered_map<tiny::Symbol, Label> functionTable_;
        std::unordered_map<Instruction *, Label> compiled_;


    };






    class t86_Backend{
    public:
        using IL = tvlm::Program;
        using PB = tiny::t86::Program;
        PB compileToTarget( IL && il){
            return tvlm::ILTiler::translate(il);

        }
    };

} // namespace tvlm

