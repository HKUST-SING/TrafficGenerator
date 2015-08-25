import sys
import threading
import random
import math
import time
import datetime
from TG_Distribution import TG_Distribution
from TG_Client import TG_Client

class TG_Poisson_Thread(threading.Thread):
    def __init__(self, senders, dists, throughput, serviceNum, process):
        self.senders=senders    #sender list
        self.dists=dists    #flow size distributions of different services
        self.services=serviceNum    #number of services
        self.process=process    #TG_Poisson_Process instance that creates this thread
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
        #global self.process.FCTs, self.process.flows
        while True:
            '''We have generated enough flows'''
            if self.process.flows>=self.process.flowNum:
                break

            '''update flows'''
            self.process.mutex.acquire()
            self.process.flows=self.process.flows+1
            self.process.mutex.release()

            '''Generate request and measure FCT'''
            startTime=datetime.datetime.now()
            sender=self.senders[random.randint(0,len(self.senders)-1)]
            serviceID=random.randint(0,self.services-1) #randomly choose a service
            tos=serviceID*4 #client and server socket share the same ToS value
            size=self.dists[serviceID].genFlowSize()    #generate flow size according to distribution of this service
            fct=self.client.request(sender, 5001, size, tos, tos)
            self.process.FCTs.append([size, fct, serviceID]) #python list should be thread-safe
            endTime=datetime.datetime.now()
            expireTime=(endTime-startTime).total_seconds()*1000000  #expire time (us)

            sleepTime=self.nextTime()-expireTime-self.deficit
            if sleepTime>0:
                self.deficit=0
                time.sleep(sleepTime/1000000.0) #second to microsecond
            else:
                self.deficit=-sleepTime

'''Generate many-to-one traffic using multiple threads'''
class TG_Poisson_Process:
    def __init__(self, senders, throughput, threadNum, flowNum, resultFileName, serviceNum, cdfFileNames):
        self.senders=senders
        self.throughput=throughput
        self.threadNum=threadNum
        self.flowNum=flowNum
        self.resultFileName=resultFileName
        self.serviceNum=serviceNum
        self.cdfFileNames=cdfFileNames

        if self.serviceNum!=len(self.cdfFileNames):
            print 'No enough flow size CDF files'
            sys.exit()

        '''Get distribution of different services'''
        self.dists=[]
        for f in self.cdfFileNames:
            self.dists.append(TG_Distribution(f))

        '''Number of flows that have been generated'''
        self.flows=0
        '''FCT results'''
        self.FCTs=[]
        '''Lock'''
        self.mutex=threading.Lock()
        self.threads=[]

    def run(self):
        '''Start threads'''
        for i in range(self.threadNum):
            t=TG_Poisson_Thread(self.senders, self.dists, self.throughput/self.threadNum, self.serviceNum, self)
            self.threads.append(t)
            t.start()

        '''Wait for all threads to finish'''
        for t in self.threads:
            t.join()

        '''Write FCT results to the file'''
        resultFile=open(self.resultFileName,'w')
        for result in self.FCTs:
            if len(result)==3:
                resultFile.write(str(result[0])+' '+str(result[1])+' '+str(result[2])+'\n')
            else:
                print 'result error'
        resultFile.close()


if __name__=='__main__':
    if len(sys.argv)<8:
        print 'Usage '+sys.argv[0]+' [senders file] [average throughput (Mbps)] [number of threads] [number of flows] [result file] [number of services (N)] [flow size CDF of service 1] ...[flow size CDF of service N]'
        sys.exit()

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

    process=TG_Poisson_Process(senders, throughput, threadNum, flowNum, resultFileName, serviceNum, cdfFileNames)
    process.run()
