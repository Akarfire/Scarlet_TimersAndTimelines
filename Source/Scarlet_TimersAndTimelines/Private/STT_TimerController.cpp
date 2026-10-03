// Fill out your copyright notice in the Description page of Project Settings.


#include "STT_TimerController.h"

// Sets default values for this component's properties
USTT_TimerController::USTT_TimerController()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USTT_TimerController::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void USTT_TimerController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Updates all of the timers
	UpdateTimers(DeltaTime);

	// Updates all timelines
	UpdateTimelines(DeltaTime);
}


// TIMERS

// Updates all active timers
void USTT_TimerController::UpdateTimers(float DeltaTime)
{
	TArray<FName> FinishedTimers;

	for (auto& TimerData : Timers)
		if (!TimerData.Value.Paused)
		{
			TimerData.Value.Time -= DeltaTime;

			if (TimerData.Value.Time <= 0)
				FinishedTimers.Add(TimerData.Value.TimerName);
		}

	for (auto& Timer : FinishedTimers)
	{
		Timers[Timer].OnTimerFinished.Broadcast(Timer);

		if (Timers[Timer].Loop)
			Timers[Timer].Time = Timers[Timer].InitialTime;

		else
			Timers[Timer].Paused = true;
	}
}


// Creates a new timer
bool USTT_TimerController::CreateTimer(FName TimerName, float Time, bool Loop, bool AutoStart)
{
	if (Timers.Contains(TimerName))
		return false;

	FTimer Timer;
	Timer.TimerName = TimerName;
	Timer.Time = Time;
	Timer.InitialTime = Time;
	Timer.Loop = Loop;

	Timers.Add(TimerName, Timer);

	if (AutoStart)
		StartTimer(TimerName);

	return true;
}

// Sets timer back to it's initial value
bool USTT_TimerController::ResetTimer(FName TimerName)
{
	if (!Timers.Contains(TimerName))
		return false;

	FTimer& Timer = Timers[TimerName];
	Timer.Time = Timer.InitialTime;

	return true;
}

// Starts a timer
bool USTT_TimerController::StartTimer(FName TimerName, bool Reset)
{
	if (!Timers.Contains(TimerName))
		return false;

	if (Reset)
		ResetTimer(TimerName);

	FTimer& Timer = Timers[TimerName];
	Timer.Paused = false;

	return true;
}

// Pauses a timer
bool USTT_TimerController::PauseTimer(FName TimerName)
{
	if (!Timers.Contains(TimerName))
		return false;

	FTimer& Timer = Timers[TimerName];
	Timer.Paused = true;

	return true;
}

// Deletes a timer
bool USTT_TimerController::DeleteTimer(FName TimerName)
{
	if (!Timers.Contains(TimerName))
		return false;

	Timers.Remove(TimerName);

	return true;
}


// Subscribes a rig element to a timer
bool USTT_TimerController::SubscribeToTimer(FName TimerName, UObject* Subscriber, FName NotificationFunctionName)
{
	if (!Timers.Contains(TimerName) || !Subscriber)
		return false;

	TScriptDelegate Delegate;
	Delegate.BindUFunction(Subscriber, NotificationFunctionName);
	Timers[TimerName].OnTimerFinished.Add(Delegate);

	return true;
}

// Unsubscribes a rig element from a timer
bool USTT_TimerController::UnSubscribeFromTimer(FName TimerName, UObject* Subscriber, FName NotificationFunctionName)
{
	if (!Timers.Contains(TimerName) || !Subscriber)
		return false;

	TScriptDelegate Delegate;
	Delegate.BindUFunction(Subscriber, NotificationFunctionName);
	Timers[TimerName].OnTimerFinished.Remove(Delegate);

	return true;
}

bool USTT_TimerController::ChangeTimerLength(FName TimerName, float NewLength)
{
	if (!Timers.Contains(TimerName))
		return false;

	Timers[TimerName].InitialTime = NewLength;

	return true;
}

// Returns the value of the timer
float USTT_TimerController::GetTimerValue(FName TimerName)
{
	if (!Timers.Contains(TimerName))
		return 0;

	return Timers[TimerName].Time;
}





// TIMELINES

