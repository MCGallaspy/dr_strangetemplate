GPP_CMD = g++ -Wall -Werror -std=c++14

all: case_study_1

case_study_1: case_study_1.hpp case_study_1.cpp
	$(GPP_CMD) case_study_1.hpp case_study_1.cpp -o case_study_1