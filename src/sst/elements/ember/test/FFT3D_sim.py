

def getWorkFlow( defaults ):
    workFlow = []
    motif = dict.copy( defaults )
    motif['cmd'] = "Init"
    workFlow.append( motif )

    motif = dict.copy( defaults )
    motif['cmd'] = "FFT3D nx=2048 ny=2048 nz=2048 npRow=8 nsPerElement=0.0016"

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
	platform = 'FFT3D'

	#topo = ''
	#shape = ''
	topo = 'dragonfly'
	shape = '16:32:16:32'

	return platform, topo, shape

def getDetailedModel():
    return "","",[]
