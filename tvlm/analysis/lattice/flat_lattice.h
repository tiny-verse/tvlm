#pragma once

#include <any>
#include "lattice.h"


namespace tvlm{
    template<typename A>
    class FlatElem {
    public:
        virtual FlatElem<A>* copy() = 0;
    };



    class FlatTop : public FlatElem<std::any> {
    public:
        FlatElem<std::any> * copy()override{
            return new FlatTop;
        }
    };


    class FlatBot : public FlatElem<std::any> {
    public:
        FlatElem<std::any> * copy()override{
            return new FlatBot;
        }
    };

        template<typename A>
    class FlatVal : public FlatElem<A> {
    public:
           FlatVal(A & elem) : elem_(elem){
           }
        FlatElem<std::any> * copy()override{
            return new FlatVal<A>(elem_);
        }
    private:
        A elem_;
    };

    template<typename A>
    class FlatLattice : public Lattice<FlatElem<A>*>{
        FlatElem<A> * bot() override{
            return new FlatBot();
        }
        FlatElem<A> * top() override{
            return new FlatTop();
        }

        FlatElem<A> * lub(FlatElem<A> * x, FlatElem<A> * y) override{
            if(dynamic_cast<FlatBot*>(x) ){
                return y->copy();
            }else if ( dynamic_cast<FlatBot*>(y) || *x == *y){
                return x->copy();
            }else if (dynamic_cast<FlatTop*>(x)|| (dynamic_cast<FlatTop*>(y)) ){
                return new FlatTop;
            }else if (  *x == *y){
                return x->copy();
            }else{
                return new FlatTop;
            }
        }

        FlatElem<A> * wrap(A & v){
            return new FlatVal<A>(v);
        }

    };


}
