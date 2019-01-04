TARGET = spi.pyd

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:%.cpp=%.o)
MDS  = $(SRCS:%.cpp=%.d)

ifeq ($(BUILD_MODE),debug)
BUILD_MODE_CFLAGS = \
    -g -DDEBUG
else
BUILD_MODE_CFLAGS = \
    -O2 -DNDEBUG
endif

CFLAGS = \
    $(BUILD_MODE_CFLAGS) \
    -mno-cygwin \
    -mwindows \
    -MD \
    -fno-common \
    -fno-exceptions \
    -fno-rtti \
    -I/c/python/include \
    -I/c/cfiler_src \
    -DWINVER=0x0500 \
    -D_WIN32_WINNT=0x0500 \
    -D_UNICODE=1 \
    -DUNICODE=1 \

LIBS = \
    -L/usr/lib/mingw \
    -L/c/python/libs \
    -lpython27 \
    -lgdi32 \
    -lstdc++ \
    -lmingw32 \

#-------------------------------------------------------------------

all: $(TARGET)

clean:
	rm -f $(OBJS) $(MDS) $(TARGET) *~

install: $(TARGET)
	@cp -upv spi.pyd ..

#-------------------------------------------------------------------

.SUFFIXES: .exe .pyd .o .cpp

#GPP=g++
#GPP=i686-pc-cygwin-g++-3
GPP=i686-w64-mingw32-g++

.cpp.o:
	$(GPP) $(CFLAGS) -o $@ -c $<

$(TARGET): $(OBJS)
	dllwrap --target=i386-mingw32 -k -def spi.def -o $@ $(OBJS) $(LIBS)
	#dllwrap --target=i386-mingw32 -k -def spi.def --driver-name g++ -o $@ $(OBJS) $(LIBS)
	strip $@

#-------------------------------------------------------------------

-include $(MDS)
