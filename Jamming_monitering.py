import numpy as np
import matplotlib.pyplot as plt 
import csv
import socket

f = open('GPS_EX.csv', 'r', encoding='utf-8')
d = csv.reader(f)

lat = []
lon = []
for line in d:
    lat.append(float(line[0]))
    lon.append(float(line[1]))

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("127.0.0.1", 11111))

change_lat = []
change_lon = []
while True:
    data = sock.recv(1024).decode()
    aa = data.split(',')
    print(aa)
    change_lat.append(float(aa[0]))
    change_lon.append(float(aa[1]))
    
    plt.cla()
    plt.title("Jamming Scenario")
    plt.xlabel("Longitude")
    plt.ylabel("Latitude")
    plt.plot(lon, lat, "-b", label="Ground Truth")
    plt.plot(change_lon, change_lat, "*r", label="GPS")
    plt.legend()
    plt.pause(0.001)

f.close()