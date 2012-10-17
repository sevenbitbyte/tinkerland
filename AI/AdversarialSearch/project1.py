#!/usr/bin/python

#0. None
#1. floor
#2. square-root
#3. factorial

import os
import sys
import math

translations = [(-1,2),(1,2),(-2,1),(-2,-1)]

def successors(knights):
	moves = list([])
	#for knight in knights:
	for move in translations:
		if(knights[0][0]+move[0] >= 0 and knights[0][1]+move[1] <= 0):
			moves.append( [(knights[0][0]+move[0], knights[0][1]+move[1]), knights[1]] )
		if(knights[1][0]+move[0] >= 0 and knights[1][1]+move[1] <= 0):
			moves.append( [knights[0], (knights[1][0]+move[0], knights[1][1]+move[1])] )

	return moves

	
	

def best_move(state, player):
	moves = successors(state[player])
	value = player
	enemy = player-1
	if(value == 0):
		value = -1
		enemy = 1

	max = value * float('inf')

	if(len(moves) == 0):
		return [value * -1,[]]

	bestMove=moves[0]
	print("Have ", len(moves), " moves with state=",state)
	for s in moves:
		tempState=state
		tempState[player] = s
		print("\tTrying ",tempState);
		score=best_move(tempState, enemy)
		if(score == value):
			bestMove = state
	return bestMove

origin = (int(sys.argv[1]), -int(sys.argv[2]))

player1 = [(origin[0],origin[1]),(origin[0]+1,origin[1]-1)]
player2 = [(origin[0]+1,origin[1]),(origin[0],origin[1]-1)]

gameState = [player1, player2]

print(best_move(gameState, 0))



