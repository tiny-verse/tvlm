#pragma once

#include <map>
#include <unordered_set>
#include "tvlm/tvlm/analysis/lattice/lattice.h"
#include "flat_lattice.h"

template<typename A, typename B>
class MAP {
public:
    explicit MAP():
    contents_(),
    specified(false){}

    MAP( B defaultVal )
    :contents_(),
    defaultValue(defaultVal),
    specified(true){
    }

    MAP( B && defaultVal )
    :contents_(),
    defaultValue(std::move(defaultVal)),
    specified(true){
    }

    MAP( MAP && other) :
    contents_(std::move(other.contents_)),
    registered_(std::move(other.registered_)),
    specified(std::move(other.specified)),
    defaultValue(std::move(other.defaultValue)){
    }

    MAP( const MAP & other) :
    contents_(other.contents_),
    registered_(other.registered_),
    specified(other.specified),
    defaultValue(other.defaultValue){
    }

    virtual ~MAP(){
        if constexpr(std::is_pointer<B>::value){
            for (auto & b: registered_) {
                delete b;
            }
        }
        registered_.clear();
    }

    MAP & operator= (const MAP & other){
//        this->~MAP();
        if constexpr(std::is_pointer<B>::value){
            for (auto & b: registered_) {
                delete b;
            }
        }
        registered_.clear();

//        MAP tmp {other};
//        std::swap(tmp, *this);
        contents_ = other.contents_;
        specified = other.specified;
        defaultValue = other.defaultValue;
        registered_ = other.registered_;
        return *this;
    }

    MAP & operator= (MAP && other){
//        this->~MAP();
        if constexpr(std::is_pointer<B>::value){
            for (auto & b: registered_) {
                delete b;
            }
        }
        registered_.clear();
//        MAP tmp {other};
//        std::swap(tmp, *this);
        contents_ = std::move(other.contents_);
        specified = other.specified;
        defaultValue = std::move(other.defaultValue);
        registered_ = std::move(other.registered_);

        other.contents_ = std::map<A,B>();
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
        auto it = contents_.find(idx);
        if( !specified ||  it != contents_.end()) {
            return it->second;
        }
        return defaultValue;
    }

    virtual B const & access (const A & idx)const{
        auto it = contents_.find(idx);
        if( !specified ||  it != contents_.end()) {
            return it->second;
        }
        return defaultValue;
    }

    typename std::map<A, B>::iterator begin(){
        return contents_.begin();
    }

    typename std::map<A, B>::const_iterator cbegin()const{
        return contents_.cbegin();
    }

    typename std::map<A, B>::const_iterator begin()const{
        return contents_.cbegin();
    }

    typename std::map<A, B>::iterator end(){
        return contents_.end();
    }

    typename std::map<A, B>::const_iterator cend()const{
        return contents_.cend();
    }

    typename std::map<A, B>::const_iterator end()const{
        return contents_.cend();
    }

    std::pair<typename std::map<A,B>::iterator, bool>
    insert(typename std::map<A,B>::value_type && x)
    { return contents_.insert(std::move(x)); }

    std::pair<typename std::map<A,B>::iterator, bool>
    insert(typename std::map<A,B>::value_type & x)
    { return contents_.insert(x); }

    std::pair<typename std::map<A,B>::iterator, bool>
    insert(const typename std::map<A,B>::value_type & x)
    { return contents_.insert(x); }

    typename std::map<A,B>::iterator
    update(typename std::map<A,B>::value_type && x)
    {
        auto [res, succ] = contents_.insert(std::move(x));
        if(!succ){
            contents_.insert_or_assign(res, x.first, std::move(x.second));
        }
        return res;
    }

    typename std::map<A,B>::iterator
    update(const typename std::map<A,B>::value_type & x)
    {
        auto [res, succ] = contents_.insert(std::move(x));
        if(!succ){
            contents_.insert_or_assign(res, x.first, x.second);
        }
        return res;
    }

    template<typename... Args>
    std::pair<typename std::map<A, B>::iterator, bool>
    emplace(Args&&... args){
        return contents_.emplace(args...);
    }

    typename std::map<A, B>::mapped_type&
    operator[](const typename std::map<A,B>::key_type& k){
        return contents_[k];
    }
    const typename std::map<A, B>::mapped_type&
    operator[](const typename std::map<A,B>::key_type& k)const{
        return contents_.at(k);
    }

    typename std::map<A,B>::iterator find(const typename std::map<A,B>::key_type& x){
        return contents_.find(x);
    }
    typename std::map<A,B>::const_iterator find(const typename std::map<A,B>::key_type& x)const {
        return contents_.find(x);
    }

    typename std::map<A,B>::size_type size() const noexcept {
        return contents_.size();
    }

    typename std::map<A,B>::iterator
    erase(typename std::map<A,B>::const_iterator __position)
    { return contents_.erase(__position); }

    typename std::map<A,B>::size_type
    erase(const typename std::map<A,B>::key_type& __x)
    { return contents_.erase(__x); }

     bool
    empty() const noexcept
    { return contents_.empty(); }


//    virtual B makeVal(B val) {
//        storage_.push_back(val);
//        return val;
//    }

    void reg( B pVal);

private:
    B defaultValue;
    bool specified;
    std::vector<B> registered_;
    std::map<A,B> contents_;
//    std::vector<std::unique_ptr<B>> storage_;


template<typename Aa, typename Bb>
friend
inline bool    operator==(const MAP<Aa, Bb>& x, const MAP<Aa, Bb>& y);
};

template<typename A, typename B>
void MAP<A, B>::reg( B pVal) {
    registered_.push_back(pVal);
}
template<typename A, typename B>
inline bool
operator!=(const MAP<A, B>& x,
           const MAP<A, B>& y)
{ return !(x == y); }

template<typename A, typename B>
inline bool
operator==(const MAP<A, B>& x,
           const MAP<A, B>& y)
{ return x.contents_ == y.contents_; }

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
                res.update(std::make_pair(e, lat_->lub(x.access(e), y.access(e))));
            }
            return res;
        }

    private:
        std::unordered_set<A> set_;
        Lattice<B>* lat_;
    };
}