// Updates all active timelines
void USTT_TimerController::UpdateTimelines(float DeltaTime)
{
	// An array of timelines, that have finished during this update
	TArray<FName> FinishedTimelines;

	// Updating all existing timelines
	for (auto& Timeline : Timelines)
	{
		if (!Timeline.Value.Paused)
		{
			float Decr = DeltaTime * Timeline.Value.PlaybackSpeed;
			if (Timeline.Value.Reversed)
				Decr *= -1;

			float PreviousTime = Timeline.Value.CurrentTime;
			Timeline.Value.CurrentTime -= Decr;
			
			// Processing notifies
			for (auto& Notify : Timeline.Value.Notifies)
			{
				float ActualNotifyTime = Timeline.Value.Length - Notify.Value;
				// If we have went over this notifie's time during this update, then broadcast the notify event
				if ((PreviousTime - ActualNotifyTime) * (Timeline.Value.CurrentTime - ActualNotifyTime) < 0)
					Timeline.Value.OnTimelineNotify.Broadcast(Timeline.Key, Notify.Key);
			}


			if (Timeline.Value.CurrentTime <= 0 || Timeline.Value.CurrentTime > Timeline.Value.Length)
				FinishedTimelines.Add(Timeline.Key);

			Timeline.Value.OnTimelineUpdated.Broadcast(Timeline.Key, Timeline.Value.Length - Timeline.Value.CurrentTime);
		}
	}

	// Processing finished timelines
	for (auto& Timeline : FinishedTimelines)
	{
		Timelines[Timeline].OnTimelineFinished.Broadcast(Timeline);

		if (Timelines[Timeline].Loop)
		{
			if (Timelines[Timeline].Reversed)
				Timelines[Timeline].CurrentTime = 0;

			else
				Timelines[Timeline].CurrentTime = Timelines[Timeline].Length;
		}

		else
			Timelines[Timeline].Paused = true;
	}
}

// Creates a new timeline
bool USTT_TimerController::CreateTimeline(FName TimelineName, float Length, bool Loop, bool AutoStart)
{
	if (Timelines.Contains(TimelineName))
		return false;

	FTimeline NewTimeline;
	NewTimeline.TimelineName = TimelineName;
	NewTimeline.Length = Length;
	NewTimeline.CurrentTime = Length;
	NewTimeline.Loop = Loop;
	NewTimeline.Paused = true;

	Timelines.Add(TimelineName, NewTimeline);

	if (AutoStart)
		StartTimeline(TimelineName);

	return true;
}

// Starts timeline playback
bool USTT_TimerController::StartTimeline(FName TimelineName)
{
	if (!Timelines.Contains(TimelineName))
		return false;

	ResetTimeline(TimelineName);
	Timelines[TimelineName].Paused = false;

	return true;
}

// Pauses timeline playback
bool USTT_TimerController::PauseTimeline(FName TimelineName)
{
	if (!Timelines.Contains(TimelineName))
		return false;

	Timelines[TimelineName].Paused = true;

	return true;
}

// Plays timeline starting at the current time
bool USTT_TimerController::PlayTimeline(FName TimelineName)
{
	if (!Timelines.Contains(TimelineName))
		return false;

	Timelines[TimelineName].Paused = false;

	return true;
}

// Reverses timeline playback
bool USTT_TimerController::ReverseTimeline(FName TimelineName)
{
	if (!Timelines.Contains(TimelineName))
		return false;

	Timelines[TimelineName].Reversed = !Timelines[TimelineName].Reversed;

	return true;
}

// Resets time line time to zero
bool USTT_TimerController::ResetTimeline(FName TimelineName)
{
	if (!Timelines.Contains(TimelineName))
		return false;

	Timelines[TimelineName].CurrentTime = Timelines[TimelineName].Length;

	return true;
}

// Sets timepline current time to a new value
bool USTT_TimerController::SetTimelineTime(FName TimelineName, float NewTime)
{
	if (!Timelines.Contains(TimelineName))
		return false;

	Timelines[TimelineName].CurrentTime = NewTime;

	return true;
}

// Sets timepline playback speed to a new value
bool USTT_TimerController::SetTimelinePlaybackSpeed(FName TimelineName, float NewSpeed)
{
	if (!Timelines.Contains(TimelineName))
		return false;

	Timelines[TimelineName].PlaybackSpeed = NewSpeed;

	return true;
}

// Deletes a timeline
bool USTT_TimerController::DeleteTimeline(FName TimelineName)
{
	if (!Timelines.Contains(TimelineName))
		return false;

	Timelines.Remove(TimelineName);

	return true;
}

