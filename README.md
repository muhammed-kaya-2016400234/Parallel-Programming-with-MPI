# Parallel-Programming-with-MPI

This project is about denoising a noisy image using parallel programming.Full project description is in "Code" tab.
Scripts ( which are needed to convert images to text , text to images or to make noise in a given image) are also given.

"main.cpp" is the part i implemented.

HOW TO RUN:

To execute the program you should place the input in the directory where source code resides.
The program is executed with following commands:
-mpic++ -g main.cpp -o main -std=c++11
-mpiexec -n <number of processor> main <input file> <output file> <beta value> <pi value>


  
  
