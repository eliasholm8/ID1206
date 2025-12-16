def sstf(start, requests):
    print("SSTF:")
    total = 0
    position = start

    while requests:
        shortest = min(requests, key = lambda x:abs(x - position))
        total += abs(shortest - position)
        print(shortest)
        position = shortest
        requests.remove(shortest)
    return total

def main():
    requests = [1912, 260, 420, 280, 2023, 2788, 2901, 1400, 1600, 1501]
    start = 1800
    print("TOTAL:" + str(sstf(start, requests)))

if __name__ == "__main__":
    main()