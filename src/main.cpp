#include <iostream>
#include <sstream>

#include <string.h>
#include <stdlib.h>

struct Token {
  enum Type {
    eOpen, eClose, eInt, eAtom, eFin
  } type;

  union Data {
    const char* s;
    int i;
  } data;

  Token(const Token& o) {
    type = o.type;
    data.i = o.data.i;

    if(o.type == eAtom) {
      data.s = strdup(data.s);
    }
  }

  Token(Type t)
    : type(t)
  {}

  Token(int v)
    : type(eInt)
  {
    data.i = v;
  }

  Token(const char* v)
    : type(eAtom)
  {
    data.s = strdup(v);
  }

  ~Token() {
    if(type == eAtom) free((void*)data.s);
  }

  static Token open() {
    return Token(eOpen);
  }

  static Token close() {
    return Token(eClose);
  }

  static Token fin() {
    return Token(eFin);
  }

  static Token number(int val) {
    return Token(val);
  }

  static Token atom(const char* val) {
    return Token(val);
  }

  Token& operator=(const Token& o) {
    if(o.type == eAtom) {
      free((void*)data.s);
    }

    type = o.type;
    data.i = o.data.i;

    if(o.type == eAtom) {
      data.s = strdup(data.s);
    }

    return *this;
  }

  void print() {
    switch(type) {
    case eOpen:
      std::cout << "OPEN\n";
      break;
    case eClose:
      std::cout << "CLOSE\n";
      break;
    case eFin:
      std::cout << "FIN\n";
      break;

    case eInt:
      std::cout << "NUMBER " << data.i << "\n";
      break;

    case eAtom:
      std::cout << "ATOM " << data.s << "\n";
      break;
    }
  }
};

Token tok(std::istream& in) {
  char c;

restart:
  c = in.get();

  if(c == '(') {
    return Token::open();
  }

  if(c == ')') {
    return Token::close();
  }

  if(c >= 48 && c <= 57) {
    int val = (int)(c - 48);

    c = in.get();

    while(c >= 48 && c <= 57) {
      val *= 10;
      val += (int)(c - 48);
      c = in.get();
    }

    in.putback(c);

    return Token::number(val);
  }

  if(!isspace(c) && isprint(c)) {
    std::ostringstream ss;

    while(!isspace(c) && isprint(c) && c != '(' && c != ')') {
      ss.put(c);
      c = in.get();
    }

    in.putback(c);
    return Token::atom(ss.str().c_str());
  }

  if(c == ' ' || c == '\t' || c == '\n') goto restart;

  return Token::fin();

}

int main(int argc, char** argv) {
  for(;;) {
    Token t = tok(std::cin);

    if(t.type == Token::eFin) return 0;

    t.print();
  }
}

