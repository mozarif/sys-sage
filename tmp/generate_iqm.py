
import sys
import json
import random

def main():
    print(sys.argv[1:])
    dim_sz = int(sys.argv[1])
    print("dim_sz: " + str(dim_sz))

    test_output={}
    test_output['backend_name'] = "Q-Exa"
    test_output['timestamp'] = "2024-11-12 03:37:55.963360+00:00"
    test_output['T1'] = []
    test_output['T2'] = []
    test_output['1q_fidelity'] = []
    test_output['readout_fidelity'] = []
    test_output['two_q_fidelity'] = {}

    for i in range(dim_sz):
        for j in range(dim_sz):
            test_output['T1'].append(str(random.random()))
            test_output['T2'].append(str(random.random()))
            test_output['1q_fidelity'].append(str(random.random()))
            test_output['readout_fidelity'].append(str(random.random()))
            #if not the last in the row, create fidelity to the next one
            if j < dim_sz-1:
                q1=i*dim_sz+j
                q2=i*dim_sz+j+1
                test_output['two_q_fidelity'][str(q1)+','+str(q2)] = str(random.random())
            if i < dim_sz-1:
                q1=i*dim_sz+j
                q2=(i+1)*dim_sz+j
                test_output['two_q_fidelity'][str(q1)+','+str(q2)] = str(random.random())


    with open('test_data_qexa_' + str(dim_sz*dim_sz) + '.json', 'w') as f:
        print(json.dumps(test_output, indent=4), file=f)
    
    
    #print(json.dumps(test_output, indent=4))


if __name__ == "__main__":
    main()