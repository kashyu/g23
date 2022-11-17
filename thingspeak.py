import time
import serial
from urllib.request import urlopen
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
url1='https://api.thingspeak.com/update?api_key=6KCAKZJ5RMHW9A2Q&field=0'
while 1:
    data_string=ser.readline().decode()
    data_float= re.findall('\d+(?:\.\d+)?',data_string)
    NID=data_float[1]
    if NID=="1":
        Light=data_float[2]
        Temperature=data_float[3]
        Motion=data_float[4]
    elif NID=="3":
        Moisture=data_float[2]
        Temperature2=data_float[3]
        Light2=data_float[4]
    counter=counter+1
    print (data_float)
    if counter>=3:
        upload1=urllib2.urlopen (url1+Light+"&field2=0"+Temperature+"&field3=0"+Motion+"&field6=0"+Moisture+"&field7=0"+Temperature2+"&field8=0"+Light2)
        upload1.read()
        upload1.close()
        counter=0
