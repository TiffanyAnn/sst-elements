

def getWorkFlow( defaults ):
    workFlow = []
    motif = dict.copy( defaults )
    motif['cmd'] = "Init"
    workFlow.append( motif )

    motif = dict.copy( defaults )
    motif['cmd'] = "Random messagesize=1024 iterations=100"
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
	platform = 'random'

	#topo = ''
	#shape = ''
	topo = 'dragonfly'
	shape = '16:32:16:32'

	return platform, topo, shape

def getDetailedModel():
    return "","",[]