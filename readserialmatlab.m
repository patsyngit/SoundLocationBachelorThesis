    clear 
%Rozpocznij komunikację UART z Arduino UNO podłączonym do portu szeregowego COM3, z prędkością 2 Mb/s
    s = serialport("COM3",2000000);
    bytesRead=0;    %Liczba przeczytanych do tej pory bajtów.
    dataRead=[];    %Wektor przechowujący kolejno odczytane bajty.
    while bytesRead<50000   %Koontynuuj czytanie bajtów dopóki nie przeczytano wszystkich przesłanych bajtów.
        dataRead = [dataRead read(s,50,"uint8")];   %Odbierz 50 bajtów wysłanych poprzez UART i zapisz je w wektorze
        bytesRead=bytesRead+50; %Po przeczytaniu bajtów zaktualizuj liczbę aktualnie przeczytanych bajtów
    end
%Przekonwertuj przeczytane bajty do słów 16-bitowych (wymagane do wykoniania dalszych operacji bitowych).
    data16bit=cast(dataRead,'uint16')
%Stwórz wektor ch0, który przechowuje słowa z zakodowaną wartością napięcia z mikrofonu referencyjnego 0.
%Ciąg bajtów data16bit(1:5:end) przechowuje wartości 8 mniej znaczących bitów słowa z przetwornika MCP3008 (B8|B6|B5|B4|B3|B2|B1|B0). Ciąg bajtów data16bit(5:5:end) przechowuje wartości 2 najbardziej znaczacych bitów słów każdego kanału (B9|B8 z CH3, B9|B8 z CH2, B9|B8 z CH1, B9|B8 z CH0)
%Dokonując operacji bitowych na wyrazach ciągu otrzymane słowo ma postać (0|0|0|0|B10|B9|B8|B6|B5|B4|B3|B2|B1|B0) i jest równe pierwotnemu słowu zakodowanemu przez przetwornik MCP3008.
    ch0=bitor(data16bit(1:5:end),bitshift(bitand(data16bit(5:5:end),0x0003),8));
%Dokonaj analogicznych operacji i stwórz wektor ch1, który przechowuje słowa z zakodowaną wartością napięcia z mikrofonu 1.
    ch1=bitor(data16bit(2:5:end),bitshift(bitand(data16bit(5:5:end),0x000C),6));
%Dokonaj analogicznych operacji i stwórz wektor ch2, który przechowuje słowa z zakodowaną wartością napięcia z mikrofonu 2.
    ch2=bitor(data16bit(3:5:end),bitshift(bitand(data16bit(5:5:end),0x0030),4));
%Dokonaj analogicznych operacji i stwórz wektor ch3, który przechowuje słowa z zakodowaną wartością napięcia z mikrofonu 3.
    ch3=bitor(data16bit(4:5:end),bitshift(bitand(data16bit(5:5:end),0x00C0),2));
%Przekonwertuj słowo na wartość napięcia sygnału z mikrofonu referencyjnego 0
    ch0=cast(ch0*5/1024,'double')
%Przekonwertuj słowo na wartość napięcia sygnału z mikrofonu 1
    ch1=cast(ch1*5/1024,'double')
%Przekonwertuj słowo na wartość napięcia sygnału z mikrofonu 2
    ch2=cast(ch2*5/1024,'double')
%Przekonwertuj słowo na wartość napięcia sygnału z mikrofonu 3
    ch3=cast(ch3*5/1024,'double')
%Stałe
    dt=100*10^-6;  %czas próbkowania sygnałów z mikrofonów
    fs=1/dt;
    vs=34300;
    fsig=300;
    Tsig=1/fsig;
    Ti=Tsig/dt;
    t=0:dt:(length(ch0)-1)*dt;
    [M,Ich0] = max(ch0);
%Zapisz indeks elementu wektora ch0, który przyjmuje wartość maksymalną pod zmienną Ich0.
    [M,Ich1] = max(ch1);
%Zapisz indeks elementu wektora ch0, który przyjmuje wartość maksymalną pod zmienną Ich0.
    [M,Ich2] = max(ch2);
%Zapisz indeks elementu wektora ch0, który przyjmuje wartość maksymalną pod zmienną Ich0.
    [M,Ich3] = max(ch3);
    
    seq=sort([Ich3 Ich2 Ich1 Ich0]);
%Wyznacz wartości opóźnienia pomiędzy sygnałami z mikrofonów i mikrofonu referencyjnego
    peakt1=(Ich1-Ich0)*dt;
    peakt2=(Ich2-Ich0)*dt;
    peakt3=(Ich3-Ich0)*dt;
    
%Posortuj rosnąco indeksy, dla których napięcia z poszególnych mikrofonów przyjmują maksimum.
    sortUp=sort([Ich3 Ich2 Ich1 Ich0]);
