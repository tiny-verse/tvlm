#pragma once

#include <unordered_map>
//
//#include "t86_backend.h"
#include "il.h"
//#include "tvlm_backend"

using Opcode = tvlm::Instruction::Opcode;
namespace tvlm{
    class SpecialRuleBin : public Rule{
    public:
        SpecialRuleBin(const tvlm::Opcode &opcode):Rule(1), opcode_(opcode){
            [&](DAG * dagInstr){
                tiny::t86::ProgramBuilder b;
                tiny::t86::Register reg = dagInstr->reg;
                tiny::t86::Register lreg = ((*dagInstr)[0])->reg;
                tiny::t86::Register rreg = ((*dagInstr)[1])->reg;
                b.add(tiny::t86::ADD(lreg, rreg));
                b.add(tiny::t86::MOV(reg, lreg));
                return b.program().moveInstructions();
            };


        }

        bool matchesOpcode(const Opcode &opcode) const override {
            return opcode == opcode_;
        }

        Rule *makeCopy() const override {
            return new SpecialRuleBin(opcode_);
        }

        bool operator==(const DAG *other) override {
            const tvlm::Instruction::BinaryOperator* binOp;
            return other->ins()->opcode_ == opcode_
                && (binOp = dynamic_cast<const tvlm::Instruction::BinaryOperator*>(other->ins()))
                && binOp->opType() == binOpType_;

        }

