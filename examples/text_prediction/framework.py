import sys
import string
import random as rd
from functions import *

#Environment:
dictionary = dict()
iterations = 0
interactive = False
moveAhead = 0
quit = False
seed = rd.randint(0,1000000)

#Functions:

def usage():
    """Print out information regarding how to use this particular program."""
    print 'Execute with:\n python program.py [arguments]\n\
For example:\n\
 python main.py -config config.txt -i 1 -file readin.txt\n\
Note that command line arguments take precedence over arguments contained in a configurations file. For example, if -iters 20 is in the config file and -iters 100 is a command line argument, then -iters 100 will be used.\n\
Valid command line arguments include\n\
-config file_name\t\twhere file_name is the path to a file containing command line arguments, one per line.\n\
-i 1\t\tTurns on interactive mode.\n\
-iters #\t\twhere # is a positive integer. Specifies the number of iterations to run the program for.\n\
-seed #\t\twhere # is a positive integer. Sets the random seed to use for this run.\n\
'

def loadIntoDictionary(argv, dictionary):
    """Parse the argv array to a dictionary of strings->strings or 
    strings->string arrays. We assume all flags begin with a hyphen and all
    arguments to flags do not. So if we see -abc 1 2 3 then in the 
    dictionary we will get: dict['abc'] = ['1','2','3']"""
    limit = len(argv)
    i = 0
    while i < limit:
        if argv[i].startswith('-'):
            j = i+1
            temp = []
            while j < limit and not argv[j].startswith('-'):
                temp.append(argv[j])
                j += 1
            if len(temp) == 1:
                dictionary[argv[i]] = temp[0]
            elif len(temp) > 1:
                dictionary[argv[i]] = temp
            else:
                print 'ERROR: Every input beginning with '+\
                '"-" should take at least one argument. "'+\
                argv[i]+'". Has no such argument.\n'
                usage()
                sys.exit()
            i = j-1
        i += 1
    return dictionary


def parseInput(argv):
    """So if you want to know how the pros deal with a main method 
    and inputs then check here:
    http://www.artima.com/weblogs/viewpost.jsp?thread=4829
    also: http://snipplr.com/view/2050/
    Or this: http://www.artima.com/forums/flat.jsp?forum=106&thread=4829
    But I tried those Then said, fuck this neck-beard bull shit, I'm doing 
    it my own way."""
    #This is important for if you're interacting with the program through 
    #the python interpreter or something. Don't ask me.
    if argv is None:
        argv = sys.argv
    #User SOS: Check for help.
    #If you want to print help and exit if no arguments are given then 
    #add the condition "or len(argv)==1"
    if '-h' in argv or '-help' in argv:
        print '\nUser SOS'
        usage()
        sys.exit(2)
    #Initialize dictionary
    dictionary = dict()
    #Check for a config file. Format it if found.
    if '-config' in argv:
        temp = argv.index('-config')+1
        try: #Try to open the file.
            f = open(argv[temp], 'r')
        except IOError as e:
            print 'ERROR: Argument following --config should be a '\
            +'file. Could not open "'+str(argv[temp])+\
            '" file does not exist.\n'
            usage()
            sys.exit()
        contents = f.read()
        f.close()
        #Format the file, converting line breaks to blank spaces and 
        #then creating an array of strings.
        contents = contents.replace("\n",' ')
        contents = contents.strip()
        contents = contents.split(' ')
        #Add contents to the dictionary
        dictionary = loadIntoDictionary(contents, dictionary)
    #Next, add in command line options. Anything beginning with a hyphen, 
    #if it's already in the dictionary, replace the value in the dict with 
    #the command line args.
    return loadIntoDictionary(argv, dictionary)


def checkForInput(quit, interactive, moveAhead):
    global wordlist
    while True:
        inStr = raw_input('Waiting on user input... (Type: h for help)\n>')
        if inStr == 'q' or inStr == 'quit':
            quit = True; break
        elif inStr == '#' or inStr == 'num':
            temp=raw_input('Enter the number of generations to advance.\n>')
            moveAhead = int(temp); break
        elif inStr == 'f' or inStr == 'finish':
            interactive = False; break
        elif inStr == 'l' or inStr == 'limit':
            temp=raw_input('Enter the new word limit.\n>')
            global sentenceLengthLimit
            sentenceLengthLimit = int(temp)
        elif inStr == 'd':
            wordlist.getSentenceDeterministic()
        elif inStr == 'w':
            wordlist.getSentenceWeighted()
        elif inStr == 'u':
            wordlist.getSentenceUniform()
        else:
            print 'l or limit\t - \tYou will then be prompted to enter a number to \n\
\t\t\tChange the sentence length limit.\n\
d\t - \tGet a deterministic sentence.\n\
w\t - \tGet a weighted random sentence.\n\
u\t - \tGet a uniform random sentence.\n\
\nfinish or f\t - \tFinish this run. \n\
\t\t\tTurn off interactive mode.\n\
num or #\t - \tYou will then be prompted to enter a number of \n\
\t\t\titerations to advance the computation\n\
quit or q\t - \tStop the simulation right now.\n\
help or h\t - \tDisplay this text.'
    return quit, interactive, moveAhead
