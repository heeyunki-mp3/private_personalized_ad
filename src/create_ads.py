import random
import string

if __name__ == '__main__':
    with open("ads.csv", "a") as f:
        for i in range(50000):
            ad = ''.join(random.choice(string.ascii_letters) for i in range(32))
            f.write(f"{ad}\n")
