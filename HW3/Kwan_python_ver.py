from collections import defaultdict 
 
def ReadFile(filename): 
    with open(filename, 'r', encoding='utf-8') as file: 
        lines = file.readlines() 
        method, frames = map(int, lines[0].split()) 
        pages = list(map(int, lines[1].strip())) 
    return method, frames, pages 
 
def FormattedFrames(frames): 
    return ''.join(map(str, reversed(frames))) 
 
def FIFO(pages, page_frames): 
    faults = 0 
    replaces = 0 
    frames = [] 
    output = [] 
     
    for page in pages: 
        if page not in frames: 
            if len(frames) >= page_frames: 
                frames.pop(0) 
                replaces += 1 
 
            faults += 1 
            frames.append(page) 
            output.append(f"{page}\t{FormattedFrames(frames)}\tF") 
             
        else: 
            output.append(f"{page}\t{FormattedFrames(frames)}") 
     
    output.append(f"Page Fault = {faults}  Page Replaces = {replaces}  Page Frames = {page_frames}") 
    return output 
 
def LRU(pages, page_frames): 
    faults = 0 
    replaces = 0 
    frames = [] 
    output = [] 
     
    for page in pages: 
        if page not in frames: 
            if len(frames) == page_frames: 
                frames.pop(0) 
                replaces += 1 
 
            faults += 1 
            frames.append(page) 
            output.append(f"{page}\t{FormattedFrames(frames)}\tF") 
 
        else: 
            frames.remove(page) 
            frames.append(page) 
            output.append(f"{page}\t{FormattedFrames(frames)}") 
     
    output.append(f"Page Fault = {faults}  Page Replaces = {replaces}  Page Frames = {page_frames}") 
    return output 
 
def LFU(pages, page_frames): 
    faults = 0 
    replaces = 0 
    counts = defaultdict(int) 
    frames = [] 
    output = [] 
 
    for page in pages: 
        counts[page] += 1 
 
        if page not in frames: 
            if len(frames) == page_frames: 
                least_freq = min(counts[frame] for frame in frames) 
                victim = next(frame for frame in frames if counts[frame] == least_freq) 
                frames.remove(victim) 
                counts[victim] = 0 
                replaces += 1 
 
            faults += 1 
            frames.append(page) 
            output.append(f"{page}\t{FormattedFrames(frames)}\tF") 
 
        else: 
            output.append(f"{page}\t{FormattedFrames(frames)}") 
 
    output.append(f"Page Fault = {faults}  Page Replaces = {replaces}  Page Frames = {page_frames}") 
    return output 
 
def MFU(pages, page_frames): 
    faults = 0 
    replaces = 0 
    counts = defaultdict(int) 
    frames = [] 
    output = [] 
     
    for page in pages: 
        counts[page] += 1 
 
        if page not in frames: 
            if len(frames) == page_frames: 
                most_freq = max(counts[frame] for frame in frames) 
                victim = next(frame for frame in frames if counts[frame] == most_freq) 
                frames.remove(victim) 
                counts[victim] = 0 
                replaces += 1 
 
            faults += 1 
            frames.append(page) 
            output.append(f"{page}\t{FormattedFrames(frames)}\tF") 
 
        else: 
            output.append(f"{page}\t{FormattedFrames(frames)}") 
 
    output.append(f"Page Fault = {faults}  Page Replaces = {replaces}  Page Frames = {page_frames}") 
    return output 
 
def LFU_LRU(pages, page_frames): 
    faults = 0 
    replaces = 0 
    counts = defaultdict(int) 
    frames = [] 
    output = [] 
 
    for page in pages: 
        counts[page] += 1 
 
        if page not in frames: 
            if len(frames) == page_frames: 
                least_freq = min(counts[frame] for frame in frames) 
                victim = next(frame for frame in frames if counts[frame] == least_freq) 
                frames.remove(victim) 
                counts[victim] = 0 
                replaces += 1 
 
            faults += 1 
            frames.append(page) 
            output.append(f"{page}\t{FormattedFrames(frames)}\tF") 
 
        else: 
            frames.remove(page) 
            frames.append(page) 
            output.append(f"{page}\t{FormattedFrames(frames)}") 
     
    output.append(f"Page Fault = {faults}  Page Replaces = {replaces}  Page Frames = {page_frames}") 
    return output 
 
while True: 
    filename = input("Please enter File Name : ") 
    method, frames, pages = ReadFile(f"{filename}.txt") 
 
    if method == 1: 
        output = ["--------------FIFO-----------------------"] + FIFO(pages, frames) 
    elif method == 2: 
        output = ["--------------LRU-----------------------"] + LRU(pages, frames) 
    elif method == 3: 
        output = ["--------------Least Frequently Used Page Replacement-----------------------"] + LFU(pages, frames) 
    elif method == 4: 
        output = ["--------------Most Frequently Used Page Replacement -----------------------"] + MFU(pages, frames) 
    elif method == 5: 
        output = ["--------------Least Frequently Used LRU Page Replacement-----------------------"] + LFU_LRU(pages, frames) 
    elif method == 6: 
        output = ["--------------FIFO-----------------------"] + FIFO(pages, frames) + \
                ["\n--------------LRU-----------------------"] + LRU(pages, frames) + \
                ["\n--------------Least Frequently Used Page Replacement-----------------------"] + LFU(pages, frames) + \
                ["\n--------------Most Frequently Used Page Replacement -----------------------"] + MFU(pages, frames) + \
                ["\n--------------Least Frequently Used LRU Page Replacement-----------------------"] + LFU_LRU(pages, frames) 
 
    with open(f"out_{filename}.txt", 'w', encoding='utf-8') as file: 
        for line in output: 
            file.write(line + '\n')