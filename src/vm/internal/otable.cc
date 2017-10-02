#ifndef TESTING
#include "base/std.h"
#include "vm/internal/base/machine.h"
#include "vm/internal/otable.h"
#else
#include<iostream>
#include "otable.h"
#endif


#include <algorithm>
#include <sstream>


ObjectTable::Singleton ObjectTable::instance_ = nullptr;

ObjectTable::ObjectTable()
:objects_({}),children_({})
{}

ObjectTable::Singleton ObjectTable::get() {
    if( instance_ == nullptr )
        instance_ = Singleton( new ObjectTable() );
    return instance_;
}

//fix style and efficiency errors in this function
//should pass in a K const& k. also why is V not prefixed by ObjectTable and
//it still works? is the ObjectTable needed in the parameter list?  
bool ObjectTable::insert(Key const& key,Value value) {
    if( objects_.find(key) == objects_.end() ) {
        objects_.insert( std::make_pair(key,value) );
        auto n = basename(key);
        auto i = children_.find(n);
        if( i == children_.end() ) {
          children_.insert( std::pair<Key,Vector>(n, {value} ) );
        }
        else {
            i->second.push_back(value);
        }
        return true;
    } 
    else {
        return false;
    }

}

ObjectTable::Value ObjectTable::find(Key const & key) {
    auto i = objects_.find(key);
    if( i != objects_.end() ) {
        return i->second;
    } 
    else {
        return nullptr;
    }
}

ObjectTable::Vector ObjectTable::children(Key const & key) {
    auto i = children_.find( basename(key) );
    if( i != children_.end() ) {
        return i->second;
    }
    else {
        return Vector({});
    }
}

bool ObjectTable::remove(Key const & key) {
    auto i = objects_.find(key);
    if( i != objects_.end() ) {
        objects_.erase(i);
        //guaranteed to exist if object exists
     //   std::assert( children_.find( basename(k)) != children_.end() );
        auto j = children_.find( basename(key) );
        auto l = find_if( j->second.begin(), j->second.end(), [&key](Value v) -> bool { return v->obname == key; } );
        if(l != j->second.end() ) {
            j->second.erase(l);
        }
        else {
            return false;
        }
        if( j->second.size() == 0 ) {
            children_.erase(j);
        }
        return true;
    }
    else {
        return false;
    }
}

#ifndef TESTING
int ObjectTable::showStatus(outbuffer_t *out, int verbose) {

    std::stringstream ss;
    if (verbose == 1) {
        ss <<  "Object name hash table status:" << std::endl;
        ss <<  "Object name hash table status:" << std::endl;
        ss << "------------------------------" << std::endl;
        ss << "Elements:        " << objects_.size() << std::endl;
        ss << "Memory used:     " << objects_.size() * sizeof(Value) << std::endl;
        ss << "Bucket count:    " << objects_.bucket_count() << std::endl;
        ss << "Load factor:     " << objects_.load_factor() << std::endl;

        outbuf_add(out, ss.str().c_str());
      }

    if (!verbose) {
        ss << "Memory used:     " << objects_.size() * sizeof(Value) << std::endl;
        outbuf_add(out, ss.str().c_str());
    }
    return objects_.size() * sizeof(Value);
}
#endif

std::string ObjectTable::basename(std::string s) {
    auto i = s.begin();
    for(; *i == '/' ;++i);
    s.erase(s.begin(),i);
    
    auto j = s.find('#');
    if(j != std::string::npos )
        s.erase( s.begin() + j, s.end() );
    i = s.end() - 1;
    for(;i-1 != s.begin() && *i == 'c' && *(i-1) == '.';i -= 2);
    s.erase(i+1, s.end() );
    return s;
}