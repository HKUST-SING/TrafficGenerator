import xmlrpclib
import thread
import os
import sys
import argparse
import datetime
import time
from SocketServer import ThreadingMixIn
from SimpleXMLRPCServer import SimpleXMLRPCServer
from threading import Lock

class MyXMLRPCServer(ThreadingMixIn, SimpleXMLRPCServer):
    pass

def gen_conf_file(lines, host, id):
    content = ''
    for line in lines:
        if line.startswith('req_size_dist '):
            content = content + 'req_size_dist conf/dist_%s_%s' % (id, host.replace('.', '_'))
        elif not line.startswith('server ' + host):
            content = content + line
    return content

def finish_task(fin_worker):
    mutex.acquire()
    print '[%d] %s finishes at %s' % (len(unfin_workers), fin_worker, datetime.datetime.now())
    unfin_workers.remove(fin_worker)
    mutex.release()

if __name__ == "__main__":
    global mutex
    mutex = Lock()

    worker_port = 8000
    sleep_secs = 3
    client = 'bin/client'
    result_script = 'bin/result.py'

    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--id", help = "ID of job (required)")
    parser.add_argument("-b", "--bandwidth", help = "expected per-host RX bandwidth (Mbps) (required)", type = int)
    parser.add_argument("-c", "--conf", help = "configuration file name (required)")
    parser.add_argument("-n", "--number", help = "number of per-host requests (instead of -t)", type = int)
    parser.add_argument("-t", "--time", help = "time in seconds to generate requests (instead of -n)", type = int)
    parser.add_argument("-s", "--seed", help = "seed to generate random numbers", type = int)
    parser.add_argument("-a", "--address", help = "address of master node (IP:port) (required)")

    args = parser.parse_args()

    job_id = args.id
    bw = args.bandwidth
    conf_file = args.conf
    exp_num = args.number
    exp_time = args.time
    seed = args.seed
    master_addr = args.address

    error = False
    if not job_id or not bw or not conf_file or not master_addr:
        print 'Some required arguments (id, bandwidth, conf, address) are missing'
        error = True
    if (not exp_num and not exp_time) or (exp_num and exp_time):
        print 'You need to specify either the number of requests (-n) or the time to generate requests (-t)'
        error = True
    if master_addr and (len(master_addr.split(':')) != 2 or not master_addr.split(':')[1].isdigit()):
        print 'Invalid master address (IP:port) %s' % master_addr
        error = True

    if error:
        sys.exit(1)

    f = open(conf_file, 'r')
    lines = f.readlines()
    f.close()

    workers = []
    req_size_file = None
    for line in lines:
        if line.startswith("server ") and len(line.split()) == 3:
            workers.append(line.split()[1])
        elif line.startswith("req_size_dist ") and len (line.split()) == 2:
            req_size_file = line.split()[1]

    if len(workers) == 0 or not req_size_file:
        print 'Invalid configuration file'
        sys.exit(1)

    '''All the workers are unfinished now'''
    unfin_workers = workers[:]

    '''Copy required files to each worker'''
    for worker in workers:
        url = 'http://%s:%d' % (worker, worker_port)
        proxy = xmlrpclib.ServerProxy(url, allow_none = True)

        '''Copy request size distribution files'''
        filename = 'conf/dist_%s_%s' % (job_id, worker.replace('.', '_'))
        f = open(req_size_file, 'r')
        content = xmlrpclib.Binary(f.read())
        f.close()
        proxy.write_file(filename, content)
        print 'write file: %s @ %s' % (filename, worker)

        '''Copy configuration files'''
        filename = 'conf/conf_%s_%s' % (job_id, worker.replace('.', '_'))
        proxy.write_file(filename, xmlrpclib.Binary(gen_conf_file(lines, worker, job_id)))
        print 'write file: %s @ %s' % (filename, worker)

    print '======================================='

    '''Start RPC server'''
    ip = master_addr.split(':')[0]
    port = int(master_addr.split(':')[1])
    server = MyXMLRPCServer((ip, port), logRequests = False, allow_none = True)
    server.register_function(finish_task, 'finish_task')

    '''Run tasks'''
    for worker in workers:
        url = 'http://%s:%d' % (worker, worker_port)
        proxy = xmlrpclib.ServerProxy(url, allow_none = True)
        conf_file_name = 'conf/conf_%s_%s' % (job_id, worker.replace('.', '_'))
        log_file_name = 'result/job_%s_%s' % (job_id, worker.replace('.', '_'))
        proxy.run_task(sleep_secs, worker, client, bw, conf_file_name, exp_num, exp_time, \
                       seed, log_file_name, master_addr)
        print 'Start task on %s' % worker

    print '======================================='

    '''Wait for tasks to finish'''
    if len(workers) > 1:
        print 'Wait for %d tasks to finish' % len(workers)
    else:
        print 'Wait for 1 task to finish'

    for i in range(len(workers)):
        server.handle_request()

    '''Wait fot unfin_workers to be empty'''
    while len(unfin_workers) > 0:
        time.sleep(0.1)

    print '======================================='

    result_dir = 'result/job_%s' % job_id
    if not os.path.exists(result_dir):
        os.makedirs(result_dir)
        print 'mkdir %s' % result_dir

    '''Fetch result files to master node'''
    for worker in workers:
        print 'Fetch results from %s' % worker
        url = 'http://%s:%d' % (worker, worker_port)
        proxy = xmlrpclib.ServerProxy(url, allow_none = True)
        log_file_name = 'result/job_%s_%s' % (job_id, worker.replace('.', '_'))
        content = proxy.read_file(log_file_name)
        with open('result/job_%s/%s' % (job_id, worker.replace('.', '_')), 'w') as handle:
            handle.write(content.data)

    print '======================================='

    '''Parse results'''
    print 'Parse results on %s' % result_dir
    os.system('python %s %s/*' % (result_script, result_dir))
