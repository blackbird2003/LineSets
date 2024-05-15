import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import *

def bezier_curve(t, P0, P1, P2, P3):
    return (1-t)**3 * P0 + 3*(1-t)**2 * t * P1 + 3*(1-t) * t**2 * P2 + t**3 * P3

def bezier_derivative(t, P0, P1, P2, P3):
    return -3*(1-t)**2 * P0 + (3*(1-t)**2 - 6*(1-t)*t) * P1 + (6*(1-t)*t - 3*t**2) * P2 + 3*t**2 * P3

def bezier_second_derivative(t, P0, P1, P2, P3):
    return 6*(1-t) * P0 + (-12*(1-t) + 6*t) * P1 + (6*(1-2*t)) * P2 + 6*t * P3

def curvature(t, P0, P1, P2, P3):
    # print(P0, P1, P2, P3)
    B_prime = bezier_derivative(t, P0, P1, P2, P3)
    B_double_prime = bezier_second_derivative(t, P0, P1, P2, P3)
    numerator = np.linalg.norm(np.cross(B_prime, B_double_prime))
    denominator = np.linalg.norm(B_prime)**3
    return numerator / denominator

def find_max_curvature(P0, P1, P2, P3):
    P0, P1, P2, P3 = map(np.array, [P0, P1, P2, P3])
    result = minimize_scalar(lambda t: - curvature(t, P0, P1, P2, P3), bounds=(0, 1), method='bounded')
    min_k = -result.fun
    return min_k, result.x
