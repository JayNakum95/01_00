#include <Novice.h>
#include <corecrt_math.h>
#include <stdio.h>
#include <cstdint>
#include <type_traits>

const char kWindowTitle[] = "GC2B_07_ナクム_ジェイ_ハルシュバルダン";
const int kWindowWidth = 1280;
const int kWindowHeight = 720;



struct Vector3 {
	float x, y, z;
};
struct Matrix4x4 {
	float m[4][4];
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
				// 逆行列が存在しない
				return MakeIdentity4x4(); // 代替措置
			}
		}

		// 対角要素を1にする
		float diag = temp.m[i][i];
		for (int k = 0; k < 4; ++k) {
			temp.m[i][k] /= diag;
			result.m[i][k] /= diag;
		}

		// 他の行のi列を0にする
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
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	Novice::Initialize(kWindowTitle, kWindowWidth, kWindowHeight);

	char keys[256] = { 0 };
	char preKeys[256] = { 0 };
	// Object world transform parameters
	Vector3 translate{ 0.0f, 0.0f, 3.0f }; // Place object 3 units in front of camera
	Vector3 rotate{ 0.0f, 0.0f, 0.0f };

	// Cross product test vectors
	Vector3 v1{ 1.2f, -3.9f, 2.5f };
	Vector3 v2{ 2.8f, 0.4f, -1.3f };

	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();

		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		if (keys[DIK_A]) { translate.x -= 0.08f; }
		if (keys[DIK_D]) { translate.x += 0.08f; }
		if (keys[DIK_W]) { translate.z += 0.08f; }
		if (keys[DIK_S]) { translate.z -= 0.08f; }
		rotate.y += 0.05f;
		// Triangle vertices in local space
		Vector3 kLocalVertices[3] = {
			{ 0.0f,  0.5f, 0.0f },  // Top
			{ 0.5f, -0.5f, 0.0f },  // Right
			{-0.5f, -0.5f, 0.0f }   // Left
		};

		// Calculate matrices
		Matrix4x4 worldMatrix = MakeAffineMatrix(
			{ 1.0f, 1.0f, 1.0f },  // Scale
			rotate,              // Rotation
			translate            // Translation
		);

		Vector3 cameraPosition{ 0.0f, 0.0f, -5.0f }; // Pull camera back to see object
		Matrix4x4 cameraMatrix = MakeAffineMatrix(
			{ 1.0f, 1.0f, 1.0f },  // Scale
			{ 0.0f, 0.0f, 0.0f },  // Rotation
			cameraPosition       // Camera position
		);

		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(
			0.45f,
			static_cast<float>(kWindowWidth) / static_cast<float>(kWindowHeight),
			0.1f,
			100.0f
		);
		Matrix4x4 tempMatrix = Multiply(viewMatrix, projectionMatrix);
		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, tempMatrix);
		Matrix4x4 viewportMatrix = MakeViewPortMatrix(
			0, 0,
			static_cast<float>(kWindowWidth),
			static_cast<float>(kWindowHeight),
			0.0f, 1.0f
		);
		// Transform vertices from local to screen space
		Vector3 screenVertices[3] = {};
		for (uint32_t i = 0; i < 3; ++i) {
			Vector3 ndcVertex = Transform(kLocalVertices[i], worldViewProjectionMatrix);
			screenVertices[i] = Transform(ndcVertex, viewportMatrix);
		}




		////描画処理
		// Draw triangle
		Novice::DrawTriangle(
			static_cast<int>(screenVertices[0].x), static_cast<int>(screenVertices[0].y),
			static_cast<int>(screenVertices[1].x), static_cast<int>(screenVertices[1].y),
			static_cast<int>(screenVertices[2].x), static_cast<int>(screenVertices[2].y),
			RED, kFillModeSolid
		);
		// Cross product output
		Vector3 cross = Cross(v1, v2);
		Novice::ScreenPrintf(10, 10, "cross: (%.2f, %.2f, %.2f)", cross.x, cross.y, cross.z);
		///描画処理完了
		Novice::EndFrame();

		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	Novice::Finalize();
	return 0;
}
