#include "pch.h"
#include "ETimer.h"
#include "SDL.h"

Elite::Timer::Timer()
	: m_BaseTime{}
	, m_PausedTime{}
	, m_StopTime{}
	, m_PreviousTime{}
	, m_CurrentTime{}

	, m_FPS{}
	, m_FPSCount{}

	, m_TotalTime{}
	, m_ElapsedTime{}
	, m_SecondsPerCount{ 1.0f / static_cast<float>(SDL_GetPerformanceFrequency()) }
	, m_ElapsedUpperBound{ 0.03f }
	, m_FPSTimer{}

	, m_IsStopped{ true }
	, m_ForceElapsedUpperBound{ false }
{
}

void Elite::Timer::Reset()
{
	const auto currentTime= SDL_GetPerformanceCounter();

	m_BaseTime = currentTime;
	m_PreviousTime = currentTime;
	m_StopTime = 0;
	m_FPSTimer = 0.0f;
	m_FPSCount = 0;
	m_IsStopped = false;
}

void Elite::Timer::Start()
{
	const auto startTime = SDL_GetPerformanceCounter();

	if (m_IsStopped)
	{
		m_PausedTime += startTime - m_StopTime;

		m_PreviousTime = startTime;
		m_StopTime = 0;
		m_IsStopped = false;
	}
}

void Elite::Timer::Update()
{
	if (m_IsStopped)
	{
		m_FPS = 0;
		m_ElapsedTime = 0.0f;
		m_TotalTime = static_cast<float>(((m_StopTime - m_PausedTime) - m_BaseTime) * m_BaseTime);
		return;
	}

	const auto currentTime = SDL_GetPerformanceCounter();
	m_CurrentTime = currentTime;

	m_ElapsedTime = static_cast<float>((m_CurrentTime - m_PreviousTime) * m_SecondsPerCount);
	m_PreviousTime = m_CurrentTime;

	if (m_ElapsedTime < 0.0f)
		m_ElapsedTime = 0.0f;

	if (m_ForceElapsedUpperBound && m_ElapsedTime > m_ElapsedUpperBound)
	{
		m_ElapsedTime = m_ElapsedUpperBound;
	}

	m_TotalTime = static_cast<float>(((m_CurrentTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);

	//FPS LOGIC
	m_FPSTimer += m_ElapsedTime;
	++m_FPSCount;
	if (m_FPSTimer >= 1.0f)
	{
		m_FPS = m_FPSCount;
		m_FPSCount = 0;
		m_FPSTimer -= 1.0f;
	}
}

void Elite::Timer::Stop()
{
	if (!m_IsStopped)
	{
		const auto currentTime= SDL_GetPerformanceCounter();

		m_StopTime = currentTime;
		m_IsStopped = true;
	}
}
