import matplotlib.pyplot as plt

tavgs = [0.0 for i in range(10)]
pavgs = [0.0 for i in range(10)]
for i in range(1, 4):
	tname = "multifiles_thread" + str(i) + ".txt"
	pname = "multifiles_proc" + str(i) + ".txt"
	tfile = open(tname, "r")
	pfile = open(pname, "r")
	j = 0
	while j < 10:    
		line = tfile.readline()
		words = line.split()
		tavgs[j] += float(words[1]) / 3.0
		j += 1
	j = 0
	while j < 10:    
		line = pfile.readline()
		words = line.split()
		pavgs[j] += float(words[1]) / 3.0
		j += 1
#print str(tavgs)
#print str(pavgs)	
		
nfiles = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512]
plt.plot(nfiles, tavgs, 'r-')
plt.plot(nfiles, pavgs, 'b-')
plt.show()
