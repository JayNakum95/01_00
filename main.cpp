#include <Novice.h>
#include <corecrt_math.h>
#include <stdio.h>
#include <cstdint>
#include <cmath>
#include <type_traits>
#include <imgui.h>

const char kWindowTitle[] = "GC2B_07_ナクム_ジェイ_ハルシュバルダン";
const int kWindowWidth = 1280;
const int kWindowHeight = 720;



struct Vector3 {
	float x, y, z;
};
struct Matrix4x4 {
	float m[4][4];
};
struct Sphere {
	Vector3 centre;
	float radius;
};
struct Plane {
	Vector3 normal;
	float distance;
};
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearZ, float farZ) {
	Matrix4x4 mat = {};
	float f = 1.0f / tanf(fovY * 0.5f);
	mat.m[0][0] = f / aspectRatio;
	mat.m[1][1] = f;
	mat.m[2][2] = farZ / (farZ - nearZ);
	mat.m[2][3] = 1.0f;
	mat.m[3][2] = (-nearZ * farZ) / (farZ - nearZ);
	mat.m[3][3] = 0.0f;
	return mat;
}

Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearZ, float farZ) {
	Matrix4x4 mat = {};
	mat.m[0][0] = 2.0f / (right - left);
	mat.m[1][1] = 2.0f / (top - bottom);
	mat.m[2][2] = 1.0f / (farZ - nearZ);
	mat.m[3][0] = -(right + left) / (right - left);
	mat.m[3][1] = -(top + bottom) / (top - bottom);
	mat.m[3][2] = -nearZ / (farZ - nearZ);
	mat.m[3][3] = 1.0f;
	return mat;
}
Matrix4x4 MakeViewPortMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 mat = {};
	mat.m[0][0] = width * 0.5f;
	mat.m[1][1] = -height * 0.5f;
	mat.m[2][2] = (maxDepth - minDepth);
	mat.m[3][0] = left + (width * 0.5f);
	mat.m[3][1] = top + (height * 0.5f);
	mat.m[3][2] = minDepth;
	mat.m[3][3] = 1.0f;
	return mat;
}

static const int kRowHeight = 20;
static const int kColumnWidth = 60;

// 行列の加算
Matrix4x4 Add(Matrix4x4& m1, Matrix4x4& m2) {
	Matrix4x4 result{};
	for (int row = 0; row < 4; ++row)
		for (int col = 0; col < 4; ++col)
			result.m[row][col] = m1.m[row][col] + m2.m[row][col];
	return result;
}

// 行列の減算
Matrix4x4 Subtract(Matrix4x4& m1, Matrix4x4& m2) {
	Matrix4x4 result{};
	for (int row = 0; row < 4; ++row)
		for (int col = 0; col < 4; ++col)
			result.m[row][col] = m1.m[row][col] - m2.m[row][col];
	return result;
}

// 行列の乗算
Matrix4x4 Multiply(Matrix4x4& m1, Matrix4x4& m2) {
	Matrix4x4 result{};
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			result.m[row][col] = 0;
			for (int k = 0; k < 4; ++k) {
				result.m[row][col] += m1.m[row][k] * m2.m[k][col];
			}
		}
	}
	return result;
}



Matrix4x4 Transpose(const Matrix4x4& m) {
	Matrix4x4 result{};
	for (int row = 0; row < 4; ++row)
		for (int col = 0; col < 4; ++col)
			result.m[row][col] = m.m[col][row];
	return result;
}

// 単位行列
static Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 result{};
	for (int i = 0; i < 4; ++i)
		result.m[i][i] = 1.0f;
	return result;
}

Matrix4x4 Inverse(Matrix4x4& m) {
	Matrix4x4 result = MakeIdentity4x4();
	Matrix4x4 temp = m;

	for (int i = 0; i < 4; ++i) {
		if (fabsf(temp.m[i][i]) < 1e-6f) {
			bool swapped = false;
			for (int j = i + 1; j < 4; ++j) {
				if (fabsf(temp.m[j][i]) > 1e-6f) {
					for (int k = 0; k < 4; ++k) {
						std::swap(temp.m[i][k], temp.m[j][k]);
						std::swap(result.m[i][k], result.m[j][k]);
					}
					swapped = true;
					break;
				}
			}
			if (!swapped) {
				return MakeIdentity4x4();
			}
		}


		float diag = temp.m[i][i];
		for (int k = 0; k < 4; ++k) {
			temp.m[i][k] /= diag;
			result.m[i][k] /= diag;
		}


		for (int j = 0; j < 4; ++j) {
			if (i == j) continue;
			float factor = temp.m[j][i];
			for (int k = 0; k < 4; ++k) {
				temp.m[j][k] -= factor * temp.m[i][k];
				result.m[j][k] -= factor * result.m[i][k];
			}
		}
	}

	return result;
}

