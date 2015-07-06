import subprocess
import sys
import os

def persist(name, nodes, iters, results):
	filename = name + '_' + nodes + '_' + str(iters)
	try:
		os.remove(filename)
	except OSError:
		pass
	with open(filename, 'w+') as file:
		for i in results:		
			file.write(i+'\n')

def process(path, nodes, iters):
	cmd = ''.join(['env CNC_LOAD_BALANCER=STEALING env DIST_CNC=MPI /usr/bin/mpiexec.hydra -n ', nodes, ' ', path])
	p = subprocess.Popen([cmd], stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
	out, err = p.communicate()
        out = out.split()	
	time = out[out.index("RUNTIME") + 1]
	return time

#Usage: bench nodes iters benchname /path/to/program
def main(argv=sys.argv):
	nodes = argv[1]
	iters = int(argv[2])
	name = argv[3]
	path = argv[4]

	results = []
	for i in range(0, iters):
		r = process(path, nodes, iters)
		results = results + [r]
		if ((i/float(iters)) * 100) % 5 == 0:
			print ''.join([str(int((i/float(iters))*100)), '%'])
	persist(name, nodes, iters, results)

if __name__ == '__main__':
    main()


