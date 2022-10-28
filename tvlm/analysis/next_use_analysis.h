#pragma once

#include <map>
#include <queue>
#include "tvlm/tvlm/il/il_builder.h"
#include "tvlm/tvlm/il/il_insns.h"
#include "instruction_analysis.h"
#include "lattice/lattice.h"
#include "lattice/flat_lattice.h"

#include "cfg.h"
#include "tvlm/analysis/lattice/next_use_lattice.h"
#include "tvlm/analysis/DeclarationAnalysis.h"

namespace tvlm{
    using Instruction = ::tvlm::Instruction;

    template<class I>
    using NextUse = MAP<const CfgNode<I>* ,  MAP<Declaration*, FlatElem<const Declaration*>*>>;

    template<class I>
    class NextUseAnalysis : public BackwardAnalysis<NextUse<I>, I>{
    //    using Declaration = tvlm::Instruction*;
        using NodeState =  MAP<Declaration*,FlatElem<const Declaration*>*>;
    //    using NodeState = MAP<const CfgNode * , Declaration*>;
    //    using Declarations = MAP< ILInstruction *, Declaration*>;

        NodeState join(const CfgNode<I> * node, NextUse<I> & state);

        NextUseAnalysis( ProgramCfg<I> * cfg, const Declarations & declarations);

        std::unordered_set<Declaration*> getSubtree(const CfgNode<I> *pNode);

        NodeState transferFun(const CfgNode<I> * node, const  NodeState & state );


        NodeState funOne(const CfgNode<I> * node, NextUse<I> & state);
    public:
        virtual ~NextUseAnalysis() {
            delete cfg_;
        }
        static NextUseAnalysis<I> * create(Program * p);
        NextUseAnalysis(Program * p);
        NextUse<I> analyze() override;
    private:
        std::unordered_set<Declaration *> allVars_;
        FlatLattice<const Declaration*>sublattice;
    //    MapLattice<Declaration , FlatElem<const Declaration>*> nodeLattice_;
        NextUseLattice nodeLattice_;
        MapLattice<const CfgNode<I> *, NodeState> lattice_;
        ProgramCfg<I> * cfg_;
    //    std::unordered_set<Declaration> allVars_;
    //    MapLattice<const CfgNode*, Declaration> nodeLattice_;
    //    MapLattice<const CfgNode *, NodeState> lattice_;
    //    ProgramCfg cfg_;
    //    std::vector<std::unique_ptr<FlatVal<const Declaration >>> createdFlatVals_;

        FlatVal<const Declaration*> * makeFlatVal(const Declaration * val , NodeState & nodeState);
    };

    //***************************************************************************************


    template<class I>
    NextUseAnalysis<I> * NextUseAnalysis<I>::create(Program *p){
        auto analysis = InstructionAnalysis(p);
        Declarations decls = analysis.analyze();

        return new NextUseAnalysis(BackwardAnalysis<NextUse<I>, I>::getCfg(p), decls);
    }
//    template<class I>
//    NextUseAnalysis<I>::NextUseAnalysis(Program *p)

    template<class I>
    std::unordered_set<Declaration*> NextUseAnalysis<I>::getSubtree(const CfgNode<I> *node) {
        DeclarationAnalysis v;
        v.begin(node->il());
        return v.result();
    }

