
class TestClass {
  int i;
  float ft;
  int *t;
}

class TestClass test_class;

void do_tests() {

  int i;
  float ft;
  mapping m;
 
  i = 1;
  ft = 1.0;
  test_class = new(class TestClass);
  
  test_class->i = i;
  test_class->ft = ft;
  test_class->t = ({ 1, 2, 3 });

  ASSERT(classp(test_class));
  ASSERT(intp(test_class->i));
  ASSERT(floatp(test_class.ft));
  
  ASSERT_EQ("class", typeof(test_class));

  ASSERT_EQ(test_class->i, i);
  ASSERT_EQ(test_class->ft, ft);
  
  ASSERT_EQ(test_class->i, test_class->i);
  ASSERT_EQ(test_class.i, test_class.i);
  ASSERT_EQ(test_class->i, test_class.i);
  ASSERT_NE(test_class->i, test_class->ft);
  ASSERT_NE(test_class.i, test_class.ft);
  ASSERT_NE(test_class->i, test_class.ft);

  ASSERT(intp(test_class->i));
  ASSERT(intp(test_class.i));
  ASSERT(floatp(test_class->ft));
  ASSERT(floatp(test_class.ft));
  
  ASSERT_NE(2, test_class->i++);
  ASSERT_NE(2.0, test_class->ft++);
  ASSERT_EQ(3, ++test_class->i);
  ASSERT_EQ(3.0, ++test_class->ft);
  
  ASSERT_NE(4, test_class.i++);
  ASSERT_NE(4.0, test_class.ft++);
  ASSERT_EQ(5, ++test_class.i);
  ASSERT_EQ(5.0, ++test_class.ft);
  
  test_class.i = ++i;
  test_class.ft = ++ft;
  
  ASSERT_EQ(test_class.i, i);
  ASSERT_EQ(test_class.ft, ft);
  
  ASSERT_EQ(i + ft, test_class->i + test_class.ft);
  
  ASSERT_EQ(([ i : ft ]), ([ test_class->i : test_class.ft ]));
    
  ASSERT_EQ(i + ft, test_class->i + test_class.ft);
  ASSERT_EQ(i + ft, test_class.i + test_class->ft);
  ASSERT_EQ(i + ft, test_class->i + test_class->ft);
  ASSERT_EQ(i + ft, test_class.i + test_class.ft);
  
  ASSERT_EQ(i - ft, test_class->i - test_class.ft);
  ASSERT_EQ(i - ft, test_class.i - test_class->ft);
  ASSERT_EQ(i - ft, test_class->i - test_class->ft);
  ASSERT_EQ(i - ft, test_class.i - test_class.ft);
  
  ASSERT_EQ(i * ft, test_class->i * test_class.ft);
  ASSERT_EQ(i * ft, test_class.i * test_class->ft);
  ASSERT_EQ(i * ft, test_class->i * test_class->ft);
  ASSERT_EQ(i * ft, test_class.i * test_class.ft);
  
  ASSERT_EQ(i / ft, test_class->i / test_class.ft);
  ASSERT_EQ(i / ft, test_class.i / test_class->ft);
  ASSERT_EQ(i / ft, test_class->i / test_class->ft);
  ASSERT_EQ(i / ft, test_class.i / test_class.ft);
  
  ASSERT_EQ(({ 3 }), test_class->t[test_class->i..test_class->i]);
  ASSERT_EQ(({ 3 }), test_class.t[test_class.i..test_class.i]);
  ASSERT_EQ(({ 2, 3 }), test_class.t[test_class.i/test_class.i..test_class.i]);
  ASSERT_EQ(({ 2, 3 }), test_class->t[test_class->i/test_class->i..test_class->i]);
  ASSERT_EQ(({ 1, 2, 3 }), test_class.t[test_class.i-test_class.i..test_class.i]);
  ASSERT_EQ(({ 1, 2, 3 }), test_class->t[test_class->i-test_class->i..test_class->i]);
  
  ASSERT_EQ(({ 1, test_class->i, 3 }), test_class->t);
  ASSERT_EQ(({ 1, test_class.i, 3 }), test_class->t);
  ASSERT_EQ(({ test_class.i/test_class.i, 2, 3 }), test_class->t);
  ASSERT_EQ(({ test_class->i/test_class->i, 2, 3 }), test_class->t);
  
  m = ([ i : ft ]);
  ASSERT_EQ(m[2], m[test_class->i]);
  ASSERT_EQ(m[2], m[test_class.i]);
  
  ASSERT_EQ(test_class->t[1], test_class->t[1]);
  ASSERT_EQ(test_class.t[1], test_class.t[1]);
  
  ASSERT_EQ(3, sizeof(test_class));  
}