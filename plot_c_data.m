% Load the CSV file
data = csvread('output.csv');

% Extract columns 1, 4, 5, 6, and 7
col1 = data(:, 1);
col4 = data(:, 4);
col5 = data(:, 5);
col6 = data(:, 6);
col7 = data(:, 7);

% Create a single figure to contain all the plots
figure;

% Plot column 1 on the first subplot
subplot(4, 1, 1);
plot(col1);
title('Column 1');
xlabel('x-axis');
ylabel('y-axis');

% Plot column 4 on the second subplot
subplot(4, 1, 2);
plot(col4);
title('Column 4');
xlabel('x-axis');
ylabel('y-axis');

% Plot column 5 on the third subplot
subplot(4, 1, 3);
plot(col5);
title('Column 5');
xlabel('x-axis');
ylabel('y-axis');

% Plot columns 6 and 7 on the fourth subplot
subplot(4, 1, 4);
plot(col6, 'b'); % 'b' for blue color
hold on;
plot(col7, 'r'); % 'r' for red color
hold off;
title('Columns 6 and 7');
xlabel('x-axis');
ylabel('y-axis');
legend('Column 6', 'Column 7');

