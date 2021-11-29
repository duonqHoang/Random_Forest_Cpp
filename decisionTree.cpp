#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <ctime>
using namespace std;

const string trainPath = "train.txt";

//Import normal dataset from file
vector<vector<int>> getDataSet(string path){
    ifstream inFile(path);
    vector<vector<int>> dataSet;
    vector<int> row;
    string line;
    if(inFile.is_open()){
        while(getline(inFile,line)){
            row.clear();
            stringstream ss(line);
            string temp;
            while(getline(ss,temp,',')){
                row.push_back(temp.at(0)-'0');
            }
            dataSet.push_back(row);
        }
    }
    inFile.close();
    return dataSet;
}

//Import no label dataset from file
vector<vector<int>> getNoLabelSet(string path){
    ifstream inFile(path);
    vector<vector<int>> dataSet;
    vector<int> row;
    string line;
    if(inFile.is_open()){
        while(getline(inFile,line)){
            row.clear();
            row.push_back(0);
            stringstream ss(line);
            string temp;
            while(getline(ss,temp,',')){
                row.push_back(temp.at(0)-'0');
            }
            dataSet.push_back(row);
        }
    }
    inFile.close();
    return dataSet;
}

//Duplicate B-Label rows in dataset by n times
void duplicateBRows(vector<vector<int>> &dataset, int times){
    for(int index=dataset.size()-1;index >= 0;index--){
        if(dataset[index][0] == 'B' - '0'){
            for(int i=1;i<times;i++){
                dataset.push_back(dataset[index]);
            }
        }
        else return;
    }
}

//Split the dataset into n folds
vector<vector<vector<int>>> cross_val_split(vector<vector<int>> dataset, int n_folds){
    vector<vector<vector<int>>> dataset_split;
    vector<vector<int>> dataset_copy = dataset;
    int fold_size = int(dataset.size() / n_folds);
    for(int i=0;i<n_folds;i++){
        vector<vector<int>> fold;
        while(fold.size()<fold_size){
            int index = rand()%dataset_copy.size();
            fold.push_back(dataset_copy[index]);
            dataset_copy.erase(next(dataset_copy.begin(), index));
        }
        dataset_split.push_back(fold);
    }
    return dataset_split;
}

struct Node{
    Node* left;
    Node* right;
    vector<vector<int>> group;
    int attIndex =-1;
    int splitValue =-1;
    bool isLeaf = false;
    char label;

    Node(){

    }

    Node(vector<vector<int>> dataset){
        group = dataset;
    }

    //Make current node leaf
    void to_terminal(){
        isLeaf = true;
        left = nullptr;
        right = nullptr;
        int lbrcount[3] = {0,0,0};
        for (int row = 0;row < group.size();row++){
            if (group[row][0]==('L'-'0')) lbrcount[0]++;
            else if (group[row][0]==('B'-'0')) lbrcount[1]++;
            else lbrcount[2]++;
        }
        int max = lbrcount[0];
        if(lbrcount[1] > max) max = lbrcount[1];
        if(lbrcount[2] > max) max = lbrcount[2];
        if(max == lbrcount[0]) label = 'L';
        else if(max == lbrcount[1]) label = 'B';
        else label = 'R';
    }
};

struct Tree{
    Node* root;

    Tree(Node* node){
        root = node;
    }
};

//Calculate Gini index
float getGiniScore(vector<vector<vector<int>>> groups, vector<int> classes){
    int n_instances = groups[0].size() + groups[1].size();
    float gini = 0;
    for(int i=0;i<2;i++){
        int size = groups[i].size();
        if (size == 0) continue;
        float score = 0.0;
        for (int j=0;j<classes.size();j++){
            int count = 0;
            for(int k = 0;k<groups[i].size();k++){
                if (groups[i][k][0] == classes[j]) count++;
            }
            float p = (float) count/(float)size;
            score+= p*p;
        }
        gini+= (1.0-score)*((float)size / (float)n_instances);
    }
    return gini;
}

//Split dataset into 2 groups with a choosen attribute value
vector<vector<vector<int>>> test_split (int index, int value, vector<vector<int>> dataset){
    vector<vector<vector<int>>> groups;
    vector<vector<int>> left;
    vector<vector<int>> right;
    for(int i=0;i<dataset.size();i++){
        if(dataset[i][index] < value){
            left.push_back(dataset[i]);
        }
        else{
            right.push_back(dataset[i]);
        }
    }
    groups.push_back(left);
    groups.push_back(right);
    return groups;
}

