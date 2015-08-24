import sys
import threading
import random
import math
import time
import datetime
from TG_Distribution import TG_Distribution
from TG_Client import TG_Client

class TG_Poisson_Thread(threading.Thread):
    def __init__(self, senders, dists, throughput, serviceNum):
        self.senders=senders    #sender list
        self.dists=dists    #flow size distributions of different services
        self.services=serviceNum    #number of services
        self.client=TG_Client() #client socket
        self.deficit=0  #deficit (us)
        avgFlowSize=0
        for dist in self.dists:
            avgFlowSize=avgFlowSize+dist.getAvgFlowSize()*1024*8    #KB to bit
        avgFlowSize=avgFlowSize/len(self.dists)
        self.avgReqRate=throughput*1024*1024/avgFlowSize    #average request rate (number per second)
        threading.Thread.__init__(self)

    '''
    return next time interval according to poisson process (microseconds)
    '''
    def nextTime(self):
        return -math.log(1.0-random.random())/self.avgReqRate*1000000

    def run(self):
        global FCTs, flows
        while True:
            '''We have generated enough flows'''
            if flows>=flowNum:
                break

            '''update flows'''
            mutex.acquire()
            flows=flows+1
            mutex.release()

            '''Generate request and measure FCT'''
            startTime=datetime.datetime.now()
            sender=self.senders[random.randint(0,len(self.senders)-1)]
            serviceID=random.randint(0,self.services-1) #randomly choose a service
            tos=serviceID*4 #client and server socket share the same ToS value
            size=self.dists[serviceID].genFlowSize()    #generate flow size according to distribution of this service
            fct=self.client.request(sender, 5001, size, tos, tos)
            FCTs.append([size, fct, serviceID]) #python list should be thread-safe
            endTime=datetime.datetime.now()
            expireTime=(endTime-startTime).total_seconds()*1000000  #expire time (us)

            sleepTime=self.nextTime()-expireTime-self.deficit
            if sleepTime>0:
                self.deficit=0
                time.sleep(sleepTime/1000000.0) #second to microsecond
            else:
                self.deficit=-sleepTime


if __name__=='__main__':
    if len(sys.argv)<8:
        print 'Usage '+sys.argv[0]+' [senders file] [average throughput (Mbps)] [number of threads] [number of flows] [result file] [number of services (N)] [flow size CDF of service 1] ...[flow size CDF of service N]'
    else:
        senderFileName=sys.argv[1]
        throughput=int(sys.argv[2])
        threadNum=int(sys.argv[3])
        flowNum=int(sys.argv[4])
        resultFileName=sys.argv[5]
        serviceNum=int(sys.argv[6])
        cdfFileNames=sys.argv[7:]

        '''Get destinations'''
        senderFile=open(senderFileName)
        senders=senderFile.readlines()
        senderFile.close()

        '''Get distribution of different services'''
        dists=[]
        for f in cdfFileNames:
            dists.append(TG_Distribution(f))

        '''Number of flows that have been generated'''
        flows=0
        '''FCT results'''
        FCTs=[]
        '''Lock'''
        mutex=threading.Lock()

        '''Start threads'''
        threads=[]
        for i in range(threadNum):
            t=TG_Poisson_Thread(senders, dists, throughput/threadNum, serviceNum)
            threads.append(t)
            t.start()

        '''Wait for all threads to finish'''
        for t in threads:
            t.join()

        '''Write FCT results to the file'''
        resultFile=open(resultFileName,'w')
        for result in FCTs:
            if len(result)==3:
                resultFile.write(str(result[0])+' '+str(result[1])+' '+str(result[2])+'\n')
            else:
                print 'result error'
        resultFile.close()
