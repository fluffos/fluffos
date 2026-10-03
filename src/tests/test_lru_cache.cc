#include <gtest/gtest.h>

#include "base/internal/lru_cache.h"

#include <string>

// lru_cache<> is used for the compiled-program / apply caches. It had no unit
// test at all, so every branch below (eviction, MRU promotion, insert of an
// already-present key) was uncovered.

TEST(LruCache, StartsEmptyAndReportsCapacity) {
  lru_cache<int, int> cache(3);

  EXPECT_TRUE(cache.empty());
  EXPECT_EQ(0u, cache.size());
  EXPECT_EQ(3u, cache.capacity());
  EXPECT_FALSE(cache.contains(1));
  EXPECT_FALSE(cache.get(1).has_value());
}

TEST(LruCache, InsertAndGet) {
  lru_cache<std::string, int> cache(2);

  cache.insert("a", 1);
  cache.insert("b", 2);

  EXPECT_FALSE(cache.empty());
  EXPECT_EQ(2u, cache.size());
  EXPECT_TRUE(cache.contains("a"));
  EXPECT_TRUE(cache.contains("b"));
  EXPECT_EQ(1, cache.get("a").value());
  EXPECT_EQ(2, cache.get("b").value());
  EXPECT_FALSE(cache.get("c").has_value());
}

// insert() on a key already in the cache keeps the stored value and does not
// touch the recency list -- callers that need an update must not rely on
// insert() overwriting.
TEST(LruCache, InsertOfExistingKeyKeepsOldValue) {
  lru_cache<int, int> cache(2);

  cache.insert(1, 10);
  cache.insert(1, 99);

  EXPECT_EQ(1u, cache.size());
  EXPECT_EQ(10, cache.get(1).value());
}

TEST(LruCache, EvictsLeastRecentlyUsedOnOverflow) {
  lru_cache<int, int> cache(2);

  cache.insert(1, 1);
  cache.insert(2, 2);
  cache.insert(3, 3);  // evicts key 1

  EXPECT_EQ(2u, cache.size());
  EXPECT_FALSE(cache.contains(1));
  EXPECT_EQ(2, cache.get(2).value());
  EXPECT_EQ(3, cache.get(3).value());
}

TEST(LruCache, GetPromotesEntryToMostRecentlyUsed) {
  lru_cache<int, int> cache(2);

  cache.insert(1, 1);
  cache.insert(2, 2);
  ASSERT_EQ(1, cache.get(1).value());  // 1 becomes most recently used

  cache.insert(3, 3);  // so 2 is the eviction victim, not 1

  EXPECT_TRUE(cache.contains(1));
  EXPECT_FALSE(cache.contains(2));
  EXPECT_TRUE(cache.contains(3));
}

// get() of the entry that is already at the front takes the other branch of
// the promotion code (no list splice).
TEST(LruCache, RepeatedGetOfFrontEntryIsStable) {
  lru_cache<int, int> cache(2);

  cache.insert(1, 1);
  cache.insert(2, 2);  // 2 is at the front

  EXPECT_EQ(2, cache.get(2).value());
  EXPECT_EQ(2, cache.get(2).value());

  cache.insert(3, 3);  // 1 is the LRU victim
  EXPECT_FALSE(cache.contains(1));
  EXPECT_TRUE(cache.contains(2));
}

TEST(LruCache, SingleSlotCacheAlwaysHoldsLastInsert) {
  lru_cache<int, std::string> cache(1);

  cache.insert(1, "one");
  cache.insert(2, "two");

  EXPECT_EQ(1u, cache.size());
  EXPECT_FALSE(cache.contains(1));
  EXPECT_EQ("two", cache.get(2).value());
}

TEST(LruCache, ClearEmptiesCacheButKeepsCapacity) {
  lru_cache<int, int> cache(2);

  cache.insert(1, 1);
  cache.insert(2, 2);
  cache.clear();

  EXPECT_TRUE(cache.empty());
  EXPECT_EQ(0u, cache.size());
  EXPECT_EQ(2u, cache.capacity());
  EXPECT_FALSE(cache.get(1).has_value());

  // still usable after clear()
  cache.insert(3, 3);
  EXPECT_EQ(3, cache.get(3).value());
}