Matrix4x4 MakeRotationX(float radian) {
	Matrix4x4 result = MakeIdentity4x4();
	float cosradian = cosf(radian);
	float sinradian = sinf(radian);
	result.m[1][1] = cosradian;
	result.m[1][2] = sinradian;
	result.m[2][1] = -sinradian;
	result.m[2][2] = cosradian;
	return result;
}
Matrix4x4 MakeRotationY(float radian) {
	Matrix4x4 result = MakeIdentity4x4();
	float cosradian = cosf(radian);
	float sinradian = sinf(radian);
	result.m[0][0] = cosradian;
	result.m[0][2] = -sinradian;
	result.m[2][0] = sinradian;
	result.m[2][2] = cosradian;
	return result;
}
Matrix4x4 MakeRotationZ(float radian) {
	Matrix4x4 result = MakeIdentity4x4();
	float cosradian = cosf(radian);
	float sinradian = sinf(radian);
	result.m[0][0] = cosradian;
	result.m[0][1] = sinradian;
	result.m[1][0] = -sinradian;
	result.m[1][1] = cosradian;
	return result;
}
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 result = MakeIdentity4x4();
	Matrix4x4 rotationX = MakeRotationX(rotate.x);
	Matrix4x4 rotationY = MakeRotationY(rotate.y);
	Matrix4x4 rotationZ = MakeRotationZ(rotate.z);


	Matrix4x4 temporaryMatrix = Multiply(rotationY, rotationZ);
	Matrix4x4 rotation = Multiply(rotationX, temporaryMatrix);

	// Apply scaling after rotation
	result.m[0][0] = rotation.m[0][0] * scale.x;
	result.m[0][1] = rotation.m[0][1] * scale.x;
	result.m[0][2] = rotation.m[0][2] * scale.x;

	result.m[1][0] = rotation.m[1][0] * scale.y;
	result.m[1][1] = rotation.m[1][1] * scale.y;
	result.m[1][2] = rotation.m[1][2] * scale.y;

	result.m[2][0] = rotation.m[2][0] * scale.z;
	result.m[2][1] = rotation.m[2][1] * scale.z;
	result.m[2][2] = rotation.m[2][2] * scale.z;

	// Apply translation last
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;

	return result;
}


Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result{};
	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + matrix.m[3][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + matrix.m[3][1];
	result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + matrix.m[3][2];
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + matrix.m[3][3];

	if (w != 0.0f) {
		result.x /= w;
		result.y /= w;
		result.z /= w;
	}

	return result;
}
Vector3 Cross(const Vector3& v1, const Vector3& v2) {
	Vector3 result{};
	result.x = v1.y * v2.z - v1.z * v2.y;
	result.y = v1.z * v2.x - v1.x * v2.z;
	result.z = v1.x * v2.y - v1.y * v2.x;
	return result;
}
void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	const float kGridHalfWidth = 2.0f;
	const uint32_t kSubdivision = 10;
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision);

	for (uint32_t xIndex = 0; xIndex <= kSubdivision; ++xIndex) {
		float x = -kGridHalfWidth + (xIndex * kGridEvery);
		Vector3 start{ x, 0.0f, -kGridHalfWidth };
		Vector3 end{ x, 0.0f, kGridHalfWidth };

		Vector3 startScreen = Transform(Transform(start, viewProjectionMatrix), viewportMatrix);
		Vector3 endScreen = Transform(Transform(end, viewProjectionMatrix), viewportMatrix);
		Novice::DrawLine(
			int(startScreen.x), int(startScreen.y), int(endScreen.x), int(endScreen.y),
			x == 0.0f ? BLACK : 0xAAAAAAFF);
	}

	for (uint32_t zIndex = 0; zIndex <= kSubdivision; ++zIndex) {
		float z = -kGridHalfWidth + (zIndex * kGridEvery);
		Vector3 start{ -kGridHalfWidth, 0.0f, z };
		Vector3 end{ kGridHalfWidth, 0.0f, z };

		Vector3 startScreen = Transform(Transform(start, viewProjectionMatrix), viewportMatrix);
		Vector3 endScreen = Transform(Transform(end, viewProjectionMatrix), viewportMatrix);
		Novice::DrawLine(
			int(startScreen.x), int(startScreen.y), int(endScreen.x), int(endScreen.y),
			z == 0.0f ? BLACK : 0xAAAAAAFF);
	}
}
float Dot(const Vector3& v1, const Vector3& v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

void DrawSphere(

	const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix,
	uint32_t color) {
	const float pi = 3.14159265358979323846f;
	const uint32_t kSubdivision = 12;
	// 経度分割1つ分の角度
	const float kLonEvery = pi * 2.0f / float(kSubdivision);
	// 緯度分割1つ分の角度
	const float kLatEvery = pi / float(kSubdivision);

	// 緯度の方向に分割
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -pi / 2.0f + kLatEvery * latIndex;

		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery;

			Vector3 a = {
				sphere.centre.x + sphere.radius * std::cos(lat) * std::cos(lon),
				sphere.centre.y + sphere.radius * std::sin(lat),
				sphere.centre.z + sphere.radius * std::cos(lat) * std::sin(lon)
			};

			Vector3 b = {
				sphere.centre.x + sphere.radius * std::cos(lat + kLatEvery) * std::cos(lon),
				sphere.centre.y + sphere.radius * std::sin(lat + kLatEvery),
				sphere.centre.z + sphere.radius * std::cos(lat + kLatEvery) * std::sin(lon)
			};

			Vector3 c = {
				sphere.centre.x + sphere.radius * std::cos(lat) * std::cos(lon + kLonEvery),
				sphere.centre.y + sphere.radius * std::sin(lat),
				sphere.centre.z + sphere.radius * std::cos(lat) * std::sin(lon + kLonEvery)
			};

			// 線を描く
			Vector3 screenA = Transform(Transform(a, viewProjectionMatrix), viewportMatrix);
			Vector3 screenB = Transform(Transform(b, viewProjectionMatrix), viewportMatrix);
			Vector3 screenC = Transform(Transform(c, viewProjectionMatrix), viewportMatrix);
			Novice::DrawLine(int(screenA.x), int(screenA.y), int(screenB.x), int(screenB.y), color);
			Novice::DrawLine(int(screenA.x), int(screenA.y), int(screenC.x), int(screenC.y), color);
		}
	}
}
float Length(const Vector3& v1, const Vector3& v2) {
	float dx = v1.x - v2.x;
	float dy = v1.y - v2.y;
	float dz = v1.z - v2.z;
	return sqrtf(dx * dx + dy * dy + dz * dz);
}
Vector3 Perpendicular(const Vector3& v1) {
	if (v1.x != 0.0f || v1.y != 0.0f) {
		return { -v1.y, v1.x, 0.0f }; // XY平面上での垂直ベクトル
	}
	return { 0.0f, -v1.z,v1.y }; // Z軸方向のベクトル
}


