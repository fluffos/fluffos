class TestClass {
  int x;
  string y;
  mapping m;
}

void do_tests() {
  int num = 1, num2 = 2;
  int *nums = ({1,3,4});
  string fluffos = "FluffOS";
  object tp = this_player();
  mapping m = ([
    1: num,
    this_player(): nums,
    "key": ([
      "another key": ([
        "finalKey": "Sup?"
      ]),
      "largo" : ([
        "montego": ([
          "baby" : "why",
          "don't": "we go?"
        ])
      ])
    ])
  ]);


  // Asserts directly on the mapping
  ASSERT_EQ(num, m[1]);
  ASSERT_EQ(nums, m[this_player()]);

  ASSERT_EQ(([
      "another key": ([
        "finalKey": "Sup?"
      ]),
      "largo" : ([
        "montego": ([
          "baby" : "why",
          "don't": "we go?"
        ])
      ])
    ]),
    m["key"]
  );
  ASSERT_EQ(([
      "another key": ([
        "finalKey": "Sup?"
      ]),
      "largo" : ([
        "montego": ([
          "baby" : "why",
          "don't": "we go?"
        ])
      ])
    ]),
  m.key
  );

  ASSERT_EQ(([
      "finalKey": "Sup?"
    ]),
    m["key"]["another key"]
  );
  ASSERT_EQ(([
      "finalKey": "Sup?"
    ]),
    m.key["another key"]
  );

  ASSERT_EQ(([
      "montego": ([
        "baby" : "why",
        "don't": "we go?"
      ])
    ]),
    m["key"]["largo"]
  );
  ASSERT_EQ(([
      "montego": ([
        "baby" : "why",
        "don't": "we go?"
      ])
    ]),
    m.key["largo"]
  );
  ASSERT_EQ(([
      "montego": ([
        "baby" : "why",
        "don't": "we go?"
      ])
    ]),
    m.key.largo
  );
  ASSERT_EQ(([
      "montego": ([
        "baby" : "why",
        "don't": "we go?"
      ])
    ]),
    m["key"].largo
  );

  // Above, but with spaces
  ASSERT_EQ(([
      "montego": ([
        "baby" : "why",
        "don't": "we go?"
      ])
    ]),
    m["key"] ["largo"]
  );
  ASSERT_EQ(([
      "montego": ([
        "baby" : "why",
        "don't": "we go?"
      ])
    ]),
    m . key["largo"]
  );
  ASSERT_EQ(([
      "montego": ([
        "baby" : "why",
        "don't": "we go?"
      ])
    ]),
    m . key . largo
  );
  ASSERT_EQ(([
      "montego": ([
        "baby" : "why",
        "don't": "we go?"
      ])
    ]),
    m ["key"] . largo
  );

  ASSERT_EQ(([
      "baby" : "why",
      "don't": "we go?"
    ]),
    m["key"]["largo"]["montego"]
  );
  ASSERT_EQ(([
      "baby" : "why",
      "don't": "we go?"
    ]),
    m.key["largo"]["montego"]
  );
  ASSERT_EQ(([
      "baby" : "why",
      "don't": "we go?"
    ]),
    m.key.largo["montego"]
  );
  ASSERT_EQ(([
      "baby" : "why",
      "don't": "we go?"
    ]),
    m.key.largo.montego
  );
  ASSERT_EQ(([
      "baby" : "why",
      "don't": "we go?"
    ]),
    m.key["largo"].montego
  );

  ASSERT_EQ(([])[0], m["nope"]);
  ASSERT_EQ(([])[0], m.nope);

  ASSERT_EQ(
    "*Tried to take a member of something that isn't a mapping.\n",
    catch(evaluate(m.still.nope))
  );
  ASSERT_EQ(
    "*Value being indexed is zero.\n",
    catch(evaluate(m["still"]["nope"]))
  );

  // Same asserts, but now on the mapping from the class

  class TestClass testClass = new(class TestClass,
    x: num2,
    y: fluffos,
    m: m
  );

  ASSERT_EQ(num, testClass.m[1]);
  ASSERT_EQ(nums, testClass.m[this_player()]);

  ASSERT_EQ(([
      "another key": ([
        "finalKey": "Sup?"
      ]),
      "largo" : ([
        "montego": ([
          "baby" : "why",
          "don't": "we go?"
        ])
      ])
    ]),
    testClass.m["key"]
  );
  ASSERT_EQ(([
      "another key": ([
        "finalKey": "Sup?"
      ]),
      "largo" : ([
        "montego": ([
          "baby" : "why",
          "don't": "we go?"
        ])
      ])
    ]),
    testClass.m.key
  );

  ASSERT_EQ(([
      "finalKey": "Sup?"
    ]),
    m["key"]["another key"]
  );
  ASSERT_EQ(([
      "finalKey": "Sup?"
    ]),
    testClass.m.key["another key"]
  );

  ASSERT_EQ(([
      "montego": ([
        "baby" : "why",
        "don't": "we go?"
      ])
    ]),
    testClass.m["key"]["largo"]
  );
  ASSERT_EQ(([
      "montego": ([
        "baby" : "why",
        "don't": "we go?"
      ])
    ]),
    testClass.m.key["largo"]
  );
  ASSERT_EQ(([
      "montego": ([
        "baby" : "why",
        "don't": "we go?"
      ])
    ]),
    testClass.m.key.largo
  );
  ASSERT_EQ(([
      "montego": ([
        "baby" : "why",
        "don't": "we go?"
      ])
    ]),
    testClass.m["key"].largo
  );

  // Above, but with spaces
  ASSERT_EQ(([
      "montego": ([
        "baby" : "why",
        "don't": "we go?"
      ])
    ]),
    testClass . m["key"] ["largo"]
  );
  ASSERT_EQ(([
      "montego": ([
        "baby" : "why",
        "don't": "we go?"
      ])
    ]),
    testClass. m . key["largo"]
  );
  ASSERT_EQ(([
      "montego": ([
        "baby" : "why",
        "don't": "we go?"
      ])
    ]),
    testClass . m . key . largo
  );
  ASSERT_EQ(([
      "montego": ([
        "baby" : "why",
        "don't": "we go?"
      ])
    ]),
    testClass . m ["key"] . largo
  );

  ASSERT_EQ(([
      "baby" : "why",
      "don't": "we go?"
    ]),
    testClass.m["key"]["largo"]["montego"]
  );
  ASSERT_EQ(([
      "baby" : "why",
      "don't": "we go?"
    ]),
    testClass.m.key["largo"]["montego"]
  );
  ASSERT_EQ(([
      "baby" : "why",
      "don't": "we go?"
    ]),
    testClass.m.key.largo["montego"]
  );
  ASSERT_EQ(([
      "baby" : "why",
      "don't": "we go?"
    ]),
    testClass.m.key.largo.montego
  );
  ASSERT_EQ(([
      "baby" : "why",
      "don't": "we go?"
    ]),
    testClass.m.key["largo"].montego
  );

  ASSERT_EQ(([])[0], testClass.m["nope"]);
  ASSERT_EQ(([])[0], testClass.m.nope);

  ASSERT_EQ(
    "*Tried to take a member of something that isn't a mapping.\n",
    catch(evaluate(testClass.m.still.nope))
  );
  ASSERT_EQ(
    "*Value being indexed is zero.\n",
    catch(evaluate(testClass.m["still"]["nope"]))
  );
}
