#include "ColoringAllocator.h"



namespace tvlm {
//
//    Register ColoringAllocator::getRegOutro(const Register & reg, const Instruction * ins){
//        register_descriptor_[reg].insert(ins);
//        address_descriptor_[ins].emplace( LocationEntry(Location::Register, reg.index()) );
//        return Register(reg);
//    }
//
//    Register ColoringAllocator::pickForSpill(const Instruction *ins) {
//        //TODO
//        return Register(0);
//    }
//
//    void ColoringAllocator::spillIntReg(const Instruction * ins){
//        //TODO
//        Register regToSpill = pickForSpill(ins);
//        //inject code?
//        address_descriptor_[ins].erase(LocationEntry(Location::Register, regToSpill.index()));
//        //
//
//    }
//
//    Register ColoringAllocator::getReg(const Instruction *ins) {
//        auto it = address_descriptor_.find(ins);
//        if(it == address_descriptor_.end()){
//
//        }else {
//            auto places = it->second;
//            auto reg = places.find(LocationEntry(Location::Register, 0));
//            if(reg != places.end()){
//                Register res = Register(reg->regIndex());
//                return getRegOutro(res, ins);
//            }else if (auto stack = places.find(LocationEntry(Location::Stack, 0)) != places.end()){
//            //spilled
//
//            //getFreeRegister
//            bool freeReg;
//            if(freeReg){
//                return getRegOutro(Register(freeReg), ins);//TODO
//            }else{
//                spillIntReg(ins);
//
//
//            }
//            }else if (auto mem = places.find(LocationEntry(Location::Memory, 0)) != places.end()){
//            //(global) variable
//
//
//            }else{
//                throw "invalid address descriptor!";
//            }
//        }
//        return tvlm::Register(0);
//    }
//
//    FRegister ColoringAllocator::getFloatReg(const Instruction *ins) {
//        return tvlm::FRegister(0);
//    }
//
//    void ColoringAllocator::clearInt(const Instruction *ins) {
//
//    }
//
//    void ColoringAllocator::clearFloat(const Instruction *ins) {
//
//    }
//
//    void ColoringAllocator::spillCallReg() {
//
//    }
//
//    void ColoringAllocator::clearAllReg() {
//
//    }
//
//    void ColoringAllocator::spillAllReg() {
//
//    }
//
//    void ColoringAllocator::prepareReturnValue(size_t size, const Instruction * ret) {
//
//    }
//
//    void ColoringAllocator::makeLocalAllocation(size_t size, const Register &reg, const Instruction * ins) {
//
//    }
//
//    void ColoringAllocator::allocateStructArg(const Type *type, const Instruction *ins) {
//
//    }
//
//    void ColoringAllocator::resetAllocSize() {
//
//    }
//
//    void ColoringAllocator::correctStackAlloc(size_t patch) {
//
//    }
//
//    Register ColoringAllocator::getIntRegister(const Instruction *ins) {
//        return tvlm::Register(0);
//    }
//
//    FRegister ColoringAllocator::getFloatRegister(const Instruction *ins) {
//        return tvlm::FRegister(0);
//    }
//
//    Register ColoringAllocator::getFreeIntRegister() {
//        return tvlm::Register(0);
//    }
//
//    FRegister ColoringAllocator::getFreeFloatRegister() {
//        return tvlm::FRegister(0);
//    }
//
//    bool ColoringAllocator::isInsInMem(const Instruction *ins) const {
//        auto tmp = address_descriptor_.at(ins);
//        for (auto & record : tmp) {
//            if(record.loc() == Location::Memory){
//                return true;
//            }
//        }
//        return false;
//    }
//
//    bool ColoringAllocator::isInsAtStack(const Instruction *ins) const {
//        return false;
//    }
//
//    bool ColoringAllocator::isInsInRegister(const Instruction *ins) const {
//        return false;
//    }

    ColoringAllocator::VirtualRegister ColoringAllocator::getRegToSpill() {
        VirtualRegister res = regQueue_.front();
        regQueue_.erase(regQueue_.begin());
        return res;
    }



    ColoringAllocator::VirtualRegister ColoringAllocator::getReg(const Instruction *currentIns) {
        VirtualRegister res = VirtualRegister(RegisterType::INTEGER, 0);
        auto it = colorPickingResult_.find(currentIns);
        if(it != colorPickingResult_.end()){
            return VirtualRegister(RegisterType::INTEGER, it->second);
        }
        throw "[Coloring Allocator] cannot find instruction in results";
    }
//
//    RegisterAllocator::VirtualRegister ColoringAllocator::getFReg(const Instruction *currentIns) {
//        return tvlm::RegisterAllocator::VirtualRegister(RegisterType::FLOAT, 0);
//    }
//
//    RegisterAllocator::VirtualRegister ColoringAllocator::getLastRegister(const Instruction *currentIns) {
//        return tvlm::RegisterAllocator::VirtualRegister(RegisterType::INTEGER, 0);
//    }

