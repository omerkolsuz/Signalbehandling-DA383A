% Define the coefficients a and b based on your IIR filter
a = [...]; % replace with actual values
b = [...]; % replace with actual values

% Define the frequency range for plotting
f = linspace(0, Fs/2, 1000); % Replace Fs with the sampling frequency

% Calculate the frequency response
H = freqz(b, a, f, Fs); % This calculates the frequency response

% Plot the magnitude and phase
figure;
subplot(2,1,1);
plot(f, 20*log10(abs(H))); % Magnitude in dB
title('Magnitude Response');
xlabel('Frequency (Hz)');
ylabel('Magnitude (dB)');

subplot(2,1,2);
plot(f, angle(H)); % Phase response
title('Phase Response');
xlabel('Frequency (Hz)');
ylabel('Phase (Radians)');
