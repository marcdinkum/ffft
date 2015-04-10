/**********************************************************************
*          Copyright (c) 2015, Hogeschool voor de Kunsten Utrecht
*                      Hilversum, the Netherlands
*                          All rights reserved
***********************************************************************
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.
*  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************
*
*  File name     : ffft.cpp
*  System name   : Fast Full Fourier Transform
* 
*  Description   : experiments in spectral domain by taking frequency
*    transform of an entire fragment of music, changing spectral features
*    and transforming back
*
***********************************************************************
*          N.B.: turn down the volume with these experiments!
*        Serious risk of blowing up speakers or damaging your ears.
*         Best played in combination with visual representation.
***********************************************************************
*
*  Author             : Marc Groenewegen
*  E-mail             : marc.groenewegen@hku.nl
*  Inspiration/idea   : Pieter suurmond
*  E-mail             : pieter.suurmond@hku.nl
*
**********************************************************************/

#include <iostream>
#include <cstdlib>
#include <math.h>
#include <sndfile.h>
#include <mffm/complexFFT.H> // from MFFT wrapper

using namespace std;

#define FRAMESPERBUFFER		1024
#define MAXNROFCHANNELS		2


int main(int argc,char **argv)
{
char *infilename=NULL;
char *outfilename=NULL;
SNDFILE	*infile = NULL;
SNDFILE	*outfile = NULL;
SF_INFO	sfinfo;	// libsndfile info struct
int frames_read,frames_written;
double *samplebuffer;
unsigned long bufptr=0;

  if(argc < 3){
    cout << "Please specify file names for input and output\n";
    exit(0);
  }
  else{
    infilename=argv[1];
    outfilename=argv[2];
  }

  // Try opening audio file for reading
  if((infile = sf_open(infilename,SFM_READ,&sfinfo)) == NULL)
  {
    cout << "File read error\n";
    exit(1);
  }

  cout << "**************************" << endl;
  cout << "* Input file information *" << endl;
  cout << "**************************" << endl;
  cout << "Sample rate: " << sfinfo.samplerate << endl;
  cout << "Duration: " << (sfinfo.frames / sfinfo.samplerate) << " sec" << endl;
  cout << "Number of channels: " << sfinfo.channels << endl;
  cout << "Number of frames: " << sfinfo.frames << endl;

  if(sfinfo.channels > MAXNROFCHANNELS)
  {
    cout << "File contains too many channels" << endl;
    exit(1);
  }

  // let n be the largest power of 2 below # frames in the file
  unsigned long n=2; // provokes a warning.. I'll sort that out later...
  while(n < sfinfo.frames) n*=2;
  n /= 2;
  cout << "n: " << n << endl;

  // interleaved sample buffer
  samplebuffer = new double[n*sfinfo.channels];

  complexFFTData fftData(n);
  complexFFT fft(&fftData);

  bufptr=0;

  // loop to read file in chunks into the sample buffer
  while((frames_read =
    sf_readf_double(infile,samplebuffer+bufptr*sfinfo.channels,FRAMESPERBUFFER)) > 0)
  {
    if(frames_read < FRAMESPERBUFFER){
      cout << "Incomplete buffer" << endl;
      break;
    }
    bufptr+=frames_read;
    if(bufptr>=n) break; // reached n, stop reading
  } // while

  // copy samples to FFT input buffer (left channel only)
  // NB: buffer pointer runs <channels> times as fast as i
  for(unsigned long i=0; i<n; ++i){
    fftData.in[i][0]=samplebuffer[i*sfinfo.channels];
    fftData.in[i][1]=0; // real input signal, imag = 0
  } // for

  fft.fwdTransform();

  double mag,phase;

  for(unsigned long i=0; i<n; ++i){
    /* calculate magnitude and phase from complex output
     * Use 1/n to compensate for the factor of n that was added by
     * the forward FFT, which could be done as part of the inverse FFT as
     * well, that is a choice
     */
    mag=sqrt(fftData.out[i][0]*fftData.out[i][0]/(n*n) +
             fftData.out[i][1]*fftData.out[i][1]/(n*n));
    phase=atan2(fftData.out[i][1],fftData.out[i][0]);

    /*
     * This is the fun part
     */
    //phase=0; // for experimenting, set the phase to 0

    /* reconstruct real and imaginary part from new magnitude & phase
     * N.B.: re-use the output buffer as source for the inverse FFT
    */
    fftData.out[i][0]=mag*cos(phase);
    fftData.out[i][1]=mag*sin(phase);
  } // for

  fft.invTransform();

  // copy only the real part to sample buffer
  for(unsigned long i=0; i<n; ++i){
    samplebuffer[i]=fftData.in[i][0];
  } // for

  // Try opening audio file for writing
  sfinfo.format= (SF_FORMAT_WAV | SF_FORMAT_PCM_16);
  sfinfo.frames=n;
  sfinfo.channels=1; // output file is mono
  
  if(sf_format_check(&sfinfo)) cout << "Output parameters OK" << endl;

  if((outfile = sf_open(outfilename,SFM_WRITE,&sfinfo)) == NULL)
  {
    cout << "Unable to open output file\n";
    exit(1);
  } // if

  bufptr=0;
  // write sample buffer in chunks to the output file
  while((frames_written =
    sf_writef_double(outfile,samplebuffer+bufptr*sfinfo.channels,FRAMESPERBUFFER)) > 0)
  {
    bufptr+=frames_written;
    if(bufptr>=n) break; // reached n, stop writing
  } // while

  delete [] samplebuffer;
  sf_close(infile);
  sf_close(outfile);

} // main()

