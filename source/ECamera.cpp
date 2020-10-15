#include "pch.h"
#include "ECamera.h"
#include "EMesh.h"
#include "SDL.h"
#include <iostream>
#include <algorithm>
Elite::Camera::Camera(const FPoint3& origin, uint32_t windowWidth, uint32_t windowHeight, float fovD, float nearPlane, float farPlane)
	: m_Origin{ origin }
	, m_RenderSystem{ RenderSystem::Software }
	, m_Width{ windowWidth }
	, m_Height{ windowHeight }
	, m_AspectRatio{ (float)windowWidth / windowHeight }
	, m_FOV{ tanf((fovD * (float)E_TO_RADIANS) / 2.f) }
	, m_LookAt{}
	, m_Pitch{}
	, m_Yaw{}
	, m_MovementSensitivity{ 60.f }
	, m_RotationSensitivity{ 0.5f }
	, m_NearPlane{ nearPlane }
	, m_FarPlane{ farPlane }
	, m_ProjectionMatrix{}
{
	//Default render system is software, so matrix is right handed by default
	m_ProjectionMatrix = {
		{ 1.f / (m_AspectRatio * m_FOV), 0, 0, 0 },
		{ 0, 1.f / m_FOV, 0, 0 },
		{ 0 , 0, m_FarPlane / (m_NearPlane - m_FarPlane), -1 },
		{ 0, 0, (m_FarPlane * m_NearPlane) / (m_NearPlane - m_FarPlane), 0 }
	};
}

#pragma region Workers
/*General*/
void Elite::Camera::Update(float dT)
{
	UpdateLookAtMatrix(dT);
}

