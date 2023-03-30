#include "ColoringAllocator.h"

namespace tvlm {

    ColoringAllocator::VirtualRegister ColoringAllocator::getRegToSpill() {
        VirtualRegister res = regQueue_.front();
        regQueue_.erase(regQueue_.begin());
        return res;
    }

    ColoringAllocator::VirtualRegister ColoringAllocator::getReg(Instruction *ins) {
        VirtualRegister res = VirtualRegister(RegisterType::INTEGER, 0);
        auto it = colorPickingResult_.find(ins);
        if(it != colorPickingResult_.end()){
            auto knownMapping = finalMapping_.find(it->second);
            if(knownMapping != finalMapping_.end()){
                res = knownMapping->second;
            }else{
                VirtualRegister tmp = SuperNaiveRegisterAllocator::getReg(ins);
                finalMapping_.emplace(it->second, tmp);
                res = tmp;
            }

            bool global = false;
            if(*ins->name().begin() == 'g'){global = true;}
            ins->setAllocName(generateInstrName(res, global));

            return res;
        }else if (ins->usages().empty()){
            // instruction is not used -- takes only free register overwrites and will be overwritten
            if(freeReg_.empty()){
                res = *freeReg_.begin();
                freeReg_.erase(freeReg_.begin());
            }
            regQueue_.push_back(res);

            bool global = false;
            if(*ins->name().begin() == 'g'){global = true;}
            ins->setAllocName(generateInstrName(res, global));

            return res;
        }
        if(dynamic_cast<const AllocG*>(ins) || dynamic_cast<const AllocL*>(ins)){
            return SuperNaiveRegisterAllocator::getReg(ins);
        }
        return res;
    }

    ColoringAllocator::VirtualRegister ColoringAllocator::getFReg(Instruction *ins) {
        VirtualRegister res = VirtualRegister(RegisterType::FLOAT, 0);
        auto it = colorPickingResult_.find(ins);
        if(it != colorPickingResult_.end()){
            auto knownMapping = finalFMapping_.find(it->second);
            if(knownMapping != finalFMapping_.end()){
                res = knownMapping->second;
            }else{
                VirtualRegister tmp = SuperNaiveRegisterAllocator::getFReg(ins);
                finalFMapping_.emplace(it->second, tmp);
                res = tmp;
            }

            bool global = false;
            if(*ins->name().begin() == 'g'){ global = true;}
            ins->setAllocName(generateInstrName(res, global));

            return res;
        }
        throw "[Coloring Allocator] cannot find instruction in results for float";
    }

    bool ColoringAllocator::setColors() {

        while(!colorPickingStack_.empty()){
            auto pos = colorPickingStack_.top();colorPickingStack_.pop();
            size_t availableColor = 1;
            while(true){
                bool OK = true;//reset
                for(auto neighPos : LRincidence_[pos]){
                    auto it = colorPickingSemiResult_.find(neighPos);
                    if(  it != colorPickingSemiResult_.end() &&
                        it->second == availableColor){
                        OK = false;
                        break;
                    }
                }
                if(OK == true){
                    break;
                }else{
                    availableColor++;
                }
            }

            for (auto instr : searchInstrs_[pos]) {
                colorPickingResult_.emplace(instr, availableColor);
            }
            colorPickingSemiResult_.emplace(pos, availableColor);
        }
    return true;
    }

    void ColoringAllocator::getLRBundles() {

    }

