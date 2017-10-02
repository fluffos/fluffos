#ifndef OTABLE_H
#define OTABLE_H

#include<cassert>
#include<memory>
#include<string>
#include<unordered_map>
#include<vector>

#ifdef TESTING
struct object_t
{
public:
    std::string obname;

    object_t(std::string const & s)
    :obname(s) {}
};
#endif

class ObjectTable /* final */
{
public:
    using Key = std::string;
    using Value = object_t*;
    using Vector = std::vector<Value>;
    using M1 = std::unordered_map<Key,Value>;
    using M2 = std::unordered_map<Key,Vector>;
    using Singleton = std::shared_ptr<ObjectTable>;

    static Singleton get();
    bool insert(Key const & k, Value v);
    Value find(Key const & k);
    Vector children(Key const & k);
    bool remove(Key const & k);
#ifndef TESTING
    int showStatus(outbuffer_t *out, int verbose);
#endif

    Key basename(Key k);
private:
    static Singleton instance_;

    ObjectTable();
    
    M1 objects_;
    M2 children_;

};

#endif