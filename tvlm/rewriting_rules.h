#pragma once

#include <unordered_map>
//
//#include "t86_backend.h"
#include "il.h"
//#include "tvlm_backend"

std::unordered_map<tvlm::Instruction *, std::unordered_set<const tvlm::Rule*>> tvlm::ILTiler::AllRulesInit(){
    std::unordered_map<Instruction *, std::unordered_set<const Rule*>> all;
    std::vector<const tvlm::Rule*> children;
    //--------------------------------------------------------------------

    std::unordered_set<const Rule*> binRules;
    binRules.insert( new SpecializedRule<tvlm::BinOp>( 1, 2));
// ADD R1 R2

    children.clear();
    children.push_back(tvlm::DummyRule::get());
    children.push_back(new SpecializedRule<tvlm::LoadImm>(1));
    binRules.insert( new SpecializedRule<tvlm::BinOp>(  1, children));
// ADD R1 I

    //------------------------------------------------------------------
    std::unordered_set<const Rule*> unRules;

    unRules.insert( new SpecializedRule<tvlm::UnOp>(  1, 1));



    //------------------------------------------------------------------
    std::unordered_set<const Rule*> allocRules;

    allocRules.insert( new SpecializedRule<tvlm::Instruction::ImmSize>(  1, 0));




    //------------------------------------------------------------------
    std::unordered_set<const Rule*> allocGRules;

    allocRules.insert( new SpecializedRule<tvlm::AllocG>(  1, 0));






    //------------------------------------------------------------------
    std::unordered_set<const Rule*> valueRules;

    valueRules.insert( new SpecializedRule<tvlm::Instruction::ImmValue>(  1, 0));






    //------------------------------------------------------------------
    std::unordered_set<const Rule*> storeRules;

    storeRules.insert( new SpecializedRule<tvlm::Instruction::StoreAddress>(  1, 2));








    //------------------------------------------------------------------
    std::unordered_set<const Rule*> loadRules;

    loadRules.insert( new SpecializedRule<tvlm::Instruction::LoadAddress>(  1, 1));





    //--------------------------------------------------------------------
    Instruction * dummyLoadImm = new tvlm::LoadImm{(int64_t)0, nullptr};
    Instruction * dummyBinOp = new tvlm::BinOp{tiny::Symbol::Add, tvlm::Instruction::Opcode::ADD,dummyLoadImm, dummyLoadImm, nullptr};
    Instruction * dummyUnOp = new tvlm::UnOp{tiny::Symbol::Sub,tvlm::Instruction::Opcode::UNSUB, dummyLoadImm, nullptr};
    Instruction * dummyAllocL = new tvlm::AllocL{0, nullptr, nullptr};
    Instruction * dummyAllocG = new tvlm::AllocG{0, nullptr, nullptr};
    Instruction * dummyStore = new tvlm::Store{dummyLoadImm, dummyLoadImm, nullptr};
    Instruction * dummyLoad = new tvlm::Load{dummyLoadImm,tvlm::ResultType::Integer, nullptr};
    //--------------------------------------------------------------------
    all.emplace(dummyBinOp,binRules);
    all.emplace(dummyUnOp,unRules);
    all.emplace(dummyAllocG,allocRules);
    all.emplace(dummyAllocL,allocRules);
    all.emplace(dummyLoadImm,valueRules);
    all.emplace(dummyStore,storeRules);
    all.emplace(dummyLoad,loadRules);

    return all;
}