    bool ColoringAllocator::generateLiveRanges() {
        //return true <=> run successful no more repeats needed
        this->spillIndexes_.clear();
        this->LRincidence_.clear();
        std::vector<std::set<size_t>> LRincidence;
        this->colorPickingStack_ = std::stack<size_t>();

        size_t colors = this->freeReg_.size() ;
        if (colors < 3 ){
            throw "[Coloring Allocator] Cannot do reg allocator -> too few registers";
        }
        programChanged_ = false;

        LRincidence_.resize(liveRanges_.size());
        LRincidence.resize(liveRanges_.size());
        for (auto & t : analysisResult_) {
            if (auto ins = dynamic_cast<tvlm::Instruction *>(t.first->il())) {
                auto lr1Pos = searchRanges_.find(ins);
                auto hasAllocation = TargetProgramFriend::getAllocatedRegisters(&targetProgram_).find(ins);
                bool hasRegister = hasAllocation !=  TargetProgramFriend::getAllocatedRegisters(&targetProgram_).end();
                if(hasRegister && !ins->usages().empty()){
                    if(lr1Pos == searchRanges_.end()){
                        throw "[Coloring Allocator.cpp] cannot find Range for instruction";
                    }
                    const std::unique_ptr<CLiveRange> & lr1 = liveRanges_.at(lr1Pos->second);
                    std::vector<CLiveRange *> tmpVector(t.second.begin(), t.second.end());
                    for (auto & lr2 : tmpVector) {
                        bool sameType = false;
                        if(lr2->type() == lr1->type()){
                            sameType = true;
                        }
                        if(sameType){

                            auto lr2Pos = lrIndex_.find(lr2);
                            if (lr2Pos == lrIndex_.end()) {
                                throw CSTR(
                                        "[Coloring Allocator.cpp] cannot find index for liveRange -- probably not registered "
                                                << lr2->il().size() << " with " << lr2->start());
                            }
                            LRincidence_[lr1Pos->second].emplace(lr2Pos->second);
                            LRincidence[lr1Pos->second].emplace(lr2Pos->second);
                            LRincidence_[lr2Pos->second].emplace(lr1Pos->second);
                            LRincidence[lr2Pos->second].emplace(lr1Pos->second);

                        }
                    }
                }else if(ins->usages().empty()){
                    unusedInstructions_.emplace(ins);
                }
            }
        }

        //**********************************************************************************
        //algo colorPicking

            for (size_t i = 0; i < LRincidence.size(); i++) {
                if(LRincidence[i].size() < colors ){
                    //remove from incidence and push to stack
                    colorPickingStack_.push(i);
                    //remove
                    LRincidence[i].clear();

                }
            }

            bool anyNonEmpty = !std::all_of(LRincidence.begin(), LRincidence.end(), [](const std::set<size_t>& input){
                        return input.empty();
                    });
            if(anyNonEmpty)
            {
                size_t selected = 0;
                size_t max = 0;
                for( size_t i = 0;i < LRincidence.size();i++){
                        if(LRincidence[i].size() > max){// spill the node with the biggest degree
                            selected = i;
                            max = LRincidence[i].size();

                        }
                }
                //add NonEmpty with biggest degree to spill stack and remove from Graph

                // ***********************  spilling *******************************
                programChanged_ = true;
                for (auto instrToSpill : searchInstrs_[selected]) {

                    Instruction *instr = dynamic_cast<Instruction *>(instrToSpill);
                    Instruction *alloc;
                    if (instr == nullptr) {
                        throw "[ColoringAllocator] - generateLiveRanges - trying to spill Function/BB ...";
                    }
                    //spill instruction instr
                    BasicBlock *bb = instr->getParentBB();

                    if (auto load = dynamic_cast<Load *>(instr)) {
                        switch (instr->resultType()) {
                            case ResultType::Double: {
                                //spilling IntegerTypeRegister
                                alloc = load->address();
                                alloc->setName(STR("sp. " << instr->name() << ":F"));

                                for (auto *u: instr->usages()) {
                                    BasicBlock *bbu = u->getParentBB();
                                    Instruction *newLoad = bbu->injectBefore(
                                            new Load(alloc, load->resultType(), load->ast()), u);
                                    u->replaceWith(load, newLoad);
                                    newLoad->setName(STR(newLoad->name() << " by " << instr->name() << ":F"));

                                }
                                bb->removeInstr(load);

                                break;
                            }
                            case ResultType::Integer:
                            {

                                //spilling IntegerTypeRegister
                                alloc = load->address();
                                alloc->setName(STR("sp. " << instr->name()));
                                for (auto *u: instr->usages()) {
                                    BasicBlock *bbu = u->getParentBB();
                                    Instruction *newLoad = bbu->injectBefore(
                                            new Load(alloc, instr->resultType(), instr->ast()), u);
                                    u->replaceWith(instr, newLoad);
                                    newLoad->setName(STR(newLoad->name() << " by " << instr->name()));
                                }

                                bb->removeInstr(load);

                                break;
                            }
                            default:
                                throw "[Coloring Allocator] wrong case of ResultType for spilling";
                                break;
                        }
                    } else {
                        switch (instr->resultType()) {
                            case ResultType::Double: {
                                alloc = bb->inject(new AllocL(Type::Double().size(), instr->ast()));//begining of BB
                                alloc->setName(STR("inject " << instr->name() << ":F"));

                                for (auto *u: instr->usages()) {
                                    BasicBlock *bbu = u->getParentBB();
                                    Instruction *newLoad = bbu->injectBefore(
                                            new Load(alloc, instr->resultType(), instr->ast()), u);
                                    u->replaceWith(instr, newLoad);
                                    newLoad->setName(STR(newLoad->name() << " by " << instr->name() << ":F"));
                                }

                                auto *nStore = bb->injectAfter(new Store(instr, alloc, instr->ast()), instr);
                                nStore->setName(STR(nStore->name() << " by " << instr->name() << ":F"));

                                break;
                            }

                            case ResultType::Integer:
                            {
                                //spilling IntegerTypeRegister
                                alloc = bb->inject(new AllocL(Type::Integer().size(), instr->ast()));//begining of BB
                                alloc->setName(STR("inject " << instr->name()));

                                for (auto *u: instr->usages()) {
                                    BasicBlock *bbu = u->getParentBB();
                                    Instruction *newLoad = bbu->injectBefore(
                                            new Load(alloc, instr->resultType(), instr->ast()), u);
                                    u->replaceWith(instr, newLoad);
                                    newLoad->setName(STR(newLoad->name() << " by " << instr->name()));

                                }

                                auto *nStore = bb->injectAfter(new Store(instr, alloc, instr->ast()), instr);
                                nStore->setName(STR(nStore->name() << " by " << instr->name()));
                                alloc->registerUsage(nStore);
                                break;
                            }
                            default:
                                throw "[Coloring Allocator] wrong case of ResultType for spilling";
                                break;
                        }
                    }
                }

                //remove from graph
                for (auto & k : LRincidence) {
                    k.erase(selected);
                }
                LRincidence[selected].clear();

                return !(programChanged_ = true);
            }else {
                return !(programChanged_);
            }

        //fill colorPicking result

    }

