#pragma once

#include <map>
#include <queue>
#include "tvlm/tvlm/il/il_builder.h"
#include "tvlm/tvlm/il/il_insns.h"
#include "instruction_analysis.h"
#include "cfg.h"
#include "DeclarationAnalysis.h"

///NO USAGE


namespace tvlm{
    using Instruction = ::tvlm::Instruction;

    template<class T>
    using LiveVars = MAP<const CfgNode<T> *, std::unordered_set<Declaration*>>;
    ;
    template<class T>
    using OutputLiveVars = MAP<const CfgNode<T> *, std::unordered_set<VirtualRegisterPlaceholder*>>;
    ;

    template<class Info>
    class ColoringLiveAnalysis : public BackwardAnalysis<LiveVars<Info>, Info>{
    protected:
        Program * getProgram(TargetProgram * p)const {
            return TargetProgramFriend::getProgram(p);
        }
    private:
//        using Declaration = tvlm::Instruction;
        using NodeState = std::unordered_set<Declaration*>;
    //    using Declarations = MAP< ILInstruction *, Declaration*>;

        NodeState join(const CfgNode<Info> * node, LiveVars<Info> & state);

        ColoringLiveAnalysis(ProgramCfg<Info> * cfg, const Declarations & declarations, TargetProgram * program);

        std::unordered_set<Declaration*> getSubtree(const CfgNode<Info> *pNode);

        NodeState transferFun(const CfgNode<Info> * node, const  NodeState & state );


        NodeState funOne(const CfgNode<Info> * node, LiveVars<Info> & state);
    public:
//        static LivenessAnalysisTMP<Info> * create(Program * p);
        virtual ~ColoringLiveAnalysis(){
            delete cfg_;
        }
        explicit ColoringLiveAnalysis(TargetProgram * p);
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
        TargetProgram * program_;

    };
//************************************************************************************************************

    template<class I>
    ColoringLiveAnalysis<I>::ColoringLiveAnalysis(TargetProgram *p):
            ColoringLiveAnalysis(BackwardAnalysis<LiveVars<I>,I>::getCfg(getProgram(p)), InstructionAnalysis(getProgram(p)).analyze(), p)
    {
    }

    template<class I>
    std::unordered_set<Declaration*> ColoringLiveAnalysis<I>::getSubtree(const CfgNode<I> *node) {
        DeclarationAnalysis v(getProgram(program_));
        v.begin(node->il());
        return v.result();
    }

    template<class I>
    typename ColoringLiveAnalysis<I>::NodeState
    ColoringLiveAnalysis<I>::transferFun(const CfgNode<I> *node, const ColoringLiveAnalysis::NodeState &state){
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
                }else if (dynamic_cast<AllocG *>(stmtNode->il())){
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
            }else if (dynamic_cast<const CfgFunExitNode<I> *>(node)){

                return nodeLattice_.bot();
            }


        } else{
        }
        return state;
    }




    template<class Info>
    LiveVars<Info> ColoringLiveAnalysis<Info>::analyze() {
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
                W.update( n->pred_.begin(), n->pred_.end());
            } else if (y.empty()) {
                X.update(std::make_pair(n, y));//X += n -> y;
            }
        }

        return std::move(X);
    }

    template<class I>
    typename ColoringLiveAnalysis<I>::NodeState
    ColoringLiveAnalysis<I>::funOne(const CfgNode<I> * node, LiveVars<I> & state)
    {
        return ColoringLiveAnalysis<I>::transferFun(node, join(node, state));
    }

    template<class I>
    typename ColoringLiveAnalysis<I>::NodeState
    ColoringLiveAnalysis<I>::join(const CfgNode<I> * node, LiveVars<I> & state)
    {
        auto acc = nodeLattice_.bot();
        for (const CfgNode<I> * s : node->succ_) {
            auto pred = state.access(s);
            acc = nodeLattice_.lub(acc, pred);
        }
        return acc;
    }

    template<typename Info>
    ColoringLiveAnalysis<Info>::ColoringLiveAnalysis(ProgramCfg<Info> * cfg, const Declarations &declarations, TargetProgram * program):
            allVars_([&](){
                std::unordered_set< Declaration*> tmp;
                for ( auto & n : cfg->nodes()){
                    tmp.emplace(n->il());
                }
                return tmp;
            }())
            ,
            nodeLattice_(PowersetLattice<Declaration*>(allVars_)),
            lattice_(MapLattice<const CfgNode<Info>*, std::unordered_set<Declaration*>>(cfg->nodes(), &nodeLattice_)),
            cfg_(cfg),
            program_(program){

    }
} //namespace tvlm