    template<class I>
    typename NextUseAnalysis<I>::NodeState
    NextUseAnalysis<I>::transferFun(const CfgNode<I> *node, const NextUseAnalysis<I>::NodeState &state){
        if(dynamic_cast<const CfgFunExitNode<I> *>(node)){
            return nodeLattice_.bot();
        }else if(dynamic_cast<const CfgStmtNode<I> *>(node)){
            auto * stmtNode = dynamic_cast<const CfgStmtNode<I> *>(node);
            auto instr = dynamic_cast<ILInstruction *>(stmtNode->il());
            if(instr){ //TODO all instructions


                if (dynamic_cast<AllocL *>(stmtNode->il())){
                    //                auto alloc  = dynamic_cast<::AllocL *>(stmtNode->il());
                    //                auto newState = state;
                    //                if(alloc)
                    //                std::unordered_set<Declaration*> children = getSubtree(node);
                    //                newState.insert(children.begin(), children.end());
                    //                newState.erase(node->il());
                    //                return newState;
                }else if (dynamic_cast<Store *>(stmtNode->il())){
                    auto store  = dynamic_cast<Store *>(stmtNode->il());
                    auto newState = state;

                    newState[store->value()] = makeFlatVal(stmtNode->il(), newState);
                    newState[store->address()] = makeFlatVal(stmtNode->il(), newState);
                    return newState;
                }else if (dynamic_cast<Load *>(stmtNode->il())){
                    auto load  = dynamic_cast<Load *>(stmtNode->il());
                    auto newState = state;
                    newState[load->address()] = makeFlatVal(stmtNode->il(), newState);
                    return newState;


                }else if (dynamic_cast<LoadImm *>(stmtNode->il())){
                    return state;
                }else if (dynamic_cast<Return *>(stmtNode->il())){
                    auto ret  = dynamic_cast<Return *>(stmtNode->il());
                    auto newState = state;

                    newState[ret->returnValue()] = makeFlatVal(stmtNode->il(), newState);
                    return newState;
                }else if (dynamic_cast<CondJump *>(stmtNode->il())){
                }else if (dynamic_cast<Jump *>(stmtNode->il())){
                }else {
                    return state;
                }
            }else if (dynamic_cast<const CfgFunExitNode<I> *>(node)){

                return nodeLattice_.bot();
            }


        } else{
        }
        return state;
    }

    template<class I>
    typename NextUseAnalysis<I>::NodeState
    NextUseAnalysis<I>::funOne(const CfgNode<I> * node, NextUse<I> & state)
    {
        return NextUseAnalysis<I>::transferFun(node, join(node, state));
    }
    template<class I>
    typename NextUseAnalysis<I>::NodeState
    NextUseAnalysis<I>::join(const CfgNode<I> * node, NextUse<I> & state)
    {
        auto states = node->succ_;
        auto acc = nodeLattice_.bot();
        for (const CfgNode<I> * s : node->succ_) {
            auto & pred = state.access(s);
            acc = nodeLattice_.lub(acc, pred);
        }
        return acc;
    }

    template<class I>
    FlatVal<const Declaration *> *
    NextUseAnalysis<I>::makeFlatVal(const Declaration *val, NextUseAnalysis::NodeState &nodeState) {
        auto tmp = new FlatVal<const Declaration*> (val);
        nodeState.reg(tmp);

        //        createdFlatVals_.emplace_back(tmp);
        return tmp;
    }

    template<class I>
    NextUse<I> NextUseAnalysis<I>::analyze() {
        NextUse<I> X = lattice_.bot();
        std::unordered_set<const CfgNode<I>*> W;
        for( auto & n : cfg_->nodes()){
            W.emplace(n);
        }

        while (!W.empty()) {
            const CfgNode<I>* n = *W.begin();
            W.erase(W.begin());
            auto x = X.access(n);
            auto y = funOne(n, X);

            if (y != x) {
                X.insert(std::make_pair(n, y));//X += n -> y
                //W ++= n.pred;
                W.insert( n->pred_.begin(), n->pred_.end());
            }
            //            else if (y()) {
            //                X.insert(std::make_pair(n, y));//X += n -> y;
            //            }
        }

        return X;
    }

    template<class I>
    NextUseAnalysis<I>::NextUseAnalysis(Program *p) :
            NextUseAnalysis(BackwardAnalysis<NextUse<I>, I>::getCfg(p),
                            InstructionAnalysis(p).analyze())
    {
//        auto analysis = InstructionAnalysis(p);
//        Declarations decls = analysis.analyze();
//
//        return new NextUseAnalysis(BackwardAnalysis<NextUse<I>, I>::getCfg(p), decls);
    }

    template<class I>
    NextUseAnalysis<I>::NextUseAnalysis(ProgramCfg<I> *cfg, const Declarations &declarations) :
            allVars_([&](){
                std::unordered_set<Declaration*> tmp;
                for ( auto & n : cfg->nodes()){
                    tmp.emplace(n->il());
                }
                return tmp;
            }())
            ,
            sublattice(FlatLattice<const Declaration *>()),
            nodeLattice_(NextUseLattice(allVars_, &sublattice )),
            lattice_(MapLattice<const CfgNode<I>*, NodeState>(cfg->nodes(), &nodeLattice_)),
    cfg_(cfg){

    }


} //namespace tvlm