    void ColoringAllocator::addLR(std::unique_ptr<CLiveRange> && lr) {
        for(auto il : lr->il()){
            if(auto ins = dynamic_cast<Instruction *>(il)) {
                searchRanges_.emplace(ins, liveRanges_.size());
                searchInstrs_[liveRanges_.size()].emplace( ins);

            }
        }
        lrIndex_.emplace(lr.get(), liveRanges_.size());
        liveRanges_.push_back(std::move(lr));
    }

    void ColoringAllocator::callingConvCallerSave(const Instruction *currIns) {

        auto cfgs = la_->instr_mapping().find(currIns);
        if(cfgs == la_->instr_mapping().end()){
            throw "[ColoringAllocator] cannot find Call instr in analysis";
        }
        auto toSpill = analysisResult_.find(cfgs->second);
        if(toSpill == analysisResult_.end()){
            throw "[ColoringAllocator] cannot find Call instr in analysis results";
        }
        for (auto & lr : toSpill->second) {
            for (auto * il : lr->il()) {
                if(auto * instr = dynamic_cast<Instruction *>(il)){

                    auto loc = addressDescriptor_.find(instr);
                    if(loc != addressDescriptor_.end()){
                        size_t i = 0;
                        for (auto l = loc->second.begin() ;l != loc->second.end() && i < loc->second.size();l++,i++) {
                            if(l->loc() == Location::Register){
                                this->spill(l->regIndex(),currIns);
                            }
                        }
                    }
                }else{
                    // we don't care we want to spill from register
                }
            }
        }
    }

    void ColoringAllocator::resetFreeRegs(const Instruction *except) {
        finalMapping_.clear();
        finalFMapping_.clear();
        return RegisterAllocator::resetFreeRegs(except);
    }
}