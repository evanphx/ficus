
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

CFLAGS += -Wall -std=c99
CXXFLAGS += -Wall -Weffc++ -Woverloaded-virtual -Wsign-promo -Werror
LDFLAGS += -lm

ifeq ($(uname_S),Linux)
  LDFLAGS += -lpthread
endif

ifneq ($(DEBUG),1)
	CXXFLAGS += -g -DDEBUG
endif

all: ficus

SRC=$(sort $(wildcard src/*.cpp))
OBJ=$(patsubst %.cpp,%.o,$(SRC))

ficus: $(OBJ) 
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJ)

clean:
	-rm ficus
	-rm src/*.o

distclean: clean
	-rm vendor/*.a

dep:
	: > depend
	for i in $(SRC); do $(CC) $(CXXFLAGS) -MM -MT $${i%.cpp}.o $$i >> depend; done

.PHONY: clean distclean dep

-include depend
