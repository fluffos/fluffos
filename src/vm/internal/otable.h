
#ifndef OTABLE_H
#define OTABLE_H

#include<list>
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

    void insert(struct object_t *);
    void remove(struct object_t *);
    object_t *find(const char *);
    int show_otable_status (outbuffer_t *, int);
    struct array_t *children(const char *);
private:
	
	ObjectTable() {}
	std::string basename(const char *full);

	static ObjectTable* instance_;

    std::unordered_map< std::string, struct object_t* > objects_;
    std::unordered_map< std::string, std::list<struct object_t*> > children_;
    long searches_, found_;
    long userLookups_;
    long userFound_;
};

#endif