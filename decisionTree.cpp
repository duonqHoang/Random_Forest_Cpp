#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
using namespace std;

const string trainPath = "train.txt";

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

Node* get_split(vector<vector<int>> dataset){
    vector<int> classes = {'L'-'0','R'-'0','B'-'0'};
    float b_score = 10.0;
    Node* newNode = new Node(dataset);
    Node* left = new Node();
    Node* right = new Node();
    newNode -> left = left;
    newNode -> right = right;
    vector<vector<vector<int>>> groups;
    for(int index = 1;index < dataset[0].size();index++){
        for(int value = 1; value <=5; value++){
            groups = test_split(index,value,dataset);
            float gini = getGiniScore(groups,classes);
            //cout<<"X"<<index<<" < "<<value<<" gini = "<<gini<<endl;
            if (gini < b_score){
                b_score = gini;
                newNode->attIndex = index;
                newNode->splitValue = value;
                left->group = groups[0];
                right->group = groups[1];
            }
        }
    }
    return newNode;
}

void split(Node* nodeTree, int maxDepth, int minSize, int depth){
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
        nodeTree->left = get_split(nodeTree->left->group);
        split(nodeTree->left, maxDepth, minSize, depth+1);
    };
    if (nodeTree->right->group.size() <= minSize){
        nodeTree->right->to_terminal();
    }
    else{
        nodeTree->right = get_split(nodeTree->right->group);
        split(nodeTree->right, maxDepth, minSize, depth+1);
    }
}

Tree* buildTree(vector<vector<int>> dataset, int maxDepth, int minSize){
    Node* root = get_split(dataset);
    split(root, maxDepth, minSize,1);
    Tree* newTree = new Tree(root);
    return newTree;
}

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

float accuracy_metric(vector<int> actual, vector<int> predicted){
    int correct = 0;
    for(int i=0;i<actual.size();i++){
        if(actual[i] == predicted[i]) correct++;
    }
    return (float)correct * 100.0 / (float)actual.size();
}

int main(){
    vector<vector<int>> trainSet = getDataSet(trainPath);
    vector<vector<int>> testSet = getDataSet("valid.txt");
    Tree* testTree = buildTree(trainSet, 10, 2);

    vector<int> actual;
    vector<int> predicted;

    for(int row=0;row<testSet.size();row++){
        actual.push_back(testSet[row][0]);
        predicted.push_back(predict(testTree->root, testSet[row])-'0');
    }

    printTree(testTree->root);
    cout<<"Accuracy: "<< accuracy_metric(actual, predicted) <<"%"<<endl;
    return 0;
}
