#pragma once

#include <map>
#include "tvlm/tvlm/analysis/lattice/lattice.h"

template<typename A, typename B>
class MAP : public std::map<A, B>{
public:
    explicit MAP():
    std::map<A, B>(),
    specified(false){}

    MAP( B defaultVal )
    : std::map<A, B>(),
    defaultValue(defaultVal),
    specified(true){

    }

    MAP<A, B> & withDefault(const B & defaultVal){
        defaultValue = defaultVal;
        specified = true;
        return *this;
    }

    virtual B & access (const A & idx){
        if( !specified || std::map<A, B>::find(idx) != std::map<A, B>::end()) {
            return this->operator[](idx);
//            return std::map<A, B>::operator[](idx);
        }
        return defaultValue;

    };
private:
    B defaultValue;
    bool specified;
};


namespace tvlm{


    template<typename A, typename B>
    class MapLattice : public Lattice<MAP<A, B>>{
    public:
        explicit MapLattice(const std::unordered_set<A> & set, Lattice<B> * lat ):
        set_(set), lat_(lat){}

        virtual MAP<A, B> top() override{
            return MAP<A, B>().withDefault(lat_->top());
        }

        virtual MAP<A, B> bot() override{
            return MAP<A, B>().withDefault(lat_->bot());
        }

        virtual MAP<A, B> lub( MAP<A, B> & x, MAP<A, B> & y) override{
            MAP<A, B> res = y;
            for (auto & xx: x) {
                auto & e = xx.first;
                res.insert(std::make_pair(e, lat_->lub(x[e], y[e])));
            }
            return res;
        }

    private:
        std::unordered_set<A> set_;
        Lattice<B>* lat_;
    };
}