// Registers a new notify in the given timeline
bool USTT_TimerController::RegisterTimelineNotify(FName TimelineName, FName NotifyName, float Time)
{
	if (!Timelines.Contains(TimelineName))
		return false;

	if (Timelines[TimelineName].Notifies.Contains(NotifyName)) 
		return false;

	Timelines[TimelineName].Notifies.Add(NotifyName, Time);

	return true;
}

bool USTT_TimerController::UnregisterTimelineNotify(FName TimelineName, FName NotifyName, float Time)
{
	if (!Timelines.Contains(TimelineName))
		return false;

	if (!Timelines[TimelineName].Notifies.Contains(NotifyName))
		return false;

	Timelines[TimelineName].Notifies.Remove(NotifyName);

	return true;
}

bool USTT_TimerController::ModifyTimelineNotify(FName TimelineName, FName NotifyName, float NewTime)
{

	if (!Timelines.Contains(TimelineName))
		return false;

	if (!Timelines[TimelineName].Notifies.Contains(NotifyName))
		return false;

	Timelines[TimelineName].Notifies[NotifyName] = NewTime;

	return true;
}

// Subscribes a rig element to a timepline
bool USTT_TimerController::SubscribeToTimeline(FName TimelineName, UObject* Subscriber, FName OnUpdateFunctionName, FName OnFinishedFunctionName, FName OnNotifyFunctionName)
{
	if (!Timelines.Contains(TimelineName) || !Subscriber)
		return false;

	// On Updated event
	if (OnUpdateFunctionName != FName(""))
	{
		FScriptDelegate Delegate;
		Delegate.BindUFunction(Subscriber, OnUpdateFunctionName);
		Timelines[TimelineName].OnTimelineUpdated.Add(Delegate);
	}

	// On Finished event
	if (OnFinishedFunctionName != FName(""))
	{
		FScriptDelegate Delegate;
		Delegate.BindUFunction(Subscriber, OnFinishedFunctionName);
		Timelines[TimelineName].OnTimelineFinished.Add(Delegate);
	}

	// On Notify event
	if (OnNotifyFunctionName != FName(""))
	{
		FScriptDelegate Delegate;
		Delegate.BindUFunction(Subscriber, OnNotifyFunctionName);
		Timelines[TimelineName].OnTimelineNotify.Add(Delegate);
	}

	return true;
}

// Unsubscribes a rig element from a timeline
bool USTT_TimerController::UnSubscribeFromTimeline(FName TimelineName, UObject* Subscriber, FName OnUpdateFunctionName, FName OnFinishedFunctionName, FName OnNotifyFunctionName)
{
	if (!Timelines.Contains(TimelineName) || !Subscriber)
		return false;

	// On Updated event
	if (OnUpdateFunctionName != FName(""))
	{
		FScriptDelegate Delegate;
		Delegate.BindUFunction(Subscriber, OnUpdateFunctionName);
		Timelines[TimelineName].OnTimelineUpdated.Remove(Delegate);
	}

	// On Finished event
	if (OnFinishedFunctionName != FName(""))
	{
		FScriptDelegate Delegate;
		Delegate.BindUFunction(Subscriber, OnFinishedFunctionName);
		Timelines[TimelineName].OnTimelineFinished.Remove(Delegate);
	}

	// On Notify event
	if (OnNotifyFunctionName != FName(""))
	{
		FScriptDelegate Delegate;
		Delegate.BindUFunction(Subscriber, OnNotifyFunctionName);
		Timelines[TimelineName].OnTimelineNotify.Remove(Delegate);
	}

	return true;
}

// Returns the value of the current time of the timeline
float USTT_TimerController::GetTimelineTime(FName TimelineName)
{
	if (!Timelines.Contains(TimelineName))
		return 0;

	return Timelines[TimelineName].CurrentTime;
}

// Returns the length of the timeline
float USTT_TimerController::GetTimelineLength(FName TimelineName)
{
	if (!Timelines.Contains(TimelineName))
		return 0;

	return Timelines[TimelineName].Length;
}

// Returns the playback speed of the timeline
float USTT_TimerController::GetTimelinePlaybackspeed(FName TimelineName)
{
	if (!Timelines.Contains(TimelineName))
		return 0;

	return Timelines[TimelineName].PlaybackSpeed;
}

// Returns timeline is reversed value
bool USTT_TimerController::GetTimelineReversed(FName TimelineName)
{
	if (!Timelines.Contains(TimelineName))
		return false;

	return Timelines[TimelineName].Reversed;
}