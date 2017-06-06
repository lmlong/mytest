#!/usr/bin/python
# -*- coding: utf8  -*-

import sys
import subprocess
import re
import datetime
import time
import os
import random

def get_stat_files():
    result=[]
    shellcmd="ls -tr *_stat*.log"
    sp = subprocess.Popen(shellcmd, shell=True, stderr=subprocess.PIPE, stdout=subprocess.PIPE)    
    # statre = re.compile(".*stat\d*.log")
    for line in sp.stdout.readlines():
        filename = line.strip("\n")
        result.append(filename)
        
    return result


chartdir_libpath="/root/ChartDirector/lib"
sys.path.append(chartdir_libpath)
from pychartdir import *
def makechart(labels, labeloffset, dataset, title, chartfile):
    c = XYChart(800, 400, 0xeeeeff, 0x000000, 1)
    c.setPlotArea(70, 58, 680, 250, 0xffffff, -1, -1, 0xcccccc, 0xcccccc)
    c.addLegend(50, 30, 0, "arialbd.ttf", 9).setBackground(Transparent)
    c.addTitle(title, "timesbi.ttf", 15).setBackground(0xccccff, 0x000000, glassEffect())
    c.xAxis().setLabels(labels).setFontAngle(-60)
    c.xAxis().setLabelStep(5,0, labeloffset)
    c.xAxis().setTitle("datetime in minute")

    c.yAxis().setTitle("100*ms")
    layer = c.addLineLayer2()
    i = 0
    colors=[0x610B21,0x1C1C1C,0x08088A,0x00FF00, 0x0B610B,0x0A2A0A,0x0000FF,0xDF013A]
    for key in dataset:
        data = dataset[key]
        ds = layer.addDataSet(data, colors[i%(len(colors))], key)
        avg = sum(data,0.0)/len(data)
        if (avg < 100000):
            ds.setUseYAxis2(True)
        i+=1
    c.makeChart(chartfile)



'''
画图对比图
'''
def makechart2(base_linename, base_labels, base_dataset, comp_linename, comp_labels, comp_dataset, title, chartfile):
    c = XYChart(1000, 500, 0xeeeeff, 0x000000, 1)
    c.setPlotArea(70, 158, 880, 250, 0xffffff, -1, -1, 0xcccccc, 0xcccccc)
    c.addLegend(80, 160, 0, "arialbd.ttf", 9).setBackground(Transparent)
    c.addTitle(title, "timesbi.ttf", 15).setBackground(0xccccff, 0x000000, glassEffect())

    labeloffset = 0
    
    c.xAxis().setLabels(base_labels).setFontAngle(-60)
    c.xAxis().setLabelStep(60,0, labeloffset)
    #c.xAxis().setTitle("datetime in minute")

    c.xAxis2().setLabels(comp_labels).setFontAngle(-60)
    c.xAxis2().setLabelStep(60,0, labeloffset)
    #c.xAxis2().setTitle("datetime in minute")

    #c.yAxis().setTitle("100*ms")
    layer = c.addLineLayer2()

    base_color = 0x08088A
    comp_color = 0x8A0808

    layer.addDataSet(base_dataset, base_color, base_linename)
    layer.addDataSet(comp_dataset, comp_color, comp_linename)

    c.makeChart(chartfile)

def scan_item(stat_result, fromtime, totime, item, statgap):
    statfiles = get_stat_files()
    if len(statfiles) == 0:
        print "no stat file found"
        return 1
    #print statfiles
    
    stat_result.clear()
    for statpoint in range(fromtime, totime, statgap):
        stat_result[statpoint] = {"timestr":"", "min":0, "avg":0, "max":0, "count_ok":0, "count_nok":0}

    timere = re.compile("=+Statistic in (\d+)s, (\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})\.\d*=+")
    statre = re.compile("%s *\| *(-{0,1}\d+) *\| *(\d+) *\| *(\d+) *\| *(\d+\.\d+) *\| *(\d+\.\d+) *\| *(\d+\.\d+) *\| *\| *(\d+) *\| *(\d+) *\| *(\d+) *\|" %(stat_item))
    end_scan = False
    start_scan  = False
    for statfile in statfiles:
        if os.stat(statfile).st_mtime < fromtime:
            print "skip %s!"%(statfile)
            continue
        f = open(statfile)
        for line in f.readlines():
            line=line.strip("\n")
            m = timere.match(line)
            if m is not None:
                stattime = m.group(2)
                timesec = int(datetime.datetime.strptime(stattime, "%Y-%m-%d %H:%M:%S").strftime("%s"))
                if timesec > totime:
                    print "%s endscan"%(stattime)
                    end_scan = True
                    break
                if timesec < fromtime:
                    continue
                cur_time_sec = (((timesec + time.timezone)/statgap) * statgap) - time.timezone
                #print  stattime,  timesec, cur_time_sec, time.timezone, fromtime
                stat_result[cur_time_sec]["timestr"] = stattime
                start_scan = True 
            elif start_scan: 
                m = statre.match(line)
                if m is not None:
                    errcode = int(m.group(1))
                    count = int(m.group(2))
                    if (errcode == 0):
                        lat_avg = int(float(m.group(4))*100)
                        lat_max = int(float(m.group(5))*100)
                        lat_min = int(float(m.group(6))*100)
                        stat_result[cur_time_sec]["avg"] = lat_avg
                        stat_result[cur_time_sec]["max"] = lat_max
                        stat_result[cur_time_sec]["min"] = lat_min
                        stat_result[cur_time_sec]["count_ok"] = count
                    else:
                        stat_result[cur_time_sec]["count_nok"] += count
                    #print stat_result[cur_time_sec]
                    
        if end_scan:
            break

    return 0


