// Maintained by AngryLizard, netliz.net

#pragma once

#include "CoreMinimal.h"

template<typename ErrorType>
class TCallPromiseBase;

/**
* Conditions for SFINAE to differentiate between different lambda return types,
* because promise behaviour is different depending on whether the input lambda a) is another promise and b) has no return statement.
*/
template<typename ErrorType, typename ReturnType>
using EnableIfPromisePtr = std::enable_if_t<std::is_convertible_v<ReturnType, TSharedPtr<TCallPromiseBase<ErrorType>>>, void*>;

template<typename ErrorType, typename ReturnType>
using EnableIfValuePtr = std::enable_if_t<!std::is_void_v<ReturnType> && !std::is_convertible_v<ReturnType, TSharedPtr<TCallPromiseBase<ErrorType>>>, void*>;

template<typename ErrorType, typename ReturnType>
using EnableIfVoidPtr = std::enable_if_t<std::is_void_v<ReturnType> && !std::is_convertible_v<ReturnType, TSharedPtr<TCallPromiseBase<ErrorType>>>, void*>;

#define DeclReturnType_OneParam(F, T) decltype(Func(*(T*)nullptr))
#define DeclReturnType(F) decltype(Func())


template<typename ErrorType, typename PromiseType>
class TCallPromise;

template<typename ErrorType, typename PromiseType>
using TCallPromisePtr = TSharedPtr<TCallPromise<ErrorType, PromiseType>>;

/**
* Promise base includes the ability to reject a promise and catch rejected promises.
* Rejected promises will skip through all Then(...) calls until they reach a Catch(...).
*/
template<typename ErrorType>
class TCallPromiseBase : public TSharedFromThis<TCallPromiseBase<ErrorType>>
{
	using FRejectDelegate = TDelegate<void(const ErrorType&)>;
	using FFinallyDelegate = TDelegate<void()>;

public:
	void Reject(const ErrorType& Error);
	void Append(TSharedPtr<TCallPromiseBase> Promise, FRejectDelegate Reject);

	void Finalize();
	void Append(TSharedPtr<TCallPromiseBase> Promise, FFinallyDelegate Finally);

	template<typename FuncType>	auto Catch(FuncType Func, EnableIfPromisePtr<ErrorType, DeclReturnType_OneParam(Func, ErrorType)> sfinae = nullptr);
	template<typename FuncType>	auto Catch(FuncType Func, EnableIfValuePtr<ErrorType, DeclReturnType_OneParam(Func, ErrorType)> sfinae = nullptr);
	template<typename FuncType> auto Catch(FuncType Func, EnableIfVoidPtr<ErrorType, DeclReturnType_OneParam(Func, ErrorType)> sfinae = nullptr);
	template<typename UserClass, typename FuncType>	auto WeakCatch(UserClass* Object, FuncType Func, EnableIfPromisePtr<ErrorType, DeclReturnType_OneParam(Func, ErrorType)> sfinae = nullptr);
	template<typename UserClass, typename FuncType>	auto WeakCatch(UserClass* Object, FuncType Func, EnableIfValuePtr<ErrorType, DeclReturnType_OneParam(Func, ErrorType)> sfinae = nullptr);
	template<typename UserClass, typename FuncType> auto WeakCatch(UserClass* Object, FuncType Func, EnableIfVoidPtr<ErrorType, DeclReturnType_OneParam(Func, ErrorType)> sfinae = nullptr);

	template<typename FuncType>	auto Finally(FuncType Func, EnableIfPromisePtr<ErrorType, DeclReturnType(Func)> sfinae = nullptr);
	template<typename FuncType>	auto Finally(FuncType Func, EnableIfValuePtr<ErrorType, DeclReturnType(Func)> sfinae = nullptr);
	template<typename FuncType> auto Finally(FuncType Func, EnableIfVoidPtr<ErrorType, DeclReturnType(Func)> sfinae = nullptr);
	template<typename UserClass, typename FuncType>	auto WeakFinally(UserClass* Object, FuncType Func, EnableIfPromisePtr<ErrorType, DeclReturnType(Func)> sfinae = nullptr);
	template<typename UserClass, typename FuncType>	auto WeakFinally(UserClass* Object, FuncType Func, EnableIfValuePtr<ErrorType, DeclReturnType(Func)> sfinae = nullptr);
	template<typename UserClass, typename FuncType> auto WeakFinally(UserClass* Object, FuncType Func, EnableIfVoidPtr<ErrorType, DeclReturnType(Func)> sfinae = nullptr);

protected:
	bool bIsFinalized = false;
	TOptional<ErrorType> RejectError;

