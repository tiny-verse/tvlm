#pragma once

#include <unordered_set>
#include <set>
#include "tvlm/tvlm/analysis/lattice/lattice.h"
#include "tvlm/tvlm/analysis/lattice/flat_lattice.h"
#include "tvlm/tvlm/analysis/lattice/map_lattice.h"
#include "tvlm/tvlm/il/il.h"


namespace tvlm{

    class Constant{
    public:
        Constant(int64_t val):intVal_(val), floatValue_(0), resType_(ResultType::Integer){}
        Constant(double val):intVal_(0), floatValue_(val), resType_(ResultType::Double){}
        int64_t getInt() const {
            if(resType_ == ResultType::Integer){
                return intVal_;
            }else{
                throw "bad type";
            }
        }
        double getFloat() const {
            if(resType_ == ResultType::Double){
                return floatValue_;
            }else{
                throw "bad type";
            }
        }
        ResultType resType()const{
            return resType_;
        }
        bool operator==(const Constant & other ) const {
            return resType_ == other.resType_ && intVal_ == other.intVal_ && floatValue_ == other.floatValue_;
        }
        bool operator!=(const Constant & other ) const {
            return ! operator==(other);
        }
        friend
        bool equals(const Constant * x, const Constant * y){
            return x->operator==(*y);
        }
    private:
        int64_t intVal_;
        double floatValue_;
        ResultType resType_;
    };
    using  Const = FlatElem<Constant>;

    
    class ConstantPropagationLattice : public FlatLattice<Constant>{
    public:
        explicit ConstantPropagationLattice( )
        {}


        Const * num(Constant c) {
            return add(new FlatVal(c));
        }

#define ROUTINE( operator) \
        if( dynamic_cast<FlatBot<Constant>*>(a)){                       \
            return a;                                                   \
        }else if ( dynamic_cast<FlatBot<Constant>*>(b)){                \
            return b;                                                   \
        }else if ( dynamic_cast<FlatTop<Constant>*>(a)){                \
            return a;                                                   \
        }else if ( dynamic_cast<FlatTop<Constant>*>(b)){                \
            return b;                                                   \
        }else{                                                          \
            auto aa = a->get();                                         \
            auto bb = b->get();                                         \
            if(aa.resType() == ResultType::Integer){                  \
                if(bb.resType() == ResultType::Integer ){             \
                    return num ( Constant( (int64_t)(aa.getInt()   operator bb.getInt() ))   );     \
                }else{                                                  \
                    return num ( Constant( (double)((double)aa.getInt() operator bb.getFloat() ))   );     \
                }                                                       \
            }else{                                                      \
                if(bb.resType() == ResultType::Integer ){             \
                    return num ( Constant( (double)(aa.getFloat() operator (double)bb.getInt() ))   );     \
                }else{                                                  \
                    return num ( Constant( (double)(aa.getFloat() operator bb.getFloat() ))   );   \
                }                                                       \
            }                                                           \
                                                                        \
        }                                                               \


#define BITROUTINE( operator) \
        if( dynamic_cast<FlatBot<Constant>*>(a)){                       \
            return a;                                                   \
        }else if ( dynamic_cast<FlatBot<Constant>*>(b)){                \
            return b;                                                   \
        }else if ( dynamic_cast<FlatTop<Constant>*>(a)){                \
            return a;                                                   \
        }else if ( dynamic_cast<FlatTop<Constant>*>(b)){                \
            return b;                                                   \
        }else{                                                          \
            auto aa = a->get();                                         \
            auto bb = b->get();                                         \
            if(aa.resType() == ResultType::Integer &&                   \
                   bb.resType() == ResultType::Integer ){               \
                    return num ( Constant( (int64_t)(aa.getInt()   operator bb.getInt() ))   );     \
            }else {                                                      \
                return bot();                                           \
                }                                                       \
                                                                       \
                                                                        \
        }                                                               \


