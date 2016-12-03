#pragma once
#include <memory>
#include <iostream>
#include <string>

// http://stackoverflow.com/questions/18856824/ad-hoc-polymorphism-and-heterogeneous-containers-with-value-semantics

std::string str(const std::string& s)
{
    return s;
}

template <typename T>
std::string str(const T& t);

class Object;

class ObjectInterface 
{
public:
    virtual ~ObjectInterface() {}
    virtual std::string __str__() = 0;
    virtual std::unique_ptr<ObjectInterface> clone() const = 0;
    
    virtual bool __lt__(const std::unique_ptr<ObjectInterface>& other) const = 0;
};

template <typename T> class ObjectImpl: public ObjectInterface 
{
public:
    template <typename ...Ts> ObjectImpl( Ts&&...ts ) 
        : t( std::forward<Ts>(ts)... ) {}
    virtual std::string __str__() override { return str(t); }
    virtual std::unique_ptr<ObjectInterface> clone() const override
    {
        return std::make_unique<ObjectImpl<T>>( t ); // This is C++14
    }
    virtual bool __lt__(const std::unique_ptr<ObjectInterface>& other) const override
    {
        auto other_impl = dynamic_cast<ObjectImpl<T>*>(other.get());
        return t < other_impl->t;
    }

private:
T t;
};


class Object
{
public:
    template <typename T> Object( T t )
        : p( std::make_unique<ObjectImpl<T>>( std::move(t) ) ) {}
    Object( const Object& other ) 
        : p( other.p->clone() ) {}
    Object( Object && other ) noexcept 
        : p( std::move(other.p) ) {}
    void swap( Object & other ) noexcept 
        { p.swap(other.p); }
    Object & operator=( Object other ) 
        { swap(other); }
    virtual std::string __str__() 
        { return p->__str__(); }
    bool __lt__ (const Object& other) const
        { return p->__lt__(other.p); }
private:
    std::unique_ptr<ObjectInterface> p;
};

std::string str(Object& o)
{
    return o.__str__();
}

template <typename T>
std::string str(const T& t)
{
    return std::to_string(t);
}

std::ostream& operator<<(std::ostream& os, Object& obj)
{
    std::cout << obj.__str__();
    return os;
}

inline bool operator< (const Object& lhs, const Object& rhs){ return lhs.__lt__(rhs); }
inline bool operator< (const Object& lhs, const auto& rhs)  { return lhs.__lt__(Object(rhs)); }
inline bool operator> (const Object& lhs, const auto& rhs)  { return rhs < lhs; }
inline bool operator<=(const Object& lhs, const auto& rhs)  { return !(lhs > rhs); }
inline bool operator>=(const Object& lhs, const auto& rhs)  { return !(lhs < rhs); }
