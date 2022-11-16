import time
import serial
from urlib.request import urlopen
import re
import urllib
    ser = serial.Serial(
    port='/dev/ttyAMA0',
    baudrate = 9600,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=1
)
counter=0
ser.flushInput()
url1='https://api.thingspeak.com/update?api_key=6KCAKZJ5RMHW9A2Q=0'
while 1:
    data_string=ser.readline().decode()
    data_float= re.findall('\d+(?:\.\d+)?',data_string)
    NID=data_float[1]
    if NID=="1":
        Light=data_float[2]
        Temperature=data_float[3]
        motion=data_float[4]
    elif NID=="2":
        Light2=data_float[2]
        Temperature2=data_float[2]
    elif NID=="3":
        Water=data_float[2]
    counter=counter+1
    print (data_float)
    if counter>=3:
        upload1=urllib2.urlopen (url1+ Light +"&field2=0"+Temperature+"&field3=0"+motion +"&field4=0"+     +"&field5=0"+  "&field6=0"+  "&field7=0"+  "&field8=0"+ )
        upload1.read()
        upload1.close()
        counter=0