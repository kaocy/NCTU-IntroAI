import sys
import random
import math

class Node:
    def __init__(self):
        self.attribute_index = None
        self.threshold = None
        self.label = None
        self.parent = None
        self.left = None
        self.right = None


def gini(data):
    if len(data) == 0:
        return 0

    tmp = sorted(data, key=lambda d: d[-1])
    pre_index = 0
    g = 1.0

    for i in range(len(tmp) - 1):
        if tmp[i][-1] != tmp[i + 1][-1]:
            g -= ((i - pre_index + 1) / len(tmp)) ** 2
            pre_index = i + 1

    g -= ((len(tmp) - pre_index) / len(tmp)) ** 2
    return g


def build_decision_tree(node, data, attribute_bagging, extremely_random_forest=False):
    # check if all data has the same label
    same_label_flag = True
    for d in data[1:]:
        if data[0][-1] != d[-1]:
            same_label_flag = False
            break
    
    if same_label_flag:
        node.label = data[0][-1]
        return
    
    best_attribute_index = None
    best_threshold = None
    best_impurity = float("inf")

    attributes = [i for i in range(len(data[0]) - 1)]
    if attribute_bagging:
        num_attribute = round(math.sqrt(len(data[0]) - 1))
        attributes = random.sample(attributes, num_attribute)
    
    if extremely_random_forest:
        attributes = random.sample(attributes, 1)

    # select attribute
    for i in attributes:
        # print(f'attritube {i}')
        data.sort(key=lambda d: d[i])
        same_attribute_flag = True

        # calculate the best threshold for the minimum impurity
        for j in range(len(data) - 1):
            if data[j][i] != data[j + 1][i]:
                
                same_attribute_flag = False
                arr1 = data[:j+1]
                arr2 = data[j+1:]
                gini1 = gini(arr1)
                gini2 = gini(arr2)
                impurity = len(arr1) * gini1 + len(arr2) * gini2

                # print(f'index {j} : {impurity}')
                
                if impurity < best_impurity:
                    best_attribute_index = i
                    best_threshold = (data[j][i] + data[j + 1][i]) / 2
                    best_impurity = impurity
        
        if same_attribute_flag:
            gini1 = gini(data)
            impurity = len(data) * gini1
                
            if impurity < best_impurity:
                best_attribute_index = i
                best_threshold = data[-1][i]
                best_impurity = impurity
    
    node.attribute_index = best_attribute_index
    node.threshold = best_threshold
    sub_data1 = [d for d in data if d[best_attribute_index] <= best_threshold]
    sub_data2 = [d for d in data if d[best_attribute_index] > best_threshold]
    
    # build the subtree
    if len(sub_data1) > 0:
        left_child = Node()
        left_child.parent = node
        node.left = left_child
        build_decision_tree(left_child, sub_data1, attribute_bagging)

    if len(sub_data2) > 0:
        right_child = Node()
        right_child.parent = node
        node.right = right_child
        build_decision_tree(right_child, sub_data2, attribute_bagging)


def classify(node, data):
    # if len(data) == 0:
    #     return 0

    # # leaf node
    # if node.label != None:
    #     correct = 0
    #     for d in data:
    #         correct += int(node.label == d[-1])
    #     return correct

    # sub_data1 = [d for d in data if d[node.attribute_index] <= node.threshold]
    # sub_data2 = [d for d in data if d[node.attribute_index] > node.threshold]
    # return classify(node.left, sub_data1) + classify(node.right, sub_data2)

    if node.label != None:
        return node.label
    if data[node.attribute_index] <= node.threshold:
        return classify(node.left, data)
    else:
        return classify(node.right, data)


def random_forest(data, tree_bagging, attribute_bagging):
    total_accuracy = 0.0

    for _ in range(10):
        training_data_pool = data[:-30]
        validation_data = data[-30:]

        num_tree = 5
        roots = []
        for i in range(num_tree):
            training_data = random.sample(training_data_pool, 60) if tree_bagging else data[:-30]
            root = Node()
            build_decision_tree(root, training_data, attribute_bagging)
            roots.append(root)
        
        correct = 0
        for d in validation_data:
            # pred = {
            #     'Iris-setosa': 0,
            #     'Iris-versicolor': 0,
            #     'Iris-virginica': 0
            # }
            pred = {
                '1': 0,
                '2': 0,
                '3': 0
            }
            for i in range(num_tree):
                pred[classify(roots[0], d)] += 1
            
            pred_label = ''
            best_value = -1
            for k, v in pred.items():
                # print(k, v)
                if v > best_value:
                    pred_label = k
                    best_value = v

            # print(pred_label, classify(roots[0], d))
            true_label = d[-1]
            correct += int(pred_label == true_label)
        
        # print(correct / len(validation_data))
        total_accuracy += correct / len(validation_data)
    print(f'Total Accuracy: {total_accuracy / 10}')
    

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('Usage: python main.py [filename]')
        sys.exit()
    
    data = []
    filename = sys.argv[1]
    with open(filename, 'r') as fp:
        lines = fp.readlines()
        for line in lines:
            arr = line[:-1].split(',')
            arr.append(arr[0])
            arr = arr[1:]
            for i in range(len(arr) - 1):
                arr[i] = float(arr[i])
            data.append(tuple(arr))

    # print(data)
    random.shuffle(data)
    
    # random_forest(data, True, True)
    random_forest(data, True, False)