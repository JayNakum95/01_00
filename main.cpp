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
struct Triangle {
	Vector3 vertices[3]; // 頂点座標
	
};

struct Segment {
	Vector3 origin;///始点
	Vector3 diff;//後点へ着分ベトル
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


void DrawSegment(const Segment& segment, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 start = Transform(Transform(segment.origin, viewProjectionMatrix), viewportMatrix);
	Vector3 end = Transform(Transform(Add(segment.origin, segment.diff), viewProjectionMatrix), viewportMatrix);
	Novice::DrawLine(
		static_cast<int>(start.x), static_cast<int>(start.y),
		static_cast<int>(end.x), static_cast<int>(end.y), color);
}

 // Define Triangle and Plane types


 struct Plane {
     Vector3 normal;
     float distance;
 };

 // Define Line type and constants
 struct Line {
     Vector3 origin;
     Vector3 diff;
     static constexpr float kTMin = 0.0f;
     static constexpr float kTMax = 1.0f;
 };

 // Overload Subtract for Vector3
 Vector3 Subtract(const Vector3& v1, const Vector3& v2) {
     return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
 }

 
 bool IsCollision(const Triangle& triangle, const Line& line) {
     Vector3 v01 = Subtract(triangle.vertices[1], triangle.vertices[0]);
     Vector3 v12 = Subtract(triangle.vertices[2], triangle.vertices[1]);
     Vector3 normal = Normalize(Cross(v01, v12));
     Plane plane{ .normal = normal, .distance = Dot(triangle.vertices[0], normal) };
     float dot = Dot(plane.normal, line.diff);
     if (dot == 0.0f) {
         return false;
     }
     float t = (plane.distance - Dot(line.origin, plane.normal)) / dot;
     if ((t < Line::kTMin) || (Line::kTMax < t)) {
         return false;
     }
     Vector3 intersect = Add(line.origin, Multiply(t, line.diff));
     Vector3 v1p = Subtract(intersect, triangle.vertices[1]);
     if (Dot(Cross(v01, v1p), normal) < 0.0f) {
         return false;
     }
     Vector3 v2p = Subtract(intersect, triangle.vertices[2]);
     if (Dot(Cross(v12, v2p), normal) < 0.0f) {
         return false;
     }
     Vector3 v0p = Subtract(intersect, triangle.vertices[0]);
     Vector3 v20 = Subtract(triangle.vertices[0], triangle.vertices[2]);
     if (Dot(Cross(v20, v0p), normal) < 0.0f) {
         return false;
     }
     return true;
 }

 void DrawTriangle(const Triangle& triangle, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	 Vector3 transformedVertices[3];
	 for (int i = 0; i < 3; ++i) {
		 transformedVertices[i] = Transform(Transform(triangle.vertices[i], viewProjectionMatrix), viewportMatrix);
	 }
	 Novice::DrawLine(
		 static_cast<int>(transformedVertices[0].x), static_cast<int>(transformedVertices[0].y),
		 static_cast<int>(transformedVertices[1].x), static_cast<int>(transformedVertices[1].y), color);
	 Novice::DrawLine(
		 static_cast<int>(transformedVertices[1].x), static_cast<int>(transformedVertices[1].y),
		 static_cast<int>(transformedVertices[2].x), static_cast<int>(transformedVertices[2].y), color);
	 Novice::DrawLine(
		 static_cast<int>(transformedVertices[2].x), static_cast<int>(transformedVertices[2].y),
		 static_cast<int>(transformedVertices[0].x), static_cast<int>(transformedVertices[0].y), color);
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


	// ビュー行列を作成

	Triangle tri;
	tri.vertices[0] = { 0.0f, 1.0f, 0.0f };
	tri.vertices[1] = { -1.0f, -1.0f, 0.0f };
	tri.vertices[2] = { 1.0f, -1.0f, 0.0f };

	Vector3 SegmentOrigin = { 0.0f, 0.0f, 0.0f };
	Vector3 SegmentDiff = { 0.0f, 1.0f, 1.0f }; // Z軸方向のセグメント
	Segment segment{ SegmentOrigin, SegmentDiff };

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



		ImGui::Begin("Window");

		ImGui::DragFloat3("triangle Vertex 0", &tri.vertices[0].x, 0.01f);
		ImGui::DragFloat3("triangle Vertex 1", &tri.vertices[1].x, 0.01f);
		ImGui::DragFloat3("triangle Vertex 2", &tri.vertices[2].x, 0.01f);
		ImGui::DragFloat3("segmentOrigin", &segment.origin.x, 0.01f);
		ImGui::DragFloat3("segmentDiff", &segment.diff.x, 0.01f);
		
		ImGui::End();




		DrawGrid(viewProjectionMatrix, viewportMatrix);
		if (IsCollision(tri, { segment.origin, segment.diff })) {
			DrawTriangle(tri, viewProjectionMatrix, viewportMatrix,RED );

		}
		else {
			DrawTriangle(tri, viewProjectionMatrix, viewportMatrix, WHITE);
		
		}
		DrawSegment(segment, viewProjectionMatrix, viewportMatrix, 0xFF0000FF);
		
		
		


		Novice::EndFrame();

		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	Novice::Finalize();
	return 0;
}

