# circle.py

import numpy as np
import matplotlib.pyplot as plt

in_string = "Hello, world! Lorem ipsum dolor sit amet. "
theta = np.arange(0, -len(in_string), -1) * 2*np.pi/(len(in_string)-1)

x = np.cos(theta+np.pi/2) * 100
y = np.sin(theta+np.pi/2) * 100 * .8

x = np.round(x)
y = np.round(y)

dx = np.diff(x).astype(int)
dy = np.diff(y).astype(int)
print(x)
print(y)

print(', '.join([f'{x:3d}' for x in dx]))
print(', '.join([f'{x:3d}' for x in dy]))

fig, ax = plt.subplots(1)
ax.plot(x, y, '.')
plt.show()