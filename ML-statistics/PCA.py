import numpy as np

# data array, Num_Item x Num_Dim
data_m = np.array( [[-1, -2], [-1, 0], [0, 0], [2, 1], [0, 1]] )

item_num = data_m.shape[0]

# mean data_arr
data_m = data_m - np.mean(data_m, axis=0)

# covariance matrix
covar_m = np.matmul(data_m.T, data_m) / item_num

# eigen value & vector
values, vectors = np.linalg.eig(covar_m)
print(vectors)
print(values)

pca_base = vectors.T
print(np.matmul(np.matmul(pca_base, covar_m), pca_base.T))