%Określ przedział zawierający zarejestrowaną falę akusytczną. Zapobiegnie to niepotrzebnemu korelowaniu szumów.
    range=sortUp(1)-60:sortUp(1)+60;
%Wyznacz wartości funkcji korelacji wzajemnej sygnałów z mikrofonu referencyjnego 0 i mikrofonu 1 i zapisz je w wektorze c1.
    [c1,lags1]=xcorr(ch0(range),ch1(range));
%Zapisz indeks, dla którego funkcja korelacji wzajemnej sygnałów przyjmuje maksimum.
    [M,I1] = max(c1);
%Wyznacz wartość opóźnienia pomiędzy sygnałami z mikrofonu 0 i mikrofonu 1
    corrt1=(length(ch0(range))-I1)*dt;
%Wykonaj analogiczne operacje dla sygnałów z mikrofonu 0 i mikrofonu 2 wyznaczając wartość opóźnienia tau2.
    [c2,lags2]=xcorr(ch0(range),ch2(range));
    [M,I2] = max(c2);
    corrt2=(length(ch0(range))-I2)*dt;
%Wykonaj analogiczne operacje dla sygnałów z mikrofonu 0 i mikrofonu 3 wyznaczając wartość opóźnienia tau3.
    [c3,lags3]=xcorr(ch0(range),ch3(range));
    [M,I3] = max(c3);
    corrt3=(length(ch0(range))-I3)*dt;
    
    x0=0;   y0=0;   %Współrzędne mikrofonu referencyjnego 0
    x1=0;   y1=-20; %Współrzędne mikrofonu 1
    x2=0;   y2=-40; %Współrzędne mikrofonu 2
    x3=20;  y3=0;   %Współrzędne mikrofonu 3
    
    peakd1=peakt1*vs;     %Wyznaczenie wartości delta_1, zgodnie z równaniem (5).
    peakd2=peakt2*vs;     %Wyznaczenie wartości delta_2, zgodnie z równaniem (5).
    peakd3=peakt3*vs;     %Wyznaczenie wartości delta_3, zgodnie z równaniem (5).
%Wyznaczenie wartości parametrów z równiań (12), (13), (14)
    A=2*x1-2*x0;    B=2*y1-2*y0;    C=2*peakd1;    D=x1^2+y1^2-x0^2-y0^2-peakd1^2;
    E=2*x2-2*x0;    F=2*y2-2*y0;    G=2*peakd2;    H=x2^2+y2^2-x0^2-y0^2-peakd2^2;
    I=2*x3-2*x0;    J=2*y3-2*y0;    K=2*peakd3;    L=x3^2+y3^2-x0^2-y0^2-peakd3^2;
%Wyznaczenie współrzędnych x i y źródła dźwięku oraz jego odległości od początku układu współrzędnych - r0
    peakx=(L-J*D/B-B/(G*B-F*C)*(K-J*C/B)*(H-F*D/B))/(I-J*A/B+B/(G*B-F*C)*(K-J*C/B)*(F*A/B-E));
    peakr0=B/(G*B-F*C)*(H-F*D/B+peakx*(F*A/B-E));
    peaky=1/B*(D-C*peakr0-A*peakx);
    
%Wyznaczenie współrzędnych x i y źródła dźwięku oraz jego odległości od początku układu współrzędnych - r0
    corrd1=corrt1*vs;     %Wyznaczenie wartości delta_1, zgodnie z równaniem (5).
    corrd2=corrt2*vs;     %Wyznaczenie wartości delta_2, zgodnie z równaniem (5).
    corrd3=corrt3*vs;     %Wyznaczenie wartości delta_3, zgodnie z równaniem (5).
%Wyznaczenie wartości parametrów z równiań (12), (13), (14)
    A=2*x1-2*x0;    B=2*y1-2*y0;    C=2*corrd1;    D=x1^2+y1^2-x0^2-y0^2-corrd1^2;
    E=2*x2-2*x0;    F=2*y2-2*y0;    G=2*corrd2;    H=x2^2+y2^2-x0^2-y0^2-corrd2^2;
    I=2*x3-2*x0;    J=2*y3-2*y0;    K=2*corrd3;    L=x3^2+y3^2-x0^2-y0^2-corrd3^2;
%Wyznaczenie współrzędnych x i y źródła dźwięku oraz jego odległości od początku układu współrzędnych - r0
    corrx=(L-J*D/B-B/(G*B-F*C)*(K-J*C/B)*(H-F*D/B))/(I-J*A/B+B/(G*B-F*C)*(K-J*C/B)*(F*A/B-E));
    corrr0=B/(G*B-F*C)*(H-F*D/B+corrx*(F*A/B-E));
    corry=1/B*(D-C*corrr0-A*corrx);