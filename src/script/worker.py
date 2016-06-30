import xmlrpclib
import sys
import os
import thread
import time
import argparse
import traceback
from SocketServer import ThreadingMixIn
from SimpleXMLRPCServer import SimpleXMLRPCServer

class MyXMLRPCServer(ThreadingMixIn, SimpleXMLRPCServer):
    pass

def read_file(path):
    try:
        print 'read file %s' % os.path.abspath(path)
        f = open(path, "rb")
        data = xmlrpclib.Binary(f.read())
        f.close()
        return data
    except:
        traceback.print_exc(file = sys.stdout)
        return None

def write_file(path, content):
    try:
        print 'write file %s' % os.path.abspath(path)
        f = open(path, "wb")
        f.write(content.data)
        f.close()
        return True
    except:
        traceback.print_exc(file = sys.stdout)
        return False

def run_task(sleep_time, worker_id, client, bw, conf_file, exp_num, exp_time, seed, log_file, master_addr):
    thread.start_new_thread(_run_task, (sleep_time, worker_id, client, bw, conf_file, exp_num, exp_time, \
                            seed, log_file, master_addr))

def _run_task(sleep_time, worker_id, client, bw, conf_file, exp_num, exp_time, seed, log_file, master_addr):

    if sleep_time is not None:
        time.sleep(sleep_time)

    cmd = '%s -b %d -c %s' % (client, bw, conf_file)

    if exp_num > 0:
        cmd = cmd + ' -n ' + str(exp_num)
    elif exp_time > 0:
        cmd = cmd + ' -t ' + str(exp_time)
    else:
        print 'Either flow number or time should be larger than 0'
        return

    if seed:
        cmd = cmd + ' -s ' + str(seed)

    if log_file:
        cmd = cmd + ' -l ' + log_file

    os.system(cmd)
    proxy = xmlrpclib.ServerProxy("http://" + master_addr)
    proxy.finish_task(worker_id)

if __name__ == "__main__":
    default_port = 8000
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--port", help = "port number (default %d)" % default_port,  type = int)

    args = parser.parse_args()
    port = default_port
    if args.port:
        port = args.port

    print 'RPC server starts on 0.0.0.0:%d' % port
    server = MyXMLRPCServer(("0.0.0.0", port), allow_none = True)
    server.register_function(read_file, 'read_file')
    server.register_function(write_file, 'write_file')
    server.register_function(run_task, 'run_task')
    server.serve_forever()
