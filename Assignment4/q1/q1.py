from bisect import *

def split_at_value(arr, x):
    i = bisect_right(arr, x)
    return arr[:i], arr[i:]

def sstf(start, requests):
    requests = requests.copy()
    print("SSTF:")
    total = 0
    current = start

    while requests:
        shortest = min(requests, key = lambda x:abs(x - current))
        total += abs(shortest - current)
        print(shortest)
        current = shortest
        requests.remove(shortest)
    return total

def SCAN(start, requests):
    print("SCAN:")
    prev = start
    total = 0
    requests = requests.copy()
    requests.append(2999)  # Shitty solution for end
    requests.sort()
    low, high = split_at_value(requests, start)

    for current in high:
        total += abs(current - prev)
        prev = current
        print(current)

    for current in reversed(low):
        total += abs(current - prev)
        prev = current
        print(current)

    return total

def C_SCAN(start, requests):
    print("C-SCAN:")
    prev = start
    total = 0
    requests = requests.copy()
    requests.append(0)  
    requests.append(2999)
    requests.sort()
    low, high = split_at_value(requests, start)

    for current in high:
        total += abs(current - prev)
        prev = current
        print(current)

    for current in low:
        total += abs(current - prev)
        prev = current
        print(current)

    return total

def fcfs(start, requests):
    print("FCFS")
    current = start
    requests = requests.copy()
    total = 0
    for next in requests:
        total += abs(current - next)
        current = next
        print(next)
    return total


    
def main():
    requests = [1912, 260, 420, 280, 2023, 2788, 2901, 1400, 1600, 1501]
    start = 1800
    prev = 1600
    print("TOTAL:" + str(sstf(start, requests)) + "\n")
    print("TOTAL:" + str(SCAN(start, requests)) + "\n")
    print("TOTAL:" + str(C_SCAN(start, requests)) + "\n")
    print("TOTAL:" + str(fcfs(start, requests)) + "\n")

if __name__ == "__main__":
    main()