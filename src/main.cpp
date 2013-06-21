#include <iostream>

struct Token {
  enum Type {
    eOpen, eClose, eInt, eAtom, eFin
  } type;

  union Data {
    const char* s;
    int i;
  } data;

  static Token open() {
    Token t = { eOpen, {0} };
    return t;
  }

  static Token close() {
    Token t = { eClose, {0} };
    return t;
  }

  static Token fin() {
    Token t = { eFin, {0} };
    return t;
  }

  static Token number(int val) {
    Token t = { eInt, { .i = val} };
    return t;
  }

  static Token atom(const char* val) {
    Token t = { eAtom, { .s = val} };
    return t;
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

  if(c == '+') {
    return Token::atom("+");
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

