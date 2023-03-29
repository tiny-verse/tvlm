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



    using CPNodeState = MAP<Declaration*, Const*>;
    template<class T>
    using ConstVars = MAP<const CfgNode<T> *, CPNodeState>;
    ;

    template<class Info = DummyClass>
    class ConstantPropagationAnalysis : public BackwardAnalysis<ConstVars<Info>, Info>{

    //    using Declaration = tvlm::Instruction;
    //    using Declarations = MAP< ILInstruction *, Declaration*>;

        CPNodeState join(const CfgNode<Info> * node, ConstVars<Info> & state);

        ConstantPropagationAnalysis( const Declarations & declarations, Program * program);

        Const*  eval(Declaration * expr , const CPNodeState & state);

        CPNodeState transferFun(const CfgNode<Info> * node, const  CPNodeState & state );


        CPNodeState funOne(const CfgNode<Info> * node, ConstVars<Info> & state);
    public:
//        static ConstantPropagationAnalysis<Info> * create(Program * p);
        virtual ~ConstantPropagationAnalysis(){
            delete cfg_;
        }
        explicit ConstantPropagationAnalysis(Program * p);
        ConstVars<Info> analyze() override;

        CPNodeState transform( ConstVars<Info> & analysis)  const
        {
            CPNodeState tmp;
            tmp.withDefault(constPropLattice_->bot());
            for (auto &cfgPair: analysis) {
//            update result for every record
                auto cpRecord = cfgPair.second;
                for(auto & cpRecordLine :  cpRecord){
                    //for every line try to update the bunch where we find if an instruction can be constantly evaluated
                    tmp.update( std::make_pair( cpRecordLine.first, //for instruction in recorded Line update the final result
                                    constPropLattice_->lub(tmp.access(cpRecordLine.first), // lub of current record
                                                                    cpRecordLine.second)) );          //and the new incoming
                }

            }
            return std::move(tmp);
        }

        std::map<const CfgNode<Info>*, const Instruction *>instr_mapping(){
            return instr_mapping_;
        };
    private:
        Program * program_;
        ProgramCfg<Info> * cfg_;
        std::map<const CfgNode<Info>*, const Instruction *>instr_mapping_;
        std::map<const IL*, Const *> varMaps_;
        std::unordered_set<Declaration*> declaredVars_;
        std::unique_ptr<ConstantPropagationLattice> constPropLattice_;
        MapLattice<Declaration*, Const *> nodeLattice_;
        MapLattice<const CfgNode<Info> *, CPNodeState> lattice_;

    };
