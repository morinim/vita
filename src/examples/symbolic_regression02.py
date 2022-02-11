#!/usr/bin/env python3

import matplotlib.animation as animation
import matplotlib.pyplot as plt
import numpy as np

# First set up the figure, the axis, and the plot element we want to animate
fig, ax = plt.subplots()
ax.axis([-10.1,8.1,-10.5,10.0])

plt.suptitle("Guess the function")
plt.xlabel("X")
plt.ylabel("Y")
plt.xticks(np.arange(-10, 10, step=2))
plt.grid(True, which="major")
plt.grid(True, which="minor", linestyle=":")
plt.minorticks_on()

s = np.arange(-10, 10, 2)
plt.plot(s, s + np.sin(s), "ro")

np.seterr(divide='ignore')

populations = []
x = np.arange(-10, 8.1, .01)

def read_functions():
   with open("pop.txt") as fp:
      functions = []
      for line in fp:
         if line.startswith("----"):
            populations.append(functions)
            functions = []
         else:
            functions.append(np.vectorize(eval("lambda X1: " + line)))

read_functions()

graphs = []

def init():
   ax.set_title("Generation 0")

   global graphs
   graphs = []

   for f in populations[0]:
      g, = ax.plot(x, f(x), "k", linewidth=0.4)
      graphs.append(g)

   return tuple(graphs)


def update(i):
   ax.set_title("Generation " + str(i))

   for j, g in enumerate(graphs):
      f = populations[i][j]
      g.set_data(x, f(x))

   return tuple(graphs)

anim = animation.FuncAnimation(fig, update, init_func=init, frames=range(len(populations)), interval=1, blit=True, repeat=True)

plt.show()
#anim.save('line.gif', dpi=80, writer='imagemagick')
