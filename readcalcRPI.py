
import numpy as np
from scipy import signal
import math

dt=80/1000000
vs=34300

file = open("raspberry_12_5k_klasniecie0060.txt", "r")
#file = open("heh.txt", "r")
values = file.readlines()
values =[int(i.strip()) for i in values]

#values=[1,2,3,4,5,6,7,8,9,10,11,12]
ch0=np.asarray(values[0::4])
ch1=np.asarray(values[1::4])
ch2=np.asarray(values[2::4])
ch3=np.asarray(values[3::4])

Ich0= np.argmax(ch0)
Ich1= np.argmax(ch1)
Ich2= np.argmax(ch2)
Ich3= np.argmax(ch3)

t1peak=(Ich0-Ich1)*dt;
t2peak=(Ich0-Ich2)*dt;
t3peak=(Ich0-Ich3)*dt;

range=np.arange(Ich0-60, Ich0+61)
c1 = signal.correlate(ch0[Ich0-60:Ich0+2000], ch1[Ich0-60:Ich0+2000], mode="full")
lags1 = signal.correlation_lags(ch0[Ich0-60:Ich0+2000].size, ch1[Ich0-60:Ich0+2000].size, mode="full")
I1= np.argmax(c1)
t1=-(ch0[Ich0-60:Ich0+2000].size-I1)*dt

c2 = signal.correlate(ch0[Ich0-60:Ich0+2000], ch2[Ich0-60:Ich0+2000], mode="full")
lags2 = signal.correlation_lags(ch0[Ich0-60:Ich0+2000].size, ch2[Ich0-60:Ich0+2000].size, mode="full")
I2= np.argmax(c2)
t2=-(ch0[Ich0-60:Ich0+2000].size-I2)*dt

c3 = signal.correlate(ch0[Ich0-60:Ich0+2000], ch3[Ich0-60:Ich0+2000], mode="full")
lags3 = signal.correlation_lags(ch0[Ich0-60:Ich0+2000].size, ch3[Ich0-60:Ich0+2000].size, mode="full")
I3= np.argmax(c3)
t3=-(ch0[Ich0-60:Ich0+2000].size-I3)*dt

#a=math.sqrt(30*30+80*80);
#t1theory=-(math.sqrt(30*30+100*100)-a)/vs; t2theory=-(math.sqrt(30*30+120*120)-a)/vs; t3theory=-(math.sqrt(50*50+80*80)-a)/vs;
#a=math.sqrt(0*0+60*60);
#t1theory=-(math.sqrt(0*0+80*80)-a)/vs; t2theory=-(math.sqrt(0*0+100*100)-a)/vs; t3theory=-(math.sqrt(20*20+60*60)-a)/vs;
#a=math.sqrt(30*30+40*40);
#t1theory=-(math.sqrt(30*30+60*60)-a)/vs; t2theory=-(math.sqrt(30*30+80*80)-a)/vs; t3theory=-(math.sqrt(50*50+40*40)-a)/vs;
#a=math.sqrt(60*60+40*40);
#t1theory=-(math.sqrt(60*60+60*60)-a)/vs; t2theory=-(math.sqrt(60*60+80*80)-a)/vs; t3theory=-(math.sqrt(80*80+40*40)-a)/vs;
a=math.sqrt(30*30+20*20);
t1theory=-(math.sqrt(30*30+0*0)-a)/vs; t2theory=-(math.sqrt(30*30+20*20)-a)/vs; t3theory=-(math.sqrt(50*50+20*20)-a)/vs;

print("\nt1= %e" % t1," t1peak= %e" % t1peak," t1theory= %e" % t1theory)
print("\nt2= %e" % t2," t2peak= %e" % t2peak," t2theory= %e" % t2theory)
print("\nt3= %e" % t3," t3peak= %e" % t3peak," t3theory= %e" % t3theory)
print(ch0[Ich0-60:Ich0+2000].size)
