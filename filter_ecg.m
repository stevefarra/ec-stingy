% Sample rate of 200 Hz recommended by paper
ts = 200;
syms z

% Low-pass transfer function from updated version of paper
lpf_expr = (1-z.^-32)/(1-z.^-1);

% Express transfer function in general form and display it
[A,B] = numden(simplify(lpf_expr))

% We see the numerator is
% z^31 + z^30 + ... + z + 1
num = ones(1,32);

% And the denominator is
% z^31
den = zeros(1,32);
den(1) = 1;

% Create transfer function object for low-pass filter
lpf_tf = tf(num, den, ts);

% High-pass transfer function is defined as
hpf_expr = z.^-16 - lpf_expr/32;

% Repeat as earlier
[A,B] = numden(simplify(lpf_expr))

% We see the numerator is
% -z^31 - z^30 - ... - z - 1
num = -1*ones(1,32);

% And the denominator is
% 32z^31
den = zeros(1,32);
den(1) = 32;

% Create transfer function object for high-pass filter
hpf_tf = tf(num, den, ts);

% Finally, cascade the low-pass and high-pass filters to realize the
% band-pass filter
bpf_tf = lpf_tf*hpf_tf;

% Plot the result
h = bodeplot(lpf_tf);
setoptions(h,'FreqUnits','Hz','PhaseVisible','off');
datacursormode on