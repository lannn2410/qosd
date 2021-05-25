LIB = -L/opt/ibm/ILOG/CPLEX_Studio1271/concert/lib/x86-64_linux/static_pic -L/opt/ibm/ILOG/CPLEX_Studio1271/cplex/lib/x86-64_linux/static_pic
INC = -I/opt/ibm/ILOG/CPLEX_Studio1271/concert/include -I/opt/ibm/ILOG/CPLEX_Studio1271/cplex/include
LOAD = -lconcert -lilocplex -lcplex

default: 
	g++ -std=c++11 *.cpp -o qos -DIL_STD $(INC) $(LIB) $(LOAD) -fopenmp -g