#pragma once

#include <utility>
#include <unordered_set>
#include <map>

#include "cfgNode.h"

using Function = tvlm::Function;
using Program = tvlm::Program;
class cfg {

};
namespace tvlm{

class FragmentCfg {
public:
    virtual ~FragmentCfg() {};
    FragmentCfg(const  std::unordered_set<CfgNode*>  & entryNodes,const  std::unordered_set<CfgNode*> &  exitNodes):
    entryNodes_(entryNodes),
    exitNodes_(exitNodes){

    }
    FragmentCfg(const FragmentCfg & other) : entryNodes_(other.entryNodes_),
    exitNodes_(other.exitNodes_){}

    bool isEmpty()const {
        return entryNodes_.empty() && exitNodes_.empty();
    }

    FragmentCfg * concat(FragmentCfg * that );

    FragmentCfg * operator |(FragmentCfg * that) ;

    std::unordered_set<const CfgNode*> nodes();

protected:
    std::unordered_set<const CfgNode*> visit(const CfgNode * n);
    std::unordered_set<CfgNode*> entryNodes_;
    std::unordered_set<CfgNode*> exitNodes_;

private:
};

class FunctionCfg : public FragmentCfg{
public:
    FunctionCfg( Function * fnc, CfgFunEntryNode * entry, CfgFunExitNode * exit, FragmentCfg *  cfg = nullptr) :
            FragmentCfg([&]( CfgFunEntryNode * entry) {
                std::unordered_set<CfgNode*> res;
                res.insert(entry);
                return res;
            }(entry),[&]( CfgFunEntryNode * exit) {
                std::unordered_set<CfgNode*> res;
                res.insert(exit);
                return res;
            }(entry)),
//            nodes_(std::vector<std::unique_ptr<CfgNode>>()),
            entry_(entry),
            exit_(exit),
            cfg_(cfg){

    }
    FunctionCfg(FunctionCfg && fnc);
    virtual ~FunctionCfg(){
        for (auto *n : nodes_) {
            delete n;
        }
        for (auto* n : frags_) {
            delete n;
        }
    }
    const CfgFunEntryNode * entry()const {
        return entry_;
    }
    const CfgFunExitNode * exit()const {
        return exit_;
    }
    CfgNode * addNode(CfgNode * node){
        nodes_.push_back((node));
        return node;
    }
    FragmentCfg * addFragment(FragmentCfg * f){
        frags_.push_back(f);
        return f;
    }
    void setCfg(FragmentCfg * cfg){
        cfg_ = cfg;
    }
private:
    std::vector<CfgNode *> nodes_;
    std::vector<FragmentCfg *> frags_;
    CfgFunEntryNode *entry_;
    CfgFunExitNode * exit_;
    FragmentCfg * cfg_;
};


class ProgramCfg : public FragmentCfg{
protected:
    ProgramCfg(Program * program, std::unordered_set<CfgNode*> && entry,
               std::unordered_set<CfgNode*> && exit,
               std::vector<std::unique_ptr<FunctionCfg>> && functionsCfg):
            FragmentCfg(std::move(entry), std::move(exit)),
            functionsCfg_(std::move(functionsCfg)),
            p_(program)
            {

    }

public:
    ProgramCfg(ProgramCfg && other): FragmentCfg(std::move(other.entryNodes_), std::move(other.exitNodes_)),
    functionsCfg_(std::move(other.functionsCfg_) ), p_(other.p_){

    }

    ProgramCfg(const ProgramCfg & other) = delete;
//    :FragmentCfg(other.entryNodes_, other.exitNodes_),
//    p_(other.p_){
//        functionsCfg_.reserve(other.functionsCfg_.size());
//        auto e  = other.functionsCfg_.begin();
//        auto f = functionsCfg_.begin();
//        for (; e != other.functionsCfg_.end();e++, f++) {
//            functionsCfg_.emplace_back(new FunctionCfg(**(e)));
//        }
//    }

public:

    static ProgramCfg get(Program * program, std::unordered_set<FunctionCfg*> & functionsCfg);


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
        std::unordered_set<FunctionCfg*> tmp;
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
    
    FragmentCfg * fromInstruction(ILInstruction * ins);

    FunctionCfg * fromFunction(Function * fnc);


private:
    CfgNode * makeStmtNode(ILInstruction * il){
        auto it = allNodesMap_.find(il);
        if(it != allNodesMap_.end()){
            return it->second;
        }
        return allNodesMap_[il] = append(new CfgStmtNode(il));
    }


    CfgNode * append(CfgNode * a){
//        allNodes.emplace_back(a);
        current_fnc->addNode(a);
        return a;
    }

    FragmentCfg * append(FragmentCfg * a){
//        allFragments.emplace_back(a);
        current_fnc->addFragment(a);
    return a;
    }

    std::vector<std::unique_ptr<CfgNode>> allNodes;
    std::vector<std::unique_ptr<FragmentCfg>> allFragments;
    std::map<IL*, CfgNode*> allNodesMap_;
    std::map<IL*, FragmentCfg*> allFragmentsMap_;
    FragmentCfg * fncexit_;
    FunctionCfg * current_fnc;

    FragmentCfg * empty(){
        return new FragmentCfg(std::unordered_set<CfgNode*> (), std::unordered_set<CfgNode*> ());
    }
    FragmentCfg * single(CfgNode * node ){
        std::unordered_set<CfgNode *> a;
        a.insert(node);
        return append(new FragmentCfg(a, a));
    }
    //liveness for registers
};


} // namespace tiny::tvlm
