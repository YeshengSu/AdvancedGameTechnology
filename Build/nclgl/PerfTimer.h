/******************************************************************************
Class: PerfTimer
Author:
Pieran Marris <p.marris@newcastle.ac.uk>
Description:

Extension of the GameTimer class in nclgl for the purpose of creating a FPS based timer.

The PerfTimer is designed to help time how long various parts of code take to execute. By
wrapping the timed part of code with a call to BeginTimingSection and EndTimingSection respectively
it will automatically keep track of how long it took to execute.

The results are cached and only updated once per second (see m_UpdateInterval to change this), so can
be output directly to the user without having to worry about changing too fast to be readable.

Example usage can be seen inside the Tuts_Physics project where a PerfTimer is used to measure execution
time of the various components of the game engine.
*/

#pragma once
#include "GameTimer.h"
#include "NCLDebug.h"

class PerfTimer
{
public:
	PerfTimer()
		: m_UpdateInterval(1.0f)
		, m_RealTimeElapsed(0.0f)
	{
		m_Timer.GetTimedMS();
		memset(&m_CurrentData, 0, sizeof(PerfTimer_Data));
		memset(&m_PreviousData, 0, sizeof(PerfTimer_Data));
	}

	virtual ~PerfTimer() {}


	//Returns the maximum execution time recorded in the last second
	inline float GetHigh() const { return m_PreviousData._max; }

	//Returns the minimum execution time recorded in the last second
	inline float GetLow() const { return m_PreviousData._min; }

	//Returns the average execution time
	inline float GetAvg() const { return m_PreviousData._sum / float(m_PreviousData._num); }

	//Changes the rate at which the results are updated/replaced
	void SetUpdateInterval(float seconds) { m_UpdateInterval = seconds; }



	//Called /before/ the section of code to measure
	void BeginTimingSection()
	{
		m_Timer.GetTimedMS(); //In the underlying GameTimer class this resets the time since last called variable.
	}


	void EndTimingSection()
	{
		float elapsed = m_Timer.GetTimedMS();

		//Set the min/max execution times
		if (m_CurrentData._num == 0)
		{
			m_CurrentData._max = elapsed;
			m_CurrentData._min = elapsed;
		}
		else
		{
			m_CurrentData._max = max(m_CurrentData._max, elapsed);
			m_CurrentData._min = min(m_CurrentData._min, elapsed);
		}

		//Accumulate data required for calculating average execution time
		m_CurrentData._num++;
		m_CurrentData._sum += elapsed;
	}


	//Must be called once per frame in the update function.
	// This is used to update the performance data and let
	// it know when a second has passed in real time as apposed
	// to just the execution time of the code measured.
	void UpdateRealElapsedTime(float dt)
	{
		m_RealTimeElapsed += dt;
		if (m_RealTimeElapsed >= m_UpdateInterval)
		{
			m_RealTimeElapsed -= m_UpdateInterval;
			m_PreviousData = m_CurrentData;
			memset(&m_CurrentData, 0, sizeof(PerfTimer_Data));
		}
	}

	// Utility function to output the current performance data to the NCLDebug status
	// Must be called once per frame in order to be shown.
	void PrintOutputToStatusEntry(const Vector4& colour, const std::string& name)
	{
		NCLDebug::AddStatusEntry(colour, "%s%5.2fms [max:%5.2fms, min:%5.2fms]", name.c_str(), GetAvg(), GetHigh(), GetLow());
	}

protected:
	float m_UpdateInterval;
	float m_RealTimeElapsed;

	GameTimer m_Timer;

	struct PerfTimer_Data
	{
		float	_max;
		float	_min;

		//Average defined by (_sum / _num)
		float	_sum;
		int		_num;
	};

	PerfTimer_Data m_PreviousData;	// Front - Last completed measurement, shown for output
	PerfTimer_Data m_CurrentData;	// Back - Currently Updating
};