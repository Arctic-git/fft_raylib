
default settings: 2048 16384

fft                1    65.472          
wave               2    23.280
wavewindowed       1    11.640
xy                 1    12.282
fftscroll          1    4
wavescroll         1    4


# fft old
small: 
energy 280 cpu 28 gpu 14
name                         min    max    avg     n   
calc                         0.16    0.32    0.23   310
fft                          2.17    4.20    3.26   310
fftscroll                    0.07    0.28    0.11   310
imgui                        0.04    0.26    0.07   310
swap                        11.25   16.54   13.03   310
wave                         0.07    0.15    0.11   310
wavescroll                   0.04    0.16    0.07   310
xy                           0.10    0.21    0.15   310

fullscreen:
energy 290 cpu 29 gpu 50

65k:
energy 2000 cpu 51 gpu 15


# fft lerp
small
energy 50 cpu 30 gpu 16
name                         min    max    avg     n   
calc                         0.14    1.19    0.60   320
fft                          0.20    1.39    0.87   320
fftscroll                    0.06    1.07    0.55   320
imgui                        0.04    0.78    0.28   320
swap                         4.11   17.58   13.60   320
wave                         0.07    0.63    0.28   320
wavescroll                   0.04    0.65    0.29   320
xy                           0.09    0.89    0.39   320

fullscreen:
energy 100 cpu 25 gpu 50

65k:
energy 130 cpu 26 gpu 13
