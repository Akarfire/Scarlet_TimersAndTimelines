// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "STT_TimerController.generated.h"


// DELEGATES

// Fires off when timer has finished
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimerFinished, FName, InTimerName);

// Fires off every tick when timeline is playing, reports the current time of the timeline
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimelineUpdated, FName, InTimelineName, float, InCurrentTime);

// Fires off when timeline has finished playing
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimelineFinished, FName, InTimelineName);

// Fires off when timeline has encountered a registered notify
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimelineNotify, FName, InTimelineName, FName, InNotifyName);



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SCARLET_TIMERSANDTIMELINES_API USTT_TimerController : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USTT_TimerController();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		

// TIMERS

protected:

	// Timer data struct
	struct FTimer
	{
		FName TimerName;
		bool Paused = true;
		float Time = 0.f;
		float InitialTime = 0.f;
		bool Loop = false;

		FOnTimerFinished OnTimerFinished;
	};

	// Map of all active Timers
	TMap<FName, FTimer> Timers;

	// Updates all active timers
	void UpdateTimers(float DeltaTime);

public:
	// Creates a new timer
	UFUNCTION(BlueprintCallable, Category = "STT|Timers")
	bool CreateTimer(FName TimerName, float Time, bool Loop = false, bool AutoStart = true);

	// Sets timer back to it's initial value
	UFUNCTION(BlueprintCallable, Category = "STT|Timers")
	bool ResetTimer(FName TimerName);

	// Starts a timer
	UFUNCTION(BlueprintCallable, Category = "STT|Timers")
	bool StartTimer(FName TimerName, bool Reset = true);

	// Pauses a timer
	UFUNCTION(BlueprintCallable, Category = "MSTT|Timers")
	bool PauseTimer(FName TimerName);

	// Deletes a timer
	UFUNCTION(BlueprintCallable, Category = "STT|Timers")
	bool DeleteTimer(FName TimerName);

	// Subscribes a rig element to a timer
	UFUNCTION(BlueprintCallable, Category = "STT|Timers")
	bool SubscribeToTimer(FName TimerName, UObject* Subscriber, FName NotificationFunctionName);

	// Unsubscribes a rig element from a timer
	UFUNCTION(BlueprintCallable, Category = "STT|Timers")
	bool UnSubscribeFromTimer(FName TimerName, UObject* Subscriber, FName NotificationFunctionName);

	// Changes timer's length (only applied after a reset)
	UFUNCTION(BlueprintCallable, Category = "STT|Timers")
	bool ChangeTimerLength(FName TimerName, float NewLength);

	// Returns the value of the timer
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "STT|Timers")
	float GetTimerValue(FName TimerName);

	// DEBUG
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "STT|Timers|Debug")
	void GetAllTimerNames(TArray<FName>& OutNames) { Timers.GetKeys(OutNames); }


// TIMELINES

protected:

	// Timeline data struct
	struct FTimeline
	{
		FName TimelineName;
		//FTimer Timer;

		float CurrentTime = 0.f;
		float Length = 0.f;

		float PlaybackSpeed = 1.f;
		bool Reversed = false;
		bool Paused = true;
		bool Loop = false;

		// Notifies list <Notify name, time>
		TMap<FName, float> Notifies;

		FOnTimelineUpdated OnTimelineUpdated;
		FOnTimelineFinished OnTimelineFinished;
		FOnTimelineNotify OnTimelineNotify;
	};

	// Map of all active timelines
	TMap<FName, FTimeline> Timelines;

	// Updates all active timelines
	void UpdateTimelines(float DeltaTime);

public:
	// Creates a new timeline
	UFUNCTION(BlueprintCallable, Category = "STT|Timelines")
	bool CreateTimeline(FName TimelineName, float Length, bool Loop = false, bool AutoStart = false);

	// Starts timeline playback
	UFUNCTION(BlueprintCallable, Category = "STT|Timelines")
	bool StartTimeline(FName TimelineName);

	// Pauses timeline playback
	UFUNCTION(BlueprintCallable, Category = "STT|Timelines")
	bool PauseTimeline(FName TimelineName);

	// Plays timeline starting at the current time
	UFUNCTION(BlueprintCallable, Category = "STT|Timelines")
	bool PlayTimeline(FName TimelineName);

	// Reverses timeline playback
	UFUNCTION(BlueprintCallable, Category = "STT|Timelines")
	bool ReverseTimeline(FName TimelineName);

	// Resets time line time to zero
	UFUNCTION(BlueprintCallable, Category = "STT|Timelines")
	bool ResetTimeline(FName TimelineName);

	// Sets timepline current time to a new value
	UFUNCTION(BlueprintCallable, Category = "STT|Timelines")
	bool SetTimelineTime(FName TimelineName, float NewTime);

	// Sets timepline playback speed to a new value
	UFUNCTION(BlueprintCallable, Category = "STT|Timelines")
	bool SetTimelinePlaybackSpeed(FName TimelineName, float NewSpeed);

	// Deletes a timeline
	UFUNCTION(BlueprintCallable, Category = "STT|Timelines")
	bool DeleteTimeline(FName TimelineName);

	// Registers a new notify in the given timeline
	UFUNCTION(BlueprintCallable, Category = "STT|Timelines")
	bool RegisterTimelineNotify(FName TimelineName, FName NotifyName, float Time);

	// Unregisters an existing notify in the given timeline
	UFUNCTION(BlueprintCallable, Category = "STT|Timelines")
	bool UnregisterTimelineNotify(FName TimelineName, FName NotifyName, float Time);

	// Changes the time value of the given timeline's notify
	UFUNCTION(BlueprintCallable, Category = "STT|Timelines")
	bool ModifyTimelineNotify(FName TimelineName, FName NotifyName, float NewTime);

	// Subscribes a rig element to a timepline, pass in "" as function name, if you don't want to subscriber to that event
	UFUNCTION(BlueprintCallable, Category = "STT|Timelines")
	bool SubscribeToTimeline(FName TimelineName, UObject* Subscriber, FName OnUpdateFunctionName, FName OnFinishedFunctionName = "", FName OnNotifyFunctionName = "");

	// Unsubscribes a rig element from a timeline
	UFUNCTION(BlueprintCallable, Category = "STT|Timelines")
	bool UnSubscribeFromTimeline(FName TimelineName, UObject* Subscriber, FName OnUpdateFunctionName, FName OnFinishedFunctionName = "", FName OnNotifyFunctionName = "");

	// Returns the value of the current time of the timeline
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "STT|Timelines")
	float GetTimelineTime(FName TimelineName);

	// Returns the length of the timeline
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "STT|Timelines")
	float GetTimelineLength(FName TimelineName);

	// Returns the playback speed of the timeline
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "STT|Timelines")
	float GetTimelinePlaybackspeed(FName TimelineName);

	// Returns timeline is reversed value
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "STT|Timelines")
	bool GetTimelineReversed(FName TimelineName);

	// DEBUG
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "STT|Timelines|Debug")
	void GetAllTimelineNames(TArray<FName>& OutNames) { Timelines.GetKeys(OutNames); }
};