//Get best split
Node* get_split(vector<vector<int>> dataset, int n_features){
    vector<int> classes = {'L'-'0','R'-'0','B'-'0'};
    float b_score = 10.0;
    Node* newNode = new Node(dataset);
    Node* left = new Node();
    Node* right = new Node();
    newNode -> left = left;
    newNode -> right = right;
    vector<vector<vector<int>>> groups;
    vector<int> features;
    srand(time(NULL));

    while(features.size() < n_features){
        int index = 1 + rand()%4;
        bool present = false;
        for(int i=0;i<features.size();i++){
            if(features[i] == index) present = true;
        }
        if(!present){
            features.push_back(index);
        }
    }

    for(int i = 0;i < features.size();i++){
        for(int value = 1; value <=5; value++){
            groups = test_split(features[i],value,dataset);
            float gini = getGiniScore(groups,classes);
            //cout<<"X"<<index<<" < "<<value<<" gini = "<<gini<<endl;
            if (gini < b_score){
                b_score = gini;
                newNode->attIndex = features[i];
                newNode->splitValue = value;
                left->group = groups[0];
                right->group = groups[1];
            }
        }
    }
    return newNode;
}

// Create child splits for a node or make terminal
void split(Node* nodeTree, int maxDepth, int minSize,int n_features, int depth){
    if(nodeTree->left == nullptr || nodeTree->right == nullptr
       || nodeTree->left->group.size() == 0 || nodeTree->right->group.size() == 0){
        nodeTree->to_terminal();
        return;
    };
    if (depth >= maxDepth){
        nodeTree->left->to_terminal();
        nodeTree->right->to_terminal();
        return;
    };
    if (nodeTree->left->group.size() <= minSize){
        nodeTree->left->to_terminal();
    }
    else{
        nodeTree->left = get_split(nodeTree->left->group, n_features);
        split(nodeTree->left, maxDepth, minSize, n_features, depth+1);
    };
    if (nodeTree->right->group.size() <= minSize){
        nodeTree->right->to_terminal();
    }
    else{
        nodeTree->right = get_split(nodeTree->right->group, n_features);
        split(nodeTree->right, maxDepth, minSize, n_features, depth+1);
    }
}

//Build a decision tree
Tree* buildTree(vector<vector<int>> dataset, int maxDepth, int minSize, int n_features){
    Node* root = get_split(dataset, n_features);
    split(root, maxDepth, minSize, n_features,1);
    Tree* newTree = new Tree(root);
    return newTree;
}

// Make a prediction with a decision tree
char predict(Node* node, vector<int> row){
    if(row[node->attIndex] < node->splitValue){
        if(node->left->isLeaf == false){
            return predict(node->left, row);
        }
        else{
            return node->left->label;
        }
    }
    else{
        if(node->right->isLeaf == false){
            return predict(node->right, row);
        }
        else{
            return node->right->label;
        }
    }
}

// Create a random subsample from the dataset with replacement
vector<vector<int>> subsample(vector<vector<int>> dataset, int n_sample){
    vector<vector<int>> sample;
    srand(time(NULL));
    while(sample.size() < n_sample){
        int i = rand() % dataset.size();
        sample.push_back(dataset[i]);
    }
    return sample;
}

//Predict with a set of trees
char bagging_predict(vector<Tree*> trees, vector<int> row){
    int lbrcount[3] = {0,0,0};
    for(int i=0;i<trees.size();i++){
        char c = predict(trees[i]->root, row);
        if (c=='L') lbrcount[0]++;
        else if (c=='B') lbrcount[1]++;
        else lbrcount[2]++;
    }
    int max = lbrcount[0];
    if(lbrcount[1] > max) max = lbrcount[1];
    if(lbrcount[2] > max) max = lbrcount[2];
    if(max == lbrcount[0]) return 'L';
    else if(max == lbrcount[1]) return 'B';
    else return 'R';
}

// Random Forest Algorithm
vector<Tree*> random_forest(vector<vector<int>> train,
                            int max_depth, int min_size, int sample_size, int n_trees, int n_features){
    vector<Tree*> trees;
    for(int i=0;i<n_trees;i++){
        vector<vector<int>> sample = subsample(train, sample_size);
        Tree* tree = buildTree(sample, max_depth, min_size, n_features);
        trees.push_back(tree);
    }
    return trees;
}

