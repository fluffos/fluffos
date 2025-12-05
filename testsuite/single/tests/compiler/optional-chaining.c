class TestClass {
  int x;
  string y;
  mapping m;
}

void do_tests() {
  int undefined = ([])[0];
  int num = 1;
  int *nums = ({1,3,4});
  string fluffos = "FluffOS";
  mapping orig = ([
    0: fluffos,
    1: num,
    this_player(): nums,
    "key": ([
      undefined: ([
        "defined": "yes"
      ]),
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

  mapping m;

  // Asserts directly on the mapping
  ASSERT_EQ(undefined, m?.[1]);
  ASSERT_EQ(undefined, m?.[this_player()]);
  ASSERT_EQ(undefined, m?.[0]);

  ASSERT_EQ(undefined, m?.[1]?.moo);
  ASSERT_EQ(undefined, m?.[this_player()]?.moo);
  ASSERT_EQ(undefined, m?.[0]?.moo);
  ASSERT_EQ(undefined, m?.key?.foo);

  ASSERT_EQ(
    "*Value being indexed is zero.\n",
    catch(evaluate(m[1]?.moo))
  );
  ASSERT_EQ(
    "*Value being indexed is zero.\n",
    catch(evaluate(m[this_player()]?.moo))
  );
  ASSERT_EQ(
    "*Value being indexed is zero.\n",
    catch(evaluate(m[0]?.moo))
  );

  // Now let's plug in our mapping!
  m = orig;

  ASSERT_EQ(num, m?.[1]);
  ASSERT_EQ(nums, m?.[this_player()]);

  ASSERT_EQ(undefined, m?.[1]?.moo);
  ASSERT_EQ(undefined, m?.[this_player()]?.moo);
  ASSERT_EQ(undefined, m[1]?.moo);
  ASSERT_EQ(undefined, m[this_player()]?.moo);
  ASSERT_EQ(undefined, m?.key?.foo);

  // Testing for undefined as a key
  ASSERT_EQ(([
      "defined": "yes"
    ]),
    m.key[undefined]
  );
  ASSERT_EQ("yes", m.key[undefined]?.defined);
  ASSERT_EQ("yes", m.key[undefined]?.defined);

  // Optional chaining with bracket-then-dot on missing keys inside a present mapping
  ASSERT_EQ(undefined, m?.key?.missing?.deep);
  ASSERT_EQ(undefined, m.key?.missing?.deep);
  ASSERT_EQ(undefined, m.key["missing"]?.deep);
}
