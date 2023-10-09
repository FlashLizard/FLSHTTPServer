from socket import *
serverName = '127.0.0.1'
serverPort = 5050
clientSocket = socket(AF_INET, SOCK_STREAM)
clientSocket.connect((serverName,serverPort))
print(f'Client is ready to send')
while(True):
    sentence = input('Input transform data:')
    if(sentence == 'quit'):
        break
    clientSocket.send(str.encode(sentence))
    recv = clientSocket.recv(1024)
    print(f'From Server:{bytes.decode(recv)}')
clientSocket.close()
print(f'Client is closed')