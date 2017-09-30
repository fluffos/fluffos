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
    std::string name;

    object_t(std::string const & s)
    :name(s) {}
};
#endif

class ObjectTable /* final */
{
public:
    using K = std::string;
    using V = object_t*;
    using L = std::vector<V>;
    using M1 = std::unordered_map<K,V>;
    using M2 = std::unordered_map<K,L>;
    using S = std::shared_ptr<ObjectTable>;

    static S get();
    bool insert(K k, V v);
    V find(K const & k);
    L children(K const & k);
    bool remove(K const & k);
    int showStatus(outbuffer_t *out, int verbose);

    K basename(K k);
private:
    static S instance_;

    ObjectTable();
    
    M1 objects_;
    M2 children_;

};

#endif