'''
    获取延时图
'''
def chart_latency(from_time, end_time, stat_item):
    stat_result = {}
    statgap = 60
    scan_item(stat_result, from_time, end_time, stat_item, statgap)
    
    '''
    print "------------------------------"
    headline="%19s %10s %10s %10s %10s"%("timestr","time", "avg", "max", "min")
    print headline
    for key in sorted(stat_result.keys()):
        oneline="%19s %10d %10d %10d %10d"%(stat_result[key]["timestr"], key, stat_result[key]["avg"], stat_result[key]["max"], stat_result[key]["min"])
        print oneline
                 
    '''
    
    findoffset = False
    labels = []
    labeloffset = 0
    labelstep = 5
    dataset= {"max":[],"avg":[], "min":[]}
    title = "%s_latentcy_max"%(stat_item)
    chartfile = "%s_latentcy_max.jpg"%(stat_item)
    for key in sorted(stat_result.keys()):
        labels.append(stat_result[key]["timestr"])
        dataset["max"].append(stat_result[key]["max"])
        dataset["min"].append(stat_result[key]["min"])
        dataset["avg"].append(stat_result[key]["avg"])
        
        if not findoffset:
            if (((key-time.timezone)/60)%(labelstep)):
                labeloffset += 1
            else:
                findoffset = True
        
    #print labeloffset    
    makechart(labels, labeloffset, dataset, title, chartfile)

'''
    获取日对比图
'''
def chart_day_comp(basetime, comptime, stat_item):
    basetime_end = basetime + 24*3600
    comptime_end = comptime + 24*3600
  
    statgap = 60

    basestat = {}
    scan_item(basestat, basetime, basetime_end, stat_item, statgap)
    compstat = {}
    scan_item(compstat, comptime, comptime_end, stat_item, statgap)

    base_linename = basestr
    comp_linename = compstr
    
    for charttype in ["count_ok", "max", "count_nok"]:
        title = "%s_%s_%s_%s"%(stat_item, charttype, basestr, compstr)
        chartfile = "%s.jpg"%(title)

        base_labels = []
        base_dataset = [] 
        for key in sorted(basestat.keys()):
            base_labels.append(basestat[key]["timestr"])
            base_dataset.append(basestat[key][charttype])

        comp_labels = []
        comp_dataset = []
        for key in sorted(compstat.keys()):
            comp_labels.append(compstat[key]["timestr"])
            comp_dataset.append(compstat[key][charttype])

        makechart2(base_linename, base_labels, base_dataset, comp_linename, comp_labels, comp_dataset, title, chartfile)
    
    
if __name__ == "__main__":
    if len(sys.argv) < 4:
        print "Usage:"
        print " %s daycomp _statItem basedate comparedate"%(sys.argv[0])
        print " %s latency _statItem fromtime [totime]"%(sys.argv[0])
        
        sys.exit(1)
    
    
    chartype = sys.argv[1]
    if chartype == "daycomp":
        stat_item = sys.argv[2]
        basetime = 0
        comptime = 0
        basestr=sys.argv[3]
        compstr = sys.argv[4]
        basetime = int(datetime.datetime.strptime(basestr, "%Y-%m-%d").strftime("%s"))
        comptime = int(datetime.datetime.strptime(compstr, "%Y-%m-%d").strftime("%s"))
        chart_day_comp(basetime, comptime, stat_item)

    elif chartype == "latency":
        stat_item = sys.argv[2]
        fromtime = 0
        totime = 0
        fromstr=sys.argv[3]
        fromtime = int(datetime.datetime.strptime(fromstr, "%Y-%m-%d %H:%M:%S").strftime("%s"))

        if len(sys.argv) < 5:
            totime = int(datetime.datetime.now().strftime("%s"))
        else:
            tostr = sys.argv[4]
            if tostr == "now":
                totime = int(datetime.datetime.now().strftime("%s"))
            else:
                totime = int(datetime.datetime.strptime(tostr, "%Y-%m-%d %H:%M:%S").strftime("%s"))
        chart_latency(fromtime, totime, stat_item)

    else:
        print("invalid charttype!! %s\n"%(chartype))
        sys.exit(1)
