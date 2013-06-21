#include <iostream>
#include <sstream>

#include <string.h>
#include <stdlib.h>

#include <list>
#include <map>

struct Token {
  enum Type {
    eOpen, eClose, eInt, eSym, eFin
  } type;

  union Data {
    const char* s;
    int i;
  } data;

  Token(const Token& o) {
    type = o.type;
    data.i = o.data.i;

    if(o.type == eSym) {
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
    : type(eSym)
  {
    data.s = strdup(v);
  }

  ~Token() {
    if(type == eSym) free((void*)data.s);
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

  static Token sym(const char* val) {
    return Token(val);
  }

  Token& operator=(const Token& o) {
    if(o.type == eSym) {
      free((void*)data.s);
    }

    type = o.type;
    data.i = o.data.i;

    if(o.type == eSym) {
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

    case eSym:
      std::cout << "SYM " << data.s << "\n";
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

  if(isnumber(c)) {
    int val = (int)(c - 48);

    c = in.get();

    while(isnumber(c)) {
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
    return Token::sym(ss.str().c_str());
  }

  if(isspace(c)) goto restart;

  return Token::fin();
}

class Sym;
class Number;
class List;
class Env;

class Value {
public:
  virtual void print(std::ostream& os) = 0;
  virtual Value* eval(Env& env) { return this; }
  virtual Sym* as_sym() { return 0; }
  virtual Number* as_number() { return 0; }
  virtual List* as_list() { return 0; }
};

class Number : public Value {
  int num_;

public:

  Number(int n)
    : num_(n)
  {}

  int num() { return num_; }
  Number* as_number() { return this; }

  void print(std::ostream& os) {
    os << num_;
  }
};

class Sym : public Value {
  std::string str_;

public:
  Sym(std::string s)
    : str_(s)
  {}

  std::string& str() { return str_; }
  Sym* as_sym() { return this; }

  void print(std::ostream& os) {
    os << str_;
  }
};

class Function {
public:
  virtual Value* run(List* args) = 0;
};

typedef Value* (CFunction)(List* args);

class IFunction : public Function {
  CFunction* func_;

public:

  IFunction(CFunction* f)
    : func_(f)
  {}

  Value* run(List* args) {
    return (*func_)(args);
  }
};

class Env {
  std::map<std::string, Function*> funcs_;

public:

  void add(std::string s, Function* f) {
    funcs_[s] = f;
  }

  Function* find(std::string& s) {
    return funcs_[s];
  }
};

class List : public Value {
  Value* val_;
  List* next_;

public:
  List(Value* v, List* n=0)
    : val_(v)
    , next_(n)
  {}

  List* as_list() { return this; }

  Value* val() { return val_; }
  List* next() { return next_; }

  void print(std::ostream& os) {
    os << "(";

    List* n = this;

    if(n->val_) {
      n->val_->print(os);
    }

    n = n->next_;

    while(n) {
      if(n->val_) {
        os << " ";
        n->val_->print(os);
      }

      n = n->next_;
    }

    os << ")";
  }

  Value* eval(Env& env) {
    if(!val_) return this;
    if(Sym* sym = val_->as_sym()) {
      if(sym->str() == "quote") return next_;
      Function* func = env.find(sym->str());
      return func->run(next_);
    }

    return new List(0);
  }
};

List* make(std::istream& in) {
  std::list<Value*> l;

  for(;;) {
    Token t = tok(in);

    switch(t.type) {
    case Token::eFin:
    case Token::eClose:
      {
        List* cur = new List(0);

        for(std::list<Value*>::reverse_iterator i = l.rbegin();
            i != l.rend();
            ++i) {
          cur = new List(*i, cur);
        }

        return cur;
      }
    case Token::eOpen:
      l.push_back(make(in));
      break;
    case Token::eInt:
      l.push_back(new Number(t.data.i));
      break;
    case Token::eSym:
      l.push_back(new Sym(t.data.s));
      break;
    }
  }
}

Value* add(List* l) {
  int sum = 0;

  while(l) {
    if(l->val()) {
      if(Number* n = l->val()->as_number()) {
        sum += n->num();
      }
    }

    l = l->next();
  }

  return new Number(sum);
}

int main(int argc, char** argv) {
  if(argc == 2 && strcmp(argv[1], "-l") == 0) {
    for(;;) {
      Token t = tok(std::cin);

      if(t.type == Token::eFin) return 0;

      t.print();
    }

    return 0;
  }

  List* l = make(std::cin);

  if(argc == 2 && strcmp(argv[1], "-p") == 0) {
    l->print(std::cout);
    std::cout << "\n";
    return 0;
  }

  Env env;
  env.add("+", new IFunction(add));

  Value* out = l->val()->eval(env);

  out->print(std::cout);

  return 0;
}

