
CPP := clang++
CFLAGS := -std=c++11 -Wall -Werror

all : main

main : bitmap main.cc
	$(CPP) $(CFLAGS) main.cc -o main

bitmap : Bitmap.h

clean:
	rm main.o