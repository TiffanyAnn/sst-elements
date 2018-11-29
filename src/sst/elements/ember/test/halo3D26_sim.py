

def getWorkFlow( defaults ):
    workFlow = []
    motif = dict.copy( defaults )
    motif['cmd'] = "Init"
    workFlow.append( motif )

    motif = dict.copy( defaults )
    motif['cmd'] = "Halo3D26 nx=1024 ny=1024 nz=1024 peflops=75000000000000 fields_per_cell=16"
    workFlow.append( motif )

    motif = dict.copy( defaults )
    motif['cmd'] = "Fini"
    workFlow.append( motif )

	# numNodes = 0 implies use all nodes on network
    numNodes = 0
    numCores = 1

    return workFlow, numNodes, numCores

def getNetwork():

	#platform = 'chamaPSM'
	#platform = 'chamaOpenIB'
	#platform = 'bgq'
	platform = 'halo3D26'

	#topo = ''
	#shape = ''
	topo = 'dragonfly'
	shape = '16:32:16:32'

	return platform, topo, shape

def getDetailedModel():
    return "","",[]
