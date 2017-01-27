from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
from past.utils import old_div
from __pylib__  import *
from sys import * 
import random as r 
sys.path.append("../random") 
import randomWeighted as rw 
class WordNode():
    def __init__ (self): 
        self.incoming = [] 
        self.outgoing = dict() 
        self.count = 1  
    def addIncoming (self,word): 
        if (not word in self.incoming): 
            self.incoming.append(word) 
          
    def addOutgoing (self,word): 
        if (word in self.outgoing.keys()): 
            self.outgoing[word] += 1 
          
    def incrementCount (self): 
        self.count += 1  
    def getNextWordUniform (self): 
        rand = r.randint(0,len(self.outgoing.keys()) - 1) 
        key = self.outgoing.keys()[rand] 
        return key  
    def getNextWordDeterministic (self): 
        heaviest = [] 
        weight = 0 
        for key in self.outgoing.keys(): 
            if (self.outgoing[key] > weight): 
                weight = self.outgoing[key] 
                heaviest = [key] 
            elif self.outgoing[key] == weight:
                heaviest.append(key)   
        key = heaviest[0] 
        if (len(heaviest) > 1): 
            key = heaviest[r.randint(0,len(heaviest) - 1)] 
         
        return key  
    def getNextWordWeighted (self): 
        weights = [] 
        for key in self.outgoing.keys(): 
            weights.append(self.outgoing[key])  
        randgen = rw.WeightedRandomGenerator(weights) 
        key = self.outgoing.keys()[randgen.next()] 
        return key   
