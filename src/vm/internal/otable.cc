#include "base/std.h"
#include "vm/internal/base/machine.h"
#include "vm/internal/otable.h"

#include <algorithm>
#include <sstream>

std::unique_ptr<ObjectTable> ObjectTable::instance_;

ObjectTable::ObjectTable() : objects_({}), children_({}) {}

// static method to return a pointer to the singleton object table.
ObjectTable& ObjectTable::instance() {
  if (!instance_) instance_.reset(new ObjectTable());
  return *(instance_.get());
}

// attempt to insert an key(obname), object pointer pair into the object table if the key is not in
// the table. also will insert the key, object pointer pair in the childrens list. if the key is
// already in the object table it fails and returns false, otherwise true.
bool ObjectTable::insert(Key const& key, Value value) {
  if (objects_.find(key) != objects_.end()) return false;

  objects_.insert(std::make_pair(key, value));
  auto base = basename(key);
  auto it = children_.find(base);
  if (it == children_.end()) {
    children_.insert(std::pair<Key, Vector>(base, {value}));
  } else {
    it->second.push_back(value);
  }
  return true;
}

// attempt to find the object named key in the object table. Return a pointer to it, or if not found
// the null pointer.
ObjectTable::Value ObjectTable::find(Key const& key) {
  auto it = objects_.find(key);
  if (it == objects_.end()) return nullptr;
  return it->second;
}

// return the children of key if any as a vector of object pointers(Values).
ObjectTable::Vector ObjectTable::children(Key const& key) {
  auto it = children_.find(basename(key));
  if (it == children_.end()) return Vector({});
  return it->second;
}

// Attempt to find the object with name key in the object table and remove it
// from both the objects map and the children map if found. Returns true if found,
// otherwise false.
// TODO: generalize this code by removing dependency on field obname.
bool ObjectTable::remove(Key const& key) {
  auto it1 = objects_.find(key);
  if (it1 == objects_.end()) return false;
  objects_.erase(it1);
  // guaranteed to exist if object exists
  auto it2 = children_.find(basename(key));
  auto it3 = std::find_if(it2->second.begin(), it2->second.end(),
                          [&key](Value v) -> bool { return v->obname == key; });
  // guaranteed to be in list if basename(key) exists in children_
  it2->second.erase(it3);
  if (it2->second.size() == 0) {
    children_.erase(it2);
  }
  return true;
}

// write some information about memory usage and other statistics of the object table to
// out_buffer_t.
// TODO: remove dependency on outbuffer_t here
int ObjectTable::showStatus(outbuffer_t* out, int verbose) {
  std::stringstream ss;

  switch (verbose) {
    case 1:
      ss << "Object name hash table status:"
         << "\n";
      ss << "------------------------------"
         << "\n";
      ss << "Elements:        " << objects_.size() << "\n";
      ss << "Memory(bytes):     " << objects_.size() * sizeof(Value) << "\n";
      ss << "Bucket count:    " << objects_.bucket_count() << "\n";
      ss << "Load factor:     " << objects_.load_factor() << "\n";
      break;

    case 0:
      ss << "Memory used(bytes):     " << objects_.size() * sizeof(Value) << "\n";
      break;

    default:
      break;
  }
  {
    std::string tmp = ss.str();
    outbuf_add(out, tmp.c_str());
  }

  return objects_.size() * sizeof(Value);
}

std::string basename(std::string s) {
  // remove all leading forward slashes from string.
  auto it1 = s.begin();
  for (; *it1 == '/'; ++it1)
    ;
  s.erase(s.begin(), it1);

  // find # in the string and delete it and all subsequent characters
  auto it2 = s.find('#');
  if (it2 != std::string::npos) s.erase(s.begin() + it2, s.end());

  // remove all repetitions of .c at the end of the string
  it1 = s.end() - 1;
  for (; it1 - 1 != s.begin() && *it1 == 'c' && *(it1 - 1) == '.'; it1 -= 2)
    ;
  s.erase(it1 + 1, s.end());
  return s;
}
