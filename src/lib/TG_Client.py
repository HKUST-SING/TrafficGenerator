import socket
import datetime
import sys

'''
TG_Client is a python class to generate requests and measure flow completion times (FCT)
ip: IP address of the remote server who actually generates flows
port: TCP port of the remote server
'''
class TG_Client:
    def __init__(self, ip, port):
        self.ip=ip
        self.port=port

'''
size: flow size (KB)
clientToS: ToS value of client socket (traffic from the client to the remote server)
serverToS: ToS value of server socket (traffic from the remote server to the client)
return FCT results (microseconds)
'''
    def request(self, size, clientToS, serverToS):
        s=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        if clientToS>0 and clientToS<=255:
            s.setsockopt(socket.IPPROTO_IP, socket.IP_TOS, clientToS)
        s.connect((self.ip, self.port))

        startTime=datetime.datetime.now()
        s.send(str(size)+' '+str(serverToS))
        while True:
            data=s.recv(8192)
            if not data:
                break
        endTime=datetime.datetime.now()
        s.close()

        fct=endTime-startTime
        return int(fct.total_seconds()*1000000)

'''
A simple example
'''
if __name__=='__main__':
    if len(sys.argv)==6:
        client=TG_Client(sys.argv[1],int(sys.argv[2]))
        print 'FCT is '+str(client.request(int(sys.argv[3]),int(sys.argv[4]),int(sys.argv[5])))+' us'
    else:
        print "Usage: "+sys.argv[0]+" [server IP] [server port] [flow size(KB)] [clientToS] [serverToS]"
