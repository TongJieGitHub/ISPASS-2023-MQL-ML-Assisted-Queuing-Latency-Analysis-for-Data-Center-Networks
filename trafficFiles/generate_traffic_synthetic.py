k = 12
num = int(k*k*k/4)
outfile = "432x432alltoall.txt"

with open(outfile, "w") as outf:
    for x in range(0,num,1):
        for y in range(0,num,1):
            if x != y:
                outf.write("C%d:S%d\n" % (y, x))
        outf.write("\n")