figure 
scatter3(beforeweights(:,1), beforeweights(:,2), beforeweights(:,3), '.b')
xlabel('objective 1')
ylabel('objective 2')
zlabel('objective 3')
title('before WS')
view(121,35)
figure 
scatter3(afterweights(:,1), afterweights(:,2), afterweights(:,3), '.b')
xlabel('objective 1')
ylabel('objective 2')
zlabel('objective 3')
title('after WS')
view(121,35)