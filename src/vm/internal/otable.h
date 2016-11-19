
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
    static ObjectTable* getInstance();
    static void cleanup();
    
    bool insert(struct object_t *);
    bool remove(struct object_t *);
    object_t *find(const char *);
    int show_otable_status (outbuffer_t *, int);
#ifndef SILENUS_DEBUG
    struct array_t *children(const char *);
#endif
protected:
    std::string basename(const char *full);
    ObjectTable()
    :searches_(0), found_(0), userLookups_(0), userFound_(0)
    {}
private:	
    
    static ObjectTable* instance_;

    std::unordered_map< std::string, struct object_t* > objects_;
    std::unordered_map< std::string, std::list<struct object_t*> > children_;
    long searches_, found_;
    long userLookups_;
    long userFound_;
};

#endif