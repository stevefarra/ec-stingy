% Open and read the file
fid = fopen('scope.csv', 'r');
data = textscan(fid, '%s', 'Delimiter', '\n');
fclose(fid);

% Reverse the order of the lines
reversedData = flipud(data{1});

% Write the reversed lines to another file
fid = fopen('scope_reversed.csv', 'w');
for i = 1:numel(reversedData)
    fprintf(fid, '%s\n', reversedData{i});
end
fclose(fid);

