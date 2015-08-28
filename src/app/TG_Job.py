import os
import sys
import threading
import time
import socket
sys.path.append('/home/wei/TrafficGenerator/src/')
from control.TG_Control import masterPort, TG_Control_Server
from client.TG_Client import TG_Client
from TG_Result import print_fct_results, fct_result_dir

user='root'
path='/home/wei/TrafficGenerator'

if __name__=='__main__':
    if len(sys.argv)<10:
        print 'Usage: '+sys.argv[0]+' [JobID] [master] [senders file] [receivers file] [average throughput (Mbps)] [number of threads] [number of flows] [number of services (N)] [flow size CDF of service 1] ...[flow size CDF of service N]'
        sys.exit()

    jobID=sys.argv[1]
    master=sys.argv[2]
    senders=[x.rstrip() for x in open(sys.argv[3]).readlines() if len(x.rstrip())>0 ]
    receivers=[x.rstrip() for x in open(sys.argv[4]).readlines() if len(x.rstrip())>0 ]
    throughput=int(sys.argv[5])
    threadNum=int(sys.argv[6])
    flowNum=int(sys.argv[7])
    serviceNum=int(sys.argv[8])
    cdfFileNames=sys.argv[9:]

    if len(cdfFileNames)!=serviceNum:
        print 'Unmatched flow size CDF files'

    if len(senders)==0:
        print 'No enough senders'
        sys.exit()

    if len(receivers)==0:
        print 'No enough receivers'
        sys.exit()

    '''List to String'''
    cdfFileNamesStr=''
    for f in cdfFileNames:
        cdfFileNamesStr=cdfFileNamesStr+f+' '

    '''Start senders'''
    for sender in senders:
        cmd='ssh %s@%s \'cd %s/src/server/; make; nohup ./server 0.0.0.0 5001 %d >log.txt &\'' % (user, sender, path, socket.SOMAXCONN)
        print cmd
        os.system(cmd)
        print 'Start server at %s' % (sender)

    '''Test senders'''
    for sender in senders:
        client=TG_Client()
        client.request(sender, 5001, 1, 0, 0)
        print 'Server %s works' % (sender)

    raw_input('Press any key to start experiment')

    '''Start experiment'''
    for receiver in receivers:
        resultFileName='%s/result/result_job_%s_%s.txt' % (path, jobID, receiver.replace('.','_'))
        cmd='ssh %s@%s \'cd %s/src/client/; nohup python TG_Poisson_Process.py \
        %s %s %s %d %d %d %s %d %s>log.txt &\''\
        % (user, receiver, path, master, receiver, sys.argv[3], throughput, threadNum, flowNum, resultFileName, serviceNum, cdfFileNamesStr)
        os.system(cmd)

    '''Start TG_Control_Server thread and wait for all the receivers to complete'''
    server=TG_Control_Server()
    t=threading.Thread(target=server.run, args=(masterPort, len(receivers)))
    t.start()
    t.join()

    os.system('rm -r %s/result/result_job_%s' % (path, jobID))
    os.system('mkdir %s/result/result_job_%s' % (path, jobID))

    '''Copy experiment results to the master'''
    for receiver in receivers:
        resultFileName='%s/result/result_job_%s_%s.txt' % (path, jobID, receiver.replace('.','_'))
        cmd='scp %s@%s:%s %s/result/result_job_%s/' % (user, receiver, resultFileName, path, jobID)
        os.system(cmd)
        print 'Copy experiment results from %s' % (receiver)

    '''Print FCT results'''
    print_fct_results(fct_result_dir('%s/result/result_job_%s/' % (path, jobID)))
