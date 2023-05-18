data_file = 'scope.csv';
data = csvread(data_file);
x = data(:,1);
x_new = x;

% Define the "add" regions
add_regions = [
    162, 174;
    469, 482;
    769, 783;
    1069, 1082;
    1359, 1370;
    1641, 1652;
    1915, 1927;
    2184, 2195
];

% Define the "subtract" regions
subtract_regions = [
    174, 183;
    482, 488;
    783, 793;
    1082, 1092;
    1370, 1380;
    1652, 1663;
    1927, 1936;
    2195, 2205
];

% Define the fixed numbers to be added and subtracted
add_number = 250; % Change this value as needed
subtract_number = 120; % Change this value as needed

% Perform addition and subtraction on the specified regions
for i = 1:size(add_regions, 1)
    start_index = add_regions(i, 1);
    end_index = add_regions(i, 2);
    x_new(start_index:end_index) = x_new(start_index:end_index) + add_number;
end

for i = 1:size(subtract_regions, 1)
    start_index = subtract_regions(i, 1);
    end_index = subtract_regions(i, 2);
    x_new(start_index:end_index) = x_new(start_index:end_index) - subtract_number;
end

% Write the modified vector to a file
csvwrite("scope_clean.csv", x_new);