    bool ColoringAllocator::setColors() {

//        std::queue<size_t> Q;
//        while(!Q.empty()){
//            auto pos = Q.front();Q.pop();
//            size_t availableColor = 1;
//
//
//
//            colorPickingResult_.emplace(searchInstrs_[pos], )
//
//        }
//
        bool twoAddress = true;
        while(!colorPickingStack_.empty()){
            auto pos = colorPickingStack_.top();colorPickingStack_.pop();
            size_t availableColor = 1;
            while(true){
                bool OK = true;//reset
                for(auto neighPos : LRincidence_[pos]){
                    auto it = colorPickingSemiResult_.find(neighPos);
//                    if(twoAddress){
//                        auto instr = searchInstrs_[pos];
//                        if (auto binOp = dynamic_cast<BinOp*>(instr)){
//                            if(binOp->lhs() == searchInstrs_[neighPos]){
//                                auto res = colorPickingSemiResult_.find(neighPos);
//                                if(res != colorPickingSemiResult_.end()){
//                                    OK = true;
//                                    availableColor = res->second;
//                                    break;
//                                }
//                            }
//                        }else if (auto unOp = dynamic_cast<UnOp*>(instr)){
//                            if(unOp->operand() == searchInstrs_[neighPos]){
//                                auto res = colorPickingSemiResult_.find(neighPos);
//                                if(res != colorPickingSemiResult_.end()){
//                                    OK = true;
//                                    availableColor = res->second;
//                                    break;
//                                }
//                            }
//                        }else if (auto trunc = dynamic_cast<Truncate*>(instr)){
//                            if(trunc->src() == searchInstrs_[neighPos]){
//                                auto res = colorPickingSemiResult_.find(neighPos);
//                                if(res != colorPickingSemiResult_.end()){
//                                    OK = true;
//                                    availableColor = res->second;
//                                    break;
//                                }
//                            }
//                        }else if (auto extend = dynamic_cast<Extend*>(instr)){
//                            if(extend->src() == searchInstrs_[neighPos]){
//                                auto res = colorPickingSemiResult_.find(neighPos);
//                                if(res != colorPickingSemiResult_.end()){
//                                    OK = true;
//                                    availableColor = res->second;
//                                    break;
//                                }
//                            }
//                        }else if (auto copy = dynamic_cast<Copy*>(instr)){
//                            if(copy->src() == searchInstrs_[neighPos]){
//                                auto res = colorPickingSemiResult_.find(neighPos);
//                                if(res != colorPickingSemiResult_.end()){
//                                    OK = true;
//                                    availableColor = res->second;
//                                    break;
//                                }
//                            }
//                        }else {
//                            //TODO fill other two address insrtuction -- to repeat register as first argument
//                        }
//                    }
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
//        this->liveRanges_.clear();
        this->LRincidence_.clear();
        std::vector<std::set<size_t>> LRincidence;
        this->colorPickingStack_ = std::stack<size_t>();
//        this->colorPickingResult_.clear();

        size_t colors = this->freeReg_.size() -1;
        programChanged_ = false;

//        for (auto & t : analysisResult_) { // step -> for each cfgNode TODO go by cfg_
//
//            std::set<std::pair<LiveRange *, size_t>, LiveRangesComparator> tmp;
//
//            //instruction alone is alive for sure
//            auto * alive = dynamic_cast<ILInstruction * >(t.first->il());
//            if(alive &&alive->resultType() != ResultType::Void ) { //if the node is instruction
//
//                auto it = rangesAlive_.find(alive);
//                if (it == rangesAlive_.end()) {
//                    auto newRange = std::make_unique<LiveRange>(alive, t.first->il());
//                    tmp.emplace(newRange.get(), liveRanges_.size());
//                    addLR(std::move(newRange));
//                } else {
//                    it->first->setEnd(t.first->il());
//                    tmp.insert(*it);
//                }
//            }
//            for( auto * alive_ : t.second ){
//                if(auto * alive = dynamic_cast<ILInstruction * >(alive_->start())){ //if the node is instruction
//                    auto it = rangesAlive_.find(alive);
//                    if(it == rangesAlive_.end()){
//                        auto newRange = std::make_unique<LiveRange>(alive, t.first->il());
//                        tmp.emplace(newRange.get(), liveRanges_.size());
//                        addLR(std::move(newRange));
//                    }else{
//                        it->first->setEnd(t.first->il());
//                        tmp.insert(*it);
//                    }
//                }
//            }
//            rangesAlive_ = std::move(tmp);
//
//            //incidence graph build
//            LRincidence_.resize(liveRanges_.size());
//            LRincidence.resize(liveRanges_.size());
//            for (auto & i : rangesAlive_) {
//                for (auto & j : rangesAlive_) {
//                    if(i.second != j.second ) {
//                        LRincidence_[i.second].emplace(j.second);
//                        LRincidence_[j.second].emplace(i.second);
//                        LRincidence[i.second].emplace(j.second);
//                        LRincidence[j.second].emplace(i.second);
//                    }
//                }
//            }
//
//        }
        LRincidence_.resize(liveRanges_.size());
        LRincidence.resize(liveRanges_.size());
        for (auto & t : analysisResult_) { // step -> for each cfgNode TODO go by cfg_


//
//            if(auto ins = dynamic_cast<tvlm::Instruction*>(t.first->il())){
//                auto lrPos = searchRanges_.find(ins);
//                if(ins->resultType() != ResultType::Void && lrPos == searchRanges_.end()){
//                    throw "[Coloring Allocator.cpp] cannot find Range for instruction";
//                }
//                    for (auto * LR : t.second) {
//                        auto otherPos = lrIndex_.find(LR);
//                        if(otherPos == lrIndex_.end()){
//                            throw "[Coloring Allocator.cpp] cannot find index for liveRange -- probably not registered";
//                        }
//                        LRincidence_[lrPos->second].emplace(otherPos->second);
//                        LRincidence_[otherPos->second].emplace(lrPos->second);
//                        LRincidence[lrPos->second].emplace(otherPos->second);
//                        LRincidence[otherPos->second].emplace(lrPos->second);
//
//                    }
//
//
//            }
            std::vector<CLiveRange*> tmpVector(t.second.begin(), t.second.end());

            for ( size_t i = 0 ; i < tmpVector.size();i++) {
                for (size_t j = i +1 ; j < tmpVector.size();j++){
                    auto lr1 = tmpVector[i];
                    auto lr2 = tmpVector[j];
                    auto lr1Pos = lrIndex_.find(lr2);
                    auto lr2Pos = lrIndex_.find(lr1);
                    if(lr1Pos == lrIndex_.end()){
                        throw STR( "[Coloring Allocator.cpp] cannot find index for liveRange lr1-- probably not registered " << lr1->il().size() << " with " << lr1->start());
                    }
                    if(lr2Pos == lrIndex_.end()){
                        throw STR( "[Coloring Allocator.cpp] cannot find index for liveRange lr2 -- probably not registered " << lr2->il().size() << " with " << lr2->start());
                    }
                    LRincidence_[lr1Pos->second].emplace(lr2Pos->second);
                    LRincidence[lr1Pos->second].emplace(lr2Pos->second);
                    LRincidence_[lr2Pos->second].emplace(lr1Pos->second);
                    LRincidence[lr2Pos->second].emplace(lr1Pos->second);

                }
            }


        }

        this->LRincidence_;
        //**********************************************************************************
        //algo colorPicking
//        bool anyNonEmpty =
//                !std::all_of(LRincidence_.begin(), LRincidence_.end(), [](const std::set<size_t>& input) {
//                            return input.empty();;
//                        });
//        while(anyNonEmpty){


//            for (size_t i = LRincidence_.size()-1; i >=  0; i--) {
            for (size_t i = 0; i < LRincidence.size(); i++) {
                if(LRincidence[i].size() < colors ){
                    //remove from incidence and push to stack
                    colorPickingStack_.push(i);
                    //remove
//                    for (auto & k : LRincidence) {
//                        k.erase(i);
//                    }
                    LRincidence[i].clear();
//                    colors--;
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
                            if(max >  colors ){
                                break;
                            }
                        }
                }
                //add NonEmpty with biggest degree to spill stack and remove from Graph
//                spillIndexes_.emplace( selected, 0); // TODO find spill place
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

                    switch (instr->resultType()) {

                        case ResultType::Double:
                        case ResultType::Integer:
                        case ResultType::StructAddress: {

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


                            break;
                        }
                        default:
                            throw "[Coloring Allocator] wrong case of ResultType for spilling";
                            break;
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
}