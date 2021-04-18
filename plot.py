import matplotlib.pyplot as plt

x = []
y = []

file = open("plot.txt", "r")
lines = file.read().split("\n")

for line in lines:
    tmp = line.split()
    if len(tmp) >= 2:
        x.append(float(tmp[0]))
        y.append(int(tmp[1]))

plt.figure()

plt.plot(x, y)
plt.ylabel("Time, ms")
plt.xlabel("LoadFactor")

plt.savefig('src/plot.jpg')

plt.show()
