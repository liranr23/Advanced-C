
#format is target-name: target dependencies
#{-tab-}actions

# All Targets
all: task1

# Tool invocations
# Executable "task1e" depends on the files *.o.
task1: 
	g++ -o restaurant *.cpp -std=c++11 -Wall -Wno-vla -pedantic -march=core2 -Os -pipe -fstack-protector-all -g3 -Wl,-O,1,-z,combreloc

#tell make that "clean" is not a file name!
.PHONY: clean

#Clean the build directory
clean: 
	rm -f *.o task1


