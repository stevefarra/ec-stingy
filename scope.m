% Read data from Serial Oscilloscope program
Fs = 360;
data = dlmread("scope.csv")(:,1);

% Optionally, plot the data
% plot(data)

fid = fopen('scope_in.txt', 'w');

% Loop through the elements of the data vector and write them to a text file
for i = 1:length(data)
    fprintf(fid, '%d\n', data(i));
end

% Close the file
fclose(fid);
