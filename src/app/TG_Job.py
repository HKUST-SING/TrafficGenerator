import os
import sys
import threading
import time
import socket
sys.path.append('/home/wei/TrafficGenerator/src/')
from control.TG_Control import masterPort, TG_Control_Server
from client.TG_Client import TG_Client
import TG_Result

user='root'
path='/home/wei/TrafficGenerator'

if __name__=='__main__':
    if len(sys.argv)<9:
        print 'Usage '+sys.argv[0]+' [JobID] [master] [workers file] [average throughput (Mbps)] [number of threads] [number of flows] [number of services (N)] [flow size CDF of service 1] ...[flow size CDF of service N]'
        sys.exit()

    jobID=sys.argv[1]
    master=sys.argv[2]
    workers=[x.rstrip() for x in open(sys.argv[3]).readlines()]
    throughput=int(sys.argv[4])
    threadNum=int(sys.argv[5])
    flowNum=int(sys.argv[6])
    serviceNum=int(sys.argv[7])
    cdfFileNames=sys.argv[8:]

    if len(cdfFileNames)!=serviceNum:
        print 'Unmatched flow size CDF files'

    if not master:
        print 'No master node'
        sys.exit()

    if len(workers)<2:
        print 'No enough workers. There should be at least two workers'
        sys.exit()

    '''List to String'''
    cdfFileNamesStr=''
    for f in cdfFileNames:
        cdfFileNamesStr=cdfFileNamesStr+f+' '

    '''Start server'''
    for worker in workers:
        cmd='ssh %s@%s \'cd %s/src/server/; make; nohup ./server 0.0.0.0 5001 %d >log.txt &\'' % (user, worker, path, socket.SOMAXCONN)
        print cmd
        os.system(cmd)
        print 'Start server at %s' % (worker)

    '''Test server'''
    for worker in workers:
        client=TG_Client()
        client.request(worker, 5001, 1, 0, 0)
        print 'Server %s works' % (worker)

    raw_input('Press any key to start experiment')

    '''Start experiment'''
    for worker in workers:
        resultFileName='%s/result/result_job_%s_%s.txt' % (path, jobID, worker.replace('.','_'))
        cmd='ssh %s@%s \'cd %s/src/client/; nohup python TG_Poisson_Process.py \
        %s %s %s %d %d %d %s %d %s>log.txt &\''\
        % (user, worker, path, master, worker, sys.argv[3], throughput, threadNum, flowNum, resultFileName, serviceNum, cdfFileNamesStr)
        os.system(cmd)

    '''Start TG_Control_Server thread and wait for all the workers to complete'''
    server=TG_Control_Server()
    t=threading.Thread(target=server.run, args=(masterPort, len(workers)))
    t.start()
    t.join()

    os.system('rm -r %s/result/result_job_%s' % (path, jobID))
    os.system('mkdir %s/result/result_job_%s' % (path, jobID))

    '''Copy experiment results to the master'''
    for worker in workers:
        resultFileName='%s/result/result_job_%s_%s.txt' % (path, jobID, worker.replace('.','_'))
        cmd='scp %s@%s:%s %s/result/result_job_%s/' % (user, worker, resultFileName, path, jobID)
        os.system(cmd)
        print 'Copy experiment results from %s' % (worker)
