

def getWorkFlow( defaults ):
    workFlow = []
    motif = dict.copy( defaults )
    motif['cmd'] = "Init"
    workFlow.append( motif )

    motif = dict.copy( defaults )
    # exascale motif['cmd'] = "Halo3D26 nx=128 ny=128 nz=128 peflops=75000000000000 fields_per_cell=30"
    motif['cmd'] = "Halo3D26 iterations=1 nx=16 ny=16 nz=16 peflops=75000000000000 fields_per_cell=10"
    workFlow.append( motif )

    motif = dict.copy( defaults )
    motif['cmd'] = "Fini"
    workFlow.append( motif )

	# numNodes = 0 implies use all nodes on network
    numNodes = 0
    numCores = 4

    return workFlow, numNodes, numCores

def getNetwork():

	#platform = 'chamaPSM'
	#platform = 'chamaOpenIB'
	#platform = 'bgq'
	platform = 'halo3D26'

	#topo = ''
	#shape = ''
	topo = 'dragonfly'
	shape = '16:32:16:2'
	# exascale shape = '16:32:16:32'

	return platform, topo, shape

def getDetailedModel():
    return "","",[]
