#!/usr/bin/env python

import getopt, sys
import struct
from numpy import array
from pylab import * #plot, xlabel, ylabel, title, legend, show
import numpy

def smooth(x,window_len=11,window='hanning'):
    """smooth the data using a window with requested size.

    This method is based on the convolution of a scaled window with the signal.
    The signal is prepared by introducing reflected copies of the signal
    (with the window size) in both ends so that transient parts are minimized
    in the begining and end part of the output signal.

    input:
        x: the input signal
        window_len: the dimension of the smoothing window; should be an odd integer
        window: the type of window from 'flat', 'hanning', 'hamming', 'bartlett', 'blackman'
            flat window will produce a moving average smoothing.

    output:
        the smoothed signal

    example:

    t=linspace(-2,2,0.1)
    x=sin(t)+randn(len(t))*0.1
    y=smooth(x)

    see also:

    numpy.hanning, numpy.hamming, numpy.bartlett, numpy.blackman, numpy.convolve
    scipy.signal.lfilter

    TODO: the window parameter could be the window itself if an array instead of a string
    """

    if x.ndim != 1:
        raise ValueError, "smooth only accepts 1 dimension arrays."

    if x.size < window_len:
        raise ValueError, "Input vector needs to be bigger than window size."


    if window_len<3:
        return x


    if not window in ['flat', 'hanning', 'hamming', 'bartlett', 'blackman']:
        raise ValueError, "Window is on of 'flat', 'hanning', 'hamming', 'bartlett', 'blackman'"


    s=numpy.r_[2*x[0]-x[window_len-1::-1],x,2*x[-1]-x[-1:-window_len:-1]]
    #print(len(s))
    if window == 'flat': #moving average
        w=numpy.ones(window_len,'d')
    else:
        w=eval('numpy.'+window+'(window_len)')

    y=numpy.convolve(w/w.sum(),s,mode='same')
    return y[window_len:-window_len+1]

class TrackPoint():
    def __init__(self, filename, line, function, entries):
        self.filename = filename
        self.line = line
        self.function = function
        self.entries = entries
        self.datafile = None

def eat_point(file):
    desc = '256sI256sI'
    dlen = 520
    data = file.read(dlen)
    if len(data) != dlen:
        return None

    ( filename, line, function, n ) = struct.unpack(desc, data)
    filename = filename.rstrip("\x00")
    function = function.rstrip("\x00")

    #print filename + ':' + str(line) + '; n = ' + str(n) + ';'

    desc = ''
    dlen = n * (8 + 8 + 8)
    data = file.read(dlen)
    if len(data) != dlen:
        return None

    while n > 0:
        desc = desc + 'ddQ'
        n = n - 1

    arr = struct.unpack(desc, data)

    point = TrackPoint(filename, line, function, array(arr).reshape(-1, 3))

    return point

def read_file(filename):
    #print '' + filename + ':'
    file = open(filename, 'rb')

    points = []

    while True:
        pt = eat_point(file)
        if pt == None:
            break
        pt.datafile = filename
        points.append(pt)

    return points

def read_files(filenames):
    pts = []
    for fn in filenames:
        pts.append(read_file(fn))
    return pts

def read_files_function_names(filenames):
    fnameslist = []
    for pts in read_files(filenames):
        for p in pts:
            fnameslist.append(p.function)
    return sorted(set(fnameslist))

def read_files_functions(fname, filenames):
    retpts = []
    for pts in read_files(filenames):
        for p in pts:
            if p.function == fname:
                retpts.append(p)
    return retpts

def plot_once(points):
    l = []
    for p in points:
        plot(p.entries[:, 0], p.entries[:, 1] * p.entries[:, 2])
        l.append(p.function)
    if len(l) > 0:
        xlabel('instant')
        ylabel('spent')
        if len(l) > 8:
            legend(l)
        show()

def plot_every(points):
    for p in points:
        xlabel('instant')
        ylabel('spent')
        title(p.function)
        plot(p.entries[:, 0], p.entries[:, 1])
        plot(p.entries[:, 0], p.entries[:, 1], 'o')
        show()

