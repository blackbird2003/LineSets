import numpy as np
from scipy.optimize import minimize, minimize_scalar
from scipy.integrate import quad
import sys
# sys.stdout = open('output_log.txt', 'w')

def bezier_curve(t, P0, P1, P2, P3):
    return (1-t)**3 * P0 + 3*(1-t)**2 * t * P1 + 3*(1-t) * t * t * P2 + t**3 * P3

def bezier_derivative(t, P0, P1, P2, P3):
    return -3*(1-t)**2 * P0 + (3*(1-t)**2 - 6*(1-t)*t) * P1 + (6*(1-t)*t - 3*t**2) * P2 + 3*t**2 * P3

def bezier_second_derivative(t, P0, P1, P2, P3):
    return 6*(1-t) * P0 + (-12*(1-t) + 6*t) * P1 + (6*(1-2*t)) * P2 + 6*t * P3

def curvature(t, P0, P1, P2, P3):
    B_prime = bezier_derivative(t, P0, P1, P2, P3)
    B_double_prime = bezier_second_derivative(t, P0, P1, P2, P3)
    numerator = np.linalg.norm(np.cross(B_prime, B_double_prime))
    denominator = np.linalg.norm(B_prime)**3
    return numerator / denominator

def find_max_curvature(P0, P1, P2, P3):
    result = minimize_scalar(lambda t: -curvature(t, P0, P1, P2, P3), bounds=(0, 1), method='bounded')
    max_k = -result.fun
    return max_k, result.x

def curve_length(P0, P1, P2, P3):
    length, _ = quad(lambda t: np.linalg.norm(bezier_derivative(t, P0, P1, P2, P3)), 0, 1)
    return length


def optimize_control_points_global(p, cp):
    n = len(p)
    print(p)
    print(cp)
    #size of cp : 2 * n - 2
    #params: 4 * n - 4
    for i in range(len(p)):
        p[i] = np.array(p[i])
    for i in range(len(cp)):
        cp[i] = np.array(cp[i])

    def objective(params):
        for i in range(0, 2 * n - 2):
            cp[i] = np.array([params[2 * i], params[2 * i + 1]])

        length = 0
        for i in range(0, n - 1):
            length += curve_length(p[i], cp[2 * i], cp[2 * i + 1], p[i + 1])

        max_k = 0
        #size of p: n
        #number of parts: n - 1
        #size of cp : 2 * n - 2
        for i in range(0, n - 2):
            k1, _ = find_max_curvature(p[i], cp[2 * i], cp[2 * i + 1], p[i + 1])
            k2, _ = find_max_curvature(cp[2 * i], cp[2 * i + 1], p[i + 1], cp[2 * i + 2])
            k3, _ = find_max_curvature(cp[2 * i + 1], p[i + 1], cp[2 * i + 2], cp[2 * i + 3])
            max_k = max(max_k, k1, k2, k3)
        # deal with last part
        i = n - 2
        k1, _ = find_max_curvature(p[i], cp[2 * i], cp[2 * i + 1], p[i + 1])
        max_k = max(max_k, k1)

        return length + 1e4*max_k

    #initial_params = [P1[0], P1[1], P2[0], P2[1]]
    initial_params = [0] * (4 * n - 4)
    bound = [(0,0)] * (4 * n - 4)
    for i in range(0, 4 * n - 4):
        bound[i] = (initial_params[i] - 100, initial_params[i] + 100)
    result = minimize(objective, initial_params, bounds=bound)

    res = [(0, 0)] * (2 * n - 2)
    for i in range(0, 2 * n - 2):
        res[i] = [result.x[2 * i], result.x[2 * i + 1]]
    return res
