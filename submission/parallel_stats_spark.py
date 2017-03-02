import sys
import numpy as np
from pyspark import SparkContext
from time import time

if __name__=="__main__":

    #this usage is for running one file at a time 
    #if len(sys.argv) != 2:
    #    print "Usage: script.py <input file>\n Options are: 1,2,3,4,5,6"
    #    exit(-1)

    #else:
    #   file_size = int(sys.argv[1])

    #this is where the timing results will go
    output_file_name= "output_file_spark.txt"
    output_file = open(output_file_name,'w')

    #this is the spark context
    sc = SparkContext("local","comp_systems")

    #input_file_name = "input_file_10^{0}.txt".format(file_size)

    #run reduce functions on every file
    for file_size in range(1,8):

        input_file_name = "./input/input_file_10^{0}.txt".format(file_size)
        data = np.loadtxt(input_file_name)
        
        distData = sc.parallelize(data)

        
        start = time()

        sum_data = distData.reduce(lambda a, b: a+b)
        #sum_data = distData.mapValues(lambda (a, b): a+b).collect()
        min_data = distData.reduce(lambda smallest, current: smallest if (smallest<current) else current)
        max_data = distData.reduce(lambda largest, current: largest if (largest>current) else current)

        end = time()        
        time_taken = round(end-start, 3)
        
        output_file.write("File of size 10^{0} has\nsum: {1}\n min: {2}\n max: {3}\ntime_taken: {4}\n\n".format(str(file_size),str(sum_data),str(min_data),str(max_data),str(time_taken)))

        #print "Done with sum of {0}, min of {1}, max of {2}, time of {3} sec!".format(str(sum_data),str(min_data),str(max_data),str(time_taken))