        Const * times(Const * a, Const * b) {
            ROUTINE(*)
        }
        Const * plus(Const * a, Const * b) {
            ROUTINE(+)
        }
        Const * minus(Const * a, Const * b) {
            ROUTINE(-)
        }
        Const * div(Const * a, Const * b) {
            if( dynamic_cast<FlatBot<Constant>*>(a)){
                    return a;
                }else if ( dynamic_cast<FlatBot<Constant>*>(b)){
                    return b;
                }else if ( dynamic_cast<FlatTop<Constant>*>(a)){
                    return a;
                }else if ( dynamic_cast<FlatTop<Constant>*>(b)){
                    return b;
                }else {
                    auto aa = a->get();
                    auto bb = b->get();
                    if (aa.resType() == ResultType::Integer) {
                        if (bb.resType() == ResultType::Integer) {
                            if(bb.getInt() == 0){
                                return bot();
                            }
                            return num ( Constant((int64_t) (aa.getInt()   / bb.getInt())) ) ;
                        } else {
                            if(bb.getFloat() == 0){
                                return bot();
                            }
                            return num ( Constant((double) ((double)aa.getInt()  / bb.getFloat())) ) ;
                        }
                    } else {
                        if (bb.resType() == ResultType::Integer) {
                            if(bb.getInt() == 0){
                                return bot();
                            }
                            return num ( Constant((double) (aa.getFloat()  / (double)bb.getInt())) ) ;
                        } else {
                            if(bb.getFloat() == 0){
                                return bot();
                            }
                            return num ( Constant((double) (aa.getFloat()  / bb.getFloat())) ) ;
                        }
                    }
            }

        }
        Const * phi(Const * a, Const * b) {
            if( dynamic_cast<FlatBot<Constant>*>(a)){
                    return a;
                }else if ( dynamic_cast<FlatBot<Constant>*>(b)){
                    return b;
                }else if ( dynamic_cast<FlatTop<Constant>*>(a)){
                    return a;
                }else if ( dynamic_cast<FlatTop<Constant>*>(b)){
                    return b;
                }else {
                    auto aa = a->get();
                    auto bb = b->get();
                    if(aa == bb){
                        return a;
                    }else{
                        return bot();
                    }
            }

        }
        Const * mod(Const * a, Const * b) {
            BITROUTINE(%)
        }
        Const * eq(Const * a, Const * b) {
            ROUTINE(==)
        }
        Const * lte(Const * a, Const * b) {
            ROUTINE(<=)
        }
        Const * lt(Const * a, Const * b) {
            ROUTINE(<)
        }
        Const * gt(Const * a, Const * b) {
            ROUTINE(>)
        }
        Const * gte(Const * a, Const * b) {
            ROUTINE(>=)
        }
        Const * neq(Const * a, Const * b) {
            ROUTINE(!=)
        }
        Const * bor(Const * a, Const * b) {
            BITROUTINE(|)
        }
        Const * band(Const * a, Const * b) {
            BITROUTINE(&)
        }
        Const * bxor(Const * a, Const * b) {
            BITROUTINE(^)
        }
        Const * lsh(Const * a, Const * b) {
            BITROUTINE(<<)
        }
        Const * rsh(Const * a, Const * b) {
            BITROUTINE(>>)

        }


        Const * trunc(Const * a) {
            if( dynamic_cast<FlatBot<Constant>*>(a)){
                return a;
            }else if ( dynamic_cast<FlatTop<Constant>*>(a)){
                return a;
            }else {
                auto aa = a->get();
                if (aa.resType() == ResultType::Integer) {
                    return a;
                }else{
                    return num(Constant((int64_t)aa.getFloat()));
                }
            }
        }

        Const * extend(Const * a) {
            if( dynamic_cast<FlatBot<Constant>*>(a)){
                return a;
            }else if ( dynamic_cast<FlatTop<Constant>*>(a)){
                return a;
            }else {
                auto aa = a->get();
                if (aa.resType() == ResultType::Integer) {
                    return num(Constant((double)aa.getInt()));
                }else{
                    return a;
                }
            }
        }


        Const * unminus(Const * a) {
            if( dynamic_cast<FlatBot<Constant>*>(a)){
                return a;
            }else if ( dynamic_cast<FlatTop<Constant>*>(a)){
                return a;
            }else {
                auto aa = a->get();
                if (aa.resType() == ResultType::Integer) {
                    return num(Constant( - aa.getInt()));
                }else{
                    return num(Constant( - aa.getFloat()));
                }
            }
        }
        Const * uninc(Const * a) {
            if( dynamic_cast<FlatBot<Constant>*>(a)){
                return a;
            }else if ( dynamic_cast<FlatTop<Constant>*>(a)){
                return a;
            }else {
                auto aa = a->get();
                if (aa.resType() == ResultType::Integer) {
                    return num(Constant( aa.getInt() + 1));
                }else{
                    return num(Constant( aa.getFloat() + 1));
                }
            }
        }
        Const * undec(Const * a) {
            if( dynamic_cast<FlatBot<Constant>*>(a)){
                return a;
            }else if ( dynamic_cast<FlatTop<Constant>*>(a)){
                return a;
            }else {
                auto aa = a->get();
                if (aa.resType() == ResultType::Integer) {
                    return num(Constant(  aa.getInt() - 1));
                }else{
                    return num(Constant( aa.getFloat() - 1));
                }
            }
        }
        Const * unnot(Const * a) {
            if( dynamic_cast<FlatBot<Constant>*>(a)){
                return a;
            }else if ( dynamic_cast<FlatTop<Constant>*>(a)){
                return a;
            }else {
                auto aa = a->get();
                if (aa.resType() == ResultType::Integer) {
                    return num(Constant( ~ aa.getInt()));
                }else{
                    return bot();
                }
            }
        }



    private:
    };


}
