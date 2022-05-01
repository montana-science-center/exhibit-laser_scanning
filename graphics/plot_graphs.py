
import numpy as np
import matplotlib.pyplot as plot
import math


time  = np.arange(0, 50, 0.1);
x = np.sin(time *0.01)
y = np.sin(time)

plot.plot(x, y)

plot.savefig('plot.svg')
plot.show()