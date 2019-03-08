/*
 Student Name: Muhammed Bera Kaya
 Student Number: 2016400234
 Compile Status: Compiling
 Working Status: Working
 Notes:
 */

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <iterator>
#include <math.h>
#include <mpi.h>
#include <random>

using namespace std;


int main(int argc, char** argv) {
    MPI_Init(NULL, NULL);
    // Find out rank, size
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);



    int N = world_size-1;   //N is the number of child processors
    int i, j;
    int rowperproc=200/N;   //shows with how many rows a child processor will deal with

    ifstream inpfile;
    inpfile.open(argv[1]);      //open input file given as argument

    ofstream out;
    out.open(argv[2]);          //open output file given as argument

    //printf("N: %d\n", N);

    double B=atof(argv[3]);    //B is Beta we used in probability calculations(it is taken from argument)
    double C=atof(argv[4]);    //C is Pi we used in probability calculations(it is taken from argument)

    double Gamma=(0.5)*log((1-C)/C);    //calculate gamma from pi



    default_random_engine gen;   //random number generator
    gen.seed(time(NULL));


    if (world_rank == 0) {  //if this is the master processor




        int arr[200][200];     //arr is the array of all pixels
        string line;
        j=0;
        while(getline(inpfile,line)){       //this loop takes the pixel values from the input file


            vector<string> tokens;
            istringstream iss(line);
            //split line according to whitespace and record them to vector "tokens"
            copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter(tokens));
            for(int z=0;z<200;z++){

                arr[j][z]=atoi(tokens[z].c_str());
            }

            j++;

        }

        //send child processors their rows
        for(i = 1 ; i <= N ; i++)
            MPI_Send(arr[rowperproc*(i-1)],rowperproc*200, MPI_INT, i, 0, MPI_COMM_WORLD);


        //receive from child proccessor ,final situation of  their rows
        for(int p=1;p<=N;p++) {
            MPI_Recv(arr[(p-1)*rowperproc], rowperproc * 200, MPI_INT, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        //print the pixel values to ouput file
        for(int k=0;k<200;k++){
            for(int y=0;y<199;y++){
                out<<arr[k][y]<<" ";
            }
            out<<arr[k][199]<<endl;
        }



    }
    else{           //if this is child processor


        int subarr[rowperproc][200];       //the beginning values of pixels of this child processor(as it is received by master)
        int arrz[rowperproc][200];         //the current values of pixels (it changes at every iteration of for loop below)


        //receive rows of thi processsor from master and  keep it in subarr
        MPI_Recv(subarr, rowperproc*200, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //copy subarr to arrz since they should be the same at the beginning
        for(int l=0;l<rowperproc;l++)
            for(int v=0;v<200;v++)
                arrz[l][v]=subarr[l][v];


        //main loop:Choose a random pixel at every iteration and flip it according to a probability
        for(int r=0;r<700000/N;r++) {
            int upperneighbor[200];     //upper neighbour row
            int lowerneighbor[200];     //lower neighbour row


            //below we provide the communication between child processors
            //alignment of sends and receives here are important to avoid deadlock
            //this part also provides that no processor executes the next iteration before others finish their iteration

            //if it is the first child processor ,it does not have an upper neighbour
            //communicate only with processor 2
            //take uppermost row of processor2 and save it to lowerneighbor array
            if(world_rank==1){
                MPI_Send(arrz[rowperproc-1],200, MPI_INT, 2, 0, MPI_COMM_WORLD);
                MPI_Recv(lowerneighbor, 200, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);



            } //if this the last child processor ,there is no lower neighbor
                //take the lowermost row of the previous child processor and save it to upperneighbour array
            else if(world_rank==N){
                MPI_Recv(upperneighbor, 200, MPI_INT, N-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(arrz[rowperproc-1],200, MPI_INT, N-1, 0, MPI_COMM_WORLD);


            }
                //else it has both upper and lower neighbors
                //take uppermost row of following processor and save it to lowerneighbor array
                //take the lowermost row of the previous child processor and save it to upperneighbour array
            else{
                MPI_Recv(upperneighbor, 200, MPI_INT, world_rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                MPI_Send(arrz[0],200, MPI_INT, world_rank-1, 0, MPI_COMM_WORLD);

                MPI_Send(arrz[rowperproc-1],200, MPI_INT, world_rank+1, 0, MPI_COMM_WORLD);
                MPI_Recv(lowerneighbor, 200, MPI_INT, world_rank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


            }

            uniform_int_distribution<int> distribution(0,199);  //produce random integers,from an uniformly distributed set between 0 and 199
                                                                //We use it to choose a  random column

            uniform_int_distribution<int> dis(0,rowperproc-1); //produce random integer between 0 and rowperproc-1
                                                                //we choose a random row of this processor with this

            int randcol=distribution(gen);  //column of the randomly choosed pixel
            int randrow=dis(gen);      //row of the randomly choosed pixel



            int count=0;   //count is incremented with every neighbor pixel of the randomly chosen pixel which has a value
                            //of 1 and decremented with if the neighbor has a value of -1
                            //we use this in probability calculation


            //following if else statements take the values of the neighbor pixels and adds them to "count".
            //We provide here; if the random pixel is on the side or corner of the picture,take only the valid neighbor pixels
            //into consideration(for example a pixel on the upper left corner does only have 3 neighbor pixels).
            //Besides,if the pixel has a neighbor pixel from another processor take the value from the upperneighbor
            //or from the lowerneighbor array else take it from arrz array

            if(world_rank==1){
                if(randrow==0) {
                    if(randcol==0) {
                        count+=(arrz[randrow][randcol + 1]);
                        count+=(arrz[randrow + 1][randcol]);
                        count+=(arrz[randrow + 1][randcol + 1]);
                    }
                    else if(randcol==199){
                        count+=(arrz[randrow][randcol-1]);

                        count+=(arrz[randrow+1][randcol-1]);
                        count+=(arrz[randrow+1][randcol]);
                    }
                    else{
                        count+=(arrz[randrow][randcol-1]);
                        count+=(arrz[randrow][randcol+1]);
                        count+=(arrz[randrow+1][randcol-1]);
                        count+=(arrz[randrow+1][randcol]);
                        count+=(arrz[randrow+1][randcol+1]);

                    }
                }

                else if(randrow==rowperproc-1){
                    if(randcol==0){
                        count+=(arrz[randrow-1][randcol]);
                        count+=(arrz[randrow-1][randcol+1]);
                        count+=(arrz[randrow][randcol+1]);
                        count+=(lowerneighbor[randcol]);
                        count+=(lowerneighbor[randcol+1]);
                    }
                    else if(randcol==199){
                        count+=(arrz[randrow-1][randcol]);
                        count+=(arrz[randrow-1][randcol-1]);
                        count+=(arrz[randrow][randcol-1]);
                        count+=(lowerneighbor[randcol]);
                        count+=(lowerneighbor[randcol-1]);
                    }
                    else{
                        count+=(arrz[randrow-1][randcol-1]);
                        count+=(arrz[randrow-1][randcol]);
                        count+=(arrz[randrow-1][randcol+1]);
                        count+=(arrz[randrow][randcol-1]);
                        count+=(arrz[randrow][randcol+1]);
                        count+=(lowerneighbor[randcol]);
                        count+=(lowerneighbor[randcol-1]);
                        count+=(lowerneighbor[randcol+1]);

                    }
                }
                else{
                    count+=(arrz[randrow-1][randcol-1]);
                    count+=(arrz[randrow-1][randcol]);
                    count+=(arrz[randrow-1][randcol+1]);
                    count+=(arrz[randrow][randcol-1]);
                    count+=(arrz[randrow][randcol+1]);
                    count+=(arrz[randrow+1][randcol-1]);
                    count+=(arrz[randrow+1][randcol]);
                    count+=(arrz[randrow+1][randcol+1]);

                }
            }
            else if(world_rank==N){
                if(randrow==0) {
                    if(randcol==0) {
                        count+=(arrz[randrow][randcol+1]);
                        count+=(arrz[randrow+1][randcol+1]);
                        count+=(arrz[randrow+1][randcol]);
                        count+=(upperneighbor[0]);
                        count+=(upperneighbor[1]);
                    }
                    else if(randcol==199){
                        count+=(arrz[randrow][randcol-1]);
                        count+=(arrz[randrow+1][randcol-1]);
                        count+=(arrz[randrow+1][randcol]);
                        count+=(upperneighbor[199]);
                        count+=(upperneighbor[198]);
                    }
                    else{
                        count+=(arrz[randrow+1][randcol-1]);
                        count+=(arrz[randrow+1][randcol]);
                        count+=(arrz[randrow+1][randcol+1]);
                        count+=(arrz[randrow][randcol-1]);
                        count+=(arrz[randrow][randcol+1]);
                        count+=(upperneighbor[randcol]);
                        count+=(upperneighbor[randcol-1]);
                        count+=(upperneighbor[randcol+1]);
                    }
                }

                else if(randrow==rowperproc-1){
                    if(randcol==0){
                        count+=(arrz[randrow-1][randcol]);
                        count+=(arrz[randrow-1][randcol+1]);
                        count+=(arrz[randrow][randcol+1]);


                    }
                    else if(randcol==199){
                        count+=(arrz[randrow-1][randcol]);
                        count+=(arrz[randrow-1][randcol-1]);
                        count+=(arrz[randrow][randcol-1]);

                    }
                    else{
                        count+=(arrz[randrow-1][randcol-1]);
                        count+=(arrz[randrow-1][randcol]);
                        count+=(arrz[randrow-1][randcol+1]);
                        count+=(arrz[randrow][randcol-1]);
                        count+=(arrz[randrow][randcol+1]);


                    }
                }
                else{
                    count+=(arrz[randrow-1][randcol-1]);
                    count+=(arrz[randrow-1][randcol]);
                    count+=(arrz[randrow-1][randcol+1]);
                    count+=(arrz[randrow][randcol-1]);
                    count+=(arrz[randrow][randcol+1]);
                    count+=(arrz[randrow+1][randcol-1]);
                    count+=(arrz[randrow+1][randcol]);
                    count+=(arrz[randrow+1][randcol+1]);

                }

            }
            else{
                if(randrow==0) {
                    if(randcol==0) {
                        count+=(arrz[randrow][randcol+1]);
                        count+=(arrz[randrow+1][randcol+1]);
                        count+=(arrz[randrow+1][randcol]);
                        count+=(upperneighbor[0]);
                        count+=(upperneighbor[1]);
                    }
                    else if(randcol==199){
                        count+=(arrz[randrow][randcol-1]);
                        count+=(arrz[randrow+1][randcol-1]);
                        count+=(arrz[randrow+1][randcol]);
                        count+=(upperneighbor[199]);
                        count+=(upperneighbor[198]);
                    }
                    else{
                        count+=(arrz[randrow+1][randcol-1]);
                        count+=(arrz[randrow+1][randcol]);
                        count+=(arrz[randrow+1][randcol+1]);
                        count+=(arrz[randrow][randcol-1]);
                        count+=(arrz[randrow][randcol+1]);
                        count+=(upperneighbor[randcol]);
                        count+=(upperneighbor[randcol-1]);
                        count+=(upperneighbor[randcol+1]);
                    }
                }
                else if(randrow==rowperproc-1){
                    if(randcol==0){
                        count+=(arrz[randrow-1][randcol]);
                        count+=(arrz[randrow-1][randcol+1]);
                        count+=(arrz[randrow][randcol+1]);
                        count+=(lowerneighbor[randcol]);
                        count+=(lowerneighbor[randcol+1]);
                    }
                    else if(randcol==199){
                        count+=(arrz[randrow-1][randcol]);
                        count+=(arrz[randrow-1][randcol-1]);
                        count+=(arrz[randrow][randcol-1]);
                        count+=(lowerneighbor[randcol]);
                        count+=(lowerneighbor[randcol-1]);
                    }
                    else{
                        count+=(arrz[randrow-1][randcol-1]);
                        count+=(arrz[randrow-1][randcol]);
                        count+=(arrz[randrow-1][randcol+1]);
                        count+=(arrz[randrow][randcol-1]);
                        count+=(arrz[randrow][randcol+1]);
                        count+=(lowerneighbor[randcol]);
                        count+=(lowerneighbor[randcol-1]);
                        count+=(lowerneighbor[randcol+1]);

                    }
                }
                else{
                    count+=(arrz[randrow-1][randcol-1]);
                    count+=(arrz[randrow-1][randcol]);
                    count+=(arrz[randrow-1][randcol+1]);
                    count+=(arrz[randrow][randcol-1]);
                    count+=(arrz[randrow][randcol+1]);
                    count+=(arrz[randrow+1][randcol-1]);
                    count+=(arrz[randrow+1][randcol]);
                    count+=(arrz[randrow+1][randcol+1]);

                }





            }

            //calculate probability
            double first=Gamma*subarr[randrow][randcol]*arrz[randrow][randcol];
            double second=B*arrz[randrow][randcol]*(count);

            double probability=(-2.0)*(first+second);

            uniform_real_distribution<double> dist(0.0,1.0);
            double randnum=dist(gen);   //select a random real number between 0 and 1.0
            //if log of this is be lower than the calculated probability,then we will flip the pixel value
            if(log(randnum)<probability){
                arrz[randrow][randcol]*=-1;
            }




        }

        //after all the iterations are done, send the final values of pixels to master processors
        MPI_Send(arrz[0],rowperproc*200, MPI_INT, 0, 0, MPI_COMM_WORLD);



    }
    MPI_Finalize();

}


