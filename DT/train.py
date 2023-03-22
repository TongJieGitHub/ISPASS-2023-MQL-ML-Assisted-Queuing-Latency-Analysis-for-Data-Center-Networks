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
from sklearn.tree import _tree

def tree_to_code_weighted(queue, tree, feature_names):
    tree_ = tree.tree_
    feature_name = [
        feature_names[i] if i != _tree.TREE_UNDEFINED else "undefined!"
        for i in tree_.feature
    ]
    #feature_names = [f.replace(" ", "_")[:-5] for f in feature_names]
    print("def predict_{}_weighted({}):".format(queue, ", ".join(feature_names)))

    def recurse(node, depth):
        indent = "    " * depth
        if tree_.feature[node] != _tree.TREE_UNDEFINED:
            name = feature_name[node]
            threshold = tree_.threshold[node]
            print("{}if {} <= {}:".format(indent, name, np.round(threshold,6)))
            recurse(tree_.children_left[node], depth + 1)
            print("{}else:  # if {} > {}".format(indent, name, np.round(threshold,6)))
            recurse(tree_.children_right[node], depth + 1)
        else:
            print("{}return ".format(indent), end ="")
            list = (tree_.value[node] / np.sum(tree_.value[node]))[0].tolist()
            print(list, sep=", ")

    recurse(0, 1)


def tree_to_code(queue, tree, feature_names, label_names):
    tree_ = tree.tree_
    feature_name = [
        feature_names[i] if i != _tree.TREE_UNDEFINED else "undefined!"
        for i in tree_.feature
    ]
    #feature_names = [f.replace(" ", "_")[:-5] for f in feature_names]
    print("def predict_{}({}):".format(queue, ", ".join(feature_names)))

    def recurse(node, depth):
        indent = "    " * depth
        if tree_.feature[node] != _tree.TREE_UNDEFINED:
            name = feature_name[node]
            threshold = tree_.threshold[node]
            print("{}if {} <= {}:".format(indent, name, np.round(threshold,6)))
            recurse(tree_.children_left[node], depth + 1)
            print("{}else:  # if {} > {}".format(indent, name, np.round(threshold,6)))
            recurse(tree_.children_right[node], depth + 1)
        else:
            print("{}return \"".format(indent), end ="")
            list = (tree_.value[node] / np.sum(tree_.value[node]))[0].tolist()
            label = label_names[list.index(max(list))]
            print(label, end ="")
            print("\"")

    recurse(0, 1)


date = '0504'
#traffic = ['incast', 'outcast']
traffic = ['alltoall', 'incast', 'interleaved', 'outcast']
#queue_list = ['client_up', 'edge_down', 'edge_up', 'agg_down', 'agg_up', 'core_down']
queue_list = ['core_down']

## Input information and parameters
train_test_split_perc = 0.2
max_tree_depth        = 8
# max_tree_depth_list = [8]

# df = pd.read_csv("flowlog_mean_DT_0428.csv", header=0)
# X = df.iloc[:, [6, 7, 8]]
# y = df.iloc[:, -1]

X, y, X_train, X_test, y_train, y_test = pd.DataFrame(), pd.DataFrame(), pd.DataFrame(), pd.DataFrame(), pd.DataFrame(), pd.DataFrame()

for queue in queue_list:
    for t in traffic:
        print('reading ' + ' flowlog_mean_DT_' + date +'_' + t + '_' + queue + '.csv')
        df = pd.read_csv('flowlog_mean_DT_' + date +'_' + t + '_' + queue + '.csv', header=0)    
        features = df.iloc[:, [6, 7, 8, 9, 10, 11]]
        label = df.iloc[:, -2]
        features_train, features_test, label_train, label_test = train_test_split(features, label, test_size=train_test_split_perc, random_state=1)
        X = pd.concat([X, features], ignore_index=True, sort=False)
        y = pd.concat([y, label], ignore_index=True, sort=False)
        X_train = pd.concat([X_train, features_train], ignore_index=True, sort=False)
        X_test = pd.concat([X_test, features_test], ignore_index=True, sort=False)
        y_train = pd.concat([y_train, label_train], ignore_index=True, sort=False)
        y_test = pd.concat([y_test, label_test], ignore_index=True, sort=False)

        ## Create a new decision tree model
        model =  DecisionTreeClassifier(max_depth=max_tree_depth)
        
        ## Train Decision Tree Classifer
        model = model.fit(X_train, y_train)
        
        # filename = 'DT_depth_'+str(max_tree_depth)
        # pickle.dump(model, open(filename, 'wb'))
        
        #model.predict_proba()
        ## Visualize the tree
        #print(tree.export_text(model, decimals=6, feature_names=["stage", "node ID", "interface ID", "data rate", "CA_sqr", "CS_sqr"]))
        print(tree.export_text(model, decimals=6, feature_names=["data rate", "rho", "rho_total", "num_flows", "CA_sqr", "CS_sqr"], show_weights=True))
        
        ## Compute accuracy on the test dataset
        pred = model.predict(X_test)
        test_accuracy = np.sum(pd.DataFrame(model.predict(X_test)) == y_test) / len(y_test) * 100
        full_accuracy = np.sum(pd.DataFrame(model.predict(X)) == y) / len(y) * 100
        f1 = f1_score(y, (model.predict(X)), average=None)
        recall = recall_score(y, (model.predict(X)), average=None)
        print()
        print('[ INFO] Accuracy on test set: %.2f' %(test_accuracy))
        print('[ INFO] Accuracy on full set: %.2f' %(full_accuracy))
        print('f1: ', f1)
        print('recall: ', recall)
        tree_to_code(queue, model, ["data_rate", "rho", "rho_total", "num_flows", "CA_sqr", "CS_sqr"], ["GEG1", "link"])
        print()
        print()
        print()
        tree_to_code_weighted(queue, model, ["data_rate", "rho", "rho_total", "num_flows", "CA_sqr", "CS_sqr"])








