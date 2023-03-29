#include "FunctionInliner.h"
#include <unordered_set>

namespace tvlm{


    void FunctionInliner::functionInlining(IL &ir) {

        using FncInlineType =  std::function<std::unique_ptr<Instruction> (std::unique_ptr<Instruction> & )>;
        using FncInlineTypes =  std::vector<FncInlineType>;



        size_t  inlining_counter = 0;
        std::unordered_set<Function *> readyToBeInlined;
        for (auto & f : ir.functions()) {
            if (eligibleForInlining(f.second.get())) {
                readyToBeInlined.emplace(f.second.get());
            }
        }
        if(readyToBeInlined.empty()) {return;}

        for (auto &fnc : ir.functions()) {
            auto & fncCallerbbs = getpureFunctionBBs(fnc.second.get());
            for(size_t bbIndex = 0; bbIndex < fncCallerbbs.size();){

                BasicBlock * bbWithCall = fncCallerbbs[bbIndex].get();
                size_t bbWithCallToInlineIndex = bbIndex;
                auto & insns= getpureBBsInstructions(bbWithCall);
                for (auto ins = insns.begin(); ins != insns.end(); ins++) {
                    auto* inlinedCall = dynamic_cast<CallStatic *>(ins->get());
                    if (inlinedCall && readyToBeInlined.find(inlinedCall->f()) != readyToBeInlined.end()) {
//==================================================  decls  ============================================
                        Function *fncToInline = inlinedCall->f();
                        auto & fncToInlinebbs = getpureFunctionBBs(fncToInline);

                        std::vector<BasicBlock*> new_BBs(fncToInlinebbs.size() + fncCallerbbs.size() + 1);
                        std::generate(new_BBs.begin(), new_BBs.end(), []{return new BasicBlock;});

                        std::unordered_map<BasicBlock*, BasicBlock*> bbMapping;
                        std::vector<std::pair<AllocL *, Store*>> argTable;
                        AllocL* returnAlloc = nullptr;
                        BasicBlock* firstHalf;
                        BasicBlock* secondHalf;
                        //cpy phase
                        std::unordered_map<Instruction *, Instruction *> instructionSwapping
                                                                  = std::unordered_map<Instruction *, Instruction *>();
//===============================================  bbMapping set  ====================================
                        //prolog
                        size_t i;
                        for(i = 0; i < bbWithCallToInlineIndex;i++ ){
                            bbMapping.emplace(fncCallerbbs[i].get() , new_BBs[i] );
                            new_BBs[i]->setName(fncCallerbbs[i]->name() );
                        }
                        //1st Half
                        firstHalf = new_BBs[i];
                        bbMapping.emplace(fncCallerbbs[i].get() , new_BBs[i] );
                        new_BBs[i]->setName(STR(bbWithCall->name() << "_1/2_[" << inlining_counter << "]"));
                        i++;

                        //fncInlining
                        for( size_t j = 0; j < fncToInlinebbs.size();i++, j++ ){
                            bbMapping.emplace(fncToInlinebbs[j].get() , new_BBs[i] );
                            new_BBs[i]->setName(STR("inlined_" << inlinedCall->f()->name() <<"[" << inlining_counter << "]_"<< fncToInlinebbs[j]->name()));

                        }

                        //2nd Half
                        secondHalf  = new_BBs[i];
                        //doesn't exist = we are splitting actualBB:
                        //    bbMapping.emplace( fncCallerbbs[i].get() ,new_BBs[i] );
                        new_BBs[i]->setName(STR(bbWithCall->name() << "_2/2_[" << inlining_counter << "]"));
                        i++;

                        //epilog
                        for(size_t j = bbWithCallToInlineIndex +1 ; j < fncCallerbbs.size();i++ , j++){
                            bbMapping.emplace(fncCallerbbs[j].get() , new_BBs[i] );
                            new_BBs[i]->setName(fncCallerbbs[j]->name());
                        }
//=============================================  bbMapping set end  ======================================

//==========================================  argument & return prep  ======================================
                        int c = 0;
                        for (auto &arg: inlinedCall->args()) {
                            ////for (auto &arg: std::reverse(inlinedCall->getArgs().begin(), inlinedCall->getArgs().end())){ //no need of to do
                            arg.first->removeUsage(inlinedCall);
                            auto argAlloc = new AllocL(arg.second->size(), inlinedCall->ast());
                            auto argStore = new Store(arg.first, argAlloc, inlinedCall->ast());
                            argStore->setName(STR("argStore" << c));
                            argAlloc->setName(STR("arg" << c++));

                            argTable.emplace_back(argAlloc, argStore);
                        }
                        if(inlinedCall->resultType() != ResultType::Void){

                            returnAlloc = new AllocL(inlinedCall->f()->getType()->size(), inlinedCall->ast());
                            returnAlloc->setName(STR("ret" << inlining_counter) );
                        }

//=====================================  lambdas: inliningFnc  ======================================
                        FncInlineTypes phaseProlog;
                        FncInlineTypes phase1stHalf;
                        FncInlineTypes phaseInlining;
                        FncInlineTypes phase2ndHalf;
                        FncInlineTypes phaseEpilog;
//=====================================  lambdas: inliningFnc  ======================================
                        FncInlineType
                        jumpReconnection = [ &instructionSwapping, bbMapping](std::unique_ptr<Instruction> & ins){
                            if (auto jmp = dynamic_cast<Jump *>(ins.get())) {

                                auto it = bbMapping.find(jmp->getTarget());
                                if(it == bbMapping.end()){
                                    throw "Invalid Jump copy";
                                }
                                auto thisit = bbMapping.find(ins->getParentBB());
                                if(thisit == bbMapping.end()){
                                    throw "Invalid BB from which Jump copy";
                                }
                                thisit->second->addSucc(it->second);
                                auto njmp = new Jump(it->second, ins->ast());
                                jmp->setName(STR("new_" << ins->name()));

                                return std::unique_ptr<Instruction> (njmp);


                            } else if (auto condJmp = dynamic_cast<CondJump *>(ins.get())) {

                                auto itt = bbMapping.find(condJmp->trueTarget());
                                auto itf = bbMapping.find(condJmp->falseTarget());
                                if(itt == bbMapping.end() || itf == bbMapping.end()){
                                    throw "Invalid CondJump copy";
                                }

                                auto itc = instructionSwapping.find(condJmp->condition());
                                if(itc == instructionSwapping.end() ){
                                    throw "Invalid CondJump copy - copy of cond";
                                }
                                auto thisit = bbMapping.find(ins->getParentBB());
                                if(thisit == bbMapping.end()){
                                    throw "Invalid BB from which CondJump copy#1";
                                }
                                thisit->second->addSucc(itf->second);
                                thisit->second->addSucc(itt->second);
                                auto newCondJump = new CondJump(itc->second, itt->second, itf->second, ins->ast());
                                    newCondJump->setName(STR("new_" << ins->name()));
                                return std::unique_ptr<Instruction>(newCondJump);


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
//                        FncInlineType
//                                beforeInlineInstructionTransforms =[&instructionSwapping, &inlinedCall, returnAlloc ](std::unique_ptr<Instruction> & ins){
//                            if(ins.get() == inlinedCall ){
//
//                                Instruction * tmp = new Load(returnAlloc, inlinedCall->resultType(), ins->ast());
//                                tmp->setName("loadRet");
//                                instructionSwapping.emplace (ins.get(), tmp );
//                                return std::unique_ptr<Instruction>(tmp);
//                            }
//                            return std::unique_ptr<Instruction>();
//                        };
//                        phase1stHalf.emplace_back(beforeInlineInstructionTransforms);

//--------------------------------------------------------------------------------------------------------------------
                        FncInlineType
                        inliningStoreBeforeReturnJump = [ returnAlloc, &instructionSwapping](std::unique_ptr<Instruction> & ins){
                            auto ret =dynamic_cast<Return *>(ins.get());
                            if ( ret && ret->returnValue() ) {
                                auto it = instructionSwapping.find(ret->returnValue());
                                if(it != instructionSwapping.end()){
                                    std::vector<std::unique_ptr<Instruction>> res = std::vector<std::unique_ptr<Instruction>>();

                                    auto returnStore =  new Store( it->second, returnAlloc,  ins->ast());
                                    returnStore->setName("returnStore_inlined" );
                                    return std::unique_ptr<Instruction>( returnStore);
                                }
                                throw "error - cant insert old address to store -when replacing return =: fnc_inlining";
                            } else{
                                return std::unique_ptr<Instruction>();
                            }
                        };
                        phaseInlining.emplace_back(inliningStoreBeforeReturnJump);
//--------------------------------------------------------------------------------------------------------------------
                        FncInlineType
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
                                        auto retjmp = new Jump(secondHalf, ins->ast());
                                        retjmp->setName(STR("retJmp"));
                                return std::unique_ptr<Instruction> (
                                                retjmp);
                            } else if (auto store =dynamic_cast<Store *>(ins.get())) {
                                if(auto argAddr = dynamic_cast<ArgAddr*>(store->address())){
                                    auto it = instructionSwapping.find(store->value() );
                                    auto value = store->value();
                                    if(it != instructionSwapping.end()){
                                        value = it->second;
                                    }
                                    auto argstore = new Store(argTable[argAddr->index()].first, value, ins->ast());
                                    argstore->setName(STR("argStore " << ins->name()));
                                    return std::unique_ptr<Instruction>(argstore);
                                }
                                return std::unique_ptr<Instruction>();
                            }
                            return std::unique_ptr<Instruction> ();
                        };
                        phaseInlining.emplace_back(inliningInstrTransformation);
//--------------------------------------------------------------------------------------------------------------------
                        FncInlineType
                        afterInlineInstructionTransforms =[&instructionSwapping, &inlinedCall, returnAlloc ](std::unique_ptr<Instruction> & ins){
                            if(ins.get() == inlinedCall && inlinedCall->resultType() != ResultType::Void){
                                Instruction * tmp = new Load(returnAlloc, inlinedCall->resultType(), ins->ast());
                                for( auto us : inlinedCall->usages()){
//                                    auto parentBB = us->getParentBB();
                                    us->replaceWith(inlinedCall, tmp);
                                }
                                tmp->setName("loadRet");
                                instructionSwapping.emplace (ins.get(), tmp );
                                return std::unique_ptr<Instruction>(tmp);
                            }else if(ins.get() == inlinedCall && inlinedCall->resultType() == ResultType::Void){
                                Instruction * tmp = new NOPInstruction( ins->ast());

                                tmp->setName("VoidLoadRet");
                                instructionSwapping.emplace (ins.get(), tmp );
                                return std::unique_ptr<Instruction>(tmp);
                            }else{
                                //ignore anything without inlined call
                                return std::unique_ptr<Instruction>();
                            }
                        };

                        phase2ndHalf.emplace_back(afterInlineInstructionTransforms);
//--------------------------------------------------------------------------------------------------------------------

//=====================================  lambda: inliningFnc  ======================================

//=============================================  main work - copy  ======================================
                        //copy until 1st half;
                        for(i = 0; i < bbWithCallToInlineIndex;i++ ){
                            fncCallerbbs[i]->copy(new_BBs[i], instructionSwapping, phaseProlog);
//                            copyInstructions(getpureBBsInstructions(getFunctionBBs(fnc.second.get())[i]).begin(),
//                                            getpureBBsInstructions(getFunctionBBs(fnc.second.get())[i]).end(),
//                                             std::insert_iterator(getpureBBsInstructions(new_BBs[i]), getpureBBsInstructions(new_BBs[i]).begin()),
//                                             instructionSwapping,
//                                             phaseProlog
//                            );
                        }
                        //copy 1st half;

                        bbWithCall->copyUntil(new_BBs[i], instructionSwapping, ins, phase1stHalf);
//                        copyInstructions(getpureBBsInstructions(bbWithCall).begin(),
//                                         ins,
//                                         std::insert_iterator(getpureBBsInstructions(new_BBs[i]), getpureBBsInstructions(new_BBs[i]).begin()),
//                                         instructionSwapping,
//                                         phase1stHalf
//                        );
                        //add args
                        for(size_t a = 0; a < argTable.size();a++){
                            firstHalf->add(argTable[a].first);
                            auto it = instructionSwapping.find(argTable[a].second->value());
                            if(it != instructionSwapping.end()){
//                                argTable[a].second->value() = it->second;
                                auto badStore = argTable[a].second; //pointing to old value
//                                badStore->replaceMe();
                                auto goodStore = new Store(it->second, badStore->address(), badStore->ast());
                                for(auto ch : badStore->children()){
                                    ch->removeUsage(badStore);
                                    ch->registerUsage(goodStore);
                                }

                                argTable[a].second = goodStore;
                                delete badStore;
                                firstHalf->add(goodStore);
                            }else{
                                firstHalf->add(argTable[a].second);
                            }
                        }
                        //return alloc
                        if(inlinedCall->resultType() != ResultType::Void){
                            firstHalf->add(returnAlloc);
                        }
                        firstHalf->add(new Jump(new_BBs[i+1], nullptr));
                        firstHalf->addSucc(new_BBs[i+1]);
                        i++;
                        //copy fncInlining
                        for( size_t j = 0; j < fncToInlinebbs.size();i++, j++ ){

                            fncToInlinebbs[j]->copy(new_BBs[i], instructionSwapping, phaseInlining);
//                            copyInstructions(getpureBBsInstructions(getFunctionBBs(fncToInline)[j]).begin(),
//                                             getpureBBsInstructions(getFunctionBBs(fncToInline)[j]).end(),
//                                             std::insert_iterator(getpureBBsInstructions(new_BBs[i]), getpureBBsInstructions(new_BBs[i]).begin()),
//                                             instructionSwapping,
//                                             phaseInlining
//                            );
                        }
                        //copy 2nd half;

                        bbWithCall->copyFrom(new_BBs[i], instructionSwapping, ins, phase2ndHalf);
//                        copyInstructions(ins,
//                                         getpureBBsInstructions(bbWithCall).end(),
//                                         std::insert_iterator(getpureBBsInstructions(new_BBs[i]), getpureBBsInstructions(new_BBs[i]).begin()),
//                                         instructionSwapping,
//                                         phase2ndHalf
//                        );
                        i++;
                        //rest of "main"fnc
                        for(size_t j = bbWithCallToInlineIndex +1 ; j < fncCallerbbs.size();i++ , j++){

                            fncCallerbbs[j]->copy(new_BBs[i], instructionSwapping, phaseEpilog);
//                            copyInstructions(getpureBBsInstructions(getFunctionBBs( fnc.second.get())[j]).begin(),
//                                             getpureBBsInstructions(getFunctionBBs( fnc.second.get())[j]).end(),
//                                             std::insert_iterator(getpureBBsInstructions(new_BBs[i]), getpureBBsInstructions(new_BBs[i]).begin()),
//                                             instructionSwapping,
//                                             phaseEpilog
//                            );
                        }
//===========================================----main work - copy----====================================
//===========================================  replaceBBs in fnc  ====================================
                        fncCallerbbs.clear();
                        for(i = 0; i < new_BBs.size();i++){
                            fncCallerbbs.emplace_back(new_BBs[i]);
                        }
//=========================================----replaceBBs in fnc----====================================
                        //repair bbs succ and predecessors
                        for ( auto  bb : getFunctionBBs(fnc.second.get())) {
                            auto insns2 = getBBsInstructions(bb);
                            if(insns2.empty()) {continue;}//empty bbs useless - skip
                            auto terminator = insns2.back();
                            auto term = dynamic_cast<Instruction::Terminator*>(terminator);
                            if(!term){
                                throw "[Funciton Inlining] fail, bb without terminator!";
                            }
                            for(auto target : term->allTargets()){

                                bb->addSucc(target);//auto adds predecessor as well


                                target->registerUsage(term); // add usages to BB that it knows who points to them
                            }

                        }


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
        throw "[CpyInstructions]wrong";
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
        if(lambded) {continue;}
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
std::unordered_map<Instruction*, Instruction*> nop = std::unordered_map<Instruction*, Instruction*>();
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
            if(dynamic_cast<Call *>(ins->get())){ //dont know if it is recursive
                return false;
            }
        }
    }

    return fnc->name() != Symbol("main") && getFunctionBBs(fnc).size() < 3;
}




}