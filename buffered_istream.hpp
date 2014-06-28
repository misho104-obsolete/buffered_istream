#ifndef BUFFERED_ISTREAM_HPP
#define BUFFERED_ISTREAM_HPP

#include <fstream>
#include <sstream>

namespace flags {
  enum buffered_istream_flags { none = 0, sharp_comment = 1 };
}

class buffered_istream {
protected:
  std::istream* ist;
  std::stringstream*  str;
  flags::buffered_istream_flags flag;
public:
  buffered_istream(std::istream* i = 0, flags::buffered_istream_flags f = flags::none);
  virtual ~buffered_istream();
  buffered_istream& open(std::istream* i);
  buffered_istream& close();

  std::string next();
  std::string current() const;
  buffered_istream& rewind();
  template <typename T> buffered_istream& operator>> (T& val);

  inline bool good() const { return ist->good(); }
  inline bool fail() const { return ist->fail(); }
  inline bool eof () const { return ist->eof(); }
  buffered_istream& set_flags(flags::buffered_istream_flags f = flags::none);
};

buffered_istream::buffered_istream(std::istream* i, flags::buffered_istream_flags f){
  if(i) open(i);
  str = new std::stringstream(std::stringstream::in | std::stringstream::out);
  str->setstate(std::ios::eofbit);
  set_flags(f);
}

buffered_istream::~buffered_istream(){
  if(str)   delete str;
}

buffered_istream& buffered_istream::open(std::istream* ist_in){
  ist = ist_in;
  return *this;
}

buffered_istream& buffered_istream::close(){
  ist = 0;
  str->clear();
  str->str("");
  return *this;
}

std::string buffered_istream::next(){
  str->clear();
  str->str("");
  char s;
  bool ignore = false;
  while(true){
    s = ist->get();
    if(ist->eof()) break;
    if((flag & flags::sharp_comment)){
      if(s == '#'){
        ignore = true;
      }else if(s == 10 or s == 13){
        ignore = false; // to put CR/LF
      }
    }
    if(not ignore) str->put(s);
    if(s == 10 or s == 13) break; // CR or LF
  }
  return str->str();
}

std::string buffered_istream::current() const {
  return str->str();
}

buffered_istream& buffered_istream::rewind() {
  str->clear();
  str->seekg(0, str->beg);
  return *this;
}

template <typename T> buffered_istream& buffered_istream::operator>> (T& val) {
  *str >> val;
  if(str->eof()){
    next();
    if(ist->eof()){ // next fails and everything is done.
      return *this;
    }
    return *this >> val;
  }
  return *this;
}

buffered_istream& buffered_istream::set_flags(flags::buffered_istream_flags f){
  flag = f;
  return *this;
}


class buffered_ifstream : public buffered_istream{
private:
  std::ifstream* file;
public:
  buffered_ifstream(const char *filename, flags::buffered_istream_flags f = flags::none);
  virtual ~buffered_ifstream();
  buffered_ifstream& open(const char* filename);
  buffered_ifstream& close();
};

buffered_ifstream::buffered_ifstream(const char *filename, flags::buffered_istream_flags f){
  open(filename);
  set_flags(f);
}

buffered_ifstream::~buffered_ifstream(){
  if(file){
    file->close();
    delete file;
  }
}

buffered_ifstream& buffered_ifstream::open(const char* filename){
  file = new std::ifstream(filename, std::ios::in);
  buffered_istream::open(file);
  return *this;
}

buffered_ifstream& buffered_ifstream::close(){
  file->close();
  delete file;
  file = 0;
  buffered_istream::close();
  return *this;
}

#endif
