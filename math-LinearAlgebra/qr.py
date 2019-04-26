import numpy as np

def QR(matrix):
    H, W = matrix.shape

    np_dtype = np.float64

    rest = np.zeros((H, 1), dtype=np_dtype)
    Q = np.zeros_like(matrix, dtype=np_dtype)
    R = np.zeros_like(matrix, dtype=np_dtype)

    Q[:, 0] = matrix[:, 0] / np.sqrt(np.sum(np.square(matrix[:, 0])))
    R[0, 0] = np.matmul( matrix[:, 0].T, Q[:, 0] )

    for i in range(1, W):
        rest = np.zeros(H, dtype=np_dtype)
        for j in range(0, i):
            R[j, i] = np.matmul( matrix[:, i].T, Q[:, j] )
            rest += R[j, i] * Q[:, j]

        Q[:, i] = matrix[:, i] - rest
        Q[:, i] = Q[:, i] / np.sqrt(np.sum(np.square(Q[:, i])))
        R[i, i] = np.matmul( matrix[:, i].T, Q[:, i] )

    return Q, R

if __name__ == "__main__":
    A = np.array( 
        [[ 1, 1, 0], 
         [1, 0, 1],
         [0, 1, 1]])
    q, r = QR(A)
    q_np, r_np = np.linalg.qr(A)
    print('-----result-------')
    print('Q:\n', q)
    print('R:\n', r)
    print('QxR:\n', np.matmul(q, r))
    print('-------end--------')

    print('-----numpy-------')
    print('Q:\n', q_np)
    print('R:\n', r_np)
    print('QxR:\n', np.matmul(q_np, r_np))
    print('-------end--------')