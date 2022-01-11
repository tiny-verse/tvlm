#pragma once

#include <any>
#include <memory>
#include <vector>
#include "lattice.h"


namespace tvlm{
    template<typename A>
    class FlatElem {
    public:
        FlatElem() = default;
        FlatElem(const FlatElem<A> & other) = delete;
        virtual FlatElem<A>* copy() const = 0;
        virtual bool operator == (const FlatElem<A> * other ) const = 0;
    };


    template<typename A>
    class FlatTop : public FlatElem<A> {
    public:
        FlatTop(): FlatElem<A>(){}
        FlatElem<A> * copy()const override{
            return new FlatTop<A>();
        }

        bool operator==(const FlatElem<A> *other) const override{
            auto val = dynamic_cast<const FlatTop<A>*>(other);
            return val;
        }
    };

    template<typename A>
    class FlatBot : public FlatElem<A> {
    public:
        FlatBot(): FlatElem<A>(){}
        FlatElem<A> * copy()const override{
            return new FlatBot<A>();
        }

        bool operator==(const FlatElem<A> *other) const override {
            auto val = dynamic_cast<const FlatBot<A>*>(other);
            return val;
        }
    };

        template<typename A>
    class FlatVal : public FlatElem<A> {
    public:
           FlatVal(const A & elem) : elem_(elem){
           }
        FlatElem<A> * copy()const override{
            return new FlatVal<A>(elem_);
        }

        bool operator==(const FlatElem<A> * other)  const  override{
            auto val = dynamic_cast<const FlatVal<A>*>(other);
            return val && val->elem_ == elem_;
        }

    private:
        A elem_;
    };

    template<typename A>
    class FlatLattice : public Lattice<FlatElem<A>*>{
    public:
        FlatLattice():bot_(new FlatBot<A>()), top_(new FlatTop<A>()){}
        FlatElem<A> * bot() override{
            return bot_.get();
        }
        FlatElem<A> * top() override{
            return top_.get();
        }

        FlatElem<A> * lub( FlatElem<A> * const&x, FlatElem<A> * const&y) override{
            if(dynamic_cast<FlatBot<A>*>(x) ){
                return add(y->copy());
            }else if ( dynamic_cast<FlatBot<A>*>(y) ){
                return add(x->copy());
            }else if (dynamic_cast<FlatTop<A>*>(x)|| (dynamic_cast<FlatTop<A>*>(y)) ){
                return add(new FlatTop<A>);
            }else if (  *(x) == y){
                return add(x->copy());
            }else{
                return add(new FlatTop<A>);
            }
        }


    protected:
        FlatElem<A> * wrap(A && v){
            return new FlatVal<A>(std::forward<A>(v));
        }

        FlatElem<A> * add(FlatElem<A> * val){
            lubs_.emplace_back(val);
            return val;
        }
        std::unique_ptr<FlatBot<A>> bot_;
        std::unique_ptr<FlatTop<A>> top_;
        std::vector<std::unique_ptr<FlatElem<A>>> lubs_;

    };


}
