# shader

import sys
import os
import numpy as np
import scipy.io as sio
from skimage.io import imread, imsave

np.set_printoptions(precision=4, suppress=True)
WD = os.path.join('cs-ComputerGraphics')
sys.path.append(WD)
from render import render_colors

np_dtype = np.float32

# setting
origin_img = imread(os.path.join(WD, '0013.jpg')) / 255.
img_h, img_w = origin_img.shape[:2]

view_point = np.array([img_w / 2, img_h / 2, 500])
view_x = np.array([1, 0, 0])
view_y = np.array([0, 1, 0])
view_z = np.array([0, 0, 1])
near = 0.1
clip_window_lb = np.array([- img_w/2, - img_h/2])
clip_window_rt = np.array([img_w/2, img_h/2])
nearplane_lb = np.array([-img_w/2, -img_h/2])
nearplane_rt = np.array([img_w/2, img_h/2])
world2view_t = np.eye(4, dtype=np_dtype)
world2view_t[3, :3] = - view_point
world2view_p = np.eye(4, dtype=np_dtype)
world2view_p[:3, :3] = np.stack([view_x, view_y, view_z], axis=1).T
world2view = np.matmul(world2view_t, world2view_p)
print(world2view)
r, l, t, b = nearplane_rt[0], nearplane_lb[0], nearplane_rt[1], nearplane_lb[1]
# parallel projection
view2NDC = np.array([[2/(r-l), 0, 0, 0],
                        [0, 2/(t-b), 0, 0],
                        [0, 0, -1, 0],
                        [-(r+l)/(r-l), -(t+b)/(t-b), 0, 1]], dtype=np_dtype
)
NDC2raster = np.array([[(r - l)/2, 0, 0, 0],
                       [0, (t - b)/2, 0, 0],
                       [0, 0, 1, 0],
                       [(r - l)/2, (t - b)/2, 0, 1]], dtype=np_dtype
)

# global light
global_light = np.array([1, 1, 1], dtype=np_dtype)

# point light
# point_light_xyz = np.array([1000, 500, 1000], dtype=np_dtype)
point_light_xyz = np.array([img_w / 2, img_h / 2, 500])
point_light_rgb = np.array([0.0, 0.9, 0.5], dtype=np_dtype)

# 3D object primitives
vertex = sio.loadmat(os.path.join(WD, '0013_0.mat'))['vertex'].T # nVer x 3
vertex = np.hstack([vertex, np.ones((vertex.shape[0], 1), dtype=np_dtype)])
print(vertex[:3, :])
vertex = np.matmul(vertex, world2view) # world -> camera
vertex = np.matmul(vertex, view2NDC)   # camera -> screen -> NDC
vertex = np.matmul(vertex, NDC2raster) # camera -> NDC
print(vertex[:3, :])

view_point = np.array([0, 0, 0])
point_light_xyz = np.concatenate([point_light_xyz, np.ones(1, dtype=np_dtype)])
point_light_xyz = np.matmul(np.matmul(np.matmul(point_light_xyz, world2view), view2NDC), NDC2raster)[:3]

vertex = vertex[:, :3]
tri = sio.loadmat(os.path.join(WD, 'tri_refine.mat'))['tri']
# assume the origin color is [0, 0, 0]
color = np.zeros((vertex.shape[0], 3), dtype=np_dtype)

normal = np.zeros((vertex.shape[0], 3), dtype=np_dtype)
surface_count = np.zeros((vertex.shape[0], 1))
for i in range(tri.shape[0]):
    i1, i2, i3 = tri[i, :]
    v1, v2, v3 = vertex[[i1, i2, i3], :]
    surface_normal = np.cross(v2 - v1, v3 - v1)
    normal[[i1, i2, i3], :] += surface_normal
    surface_count[[i1, i2, i3], :] += 1

normal /= surface_count
normal /= np.linalg.norm(normal, axis=1, keepdims=True)

# print(np.sum(np.square(normal[:5, :]), axis=1))
diffuse_rate = np.ones(
    (vertex.shape[0], 3), dtype=np_dtype) * np.array([0.2, 0.2, 0.2])
specular_rate = np.ones(
    (vertex.shape[0], 3), dtype=np_dtype) * np.array([0.6, 0.6, 0.6])

# ambient component
color += diffuse_rate * global_light

# diffuse component
v2s = point_light_xyz - vertex  # vertex to light source
v2s /= np.linalg.norm(v2s, axis=1, keepdims=True)  # Nver x 3
normal_dot_light = np.sum(normal * v2s, axis=1, keepdims=True)
# print(np.sum(np.clip(normal_dot_light,  0, 1)))
color += diffuse_rate * point_light_rgb * np.clip(normal_dot_light, 0, 1)

# specular component
v2v = view_point - vertex  # vertex to view point
v2v /= np.linalg.norm(v2v, axis=1, keepdims=True)  # Nver x 3
reflection = 2 * normal_dot_light * normal - v2s
view_dot_reflection = np.sum(v2v * reflection, axis=1, keepdims=True)
# print(np.sum(np.clip(view_dot_reflection, 0, 1)))
W = np.where(normal_dot_light != 0, np.clip(view_dot_reflection, 0, 1),
             np.zeros_like(view_dot_reflection))
# print(np.sum(W))
color += specular_rate * point_light_rgb * W
color = np.clip(color, 0, 1)


# render & save
render_img = render_colors(vertex, tri, color, img_h, img_w, BG=origin_img)
render_img = np.clip(render_img, 0, 1)
imsave(os.path.join(WD, 'render.jpg'), render_img)