//Print a decision tree
void printTree(Node* node, int depth=0){
    if (node->isLeaf == false){
        for(int i=0;i<depth;i++){
            cout<<"  ";
        }
        cout<<'['<<"X"<<node->attIndex<<" < "<<  node->splitValue<<']'<<endl;
        printTree(node->left, depth+1);
        printTree(node->right, depth+1);
    }
    else{
        for(int i=0;i<depth;i++){
            cout<<"  ";
        }
        cout<< '[' << node->label << ']'<<endl;
    }
}

//Calculate accuracy with vectors of labels
float accuracy_metric(vector<int> actual , vector<int> predicted){
    int correct = 0;
    for(int i=0;i<actual.size();i++){
        if(actual[i] == predicted[i]) correct++;
    }
    return (float)correct * 100.0 / (float)actual.size();
}

//Calculate accuracy with a forest and a test dataset
float accuracy_metric(vector<Tree*> forest, vector<vector<int>> testSet){
    vector<int> actual;
    vector<int> predicted;
    for(int i=0;i<testSet.size();i++){
        actual.push_back(testSet[i][0]);
        predicted.push_back(bagging_predict(forest,testSet[i])-'0');
    }
    return accuracy_metric(actual, predicted);
}

//Evaluate Random Forest algorithm with cross validation
float evaluate_forest(vector<vector<int>> dataset, int n_folds,
                               int max_depth, int min_size, int sample_size, int n_trees, int n_features){
    vector<vector<vector<int>>> folds = cross_val_split(dataset, n_folds);
    vector<float> scores;

    for(int curr=0;curr<folds.size();curr++){
        vector<vector<int>> train;
        vector<vector<int>> test = folds[curr];
        for(int i=0;i<folds.size();i++){
            if(i != curr){
                train.insert(train.end(),folds[i].begin(),folds[i].end());
            }
        }
        vector<int> actual;
        vector<int> predicted;
        vector<Tree*> trees = random_forest(train,max_depth,min_size,sample_size,n_trees,n_features);

        for(int i=0;i< test.size();i++){
            actual.push_back(test[i][0]);
            predicted.push_back(bagging_predict(trees, test[i])-'0');
        }

        scores.push_back(accuracy_metric(actual,predicted));
    }
    float scoreSum = 0.0;
    for(int i=0;i<scores.size();i++){
        scoreSum += scores[i];
    }
    return scoreSum/scores.size();
}

//Export data with predicted labels to file
void exportResult(vector<vector<int>> &testSet, vector<int> predict){
    for(int i=0;i<testSet.size();i++){
        testSet[i][0] = predict[i];
    }
    ofstream output("result.txt") ;
    for(int row=0;row<testSet.size();row++){
        for(int col = 0;col<5;col++){
            output << char(testSet[row][col] + '0');
            if(col < 4) output<<',';
        }
        output<<endl;
    }
    output.close();
}

int main(){
    vector<vector<int>> trainSet = getDataSet(trainPath);
    vector<vector<int>> testSet = getDataSet("valid.txt");
    vector<vector<int>> hiddenTest = getNoLabelSet("private_test.txt");
    duplicateBRows(trainSet, 7);

    int max_depth = 10;
    int min_size = 2;
    int sample_size = trainSet.size()*1/2;
    int n_trees = 300;
    int n_features = 4;

    /*
    vector<int> actual;
    vector<int> predicted;

    Tree* testTree = buildTree(trainSet, max_depth, min_size, 4);
    printTree(testTree->root);

    for(int i=0;i<testSet.size();i++){
        actual.push_back(testSet[i][0]);
        predicted.push_back(predict(testTree->root,testSet[i])-'0');
    }
    */
    //cout<<evaluate_forest(trainSet,7,max_depth,min_size,sample_size,n_trees,n_features);

    vector<Tree*> forest = random_forest(trainSet,max_depth,min_size,sample_size,n_trees,n_features);
    cout<<accuracy_metric(forest,testSet);

    vector<int> result;
    for(int i=0;i<hiddenTest.size();i++){
        result.push_back(bagging_predict(forest,hiddenTest[i])-'0');
    }
    exportResult(hiddenTest,result);

    return 0;
}
