#include "FunctionInliner.h"
#include <unordered_set>

namespace tvlm{


    void FunctionInliner::functionInlining(IL &ir) {
        size_t  inlining_counter = 0;
        std::unordered_set<Function *> readyToBeInlined;
        for (auto & f : ir.functions()) {
            if (eligibleForInlining(f.second.get())) {
                readyToBeInlined.emplace(f.second.get());
            }
        }
        if(readyToBeInlined.empty()) {return;}

        for (auto &fnc : ir.functions()) {
            auto & bbs = getpureFunctionBBs(fnc.second.get());
            for(size_t bbIndex = 0; bbIndex < bbs.size();){

                BasicBlock * bbWithCall = bbs[bbIndex].get();
                size_t bbWithCallToInlineIndex = bbIndex;
                auto & insns= getpureBBsInstructions(bbWithCall);
                for (auto ins = insns.begin(); ins != insns.end(); ins++) {
                    auto* inlinedCall = dynamic_cast<CallStatic *>(ins->get());
                    if (inlinedCall && readyToBeInlined.find(inlinedCall->f()) != readyToBeInlined.end()) {
//==================================================  decls  ============================================
                        Function *fncToInline = inlinedCall->f();
                        auto & fncToInlinebbs = getpureFunctionBBs(fncToInline);
                        std::vector<BasicBlock*> new_BBs(fncToInlinebbs.size() + getFunctionBBs(fnc.second.get()).size() + 1);
                        std::generate(new_BBs.begin(), new_BBs.end(), []{return new BasicBlock;});
                        for(size_t bbIndex = 0;  bbIndex +1 < new_BBs.size() ; bbIndex){
                            auto * bb = new_BBs[bbIndex];
                            bb->addSucc(new_BBs[bbIndex-+1]);
                        }
                        std::unordered_map<BasicBlock*, BasicBlock*> bbMapping;
                        std::vector<std::pair<AllocL *, Store*>> argTable;
                        AllocL* returnAlloc;
                        BasicBlock* firstHalf;
                        BasicBlock* secondHalf;
                        //cpy phase
                        std::unordered_map<Instruction *, Instruction *> instructionSwapping
                                                                  = std::unordered_map<Instruction *, Instruction *>();
//===============================================  bbMapping set  ====================================
                        //prolog
                        size_t i;
                        for(i = 0; i < bbWithCallToInlineIndex;i++ ){
                            bbMapping.emplace(getFunctionBBs(fnc.second.get())[i] , new_BBs[i] );
                            new_BBs[i]->setName(getFunctionBBs(fnc.second.get())[i]->name() );
                        }
                        //1st Half
                        firstHalf = new_BBs[i];
                        bbMapping.emplace(getFunctionBBs(fnc.second.get())[i] , new_BBs[i] );
                        new_BBs[i]->setName(bbWithCall->name() + "_1/2_");
                        i++;

                        //fncInlining
                        for( size_t j = 0; j < getFunctionBBs(fncToInline).size();i++, j++ ){
                            bbMapping.emplace(getFunctionBBs(fncToInline)[j] , new_BBs[i] );
                            new_BBs[i]->setName("inlined_" +  getFunctionBBs(fncToInline)[j]->name());

                        }

                        //2nd Half
                        secondHalf  = new_BBs[i];
                        //doesn't exist = we are splitting actualBB:
                        //    bbMapping.emplace( fnc.second->bbs_[i].get() ,new_BBs[i] );
                        new_BBs[i]->setName(bbWithCall->name() + "_2/2_");
                        i++;

                        //epilog
                        auto & bbs = getpureFunctionBBs(fnc.second.get());
                        for(size_t j = bbWithCallToInlineIndex +1 ; j < bbs.size();i++ , j++){
                            bbMapping.emplace(bbs[j].get() , new_BBs[i] );
                            new_BBs[i]->setName(bbs[j]->name());
                        }
//=============================================  bbMapping set end  ======================================

//==========================================  argument & return prep  ======================================
                        int c = 0;
                        for (auto &arg: inlinedCall->args()) {
                            ////for (auto &arg: std::reverse(inlinedCall->getArgs().begin(), inlinedCall->getArgs().end())){ //no need of to do

                            auto argAlloc = new AllocL(arg.second->size(), inlinedCall->ast());
                            auto argStore = new Store(argAlloc, arg.first, inlinedCall->ast());
                            argStore->setName(STR("argStore" << c));
                            argAlloc->setName(STR("arg" << c++));

                            argTable.emplace_back(argAlloc, argStore);
                        }
                        returnAlloc = new AllocL(inlinedCall->f()->getType()->size(), inlinedCall->ast());
                        returnAlloc->setName(STR("ret" << inlining_counter) );

//=====================================  lambdas: inliningFnc  ======================================
                        std::vector<std::function<std::unique_ptr<Instruction>(std::unique_ptr<Instruction> &)>> phaseProlog;
                        std::vector<std::function<std::unique_ptr<Instruction>(std::unique_ptr<Instruction> &)>> phase1stHalf;
                        std::vector<std::function<std::unique_ptr<Instruction>(std::unique_ptr<Instruction> &)>> phaseInlining;
                        std::vector<std::function<std::unique_ptr<Instruction>(std::unique_ptr<Instruction> &)>> phase2ndHalf;
                        std::vector<std::function<std::unique_ptr<Instruction>(std::unique_ptr<Instruction> &)>> phaseEpilog;
//=====================================  lambdas: inliningFnc  ======================================
                        std::function<std::unique_ptr<Instruction> (std::unique_ptr<Instruction> & )>
                        jumpReconnection = [ &instructionSwapping, bbMapping](std::unique_ptr<Instruction> & ins){
                            if (auto jmp = dynamic_cast<Jump *>(ins.get())) {

                                auto it = bbMapping.find(jmp->getTarget(0));
                                if(it == bbMapping.end()){
                                    throw "Invalid Jump copy";
                                }
                                return std::unique_ptr<Instruction> (new Jump(it->second, ins->ast()));


                            } else if (auto condJmp = dynamic_cast<CondJump *>(ins.get())) {

                                auto itt = bbMapping.find(condJmp->trueTarget());
                                auto itf = bbMapping.find(condJmp->falseTarget());
                                if(itt == bbMapping.end() || itf == bbMapping.end()){
                                    throw "Invalid CondJump copy";
                                }
                                Instruction * newCond;
                                auto itc = instructionSwapping.find(condJmp->condition());
                                if(itc == instructionSwapping.end() ){
                                    throw "Invalid CondJump copy - copy of cond";
                                }

                                return std::unique_ptr<Instruction>(new CondJump(itc->second, itt->second, itf->second, ins->ast()));

                            } else if (auto phi = dynamic_cast<Phi *>(ins.get())) {

                                auto newPhi = dynamic_cast<Phi*>(phi->copyWithSwap(instructionSwapping));
                                for(auto cont = newPhi->contents().begin();cont != newPhi->contents().begin();cont++){
                                    std::pair<BasicBlock*, Instruction*> contentsCpy = *cont;
                                    auto itc = bbMapping.find(cont->first);
                                    if(itc == bbMapping.end() ){
                                        throw "Invalid CondJump copy - copy of cond";
                                    }
                                    contentsCpy.first = itc->second;
                                    cont = newPhi->contents().erase(cont);
                                    newPhi->contents().insert(cont, contentsCpy);
                                }

                                return std::unique_ptr<Instruction>(newPhi);

                            }
                            return std::unique_ptr<Instruction> ();
                        };
                        phaseProlog.emplace_back(jumpReconnection);
                        phase1stHalf.emplace_back(jumpReconnection);
                        phaseInlining.emplace_back(jumpReconnection);
                        phase2ndHalf.emplace_back(jumpReconnection);
                        phaseEpilog.emplace_back(jumpReconnection);
//--------------------------------------------------------------------------------------------------------------------
                        std::function<std::unique_ptr<Instruction> (std::unique_ptr<Instruction> &)>
                        inliningStoreBeforeReturnJump = [ returnAlloc, &instructionSwapping, &inlinedCall](std::unique_ptr<Instruction> & ins){
                            if (auto ret =dynamic_cast<Return *>(ins.get())) {
                                auto it = instructionSwapping.find(ret->returnValue());
                                if(it != instructionSwapping.end()){
                                    return std::unique_ptr<Instruction>( new Store( returnAlloc, it->second, ins->ast()));
                                }
                                throw "error - cant insert old address to store -when replacing return =: fnc_inlining";
                            } else{
                                return std::unique_ptr<Instruction>();
                            }
                        };
                        phaseInlining.emplace_back(inliningStoreBeforeReturnJump);
//--------------------------------------------------------------------------------------------------------------------
                        std::function<std::unique_ptr<Instruction> (std::unique_ptr<Instruction> & )>
                        inliningInstrTransformation = [&argTable,  secondHalf, &instructionSwapping, bbMapping](std::unique_ptr<Instruction> & ins){
                            if (dynamic_cast<ArgAddr *>(ins.get())) {
                                return std::unique_ptr<Instruction> (new NOPInstruction(ins->ast()));//no need to do anything
                            } else if (auto load = dynamic_cast<Load *>(ins.get())) {
                                if (auto argAddrIns = dynamic_cast<ArgAddr *>(load->address())) {
                                    auto newLoad = new Load(argTable[argAddrIns->index()].first,
                                                                         load->resultType(), ins->ast());
                                    newLoad->setName(STR("Larg" << argAddrIns->index()));
                                    instructionSwapping.emplace(ins.get(), newLoad);
                                    return std::unique_ptr<Instruction>(newLoad);
                                }
                            } else if (dynamic_cast<Return *>(ins.get())) {
                                //store in another  lambda
                                return std::unique_ptr<Instruction> ( new Jump(secondHalf, ins->ast()) );
                            } else if (auto store =dynamic_cast<Store *>(ins.get())) {
                                if(auto argAddr = dynamic_cast<ArgAddr*>(store->address())){
                                    auto it = instructionSwapping.find(store->value() );
                                    auto value = store->value();
                                    if(it != instructionSwapping.end()){
                                        value = it->second;
                                    }
                                    return std::unique_ptr<Instruction>(new Store(argTable[argAddr->index()].first, value, ins->ast()));
                                }
                                return std::unique_ptr<Instruction>();
                            }
                            return std::unique_ptr<Instruction> ();
                        };
                        phaseInlining.emplace_back(inliningInstrTransformation);
//--------------------------------------------------------------------------------------------------------------------
                        std::function<std::unique_ptr<Instruction> (std::unique_ptr<Instruction> & )>
                        afterInlineInstructionTransforms =[&instructionSwapping, &inlinedCall, returnAlloc ](std::unique_ptr<Instruction> & ins){
                            if(ins.get() == inlinedCall){
                                Instruction * tmp = new Load(returnAlloc, inlinedCall->resultType(), ins->ast());
                                tmp->setName("loadRet");
                                instructionSwapping.emplace (std::make_pair(ins.get(), tmp) );
                                return std::unique_ptr<Instruction>(tmp);
                            }
                            return std::unique_ptr<Instruction>();
                        };

                        phase2ndHalf.emplace_back(afterInlineInstructionTransforms);
//--------------------------------------------------------------------------------------------------------------------

//=====================================  lambda: inliningFnc  ======================================

//=============================================  main work - copy  ======================================
                        //copy until 1st half;
                        for(i = 0; i < bbWithCallToInlineIndex;i++ ){
                            copyInstructions(getpureBBsInstructions(getFunctionBBs(fnc.second.get())[i]).begin(),
                                            getpureBBsInstructions(getFunctionBBs(fnc.second.get())[i]).end(),
                                             std::insert_iterator(getpureBBsInstructions(new_BBs[i]), getpureBBsInstructions(new_BBs[i]).begin()),
                                             instructionSwapping,
                                             phaseProlog
                            );
                        }
                        //copy 1st half;
                        copyInstructions(getpureBBsInstructions(bbWithCall).begin(),
                                         ins,
                                         std::insert_iterator(getpureBBsInstructions(new_BBs[i]), getpureBBsInstructions(new_BBs[i]).begin()),
                                         instructionSwapping,
                                         phase1stHalf
                        );
                        //add args
                        for(size_t a = 0; a < argTable.size();a++){
                            firstHalf->add(argTable[a].first);
                            auto it = instructionSwapping.find(argTable[a].second->value());
                            if(it != instructionSwapping.end()){
//                                argTable[a].second->value() = it->second;
                                auto badStore = argTable[a].second;
                                badStore->replaceMe(new Store(it->second, badStore->address(), badStore->ast()));
                            }
                            firstHalf->add(argTable[a].second);
                        }
                        //return alloc
                        firstHalf->add(returnAlloc);
                        firstHalf->add(new Jump(new_BBs[i+1], nullptr));
                        i++;
                        //copy fncInlining
                        for( size_t j = 0; j < getpureFunctionBBs(fncToInline).size();i++, j++ ){
                            copyInstructions(getpureBBsInstructions(getFunctionBBs(fncToInline)[j]).begin(),
                                             getpureBBsInstructions(getFunctionBBs(fncToInline)[j]).end(),
                                             std::insert_iterator(getpureBBsInstructions(new_BBs[i]), getpureBBsInstructions(new_BBs[i]).begin()),
                                             instructionSwapping,
                                             phaseInlining
                            );
                        }
                        //copy 2nd half;
                        copyInstructions(ins,
                                         getpureBBsInstructions(bbWithCall).end(),
                                         std::insert_iterator(getpureBBsInstructions(new_BBs[i]), getpureBBsInstructions(new_BBs[i]).begin()),
                                         instructionSwapping,
                                         phase2ndHalf
                        );
                        i++;
                        //rest of "main"fnc
                        for(size_t j = bbWithCallToInlineIndex +1 ; j < getFunctionBBs(fnc.second.get()).size();i++ , j++){
                            copyInstructions(getpureBBsInstructions(getFunctionBBs( fnc.second.get())[j]).begin(),
                                             getpureBBsInstructions(getFunctionBBs( fnc.second.get())[j]).end(),
                                             std::insert_iterator(getpureBBsInstructions(new_BBs[i]), getpureBBsInstructions(new_BBs[i]).begin()),
                                             instructionSwapping,
                                             phaseEpilog
                            );
                        }
//===========================================----main work - copy----====================================
//===========================================  replaceBBs in fnc  ====================================
                        getpureFunctionBBs(fnc.second.get()).clear();
                        for(i = 0; i < new_BBs.size();i++){
                            getpureFunctionBBs(fnc.second.get()).emplace_back(new_BBs[i]);
                        }
//=========================================----replaceBBs in fnc----====================================

                        inlining_counter++;
                        break;//inlined ... this bbWithCall is over

                    }

                }
                bbIndex++;
            }
        }
    }

