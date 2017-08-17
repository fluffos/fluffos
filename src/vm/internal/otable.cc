#include "base/std.h"
#include "vm/internal/otable.h"
#include "vm/internal/base/machine.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <cassert>
#include <cstdio>

#ifdef SILENUS_DEBUG
#include <iostream>
#endif

/*
 * Object name hash table.  Object names are unique, so no special
 * problems - like stralloc.c.  For non-unique hashed names, we need
 * a better package (if we want to be able to get at them all) - we
 * cant move them to the head of the hash chain, for example.
 *
 * Note: if you change an object name, you must remove it and reenter it.
 */


std::unordered_map< std::string, struct object_t* > ObjectTable::objects_;
std::unordered_map< std::string, std::list<struct object_t*> > ObjectTable::children_;


long ObjectTable::searches_ = 0;
long ObjectTable::found_ = 0;
long ObjectTable::userLookups_ = 0;
long ObjectTable::userFound_ = 0;

// ObjectTable* ObjectTable::instance_ = nullptr;

void ObjectTable::init() 
{
    // instance_ = new ObjectTable;
}
    
// ObjectTable* ObjectTable::getInstance() { return instance_; }

/*
void ObjectTable::cleanup()
{
    delete instance_;
}
*/

/*
 * Add an object to the table - can't have duplicate names.
 * 
 * Exception: Precompiled objects have a dummy entry here, but it is
 * guaranteed to be behind the real entry if a real entry exists.
 */

bool ObjectTable::insert(struct object_t * ob) {
        
#ifdef DEBUG
        auto i = objects_.find(ob->obname);
        searches_++;
        if( i != objects_.end() && i->second != master_ob && i->second != simul_efun_ob)
        {
            found_++;
            DEBUG_CHECK1(i->second && i->second != ob,
				"Duplicate object \"/%s\" in object hash table",
				ob->obname);
            //TODO remove ref to object_t here
            //free_object(&ob,"ObjectTable::insert");
            objects_.erase(i);
        }
#endif
        objects_.insert( std::make_pair( std::string(ob->obname),ob) );
        //add_ref(ob,"ObjectTable");
        //TODO add ref to object_t here.
		auto base = basename(ob->obname);

        auto j = children_.find(base);
        if(j != children_.end() )
        {
            (j->second).push_back(ob);
            //add_ref(ob,"ObjectTable::children");
        }
        else
        {
            
			children_.insert( std::make_pair(base,std::list<decltype(ob)>{ob} ) );
                        //add_ref(ob,"ObjectTable");
                        //TODO add ref to object_t here
                        //base object isn't it's own child in fluffos
        }
	return true;
}

/*
 * Remove an object from the table - generally called when it
 * is removed from the next_all list - i.e. in destruct.
 */

//need to check for exceptions.
bool ObjectTable::remove(struct object_t * ob) 
{
        auto i = objects_.find(ob->obname);
        searches_++;
        if( i == objects_.end() )
        {
#ifndef SILENUS_DEBUG
            fatal("couldn't find object %s in obj_table", ob->obname);
#else
            return false;
#endif
        }
        else
        {
            found_++;
            //free_object(i->second);
            objects_.erase(i);
            
            auto base = basename(ob->obname);
            auto j = children_.find(base);
            auto k = std::find( (j->second).begin(), (j->second).end(), ob );
            
            if( k != (j->second).end() )
            {
                //free_object(*k);
                (j->second).erase(k);
            }
            else
            {
#ifndef SILENUS_DEBUG
                fatal("object not found in children list");
#else
                return false;
#endif
            }
        }
}

/*
 * Lookup an object in the hash table; if it isn't there, return null.
 * This is only different to find_object_n in that it collects different
 * stats; more finds are actually done than the user ever asks for.
 */
object_t *ObjectTable::find(const char * s) 
{
    userLookups_++;
    auto i = objects_.find(s);
    if(i != objects_.end() )
    {
        userFound_++;
        return i->second;
    }
    else
        return nullptr;
}

//TODO: change  this to return std::vector and have the array code convert std::vector into array_t
#ifndef SILENUS_DEBUG 
array_t *ObjectTable::children(const char * s) 
{
  
	auto base = basename(s);
	auto i = children_.find(base);
        
        
	// assert(i != children_.end()); // at least one entry should exist

	auto ret = allocate_empty_array( CONFIG_INT(__MAX_ARRAY_SIZE__) );
	auto k = 0;
        
        if(i != children_.end() )
        {
            for (auto ob : i->second)
            {
		ret->item[k].type = T_OBJECT;
		ret->item[k].u.ob = ob;
		add_ref(ob, "children");
		++k;
            }
        }
	ret = resize_array(ret, k);
	return ret;
}
#endif

/*
 * Print stats, returns the total size of the object table.  All objects
 * are in table, so their size is included as well.
 */

//need to revise these statistics since they aren't quite right anymore.
//print out bucket and stl statistics?
//change to use a std::string and an string ostream?
int ObjectTable::show_otable_status(outbuffer_t * out, int verbose) 
{

	std::stringstream ss;

	if (verbose == 1) 
	{
		ss << "Object name hash table status:" << std::endl;
		ss << "------------------------------" << std::endl;
//		ss << "Average hash chain length:       " << objects_.size() / (float)objects_.size() << std::endl;
		ss << "Internal lookups (succeeded):    " << searches_ - userLookups_ << "(" << found_ - userFound_ << ")" << std::endl;
		ss << "External lookups(succeeded) :    " << userLookups_ << "(" << userFound_ << ")" << std::endl;
#ifndef SILENUS_DEBUG
		outbuf_addv( out, ss.str().c_str() );
#else
                std::cout << ss.str();
#endif

//		sprintf(sbuf, "%10.2f", (float) obj_probes / obj_searches);
//		outbuf_addv(out, "Average search length:           %s\n", sbuf);

//		outbuf_addv(out, "Bucket size:			%s\n",sbuf);
//		outbuf_addv(out, "Bucket count:");
	}
	int starts = (long) objects_.max_size() *sizeof(object_t *)+objects_.size()
			* sizeof(object_t);

	if (!verbose) {
		ss << "Obj table overhead:\t\t" << sizeof(object_t*) * objects_.size() << " " << starts << std::endl;
#ifndef SILENUS_DEBUG
		outbuf_addv( out, ss.str().c_str() );
#else
                std::cout << ss.str();
#endif                
	}
	return starts;
}

std::string ObjectTable::basename(const char *full) 
{
//TODO: this function could be alot simplier if it didn't do arguments parsing and made certain sane assumptions about the name passed in
//revise rest of the driver code to make sure that the assumptions are not violated i.e. no (multiple) trailing ".c" and no multiple leading '/'
	std::string s = { full };
        
        auto i = s.begin();
        for(;i != s.end() && *i == '/' ;++i); 
        if( i != s.begin() )
            s.erase(s.begin(),i);
        
        
        auto j = s.find("#");
        if(j != std::string::npos)       
            s.erase(s.begin()+j, s.end());
        else 
        {
            auto l = s.length();
            auto k = l;
            for(;k > 2 && s[k-2] == '.' && s[k-1] == 'c';k-=2);
            if( k != l )
                s.erase(k, std::string::npos);
        }
	return s;
}
