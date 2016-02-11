OBJ = main.o
OUT = qualification.bin

TGZ = qualification.tgz
SRCFILES = main.cpp ../../util.hpp

include ../../Makefile.common

run: $(TGZ) $(OUT)
	./$(OUT) < ../busy_day.in > busy_day.out
	./$(OUT) < ../mother_of_all_warehouses.in > mother_of_all_warehouses.out
	./$(OUT) < ../redundancy.in > redundancy.out
