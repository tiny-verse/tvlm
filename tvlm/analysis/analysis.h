#pragma once
#include "cfg.h"
#include "tvlm/tvlm/codeGeneration/ProgramBuilder.h"

namespace tvlm {



    template<class T, class I = DummyClass>
    class Analysis : public TargetProgramFriend{
    protected:
        Analysis():cfg_mapping_(), instr_mapping_(){}
        std::map<const CfgNode<I>*, const Instruction *>cfg_mapping_;
        std::map< const Instruction *, const CfgNode<I>*>instr_mapping_;


    public:
        std::map<const CfgNode<I>*, const Instruction *>& cfg_mapping(){
            return cfg_mapping_;
        };
        std::map< const Instruction *, const CfgNode<I>*> & instr_mapping(){
            return instr_mapping_;
        };
        virtual T  analyze() = 0;
        virtual ~Analysis() = default;

        virtual const std::vector<tvlm::CfgNode<I> *> & next(const CfgNode<I> * n)const{
            return n->succ_;
        };
    };

    template<typename T, typename I>
    class BackwardAnalysis : public Analysis<T, I> {
    protected:
        BackwardAnalysis():Analysis<T, I>(){}
        virtual ProgramCfg<I> * getCfg(Program * p){
            auto builder = std::make_unique<IntraProceduralCfgBuilder<I>>(&this->cfg_mapping_, &this->instr_mapping_);
            return builder->fromProgram(p);
        }
    public:
        virtual ~BackwardAnalysis() = default;
//        virtual T analyze() = 0;
        const std::vector<tvlm::CfgNode<I> *> & next(const CfgNode<I> *n)const override{
            return n->succ_;
        };
    };

    template<typename T, typename I>
    class ForwardAnalysis : public Analysis<T, I> {
    public:
        virtual ~ForwardAnalysis() = default;
//        virtual T analyze() = 0;
        const std::vector<tvlm::CfgNode<I> *> & next(const CfgNode<I> *n)const override {
            return n->pred_;
        };
    protected:
        ForwardAnalysis():Analysis<T, I>(){}
         virtual ProgramCfg<I> * getCfg(Program * p){
            auto builder = std::make_unique<IntraProceduralCfgBuilder<I>>(&this->cfg_mapping_, &this->instr_mapping_);
            return builder->fromProgram(p);
        }
    };
}
