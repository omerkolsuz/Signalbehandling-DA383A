n=0:1:80000; 
fs=40000;      

x1=sin(2*pi*500*n/fs);  
sound(x1,fs)
stem(n,x1)

x2=0.2*sin(2*pi*5000*n/fs);
x=x1+x2;
stem(n,x)
sound(x, fs)

y=ma(10, x);

stem(n, y,'r')
hold on
stem(n, x, 'b')
hold off
