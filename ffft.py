#
# FFFT : Fast Full Fourier Transform
#
# Marc Groenewegen 2020
# Original idea: Pieter Suurmond
#
# Take the FFT of an entire WAV file, modify parameters in the frequency
# domain and transform the entire spectrum back to the time domain
#

####################################################################
# WARNING: always look at the output file and turn down the volume
#          before you play it!
#
# The output may contain VERY LOUD parts!
####################################################################

import os
import sys
import numpy as np
from scipy.signal import get_window
from scipy.fftpack import fft,ifft
from scipy.io.wavfile import read
from scipy.io.wavfile import write
import matplotlib.pyplot as plt
import math
import random

def ffft(infile,outfile):

  INT16_FAC = (2**15)-1
  INT32_FAC = (2**31)-1
  INT64_FAC = (2**63)-1
  norm_fact = {'int16':INT16_FAC, 'int32':INT32_FAC, 'int64':INT64_FAC,'float32':1.0,'float64':1.0}

  # read input file and if necessary, convert it to mono
  (fs,x) = read(infile)
  #print x.shape
  # take only first (mono) channel
  x=x[:,0]
  #print x.shape
  x = np.float32(x)/norm_fact[x.dtype.name]
  #print x[:200]

  N=2
  while N < len(x) :
    N *= 2

  N = int(N/2) # largest power of 2 _below_ length of file

  x = x[:N] # truncate

  Y=fft(x)

  # decompose spectrum
  YMag=abs(Y)
  YPhase=np.angle(Y)

  # set the phase of every bin to zero
  # YPhase = 0;

  # reconstruct spectrum
  Y = YMag*(np.cos(YPhase) + 1j*np.sin(YPhase))

  # back to time domain
  y=ifft(Y)

  # write the result
  write(outfile,fs,np.real(y))


if __name__ == '__main__':
  if len(sys.argv) < 2:
    print("Please enter input filename")
    sys.exit()
  infile=sys.argv[1]
  basename=os.path.splitext(sys.argv[1])[0]
  extension=os.path.splitext(sys.argv[1])[1]
  outfile=basename + "_out" + extension
  print(outfile)
  ffft(infile,outfile)