void Elite::Camera::UpdateLookAtMatrix(float dT)
{
	IVector2 dMove;

	uint32_t buttonMask = SDL_GetRelativeMouseState(&dMove.x, &dMove.y);
	const uint8_t* keyState = SDL_GetKeyboardState(nullptr);

	FVector3 movement{};
	FVector3 worldMovement{};

	//Vertical (Y) movement in world space
	if (buttonMask == (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK))
	{
		//m_Origin.y -= m_MovementSensitivity * dMove.y;
		worldMovement.y -= m_MovementSensitivity * dMove.y;
	}
	//Vertical (Y) movement in local space
	if (buttonMask == (SDL_BUTTON_LMASK | SDL_BUTTON_MMASK))
	{
		movement.y += m_MovementSensitivity * dMove.y;
	}
	//Horizontal rotation + Horizontal movement (Z) in local space
	if (buttonMask == SDL_BUTTON_LMASK)
	{
		m_Yaw -= dMove.x * float(E_TO_RADIANS) * m_RotationSensitivity;
		movement.z += m_MovementSensitivity * dMove.y;
	}
	//Horizontal movement (X) in local space + Horizontal movement (Z) in local space
	if (buttonMask == SDL_BUTTON_MMASK)
	{
		movement.x += m_MovementSensitivity * dMove.x;
		movement.z += m_MovementSensitivity * dMove.y;
	}
	//Free cam rotation
	if (buttonMask == SDL_BUTTON_RMASK)
	{
		m_Yaw -= dMove.x * float(E_TO_RADIANS) * m_RotationSensitivity;
		m_Pitch -= dMove.y * float(E_TO_RADIANS) * m_RotationSensitivity;
	}

	//Keyboard movement (Y) in world space
	if (keyState[SDL_SCANCODE_E])
	{
		//m_Origin.y += m_MovementSensitivity * dT;
		worldMovement.y += m_MovementSensitivity;
	}
	if (keyState[SDL_SCANCODE_Q])
	{
		//m_Origin.y -= m_MovementSensitivity * dT;
		worldMovement.y -= m_MovementSensitivity;
	}


	//Keyboard movement (X/Z) in local space
	if (keyState[SDL_SCANCODE_W])
	{
		movement.z -= m_MovementSensitivity;
	}
	if (keyState[SDL_SCANCODE_S])
	{
		movement.z += m_MovementSensitivity;
	}
	if (keyState[SDL_SCANCODE_A])
	{
		movement.x -= m_MovementSensitivity;
	}
	if (keyState[SDL_SCANCODE_D])
	{
		movement.x += m_MovementSensitivity;
	}

	//Apply localspace movement (m_LookAt needs to be transposed for D3D because...reasons??? - 
	//seemingly because vector transformation need the transposed inverse of the world matrix)
	//Some movement is still broken when y < 0

	switch (m_RenderSystem)
	{
	case RenderSystem::Software:
		m_Origin += (FVector3(m_LookAt * FVector4(movement)) + worldMovement) * dT;
		break;
	case RenderSystem::D3D:
		m_Origin += (FVector3(Transpose(m_LookAt) * FVector4(movement)) + worldMovement) * dT;
		break;
	default:
		break;
	}

	//Clamping pitch to prevent wonky behaviour when going over (or close to), 90 degrees.
	m_Pitch = Clamp(m_Pitch, (float)E_TO_RADIANS * -80.f, (float)E_TO_RADIANS * 80.f);
	//Fmodding yaw to allow for rollover and 360 degree movement.
	m_Yaw = fmod(m_Yaw, (float)E_PI_2);



	//FPS Camera approach adapted from https://www.3dgep.com/understanding-the-view-matrix/

	//Pure FPS camera
	/*float cosPitch = cos(m_Pitch);
	float sinPitch = sin(m_Pitch);
	float cosYaw = cos(m_Yaw);
	float sinYaw = sin(m_Yaw);

	FVector4 forward{ - sinYaw * cosPitch, sinPitch, cosPitch * cosYaw };
	FVector4 right{ cosYaw, 0, -sinYaw };
	FVector4 up{ sinYaw * sinPitch, cosPitch, cosYaw * sinPitch };
	FVector4 transXRot{ -Dot(right, FVector4(m_Origin)), -Dot(up, FVector4(m_Origin)), Dot(forward, FVector4(m_Origin)), 1 };
	m_LookAt = FMatrix4(right, up, forward, transXRot);*/


	//Using intermediate approach as flipping the x and y of the forward vector also has to affect the right and up vector
	//FPS Camera for Forward, rest is lookat construction
	float cosPitch = cos(m_Pitch);
	float sinPitch = sin(m_Pitch);
	float cosYaw = cos(m_Yaw);
	float sinYaw = sin(m_Yaw);

	FVector4 forward{};
	FVector4 worldUp{ 0, 1, 0, 0 };
	FVector4 right{};
	FVector4 up{};

	switch (m_RenderSystem)
	{
	case RenderSystem::Software:

		forward = { sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw };
		right = { GetNormalized(Cross(worldUp.xyz, forward.xyz)), 0.f };
		up = { GetNormalized(Cross(forward.xyz, right.xyz)), 0.f };

		m_LookAt = FMatrix4(right, up, forward, FVector4(m_Origin.x, m_Origin.y, m_Origin.z, 1.f));

		break;
	case RenderSystem::D3D:

		forward = { -sinYaw * cosPitch, sinPitch, cosPitch * cosYaw };
		right = { GetNormalized(Cross(worldUp.xyz, forward.xyz)), 0.f };
		up = { GetNormalized(Cross(forward.xyz, right.xyz)), 0.f };

		m_LookAt = FMatrix4(right, up, forward, FVector4(m_Origin.x, m_Origin.y, -m_Origin.z, 1.f));
		break;
	default:
		break;
	}

	//FPS Camera for everything, lookat for final step, not used anymore as the flipping of the x and y in the forward vector -
	//also need to affect right and up vector creation
	/*float cosPitch = cos(m_Pitch);
	float sinPitch = sin(m_Pitch);
	float cosYaw = cos(m_Yaw);
	float sinYaw = sin(m_Yaw);

	FVector4 forward{-sinYaw * cosPitch, sinPitch, cosPitch * cosYaw };
	FVector4 right{ cosYaw, 0, -sinYaw };
	FVector4 up{ sinYaw * sinPitch, cosPitch, cosYaw * sinPitch };
	m_LookAt = FMatrix4(right, up, forward, FVector4(m_Origin.x, m_Origin.y, -m_Origin.z, 1.f));*/

}

