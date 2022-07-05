#pragma once

#include <map>
#include <queue>
#include "tvlm/tvlm/il/il_builder.h"
#include "tvlm/tvlm/il/il_insns.h"
#include "instruction_analysis.h"
#include "cfg.h"
#include "ILUsageVisitor.h"

namespace tvlm{
    using Instruction = ::tvlm::Instruction;

    template<class T>
    using LiveVars = MAP<const CfgNode<T> *, std::unordered_set<Declaration*>>;
    ;

    template<class Info>
    class LivenessAnalysis : public BackwardAnalysis<LiveVars<Info>, Info>{
    //    using Declaration = tvlm::Instruction;
        using NodeState = std::unordered_set<Declaration*>;
    //    using Declarations = MAP< ILInstruction *, Declaration*>;

        NodeState join(const CfgNode<Info> * node, LiveVars<Info> & state);

        LivenessAnalysis( ProgramCfg<Info> && cfg, const Declarations & declarations);

        std::unordered_set<Declaration*> getSubtree(const CfgNode<Info> *pNode);

        NodeState transferFun(const CfgNode<Info> * node, const  NodeState & state );


        NodeState funOne(const CfgNode<Info> * node, LiveVars<Info> & state);
    public:
//        static LivenessAnalysis<Info> * create(Program * p);
        explicit LivenessAnalysis(Program * p);
        LiveVars<Info> analyze() override;
    private:
        NodeState allVars_;
        PowersetLattice<Declaration*> nodeLattice_;
        MapLattice<const CfgNode<Info> *, NodeState> lattice_;
        ProgramCfg<Info> cfg_;

    };
//************************************************************************************************************

    template<class I>
    LivenessAnalysis<I>::LivenessAnalysis(Program *p):
            LivenessAnalysis(BackwardAnalysis<LiveVars<I>,I>::getCfg(p), InstructionAnalysis(p).analyze())
    {
    }

    template<class I>
    std::unordered_set<Declaration*> LivenessAnalysis<I>::getSubtree(const CfgNode<I> *node) {
        ILUsageVisitor v;
        v.begin(node->il());
        return v.result();
    }

    template<class I>
    typename LivenessAnalysis<I>::NodeState
    LivenessAnalysis<I>::transferFun(const CfgNode<I> *node, const LivenessAnalysis::NodeState &state){
        if(dynamic_cast<const CfgFunExitNode<I> *>(node)){
            return nodeLattice_.bot();
        }else if(dynamic_cast<const CfgStmtNode<I> *>(node)){
            auto * stmtNode = dynamic_cast<const CfgStmtNode<I> *>(node);
            auto instr = dynamic_cast<ILInstruction *>(stmtNode->il());
            if(instr){ //TODO rules to add and remove from states


                if (dynamic_cast<AllocL *>(stmtNode->il())){
                    auto newState = state;
                    std::unordered_set<Declaration*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;
                }else if (dynamic_cast<Store *>(stmtNode->il())){
                    auto newState = state;
                    std::unordered_set<Declaration*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;
                }else if (dynamic_cast<Load *>(stmtNode->il())){
                    auto newState = state;
                    std::unordered_set<Declaration*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;
                }else if (dynamic_cast<LoadImm *>(stmtNode->il())){
                    auto newState = state;
                    std::unordered_set<Declaration*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;
                }else if (dynamic_cast<Return *>(stmtNode->il())){
                    auto ret  = dynamic_cast<Return *>(stmtNode->il());
                    auto newState = state;
                    newState.emplace(ret->returnValue());
                    newState.erase(node->il());
                    return newState;
                }else if (dynamic_cast<CondJump *>(stmtNode->il())){
                    auto newState = state;
                    std::unordered_set<Declaration*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;
                }else if (dynamic_cast<Jump *>(stmtNode->il())){
                    auto newState = state;
                    std::unordered_set<Declaration*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;
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


    template<typename Info>
    LivenessAnalysis<Info>::LivenessAnalysis(ProgramCfg<Info> &&cfg, const Declarations &declarations):
            allVars_([&](){
                std::unordered_set<Declaration*> tmp;
                for ( auto & n : cfg.nodes()){
                    tmp.emplace(n->il());
                }
                return tmp;
            }())
            ,
            nodeLattice_(PowersetLattice<Declaration*>(allVars_)),
            lattice_(MapLattice<const CfgNode<Info>*, std::unordered_set<Declaration*>>(cfg.nodes(), &nodeLattice_)),
    cfg_(std::move(cfg)){

    }

    template<class Info>
    LiveVars<Info> LivenessAnalysis<Info>::analyze() {
        LiveVars<Info> X = lattice_.bot();
        std::unordered_set<const CfgNode<Info>*> W;
        for( auto & n : cfg_.nodes()){
            W.emplace(n);
        }

        while (!W.empty()) {
            const CfgNode<Info> * n = *W.begin();
            W.erase(W.begin());
            auto x = X.access(n);
            auto y = funOne(n, X);

            if (y != x) {
                X.insert(std::make_pair(n, y));//X += n -> y
                //W ++= n.pred;
                W.insert( n->pred_.begin(), n->pred_.end());
            } else if (y.empty()) {
                X.insert(std::make_pair(n, y));//X += n -> y;
            }
        }

        return X;
    }

    template<class I>
    typename LivenessAnalysis<I>::NodeState
    LivenessAnalysis<I>::funOne(const CfgNode<I> * node, LiveVars<I> & state)
    {
        return LivenessAnalysis<I>::transferFun(node, join(node, state));
    }

    template<class I>
    typename LivenessAnalysis<I>::NodeState
    LivenessAnalysis<I>::join(const CfgNode<I> * node, LiveVars<I> & state)
    {
        auto states = node->succ_;
        auto acc = nodeLattice_.bot();
        for (const CfgNode<I> * s : node->succ_) {
            auto pred = state.access(s);
            acc = nodeLattice_.lub(acc, pred);
        }
        return acc;
    }


} //namespace tvlm