	FRejectDelegate RejectDelegate;
	FFinallyDelegate FinallyDelegate;
};

/**
* Generalized promise allows returning a value that will be provided in the future.
* Promises can be chained with Then(...) calls.
*/
template<typename ErrorType, typename PromiseType>
class TCallPromise : public TCallPromiseBase<ErrorType>
{
	template <typename, ESPMode>
	friend class SharedPointerInternals::TIntrusiveReferenceController; // Enable MakeShared

	template <typename, typename>
	friend class TCallPromise;

	using FAcceptDelegate = TDelegate<void(const PromiseType&)>;

public:
	static TCallPromisePtr<ErrorType, PromiseType> Create();
	static TCallPromisePtr<ErrorType, PromiseType> Accepted(const PromiseType& Value);
	static TCallPromisePtr<ErrorType, PromiseType> Rejected(const ErrorType& Error);

	void Accept(const PromiseType& Value);
	void Merge(TCallPromisePtr<ErrorType, PromiseType> Promise);
	void Append(TSharedPtr<TCallPromiseBase<ErrorType>> Promise, FAcceptDelegate Accept);

	template<typename FuncType>	auto Then(FuncType Func, EnableIfPromisePtr<ErrorType, DeclReturnType_OneParam(Func, PromiseType)> sfinae = nullptr);
	template<typename FuncType>	auto Then(FuncType Func, EnableIfValuePtr<ErrorType, DeclReturnType_OneParam(Func, PromiseType)> sfinae = nullptr);
	template<typename FuncType> auto Then(FuncType Func, EnableIfVoidPtr<ErrorType, DeclReturnType_OneParam(Func, PromiseType)> sfinae = nullptr);
	template<typename UserClass, typename FuncType>	auto WeakThen(UserClass* Object, FuncType Func, EnableIfPromisePtr<ErrorType, DeclReturnType_OneParam(Func, PromiseType)> sfinae = nullptr);
	template<typename UserClass, typename FuncType>	auto WeakThen(UserClass* Object, FuncType Func, EnableIfValuePtr<ErrorType, DeclReturnType_OneParam(Func, PromiseType)> sfinae = nullptr);
	template<typename UserClass, typename FuncType> auto WeakThen(UserClass* Object, FuncType Func, EnableIfVoidPtr<ErrorType, DeclReturnType_OneParam(Func, PromiseType)> sfinae = nullptr);

private:
	TCallPromise() {}
	TOptional<PromiseType> AcceptValue;

	FAcceptDelegate AcceptDelegate;
};

/**
* Empty promise spezialization for functions that return nothing.
*/
template<typename ErrorType>
class TCallPromise<ErrorType, void> : public TCallPromiseBase<ErrorType>
{
	template <typename, ESPMode>
	friend class SharedPointerInternals::TIntrusiveReferenceController; // Enable MakeShared

	template <typename, typename>
	friend class TCallPromise;

	using FAcceptDelegate = TDelegate<void()>;

public:
	static TCallPromisePtr<ErrorType, void> Create();
	static TCallPromisePtr<ErrorType, void> Accepted();
	static TCallPromisePtr<ErrorType, void> Rejected(const ErrorType& Error);

	void Accept();
	void Merge(TCallPromisePtr<ErrorType, void> Promise);
	void Append(TSharedPtr<TCallPromiseBase<ErrorType>> Promise, FAcceptDelegate Accept);

