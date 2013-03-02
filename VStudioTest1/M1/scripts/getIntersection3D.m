% Returns the least square intersection estimation 3D of lines.
%   Lines are represented in a+v*t form.
%   A has a_i as column vecotors
%   V has v_i as column vecotors
% Returns
%   p vector of intersection
%   e vector of square erros
function [p,eS] = getIntersection3D(A,V)
sumN = zeros(3,3);
sumNa = zeros(3,1);

for i=1:size(A,2)
    a = A(:,i);
    v = V(:,i);
    v = v ./ norm(v); % in case it would not have unity length
    Ni = eye(3,3) - v*v';
    sumN = sumN + Ni;
    sumNa = sumNa + (Ni * a);
end

p = sumN^(-1) * sumNa;

% calculate errors for every line
for i=1:size(A,2)
    a = A(:,i);
    v = V(:,i);
    v = v ./ norm(v); % in case it would not have unity length
    Ni = eye(3,3) - v*v';
    eS(i) = (p-a)'*Ni*(p-a);
end
