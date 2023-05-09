% Read data from file
data = csvread("output.csv");

% Plot the first four datastreams
subplot(5,1,1);
plot(data(:,1));
title("Datastream 1");

subplot(5,1,2);
plot(data(:,2));
title("Datastream 2");

subplot(5,1,3);
plot(data(:,3));
title("Datastream 3");

subplot(5,1,4);
plot(data(:,4));
title("Datastream 4");

% Plot the fifth datastream and hold the plot
subplot(5,1,5);
plot(data(:,5));
title("Datastream 5");
hold on;

##% Plot the sixth datastream on the same plot
##plot(data(:,6));
##title("Datastream 5 and 6");
