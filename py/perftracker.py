#!/usr/bin/env python

from pdb import set_trace
import struct
from numpy import array
from pylab import plot, xlabel, ylabel, title, legend, show

class TrackPoint():
        def __init__(self, filename, line, function, entries):
                self.filename = filename
                self.line = line
                self.function = function
                self.entries = entries

class Track():
        def eat_point(self, file):
                ( filename, line, function, n ) = struct.unpack('256sI256sI', file.read(520))
                filename = filename.rstrip("\x00")
                function = function.rstrip("\x00")

                #print filename + ':' + str(line) + '; n = ' + str(n) + ';'
                n = n * 2

                arr = struct.unpack(str(n) + 'd', file.read(n * 8))

                point = TrackPoint(filename, line, function, array(arr).reshape(-1, 2))

                return point

        def read_file(self, filename):
                #print '' + filename + ':'
                file = open(filename, 'rb')

                pts = []

                try:
                        while True:
                                pt = self.eat_point(file)
                                pts.append(pt)
                                #print pt.function + ';'

                except struct.error:
                        pass

                self.points = pts

class TrackAnalyzer():
        def __init__(self, track):
                self.track = track

        def plot_all(self):
                n = -1
                for p in self.track.points:
                        if n < 0 or n > 512:
                                if n >= 0:
                                        legend(l)
                                        show()
                                        #pass
                                n = 0
                                l = []
                        plot(p.entries[:, 0], p.entries[:, 1])
                        l.append(p.function)
                        n = n + 1

def read_track_file(filename):
        t = Track()
        t.read_file(filename)
        return t

def plot_track(track):
        ta = TrackAnalyzer(track)
        ta.plot_all()
        show()

o = read_track_file('perftracker.out-2')
#n = read_track_file('perftracker~improved.out')

#tr = Track()
#tr.read_file('perftracker.out')
#
#ta = TrackAnalyzer(tr)
#ta.plot_all()

for p in o.points:
        print p.filename + ':' + str(p.line) + ': ' + p.function
        plot(p.entries[:, 0], p.entries[:, 1])
        plot(p.entries[:, 0], p.entries[:, 1], 'ro')
        show()

plot_track(o)

set_trace()

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