//************************************************************************************************************

    template<class I>
    ConstantPropagationAnalysis<I>::ConstantPropagationAnalysis(Program *p):
            ConstantPropagationAnalysis( InstructionAnalysis(p).analyze(), p)
    {
    }

    template<class I>
    Const* ConstantPropagationAnalysis<I>::eval(Declaration * expr, const CPNodeState &state) {

        if (auto store = dynamic_cast<Store *>(expr)) {
            return  state.access(store->address());
        } else if (auto load = dynamic_cast<Load *>(expr)) {
            return  state.access(load->address());
        }else if (auto loadImm = dynamic_cast<LoadImm *>(expr)){
//                    std::unordered_set<Declaration*> children = getSubtree(node);
//                    newState.insert(children.begin(), children.end());
            if(loadImm->resultType() == ResultType::Double){
                 return constPropLattice_->num(Constant(loadImm->valueFloat()));
            }else{
                return  constPropLattice_->num(Constant(loadImm->valueInt()));
            }

        } if (auto cpy = dynamic_cast<Copy *>(expr)){

            return state.access(cpy->src()) ;

        }else if (auto ext = dynamic_cast<Extend *>(expr)){
            auto old = eval(ext->src(), state);
            return  constPropLattice_->extend(old) ;

        }else if (auto trunc = dynamic_cast<Truncate *>(expr)){
            auto old = eval(trunc->src(), state);
            return constPropLattice_->trunc(old) ;
        }else if (auto binop = dynamic_cast<BinOp *>(expr)){
            auto oldLhs = eval(binop->lhs(), state);
            auto oldRhs = eval(binop->rhs(), state);
            switch(binop->opType()){
                case BinOpType::ADD:
                    return  constPropLattice_->plus(oldLhs, oldRhs);
                    break;
                case BinOpType::SUB:
                    return  constPropLattice_->minus(oldLhs, oldRhs);
                    break;
                case BinOpType::MUL:
                    return  constPropLattice_->times(oldLhs, oldRhs);
                    break;
                case BinOpType::DIV:
                    return  constPropLattice_->div(oldLhs, oldRhs);
                    break;
                case BinOpType::MOD:
                    return  constPropLattice_->mod(oldLhs, oldRhs);
                    break;
                case BinOpType::LSH:
                    return  constPropLattice_->lsh(oldLhs, oldRhs);
                    break;
                case BinOpType::RSH:
                    return  constPropLattice_->rsh(oldLhs, oldRhs);
                    break;
                case BinOpType::AND:
                    return  constPropLattice_->band(oldLhs, oldRhs);
                    break;
                case BinOpType::XOR:
                    return  constPropLattice_->bxor(oldLhs, oldRhs);
                    break;
                case BinOpType::OR:
                    return  constPropLattice_->bor(oldLhs, oldRhs);
                    break;
                case BinOpType::NEQ:
                    return  constPropLattice_->neq(oldLhs, oldRhs);
                    break;
                case BinOpType::EQ:
                    return  constPropLattice_->eq(oldLhs, oldRhs);
                    break;
                case BinOpType::LTE:
                    return  constPropLattice_->lte(oldLhs, oldRhs);
                    break;
                case BinOpType::LT:
                    return  constPropLattice_->lt(oldLhs, oldRhs);
                    break;
                case BinOpType::GT:
                    return  constPropLattice_->gt(oldLhs, oldRhs);
                    break;
                case BinOpType::GTE:
                    return  constPropLattice_->gte(oldLhs, oldRhs);
                    break;
            }

        }else if (auto unop = dynamic_cast<UnOp *>(expr)){
            auto old = eval(unop->operand(), state);

            switch (unop->opType()) {
                case UnOpType::UNSUB:
                    return  constPropLattice_->unminus(old);
                case UnOpType::NOT:
                    return  constPropLattice_->unnot(old);
                case UnOpType::INC:
                    return  constPropLattice_->uninc(old);
                case UnOpType::DEC:
                    return  constPropLattice_->undec(old);
            }

        }else if (auto phi = dynamic_cast<Phi *>(expr)){
            if(phi->contents().size() <2){
                if(phi->contents().empty()){
                    throw "[Constant Propagation] empty phi?";
                }else{
                    auto cont = * phi->contents().begin();

                    auto old = eval(cont.second, state);
                    return  constPropLattice_->undec(old) ;
                }
            }else{
                Const * tmp = eval(phi->contents().begin()->second, state);
                for (auto it = ++phi->contents().begin()  ; it != phi->contents().end() ;it++ ) {
                    tmp = constPropLattice_->phi(tmp, eval(it->second, state));
                }
                return  tmp ;
            }


        }else{
            return constPropLattice_->top();
        }
        throw "[Constant propagation] cannot eval";
    }

    template<class I>
    CPNodeState
    ConstantPropagationAnalysis<I>::transferFun(const CfgNode<I> *node, const CPNodeState &state){
        if(auto tmp = dynamic_cast<const CfgFunEntryNode<I> *>(node)){
            return state;
        }else if (auto tmp1 = dynamic_cast<const CfgFunExitNode<I> *>(node)){

            return nodeLattice_.bot();
        }else if (auto tmp2 = dynamic_cast<const CfgGlobExitNode<I> *>(node)){

            return state;
        }else if(auto * stmtNode = dynamic_cast<const CfgStmtNode<I> *>(node)){
            auto instr = dynamic_cast<ILInstruction *>(stmtNode->il());
            if(instr) { //TODO rules to add and remove from states


                if (dynamic_cast<AllocL *>(stmtNode->il())) {
                    auto newState = state;
                    newState.insert(std::make_pair(instr, constPropLattice_->bot()));
                    return newState;
//                    return ;
                } else if (dynamic_cast<AllocG *>(stmtNode->il())) {
                    auto newState = state;
                    newState.insert(std::make_pair(instr, constPropLattice_->bot()));
                    return newState;

                } else if (dynamic_cast<ArgAddr *>(stmtNode->il())) {
                    auto newState = state;
                    newState.update(std::make_pair(instr, constPropLattice_->top()));
                    return newState;
                }
                else if (auto store = dynamic_cast<Store *>(stmtNode->il())) {
                    auto newState = state;

                    Const * newValue = eval( store->value(),state);
                    Const * realValue = constPropLattice_->lub(newValue, state.access(store->address()));
                    newState.update(std::make_pair(store->address(), realValue));
                    return newState;
                }
//                else if (auto load = dynamic_cast<Load *>(stmtNode->il())) {
//                    auto newState = state;
//
//                    newState.update(std::make_pair(load, state.access(load->address())));
//
//                    return newState;
//                }
                else {}
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
                W.insert( this->next(n).begin(), this->next(n).end());
            } else if (y.empty()) {
                X.update(std::make_pair(n, y));//X += n -> y;
            }
        }


        return std::move(X);
    }

    template<class I>
    CPNodeState
    ConstantPropagationAnalysis<I>::funOne(const CfgNode<I> * node, ConstVars<I> & state)
    {
        return ConstantPropagationAnalysis<I>::transferFun(node, join(node, state));
    }

    template<class I>
    CPNodeState
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
            constPropLattice_(std::make_unique<ConstantPropagationLattice>() ),
            nodeLattice_(MapLattice<Declaration*, Const*>(declaredVars_, constPropLattice_.get() )),
            lattice_(MapLattice<const CfgNode<Info>*, CPNodeState>(cfg_->nodes(), &nodeLattice_)),
            program_(program){

    }
} //namespace tvlm
