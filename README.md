# Parallel-Programming-with-MPI

This project is about denoising a noisy image using parallel programming.Full project description is in "Code" tab.
Scripts ( which are needed to convert images to text , text to images or to make noise in a given image) and some examples of input images and input text files are also given."main.cpp" is the part i implemented.["figure1.png"](https://github.com/muhammed-kaya-2016400234/Parallel-Programming-with-MPI/blob/master/figure_1.png) is the outcome of the code when the input is ["yinyang_noisy.txt"](https://github.com/muhammed-kaya-2016400234/Parallel-Programming-with-MPI/blob/master/input-output/yinyang_noisy.txt)which is the text file converted version of the noisy image(converted with "image_to_text.py").


Example input and output:

Input![input](https://github.com/muhammed-kaya-2016400234/Parallel-Programming-with-MPI/blob/master/input-output/yinyang_noisy.png)|Output <img src="(https://github.com/muhammed-kaya-2016400234/Parallel-Programming-with-MPI/blob/master/figure_1.png" alt="output" width="200" height="200">
-- | - 

HOW TO RUN:

To execute the program you should place the input in the directory where source code resides.
The program is executed with following commands:
-mpic++ -g main.cpp -o main -std=c++11
-mpiexec -n <number of processor> main <input file> <output file> <beta value> <pi value>


  
  
