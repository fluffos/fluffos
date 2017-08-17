
#ifndef OTABLE_H
#define OTABLE_H

#include<list>
#include<memory>
#include<string>
#include<unordered_map>

//change this into a singleton?
//or make the otable and callout tables a part of a new runtime data structure?
//change basename to use std::string? 


class ObjectTable
{
public:
    static void init(); 
//    static ObjectTable* getInstance();
//    static void cleanup();
    
    static bool insert(struct object_t *);
    static bool remove(struct object_t *);
    static object_t *find(const char *);
    static int show_otable_status (outbuffer_t *, int);
#ifndef SILENUS_DEBUG
    //TODO: change this to return std::vector add function in array.cc to convert std::vector to array_t
    static struct array_t *children(const char *);
#endif
protected:
    static std::string basename(const char *full);
    /*
    ObjectTable()
    :searches_(0), found_(0), userLookups_(0), userFound_(0)
    {}
    */
private:	
    
    // static ObjectTable* instance_;

    static std::unordered_map< std::string, struct object_t* > objects_;
    static std::unordered_map< std::string, std::list<struct object_t*> > children_;
    static long searches_;
    static long found_;
    static long userLookups_;
    static long userFound_;
};

#endif
