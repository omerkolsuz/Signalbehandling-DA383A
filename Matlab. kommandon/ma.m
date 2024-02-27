function y = ma(M,x)
% Moving averagefilter
% M+1 terms
% for k = 1:M
% y(k)=0;
% end
y = zeros(1,M); % Makes for faster computations
for k = M+1:length(x)
y(k) = sum(x(k-M:k))/(M+1);
end