// Helper: Scalar * Vector3
Vector3 Multiply(float scalar, const Vector3& v) {
	return { scalar * v.x, scalar * v.y, scalar * v.z };
}

// Helper: Vector3 + Vector3
Vector3 Add(const Vector3& v1, const Vector3& v2) {
	return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

// Helper: Normalize Vector3
Vector3 Normalize(const Vector3& v) {
	float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	if (len > 1e-6f) {
		return { v.x / len, v.y / len, v.z / len };
	}
	return { 0.0f, 0.0f, 0.0f };
}

void DrawPlane(const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 center = Multiply(plane.distance, plane.normal);
	Vector3 perpendiculars[4];
	perpendiculars[0] = Normalize(Perpendicular(plane.normal));
	perpendiculars[1] = { -perpendiculars[0].x, -perpendiculars[0].y, -perpendiculars[0].z };
	perpendiculars[2] = Cross(plane.normal, perpendiculars[0]);
	perpendiculars[3] = { -perpendiculars[2].x, -perpendiculars[2].y, -perpendiculars[2].z };

	Vector3 points[4];
	const float kPlaneSize = 2.0f;
	for (int32_t index = 0; index < 4; ++index) {
		Vector3 extend = Multiply(kPlaneSize, perpendiculars[index]);
		Vector3 point = Add(center, extend);
		points[index] = Transform(Transform(point, viewProjectionMatrix), viewportMatrix);
	}

    // Draw rectangle (plane) by connecting the 4 points
	Novice::DrawLine(
		static_cast<int>(points[0].x), static_cast<int>(points[0].y),
		static_cast<int>(points[2].x), static_cast<int>(points[2].y), color);
	Novice::DrawLine(
		static_cast<int>(points[1].x), static_cast<int>(points[1].y),
		static_cast<int>(points[2].x), static_cast<int>(points[2].y), color);
	Novice::DrawLine(
		static_cast<int>(points[1].x), static_cast<int>(points[1].y),
		static_cast<int>(points[3].x), static_cast<int>(points[3].y), color);
	Novice::DrawLine(
		static_cast<int>(points[3].x), static_cast<int>(points[3].y),
		static_cast<int>(points[0].x), static_cast<int>(points[0].y), color);

    
    
}


bool IsSpherePlaneColliding(const Sphere& sphere, const Plane& plane) {
	float len = sqrtf(plane.normal.x * plane.normal.x +
		plane.normal.y * plane.normal.y +
		plane.normal.z * plane.normal.z);

	if (len < 1e-6f) return false;  // Degenerate normal

	Vector3 normalizedNormal = {
		plane.normal.x / len,
		plane.normal.y / len,
		plane.normal.z / len
	};

	float signedDistance = Dot(normalizedNormal, sphere.centre) - plane.distance;
	return fabsf(signedDistance) <= sphere.radius;
}


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	Novice::Initialize(kWindowTitle, kWindowWidth, kWindowHeight);
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };
	Vector3 cameraTranslate{ 0.0f, 0.0f, 6.0f };
	Vector3 cameraRotate{ 6.0f, 0.0f, 0.0f };
	bool isDragging = false;
	bool isRightDragging = false;
	int lastMouseX = 0, lastMouseY = 0;
	int wheel = 0; // マウスホイールの値	


	// ビュー行列を作成

	Vector3 sphere1Center = { -1.0f, 0.0f, 0.0f };
	float sphere1Radius = 1.0f;
	Sphere sphere1{ sphere1Center, sphere1Radius };

	Vector3 planeNormal = { 0.0f, 1.0f, 0.0f };
	float planeDistance = 0.0f; // 平面の原点からの距離
	Plane plane{ planeNormal, planeDistance };


	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();

		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		Matrix4x4 cameraScale{ { {1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1} } };
		Matrix4x4 cameraRotX = MakeRotationX(cameraRotate.x);
		Matrix4x4 cameraRotY = MakeRotationY(cameraRotate.y);
		Matrix4x4 cameraRotZ = MakeRotationZ(cameraRotate.z);
		Matrix4x4 tempMatrix = Multiply(cameraRotZ, cameraRotX);
		Matrix4x4 cameraRot = Multiply(tempMatrix, cameraRotY);
		Matrix4x4 cameraTrans = MakeIdentity4x4();
		cameraTrans.m[3][0] = cameraTranslate.x;
		cameraTrans.m[3][1] = cameraTranslate.y;
		cameraTrans.m[3][2] = cameraTranslate.z;
		Matrix4x4 viewMatrix = Multiply(cameraRot, cameraTrans);

		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);
		Matrix4x4 viewportMatrix = MakeViewPortMatrix(0.0f, 0.0f, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);



		int mouseX, mouseY;
		Novice::GetMousePosition(&mouseX, &mouseY);

		int mouseL = Novice::IsPressMouse(2); // mouse button 2(middle mouse button)

		if (mouseL && !isDragging) {
			isDragging = true;
			lastMouseX = mouseX;
			lastMouseY = mouseY;
		}
		else if (!mouseL && isDragging) {
			isDragging = false;
		}

		if (isDragging) {
			int dx = mouseX - lastMouseX;
			int dy = mouseY - lastMouseY;
			cameraRotate.y += dx * 0.01f;
			cameraRotate.x += dy * 0.01f;
			lastMouseX = mouseX;
			lastMouseY = mouseY;
		}
		int mouseR = Novice::IsPressMouse(1); // 1: right button
		if (mouseR && !isRightDragging) {
			isRightDragging = true;
			lastMouseX = mouseX;
			lastMouseY = mouseY;
		}
		else if (!mouseR && isRightDragging) {
			isRightDragging = false;
		}
		if (isRightDragging) {
			int dx = mouseX - lastMouseX;
			int dy = mouseY - lastMouseY;
			cameraTranslate.x -= dx * 0.01f;
			cameraTranslate.y += dy * 0.01f; // Y軸は逆方向に動かす
			lastMouseX = mouseX;
			lastMouseY = mouseY;
		}
		wheel += Novice::GetWheel();
		if (wheel != 0) {
			cameraTranslate.z += wheel * 0.1f; 
			wheel = 0; 
		}



		ImGui::Begin("Window");
		ImGui::DragFloat3("sphereCenter", &sphere1.centre.x, 0.01f);
		ImGui::DragFloat("sphereRadius", &sphere1.radius, 0.01f);
		ImGui::DragFloat3("planeNormal", &plane.normal.x, 0.01f);
		plane.normal = Normalize(plane.normal);

		ImGui::DragFloat("planeDistance", &plane.distance, 0.01f);
		ImGui::End();




		DrawGrid(viewProjectionMatrix, viewportMatrix);

		DrawPlane(plane, viewProjectionMatrix, viewportMatrix, 0x00FF00FF);
		if (IsSpherePlaneColliding(sphere1, plane)) {
			DrawSphere(sphere1, viewProjectionMatrix, viewportMatrix, 0xFF0000FF);
		}
		else {
			DrawSphere(sphere1, viewProjectionMatrix, viewportMatrix, 0x00FF00FF);
		}



		Novice::EndFrame();

		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	Novice::Finalize();
	return 0;
}
