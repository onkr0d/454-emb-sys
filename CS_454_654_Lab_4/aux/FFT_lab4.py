#####################################################
###                                               ###
### BU CAS CS454/654 Embedded Systems Development ###
###                                               ###
### Author: Renato Mancuso (BU)                   ###
###                                               ###
### File: FFT Generation/plotting demo            ###
###                                               ###
#####################################################

from scipy.fftpack import fft, fftshift, fftfreq
import numpy as np
import matplotlib.pyplot as plt

## Where to save generated figures
savepath = "/home/renato/BU/Classes/CS454_Sp24/Demos/Python/"

## Generic function to compute the FFT.
## @s: sampled signal for which to compute the FFT
## @fc: frequency of sampling in Hz
## returns: xf and yf ready for plotting


def getFFT(s, fc):
    # Sampling period
    Tc = 1 / fc
    # Number of samples
    N = len(s)

    # Compute y-axis values
    yf = fftshift(fft(s) / (N / 2))

    # Shift and normalize x-axis values
    xf = fftshift(fftfreq(N, Tc))

    return xf, yf


def plotSignal(x, y, title):
    plt.title(title)
    plt.xlabel("time (s)")
    plt.ylabel("$s(t)$ value")
    plt.plot(x, y)
    plt.grid()
    plt.show()


def plotFFT(xf, yf, extra_label=""):
    plt.title("Fast Fuorier Transform (FFT) " + extra_label)
    plt.xlabel("frenqeucy (Hz)")
    plt.ylabel("magnitude of harmonic")
    plt.plot(xf, np.abs(yf))
    plt.grid()
    plt.show()


### PART 0: Simple signal at 5000 Hz ###
# Generate time axis for a 5000 Hz sampled signal between 0 and 2 seconds
fc = 5000.0
Tc = 1 / fc
time_start = 0.0
time_end = 2.0
N = int((time_end - time_start) / Tc)
t = np.linspace(time_start, time_end, N, endpoint=False)

# Generate sampled signal
s = 1 + 0.8 * np.sin(2 * np.pi * 5 * t)

xf, yf = getFFT(s, fc)
plotSignal(t, s, "Simple signal: $s(t) = 1 + 0.8\sin(2\pi5t)$")
plotFFT(xf, yf, "of signal (no noise)")

### PART 1: Simple signal at 5000 Hz ###
# Generate time axis for a 5000 Hz sampled signal between 0 and 2 seconds
fc = 5000.0
Tc = 1 / fc
time_start = 0.0
time_end = 2.0
N = int((time_end - time_start) / Tc)
t = np.linspace(time_start, time_end, N, endpoint=False)

# Generate sampled signal
s = 1 + 0.8 * np.sin(2 * np.pi * 5 * t)

# Generate sampled noise
n = 0.4 * np.sin(2 * np.pi * 30 * t) + 0.2 * np.sin(2 * np.pi * 43 * t)
plotSignal(t, s, "Simple signal (no noise)")

xf, yf = getFFT(s + n, fc)
plotSignal(t, s + n, "Simple signal + noise")
plotFFT(xf, yf, "of signal + noise")

### PART 2: Simple signal at 50 Hz ###
# Generate time axis for a 50 Hz sampled signal between 0 and 1 seconds
fc = 50.0
Tc = 1 / fc
time_start = 0.0
time_end = 2.0
N = int((time_end - time_start) / Tc)
t = np.linspace(time_start, time_end, N, endpoint=False)

# Generate sampled signal
s = 1 + 0.8 * np.sin(2 * np.pi * 5 * t)

# Generate sampled noise
n = 0.4 * np.sin(2 * np.pi * 30 * t) + 0.2 * np.sin(2 * np.pi * 43 * t)
plotSignal(t, s, "Pure signal (fc = 50 Hz): $s(t) = 1 + 0.8\sin(2\pi5t)$")

xf, yf = getFFT(s + n, fc)
plotSignal(t, s + n, "Signal + Noise (fc = 50 Hz)")
plotFFT(xf, yf, "of signal + noise sampled at 50 Hz")
