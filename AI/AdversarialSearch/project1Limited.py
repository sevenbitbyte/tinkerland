#!/usr/bin/python3

import os
import sys
import math

#Directions a knight may move
translations = [(-1,2),(1,2),(-2,1),(-2,-1)]

#Decides if a give point is occupied and within the game grid
def isValid(point, state):
	if(point[0] < 0 or point[1] > 0):
		return False

	if(point[0] >= 0 and point[1] <= 0):
		for player in state:
			for knight in player:
				if(knight[0] == point[0] and knight[1] == point[1]):
					return False
	return True


#Genereates a list of successors
def successors(state, player):
	moves = list([])
	for move in translations:
		update = (state[player][0][0]+move[0], state[player][0][1]+move[1])
		if(isValid(update,state)):
			moves.append( [update,state[player][1]] )
		update = (state[player][1][0]+move[0], state[player][1][1]+move[1])
		if(isValid(update,state)):
			moves.append( [state[player][0],update] )

	return moves


#Computes the area between a given point
#and the origin of the game grid
def areaOf(point):
	return point[0]*point[1]*-1


#Given a list of points computes the sum of their areas
def sumAreas(pointList):
	sum = 0
	for pair in pointList:
		sum = sum + areaOf(pair[0]) + areaOf(pair[1])
	return sum


#Heuristic function, given a game state computes the difference
#in possible move areas between player1 and player0
def areaDelta(state):
	return sumAreas([state[1]]) - sumAreas([state[0]])


#Heuristic function, given a game state computes the difference
#in possible successors between player1 and player0
def successorDelta(state):
	return len(successors(state, 1)) - len(successors(state, 0))


#Computes the manhattan distance from the given point to the origin
def manhattanDist(point):
	return math.fabs(point[0]) + math.fabs(point[1])


#Heuristic function, given a game state computes the difference
#in maximum possible move count between player1 and player0
def movesDelta(state):
	moves0 = manhattanDist(state[0][0]) +  manhattanDist(state[0][1])
	moves1 = manhattanDist(state[1][0]) +  manhattanDist(state[1][1])
	return moves1 - moves0
	

#Generates the next optimal move given the current game state,
#which player to play as, a maximum number of moves to lookahead,
#and a reference to a heuristic function to use when max depth is reached
def best_move_limited(state, player, depth, evalFuncRef):
	moves = successors(state, player)
	value = player
	enemy = player-1
	if(value == 0):
		value = -1
		enemy = 1

	if(len(moves) == 0):
		return [value * float('-inf'),[]]

	if(depth == 0):
		#print("Expected value = ", evalFuncRef(state), "at depth = ",depth)
		return [evalFuncRef(state), []]

	bestValue=float('-inf')
	bestMove=moves[0]
	#print("Have ", len(moves), " moves with state=",state)
	for s in moves:
		tempState=state
		tempState[player] = s
		#print("\tTrying ",tempState);
		result=best_move_limited(tempState, enemy, depth-1, evalFuncRef)
		#print("Tried ", tempState, " value=",result[0])
		if(result[0]*value > bestValue):
			bestValue = result[0]*value
			bestMove = s
			
	return [bestValue*value, bestMove]

#Playes out a full game given a starting state and which player should
#start first. Also, a heuristic function is required for each player
def play(state, player, evalFuncRef0, evalFuncRef1):
	moveCount = 0
	moveList = []

	lookAhead0 = 6
	lookAhead1 = 6
	moveList.append([state[0],state[1]])

	while True:
		evalFuncRef=evalFuncRef0
		depth = lookAhead0
		if(player == 1):
			evalFuncRef=evalFuncRef1
			depth=lookAhead1

		tempState = [state[0], state[1]]
		move = best_move_limited(tempState, player, depth, evalFuncRef)

		if(len(move[1]) == 0):
			break

		state[player] = move[1]
		moveCount = moveCount + 1
		moveList.append([state[0],state[1]])

		if(player == 1):
			player = 0
		else:
			player = 1

	return [player, moveList]

def pointToString(point):
	return str.format("{0[0]:0=+3}{0[1]:0=+3}", point)

def gameStateToString(state, turn=0):
	pt0 = pointToString(state[0][0])
	pt1 = pointToString(state[0][1])
	pt2 = pointToString(state[1][0])
	pt3 = pointToString(state[1][1])
	out_str = ''
	out_str += pt0 + pt1 + pt2 + pt3 + str.format("T{0}", turn)
	return out_str
	
#Draws a given game state using ImageMagick
def drawGameState(state, turn=0):
	fname = gameStateToString(state, turn)
	os.system(str.format("cp imgs/board.jpg simages/state{0}.jpg", fname))

	scale = 26.0

	fmtStr = "convert -composite -geometry +{0}+{1} -compose src-over simages/state{2}.jpg imgs/wn.gif simages/state{2}.jpg"
	os.system(str.format(fmtStr, math.fabs(state[0][0][0]*scale), math.fabs(state[0][0][1]*scale), fname))
	os.system(str.format(fmtStr, math.fabs(state[0][1][0]*scale), math.fabs(state[0][1][1]*scale), fname))
	
	fmtStr = "convert -composite -geometry +{0}+{1} -compose src-over simages/state{2}.jpg imgs/bn.gif simages/state{2}.jpg"
	os.system(str.format(fmtStr, math.fabs(state[1][0][0]*scale), math.fabs(state[1][0][1]*scale), fname))
	os.system(str.format(fmtStr, math.fabs(state[1][1][0]*scale), math.fabs(state[1][1][1]*scale), fname))

	return fname

#Constructs a webpage depicting the moves taken during the game
def buildWebPage(moveList, gameName):
	print("Rendering game...")

	os.system("mkdir html/")
	os.system("mkdir simages/")

	out_file = open(str.format("html/game{0}.html", gameName), "wt")
	out_file.write(str.format("<html><head><title>Game: {0}</title></head><body>\n", gameName))

	move=0
	for state in moveList:
		imgName = drawGameState(state)
		out_file.write(str.format("<b>Move {1}</b><br><img src=../simages/state{0}.jpg>\n<br>", imgName, move))
		move += 1

	out_file.write("</body></html>")
	out_file.close()
	return


#Parse command line options
if(sys.argv[1] != "-l"):
	origin = (int(sys.argv[1]), -int(sys.argv[2]))
	depthLimit = int(sys.argv[3])
	player1 = [(origin[0],origin[1]),(origin[0]+1,origin[1]-1)]
	player2 = [(origin[0]+1,origin[1]),(origin[0],origin[1]-1)]
else:
	depthLimit = int(sys.argv[10])
	player1 = [(int((sys.argv[2])),int((sys.argv[3]))),(int((sys.argv[4])),int((sys.argv[5])))]
	player2 = [(int((sys.argv[6])),int((sys.argv[7]))),(int((sys.argv[8])),int((sys.argv[9])))]

#Construct game state
gameState = [player1, player2]
initState = gameStateToString(gameState)

#Play game
moves = play(gameState, 0, movesDelta, movesDelta)
print("Player ", moves[0], " lost")

#Construct webpage output
buildWebPage(moves[1], initState)




