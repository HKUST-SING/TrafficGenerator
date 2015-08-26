import socket
import threading
import sys

masterPort=10086
finishWorkers=[]

class TG_Control_Server_Thread(threading.Thread):
    def __init__(self, socket):
        self.socket=socket
        threading.Thread.__init__(self)

    def run(self):
        data=self.socket.recv(8192)
        self.socket.close()
        print '%s finishes' % (data)
        finishWorkers.append(data)

class TG_Control_Server:
    def __init__(self):
        self.server=socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def run(self, port):
        self.server.bind(('0.0.0.0', port))
        self.server.listen(socket.SOMAXCONN)
        while True:
            clientsocket, addr=self.server.accept()
            t=TG_Control_Server_Thread(clientsocket)
            t.start()

    def stop(self):
        self.server.close()

class TG_Control_Client:
    def __init__(self):
        self.client=socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def send(self, master, port, worker):
        self.client.connect((master, port))
        self.client.sendall(worker)
        self.client.close()

if __name__=='__main__':
    server=TG_Control_Server()
    server.run(masterPort)
