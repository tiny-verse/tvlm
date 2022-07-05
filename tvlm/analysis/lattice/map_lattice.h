#pragma once

#include <map>
#include <unordered_set>
#include "tvlm/tvlm/analysis/lattice/lattice.h"
#include "flat_lattice.h"

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
    MAP( B && defaultVal )
    : std::map<A, B>(),
    defaultValue(std::move(defaultVal)),
    specified(true){

    }
    MAP( MAP && other) :registered_(std::move(other.registered_)),
        specified(other.specified), defaultValue(std::move(other.defaultValue)){

    }
    MAP( const MAP & other) :registered_(other.registered_),
        specified(other.specified), defaultValue(other.defaultValue){

    }

    virtual ~MAP(){
        for (auto & b: registered_) {
            if constexpr(std::is_pointer<B>::value){
                delete b;
            }
        }
        registered_.clear();
    }
    MAP operator= (const MAP & other){
        this->~MAP();
//        MAP tmp {other};
//        std::swap(tmp, *this);
        specified = other.specified;
        defaultValue = other.defaultValue;
        registered_ = other.registered_;
        return *this;
    }
    MAP operator= (MAP && other){
        this->~MAP();
//        MAP tmp {other};
//        std::swap(tmp, *this);
        specified = other.specified;
        defaultValue = std::move(other.defaultValue);
        registered_ = std::move(other.registered_);
        return *this;
    }

    MAP<A, B> & withDefault(const B & defaultVal){
        defaultValue = defaultVal;
        specified = true;
        return *this;
    }
    MAP<A, B> & withDefault(B && defaultVal){
        defaultValue = std::move(defaultVal);
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
    virtual B const & access (const A & idx)const{
        if( !specified || std::map<A, B>::find(idx) != std::map<A, B>::end()) {
            return this->at(idx);
//            return std::map<A, B>::operator[](idx);
        }
        return defaultValue;

    };
//    virtual B makeVal(B val) {
//        storage_.push_back(val);
//        return val;
//    }

    void reg( B pVal);

private:
    B defaultValue;
    bool specified;
    std::vector<B> registered_;
//    std::vector<std::unique_ptr<B>> storage_;
};

template<typename A, typename B>
void MAP<A, B>::reg( B pVal) {
    registered_.push_back(pVal);
}


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

        virtual MAP<A, B> lub( const MAP<A, B> & x, const MAP<A, B> & y) override{
            MAP<A, B> res = y;
            for (auto & xx: x) {
                auto & e = xx.first;
                res.insert(std::make_pair(e, lat_->lub(x.access(e), y.access(e))));
            }
            return res;
        }

    private:
        std::unordered_set<A> set_;
        Lattice<B>* lat_;
    };
}
