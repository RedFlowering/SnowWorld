// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Async/AsyncWork.h"

/**
 * 비동기 작업 결과를 게임 스레드로 전달하기 위한 템플릿 구조체
 */
template<typename ResultType>
struct FHarmoniaAsyncTaskResult
{
	bool bSuccess;
	ResultType Result;
	FString ErrorMessage;

	FHarmoniaAsyncTaskResult()
		: bSuccess(false)
	{
	}

	FHarmoniaAsyncTaskResult(bool InSuccess, const ResultType& InResult, const FString& InError = TEXT(""))
		: bSuccess(InSuccess)
		, Result(InResult)
		, ErrorMessage(InError)
	{
	}
};

/**
 * 비동기 작업을 위한 기본 템플릿 클래스
 * 백그라운드 스레드에서 작업을 수행하고 게임 스레드에서 콜백을 실행합니다.
 */
template<typename ResultType>
class FHarmoniaAsyncTask : public FNonAbandonableTask
{
public:
	using FWorkFunction = TFunction<ResultType()>;
	using FCompletionCallback = TFunction<void(const ResultType&)>;

	FHarmoniaAsyncTask(FWorkFunction InWorkFunction, FCompletionCallback InCallback)
		: WorkFunction(MoveTemp(InWorkFunction))
		, CompletionCallback(MoveTemp(InCallback))
	{
	}

	// 백그라운드 스레드에서 실행되는 작업
	void DoWork()
	{
		if (WorkFunction)
		{
			Result = WorkFunction();
		}
	}

	// 작업이 완료된 후 게임 스레드에서 호출되는 함수
	void OnComplete()
	{
		if (CompletionCallback)
		{
			CompletionCallback(Result);
		}
	}

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FHarmoniaAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}

private:
	FWorkFunction WorkFunction;
	FCompletionCallback CompletionCallback;
	ResultType Result;
};

/**
 * 비동기 작업 헬퍼 함수들
 */
namespace HarmoniaAsyncHelpers
{
	/**
	 * 비동기 작업을 실행하고 완료 시 게임 스레드에서 콜백을 호출합니다.
	 * @param WorkFunction - 백그라운드 스레드에서 실행할 작업
	 * @param CompletionCallback - 게임 스레드에서 실행할 완료 콜백 (선택사항)
	 */
	template<typename ResultType>
	static void ExecuteAsync(
		TFunction<ResultType()> WorkFunction,
		TFunction<void(const ResultType&)> CompletionCallback = nullptr)
	{
		if (!WorkFunction)
		{
			return;
		}

		// AsyncTask를 사용하여 백그라운드에서 작업 실행
		using FTaskType = FHarmoniaAsyncTask<ResultType>;

		auto Task = new FAsyncTask<FTaskType>(
			MoveTemp(WorkFunction),
			[CompletionCallback](const ResultType& Result)
			{
				// 게임 스레드에서 콜백 실행
				if (CompletionCallback)
				{
					AsyncTask(ENamedThreads::GameThread, [CompletionCallback, Result]()
					{
						CompletionCallback(Result);
					});
				}
			}
		);

		Task->StartBackgroundTask();
	}

	/**
	 * 지연 후 게임 스레드에서 함수를 실행합니다.
	 * @param DelayInSeconds - 지연 시간 (초)
	 * @param Callback - 실행할 함수
	 */
	static void ExecuteDelayed(UWorld* World, float DelayInSeconds, TFunction<void()> Callback)
	{
		if (!World || !Callback)
		{
			return;
		}

		FTimerHandle TimerHandle;
		World->GetTimerManager().SetTimer(
			TimerHandle,
			[Callback]() { Callback(); },
			DelayInSeconds,
			false
		);
	}

	/**
	 * 네트워크 작업 시뮬레이션 (테스트용)
	 * 실제 서버 연동 시 HTTP 요청 등으로 대체해야 합니다.
	 */
	template<typename ResultType>
	static void SimulateNetworkOperation(
		TFunction<ResultType()> SimulationFunction,
		TFunction<void(const ResultType&)> CompletionCallback,
		float SimulatedLatency = 0.5f)
	{
		// 백그라운드에서 시뮬레이션 실행
		ExecuteAsync<ResultType>(
			[SimulationFunction, SimulatedLatency]() -> ResultType
			{
				// 네트워크 지연 시뮬레이션
				FPlatformProcess::Sleep(SimulatedLatency);
				return SimulationFunction();
			},
			CompletionCallback
		);
	}
}
