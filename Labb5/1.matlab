[y_orig, Fs] = audioread('ekg.mp3');
y_orig = y_orig(1:20000);
y = y_orig + rand(1,20000) * 0.5 - 0.25;
Y = fft(y);
y = filter(coefficients, 1, y);
% Compute the frequency axis
N = length(y);
frequencies = (0:N-1) * Fs / N;
magnitude_spectrum = abs(fft(y));
plot(frequencies, magnitude_spectrum);
title('Frequency Spectrum');
xlabel('Frequency (Hz)');
ylabel('Magnitude');
