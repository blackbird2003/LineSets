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

def optimize_control_points(mode, optPara, LP0, LP1, LP2, P0, P1, P2, P3):
    print("python begin,", P0, P1, P2, P3)
    LP0 = np.array(LP0)
    LP1 = np.array(LP1)
    LP2 = np.array(LP2)
    P0 = np.array(P0)
    P1 = np.array(P1)
    P2 = np.array(P2)
    P3 = np.array(P3)

    P1_dir = P1 - P0
    def objective(params):
        P1x, P1y, P2x, P2y = params
        P1 = np.array([P1x, P1y])
        P2 = np.array([P2x, P2y])
        LP2 = P0 * 2 - P1;
        length = curve_length(LP0, LP1, LP2, P0) + curve_length(P0, P1, P2, P3)
        k1, _ = find_max_curvature(LP0, LP1, LP2, P0)
        k2, _ = find_max_curvature(LP1, LP2, P0, P1)
        k3, _ = find_max_curvature(LP2, P0, P1, P2)
        k4, _ = find_max_curvature(P0, P1, P2, P3)
        max_k = max(k1, k2, k3, k4)
        return length + optPara*max_k


    initial_params = [P1[0], P1[1], P2[0], P2[1]]

    if mode == 0:
        cost = objective(initial_params)
        return P1[0], P1[1], P2[0], P2[1], cost

    else:
        result = minimize(objective, initial_params, bounds=[(P1[0]-200, P1[0]+200), (P1[1]-200, P1[1]+200), (P2[0]-200, P2[0]+200), (P2[1]-200, P2[1]+200)])
        P1x_opt, P1y_opt, P2x_opt, P2y_opt = result.x
        P1_opt = np.array([P1x_opt, P1y_opt])
        P2_opt = np.array([P2x_opt, P2y_opt])
        print("python finished", P1_opt, P2_opt)
        print(result)
        return P1_opt[0], P1_opt[1], P2_opt[0], P2_opt[1], result.fun


# optimize_control_points([1, 1],[2, 2],[3, 3],[4, 4],[5, 5],[6, 6],[7, 7]);
