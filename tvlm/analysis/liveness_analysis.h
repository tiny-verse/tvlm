#pragma once

#include <map>
#include <queue>
#include "tvlm/tvlm/il/il_builder.h"
#include "tvlm/tvlm/il/il_insns.h"
#include "instruction_analysis.h"
#include "cfg.h"
#include "DeclarationAnalysis.h"

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

        LivenessAnalysis( const Declarations & declarations, Program * program);

        std::unordered_set<Declaration*> getSubtree(const CfgNode<Info> *pNode);

        NodeState transferFun(const CfgNode<Info> * node, const  NodeState & state );


        NodeState funOne(const CfgNode<Info> * node, LiveVars<Info> & state);
    public:
//        static LivenessAnalysis<Info> * create(Program * p);
        virtual ~LivenessAnalysis(){
            delete cfg_;
        }
        explicit LivenessAnalysis(Program * p);
        LiveVars<Info> analyze() override;

        std::map<const CfgNode<Info>*, const Instruction *>instr_mapping(){
            return instr_mapping_;
        };
    private:
        std::map<const CfgNode<Info>*, const Instruction *>instr_mapping_;
        NodeState allVars_;
        PowersetLattice<Declaration*> nodeLattice_;
        MapLattice<const CfgNode<Info> *, NodeState> lattice_;
        ProgramCfg<Info> * cfg_;
        Program * program_;

    };
//************************************************************************************************************

    template<class I>
    LivenessAnalysis<I>::LivenessAnalysis(Program *p):
            LivenessAnalysis( InstructionAnalysis(p).analyze(), p)
    {
    }

    template<class I>
    std::unordered_set<Declaration*> LivenessAnalysis<I>::getSubtree(const CfgNode<I> *node) {
        DeclarationAnalysis v(program_);
        v.begin(node->il());
        return v.result();
    }

    template<class I>
    typename LivenessAnalysis<I>::NodeState
    LivenessAnalysis<I>::transferFun(const CfgNode<I> *node, const LivenessAnalysis::NodeState &state){
        if(auto tmp = dynamic_cast<const CfgFunEntryNode<I> *>(node)){
            return state;
        }else if (auto tmp1 = dynamic_cast<const CfgFunExitNode<I> *>(node)){

            return nodeLattice_.bot();
        }else if (auto tmp2 = dynamic_cast<const CfgGlobExitNode<I> *>(node)){

            return state;
        }else if(auto * stmtNode = dynamic_cast<const CfgStmtNode<I> *>(node)){
            auto instr = dynamic_cast<ILInstruction *>(stmtNode->il());
            if(instr){ //TODO rules to add and remove from states


                if (dynamic_cast<AllocL *>(stmtNode->il())){
                    auto newState = state;
//                    std::unordered_set<Declaration*> children = getSubtree(node);
//                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;
//                    return ;
                }else if (dynamic_cast<AllocG *>(stmtNode->il())){
                    auto newState = state;
//                    std::unordered_set<Declaration*> children = getSubtree(node);
//                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (auto store = dynamic_cast<Store *>(stmtNode->il())){
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
//                    newState.erase(node->il());
                    return newState;
                }else if (dynamic_cast<CondJump *>(stmtNode->il())){
                    auto newState = state;
                    std::unordered_set<Declaration*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
//                    newState.erase(node->il());
                    return newState;
                }else if (dynamic_cast<Jump *>(stmtNode->il())){
                    auto newState = state;
                    std::unordered_set<Declaration*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
//                    newState.erase(node->il());
                    return newState;
                }else if (dynamic_cast<PutChar *>(stmtNode->il())){
                    auto newState = state;
                    std::unordered_set<Declaration*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;
                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<GetChar *>(stmtNode->il())){
                    auto newState = state;
                    std::unordered_set<Declaration*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<Copy *>(stmtNode->il())){
                    auto newState = state;
                    std::unordered_set<Declaration*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<Extend *>(stmtNode->il())){
                    auto newState = state;
                    std::unordered_set<Declaration*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<Truncate *>(stmtNode->il())){
                    auto newState = state;
                    std::unordered_set<Declaration*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<BinOp *>(stmtNode->il())){
                    auto newState = state;
                    std::unordered_set<Declaration*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<UnOp *>(stmtNode->il())){
                    auto newState = state;
                    std::unordered_set<Declaration*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<Return *>(stmtNode->il())){
                    auto newState = state;
                    std::unordered_set<Declaration*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<Phi *>(stmtNode->il())){
                    auto newState = state;
                    std::unordered_set<Declaration*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<Call *>(stmtNode->il())){
                    auto newState = state;
                    std::unordered_set<Declaration*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<ElemAddrIndex *>(stmtNode->il())){
                    auto newState = state;
                    std::unordered_set<Declaration*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<ElemAddrOffset *>(stmtNode->il())){
                    auto newState = state;
                    std::unordered_set<Declaration*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<StructAssign *>(stmtNode->il())){
                    auto newState = state;
                    std::unordered_set<Declaration*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<CallStatic *>(stmtNode->il())){
                    auto newState = state;
                    std::unordered_set<Declaration*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }
            }


        } else{
        }
        return state;
    }




    template<class Info>
    LiveVars<Info> LivenessAnalysis<Info>::analyze() {
        LiveVars<Info> X = lattice_.bot();
        std::unordered_set<const CfgNode<Info>*> W;
        for( auto & n : cfg_->nodes()){
            W.emplace(n);
        }

        while (!W.empty()) {
            const CfgNode<Info> * n = *W.begin();
            W.erase(W.begin());
            auto x = X.access(n);
            auto y = funOne(n, X);

            if (y != x) {
                X.update(std::make_pair(n, y));//X += n -> y
                //W ++= n.pred;
                W.insert( n->pred_.begin(), n->pred_.end());
            } else if (y.empty()) {
                X.update(std::make_pair(n, y));//X += n -> y;
            }
        }

        return std::move(X);
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
        auto acc = nodeLattice_.bot();
        for (const CfgNode<I> * s : node->succ_) {
            auto pred = state.access(s);
            acc = nodeLattice_.lub(acc, pred);
        }
        return acc;
    }

    template<typename Info>
    LivenessAnalysis<Info>::LivenessAnalysis( const Declarations &declarations, Program * program):
            BackwardAnalysis<LiveVars<Info>, Info>(),
            cfg_(this->getCfg(program)),
            allVars_([&](){
                std::unordered_set< Declaration*> tmp;
                for ( auto & n : cfg_->nodes()){
                    tmp.emplace(n->il());
                }
                return tmp;
            }())
            ,
            nodeLattice_(PowersetLattice<Declaration*>(allVars_)),
            lattice_(MapLattice<const CfgNode<Info>*, std::unordered_set<Declaration*>>(cfg_->nodes(), &nodeLattice_)),
            program_(program){

    }
} //namespace tvlm