    void FunctionInliner::copyInstructions(std::vector<std::unique_ptr<Instruction>>::iterator beginSource,
    const std::vector<std::unique_ptr<Instruction>>::iterator &endSource,
            std::insert_iterator<std::vector<std::unique_ptr<Instruction>>>  beginDestination,
    std::unordered_map<Instruction *, Instruction *> & swapInstr,
    const std::vector<std::function<std::unique_ptr<Instruction>(std::unique_ptr<Instruction> &)>> &lambdaChange =
            std::vector<std::function<std::unique_ptr<Instruction>(std::unique_ptr<Instruction> &)>>( )
    ){
    for (; beginSource != endSource; beginSource++) {
    std::unique_ptr<Instruction> toReplace;

    bool lambded = false;
    for(auto & l : lambdaChange){
    if( (toReplace = l(*beginSource)) ){
    swapInstr.emplace(beginSource->get(), toReplace.get());
    beginDestination = std::move(toReplace);
    lambded = true;
}
}
if(lambded) continue;
toReplace = std::unique_ptr<Instruction>(beginSource->get()->copyWithSwap(swapInstr));

swapInstr.emplace(beginSource->get(), toReplace.get());
beginDestination = std::move(toReplace);
}
}


void FunctionInliner::copyInstructions(const std::vector<std::unique_ptr<Instruction>>::iterator & beginSource,
const std::vector<std::unique_ptr<Instruction>>::iterator &endSource,
const std::insert_iterator<std::vector<std::unique_ptr<Instruction>>> & beginDestination,
const std::vector<std::function<std::unique_ptr<Instruction>(std::unique_ptr<Instruction> &)>> &lambdaChange =
        std::vector<std::function<std::unique_ptr<Instruction>(std::unique_ptr<Instruction> &)>>( )
){
std::unordered_map<Instruction*, Instruction*> nop;
copyInstructions(beginSource, endSource, beginDestination, nop, lambdaChange);
}


bool FunctionInliner::eligibleForInlining(Function *fnc) {

    //not recursive
    for (auto &bb : getFunctionBBs(fnc)) {
        for (auto ins = getpureBBsInstructions(bb).begin(); ins != getpureBBsInstructions(bb).end(); ins++) {
            auto in = dynamic_cast<CallStatic *>(ins->get());
            if (in && in->f() == fnc) {
                return false;
            }
        }
    }

    return fnc->name() != Symbol("main") && getFunctionBBs(fnc).size() > 1;
}




}