        int countCost() override {
            // need to know subRules;
            return cost_;
        }
        tvlm::Opcode opcode_;
        tvlm::BinOpType binOpType_;

    };



std::vector<std::unique_ptr<Rule>> tvlm::ILTiler::AllRulesInit(){
    std::vector<std::unique_ptr<Rule>> all;
    all.emplace_back(new DummyRule());
    //--------------------------------------------------------------------
//
//    std::set<std::unique_ptr<Rule>> addRules;
//    std::set<std::unique_ptr<Rule>> subRules;
//    std::set<std::unique_ptr<Rule>> mulRules;
//    std::set<std::unique_ptr<Rule>> modRules;
//    std::set<std::unique_ptr<Rule>> divRules;
//    std::set<std::unique_ptr<Rule>> lshRules;
//    std::set<std::unique_ptr<Rule>> rshRules;
//    std::set<std::unique_ptr<Rule>> xorRules;
//    std::set<std::unique_ptr<Rule>> orRules;
//    std::set<std::unique_ptr<Rule>> andRules;
//    std::set<std::unique_ptr<Rule>> neqRules;
//    std::set<std::unique_ptr<Rule>> eqRules;
//    std::set<std::unique_ptr<Rule>> lteRules;
//    std::set<std::unique_ptr<Rule>> ltRules;
//    std::set<std::unique_ptr<Rule>> gtRules;
//    std::set<std::unique_ptr<Rule>> gteRules;
//// 'BIN' R1 R2
////    binRules.insert(std::make_unique<SpecializedRule>(Opcode::BinOp, 1, 2));
//    addRules.insert(std::make_unique<SpecializedRule>(Opcode::ADD, 1, 2));
//    subRules.insert(std::make_unique<SpecializedRule>(Opcode::SUB, 1, 2));
//    mulRules.insert(std::make_unique<SpecializedRule>(Opcode::MUL, 1, 2));
//    modRules.insert(std::make_unique<SpecializedRule>(Opcode::MOD, 1, 2));
//    divRules.insert(std::make_unique<SpecializedRule>(Opcode::DIV, 1, 2));
//    lshRules.insert(std::make_unique<SpecializedRule>(Opcode::LSH, 1, 2));
//    rshRules.insert(std::make_unique<SpecializedRule>(Opcode::RSH, 1, 2));
//    xorRules.insert(std::make_unique<SpecializedRule>(Opcode::XOR, 1, 2));
//    orRules.insert(std::make_unique<SpecializedRule>(Opcode::OR, 1, 2));
//    andRules.insert(std::make_unique<SpecializedRule>(Opcode::AND, 1, 2));
//    neqRules.insert(std::make_unique<SpecializedRule>(Opcode::NEQ, 1, 2));
//    eqRules.insert(std::make_unique<SpecializedRule>(Opcode::EQ, 1, 2));
//    lteRules.insert(std::make_unique<SpecializedRule>(Opcode::LTE, 1, 2));
//    ltRules.insert(std::make_unique<SpecializedRule>(Opcode::LT, 1, 2));
//    gteRules.insert(std::make_unique<SpecializedRule>(Opcode::GTE, 1, 2));
//    gtRules.insert(std::make_unique<SpecializedRule>(Opcode::GT, 1, 2));
//
//    children.clear();
//    children.push_back(std::shared_ptr<Rule>(tvlm::DummyRule::getnew()));
//    children.push_back(std::shared_ptr<Rule>(new SpecializedRule(Opcode::LoadImm, 1)));
////    binRules.insert(std::make_unique< SpecializedRule>(Opcode::BinOp,  1, children));
//// ADD R1 I
//    addRules.insert(std::make_unique<SpecializedRule>(Opcode::ADD, 1, children));
//    subRules.insert(std::make_unique<SpecializedRule>(Opcode::SUB, 1, children));
//    mulRules.insert(std::make_unique<SpecializedRule>(Opcode::MUL, 1, children));
//    modRules.insert(std::make_unique<SpecializedRule>(Opcode::MOD, 1, children));
//    divRules.insert(std::make_unique<SpecializedRule>(Opcode::DIV, 1, children));
//    lshRules.insert(std::make_unique<SpecializedRule>(Opcode::LSH, 1, children));
//    rshRules.insert(std::make_unique<SpecializedRule>(Opcode::RSH, 1, children));
//    xorRules.insert(std::make_unique<SpecializedRule>(Opcode::XOR, 1, children));
//    orRules.insert(std::make_unique<SpecializedRule>(Opcode::OR, 1, children));
//    andRules.insert(std::make_unique<SpecializedRule>(Opcode::AND, 1, children));
//    neqRules.insert(std::make_unique<SpecializedRule>(Opcode::NEQ, 1, children));
//    eqRules.insert(std::make_unique<SpecializedRule>(Opcode::EQ, 1, children));
//    lteRules.insert(std::make_unique<SpecializedRule>(Opcode::LTE, 1, children));
//    ltRules.insert(std::make_unique<SpecializedRule>(Opcode::LT, 1, children));
//    gteRules.insert(std::make_unique<SpecializedRule>(Opcode::GTE, 1, children));
//    gtRules.insert(std::make_unique<SpecializedRule>(Opcode::GT, 1, children));
//
//    //------------------------------------------------------------------
//    std::set<std::unique_ptr<Rule>> unRules;
//
//    unRules.insert(std::make_unique<SpecializedRule>(Opcode::UNSUB,  1, 1));
//
//
//
//    //------------------------------------------------------------------
//    std::set<std::unique_ptr<Rule>> allocRules;
//
//    allocRules.insert( std::make_unique<SpecializedRule>(Opcode::AllocL,  1, 0));
//
//
//
//
//    //------------------------------------------------------------------
//    std::set<std::unique_ptr<Rule>> allocGRules;
//
//    allocRules.insert( std::make_unique<SpecializedRule >(Opcode::AllocG, 1, 0));
//
//
//
//
//
//
//    //------------------------------------------------------------------
//    std::set<std::unique_ptr<Rule>> valueRules;
//
//    valueRules.insert( std::make_unique<SpecializedRule>(Opcode::LoadImm,  1, 0));
//
//
//
//
//
//
//    //------------------------------------------------------------------
//    std::set<std::unique_ptr<Rule>> storeRules;
//
//    storeRules.insert( std::make_unique<SpecializedRule>(Opcode::Store,  1, 2));
//
//
//
//
//
//
//
//
//    //------------------------------------------------------------------
//    std::set<std::unique_ptr<Rule>> loadRules;
//
//    loadRules.insert( std::make_unique<SpecializedRule>(Opcode::Load,  1, 1));
//
//
//
//    //--------------------------------------------------------------------
//
//    std::set<std::unique_ptr<Rule>> returnRules;
//
//    returnRules.insert( std::make_unique<SpecializedRule>(Opcode::Return,  1, 1));
//
//
//
//
//    //--------------------------------------------------------------------
//
//    std::set<std::unique_ptr<Rule>> callRules;
//
//    callRules.insert( std::make_unique<SpecializedRule>(Opcode::Call,  1, 1));
//
//
//
//
//    //--------------------------------------------------------------------
//
//    std::set<std::unique_ptr<Rule>> callStaticRules;
//
//    callStaticRules.insert( std::make_unique<SpecializedRule>(Opcode::CallStatic,  1, 0)); //TODO how to specify arguments?
//
//
//
//    //--------------------------------------------------------------------
//    all.emplace(Opcode::ADD, std::move(addRules) );
//    all.emplace(Opcode::SUB, std::move(subRules) );
//    all.emplace(Opcode::MOD, std::move(modRules) );
//    all.emplace(Opcode::MUL, std::move(mulRules) );
//    all.emplace(Opcode::DIV, std::move(divRules) );
//    all.emplace(Opcode::AND, std::move(andRules) );
//    all.emplace(Opcode::OR, std::move(orRules) );
//    all.emplace(Opcode::XOR, std::move(xorRules) );
//    all.emplace(Opcode::NEQ, std::move(neqRules) );
//    all.emplace(Opcode::EQ, std::move(eqRules) );
//    all.emplace(Opcode::LTE, std::move(lteRules) );
//    all.emplace(Opcode::LT, std::move(ltRules) );
//    all.emplace(Opcode::GTE, std::move(gteRules) );
//    all.emplace(Opcode::GT, std::move(gtRules) );
//    all.emplace(Opcode::LSH, std::move(lshRules) );
//    all.emplace(Opcode::RSH, std::move(rshRules) );
////    all.emplace(Opcode::BinOp, std::move(binRules) );
//
//    all.emplace(Opcode::UnOp,std::move(unRules) );
//    all.emplace(Opcode::AllocG,std::move(allocGRules) );
//    all.emplace(Opcode::AllocL,std::move(allocRules) );
//    all.emplace(Opcode::LoadImm,std::move(valueRules) );
//    all.emplace(Opcode::Store,std::move(storeRules) );
//    all.emplace(Opcode::Load,std::move(loadRules) );
//    all.emplace(Opcode::Return,std::move(returnRules) );
//    all.emplace(Opcode::Call,std::move(callRules) );
//    all.emplace(Opcode::CallStatic,std::move(callStaticRules) );



    return std::move(all);
}
}
