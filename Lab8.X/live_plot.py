#/************************************************/
#/*                                              */
#/*  Live Plotting Utility                       */
#/*                                              */
#/*  This quick & dirty python script performs   */
#/*  live-plotting of a data stream in input     */
#/*  via the signal.txt file.                    */
#/*                                              */
#/*  Author: Renato Mancuso (BU)                 */
#/*  Class: CS454/654 Embedded Syst. Devel.      */
#/*  Date: April 2020                            */
#/*                                              */
#/************************************************/

import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import style

style.use('fivethirtyeight')

fig = plt.figure()
ax1 = fig.add_subplot(1,1,1)

def animate(i):
    graph_data = open('signal.txt','r').read()
    lines = graph_data.split('\n')
    xs = []
    ys = []
    zs = []
    ws = []

    lines = lines[-100:]
    
    for line in lines:
        if len(line) > 1:
            split = line.split(",")
            if (len(split) >= 2):
                x, y = split[0:2]
                xs.append(float(x))

                if (y != ""):
                    ys.append(float(y))
                else:
                    ys.append(None)
                    
            if (len(split) >= 3):
                z = split[2]

                if (z != ""):
                    zs.append(float(z))
                else:
                    zs.append(None)
                    
            if (len(split) >= 4):
                w = split[3]
                if (w != ""):
                    ws.append(float(w))
                else:
                    ws.append(None)
                    
    ax1.clear()
    ax1.plot(xs, ys)

    if (len(zs) > 0):
        ax1.plot(xs, zs)

    if (len(ws) > 0):    
        ax1.plot(xs, ws)

ani = animation.FuncAnimation(fig, animate, interval=5)
plt.show()
