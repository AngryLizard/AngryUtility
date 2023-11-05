
#pragma once

#include "CoreMinimal.h"
#include "CallPromise.h"

/**
* In a asynchonized environment it is sometimes desired that only one string of actions is performed.
* This queue keeps track of running promises and makes sure none are run at the same time.
*/
template<typename ErrorType>
struct FCallPromiseQueue
{
	using FQueueDelegate = TDelegate<TSharedPtr<TCallPromiseBase<ErrorType>>()>;

	bool IsReady() const;
	void Queue(FQueueDelegate Call);
	TCallPromisePtr<ErrorType, void> Await();

private:
	TArray<TCallPromisePtr<ErrorType, void>> NextQueue;
};

template<typename ErrorType>
bool FCallPromiseQueue<ErrorType>::IsReady() const
{
	return NextQueue.Num() == 0;
}

template<typename ErrorType>
void FCallPromiseQueue<ErrorType>::Queue(FQueueDelegate Call)
{
	Await()
		->Then([this, Call]()
	{
		if (!Call.IsBound())
		{
			return;
		}

		Call.Execute()
			->Finally([this]()
		{
			if (NextQueue.Num() > 0)
			{
				TCallPromisePtr<ErrorType, void> Next = NextQueue[0];
				NextQueue.RemoveAt(0);
				Next->Accept();
			}
		});
	});
}

template<typename ErrorType>
TCallPromisePtr<ErrorType, void> FCallPromiseQueue<ErrorType>::Await()
{
	if (IsReady())
	{
		return TWebPromise<void>::Accepted();
	}

	TWebPromisePtr<void> Promise = TWebPromise<void>::Create();
	NextQueue.Emplace(Promise);
	return Promise;
}