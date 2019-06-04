

def getWorkFlow( defaults ):
    workFlow = []
    motif = dict.copy( defaults )
    motif['cmd'] = "Init"
    workFlow.append( motif )

    motif = dict.copy( defaults )
    # exascale motif['cmd'] = "Halo3D26 nx=128 ny=128 nz=128 peflops=75000000000000 fields_per_cell=30"
    motif['cmd'] = "Halo3D pex=64 pey=64 pez=64 nx=64 ny=64 nz=64 peflops=75000000000000 fields_per_cell=30"
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
	shape = '16:32:16:16'
	# exascale shape = '16:32:16:32'

	return platform, topo, shape

def getDetailedModel():
    return "","",[]