	template<typename FuncType>	auto Then(FuncType Func, EnableIfPromisePtr<ErrorType, DeclReturnType(Func)> sfinae = nullptr);
	template<typename FuncType>	auto Then(FuncType Func, EnableIfValuePtr<ErrorType, DeclReturnType(Func)> sfinae = nullptr);
	template<typename FuncType> auto Then(FuncType Func, EnableIfVoidPtr<ErrorType, DeclReturnType(Func)> sfinae = nullptr);
	template<typename UserClass, typename FuncType>	auto WeakThen(UserClass* Object, FuncType Func, EnableIfPromisePtr<ErrorType, DeclReturnType(Func)> sfinae = nullptr);
	template<typename UserClass, typename FuncType>	auto WeakThen(UserClass* Object, FuncType Func, EnableIfValuePtr<ErrorType, DeclReturnType(Func)> sfinae = nullptr);
	template<typename UserClass, typename FuncType> auto WeakThen(UserClass* Object, FuncType Func, EnableIfVoidPtr<ErrorType, DeclReturnType(Func)> sfinae = nullptr);

private:
	TCallPromise() {}
	bool bIsAccepted = false;

	FAcceptDelegate AcceptDelegate;
};

// TCallPromiseBase Implementation

template<typename ErrorType>
void TCallPromiseBase<ErrorType>::Reject(const ErrorType& Error)
{
	RejectError = Error;
	if (RejectDelegate.IsBound())
	{
		RejectDelegate.ExecuteIfBound(Error);
		RejectDelegate.Unbind();
	}
	else
	{
		Finalize();
	}
}

