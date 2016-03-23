import sys
import os
import glob

def fct_result_file(fileName):
    f = open(fileName)
    results = []
    while True:
        line = f.readline().rstrip()
        if not line:
            break
        arr = line.split()
        if len(arr) >= 2:
            '''size, fct, dscp, sending rate, goodput'''
            results.append([int(arr[0]), int(arr[1])])
    f.close()
    return results

def goodput_result_file(fileName):
    f = open(fileName)
    results = []
    while True:
        line = f.readline().rstrip()
        if not line:
            break
        arr = line.split()
        if len(arr) >= 5:
            '''size, fct, dscp, sending rate, goodput'''
            results.append([int(arr[0]), int(arr[4])])
    f.close()
    return results

def fct_result_dir(dirName):
    results=[]
    for f in glob.glob(dirName+'*.txt'):
        results.extend(fct_result_file(f))
    return results

def goodput_result_dir(dirName):
    results=[]
    for f in glob.glob(dirName+'*.txt'):
        results.extend(goodput_result_file(f))
    return results

def average_result(input_tuple_list):
    input_list = [x[1] for x in input_tuple_list]
    if len(input_list) > 0:
        return sum(input_list) / len(input_list)
    else:
        return 0

def tail_result(input_tuple_list):
    input_list = [x[1] for x in input_tuple_list]
    input_list.sort()
    if len(input_list) > 0:
        return input_list[int(0.99 * len(input_list))]
    else:
        return 0

def print_fct_results(results):
     # (0, 100KB)
    small = filter(lambda x: x[0] < 100*1024, results)
    # (100KB, 10MB)
    medium = filter(lambda x: 100*1024 <= x[0] < 10240*1024, results)
    # (10MB, infi)
    large = filter(lambda x: x[0] >= 10240*1024, results)

    print '%d flows/requests overall average completion time: %d us' % (len(results), average_result(results))
    print '%d flows/requests (0, 100KB) average completion time: %d us' % (len(small), average_result(small))
    print '%d flows/requests (0, 100KB) 99th percentile completion time: %d us' % (len(small), tail_result(small))
    print '%d flows/requests [100KB, 10MB) average completion time: %d us' % (len(medium), average_result(medium))
    print '%d flows/requests [10MB, ) average completion time: %d us' % (len(large), average_result(large))

def print_goodput_results(results):
    print '%d flows/requests overall average goodput: %d Mbps' % (len(results), average_result(results))

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print 'Usages: %s [result file/directory]' % sys.argv[0]
        sys.exit()

    path = sys.argv[1]
    if os.path.isfile(path):
        print_fct_results(fct_result_file(path))
        #print '======================================================'
        print_goodput_results(goodput_result_file(path))
    elif os.path.isdir(path):
        if not path.endswith('/'):
            path = path + '/'
        print_fct_results(fct_result_dir(path))
        #print '======================================================'
        print_goodput_results(goodput_result_dir(path))
    else:
        print 'Illegal input path'
