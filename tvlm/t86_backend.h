#pragma once

#include <unordered_set>
#include "il.h"
#include "t86/program/programbuilder.h"


namespace tvlm {



    class DAG;
    class Rule {
    public:
        virtual bool compareTo(Instruction * ins) const {
            return false;
        }
        Rule(int cost) : cost_(cost), num_(counter++){ }
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
        DummyRule(DummyRule & other) = delete;

        void operator=(const DummyRule &) = delete;

        static DummyRule *get();


    protected:
        DummyRule():Rule(0){}

        virtual bool operator== (const DAG * other) const override;
        virtual const Rule * operator[] (size_t idx) const override{
            return nullptr;
        }

    private:
        static DummyRule * dummy;
    };

    template<typename T>
    class SpecializedRule : public Rule{
    public:

        // recursive comparison
        explicit SpecializedRule(  int cost, const std::vector<const Rule*> & children = std::vector<const Rule*>())
                :Rule( cost), children_(children){
        }
        explicit SpecializedRule( int cost, int size)
                :Rule( cost), children_(std::move(std::vector<const Rule*> (size, DummyRule::get())) ){

        }
        virtual bool compareTo(Instruction * ins) const {
            return dynamic_cast<T*>(ins);
        }
        bool operator== (const DAG * other) const override;
        virtual const Rule * operator[] (size_t idx) const override{
            return children_[idx];
        }
        const std::vector<const Rule*> & children_;
    private:
    };



    class DAG {
    public:
        virtual bool compareTo(Instruction * ins) const {
            return false;
        }
        DAG(Instruction * src) :  src_(src){}

        virtual std::unordered_set<const Rule*> tile(const std::unordered_map<Instruction *, std::unordered_set<const Rule*>> & allRules) const = 0;

        virtual bool operator==(const Rule * other) const = 0;

    protected:
        Instruction * src_;
        std::vector<tiny::t86::Instruction *> code_;
    };

    template<typename T>
    class SpecializedDAG : public DAG{
    public:

        // recursive comparison
        explicit SpecializedDAG( T *src, const std::vector<DAG*> & children = std::vector<DAG*>())
        :DAG( src), children_(children){
        }
        bool operator== (const Rule * other) const override{ //TODO maybe rename matches
            auto ot = dynamic_cast<const SpecializedRule<T>*>(other);
            bool ret = ot;
            auto it = children_.begin(), otit = ot->children_.begin();
            if(children_.size() != ot->children_.size()) return false; // different arity
            while(ret && it !=children_.end()){
                ret = (**it == *otit || dynamic_cast<const DummyRule*>(*otit) );// compare recursively or ignore subtree simulated for REG
                it++;otit++;
            }
            if(it == children_.end()) return true; // all compared

            return ret;
        }

        virtual std::unordered_set<const Rule*> tile(const std::unordered_map<Instruction *, std::unordered_set<const Rule*>> & allRules) const override {
            //labels empty
            std::unordered_set<const Rule *> labels;
            std::vector<std::unordered_set<const Rule*>> tiledChildren;
            tiledChildren.resize(children_.size());
            for (int i = 0; i < children_.size(); i++) {
                tiledChildren[i] = children_[i]->tile(allRules);
            }


//            auto it = allRules.find(src_); // BAD
            auto it = allRules.begin(); //mathces type
            for(; it != allRules.end();it++ ){
                if( dynamic_cast<const T*>(it->first)){
                    break;
                }
            }
            if(it == allRules.end()) {
                std::cerr << "no rules matches this instruction! ch:"<< children_.size() << "src: " << src_->name() << std::endl;
                return labels;
            }
            for (auto & r : it->second) { //all that  matches operation
                bool ret = true;
                for(int i = 0; i < tiledChildren.size();i++){
                    auto & chLabels = tiledChildren[i];
                    const Rule *  tmp = (*r)[i];
                    auto chIt = chLabels.find(tmp); // find rule in
                    if(chIt == chLabels.end()){
                        ret = false; break; //does not satisfy childs label
                    }
                }
                if(ret){ // all satisfied
                    labels.emplace(r);
                }
            }


            return labels;
        }
    private:
         std::vector<DAG*> children_;
//        std::unordered_set<Rule*> labels_;
    };


    template<typename T>
    bool SpecializedRule<T>::operator==(const DAG *other) const {
        return *other == this;

    }

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
//        DAG(Instruction * ins, DAG * next = nullptr) :ins_(ins), next_(next){}
//        Instruction * ins_;
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

            std::unordered_set<const Rule *> globLabels = v.globals_->tile(allRules_);

            std::cerr << "huh" << globLabels.size() << std::endl;
            tiny::t86::ProgramBuilder pb;
            return pb.program();
        }

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

        static std::unordered_map<Instruction *, std::unordered_set<const Rule*>> allRules_ ;
        static std::unordered_map<Instruction *, std::unordered_set<const Rule*>> AllRulesInit();
        static std::unordered_map<Instruction *, std::unordered_set<const Rule*>> allRules() {
            if(allRules_.empty()){
                allRules_ = AllRulesInit();
            }
            return allRules_;
        }


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

