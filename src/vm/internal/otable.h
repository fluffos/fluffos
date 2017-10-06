#ifndef OTABLE_H
#define OTABLE_H

#include<cassert>
#include<memory>
#include<string>
#include<unordered_map>
#include<vector>

struct object_t;

std::string basename(std::string s);

class ObjectTable /* final */
{
public:
    using Key = std::string;
    using Value = object_t*;
    using Vector = std::vector<Value>;
    using Singleton = std::shared_ptr<ObjectTable>;

    static Singleton get();
    bool insert(Key const & key, Value value);
    Value find(Key const & key);
    Vector children(Key const & key);
    bool remove(Key const & key);
#ifndef TESTING
    int showStatus(outbuffer_t *out, int verbose);
#endif

    
private:
    static Singleton instance_;

    ObjectTable();
    
    std::unordered_map<Key,Value> objects_;
    std::unordered_map<Key,Vector> children_;
};

#endif