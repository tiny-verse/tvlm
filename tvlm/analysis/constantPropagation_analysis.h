#pragma once

#include <map>
#include <queue>
#include "tvlm/tvlm/il/il_builder.h"
#include "tvlm/tvlm/il/il_insns.h"
#include "instruction_analysis.h"
#include "lattice/constant_propagation_lattice.h"
#include "cfg.h"
#include "DeclarationAnalysis.h"

namespace tvlm{
    using Instruction = ::tvlm::Instruction;



    template<class T>
    using ConstVars = MAP<const CfgNode<T> *, MAP<Declaration*, Const*>>;
    ;

    template<class Info = DummyClass>
    class ConstantPropagationAnalysis : public BackwardAnalysis<ConstVars<Info>, Info>{

    //    using Declaration = tvlm::Instruction;
    //    using Declarations = MAP< ILInstruction *, Declaration*>;
        using NodeState = MAP<Declaration*, Const*>;

        NodeState join(const CfgNode<Info> * node, ConstVars<Info> & state);

        ConstantPropagationAnalysis( const Declarations & declarations, Program * program);

        Const*  eval(Declaration * expr , NodeState & state);

        NodeState transferFun(const CfgNode<Info> * node, const  NodeState & state );


        NodeState funOne(const CfgNode<Info> * node, ConstVars<Info> & state);
    public:
//        static ConstantPropagationAnalysis<Info> * create(Program * p);
        virtual ~ConstantPropagationAnalysis(){
            delete cfg_;
        }
        explicit ConstantPropagationAnalysis(Program * p);
        ConstVars<Info> analyze() override;

        std::map<const CfgNode<Info>*, const Instruction *>instr_mapping(){
            return instr_mapping_;
        };
    private:
        Program * program_;
        ProgramCfg<Info> * cfg_;
        std::map<const CfgNode<Info>*, const Instruction *>instr_mapping_;
        std::unordered_set<Declaration*> declaredVars_;
        std::unique_ptr<ConstantPropagationLattice<Info>> constPropLattice_;
        MapLattice<Declaration*, Const *> nodeLattice_;
        MapLattice<const CfgNode<Info> *, NodeState> lattice_;

    };
//************************************************************************************************************

    template<class I>
    ConstantPropagationAnalysis<I>::ConstantPropagationAnalysis(Program *p):
            ConstantPropagationAnalysis( InstructionAnalysis(p).analyze(), p)
    {
    }

    template<class I>
    Const* ConstantPropagationAnalysis<I>::eval(Declaration * expr, ConstantPropagationAnalysis<I>::NodeState &state) {

        //TODO

    }

    template<class I>
    typename ConstantPropagationAnalysis<I>::NodeState
    ConstantPropagationAnalysis<I>::transferFun(const CfgNode<I> *node, const ConstantPropagationAnalysis::NodeState &state){
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
                        newState.insert(std::make_pair(instr, constPropLattice_->top()));
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
//                    std::unordered_set<Declaration*> children = getSubtree(node);
//                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;
                }else if (dynamic_cast<Load *>(stmtNode->il())){
                    auto newState = state;
//                    std::unordered_set<Declaration*> children = getSubtree(node);
//                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;
                }else if (dynamic_cast<LoadImm *>(stmtNode->il())){
                    auto newState = state;
//                    std::unordered_set<Declaration*> children = getSubtree(node);
//                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;
                }else if (dynamic_cast<Return *>(stmtNode->il())){
                    auto ret  = dynamic_cast<Return *>(stmtNode->il());
                    auto newState = state;
//                    newState.emplace(ret->returnValue());
//                    newState.erase(node->il());
                    return newState;
                }else if (dynamic_cast<CondJump *>(stmtNode->il())){
                    auto newState = state;
//                    std::unordered_set<Declaration*> children = getSubtree(node);
//                    newState.insert(children.begin(), children.end());
//                    newState.erase(node->il());
                    return newState;
                }else if (dynamic_cast<Copy *>(stmtNode->il())){
                    auto newState = state;
//                    std::unordered_set<Declaration*> children = getSubtree(node);
//                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<Extend *>(stmtNode->il())){
                    auto newState = state;
//                    std::unordered_set<Declaration*> children = getSubtree(node);
//                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<Truncate *>(stmtNode->il())){
                    auto newState = state;
//                    std::unordered_set<Declaration*> children = getSubtree(node);
//                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<BinOp *>(stmtNode->il())){
                    auto newState = state;
//                    std::unordered_set<Declaration*> children = getSubtree(node);
//                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<UnOp *>(stmtNode->il())){
                    auto newState = state;
//                    std::unordered_set<Declaration*> children = getSubtree(node);
//                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<Phi *>(stmtNode->il())){
                    auto newState = state;
//                    std::unordered_set<Declaration*> children = getSubtree(node);
//                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<Call *>(stmtNode->il())){
                    auto newState = state;
//                    std::unordered_set<Declaration*> children = getSubtree(node);
//                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<ElemAddrIndex *>(stmtNode->il())){
                    auto newState = state;
//                    std::unordered_set<Declaration*> children = getSubtree(node);
//                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<ElemAddrOffset *>(stmtNode->il())){
                    auto newState = state;
//                    std::unordered_set<Declaration*> children = getSubtree(node);
//                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<StructAssign *>(stmtNode->il())){
                    auto newState = state;
//                    std::unordered_set<Declaration*> children = getSubtree(node);
//                    newState.insert(children.begin(), children.end());
                    newState.erase(node->il());
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<CallStatic *>(stmtNode->il())){
                    auto newState = state;
//                    std::unordered_set<Declaration*> children = getSubtree(node);
//                    newState.insert(children.begin(), children.end());
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
    ConstVars<Info> ConstantPropagationAnalysis<Info>::analyze() {
        ConstVars<Info> X = lattice_.bot();
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
    typename ConstantPropagationAnalysis<I>::NodeState
    ConstantPropagationAnalysis<I>::funOne(const CfgNode<I> * node, ConstVars<I> & state)
    {
        return ConstantPropagationAnalysis<I>::transferFun(node, join(node, state));
    }

    template<class I>
    typename ConstantPropagationAnalysis<I>::NodeState
    ConstantPropagationAnalysis<I>::join(const CfgNode<I> * node, ConstVars<I> & state)
    {
        auto acc = nodeLattice_.bot();
        for (const CfgNode<I> * s : node->pred_) {
            auto pred = state.access(s);
            acc = nodeLattice_.lub(acc, pred);
        }
        return acc;
    }

    template<typename Info>
    ConstantPropagationAnalysis<Info>::ConstantPropagationAnalysis( const Declarations &declarations, Program * program):
            BackwardAnalysis<ConstVars<Info>, Info>(),
            cfg_(this->getCfg(program)),
            declaredVars_([&](){
                std::unordered_set< Declaration*> tmp;
                for ( auto & n : cfg_->nodes()){
                    tmp.emplace(n->il());
                }
                return tmp;
            }())
            ,
            constPropLattice_(std::make_unique<ConstantPropagationLattice<Info>>() ),
            nodeLattice_(MapLattice<Declaration*, Const*>(declaredVars_, constPropLattice_.get() )),
            lattice_(MapLattice<const CfgNode<Info>*, NodeState>(cfg_->nodes(), &nodeLattice_)),
            program_(program){

    }
} //namespace tvlm
