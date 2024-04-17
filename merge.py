#! /usr/bin/python3

import sys

def valid_range(a):
    return a[0] <= a[1]

'''
Read ip ranges for the source.
Each line is expected to be one range like:
    0xff000000 - 0xff001000
where the hexadecimal numbers are interpreted
as an ip address(32-bit)
'''
def read_ranges(source):
    numbers=[]
    for line in source:
        line = line.replace("-", " ")
        rng = tuple(map(lambda x: int(x, base=16), line.split()))
        if rng and valid_range(rng):
            numbers.append(rng)
    return numbers

'''
The ranges are inclusive (i.e. closed ranges).
    eg. 101 - 201
The above range includes both 101 and 201.
'''
def range_length(t):
    return t[1] - t[0] + 1

'''
Check if two ranges are adjacent or overlapping.
eg.
    (1, 2), (2, 3)   =>   True. Overlapping ranges
    (1, 2), (3, 4)   =>   True. Adjacent ranges. Will become (1, 4)
    (1, 2), (4, 5)   =>   False. Not adjacent or overlapping.
'''
def overlapping(a, b):
    c = (min(a[0], b[0]), max(a[1], b[1]))
    if range_length(c) <= range_length(a) + range_length(b):
        return (True, c)
    return (False, b)

def merge_ranges(old_ranges):
    new_ranges = [old_ranges[0]]

    for rng in old_ranges[1:]:
        merged, new_rng =  overlapping(new_ranges[-1], rng)
        if merged:
            new_ranges[-1] = new_rng
        else:
            new_ranges.append(new_rng)
    return new_ranges

def ipv4_string(addr):
    return f"{(addr>>24)&0xff}.{(addr>>16)&0xff}.{(addr>> 8)&0xff}.{(addr>> 0)&0xff}"

def print_ipv4(addr):
    print((addr>>24)&0xff,
          (addr>>16)&0xff,
          (addr>> 8)&0xff,
          (addr>> 0)&0xff,
          sep='.', end='')

def process_and_merge_ranges(ranges):
    ranges.sort()
    new_ranges = merge_ranges(ranges)
    print(new_ranges)
    for addr in new_ranges:
        print(f"{ipv4_string(addr[0])} - {ipv4_string(addr[1])}")
    print("Total: ", len(new_ranges))

if __name__ == "__main__":
    if len(sys.argv) == 1:
        ranges = read_ranges(sys.stdin)
    else:
        with open(sys.argv[1]) as f:
            ranges = read_ranges(f)

    if not ranges:
        sys.exit(0)

    process_and_merge_ranges(ranges)
