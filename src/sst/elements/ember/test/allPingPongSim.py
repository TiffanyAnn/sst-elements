

def getWorkFlow( defaults ):
    workFlow = []
    motif = dict.copy( defaults )
    motif['cmd'] = "Init"
    workFlow.append( motif )

    motif = dict.copy( defaults )
    motif['cmd'] = "AllPingPong iterations=100 messageSize=256"
    workFlow.append( motif )

    motif = dict.copy( defaults )
    motif['cmd'] = "Fini"
    workFlow.append( motif )

	# numNodes = 0 implies use all nodes on network
    numNodes = 0
    numCores = 4

    return workFlow, numNodes, numCores

def getNetwork():

	platform = 'allPingPong'

	topo = 'dragonfly'
	shape = '16:32:16:16'
	return platform, topo, shape

def getDetailedModel():
    return "","",[]
