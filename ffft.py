######################################################################
#          Copyright (c) 2020, Hogeschool voor de Kunsten Utrecht
#                      Hilversum, the Netherlands
#                          All rights reserved
#######################################################################
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.
#  If not, see <http://www.gnu.org/licenses/>.
#######################################################################
#
# FFFT : Fast Full Fourier Transform
#
# Marc Groenewegen 2020 <marc.groenewegen@hku.nl>
# Original idea: Pieter Suurmond <pieter.suurmond@hku.nl>
#
# Take the FFT of an entire WAV file, modify parameters in the frequency
# domain and transform the entire spectrum back to the time domain
####################################################################


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
  # and be very careful when playing the result!
  # YPhase = 0;

  # reconstruct spectrum
  Y = YMag*(np.cos(YPhase) + 1j*np.sin(YPhase))

  # back to time domain
  y=ifft(Y)

  # write the result
  write(outfile,fs,np.real(y))


if __name__ == '__main__':
  if len(sys.argv) < 3:
    print("Please enter filenames for input and output")
    sys.exit()
  infile=sys.argv[1]
  outfile=sys.argv[2]
  ffft(infile,outfile)

