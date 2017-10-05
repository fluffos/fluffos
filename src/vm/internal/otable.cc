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

//static method to return a pointer to the singleton object table.
ObjectTable::Singleton ObjectTable::get() {
    if( instance_ == nullptr )
        instance_ = Singleton( new ObjectTable() );
    return instance_;
}

//fix style and efficiency errors in this function
//should pass in a K const& k. also why is V not prefixed by ObjectTable and
//it still works? is the ObjectTable needed in the parameter list? 

//attempt to insert an key(obname), object pointer pair into the object table if the key is not in the table.
//also will insert the key, object pointer pair in the childrens list.
//if the key is already in the object table it fails and returns false, otherwise true.
bool ObjectTable::insert(Key const& key,Value value) {
    if( objects_.find(key) == objects_.end() ) {
        objects_.insert( std::make_pair(key,value) );
        auto base = basename(key);
        auto it = children_.find(base);
        if( it == children_.end() ) {
          children_.insert( std::pair<Key,Vector>(base, {value} ) );
        }
        else {
            it->second.push_back(value);
        }
        return true;
    } 
    else {
        return false;
    }

}

//attempt to find the object named key in the object table. Return a pointer to it, or if not found the null pointer.
ObjectTable::Value ObjectTable::find(Key const & key) {
    auto it = objects_.find(key);
    if( it != objects_.end() ) {
        return it->second;
    } 
    else {
        return nullptr;
    }
}

//return the children of key if any as a vector of object pointers(Values).
ObjectTable::Vector ObjectTable::children(Key const & key) {
    auto it = children_.find( key );
    if( it != children_.end() ) {
        return it->second;
    }
    else {
        return Vector({});
    }
}

//Attempt to find the object with name key in the object table and remove it
//from both the objects map and the children map if found. Returns true if found,
//otherwise false.
//TODO: generalize this code by removing dependency on field obname.
bool ObjectTable::remove(Key const & key) {
    
    auto it1 = objects_.find(key);
    if( it1 != objects_.end() ) {
        objects_.erase(it1);
        //guaranteed to exist if object exists
        //std::assert( children_.find( basename(key)) != children_.end() );
        auto it2 = children_.find( basename(key) );
        auto it3 = find_if( it2->second.begin(), it2->second.end(), [&key](Value v) -> bool { return v->obname == key; } );
        if(it3 != it2->second.end() ) {
            it2->second.erase(it3);
        }
        else {
            return false;
        }
        if( it2->second.size() == 0 ) {
            children_.erase(it2);
        }
        return true;
    }
    else {
        return false;
    }
}

//write some information about memory usage and other statistics of the object table to out_buffer_t.
//TODO: remove dependency on outbuffer_t here
#ifndef TESTING
int ObjectTable::showStatus(outbuffer_t *out, int verbose) {

    std::stringstream ss;
    if (verbose == 1) {
        ss <<  "Object name hash table status:" << std::endl;
        ss <<  "Object name hash table status:" << std::endl;
        ss << "------------------------------" << std::endl;
        ss << "Elements:        " << objects_.size() << std::endl;
        ss << "Memory(bytes):     " << objects_.size() * sizeof(Value) << std::endl;
        ss << "Bucket count:    " << objects_.bucket_count() << std::endl;
        ss << "Load factor:     " << objects_.load_factor() << std::endl;

        outbuf_add(out, ss.str().c_str());
      }

    if (!verbose) {
        ss << "Memory used(bytes):     " << objects_.size() * sizeof(Value) << std::endl;
        outbuf_add(out, ss.str().c_str());
    }
    return objects_.size() * sizeof(Value);
}
#endif

std::string basename(std::string s) {
    
    //remove all leading forward slashes from string.
    auto it1 = s.begin();
    for(; *it1 == '/' ;++it1);
    s.erase(s.begin(),it1);
    
    //find # in the string and delete it and all subsequent characters
    auto it2 = s.find('#');
    if(it2 != std::string::npos )
        s.erase( s.begin() + it2, s.end() );
    
    //remove all repetitions of .c at the end of the string
    it1 = s.end() - 1;
    for(;it1-1 != s.begin() && *it1 == 'c' && *(it1-1) == '.';it1 -= 2);
    s.erase(it1+1, s.end() );
    return s;
}