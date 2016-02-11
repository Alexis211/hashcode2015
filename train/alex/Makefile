OBJ = main.o solve1.o solve2.o
OUT = train.bin

TGZ = train.tgz
SRCFILES = main.cpp solve1.cpp solve2.cpp main.hpp ../../util.hpp

include ../../Makefile.common

run:  $(TGZ) $(OUT)
	./$(OUT) < ../learn_and_teach.in > learn_and_teach.out
	./$(OUT) < ../logo.in > logo.out
	./$(OUT) < ../right_angle.in > right_angle.out
