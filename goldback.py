from datetime import datetime
from itertools import combinations

def prime_sieve(n):
    size = n // 2
    sieve = [1] * size
    limit = int(n ** 0.5)
    for i in range(1, limit):
        if sieve[i]:
            val = 2 * i + 1
            tmp = ((size - 1) - i) // val
            sieve[i+val::val] = [0] * tmp
    return [i * 2 + 1 for i, v in enumerate(sieve) if v and i > 0]

if __name__ == "__main__":
    print("This code takes an even integer n greater than 2 and expresses all even numbers between 2 and n as a sum of two primes.")

    n = int(input("Enter n: "))

    while n & 1 or n < 2:
        print("You must enter an even number greater than 2.")
        try: n = int(input("Enter n: "))
        except: pass
    else:
        print("OK, {} is a good number. Here is your decomposition: ".format(n))
        start = datetime.now()
        primes = prime_sieve(n)
        counter = [0] * max(n + 1, 5)
        counter[4] = 1
        for i in primes:
            for j in primes:
                if i < j or i + j > n:
                    break
                counter[i + j] += 1
        for i in range(4, n + 1, 2):
            print("{}, {}".format(i, counter[i]))

    print("Time taken: ", datetime.now()-start)
