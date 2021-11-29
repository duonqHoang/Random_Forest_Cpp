# Decision Tree and Random Forest algorithm

Giáo viên hướng dẫn: CN Nguyễn Đình Tuân.

Tham khảo: 
  
https://machinelearningmastery.com/implement-decision-tree-algorithm-scratch-python/

https://machinelearningmastery.com/implement-random-forest-scratch-python/
## 1. Bộ dữ liệu và output
### Input:
Sử dụng cho bài toán phân loại cân có thăng bằng hay không.
- Mỗi row có dạng (Y,X1,X2,X3,X4):
  - 4 Thuộc tính: {X1 , X2 , X3 , X4}
  - Nhãn Y là một trong 3 giá trị (L,R,B) : cân bị nghiêng về trái (L), nghiêng về bên phải(R) và cân bằng (B).
  
    ![image](https://user-images.githubusercontent.com/79439580/143393606-ab8c9900-6491-411e-8801-89af9c9158ff.png)
  
- File train.txt dùng để build forest
- File valid.txt dùng để test và đánh giá forest
- File private_test.txt gồm các row không có nhãn dùng để dự đoán bằng forest

### Output:

- Dự đoán file bất kì và xuất kết quả ra file result.txt
- In ra độ chính xác với 1 file dữ liệu có sẵn nhãn
- Có thể in ra 1 cây quyết định

## 2.Thuật toán và module

- Decision tree : tạo ra 1 cây quyết định với các thông số max depth, min size
- Random forest : tạo ra 1 rừng cây quyết định với số lượng cây, kích thước và số thuộc tính tuỳ chọn
- Cross validation : đánh giá thuật toán nếu cần trong trường hợp không có file test

## 3.Cấu trúc

- Struct Node : 
  - Lưu 1 group các rows, thuộc tính và chỉ số để chia tiếp ra Node left và right nếu là node trong
  - Sau khi dùng hàm to_terminal sẽ trở thành Node lá với Label là L, R hoặc B
- Struct Tree :
  - Lưu root Node
- GetDataSet(path): Trả về một group các rows được đọc từ file gồm các rows có nhãn
- GetNoLabelSet(path): Giống hàm GetDataSet nhưng dùng cho file gồm các rows không có nhãn
- duplicateBRows(dataset): 
- Cross_val_split(dataset, n_folds): chia một dataset(group các rows) thành n_folds phần với các phần tử chọn random (không trùng) từ dataset
- GetGiniScore(groups, classes): tính chỉ số Gini cho 2 group với classes là các nhãn (L, R, B)
- Test_split(index, value, dataset): chia dataset ra làm 2 phần dựa trên giá trị value của thuộc tính thứ index
- Get_split(dataset, n_features): chọn ra cách chia dataset tốt nhất sử dụng n_features thuộc tính ngẫu nhiên
- Split(Node, maxDepth, minSize, n_features, depth): xây một Decision tree vói root node, max depth, min size, n_features bằng đệ quy
- buildTree(...) trả về một Decision tree với root node đã được chia từ hàm Split()
- predict(Node, row) dùng một root Node để dự đoán label của một row
- Subsample(dataset, n_sample) tạo một dataset mới với n_sample các rows được chọn ngẫu nhiên(có thể lặp) từ dataset ban đầu
- Bagging_predict(Trees, row) dùng nhiều decision Tree để dự đoán label của một row
- random_forest(dataset, max_depth, min_size, sample_size, n_trees, n_features) trả về một rừng có (n_trees) các decision Tree có kích thước (sample_size)
- printTree(Node) in ra một decisionTree 
- accuracy_metric: tính % đoán đúng
- evaluate_forest: đánh giá độ chính xác của cây bằng cross validation trong trường hợp không có file test
- exportResult(testSet, vector<> predict) xuất kết quả dự đoán lưu trong vector<> predict của testSet ra file result.txt

## 4.Lịch sử thực hiện:

- Dựng một Decision Tree (Accuracy ~81%)
- Cài đặt Cross Validation
- Cải tiến lên Random Forest (Max Accuracy ~86%)
- Dự đoán cho file không có nhãn
- Thêm export to file
