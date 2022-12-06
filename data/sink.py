# https://github.com/hknaydin/Contiki-OS-with-Python-Server-communication

import socket
# import struct
# import sys
# import logging.handlers

# UDP_LOCAL_IP = 'fd00::5'
UDP_LOCAL_IP = 'aaaa::1'
UDP_LOCAL_PORT = 5678
UDP_REMOTE_PORT = 8765

server = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.bind((UDP_LOCAL_IP, UDP_LOCAL_PORT))
server.listen(4)

client_socket, client_address = server.accept()
print(f"Connection from {client_address} has been established!")

while True:
    data, addr = client_socket.recvfrom(1024)
    print(data)
    # print("ip address :", addr[0] , "port :", addr[1]  )
    # print('Received', data," " , 'lenght :', len(data) )

    # print(data.encode("hex"))

    # if message_type(data) == 1:
    #     client_message = time_handler(len(data), data)
    #     sent = socket_rx.sendto(client_message, addr)
    #     print(sys.stderr, 'sent %s bytes back to %s' % (sent, addr))
    # else :
    #     print("ERROR: Message Type is not supported")
    
# #==========================================================
# log = logging.getLogger('JRC')
# log.setLevel(logging.ERROR)
# log.addHandler(logging.NullHandler())
# #==========================================================

# shared_key = "acbacbaaacbacbaa";

# # 2 + 8 + 16 byte 
# #==========================================================
# def time_handler(message_lenght, data):

#     message = "ha"
#     client_addr = ""
    
#     if message_lenght == 36:
#         for x in range(28,36):
#             client_addr += data[x]

#     elif message_lenght == 38:
#         for x in range(30,38):
#             client_addr += data[x]

#     print(len(client_addr))

#     message = message + client_addr + shared_key
#     print(message.encode("hex"), "leng :", len(message))

#     return message
# #==========================================================

# def message_type(data):
#     type = -1

#     if data[0] == "#":
#         type = 1
#     else :
#         type = 1

#     return type;
# #==========================================================
# def file_logger():
    
#     fileLogger = logging.handlers.RotatingFileHandler(
#         filename    = 'test.log',
#         mode        = 'w',
#         backupCount = 5,
#     )
    
#     fileLogger.setFormatter(
#         logging.Formatter(
#             '%(asctime)s [%(name)s:%(levelname)s] %(message)s'
#         )
#     )
#     consoleLogger = logging.StreamHandler()
#     consoleLogger.setLevel(logging.DEBUG)

#     log = logging.getLogger('JRC')
#     log.setLevel(logging.DEBUG)
#     log.addHandler(fileLogger)
#     log.addHandler(consoleLogger)    
# #==========================================================

# file_logger()
