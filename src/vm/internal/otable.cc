#include "base/std.h"
#include "vm/internal/otable.h"
#include "vm/internal/base/machine.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <cassert>
#include <cstdio>

/*
 * Object name hash table.  Object names are unique, so no special
 * problems - like stralloc.c.  For non-unique hashed names, we need
 * a better package (if we want to be able to get at them all) - we
 * cant move them to the head of the hash chain, for example.
 *
 * Note: if you change an object name, you must remove it and reenter it.
 */

ObjectTable* ObjectTable::instance_ = nullptr;

void ObjectTable::init() 
{
    instance_ = new ObjectTable;
}
    
ObjectTable* ObjectTable::getInstance() { return instance_; }

/*
 * Add an object to the table - can't have duplicate names.
 * 
 * Exception: Precompiled objects have a dummy entry here, but it is
 * guaranteed to be behind the real entry if a real entry exists.
 */

void ObjectTable::insert(struct object_t * ob) {
        
#ifdef DEBUG
        auto i = objects_.find(ob->obname);
        searches_++;
        if( i != objects_.end() && i->second != master_ob && i->second != simul_efun_ob)
        {
            found_++;
            DEBUG_CHECK1(i->second && i->second != ob,
				"Duplicate object \"/%s\" in object hash table",
				ob->obname);
            objects_.erase(i);
        }
#endif
        objects_.insert( std::make_pair(ob->obname,ob) );
		auto base = basename(ob->obname);

        auto j = children_.find(base);
        if(j != children_.end() )
        {
            (j->second).push_back(ob);
        }
        else
        {
			children_.insert( std::make_pair(base,std::list<decltype(ob)>{ob} ) );
        }
	return;
}

/*
 * Remove an object from the table - generally called when it
 * is removed from the next_all list - i.e. in destruct.
 */

//need to check for exceptions.
void ObjectTable::remove(struct object_t * ob) 
{
        auto i = objects_.find(ob->obname);
        searches_++;
        if( i == objects_.end() )
        {
            fatal("couldn't find object %s in obj_table", ob->obname);
        }
        else
        {
            found_++;
            objects_.erase(i);
            
            auto base = basename(ob->obname);
            auto j = children_.find(base);
            auto k = std::find( (j->second).begin(), (j->second).end(), ob );
            
            if( k == (j->second).end() )
            {
                fatal("object not found in children list");
            }
            else
            {
                (j->second).remove(ob);
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

array_t *ObjectTable::children(const char * s) 
{
    
	auto base = basename(s);
	auto i = children_.find(base);
	assert(i != children_.end());

	auto ret = allocate_empty_array((i->second).size());
	auto k = 0;
	for (auto ob : i->second)
	{
		ret->item[k].type = T_OBJECT;
		ret->item[k].u.ob = ob;
		add_ref(ob, "children");
		++k;
	}
	ret = resize_array(ret, k);
	return ret;
}


/*
 * Print stats, returns the total size of the object table.  All objects
 * are in table, so their size is included as well.
 */

//need to revise these statistics since they aren't quite right anymore.
//print out bucket and stl statistics?
//change to use a std::string and an string ostream?
int ObjectTable::show_otable_status(outbuffer_t * out, int verbose) {

	std::stringstream ss;

	if (verbose == 1) 
	{
		ss << "Object name hash table status:" << std::endl;
		ss << "------------------------------" << std::endl;
		ss << "Average hash chain length:       " << objects_.size() / (float)objects_.max_size() << std::endl;
		ss << "Internal lookups (succeeded):    " << searches_ - userLookups_ << "(" << found_ - userFound_ << ")" << std::endl;
		ss << "External lookups(succeeded) :    " << userLookups_ << "(" << userFound_ << ")" << std::endl;

		outbuf_addv( out, ss.str().c_str() );

//		sprintf(sbuf, "%10.2f", (float) obj_probes / obj_searches);
//		outbuf_addv(out, "Average search length:           %s\n", sbuf);

//		outbuf_addv(out, "Bucket size:			%s\n",sbuf);
//		outbuf_addv(out, "Bucket count:");
	}
	int starts = (long) objects_.max_size() *sizeof(object_t *)+objects_.size()
			* sizeof(object_t);

	if (!verbose) {
		ss << "Obj table overhead:\t\t" << sizeof(object_t*) * objects_.max_size() << " " << starts << std::endl;
		outbuf_addv(out, ss.str().c_str());
	}
	return starts;
}

std::string ObjectTable::basename(const char *full) {

	std::string s = { full };
	for (auto i = 0; i < s.length(); ++i)
	{
		if (s[i] == '#')
		{
			s.erase(s.begin() + i, s.end());
			break;
		}
	}
	return s;
}