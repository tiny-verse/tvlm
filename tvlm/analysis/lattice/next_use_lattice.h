#pragma once

#include <unordered_set>
#include "tvlm/tvlm/analysis/lattice/lattice.h"
#include "tvlm/tvlm/analysis/lattice/map_lattice.h"
#include "tvlm/tvlm/analysis/lattice/flat_lattice.h"
#include "tvlm/analysis/instruction_analysis.h"


namespace tvlm{


    class NextUseLattice : public MapLattice< Declaration* ,FlatElem<const Declaration*>*>{
    public:
        explicit NextUseLattice(const std::unordered_set<Declaration*> & set, FlatLattice<const Declaration*> * lat ):
                MapLattice< Declaration* ,FlatElem<const Declaration*>*>(set, lat){}

//        MAP<Declaration const, Declaration const> lub(const MAP<Declaration const, Declaration const> &x,
//                                                      const MAP<Declaration const, Declaration const> &y) override {
//          if(dynamic_cast<FlatBot<const Declaration>*>(x) ){
//                return y->copy();
//            }else if ( dynamic_cast<FlatBot<const Declaration>*>(y) ){
//                return x->copy();
//            }else if (dynamic_cast<FlatTop<Declaration>*>(x)|| (dynamic_cast<FlatTop<const Declaration>*>(y)) ){
//                return new FlatTop<const Declaration>;
//            }else if (  *x == y){
//                return x->copy();
//            }else{
//                return new FlatTop<const Declaration>;
//            }
//        }

    public:
        FlatElem<const Declaration*> * makeVal(const Declaration *val){
            auto tmp = new FlatVal<const Declaration *>(val);
            stored_.emplace_back(tmp);
            return tmp;
         }
    private:
        std::vector<std::unique_ptr<FlatVal<const Declaration *>>> stored_;
    };

}
