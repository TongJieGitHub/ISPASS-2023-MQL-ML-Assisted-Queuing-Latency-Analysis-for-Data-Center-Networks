## Load libraries
import pandas as pd
import numpy as np
from sklearn.tree import DecisionTreeClassifier 
from sklearn.model_selection import train_test_split 
from sklearn import metrics 
from sklearn import tree
import scipy.io
import pickle
from sklearn.metrics import f1_score
from sklearn.metrics import recall_score

## Input information and parameters
num_features          = 12
train_test_split_perc = 0.1
#max_tree_depth        = 6
hit_list = [0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1]

max_tree_depth_list = [3]
df = pd.read_csv("flowlog_mean_DT_0428.csv", header=0)
df['src address'] = df['src address'].astype('str')
df['des address'] = df['des address'].astype('str')
#df = df[df['src address'] == '201.1.1.2']
#df = df[(df['node ID'] == 20) | (df['node ID'] == 21) | (df['node ID'] == 22) | (df['node ID'] == 23) | (df['node ID'] == 24) | (df['node ID'] == 25) | (df['node ID'] == 26) | (df['node ID'] == 27)]
df = df[(df['node ID'] == 12) | (df['node ID'] == 13) | (df['node ID'] == 14) | (df['node ID'] == 15)]
# df = df[(df['node ID'] == 4) | (df['node ID'] == 6)]
# df = df[(df['node ID'] == 0)]
#df = df[(df['interface ID'] == 1)]
#df = df[df['stage'] == 0]
## If feature names are not available, decide based on number of features
#X = diabetes.iloc[:, 3 : 9]
X = df.iloc[:, [6, 7, 8]]
y = df.iloc[:, -1]

## Partition the data based on train-test-split
## The 0.3 indicates that the training set is 30% of the full set
## You can change this number
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=train_test_split_perc, random_state=1)


for max_tree_depth in max_tree_depth_list:

    ## Create a new decision tree model
    model =  DecisionTreeClassifier(max_depth=max_tree_depth)
    
    ## Train Decision Tree Classifer
    model = model.fit(X_train, y_train)
    
    filename = 'DT_depth_'+str(max_tree_depth)
    pickle.dump(model, open(filename, 'wb'))
    #model.predict_proba()
    ## Visualize the tree
    #print(tree.export_text(model, decimals=6, feature_names=["stage", "node ID", "interface ID", "data rate", "CA_sqr", "CS_sqr"]))
    print(tree.export_text(model, decimals=6, feature_names=["data rate", "CA_sqr", "CS_sqr"], show_weights=True))
    
    ## Compute accuracy on the test dataset
    test_accuracy = np.sum(model.predict(X_test) == y_test) / len(y_test) * 100
    full_accuracy = np.sum(model.predict(X) == y) / len(y) * 100
    f1 = f1_score(y, (model.predict(X)), average=None)
    recall = recall_score(y, (model.predict(X)), average=None)
    print()
    print('[ INFO] Accuracy on test set: %.2f' %(test_accuracy))
    print('[ INFO] Accuracy on full set: %.2f' %(full_accuracy))
    print('f1: ', f1)
    print('recall: ', recall)
    # filename = 'DT_depth_'+str(max_tree_depth)
    # regressor = pickle.load(open(filename, 'rb'))
    # for hit_rate in hit_list:
    #     st_filename = 'source_throttling_' + str(hit_rate) +'.csv'
    #     #st_filename = 'source_throttling.csv'
    #     data_hit_rate = pd.read_csv(st_filename, header=0)
        
    #     features_hit = data_hit_rate.iloc[:, 0 : num_features]
    #     label_hit = data_hit_rate.iloc[:, -1]
        
    #     #pred = regressor.predict(data_hit_rate)
    #     full_accuracy_hitrate = np.sum(regressor.predict(features_hit) == label_hit) / len(y) * 100
    #     print()
    #     print('[ INFO] Accuracy on full set with hit rate '+ str(hit_rate) + ' and max tree depth ' + str(max_tree_depth)+' is ' + str(full_accuracy_hitrate))
    #     print()