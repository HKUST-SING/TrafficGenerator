import sys
import os

''' Parse a file to get FCT and goodput results '''
def parse_file(file_name):
    results = []
    f = open(file_name)
    while True:
        line = f.readline().rstrip()
        if not line:
            break
        arr = line.split()
        '''size, fct, dscp, sending rate, goodput'''
        if len(arr) >= 5:
            '''[size, fct, goodput]'''
            results.append([int(arr[0]), int(arr[1]), int(arr[4])])
    f.close()
    return results

''' Get average result '''
def average_result(input_tuple_list, index):
    input_list = [x[index] for x in input_tuple_list]
    if len(input_list) > 0:
        return sum(input_list) / len(input_list)
    else:
        return 0

''' Get cumulative distribution function (CDF) result '''
def cdf_result(input_tuple_list, index, cdf):
    input_list = [x[index] for x in input_tuple_list]
    input_list.sort()
    if len(input_list) > 0 and cdf >= 0 and cdf <= 1:
        return input_list[int(cdf * len(input_list))]
    else:
        return 0

def average_fct_result(input_tuple_list):
    return average_result(input_tuple_list, 1)

def average_goodput_result(input_tuple_list):
    return average_result(input_tuple_list, 2)

def cdf_fct_result(input_tuple_list, cdf):
    return cdf_result(input_tuple_list, 1, cdf)

def cdf_goodput_result(input_tuple_list, cdf):
    return cdf_result(input_tuple_list, 2, cdf)


def print_result(results):
     # (0, 100KB)
    small = filter(lambda x: x[0] < 100 * 1024, results)
    # (100KB, 10MB)
    medium = filter(lambda x: 100 * 1024 <= x[0] < 10 * 1024 * 1024, results)
    # (10MB, infi)
    large = filter(lambda x: x[0] >= 10 * 1024 * 1024, results)

    print '%d flows/requests overall average completion time: %d us' % (len(results), average_fct_result(results))
    print '%d flows/requests (0, 100KB) average completion time: %d us' % (len(small), average_fct_result(small))
    print '%d flows/requests (0, 100KB) 99th percentile completion time: %d us' % (len(small), cdf_fct_result(small, 0.99))
    print '%d flows/requests [100KB, 10MB) average completion time: %d us' % (len(medium), average_fct_result(medium))
    print '%d flows/requests [10MB, ) average completion time: %d us' % (len(large), average_fct_result(large))
    print '%d flows/requests overall average goodput: %d Mbps' % (len(results), average_goodput_result(results))


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print 'Usages: %s <file1> [file2 ...]' % sys.argv[0]
        sys.exit()

    files = sys.argv[1:]
    final_results = []
    num_file_parse = 0

    for f in files:
        if os.path.isfile(f):
            final_results.extend(parse_file(f))
            num_file_parse = num_file_parse + 1

    if num_file_parse <= 1:
        print "Parse %d file" % num_file_parse
    else:
        print "Parse %d files" % num_file_parse

    print_result(final_results)
