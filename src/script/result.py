import sys
import os
import glob

def fct_result_file(fileName):
    f=open(fileName)
    results=[]
    while True:
        line=f.readline().rstrip()
        if not line:
            break
        arr=line.split()
        if len(arr)>=3:
            '''size fct service'''
            results.append([int(arr[0]), int(arr[1]), int(arr[2])])
    f.close()
    return results

def fct_result_dir(dirName):
    results=[]
    for f in glob.glob(dirName+'*.txt'):
        results.extend(fct_result_file(f))
    return results

def average_fct(input_tuple_list):
    input_list=[x[1] for x in input_tuple_list]
    if len(input_list)>0:
        return sum(input_list)/len(input_list)
    else:
        return 0

def tail_fct(input_tuple_list):
    input_list=[x[1] for x in input_tuple_list]
    input_list.sort()
    if len(input_list)>0:
        return input_list[int(0.99*len(input_list))]
    else:
        return 0

def print_fct_results(results):
     # (0, 100KB)
    small=filter(lambda x: x[0]<100*1024, results)
    # (100KB, 10MB)
    medium=filter(lambda x: 100*1024<=x[0]<10240*1024, results)
    # (10MB, infi)
    large=filter(lambda x: x[0]>=10240*1024, results)

    print '%d flows/requests overall average: %d us' % (len(results), average_fct(results))
    print '%d flows/requests (0, 100KB) average: %d us' % (len(small), average_fct(small))
    print '%d flows/requests (0, 100KB) 99th percentile: %d us' % (len(small), tail_fct(small))
    print '%d flows/requests [100KB, 10MB) average : %d us' % (len(medium), average_fct(medium))
    print '%d flows/requests [10MB, ) average : %d us' % (len(large), average_fct(large))

if __name__=='__main__':
    if len(sys.argv)!=2:
        print 'Usages: %s [result file/directory]' % sys.argv[0]
        sys.exit()

    path=sys.argv[1]
    if os.path.isfile(path):
        print_fct_results(fct_result_file(path))
    elif os.path.isdir(path):
        if not path.endswith('/'):
            path=path+'/'
        print_fct_results(fct_result_dir(path))
    else:
        print 'Illegal input path'
