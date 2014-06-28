#include <iostream>
#include <cstdlib>
#include <vector>
#include <math.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TextOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "../buffered_istream.hpp"

using namespace std;

class BufferedIstreamTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(BufferedIstreamTest);

  CPPUNIT_TEST(test_next);
  CPPUNIT_TEST(test_current);
  CPPUNIT_TEST(test_sharp_comment);

  CPPUNIT_TEST(test_oper);
  CPPUNIT_TEST(test_oper_sharp_comment);

  CPPUNIT_TEST(test_rewind_1);
  CPPUNIT_TEST(test_rewind_2);


  CPPUNIT_TEST_SUITE_END();

private:

public:

  BufferedIstreamTest()  {
    srand((unsigned) time(NULL));
  }
  ~BufferedIstreamTest() { }

  virtual void setUp() {
    FILE* f = fopen("tmp1","w");
    fprintf(f,
            "andy  \n"
            "bella\n"
            "# cecillia\n"
            "dolcy # dolce\n"
            "eve\n"
            );
    fclose(f);
    f = fopen("tmp2","w");
    fprintf(f,
            "1 2 3 4 5        \n"
            "        6    7   8\n"
            "9    10    \n"
            );
    fclose(f);
    f = fopen("tmp3","w");
    for (int i=0; i < 99; i++){
      // insert comment lines
      while(rand() % 5 != 0){
        while(rand() % 2 != 0) fprintf(f, "     # comment\n");
        while(rand() % 2 != 0) fprintf(f, "\n");
      }
      fprintf(f, "%d\t%g  # comment\n", i, (static_cast<double>(rand())/RAND_MAX) * pow(10, rand() % 20 - 10) * pow(-1, rand() % 2));
    }
    fclose(f);
  }

  virtual void tearDown() {
    remove("tmp1");
    remove("tmp2");
    remove("tmp3");
  }

  void test_next(){
    buffered_ifstream in("tmp1");
    vector<string> expected;
    expected.push_back("andy  \n");
    expected.push_back("bella\n");
    expected.push_back("# cecillia\n");
    expected.push_back("dolcy # dolce\n");
    expected.push_back("eve\n");
    int i=0;
    while(-1){
      string a = in.next();
      if(in.fail())break;
      CPPUNIT_ASSERT(i<5);
      CPPUNIT_ASSERT(a == expected.at(i));
      i++;
    }
  }

  void test_current(){
    buffered_ifstream in("tmp1");
    vector<string> expected;
    expected.push_back("andy  \n");
    expected.push_back("bella\n");
    expected.push_back("# cecillia\n");
    expected.push_back("dolcy # dolce\n");
    expected.push_back("eve\n");

    int i=0;
    while(-1){
      string a = in.next();
      if(in.fail())break;
      CPPUNIT_ASSERT(i<5);
      CPPUNIT_ASSERT(a == expected.at(i));
      while(rand() % 10 != 0){
        CPPUNIT_ASSERT(in.current() == expected.at(i));
        CPPUNIT_ASSERT(in.good());
        CPPUNIT_ASSERT(not in.eof());
      }
      i++;
    }
  }

  void test_sharp_comment(){
    buffered_ifstream in("tmp1", flags::sharp_comment);
    vector<string> expected;
    expected.push_back("andy  \n");
    expected.push_back("bella\n");
    expected.push_back("\n");
    expected.push_back("dolcy \n");
    expected.push_back("eve\n");

    int i=0;
    while(-1){
      string a = in.next();
      if(in.fail())break;
      CPPUNIT_ASSERT(i<5);
      CPPUNIT_ASSERT(a == expected.at(i));
      while(rand() % 10 != 0){
        CPPUNIT_ASSERT(in.current() == expected.at(i));
        CPPUNIT_ASSERT(in.good());
        CPPUNIT_ASSERT(not in.eof());
      }
      i++;
    }
  }

  void test_oper(){
    buffered_ifstream in("tmp2");
    int a,b;
    double c;
    in >> a;
    CPPUNIT_ASSERT(a==1);
    in >> a;
    CPPUNIT_ASSERT(a==2);
    in >> a >> b;
    CPPUNIT_ASSERT(a==3 && b==4);
    in >> c >> a >> b;
    CPPUNIT_ASSERT(a==6 && b==7);
    in >> a >> b;
    CPPUNIT_ASSERT(a==8 && b==9);
  }

  void test_oper_sharp_comment(){
    buffered_ifstream in("tmp3", flags::sharp_comment);
    for(int i=0; ; i++){
      int n = -1;
      double d;
      in >> n >> d;
      if(in.eof()) break;
      CPPUNIT_ASSERT(n == i);
    }
  }

  void test_rewind_1(){
    // rewind does not affect "next".
    buffered_ifstream in("tmp1", flags::sharp_comment);
    vector<string> expected;
    expected.push_back("andy  \n");
    expected.push_back("bella\n");
    expected.push_back("\n");
    expected.push_back("dolcy \n");
    expected.push_back("eve\n");

    int i=0;
    while(-1){
      string a = in.next();
      if(in.fail())break;
      CPPUNIT_ASSERT(i<5);
      CPPUNIT_ASSERT(a == expected.at(i));
      while(rand() % 10 != 0) {
        in.rewind();
        // rewind let the pointer to the front of the string, but next forces to read the next line.
        // thus this rewind has no effect.
      }
      i++;
    }
  }

  void test_rewind_2(){
    // rewind does affect ">>".
    buffered_ifstream in("tmp3", flags::sharp_comment);
    for(int i=0; ; i++){
      int n;
      double d;

      in >> n >> d;
      //      std::cout << ">>" << in.current() << std::endl;
      if(in.eof()) break;
      CPPUNIT_ASSERT(n == i);

      bool flag_rewinded = false, flag_nextline = false;

      while(rand() % 2 == 0) {
        in.rewind();
        flag_rewinded = true;
      }

      if( rand() % 2 == 0){
        in.next();
        flag_nextline = true;

        while(rand() % 2 == 0){
          in.rewind();
        }
      }

      // if rewinded, the line is reloaded.
      // however, next forces to go to the next line. Read and store it in the buffer.
      // the pointer is at the beginning; so rewind has no effect.
      if(flag_rewinded and not flag_nextline) i--;
    }
  }


};


CPPUNIT_TEST_SUITE_REGISTRATION(BufferedIstreamTest);

int main() {
  CppUnit::TextUi::TestRunner runner;
  runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
  CppUnit::Outputter* outputter =
    new CppUnit::TextOutputter(&runner.result(),std::cout);
  runner.setOutputter(outputter);
  return runner.run() ? 0 : 1;
}
