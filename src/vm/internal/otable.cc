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


ObjectTable::S ObjectTable::instance_ = nullptr;

ObjectTable::ObjectTable()
:objects_({}),children_({})
{}

ObjectTable::S ObjectTable::get() {
    if( instance_ == nullptr )
        instance_ = S( new ObjectTable() );
    return instance_;
}

bool ObjectTable::insert(ObjectTable::K k,V v) {
    if( objects_.find(k) == objects_.end() ) {
        objects_.insert( std::make_pair(k,v) );
        auto n = basename(k);
        auto i = children_.find(n);
        if( i == children_.end() ) {
          children_.insert( std::pair<K, L>( n, {v} ) );
        }
        else {
            i->second.push_back(v);
        }
        return true;
    } 
    else {
        return false;
    }

}

ObjectTable::V ObjectTable::find(ObjectTable::K const & k) {
    auto i = objects_.find(k);
    if( i != objects_.end() )
        return i->second;
    else
        return nullptr;
}

ObjectTable::L ObjectTable::children(ObjectTable::K const & k) {
    auto i = children_.find( basename(k) );
    if( i != children_.end() )
        return i->second;
    else
        return L({});
}

bool ObjectTable::remove(ObjectTable::K const & k) {
    auto i = objects_.find(k);
    if( i != objects_.end() ) {
        objects_.erase(i);
        //guaranteed to exist if object exists
     //   std::assert( children_.find( basename(k)) != children_.end() );
        auto j = children_.find( basename(k) );
        auto l = find_if( j->second.begin(), j->second.end(), [&k](V v) -> bool { return v->obname == k; } );
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
        ss << "Memory used:     " << objects_.size() * sizeof(V) << std::endl;
        ss << "Bucket count:    " << objects_.bucket_count() << std::endl;
        ss << "Load factor:     " << objects_.load_factor() << std::endl;

        outbuf_add(out, ss.str().c_str());
      }

    if (!verbose) {
        ss << "Memory used:     " << objects_.size() * sizeof(V) << std::endl;
        outbuf_add(out, ss.str().c_str());
    }
    return objects_.size() * sizeof(V);
}
#endif

ObjectTable::K ObjectTable::basename(ObjectTable::K k) {
    auto i = k.begin();
    for(; *i == '/' ;++i);
    k.erase(k.begin(),i);
    
    auto j = k.find('#');
    if(j != std::string::npos )
        k.erase( k.begin() + j, k.end() );
    i = k.end() - 1;
    for(;i-1 != k.begin() && *i == 'c' && *(i-1) == '.';i -= 2);
    k.erase(i+1, k.end() );
    return k;
}