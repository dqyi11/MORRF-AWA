import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np  
import sys         

def viz(weights):
    fig = plt.figure()     
    ax = fig.add_subplot(111, projection='3d')
    pl = ax.scatter(weights[:,0], weights[:,1], weights[:,2], 'bs')
    ax.set_xlabel("Weight 1", fontsize='x-large')
    ax.set_ylabel("Weight 2", fontsize='x-large')
    ax.set_zlabel("Weight 3", fontsize='x-large')
    plt.show()


def load(filename):
 
    weights = []
    with open(filename, 'r') as f:
        for line in f:
            line = line.replace('\n','')      
            ss = line.split(' ')
            print ss
            weights.append([ float(ss[0]), float(ss[1]), float(ss[2]) ])
    return np.array(weights);

if __name__ == '__main__':

    filename = sys.argv[1]

    weights = load(filename)
    viz(weights)
