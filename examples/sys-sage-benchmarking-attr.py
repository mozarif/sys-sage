#TODO Fix this bullshit

import sys_sage as syge 
import sys 
import os  
import timeit


#this file benchmarks and prints out performance information about basic operations with sys-sage
if __name__ == "__main__":
    

    t = syge.Topology()
    n = syge.Node(t, 1)
    
    def update():
        n["example"] = 1
    time_to_update = timeit.timeit(update, number = 1000000)
    print("time to update: "+str(time_to_update*1000))
    
    time_to_read = timeit.timeit(lambda: n["example"], number = 1000000)
    print("time to read: "+str(time_to_read*1000))
    
    

        