/*Software*/

void Elite::Camera::MakeScreenSpace(Mesh* pMesh)
{
	std::vector<VertexOutput> sSVertices;

	auto viewProjWorldMatrix = m_ProjectionMatrix * Inverse(m_LookAt) * pMesh->GetWorld();

	for (auto& v : pMesh->GetVertices())
	{
		VertexOutput sSV{};
		sSV.uv = v.uv;
		sSV.worldPos = v.pos;

		sSV.pos = viewProjWorldMatrix * FPoint4(v.pos);
		sSV.normal = FVector3(pMesh->GetWorld() * FVector4(v.normal));
		sSV.tangent = FVector3(pMesh->GetWorld() * FVector4(v.tangent));

		sSV.pos.x /= sSV.pos.w;
		sSV.pos.y /= sSV.pos.w;
		sSV.pos.z /= sSV.pos.w;

		sSV.viewDirection = GetNormalized((FMatrix3(pMesh->GetWorld()) * v.pos) - m_Origin);
		if (sSV.pos.x < -1 || sSV.pos.x > 1 || sSV.pos.y < -1 || sSV.pos.y > 1 || sSV.pos.z < 0 || sSV.pos.z > 1)
		{
			sSV.culled = true;
		}
		else
		{
			sSV.culled = false;
		}

		sSV.pos.x = (((sSV.pos.x + 1) / 2) * m_Width);
		sSV.pos.y = (((1 - sSV.pos.y) / 2) * m_Height);
		sSVertices.push_back(sSV);
	}
	pMesh->SetScreenSpaceVertices(sSVertices);
}
#pragma endregion

#pragma region Setters
void Elite::Camera::SetResolution(uint32_t width, uint32_t height)
{
	m_Width = width;
	m_Height = height;
	m_AspectRatio = (float)width / height;
}
void Elite::Camera::SetFOV(float fovD)
{
	m_FOV = tanf((fovD * (float)E_TO_RADIANS) / 2.f);
}

void Elite::Camera::ToggleRenderSystem(const RenderSystem& renderSystem)
{
	m_RenderSystem = renderSystem;
	switch (m_RenderSystem)
	{
	case RenderSystem::Software:
		m_ProjectionMatrix = {
			{ 1.f / (m_AspectRatio * m_FOV), 0, 0, 0 },
			{ 0, 1.f / m_FOV, 0, 0 },
			{ 0 , 0, m_FarPlane / (m_NearPlane - m_FarPlane), - 1 },
			{ 0, 0, (m_FarPlane * m_NearPlane) / (m_NearPlane - m_FarPlane), 0 }
		};
		break;
	case RenderSystem::D3D:
		m_ProjectionMatrix = {
				{ 1.f / (m_AspectRatio * m_FOV), 0, 0, 0 },
				{ 0, 1.f / m_FOV, 0, 0 },
				{ 0, 0, m_FarPlane / (m_FarPlane - m_NearPlane), 1} ,
				{ 0, 0, -(m_NearPlane * m_FarPlane) / (m_FarPlane - m_NearPlane), 0 }
		};
		break;
	default:
		break;
	}
}
#pragma endregion

#pragma region Getters
Elite::FMatrix4 Elite::Camera::GetInverseViewMatrix() const
{
	return m_LookAt;
}

Elite::FMatrix4 Elite::Camera::GetViewMatrix() const
{
	return Inverse(m_LookAt);
}

Elite::FMatrix4 Elite::Camera::GetProjectionMatrix() const
{
	return m_ProjectionMatrix;
}
#pragma endregion
