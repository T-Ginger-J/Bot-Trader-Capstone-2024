 figure;
 hold on;
 set(gca,'Color','k');
 set(gca,'YLim',[-2 2]);
 % generate five seconds of data with frequency f
 fs = 200;
 dt = 1/fs;
 t  = linspace(0,5-dt,5*fs);
 f  = 2;
 a  = 1;
 y  = a*sin(2*pi*t*f);
 h = plot(t,y,'Color','g');
 while true
    % generate a new second of data
    tn = linspace(max(t)+dt,max(t)+1,fs);
    % randomly change the frequency
    if rand>0.6
        fn = randi(5,1,1);
    else
        fn = f;
    end
    yn = sin(2*pi*tn*fn);
    % remove the old data and replace with the new
    t = [t(fs+1:end) tn];
    y = [y(fs+1:end) yn];
    % update the plot
    set(h,'XData',t,'YData',y);
    pause(1);
 end