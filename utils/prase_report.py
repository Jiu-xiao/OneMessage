#!/usr/bin/python3

import serial
import csv
import signal
import time
import datetime
import json

prefix = '@msg'.encode()
suffix = '@end'.encode()

topic_map = {}
activity = []
activ_map = ['publish', 'subscribe', 'filter', 'link', 'export']


def prase_topic_map(_map):
    _start = 0
    _end = 0
    _index = 0
    while _index < len(_map):
        # 前两个字节为话题id
        id = _map[_start]+_map[_start+1] * 256

        # 得到话题名字符串起始坐标
        _index += 2
        _start = _index

        # 得到话题名字符串结束坐标
        while _index != len(_map) and _map[_index] != 0:
            _index += 1
        _end = _index

        # 将话题加入字典
        topic_map[id] = str(_map[_start:_end], encoding='utf-8')

        # 移动到下一个话题开始坐标
        _start = _end + 1
        _index = _start


def press_data(_data):
    activity.clear()

    # 单个数据包为固定8字节
    for i in range(int(len(_data)/8)):
        _tmp = []
        _raw = _data[i*8:i*8+8]
        _tmp.append(_raw[2] + _raw[3] * 256)  # id
        _tmp.append(_raw[0])  # 活动
        _tmp.append(_raw[7] * (2 ** 24) + _raw[6] *
                    (2 ** 16) + _raw[5] * (2 ** 8) + _raw[4])  # 时间
        activity.append(_tmp)
        print(_tmp)


def byte_match(_data, _source, len):
    for i in range(len):
        if _data[i] != _source[i]:
            return 0
    return 1


# 读取配置文件
config_file = open('config.json', 'r')
config_data = json.load(config_file)

port = config_data['config']['port']
speed = config_data['config']['speed']

# 打开串口
ser = serial.Serial(port, speed)

# 配置CSV文件
curr_time = datetime.datetime.now()
timestamp = datetime.datetime.strftime(curr_time, '%Y-%m-%d %H:%M:%S')
csvfile = open(timestamp+'.csv', "w", encoding='utf8', newline='')
fileheader = ['time', 'name', 'operation']
writer = csv.writer(csvfile)
writer.writerow(fileheader)

_continue = True


def save(signum, frame):
    _continue = False
    csvfile.close()
    print('Saved to '+timestamp+'.csv')


signal.signal(signal.SIGINT, save)

try:
    while _continue:
        time.sleep(config_data['config']['detection_cycle'])
        start = 0
        end = 0
        count = ser.inWaiting()
        if count > 0:
            s = ser.read(count)
            # 遍历前缀
            for c in range(len(s)):
                _tmp = s[c:]
                if byte_match(_tmp, prefix, len(prefix)):
                    start = c + len(prefix)
                    break

            # 遍历后缀
            for c in range(start, len(s) - start):
                _tmp = s[c:]
                if byte_match(_tmp, suffix, len(suffix)):
                    end = c
                    break
            if end <= start:
                continue
        else:
            continue

        # 解析topic_map
        prase_topic_map(s[start:end])

        # 解析数据
        press_data(s[end+len(suffix):])

        # 写入CSV
        for item in activity:
            if not topic_map.__contains__(item[0]):
                writer.writerow([item[2]*config_data['config']
                                ['time_resolution'], 'Unknow',  activ_map[item[1]]])
            else:
                writer.writerow([item[2]*config_data['config']
                                ['time_resolution'], topic_map[item[0]],  activ_map[item[1]]])
except Exception as e:
    print(str(e))