template<typename ErrorType>
void TCallPromiseBase<ErrorType>::Finalize()
{
	bIsFinalized = true;
	FinallyDelegate.ExecuteIfBound();
	FinallyDelegate.Unbind();
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

template<typename ErrorType>
template<typename FuncType>
auto TCallPromiseBase<ErrorType>::Catch(FuncType Func, EnableIfPromisePtr<ErrorType, DeclReturnType_OneParam(Func, ErrorType)> sfinae)
{
	using ReturnPromiseType = DeclReturnType_OneParam(Func, ErrorType)::ElementType;
	TSharedPtr<ReturnPromiseType> Promise = ReturnPromiseType::Create();

	Append(Promise, FRejectDelegate::CreateLambda(
		[Func = MoveTemp(Func), Promise](const ErrorType& Error)
		{
			Promise->Merge(Func(Error));
		}));

	return Promise;
}

template<typename ErrorType>
template<typename FuncType>
auto TCallPromiseBase<ErrorType>::Catch(FuncType Func, EnableIfValuePtr<ErrorType, DeclReturnType_OneParam(Func, ErrorType)> sfinae)
{
	using ReturnType = DeclReturnType_OneParam(Func, ErrorType);
	TCallPromisePtr<ErrorType, ReturnType> Promise = TCallPromise<ErrorType, ReturnType>::Create();

	Append(Promise, FRejectDelegate::CreateLambda(
		[Func = MoveTemp(Func), Promise](const ErrorType& Error)
		{
			Promise->Accept(Func(Error));
		}));

	return Promise;
}

template<typename ErrorType>
template<typename FuncType>
auto TCallPromiseBase<ErrorType>::Catch(FuncType Func, EnableIfVoidPtr<ErrorType, DeclReturnType_OneParam(Func, ErrorType)> sfinae)
{
	TCallPromisePtr<ErrorType, void> Promise = TCallPromise<ErrorType, void>::Create();

	Append(Promise, FRejectDelegate::CreateLambda(
		[Func = MoveTemp(Func), Promise](const ErrorType& Error)
		{
			Func(Error);
			Promise->Accept();
		}));

	return Promise;
}

template<typename ErrorType>
template<typename UserClass, typename FuncType>
auto TCallPromiseBase<ErrorType>::WeakCatch(UserClass* Object, FuncType Func, EnableIfPromisePtr<ErrorType, DeclReturnType_OneParam(Func, ErrorType)> sfinae)
{
	using ReturnPromiseType = DeclReturnType_OneParam(Func, ErrorType)::ElementType;
	TSharedPtr<ReturnPromiseType> Promise = ReturnPromiseType::Create();

	Append(Promise, FRejectDelegate::CreateWeakLambda(Object,
		[Func = MoveTemp(Func), Promise](const ErrorType& Error)
		{
			Promise->Merge(Func(Error));
		}));

	return Promise;
}

template<typename ErrorType>
template<typename UserClass, typename FuncType>
auto TCallPromiseBase<ErrorType>::WeakCatch(UserClass* Object, FuncType Func, EnableIfValuePtr<ErrorType, DeclReturnType_OneParam(Func, ErrorType)> sfinae)
{
	using ReturnType = DeclReturnType_OneParam(Func, ErrorType);
	TCallPromisePtr<ErrorType, ReturnType> Promise = TCallPromise<ErrorType, ReturnType>::Create();

	Append(Promise, FRejectDelegate::CreateWeakLambda(Object,
		[Func = MoveTemp(Func), Promise](const ErrorType& Error)
		{
			Promise->Accept(Func(Error));
		}));

	return Promise;
}

template<typename ErrorType>
template<typename UserClass, typename FuncType>
auto TCallPromiseBase<ErrorType>::WeakCatch(UserClass* Object, FuncType Func, EnableIfVoidPtr<ErrorType, DeclReturnType_OneParam(Func, ErrorType)> sfinae)
{
	TCallPromisePtr<ErrorType, void> Promise = TCallPromise<ErrorType, void>::Create();

	Append(Promise, FRejectDelegate::CreateWeakLambda(Object,
		[Func = MoveTemp(Func), Promise](const ErrorType& Error)
		{
			Func(Error);
			Promise->Accept();
		}));

	return Promise;
}

template<typename ErrorType>
void TCallPromiseBase<ErrorType>::Append(TSharedPtr<TCallPromiseBase> Promise, FRejectDelegate Reject)
{
	if (RejectError.IsSet())
	{
		Reject.ExecuteIfBound(RejectError.GetValue());
		return;
	}

	if (bIsFinalized)
	{
		Promise->Finalize();
		return;
	}

	check(!RejectDelegate.IsBound());
	RejectDelegate = MoveTemp(Reject);

	check(!FinallyDelegate.IsBound());
	FinallyDelegate = FFinallyDelegate::CreateLambda([Promise]()
	{
		Promise->Finalize();
	});
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

template<typename ErrorType>
template<typename FuncType>
auto TCallPromiseBase<ErrorType>::Finally(FuncType Func, EnableIfPromisePtr<ErrorType, DeclReturnType(Func)> sfinae)
{
	using ReturnPromiseType = DeclReturnType_OneParam(Func, ErrorType)::ElementType;
	TSharedPtr<ReturnPromiseType> Promise = ReturnPromiseType::Create();

	Append(Promise, FFinallyDelegate::CreateLambda(
		[Func = MoveTemp(Func), Promise]()
		{
			Promise->Merge(Func());
		}));

	return Promise;
}

template<typename ErrorType>
template<typename FuncType>
auto TCallPromiseBase<ErrorType>::Finally(FuncType Func, EnableIfValuePtr<ErrorType, DeclReturnType(Func)> sfinae)
{
	using ReturnType = DeclReturnType_OneParam(Func, ErrorType);
	TCallPromisePtr<ErrorType, ReturnType> Promise = TCallPromise<ErrorType, ReturnType>::Create();

	Append(Promise, FFinallyDelegate::CreateLambda(
		[Func = MoveTemp(Func), Promise]()
		{
			Promise->Accept(Func());
		}));

	return Promise;
}

template<typename ErrorType>
template<typename FuncType>
auto TCallPromiseBase<ErrorType>::Finally(FuncType Func, EnableIfVoidPtr<ErrorType, DeclReturnType(Func)> sfinae)
{
	TCallPromisePtr<ErrorType, void> Promise = TCallPromise<ErrorType, void>::Create();

	Append(Promise, FFinallyDelegate::CreateLambda(
		[Func = MoveTemp(Func), Promise]()
		{
			Func();
			Promise->Accept();
		}));

	return Promise;
}

template<typename ErrorType>
template<typename UserClass, typename FuncType>
auto TCallPromiseBase<ErrorType>::WeakFinally(UserClass* Object, FuncType Func, EnableIfPromisePtr<ErrorType, DeclReturnType(Func)> sfinae)
{
	using ReturnPromiseType = DeclReturnType(Func)::ElementType;
	TSharedPtr<ReturnPromiseType> Promise = ReturnPromiseType::Create();

	Append(Promise, FFinallyDelegate::CreateWeakLambda(Object,
		[Func = MoveTemp(Func), Promise]()
		{
			Promise->Merge(Func());
		}));

	return Promise;
}

template<typename ErrorType>
template<typename UserClass, typename FuncType>
auto TCallPromiseBase<ErrorType>::WeakFinally(UserClass* Object, FuncType Func, EnableIfValuePtr<ErrorType, DeclReturnType(Func)> sfinae)
{
	using ReturnType = DeclReturnType(Func);
	TCallPromisePtr<ErrorType, ReturnType> Promise = TCallPromise<ErrorType, ReturnType>::Create();

	Append(Promise, FFinallyDelegate::CreateWeakLambda(Object,
		[Func = MoveTemp(Func), Promise]()
		{
			Promise->Accept(Func());
		}));

	return Promise;
}

template<typename ErrorType>
template<typename UserClass, typename FuncType>
auto TCallPromiseBase<ErrorType>::WeakFinally(UserClass* Object, FuncType Func, EnableIfVoidPtr<ErrorType, DeclReturnType(Func)> sfinae)
{
	TCallPromisePtr<ErrorType, void> Promise = TCallPromise<ErrorType, void>::Create();

	Append(Promise, FFinallyDelegate::CreateWeakLambda(Object,
		[Func = MoveTemp(Func), Promise]()
		{
			Func();
			Promise->Accept();
		}));

	return Promise;
}

template<typename ErrorType>
void TCallPromiseBase<ErrorType>::Append(TSharedPtr<TCallPromiseBase> Promise, FFinallyDelegate Finally)
{
	if (bIsFinalized)
	{
		Finally.ExecuteIfBound();
		return;
	}

	check(!FinallyDelegate.IsBound());
	FinallyDelegate = MoveTemp(Finally);
}

// TCallPromise<ErrorType, PromiseType> Implementation

template<typename ErrorType, typename PromiseType>
TCallPromisePtr<ErrorType, PromiseType> TCallPromise<ErrorType, PromiseType>::Create()
{
	return MakeShared<TCallPromise<ErrorType, PromiseType>>();
}

template<typename ErrorType, typename PromiseType>
TCallPromisePtr<ErrorType, PromiseType> TCallPromise<ErrorType, PromiseType>::Accepted(const PromiseType& Value)
{
	TCallPromisePtr<ErrorType, PromiseType> Promise = Create();
	Promise->Accept(Value);
	return Promise;
}

template<typename ErrorType, typename PromiseType>
TCallPromisePtr<ErrorType, PromiseType> TCallPromise<ErrorType, PromiseType>::Rejected(const ErrorType& Error)
{
	TCallPromisePtr<ErrorType, PromiseType> Promise = Create();
	Promise->Reject(Error);
	return Promise;
}

template<typename ErrorType, typename PromiseType>
void TCallPromise<ErrorType, PromiseType>::Accept(const PromiseType& Value)
{
	AcceptValue = Value;
	if (AcceptDelegate.IsBound())
	{
		AcceptDelegate.ExecuteIfBound(Value);
		AcceptDelegate.Unbind();
	}
	else
	{
		Finalize();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

template<typename ErrorType, typename PromiseType>
template<typename FuncType>
auto TCallPromise<ErrorType, PromiseType>::Then(FuncType Func, EnableIfPromisePtr<ErrorType, DeclReturnType_OneParam(Func, PromiseType)> sfinae)
{
	using ReturnPromiseType = DeclReturnType_OneParam(Func, PromiseType)::ElementType;
	TSharedPtr<ReturnPromiseType> Promise = ReturnPromiseType::Create();

	Append(Promise, FAcceptDelegate::CreateLambda(
		[Func = MoveTemp(Func), Promise](const PromiseType& Result)
		{
			Promise->Merge(Func(Result));
		}));

	return Promise;
}

template<typename ErrorType, typename PromiseType>
template<typename FuncType>
auto TCallPromise<ErrorType, PromiseType>::Then(FuncType Func, EnableIfValuePtr<ErrorType, DeclReturnType_OneParam(Func, PromiseType)> sfinae)
{
	using ReturnType = DeclReturnType_OneParam(Func, PromiseType);
	TCallPromisePtr<ErrorType, ReturnType> Promise = TCallPromise<ErrorType, ReturnType>::Create();

	Append(Promise, FAcceptDelegate::CreateLambda(
		[Func = MoveTemp(Func), Promise](const PromiseType& Result)
		{
			Promise->Accept(Func(Result));
		}));

	return Promise;
}

template<typename ErrorType, typename PromiseType>
template<typename FuncType>
auto TCallPromise<ErrorType, PromiseType>::Then(FuncType Func, EnableIfVoidPtr<ErrorType, DeclReturnType_OneParam(Func, PromiseType)> sfinae)
{
	TCallPromisePtr<ErrorType, void> Promise = TCallPromise<ErrorType, void>::Create();

	Append(Promise, FAcceptDelegate::CreateLambda(
		[Func = MoveTemp(Func), Promise](const PromiseType& Result)
		{
			Func(Result);
			Promise->Accept();
		}));

	return Promise;
}

template<typename ErrorType, typename PromiseType>
template<typename UserClass, typename FuncType>
auto TCallPromise<ErrorType, PromiseType>::WeakThen(UserClass* Object, FuncType Func, EnableIfPromisePtr<ErrorType, DeclReturnType_OneParam(Func, PromiseType)> sfinae)
{
	using ReturnPromiseType = DeclReturnType_OneParam(Func, PromiseType)::ElementType;
	TSharedPtr<ReturnPromiseType> Promise = ReturnPromiseType::Create();

	Append(Promise, FAcceptDelegate::CreateWeakLambda(Object,
		[Func = MoveTemp(Func), Promise](const PromiseType& Result)
		{
			Promise->Merge(Func(Result));
		}));

	return Promise;
}

template<typename ErrorType, typename PromiseType>
template<typename UserClass, typename FuncType>
auto TCallPromise<ErrorType, PromiseType>::WeakThen(UserClass* Object, FuncType Func, EnableIfValuePtr<ErrorType, DeclReturnType_OneParam(Func, PromiseType)> sfinae)
{
	using ReturnType = DeclReturnType_OneParam(Func, PromiseType);
	TCallPromisePtr<ErrorType, ReturnType> Promise = TCallPromise<ErrorType, ReturnType>::Create();

	Append(Promise, FAcceptDelegate::CreateWeakLambda(Object,
		[Func = MoveTemp(Func), Promise](const PromiseType& Result)
		{
			Promise->Accept(Func(Result));
		}));

	return Promise;
}

template<typename ErrorType, typename PromiseType>
template<typename UserClass, typename FuncType>
auto TCallPromise<ErrorType, PromiseType>::WeakThen(UserClass* Object, FuncType Func, EnableIfVoidPtr<ErrorType, DeclReturnType_OneParam(Func, PromiseType)> sfinae)
{
	TCallPromisePtr<ErrorType, void> Promise = TCallPromise<ErrorType, void>::Create();

	Append(Promise, FAcceptDelegate::CreateWeakLambda(Object,
		[Func = MoveTemp(Func), Promise](const PromiseType& Result)
		{
			Func(Result);
			Promise->Accept();
		}));

	return Promise;
}

template<typename ErrorType, typename PromiseType>
void TCallPromise<ErrorType, PromiseType>::Append(TSharedPtr<TCallPromiseBase<ErrorType>> Promise, FAcceptDelegate Accept)
{
	if (AcceptValue.IsSet())
	{
		Accept.ExecuteIfBound(AcceptValue.GetValue());
		return;
	}

	if (RejectError.IsSet())
	{
		Promise->Reject(RejectError.GetValue());
		return;
	}

	if (bIsFinalized)
	{
		Promise->Finalize();
		return;
	}
	
	check(!AcceptDelegate.IsBound());
	AcceptDelegate = MoveTemp(Accept);

	check(!RejectDelegate.IsBound());
	RejectDelegate = FRejectDelegate::CreateLambda([Promise](const ErrorType& Error)
	{
		Promise->Reject(Error);
	});

	check(!FinallyDelegate.IsBound());
	FinallyDelegate = FFinallyDelegate::CreateLambda([Promise]()
	{
		Promise->Finalize();
	});
}

template<typename ErrorType, typename PromiseType>
void TCallPromise<ErrorType, PromiseType>::Merge(TCallPromisePtr<ErrorType, PromiseType> Promise)
{
	if (Promise->RejectError.IsSet())
	{
		Reject(Promise->RejectError.GetValue());
		return;
	}

	if (Promise->AcceptValue.IsSet())
	{
		Accept(Promise->AcceptValue.GetValue());
		return;
	}

	if (Promise->bIsFinalized)
	{
		Finalize();
		return;
	}

	check(!Promise->AcceptDelegate.IsBound());
	Promise->AcceptDelegate = FAcceptDelegate::CreateLambda([this, SelfPromise = AsShared()](const PromiseType& Result)
	{
		// SelfPromise prevents deletion
		Accept(Result);
	});

	check(!Promise->RejectDelegate.IsBound());
	Promise->RejectDelegate = FRejectDelegate::CreateLambda([SelfPromise = AsShared()](const ErrorType& Error)
	{
		SelfPromise->Reject(Error);
	});

	check(!Promise->FinallyDelegate.IsBound());
	Promise->FinallyDelegate = FFinallyDelegate::CreateLambda([SelfPromise = AsShared()]()
	{
		SelfPromise->Finalize();
	});
}

// TCallPromise<ErrorType, void> Implementation

template<typename ErrorType>
TCallPromisePtr<ErrorType, void> TCallPromise<ErrorType, void>::Create()
{
	return MakeShared<TCallPromise<ErrorType, void>>();
}

template<typename ErrorType>
TCallPromisePtr<ErrorType, void> TCallPromise<ErrorType, void>::Accepted()
{
	TCallPromisePtr<ErrorType, void> Promise = Create();
	Promise->Accept();
	return Promise;
}

template<typename ErrorType>
TCallPromisePtr<ErrorType, void> TCallPromise<ErrorType, void>::Rejected(const ErrorType& Error)
{
	TCallPromisePtr<ErrorType, void> Promise = Create();
	Promise->Reject(Error);
	return Promise;
}

template<typename ErrorType>
void TCallPromise<ErrorType, void>::Accept()
{
	bIsAccepted = true;
	if (AcceptDelegate.IsBound())
	{
		AcceptDelegate.ExecuteIfBound();
		AcceptDelegate.Unbind();
	}
	else
	{
		Finalize();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

template<typename ErrorType>
template<typename FuncType>
auto TCallPromise<ErrorType, void>::Then(FuncType Func, EnableIfPromisePtr<ErrorType, DeclReturnType(Func)> sfinae)
{
	using ReturnPromiseType = DeclReturnType(Func)::ElementType;
	TSharedPtr<ReturnPromiseType> Promise = ReturnPromiseType::Create();

	Append(Promise, FAcceptDelegate::CreateLambda(
		[Func = MoveTemp(Func), Promise]()
		{
			Promise->Merge(Func());
		}));

	return Promise;
}

template<typename ErrorType>
template<typename FuncType>
auto TCallPromise<ErrorType, void>::Then(FuncType Func, EnableIfValuePtr<ErrorType, DeclReturnType(Func)> sfinae)
{
	using ReturnType = DeclReturnType(Func);
	TCallPromisePtr<ErrorType, ReturnType> Promise = TCallPromise<ErrorType, ReturnType>::Create();

	Append(Promise, FAcceptDelegate::CreateLambda(
		[Func = MoveTemp(Func), Promise]()
		{
			Promise->Accept(Func());
		}));

	return Promise;
}

template<typename ErrorType>
template<typename FuncType>
auto TCallPromise<ErrorType, void>::Then(FuncType Func, EnableIfVoidPtr<ErrorType, DeclReturnType(Func)> sfinae)
{
	TCallPromisePtr<ErrorType, void> Promise = TCallPromise<ErrorType, void>::Create();

	Append(Promise, FAcceptDelegate::CreateLambda(
		[Func = MoveTemp(Func), Promise]()
		{
			Func();
			Promise->Accept();
		}));

	return Promise;
}

template<typename ErrorType>
template<typename UserClass, typename FuncType>
auto TCallPromise<ErrorType, void>::WeakThen(UserClass* Object, FuncType Func, EnableIfPromisePtr<ErrorType, DeclReturnType(Func)> sfinae)
{
	using ReturnPromiseType = DeclReturnType(Func)::ElementType;
	TSharedPtr<ReturnPromiseType> Promise = ReturnPromiseType::Create();

	Append(Promise, FAcceptDelegate::CreateWeakLambda(Object,
		[Func = MoveTemp(Func), Promise]()
		{
			Promise->Merge(Func());
		}));

	return Promise;
}

template<typename ErrorType>
template<typename UserClass, typename FuncType>
auto TCallPromise<ErrorType, void>::WeakThen(UserClass* Object, FuncType Func, EnableIfValuePtr<ErrorType, DeclReturnType(Func)> sfinae)
{
	using ReturnType = DeclReturnType(Func);
	TCallPromisePtr<ErrorType, ReturnType> Promise = TCallPromise<ErrorType, ReturnType>::Create();

	Append(Promise, FAcceptDelegate::CreateWeakLambda(Object,
		[Func = MoveTemp(Func), Promise]()
		{
			Promise->Accept(Func());
		}));

	return Promise;
}

template<typename ErrorType>
template<typename UserClass, typename FuncType>
auto TCallPromise<ErrorType, void>::WeakThen(UserClass* Object, FuncType Func, EnableIfVoidPtr<ErrorType, DeclReturnType(Func)> sfinae)
{
	TCallPromisePtr<ErrorType, void> Promise = TCallPromise<ErrorType, void>::Create();

	Append(Promise, FAcceptDelegate::CreateWeakLambda(Object,
		[Func = MoveTemp(Func), Promise]()
		{
			Func();
			Promise->Accept();
		}));
	
	return Promise;
}

template<typename ErrorType>
void TCallPromise<ErrorType, void>::Append(TSharedPtr<TCallPromiseBase<ErrorType>> Promise, FAcceptDelegate Accept)
{
	if (bIsAccepted)
	{
		Accept.ExecuteIfBound();
		return;
	}
	
	if (RejectError.IsSet())
	{
		Promise->Reject(RejectError.GetValue());
		return;
	}

	if (bIsFinalized)
	{
		Promise->Finalize();
		return;
	}

	check(!AcceptDelegate.IsBound());
	AcceptDelegate = MoveTemp(Accept);

	check(!RejectDelegate.IsBound());
	RejectDelegate = FRejectDelegate::CreateLambda([Promise](const ErrorType& Error)
	{
		Promise->Reject(Error);
	});

	check(!FinallyDelegate.IsBound());
	FinallyDelegate = FFinallyDelegate::CreateLambda([Promise]()
	{
		Promise->Finalize();
	});
}

template<typename ErrorType>
void TCallPromise<ErrorType, void>::Merge(TCallPromisePtr<ErrorType, void> Promise)
{
	if (Promise->bIsAccepted)
	{
		Accept();
		return;
	}

	if (Promise->RejectError.IsSet())
	{
		Reject(Promise->RejectError.GetValue());
		return;
	}

	if (Promise->bIsFinalized)
	{
		Finalize();
		return;
	}

	check(!Promise->AcceptDelegate.IsBound());
	Promise->AcceptDelegate = FAcceptDelegate::CreateLambda([this, SelfPromise = AsShared()]()
	{
		// SelfPromise prevents deletion
		Accept();
	});

	check(!Promise->RejectDelegate.IsBound());
	Promise->RejectDelegate = FRejectDelegate::CreateLambda([SelfPromise = AsShared()](const ErrorType& Error)
	{
		SelfPromise->Reject(Error);
	});

	check(!Promise->FinallyDelegate.IsBound());
	Promise->FinallyDelegate = FFinallyDelegate::CreateLambda([SelfPromise = AsShared()]()
	{
		SelfPromise->Finalize();
	});
}
