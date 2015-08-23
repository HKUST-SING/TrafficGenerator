import sys
import random

'''
TG_Distribution is a python class to generated flow size according to a distribution
'''
class TG_Distribution:

    '''
    read flow size CDF distribution file
    '''
    def __init__(self, cdfFileName):
        self.distribution=[]
        cdfFile=open(cdfFileName)
        lines=cdfFile.readlines()
        for line in lines:
            arr=line.split()
            if len(arr)==2:
                self.distribution.append([float(arr[1]), float(arr[0])])
        cdfFile.close()

    '''
    return flow size (KB) according to self.distribution
    '''
    def genFlowSize(self):
        number=random.random()
        #For debug
        #print number
        i=0
        for var in self.distribution:
            if var[0]>number:
                x1=self.distribution[i-1][0]
                x2=self.distribution[i][0]
                y1=self.distribution[i-1][1]
                y2=self.distribution[i][1]
                value=int(((y2-y1)/(x2-x1)*number+(x2*y1-x1*y2)/(x2-x1))*1500/1024)
                return value
            elif var[0]==number:
                return var[1]
            else:
                i=i+1
        return 0

    '''
    return average flow size (KB) of self.distribution
    '''
    def getAvgFlowSize(self):
        avg=0
        for i in range(1, len(self.distribution)):
            avg=avg+(self.distribution[i][0]-self.distribution[i-1][0])*(self.distribution[i][1]+self.distribution[i-1][1])/2*1500/1024
        return avg

'''
A simple example
'''
if __name__=='__main__':
    if len(sys.argv)==2:
        dist=TG_Distribution(sys.argv[1])
        print 'Average flow size is '+str(dist.getAvgFlowSize())+' KB'
        print 'Generate a flow size: '+str(dist.genFlowSize())+' KB'
    else:
        print "Usage: "+sys.argv[0]+" [flow size CDF file]"
