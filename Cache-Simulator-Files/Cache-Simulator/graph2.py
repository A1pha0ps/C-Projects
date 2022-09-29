#!/usr/bin/python3

import sys
import os
import time
import matplotlib.pyplot as plt

# associativity range
assoc_range = [2]
# block size range
bsize_range = [b for b in range(6, 7)]
# capacity range
cap_range = [c for c in range(9, 23)]
# number of cores (1, 2, 4)
cores = [1]
# coherence protocol: (none, vi, or msi)
bus = ['wb', 'wt']
protocol = 'none'

expname = 'exp2'
figname = 'graph2.png'


def get_stats(logfile, key):
    for line in open(logfile):
        if line[2:].startswith(key):
            line = line.split()
            return float(line[1])
    return 0


def run_exp(logfile, core, cap, bsize, assoc):
    trace = 'trace.%dt.long.txt' % core
    cmd = "./p5 -t %s -p %s -n %d -cache %d %d %d >> %s" % (
        trace, protocol, core, cap, bsize, assoc, logfile)
    print(cmd)
    os.system(cmd)


def graph():
    timestr = time.strftime("%m.%d-%H_%M_%S")
    folder = "results/" + expname + "/" + timestr + "/"
    os.system("mkdir -p " + folder)

    miss_rate = {a: [] for a in assoc_range}
    bus_rate = {br: [] for br in bus}

    for a in assoc_range:
        for b in bsize_range:
            for c in cap_range:
                for d in cores:
                    logfile = folder + "%s-%02d-%02d-%02d-%02d.out" % (
                        protocol, d, c, b, a)
                    run_exp(logfile, d, c, b, a)
                    # miss_rate[a].append(get_stats(logfile, 'miss_rate') / 100)
                    wb = get_stats(logfile, 'B_written_cache_to_bus_wb')
                    wt = get_stats(logfile, 'B_written_cache_to_bus_wt')
                    bus_rate['wb'].append(wb)
                    bus_rate['wt'].append(wt)

    plots = []
    # for a in miss_rate:
    for br in bus_rate:
        p, = plt.plot([2 ** i for i in cap_range], bus_rate[br])
        plots.append(p)
    plt.legend(plots, ['%s' % br for br in bus])
    plt.xscale('log', base=2)
    plt.title('Graph #2: Traffic vs Cache Size')
    plt.xlabel('Capacity (B)')
    plt.ylabel('Traffic (B)')
    plt.savefig(figname)
    plt.show()


graph()
