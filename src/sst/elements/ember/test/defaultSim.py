

def getWorkFlow( defaults ):
    workFlow = []
    motif = dict.copy( defaults )
    motif['cmd'] = "Init"
    workFlow.append( motif )

    motif = dict.copy( defaults )
    #motif['cmd'] = "Sweep3D pex=128 pey=128 nx=256 ny=256 nz=256 nodeflops=75000000000000 fields_per_cell=6 kba=8"
    motif['cmd'] = "LQCD nx=144 ny=144 nz=144 nt=192 peflops=75000000000000"
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
	platform = 'default'

	#topo = ''
	#shape = ''
	topo = 'dragonfly'
	shape = '16:32:16:32'

	return platform, topo, shape

def getDetailedModel():
    return "","",[]
