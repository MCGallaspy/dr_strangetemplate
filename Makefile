GPP_CMD = g++ -O3 -Wall -Werror -std=c++11

all: case_study_1 case_study_2 case_study_3

assembly: case_study_1.s case_study_2.s

case_study_2: case_study_2.hpp case_study_2.cpp
	$(GPP_CMD) case_study_2.hpp case_study_2.cpp -o case_study_2

case_study_1: case_study_1.hpp case_study_1.cpp
	$(GPP_CMD) case_study_1.hpp case_study_1.cpp -o case_study_1

case_study_1.s: case_study_1
	objdump -S --disassemble case_study_1.exe > case_study_1.s

case_study_2.s: case_study_2
	objdump -S --disassemble case_study_2.exe > case_study_2.s

case_study_3: case_study_3.hpp case_study_3.cpp
	$(GPP_CMD) -std=c++14 case_study_3.hpp case_study_3.cpp -o case_study_3

clean:
	-rm *.s
	-rm *.exe
	-rm *.o