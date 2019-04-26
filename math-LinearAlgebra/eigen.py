import numpy as np

from qr import QR


def Eigen(matrix):
    N = matrix.shape[0]
    np_dtype = np.float64
    values = np.zeros(N, dtype=np_dtype)
    vectors = np.eye(N, dtype=np_dtype)
    m = np.ascontiguousarray(matrix, dtype=np_dtype)
    for i in range(30):
        q, r = QR(m)
        np.matmul(vectors, q, out=vectors)
        np.matmul(r, q, out=m)
    for i in range(N):
        values[i] = m[i, i]
    return values, vectors


if __name__ == "__main__":
    A = np.array(
        [[1, 1, 1],
         [1, 2, 1],
         [1, 1, 2]])
    values, vectors = Eigen(A)

    values_np, vectors_np = np.linalg.eig(A)
    print('-----------test-result--------------')
    print('eigen value:\n', values)
    print('eigen vector:\n', vectors)
    print(np.matmul(A, vectors))
    print('----------------end-----------------')

    print('--------------numpy----------------')
    print('eigen value:\n', values_np)
    print('eigen vector:\n', vectors_np)
    print(np.matmul(A, vectors_np))
    print('----------------end-----------------')
