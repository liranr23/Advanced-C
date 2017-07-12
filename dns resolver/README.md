# Multithreaded DNS Name Resolution System

A program that get input files with hostnames and give the resolved ip.

## Instructions:

Open the terminal in the directory containing the files.

Run:
```bash
$export LD_LIBRARY_PATH=%FULL_PATH_TO_THIS_DIR%
```
(for example: export LD_LIBRARY_PATH=/home/user/dns resolver)
```bash
$g++ -o multi-lookup Driver.cpp -std=c++11 -Wall -Wno-vla -pedantic -lpthread -L. -lmyutil
```
You can add more customization to your compiling for example:
```bash
$g++ -o multi-lookup Driver.cpp -std=c++11 -Wall -Wno-vla -pedantic -march=core2 -Os -pipe -fstack-protector-all -g3 -Wl,-O,1,-z,combreloc -lpthread -L. -lmyutil
```
#### Execute file command:
```bash
$./multi-lookup filename1 filename2 resultfile
```
(for example[there is no limit of inputfiles]: ./multi-lookup names1.txt names2.txt results.txt)

#### NOTICE: the result file need to be created before(Bogus Output File)!

This programm implements Requester Threads, Resolver Threads and a Dumper Thread with the use of Thread Pools.

#### Example host names files are included in the input files directory.
