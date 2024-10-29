import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import cv2
import yaml


def plot_camera_pose(K, R, t, ax, scale=0.3):
    """
    카메라 pose를 3D 공간상에 시각화
    """
    # 카메라 중심점 계산
    C = -R.T @ t

    # 카메라 좌표계의 축
    axes = np.array([[scale, 0, 0], [0, scale, 0], [0, 0, scale]])
    axes_world = (R.T @ axes.T).T + C.flatten()

    # 카메라 피라미드를 그리기 위한 점들
    cam_points = (
        np.array(
            [
                [-0.1, -0.1, 0.2],
                [0.1, -0.1, 0.2],
                [0.1, 0.1, 0.2],
                [-0.1, 0.1, 0.2],
                [0, 0, 0],
            ]
        )
        * scale
    )

    # 점들을 월드 좌표계로 변환
    cam_points = (R.T @ cam_points.T).T + C.reshape(1, 3)

    # 피라미드 면 그리기
    faces = [[0, 1, 4], [1, 2, 4], [2, 3, 4], [3, 0, 4], [0, 1, 2, 3]]

    for i, face in enumerate(faces):
        points = cam_points[face]
        x = points[:, 0]
        y = points[:, 1]
        z = points[:, 2]
        if i == 4:
            color = "y"
        else:
            color = "g"
        ax.plot_trisurf(x, y, -z, color=color, alpha=0.7)

    # 카메라 좌표계의 축 그리기
    ax.quiver(
        C[0],
        C[1],
        -C[2],
        (axes_world[:, 0] - C[0]) * scale,
        (axes_world[:, 1] - C[1]) * scale,
        (C[2] - axes_world[:, 2]) * scale,
        color=["r", "g", "b"],
        arrow_length_ratio=0.1,
    )

    plot_checkerboard(ax)


def plot_checkerboard(ax, row=11, col=8, square_size=0.03):
    """
    체커보드의 중심이 (0,0)이 되도록 체커보드를 그리기
    """
    # 체커보드의 전체 크기 계산
    total_width = row * square_size
    total_height = col * square_size

    # 체커보드의 중심이 (0,0)이 되도록 오프셋 계산
    offset_x = 0
    offset_y = 0

    for i in range(row):
        for j in range(col):
            color = "black" if (i + j) % 2 == 0 else "white"
            x = [i * square_size + offset_x, (i + 1) * square_size + offset_x]
            y = [j * square_size + offset_y, (j + 1) * square_size + offset_y]
            X, Y = np.meshgrid(x, y)
            Z = np.zeros_like(X)
            ax.plot_surface(X, Y, Z, color=color)


if __name__ == "__main__":
    intrinsic_matrix = None
    rotation_vectors = []
    translation_vectors = []

    fs = cv2.FileStorage("camera_params.yml", cv2.FILE_STORAGE_READ)
    fn_r = fs.getNode("rotation_vectors")
    fn_t = fs.getNode("translation_vectors")
    fn_i = fs.getNode("camera_matrix")
    intrinsic_matrix = fn_i.mat()

    for i in range(fn_r.size()):
        rotation_vectors.append(fn_r.at(i).mat())
        translation_vectors.append(fn_t.at(i).mat())

    # 메인 시각화 코드
    fig = plt.figure(figsize=(12, 12))
    ax = fig.add_subplot(111, projection="3d")

    # rotation vector와 translation vector를 순회하면서 각 카메라 pose 시각화
    for rvec, tvec in zip(rotation_vectors, translation_vectors):
        # Rodrigues 변환으로 회전 행렬 얻기
        R, _ = cv2.Rodrigues(np.array(rvec).reshape(3, 1))
        t = np.array(tvec).reshape(3, 1)

        plot_camera_pose(intrinsic_matrix, R, t, ax)

    # 축 레이블 설정
    ax.set_xlabel("X")
    ax.set_ylabel("Y")
    ax.set_zlabel("Z")

    # 축 범위 설정
    ax.set_xlim([-0.5, 0.5])
    ax.set_ylim([-0.5, 0.5])
    ax.set_zlim([0, 1])

    # 보기 각도 설정
    ax.view_init(elev=30, azim=45)

    plt.show()
