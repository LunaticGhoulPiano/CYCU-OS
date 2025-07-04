import time
import datetime
import pytz
from multiprocessing import Process, Pool
from threading import Thread, Lock
import asyncio

class HW1:
    def __init__(self, filename, k, method):
        self.k = k
        self.time = 0
        with open(f'{filename}.txt', 'r') as f:
            self.data = [int(line.strip()) for line in f]
        self.n = len(self.data)
        self.filename = f'{filename}_output{method}.txt'
    
    def bubbleSort(self, arr):
        l = len(arr)
        swapped = True
        while swapped:
            swapped = False
            last_swap_index = 0
            for i in range(1, l):
                if arr[i-1] > arr[i]:
                    arr[i-1], arr[i] = arr[i], arr[i-1]
                    swapped = True
                    last_swap_index = i
            l = last_swap_index
        return arr
    
    def exec(self):
        self.time = time.time() # start

        self.data = self.bubbleSort(self.data)

        self.time = (time.time() - self.time) * 1000 # end
        with open(self.filename, 'w') as f:
            f.write('Sort : \n')
            f.writelines(f'{line}\n' for line in self.data)
            f.write(f'CPU Time : {self.time}\n')
            f.write(datetime.datetime.now(pytz.utc).astimezone(pytz.timezone('Asia/Taipei')).strftime('Output Time : %Y-%m-%d %H:%M:%S.%f%z\n').replace('+0800', '+08:00'))

class HW2(HW1):
    def merge(self, left, right):
        result = []
        i = j = 0
        len_left, len_right = len(left), len(right)
        while i < len_left and j < len_right:
            if left[i] < right[j]:
                result.append(left[i])
                i += 1
            else:
                result.append(right[j])
                j += 1
        result.extend(left[i:])
        result.extend(right[j:])
        return result
    
    def mergeSort(self, arr):
        if len(arr) <= 1:
            return arr
        mid = len(arr) // 2
        left_arr = self.mergeSort(arr[:mid])
        right_arr = self.mergeSort(arr[mid:])
        return self.merge(left_arr, right_arr)
    
    def exec_sub(self):
        self.sublists = [self.bubbleSort(sublist) for sublist in self.sublists] # bubble sort
        return self.mergeSort([line for sublist in self.sublists for line in sublist]) # merge sort
    
    def splitData(self):
        a = self.n // self.k
        b = self.n % self.k
        self.sublists = [self.data[i * a + min(i, b):(i + 1) * a + min(i + 1, b)] for i in range(self.k)]
        
    def exec(self):
        self.splitData()
        self.time = time.time() # start

        with Pool(processes = 1) as pool:
            self.data = pool.apply_async(self.exec_sub).get()

        self.time = (time.time() - self.time) * 1000 # end
        with open(self.filename, 'w') as f:
            f.write('Sort : \n')
            f.writelines(f'{line}\n' for line in self.data)
            f.write(f'CPU Time : {self.time}\n')
            f.write(datetime.datetime.now(pytz.utc).astimezone(pytz.timezone('Asia/Taipei')).strftime('Output Time : %Y-%m-%d %H:%M:%S.%f%z\n').replace('+0800', '+08:00'))

class HW3(HW2):
    def bubbleSortProcess(self, sublist):
        return self.bubbleSort(sublist)
    
    def mergeSortProcess(self, left, right):
        return self.merge(left, right)
    
    def exec(self):
        self.splitData()
        self.time = time.time() # start

        # bubble sort
        with Pool(processes = self.k) as pool:
            sorted_sublists = pool.map(self.bubbleSortProcess, self.sublists)
        
        # merge sort
        while len(sorted_sublists) > 1:
            with Pool(processes = len(sorted_sublists) // 2) as pool:
                merged_lists = pool.starmap(self.mergeSortProcess, zip(sorted_sublists[::2], sorted_sublists[1::2]))
                if len(sorted_sublists) %2 == 1:
                    merged_lists.append(sorted_sublists[-1])
                sorted_sublists = merged_lists
        self.data = sorted_sublists[0]

        self.time = (time.time() - self.time) * 1000 # end
        with open(self.filename, 'w') as f:
            f.write('Sort : \n')
            f.writelines(f'{line}\n' for line in self.data)
            f.write(f'CPU Time : {self.time}\n')
            f.write(datetime.datetime.now(pytz.utc).astimezone(pytz.timezone('Asia/Taipei')).strftime('Output Time : %Y-%m-%d %H:%M:%S.%f%z\n').replace('+0800', '+08:00'))

class HW4(HW3):
    def bubbleSortThread(self, sublist, result, lock):
        sorted_sublist = self.bubbleSort(sublist)
        with lock:
            result.append(sorted_sublist)
    
    async def mergeSortThread(self, left, right):
        return self.merge(left, right)

    def exec(self):
        self.splitData()
        self.time = time.time() # start

        # bubble sort
        sorted_sublists = []
        threads = []
        lock = Lock()
        for sublist in self.sublists:
            result = []
            thread = Thread(target = self.bubbleSortThread, args = (sublist, result, lock))
            threads.append(thread)
            thread.start()
            thread.join()
            sorted_sublists.extend(result)
        
        # merge sort
        loop = asyncio.get_event_loop()
        for i in range(self.k-1):
            left = sorted_sublists[i]
            right = sorted_sublists[i+1]
            sorted_sublists[i+1] = loop.run_until_complete(self.mergeSortThread(left, right))
        
        self.data = sorted_sublists[-1]

        self.time = (time.time() - self.time) * 1000 # end
        with open(self.filename, 'w') as f:
            f.write('Sort : \n')
            f.writelines(f'{line}\n' for line in self.data)
            f.write(f'CPU Time : {self.time}\n')
            f.write(datetime.datetime.now(pytz.utc).astimezone(pytz.timezone('Asia/Taipei')).strftime('Output Time : %Y-%m-%d %H:%M:%S.%f%z\n').replace('+0800', '+08:00'))

def main(filename, k, method):
    """
    filename = input('請輸入檔案名稱:\n')
    k = int(input('請輸入要切成幾份:\n'))
    method = int(input('請輸入方法編號:(方法1, 方法2, 方法3, 方法4)\n'))
    """
    match method:
        case 1:
            obj = HW1(filename, k, method)
            obj.exec()
        case 2:
            obj = HW2(filename, k, method)
            obj.exec()
        case 3:
            obj = HW3(filename, k, method)
            obj.exec()
        case 4:
            obj = HW4(filename, k, method)
            obj.exec()
    
    return obj

if __name__ == '__main__':
    names = ['input_1w', 'input_10w', 'input_50w', 'input_100w']
    ks = [1, 5, 13, 17]
    methods = [1, 2, 3, 4]
    for name in names:
        for k in ks:
            for method in methods:
                obj = main(name, k, method)
                with open('exp.txt', 'a') as exp:
                    exp.write(f'file = {name}, n = {obj.n}, k = {k}, method = {method}, time = {obj.time}ms\n')