#!/usr/bin/env python

from pdb import set_trace
import struct
from numpy import array
from pylab import * #plot, xlabel, ylabel, title, legend, show

class TrackPoint():
        def __init__(self, filename, line, function, entries):
                self.filename = filename
                self.line = line
                self.function = function
                self.entries = entries

def eat_point(file):
        ( filename, line, function, n ) = struct.unpack('256sI256sI', file.read(520))
        filename = filename.rstrip("\x00")
        function = function.rstrip("\x00")

        #print filename + ':' + str(line) + '; n = ' + str(n) + ';'
        n = n * 2

        arr = struct.unpack(str(n) + 'd', file.read(n * 8))

        point = TrackPoint(filename, line, function, array(arr).reshape(-1, 2))

        return point

def read_file(filename):
        #print '' + filename + ':'
        file = open(filename, 'rb')

        points = []

        try:
                while True:
                        pt = eat_point(file)
                        points.append(pt)
                        #print pt.function + ';'

        except struct.error:
                pass

        return points

def plot_once(points):
        l = []
        for p in points:
                plot(p.entries[:, 0], p.entries[:, 1])
                l.append(p.function)
        if len(l) > 0:
                xlabel('instant')
                ylabel('spent')
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


pts = read_file('perftracker.out-2')

set_trace()

plot_once(pts)
plot_every(pts)
plot_rising(pts)


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
