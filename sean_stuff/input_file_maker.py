from random import randint


for i in range(1,12):
    
    outputName = "./input/input_file_10^{0}.txt".format(i)
    file = open(outputName,'w')
    print("writing to file ",outputName, "with i of ", str(i))
    for j in range(1,pow(10,i)+1):
        #print(str(pow(10,i))+1))
        num = randint(-222222,222222)
        file.write(str(num)+'\n')


