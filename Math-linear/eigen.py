import numpy as np

from qr import QR

def Eigen(matrix):
    N = matrix.shape[0]
    values = np.zeros(N, dtype=np.float32)
    vectors = np.eye(N, dtype=np.float32)
    m = np.ascontiguousarray(matrix, dtype=np.float32)
    for i in range(30):
        q, r = QR(m)
        np.matmul(vectors, q, out=vectors)
        np.matmul(r, q, out=m)
    for i in range(N):
        values[i] = m[i, i]
    return values, vectors
    
    
if __name__ == "__main__":
    A = np.array( 
        [[ 1, 1, 0], 
         [1, 0, 1],
         [0, 1, 1]])
    values, vectors = Eigen(A)

    values_np, vectors_np = np.linalg.eig(A)
    print(values)
    print(vectors)
    print(values_np)
    print(vectors_np)
    print(np.matmul(A, vectors))
    print(np.matmul(A, vectors_np))
