CXX ?= c++
CXXFLAGS ?= -O2 -Wall -Wextra -pedantic

TARGET = udp_ascii_listener
SOURCE = udp_ascii_listener.cc

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGET)
