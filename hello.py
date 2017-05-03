#!/usr/bin/python


def isWon(game, turn):
    diagsum1 = 0
    diagsum2 = 0
    for x in range(0,2):
        if game[x][0] + game[x][1] + game[x][2] == -3 or game[x][0] + game[x][1] + game[x][2] == 3:
            print "player %d has won the game!\n" % (turn%2+1)
            return True
        elif game[0][x] + game[1][x] + game[2][x] == -3 or game[0][x] + game[1][x] + game[2][x] == 3:
            print "player %d has won the game!\n" % (turn%2+1)
            return True
        diagsum1 = diagsum1 + game[x][x]
        diagsum2 = diagsum2 + game[x][2-x]
        if (diagsum1 == 3 or diagsum2 == 3 or diagsum1 == -3 or diagsum1 == -3):
            print "player %d has won the game!\n" % (turn%2+1);
            return True
    return False;

def isValidMove(game, x, y):
    if (x > 2 or x < 0 or y > 2 or y < 0): return False
    if (game[x][y] != 0): return False
    return True

def printGame(game):
    for y in range(3):
        for x in range(3):
            if (game[x][y] != 0):
                if (game[x][y] == -1): print "O",
                else: print "X",
            else:
                print " ",
            if (x != 2): print "|",
        print "\n",

def advanceTurn(game, turn):
    printGame(game)
    while(True):
        x = int(raw_input("enter x location:"))
        y = int(raw_input("enter y location:"))
        if(isValidMove(game,x,y)): break

    if (turn%2 == 0):
        game[x][y] = -1
    else :
        game[x][y] = 1
    print "turn = %d" % turn

#Main
game = [[0 for x in range(3)] for y in range(3)]
turn = 0
outcome = False
while (not outcome):
    advanceTurn(game, turn)
    outcome = isWon(game,turn)
    turn = turn+1

printGame(game)
