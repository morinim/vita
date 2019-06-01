#!/usr/bin/env python3

import matplotlib.pyplot as plt
import numpy as np

plt.suptitle("Guess the function")
plt.xlabel("X")
plt.ylabel("Y")
plt.xticks(np.arange(-10, 10, step=2))
plt.ylim(-10.5, 10.5)
plt.grid(True, which="major")
plt.grid(True, which="minor", linestyle=":")
plt.minorticks_on()

s = np.arange(-10, 10, 2)
x = np.arange(-10, 8.01, .01)

plt.plot(s, s + np.sin(s), "ro")
#plt.plot(x, x + np.sin(x), "k")

plt.show()
#plt.savefig("symbolic_regression01.png")