def plot_rising(points):
    for p in points:
        arr = p.entries

        if len(arr) < 50:
            continue

        middle = (arr[-1, 0] - arr[0, 0]) / 2 + arr[0, 0]
        left = nonzero(arr[:, 0] <= middle)
        right = nonzero(arr[:, 0] > middle)

        arr_l = (arr[left, 1])[0]
        arr_r = (arr[right, 1])[0]

        left_p = sum(arr_l) / len(arr_l)
        right_p = sum(arr_r) / len(arr_r)

        if left_p >= right_p:
            continue

        #print str(left_p) + ' vs ' + str(right_p)
        print p.function
        title(p.function)
        xlabel('instant')
        ylabel('spent')
        plot(arr[:, 0], arr[:, 1])
        plot(arr[:, 0], arr[:, 1], 'o')
        show()

def cmd_ls(args):
    if len(args) <= 0:
        errx('ls: missing filename')

    filenames = args

    i = 0
    for f in read_files_function_names(filenames):
        print str(i) + ': ' + f
        i = i + 1

def cmd_plotfunc(args):
    if len(args) < 2:
        errx('plotfunc: missing arguments')

    funcno = int(args[0])
    if str(funcno) != args[0]:
        errx('plotfunc: first argument must be a valid number')

    filenames = args[1:]

    function_names = read_files_function_names(filenames)

    if funcno >= len(function_names) or funcno < 0:
        errx('plotfunc: invalid function number ' + str(funcno))

    l = []
    pts = read_files_functions(function_names[funcno], filenames)
    for p in pts:
        plot(p.entries[:, 0], p.entries[:, 1] * p.entries[:, 2])
        l.append(p.datafile + ' (' + p.filename + ':' + str(p.line) + ')')
    if len(pts) > 0:
        title(p.function)
        xlabel('instant')
        ylabel('spent')
        legend(l)
        show()

def warn(msg):
    print >> sys.stderr, sys.argv[0] + ': ' + msg

def errx(msg):
    warn(msg)
    sys.exit(1)

def usage():
    print 'usage: ' + sys.argv[0] + ' [-h] plot|list|plotfunc'

def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], 'hv', ['help'])
    except getopt.GetoptError, err:
        # print help information and exit:
        print str(err) # will print something like 'option -a not recognized'
        usage()
        sys.exit(2)

    verbose = False

    for o, a in opts:
        if o == '-v':
            verbose = True
        elif o in ('-h', '--help'):
            usage()
            sys.exit(1)
        else:
            assert False, 'unhandled option'

    if len(args) < 1:
        warn('missing command')
        usage()
        sys.exit(1)

    cmd = args[0]
    args = args[1:]

    if cmd == 'plot':
        plot(args)
    elif cmd == 'list' or cmd == 'ls':
        cmd_ls(args)
    elif cmd == 'plotfunc' or cmd == 'pf':
        cmd_plotfunc(args)
    else:
        errx('invalid command `' + cmd + '\'')

if __name__ == '__main__':
    main()

#import pdb
#pdb.set_trace()
#
#plot_once(pts)
#plot_every(pts)
#plot_rising(pts)


#class TrackAnalyzer():
#        def __init__(self, track):
#                self.track = track
#
#        def plot_all(self):
#                n = -1
#                for p in self.track.points:
#                        if n < 0 or n > 512:
#                                if n >= 0:
#                                        legend(l)
#                                        show()
#                                        #pass
#                                n = 0
#                                l = []
#                        plot(p.entries[:, 0], p.entries[:, 1])
#                        l.append(p.function)
#                        n = n + 1


#tr = Track()
#tr.read_file('perftracker.out')
#
#ta = TrackAnalyzer(tr)
#ta.plot_all()


#try:
#        while True:
#                print 'fuck'
#                ( filename, line, function, n_entries ) = unpack('256sI256sI', f.read(520))
#                filename = filename.rstrip("\x00")
#                function = function.rstrip("\x00")
#
#                print 'fuck1'
#                arr = unpack(str(n_entries) + 'd', f.read(n_entries * 8))
#                print 'fuck2'
#                entries = [ [ arr[::2] ], [ arr[1::2] ] ]
#
#                point = TrackPoint(filename, line, function, entries)
#
#                points = [ point ]
#finally:
#        f.close()
#
#print point

#from pylab import *
#
#plot(instant, spent)
#
#xlabel('instant')
#ylabel('spent')
#title(filename + ':' + str(line) + ': ' + function)
#grid(True)
#show()

# vim:set et:
# vi:set sw=4:
