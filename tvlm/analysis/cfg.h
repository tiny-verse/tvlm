#pragma once

#include <utility>

#include "cfgNode.h"
#include "tvlm_backend"

using Function = tvlm::Function;
using Program = tvlm::Program;
class cfg {

};
namespace tvlm{

class FragmentCfg {
public:
    FragmentCfg( std::set<CfgNode*>  entryNodes, std::set<CfgNode*>  exitNodes):
    entryNodes_(std::move(entryNodes)),
    exitNodes_(std::move(exitNodes)){

    }

    bool isEmpty()const {
        return entryNodes_.empty() && exitNodes_.empty();
    }

    FragmentCfg * concat(FragmentCfg * that );

    FragmentCfg * operator |(FragmentCfg * that) ;

    std::set<const CfgNode*> nodes();

protected:
    std::set<const CfgNode*> visit(const CfgNode * n);
    std::set<CfgNode*> entryNodes_;
    std::set<CfgNode*> exitNodes_;

private:
};

class FunctionCfg : public FragmentCfg{
public:
    FunctionCfg( Function * fnc, CfgFunEntryNode * entry, CfgFunExitNode * exit, FragmentCfg *  cfg) :
            FragmentCfg([&]( CfgFunEntryNode * entry) {
                std::set<CfgNode*> res;
                res.insert(entry);
                return res;
            }(entry),[&]( CfgFunEntryNode * exit) {
                std::set<CfgNode*> res;
                res.insert(exit);
                return res;
            }(entry)),

            entry_(entry),
            exit_(exit),
            cfg_(cfg){

    }
    const CfgFunEntryNode * entry()const {
        return entry_;
    }
    const CfgFunExitNode * exit()const {
        return exit_;
    }
private:
    CfgFunEntryNode *entry_;
    CfgFunExitNode * exit_;
    FragmentCfg * cfg_;
};

class ProgramCfg : public FragmentCfg{
protected:
    ProgramCfg(Program * program, std::set<CfgNode*> && entry,
               std::set<CfgNode*> && exit,
               std::vector<std::unique_ptr<FunctionCfg>> && functionsCfg):
            FragmentCfg(std::move(entry), std::move(exit)),
            functionsCfg_(std::move(functionsCfg)),
            p_(program)
            {

    }

public:
    ProgramCfg(ProgramCfg && other): FragmentCfg(std::move(other.entryNodes_), std::move(other.exitNodes_)),functionsCfg_(std::move(other.functionsCfg_) ), p_(other.p_){

    }

    ProgramCfg(const ProgramCfg & other)
    :FragmentCfg(other.entryNodes_, other.exitNodes_),
    p_(other.p_){
        functionsCfg_.reserve(other.functionsCfg_.size());
        auto e  = other.functionsCfg_.begin();
        auto f = functionsCfg_.begin();
        for (; e != other.functionsCfg_.end();e++, f++) {
            functionsCfg_.emplace_back(new FunctionCfg(**(e)));
        }
    }

public:

    static ProgramCfg get(Program * program, std::set<FunctionCfg*> & functionsCfg){
        std::set<CfgNode*> setEntry;
        for(auto & e: functionsCfg ){
            setEntry.insert((CfgNode*)e->entry());
        }
        std::set<CfgNode*> setExit;
        for(auto & e: functionsCfg ){
            setExit.insert((CfgNode*)e->exit());
        }

        std::vector<std::unique_ptr<FunctionCfg>> tmp;
        tmp.reserve(functionsCfg.size());
        for (const auto e: functionsCfg) {
            tmp.emplace_back(e);
        }
        return ProgramCfg(program, std::move(setEntry) ,std::move(setExit) , std::move(tmp) );
    }


private:
    std::vector<std::unique_ptr<FunctionCfg>> functionsCfg_;
    Program  * p_;

};

using ILInstruction = ::tvlm::Instruction;
using Function = ::tvlm::Function;
class CfgBuilder {
public:
    virtual FragmentCfg* fromInstruction( ILInstruction* ins ) = 0;

    virtual FunctionCfg* fromFunction( Function * fnc) = 0;

    ProgramCfg fromProgram(Program * p){
        CfgNode::counter_ = 0;
        std::set<FunctionCfg*> tmp;
        for (auto & ff : p->functions()) {
            tmp.insert( fromFunction(ff.second.get()) );
        }

        return ProgramCfg::get(p, tmp);
    }

protected:
    static std::vector<std::unique_ptr<::tvlm::BasicBlock>> & getBBs(Function * fnc){
        return fnc->bbs_;
    }

    static std::vector<std::unique_ptr<Instruction>> & getInstructions(::tvlm::BasicBlock * bb) {
        return bb->insns_;
    }
};

class IntraProceduralCfgBuilder : public CfgBuilder{
public:
    FragmentCfg * fromBB(::tvlm::BasicBlock * bb);


    FragmentCfg * fromInstruction(ILInstruction * ins){
//        if(dynamic_cast<::tvlm::Load *>(ins)) {//assignment // TODO
//            return single(new CfgStmtNode(ins));
//        }else if (){
//
//        }

        if(dynamic_cast<::tvlm::CondJump *>(ins)) {
           ::tvlm::CondJump * condJump = dynamic_cast<::tvlm::CondJump*>(ins);
           auto guardCfg = single(append(new CfgStmtNode(condJump->condition())));
           auto guardedThenCfg = append(guardCfg->concat(append(fromBB(condJump->getTarget(1)))));
           if(condJump->numTargets() == 2){
               auto guardedElseCfg = append(guardCfg->concat(append(fromBB(condJump->getTarget(0)))));
               return append(*guardedThenCfg | guardedElseCfg);
           }
           return append( *guardedThenCfg | guardCfg);
        }

        return single(append(new CfgStmtNode(ins)));
    }

    FunctionCfg * fromFunction(Function * fnc);


private:
    CfgNode * append(CfgNode * a){
        allNodes.emplace_back(a);
        return a;
    }

    FragmentCfg * append(FragmentCfg * a){
        allFragments.emplace_back(a);
        return a;
    }

    std::vector<std::unique_ptr<CfgNode>> allNodes;
    std::vector<std::unique_ptr<FragmentCfg>> allFragments;

    FragmentCfg * empty(){
        return append(new FragmentCfg(std::set<CfgNode*> (), std::set<CfgNode*> ()));
    }
    FragmentCfg * single(CfgNode * node ){
        std::set<CfgNode *> a;
        a.insert(node);
        return append(new FragmentCfg(a, a));
    }
    //liveness for registers
};


} // namespace tiny::tvlm
